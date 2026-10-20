#ifndef GOOGLE_CLOUD_TRANSPORT_H
#define GOOGLE_CLOUD_TRANSPORT_H
#include <stddef.h>

/* mbedTLS BIO callbacks for the Google client's nonblocking LwIP socket. */
int GoogleCloud_TlsRecv(void *context, unsigned char *buffer, size_t length);
int GoogleCloud_TlsSend(void *context, const unsigned char *buffer, size_t length);
#endif
