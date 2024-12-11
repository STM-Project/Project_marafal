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


#if defined(MBEDTLS_SSL_CACHE_C)
#include "mbedtls/ssl_cache.h"
#endif

#include "string.h"
#include "FreeRTOS.h"
#include "task.h"
#include "sys.h"
#include "platform.h"
#include "memory_buffer_alloc.h"

#if defined(MBEDTLS_SSL_CACHE_C)
#include "mbedtls/ssl_cache.h"
#endif

#include "string.h"
#include "ff.h"
#include "sd_card.h"
#include "timer.h"

/* USER CODE END 1 */

/* Global variables ---------------------------------------------------------*/
mbedtls_ssl_context ssl;
mbedtls_ssl_config conf;
mbedtls_x509_crt cert;
mbedtls_ctr_drbg_context ctr_drbg;
mbedtls_entropy_context entropy;

/* USER CODE BEGIN 2 */

extern void Dbg(int on, char *txt);
extern char* GETVAL_ptr();

static mbedtls_net_context listen_fd, client_fd;
mbedtls_x509_crt srvcert;
mbedtls_pk_context pkey;
static const uint8_t *pers = (uint8_t*) "ssl_server";

#if defined(MBEDTLS_SSL_CACHE_C)
 mbedtls_ssl_cache_context cache;
#endif

static char buf[2400];

unsigned char memory_buf[4*16384];// __attribute__ ((section(".sdram")));

sys_thread_t  defaultTaskHandle2;

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

static void SSL_Server(void *arg)   //INFO o heap4 !!! https://www.freertos.org/Documentation/02-Kernel/02-Kernel-features/09-Memory-management/01-Memory-management
{
	int ret;

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

	 // https://gitlab.com/suyu-emu/mbedtls/-/blob/mbedtls-2.16/library/certs.c
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



  do{

		startt:

			mbedtls_net_free(&client_fd);
			mbedtls_ssl_session_reset(&ssl);




		if ((ret = mbedtls_net_accept(&listen_fd, &client_fd, NULL, 0, NULL)) != 0)  //To TU OCZEKUJE !!!
			goto exit;

		mbedtls_ssl_set_bio(&ssl, &client_fd, mbedtls_net_send, mbedtls_net_recv, NULL);


			while ((ret = mbedtls_ssl_handshake(&ssl)) != 0)
			{
				if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE)
					goto startt;
			}


		int len = sizeof(buf) - 1;
		memset(buf, 0, sizeof(buf));

		ret = mbedtls_ssl_read(&ssl, (unsigned char*) buf, len); /*buf[30]=0;*/



		if(strstr(buf, "GET / "))  //if ((len >= 5) && (strncmp(buf, "GET /", 5) == 0))
		{

			  if(TakeMutex2(Semphr_sdram,Semphr_cardSD,1000))
			  {
				  SDCardFileOpen(0,"aaa.htm",FA_READ);
				  int len = SDCardFileRead(0, GETVAL_ptr(0), 180000);
				  SDCardFileClose(0);

				  int count=0;
				  while(1)
				  {
					  if(len < 16384)  //MBEDTLS_SSL_OUT_CONTENT_LEN
					  {
							while ((ret = mbedtls_ssl_write(&ssl, (unsigned char*)GETVAL_ptr(count), len)) <= 0)
							{
								if (ret == MBEDTLS_ERR_NET_CONN_RESET)
									goto exit;
								if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE)
									goto exit;
							}
							break;
					  }
					  else
					  {
							while ((ret = mbedtls_ssl_write(&ssl, (unsigned char*)GETVAL_ptr(count), 16384)) <= 0)
							{
								if (ret == MBEDTLS_ERR_NET_CONN_RESET)
									goto exit;
								if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE)
									goto exit;
							}
							count += 16384;
							len -= 16384;

					  }
				  }
				  GiveMutex2(Semphr_sdram,Semphr_cardSD);
				  Dbg(1,"\r\nGET_GET_GET...");

			  }


    }
	 else
	 {
		 while ((ret = mbedtls_ssl_write(&ssl, (unsigned char*)"1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890", 100)) <= 0)
		 {
			 if (ret == MBEDTLS_ERR_NET_CONN_RESET)
				 goto exit;
			 if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE)
				 goto exit;
		 }
		 Dbg(1,"i");
	 }



//			while ((ret = mbedtls_ssl_write(&ssl, (unsigned char*) "<html><body>1234567890</body></html>", 36)) <= 0)
//			{
//				if (ret == MBEDTLS_ERR_NET_CONN_RESET)
//					goto exit;
//
//				if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE)
//					goto exit;
//			}





// if(XXXXX_DD > SSSS_FFF)
// 	#error
// w mbdetls dac buffer  GETVAL_ptr() !!!!

	  
		while ((ret = mbedtls_ssl_close_notify(&ssl)) < 0)
		{
			if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE)
				goto exit;
		}



		//goto renegat;

  }while(1);

	//}while(1);



//  renegat:
//
//  do{
//
//
//
//		if ((ret = mbedtls_net_accept(&listen_fd, &client_fd, NULL, 0, NULL)) != 0)  //To TU OCZEKUJE !!!
//			goto exit;
//
//
//		  ret =  mbedtls_ssl_renegotiate(&ssl);
//			if (ret != 0)
//				goto exit;
//
//			goto Renegat_Jeszcze;
//
//
//
//  }while(1);





		exit:

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

		osThreadTerminate(defaultTaskHandle2);



}


void https_server_netconn_init(void)
{
	defaultTaskHandle2 = sys_thread_new("HTTPS", SSL_Server, NULL, 1024, 4);
}

/* USER CODE END 4 */

/**
  * @}
  */

/**
  * @}
  */

