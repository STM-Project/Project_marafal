#include "google_cloud.h"
#include "google_cloud_config.h"
#include "google_cloud_protocol.h"
#include "google_cloud_roots.h"
#include "FreeRTOS.h"
#include "task.h"
#include "stm32f7xx_hal.h"
#include "rng.h"
#include "lwip/sockets.h"
#include "lwip/netdb.h"
#include "lwip/tcpip.h"
#include "lwip/dns.h"
#include "lwip/apps/sntp.h"
#include "mbedtls/ssl.h"
#include "mbedtls/net_sockets.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/platform_time.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

volatile GoogleCloudStatus googleCloudStatus;
static TaskHandle_t cloudTask;
static uint32_t utcBase, utcTick;
/* Used by this task only; keep HTTP buffers off the task stack. */
static char wire[8192], request[3072], redirectPath[2048];
static CloudHttpResponse response;

void GoogleCloud_SetUtc(uint32_t seconds)
{
    if (seconds < 1735689600U) return; /* refuse dates before 2025 */
    taskENTER_CRITICAL();
    utcBase = seconds;
    utcTick = HAL_GetTick();
    taskEXIT_CRITICAL();
}

static mbedtls_time_t cloudTime(mbedtls_time_t *out)
{
    uint32_t now, base, tick;
    taskENTER_CRITICAL();
    base = utcBase; tick = utcTick; now = HAL_GetTick();
    taskEXIT_CRITICAL();
    /* SNTP refreshes hourly. Stop TLS if the clock has gone stale. */
    mbedtls_time_t result = (!base || (uint32_t)(now-tick) > 86400000U) ?
        0 : (mbedtls_time_t)(base + (uint32_t)(now-tick)/1000U);
    if (out) *out = result;
    return result;
}

static void startSntp(void *unused)
{
    (void)unused;
    if (ip_addr_isany(dns_getserver(0))) {
        ip_addr_t server;
        IP_ADDR4(&server, 8, 8, 8, 8);
        dns_setserver(0, &server);
        IP_ADDR4(&server, 8, 8, 4, 4);
        dns_setserver(1, &server);
    }
    if (!sntp_enabled()) {
        sntp_setoperatingmode(SNTP_OPMODE_POLL);
        sntp_setservername(0, "time.google.com");
        sntp_init();
    }
}

__attribute__((weak)) int GoogleCloud_ReadTemperature10(void) { return 255; }

/* Use the HAL hardware RNG directly: no shared global entropy context. */
static int randomBytes(void *unused, unsigned char *out, size_t length)
{
    (void)unused;
    while (length) {
        uint32_t word;
        size_t n = length < sizeof(word) ? length : sizeof(word);
        if (HAL_RNG_GenerateRandomNumber(&hrng, &word) != HAL_OK) return -1;
        memcpy(out, &word, n);
        out += n; length -= n;
    }
    return 0;
}

static int expired(uint32_t start)
{
    return (uint32_t)(HAL_GetTick()-start) >= GOOGLE_CLOUD_TIMEOUT_MS;
}

/* LwIP DNS has its own bounded retry policy. TCP and TLS use nonblocking I/O. */
static int connectHost(mbedtls_net_context *net, const char *host, uint32_t start)
{
    struct addrinfo hints, *list = NULL, *p;
    int result = -10;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    if (lwip_getaddrinfo(host, "443", &hints, &list)) return -11;
    for (p=list; p && !expired(start); p=p->ai_next) {
        int fd = lwip_socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (fd < 0) continue;
        if (lwip_fcntl(fd, F_SETFL, O_NONBLOCK) < 0) { lwip_close(fd); continue; }
        int rc = lwip_connect(fd, p->ai_addr, p->ai_addrlen);
        if (rc < 0 && errno == EINPROGRESS) {
            do {
                fd_set writes;
                struct timeval timeout = {0, 200000};
                FD_ZERO(&writes); FD_SET(fd, &writes);
                rc = lwip_select(fd+1, NULL, &writes, NULL, &timeout);
            } while (rc == 0 && !expired(start));
            if (rc > 0) {
                int error = 0;
                socklen_t size = sizeof(error);
                rc = lwip_getsockopt(fd, SOL_SOCKET, SO_ERROR, &error, &size);
                if (!rc && error) rc = -1;
            } else rc = -1;
        }
        if (!rc) { net->fd = fd; result = 0; break; }
        lwip_close(fd);
    }
    lwip_freeaddrinfo(list);
    return result;
}

static int retryTls(int code, uint32_t start)
{
    if ((code == MBEDTLS_ERR_SSL_WANT_READ || code == MBEDTLS_ERR_SSL_WANT_WRITE) &&
        !expired(start)) { vTaskDelay(pdMS_TO_TICKS(5)); return 1; }
    return 0;
}

static int exchange(const char *host, const char *path, const char *post,
                    mbedtls_x509_crt *ca, mbedtls_ctr_drbg_context *rng)
{
    mbedtls_net_context net;
    mbedtls_ssl_context ssl;
    mbedtls_ssl_config config;
    uint32_t start = HAL_GetTick();
    int rc, length;
    size_t sent = 0, received = 0;
    googleCloudStatus.certificate_flags = 0;
    mbedtls_net_init(&net);
    mbedtls_ssl_init(&ssl);
    mbedtls_ssl_config_init(&config);
    rc = connectHost(&net, host, start);
    if (rc) goto done;
    rc = mbedtls_ssl_config_defaults(&config, MBEDTLS_SSL_IS_CLIENT,
                                   MBEDTLS_SSL_TRANSPORT_STREAM, MBEDTLS_SSL_PRESET_DEFAULT);
    if (rc) goto done;
    mbedtls_ssl_conf_authmode(&config, MBEDTLS_SSL_VERIFY_REQUIRED);
    mbedtls_ssl_conf_ca_chain(&config, ca, NULL);
    mbedtls_ssl_conf_rng(&config, mbedtls_ctr_drbg_random, rng);
    mbedtls_ssl_conf_min_version(&config, MBEDTLS_SSL_MAJOR_VERSION_3, MBEDTLS_SSL_MINOR_VERSION_3);
    rc = mbedtls_ssl_setup(&ssl, &config);
    if (rc) goto done;
    rc = mbedtls_ssl_set_hostname(&ssl, host); /* SNI + hostname verification */
    if (rc) goto done;
    mbedtls_ssl_set_bio(&ssl, &net, mbedtls_net_send, mbedtls_net_recv, NULL);
    do { rc = mbedtls_ssl_handshake(&ssl); } while (retryTls(rc, start));
    googleCloudStatus.certificate_flags = mbedtls_ssl_get_verify_result(&ssl);
    if (rc || googleCloudStatus.certificate_flags) { if (!rc) rc = -12; goto done; }
    if (post) {
        length = snprintf(request, sizeof(request),
            "POST %s HTTP/1.1\r\nHost: %s\r\nContent-Type: application/x-www-form-urlencoded\r\n"
            "Content-Length: %u\r\nConnection: close\r\n\r\n%s", path, host, (unsigned)strlen(post), post);
    } else {
        length = snprintf(request, sizeof(request),
            "GET %s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n", path, host);
    }
    if (length < 0 || (size_t)length >= sizeof(request)) { rc = -13; goto done; }
    while (sent < (size_t)length && !expired(start)) {
        rc = mbedtls_ssl_write(&ssl, (unsigned char *)request+sent, (size_t)length-sent);
        if (rc > 0) sent += (size_t)rc;
        else if (!retryTls(rc, start)) { if (!rc) rc = -14; goto done; }
    }
    if (sent != (size_t)length) { rc = -15; goto done; }
    while (!expired(start)) {
        rc = mbedtls_ssl_read(&ssl, (unsigned char *)wire+received, sizeof(wire)-received);
        if (rc == MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY) rc = 0;
        if (rc >= 0) {
            int eof = (rc == 0);
            received += (size_t)rc;
            rc = CloudHttp_Parse(wire, received, eof, &response);
            if (rc == 0) goto done;
            if (rc < 0 || eof || received == sizeof(wire)) { rc = -16; goto done; }
        } else if (!retryTls(rc, start)) goto done;
    }
    rc = -17;
done:
    /* Closing a nonblocking socket is bounded; no unbounded close_notify loop. */
    mbedtls_net_free(&net);
    mbedtls_ssl_free(&ssl);
    mbedtls_ssl_config_free(&config);
    return rc;
}

static int configValid(void)
{
    const char *p;
    if (strlen(GOOGLE_CLOUD_TOKEN) != 64 || strlen(GOOGLE_CLOUD_DEVICE) < 1 ||
        strlen(GOOGLE_CLOUD_DEVICE) > 32) return 0;
    for (p=GOOGLE_CLOUD_TOKEN; *p; ++p) if (!isxdigit((unsigned char)*p)) return 0;
    for (p=GOOGLE_CLOUD_DEVICE; *p; ++p)
        if (!isalnum((unsigned char)*p) && *p != '-' && *p != '_') return 0;
    if (strncmp(GOOGLE_CLOUD_PATH, "/macros/s/", 9) || strlen(GOOGLE_CLOUD_PATH) > 512 ||
        strlen(GOOGLE_CLOUD_PATH) < 16) return 0;
    for (p=GOOGLE_CLOUD_PATH; *p; ++p)
        if (!isalnum((unsigned char)*p) && *p != '-' && *p != '_' && *p != '/') return 0;
    return strcmp(GOOGLE_CLOUD_PATH+strlen(GOOGLE_CLOUD_PATH)-5, "/exec") == 0;
}

static void cloudMain(void *unused)
{
    mbedtls_x509_crt ca;
    mbedtls_ctr_drbg_context rng;
    TickType_t wake = xTaskGetTickCount();
    (void)unused;
    mbedtls_x509_crt_init(&ca);
    mbedtls_ctr_drbg_init(&rng);
    int rc = mbedtls_x509_crt_parse(&ca, (const unsigned char *)google_cloud_roots, sizeof(google_cloud_roots));
    if (!rc) rc = mbedtls_ctr_drbg_seed(&rng, randomBytes, NULL,
        (const unsigned char *)GOOGLE_CLOUD_DEVICE, strlen(GOOGLE_CLOUD_DEVICE));
    if (rc) {
        googleCloudStatus.last_error = rc;
        googleCloudStatus.failures++;
        mbedtls_ctr_drbg_free(&rng); mbedtls_x509_crt_free(&ca);
        cloudTask = NULL;
        vTaskDelete(NULL);
        return;
    }
    for (;;) {
        unsigned char random[16];
        char nonce[33], post[256];
        int relay, temp = GoogleCloud_ReadTemperature10();
        googleCloudStatus.temperature10 = temp;
        if (!cloudTime(NULL)) rc = -2;
        else if (temp < -500 || temp > 1500) rc = -18;
        else if (!(rc = mbedtls_ctr_drbg_random(&rng, random, sizeof(random)))) {
            for (unsigned i=0; i<sizeof(random); ++i) sprintf(nonce+2*i, "%02x", random[i]);
            snprintf(post, sizeof(post), "token=%s&device=%s&temperature10=%d&relayAck=%d&nonce=%s",
                GOOGLE_CLOUD_TOKEN, GOOGLE_CLOUD_DEVICE, temp, googleCloudStatus.demo_relay, nonce);
            rc = exchange("script.google.com", GOOGLE_CLOUD_PATH, post, &ca, &rng);
            if (!rc && (response.status == 302 || response.status == 303)) {
                rc = CloudRedirect_Path(response.location, redirectPath, sizeof(redirectPath)) ? -23 : 0;
                if (!rc) rc = exchange("script.googleusercontent.com", redirectPath, NULL, &ca, &rng);
            }
            if (!rc && response.status != 200) rc = -19;
            if (!rc) rc = CloudReply_Parse(response.body, nonce, &relay) ? -22 : 0;
            if (!rc) googleCloudStatus.demo_relay = relay; /* deliberately RAM only */
        }
        googleCloudStatus.last_error = rc;
        if (rc) googleCloudStatus.failures++; else googleCloudStatus.successes++;
        /* Poll every 10 seconds; never run catch-up requests after a slow attempt. */
        if ((TickType_t)(xTaskGetTickCount()-wake) >= pdMS_TO_TICKS(GOOGLE_CLOUD_PERIOD_MS))
            wake = xTaskGetTickCount();
        vTaskDelayUntil(&wake, pdMS_TO_TICKS(GOOGLE_CLOUD_PERIOD_MS));
    }
}

void GoogleCloud_Start(void)
{
    if (!GOOGLE_CLOUD_ENABLED || cloudTask) return;
    if (!configValid()) { googleCloudStatus.last_error = -1; return; }
    mbedtls_platform_set_time(cloudTime);
    if (tcpip_callback(startSntp, NULL) != ERR_OK) { googleCloudStatus.last_error = -20; return; }
    if (xTaskCreate(cloudMain, "GoogleCloud", 4096, NULL, tskIDLE_PRIORITY+1, &cloudTask) != pdPASS)
        googleCloudStatus.last_error = -21;
}
