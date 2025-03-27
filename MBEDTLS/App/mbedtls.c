/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : mbedtls.c
  * Description        : This file provides code for the configuration
  *                      of the mbedtls instances.
  ******************************************************************************
  ******************************************************************************
   * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "mbedtls.h"

/* USER CODE BEGIN 0 */
#include "mbedtls/certs.h"
#include "mbedtls/x509.h"
#include "mbedtls/net_sockets.h"
#include "mbedtls/error.h"
/* USER CODE END 0 */

/* USER CODE BEGIN 1 */
#include "memory_buffer_alloc.h"
#if defined(MBEDTLS_SSL_CACHE_C)
#include "mbedtls/ssl_cache.h"
#endif
#include "FreeRTOS.h"
#include "task.h"
#include "sys.h"
#include "ff.h"
#include "sd_card.h"
#include "timer.h"

#include "string.h"
#include "string_oper.h"
#include "mini_printf.h"
/* USER CODE END 1 */

/* Global variables ---------------------------------------------------------*/
mbedtls_ssl_context ssl;
mbedtls_ssl_config conf;
mbedtls_x509_crt cert;
mbedtls_ctr_drbg_context ctr_drbg;
mbedtls_entropy_context entropy;

/* USER CODE BEGIN 2 */
#define HTTPS_DEBUG	1
#define HTTPS_MAX_WRITE_BUFF	16384

#if HTTPS_MAX_WRITE_BUFF > MBEDTLS_SSL_MAX_CONTENT_LEN
#error "Write buffer size NOT large as MBEDTLS_SSL_MAX_CONTENT_LEN"
#endif

extern void Dbg(int on, char *txt);
extern char* GETVAL_ptr();

static mbedtls_net_context listen_fd, client_fd;
static mbedtls_x509_crt srvcert;		/* Create own certificate -> https://base64.guru/converter/decode/hex  (hex-ascii or ascii-hex) */
static mbedtls_pk_context pkey;
static const uint8_t *pers = (uint8_t*) "ssl_server";

#if defined(MBEDTLS_SSL_CACHE_C)
 mbedtls_ssl_cache_context cache;
#endif

unsigned char memory_buf[4*HTTPS_MAX_WRITE_BUFF];
static char buffRecv[110];

static sys_thread_t  vTaskHandleServer;
/* USER CODE END 2 */

/* MBEDTLS init function */
void MX_MBEDTLS_Init(void)
{
   /**
  */
  mbedtls_ssl_init(&ssl);
  mbedtls_ssl_config_init(&conf);
  mbedtls_x509_crt_init(&cert);
  mbedtls_ctr_drbg_init(&ctr_drbg);
  mbedtls_entropy_init( &entropy );
  /* USER CODE BEGIN 3 */

  /* USER CODE END 3 */

}

/* USER CODE BEGIN 4 */
static int HTTPS_send(mbedtls_ssl_context *ssl, char *data, size_t len){
	int ret=0;
	while ((ret = mbedtls_ssl_write(ssl, (const unsigned char*)data, len)) <= 0){
		if (ret == MBEDTLS_ERR_NET_CONN_RESET)
			return 1;
		else if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE)
			return 2;
	}
	return 0;	/* return(ret)  -  'ret' in this line code is bytes written, not use yet */
}
static int HTTPS_recv(mbedtls_ssl_context *ssl, char *data, size_t len){
	int ret = mbedtls_ssl_read(ssl, (unsigned char*)data, len);
   if(ret <= 0)	/* MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY,	MBEDTLS_ERR_NET_CONN_RESET,	MBEDTLS_ERR_SSL_WANT_READ,	  MBEDTLS_ERR_SSL_WANT_WRITE,	  ... */
   	return 1;
   else if(ret < 5)
   	return 2;
   return 0;	/* return(ret)  -  'ret' in this line code is bytes read, not use yet */
}

static void HTTPS_close(void){
	mbedtls_net_free(&client_fd);
	mbedtls_net_free(&listen_fd);
	mbedtls_x509_crt_free(&srvcert);
	mbedtls_pk_free(&pkey);
	mbedtls_ssl_free(&ssl);
	mbedtls_ssl_config_free(&conf);
	#if defined(MBEDTLS_SSL_CACHE_C)
	 mbedtls_ssl_cache_free(&cache);
	#endif
	mbedtls_ctr_drbg_free(&ctr_drbg);
	mbedtls_entropy_free(&entropy);
}

static void SSL_Server(void *arg)
{
	int ret,len;

	/* START__SSL_Server: */
	#ifdef MBEDTLS_MEMORY_BUFFER_ALLOC_C
		mbedtls_memory_buffer_alloc_init(memory_buf, sizeof(memory_buf));
	#endif
	mbedtls_net_init(&listen_fd);
	mbedtls_net_init(&client_fd);
	mbedtls_ssl_init(&ssl);
	mbedtls_ssl_config_init(&conf);
	#if defined(MBEDTLS_SSL_CACHE_C)
		mbedtls_ssl_cache_init(&cache);
	#endif
	mbedtls_x509_crt_init(&cert);
	mbedtls_ctr_drbg_init(&ctr_drbg);
	mbedtls_entropy_init( &entropy );

	mbedtls_pk_init(&pkey);

	ret = mbedtls_x509_crt_parse(&srvcert, (const unsigned char *) mbedtls_test_srv_crt, mbedtls_test_srv_crt_len);
	if (ret != 0)
		goto exit;

	ret = mbedtls_x509_crt_parse(&srvcert, (const unsigned char *) mbedtls_test_cas_pem, mbedtls_test_cas_pem_len);
	if (ret != 0)
		goto exit;

	ret = mbedtls_pk_parse_key(&pkey, (const unsigned char *) mbedtls_test_srv_key, mbedtls_test_srv_key_len, NULL, 0);
	if (ret != 0)
		goto exit;


	if ((ret = mbedtls_net_bind(&listen_fd, NULL, "443", MBEDTLS_NET_PROTO_TCP)) != 0)
		goto exit;

	ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy, (const unsigned char *) pers, strlen((char *) pers));
	if (ret != 0)
		goto exit;

	ret = mbedtls_ssl_config_defaults(&conf, MBEDTLS_SSL_IS_SERVER, MBEDTLS_SSL_TRANSPORT_STREAM, MBEDTLS_SSL_PRESET_DEFAULT);
	if (ret != 0)
		goto exit;

	mbedtls_ssl_conf_rng(&conf, mbedtls_ctr_drbg_random, &ctr_drbg);


	#if defined(MBEDTLS_SSL_CACHE_C)
		mbedtls_ssl_conf_session_cache(&conf, &cache, mbedtls_ssl_cache_get, mbedtls_ssl_cache_set);
	#endif

	mbedtls_ssl_conf_ca_chain(&conf, srvcert.next, NULL);

	ret = mbedtls_ssl_conf_own_cert(&conf, &srvcert, &pkey);
	if(ret != 0)
		goto exit;

	ret = mbedtls_ssl_setup(&ssl, &conf);
	if(ret != 0)
		goto exit;

	mbedtls_ssl_conf_read_timeout(&conf, 2000);

	do
	{
		RESET_Connection:
		mbedtls_net_free(&client_fd);
		mbedtls_ssl_session_reset(&ssl);

		ret = mbedtls_net_accept(&listen_fd, &client_fd, NULL, 0, NULL);		/* wait for connection */
		if (ret != 0)
			goto RESET_Connection;

		mbedtls_ssl_set_bio(&ssl, &client_fd, mbedtls_net_send, mbedtls_net_recv, mbedtls_net_recv_timeout);

		while ((ret = mbedtls_ssl_handshake(&ssl)) != 0)
		{
			if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE)
				goto RESET_Connection;
		}

		len = sizeof(buffRecv);
		memset(buffRecv, 0, len);
		if(HTTPS_recv(&ssl, buffRecv, len-1))
			goto RESET_Connection;

		if(0==strncmp(buffRecv, "GET / ...", 6))		/* strstr(buffRecv, "GET / ") */
		{
			if(TakeMutex2(Semphr_sdram, Semphr_cardSD, 1000))
			{
				int count=0, len;
				SDCardFileOpen(0,"aaa.htm",FA_READ);
				len = SDCardFileRead(0, GETVAL_ptr(0), 180000);
				SDCardFileClose(0);
				GiveMutex(Semphr_cardSD);

				while(1)
				{
					if(len < HTTPS_MAX_WRITE_BUFF)
					{
						if(HTTPS_send(&ssl,GETVAL_ptr(count),len)){	GiveMutex(Semphr_sdram);
							goto RESET_Connection;	}
						break;
					}
					else
					{
						if(HTTPS_send(&ssl,GETVAL_ptr(count),HTTPS_MAX_WRITE_BUFF)){	GiveMutex(Semphr_sdram);
							goto RESET_Connection;	}

						count += HTTPS_MAX_WRITE_BUFF;
						len -= HTTPS_MAX_WRITE_BUFF;
					}
				}
				GiveMutex(Semphr_sdram);
				Dbg(HTTPS_DEBUG,"\r\nGET...");
			}
		}
		else if(0==strncmp(buffRecv, "GET /test", 9))
		{
			mini_snprintf(buffRecv,sizeof(buffRecv)-1,"<html><body>Test: %s</body></html>",mbedtls_ssl_get_ciphersuite(&ssl));

			if(HTTPS_send(&ssl,buffRecv,strlen(buffRecv)))
				goto RESET_Connection;

			Dbg(HTTPS_DEBUG,"j");
		}
		else
		{
			static int testVar=0;
			char *ptr = Int2Str(++testVar,Zero,6,Sign_none);

			strcpy(buffRecv,"1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890");
			buffRecv[0] = *(ptr+0);
			buffRecv[1] = *(ptr+1);
			buffRecv[2] = *(ptr+2);
			buffRecv[3] = *(ptr+3);
			buffRecv[4] = *(ptr+4);
			buffRecv[5] = *(ptr+5);
			buffRecv[6] = ' ';
			buffRecv[7] = ' ';

			if(HTTPS_send(&ssl,buffRecv,100))
				goto RESET_Connection;

			Dbg(HTTPS_DEBUG,"i");
		}

	  


		while ((ret = mbedtls_ssl_close_notify(&ssl)) < 0)
		{
			if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE)
				goto RESET_Connection;
		}


	}while(1);


	exit:
	HTTPS_close();
	/* goto START__SSL_Server; */

	osThreadTerminate(vTaskHandleServer);

}


void https_server_netconn_init(void)
{
	vTaskHandleServer = sys_thread_new("HTTPS", SSL_Server, NULL, 1024, -2);
}

/* USER CODE END 4 */

/**
  * @}
  */

/**
  * @}
  */

