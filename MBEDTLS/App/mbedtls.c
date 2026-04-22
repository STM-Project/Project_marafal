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
#include "lwip/api.h"
#include "platform.h"
#include "_smtp.h"
#include "variables.h"
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
#define SMTP_MAIL_BUFFER	2048

#if HTTPS_MAX_WRITE_BUFF > MBEDTLS_SSL_MAX_CONTENT_LEN
#error "Write buffer size NOT large as MBEDTLS_SSL_MAX_CONTENT_LEN"
#endif

extern void Dbg(int on, char *txt);
extern char* GETVAL_ptr();

static mbedtls_net_context listen_fd, client_fd;
static mbedtls_pk_context pkey;
static char sendBuffer[SMTP_MAIL_BUFFER];  	//daj jako malloc !!!!

#if defined(MBEDTLS_SSL_CACHE_C)
 mbedtls_ssl_cache_context cache;
#endif

 __attribute__ ((section(".sdram"))) unsigned char memory_buf[8*HTTPS_MAX_WRITE_BUFF];
//__attribute__ ((section(".sdram")))  __attribute__((aligned(8)))unsigned char memory_buf2[4*HTTPS_MAX_WRITE_BUFF];

static char buffRecv[110];
uint8_t vrfy_buf[512]={0};

static sys_thread_t  vTaskHandleServer;
/* USER CODE END 2 */

/* MBEDTLS init function */
void MX_MBEDTLS_Init(void)
{
   /**
  */
  mbedtls_ssl_init(&ssl);
  mbedtls_ssl_config_init(&conf);
  mbedtls_x509_crt_init(&cert);					/* Create own certificate -> https://base64.guru/converter/decode/hex  (hex-ascii or ascii-hex) */
  mbedtls_ctr_drbg_init(&ctr_drbg);
  mbedtls_entropy_init( &entropy );
  /* USER CODE BEGIN 3 */

  /* USER CODE END 3 */

}

/* USER CODE BEGIN 4 */

/*-------------------- HTTPS ---------------- */

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
	mbedtls_x509_crt_free(&cert);
	mbedtls_pk_free(&pkey);
	mbedtls_ssl_free(&ssl);
	mbedtls_ssl_config_free(&conf);
	#if defined(MBEDTLS_SSL_CACHE_C)
	 mbedtls_ssl_cache_free(&cache);
	#endif
	mbedtls_ctr_drbg_free(&ctr_drbg);
	mbedtls_entropy_free(&entropy);
}

static const int my_non_rsa_ciphers[] = {
    MBEDTLS_TLS_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256,
	 MBEDTLS_TLS_ECDH_ECDSA_WITH_AES_128_GCM_SHA256,
    0
};

static void SSL_Server(void *arg)
{
	TakeMutex(Semphr_sdram, 3000);

	int ret,len;
	const uint8_t *pers = (uint8_t*) "ssl_server";

	/* START__SSL_Server: */
	#ifdef MBEDTLS_MEMORY_BUFFER_ALLOC_C
		mbedtls_memory_buffer_alloc_init(memory_buf, sizeof(memory_buf));
	#endif

		MX_MBEDTLS_Init();

	mbedtls_net_init(&listen_fd);
	mbedtls_net_init(&client_fd);

	#if defined(MBEDTLS_SSL_CACHE_C)
		mbedtls_ssl_cache_init(&cache);
	#endif

	mbedtls_pk_init(&pkey);

/*	ret = mbedtls_x509_crt_parse(&cert, (const unsigned char *) mbedtls_test_srv_crt, mbedtls_test_srv_crt_len); */
	ret = mbedtls_x509_crt_parse(&cert, (const unsigned char *) mbedtls_test_srv_crt_ec, mbedtls_test_srv_crt_ec_len);
	if (ret != 0)
		goto exit;

/*	ret = mbedtls_x509_crt_parse(&cert, (const unsigned char *) mbedtls_test_cas_pem, mbedtls_test_cas_pem_len);
	if (ret != 0)
		goto exit; */

/* ret = mbedtls_pk_parse_key(&pkey, (const unsigned char *) mbedtls_test_srv_key, mbedtls_test_srv_key_len, NULL, 0); */
	ret = mbedtls_pk_parse_key(&pkey, (const unsigned char *) mbedtls_test_srv_key_ec, mbedtls_test_srv_key_ec_len, NULL, 0);
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

/* mbedtls_ssl_conf_ciphersuites(&conf, mbedtls_ssl_list_ciphersuites()); */
	mbedtls_ssl_conf_ciphersuites(&conf, my_non_rsa_ciphers);

	mbedtls_ssl_conf_rng(&conf, mbedtls_ctr_drbg_random, &ctr_drbg);

	#if defined(MBEDTLS_SSL_CACHE_C)
		mbedtls_ssl_conf_session_cache(&conf, &cache, mbedtls_ssl_cache_get, mbedtls_ssl_cache_set);
	#endif

	mbedtls_ssl_conf_ca_chain(&conf, cert.next, NULL);

	ret = mbedtls_ssl_conf_own_cert(&conf, &cert, &pkey);
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

		GiveMutex(Semphr_sdram);

		ret = mbedtls_net_accept(&listen_fd, &client_fd, NULL, 0, NULL);		/* wait for connection */
		if (ret != 0)
			goto RESET_Connection;

		TakeMutex(Semphr_sdram, 3000);

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
			if(TakeMutex(Semphr_cardSD, 1000))
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
						if(HTTPS_send(&ssl,GETVAL_ptr(count),len)){
							goto RESET_Connection;	}
						break;
					}
					else
					{
						if(HTTPS_send(&ssl,GETVAL_ptr(count),HTTPS_MAX_WRITE_BUFF)){
							goto RESET_Connection;	}

						count += HTTPS_MAX_WRITE_BUFF;
						len -= HTTPS_MAX_WRITE_BUFF;
					}
				}

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
	Dbg(1,"___HTTPS CLOSED___");
	HTTPS_close();

	/* goto START__SSL_Server; */
	osThreadTerminate(vTaskHandleServer);
}

	/*----------------- SMTPS -------------- */

static int SMTP_SSL_Send(mbedtls_ssl_context *ssl, char *req )
{
	int len = strlen((char*)req);
	int ret2 = mbedtls_ssl_write(ssl,(const unsigned char*)req,len);

	 while(ret2 <= 0)
	 {
	   if( ret2 != MBEDTLS_ERR_SSL_WANT_READ && \
		   ret2 != MBEDTLS_ERR_SSL_WANT_WRITE )
	   {
	      return 0;
	   }
	 }
	 return 1;
}

static int SMTP_SSL_Reciev(mbedtls_ssl_context *ssl, char *req)
{
	unsigned char recvBuffer[1024] ={0};
	int ret2 = 0;

	memset(recvBuffer, 0, 1024);
	ret2 = mbedtls_ssl_read(ssl,recvBuffer,200);

	Dbg(1,(char*)recvBuffer);

	if(ret2 < 0)
		return 1;

	if(strstr((const char*)recvBuffer,req))
		return 0;
	else
		return 1;
}

static int SMTP_SSL_EHLO(mbedtls_ssl_context *ssl, char *serverName)
{
	char sendBuffer[96] = {0};

	if(NULL==serverName)
		return 1;

	if(SMTP_SSL_Reciev(ssl,"220"))
		return 1;

	strcat(sendBuffer,"EHLO ");
	strcat(sendBuffer, serverName);
	strcat(sendBuffer,"\r\n");
	SMTP_SSL_Send(ssl,sendBuffer);

	if(SMTP_SSL_Reciev(ssl,"250"))
		return 1;

	return 0;
}

static int SMTP_SSL_AuthLogin(mbedtls_ssl_context *ssl, s_smtp_sender *client)
{
	char sendBuffer[96] = {0};

	SMTP_SSL_Send(ssl,"AUTH LOGIN\r\n");

	if(SMTP_SSL_Reciev(ssl,"334"))
		return 1;

	if(NULL==client)
		return 1;
	strcat(sendBuffer,(const char*)base64_enc2(client->login));
	strcat(sendBuffer,"\r\n");
	SMTP_SSL_Send(ssl,sendBuffer);

	if(SMTP_SSL_Reciev(ssl,"334"))
		return 1;

	memset(sendBuffer, 0, 96);
	strcat(sendBuffer,(const char*)base64_enc2(client->password));
	strcat(sendBuffer,"\r\n");
	SMTP_SSL_Send(ssl,sendBuffer);

	if(SMTP_SSL_Reciev(ssl,"235"))
		return 1;

	return 0;
}

static int SMTP_SSL_MailFrom(mbedtls_ssl_context *ssl, char *mailAddress)
{
	char sendBuffer[96] = {0};

	if(NULL==mailAddress)
		return 1;

	strcat(sendBuffer,"MAIL From:<");
	strcat(sendBuffer, mailAddress);
	strcat(sendBuffer,">\r\n");
	SMTP_SSL_Send(ssl,sendBuffer);

	if(SMTP_SSL_Reciev(ssl,"250"))
		return 1;
	return 0;
}

static int SMTP_SSL_RecipientTo(mbedtls_ssl_context *ssl, s_smtp_recipient client[])
{
	char sendBuffer[96] = {0};
	uint8_t nuberOfRecipients = 0;

	for (int i = 0; i<MAX_EMAIL_RECIPIENTS; ++i)
	{
		if ((EmailSendParam.recepientsMask>>i)&0x01)
		{
			if(client[i].email[0]!=' ' && client[i].email[1]!='\0')
			{
				memset(sendBuffer, 0, 96);
				strncpy(sendBuffer,"RCPT TO:<",12);
				strncat(sendBuffer,&client[i].email[0],64);
				strncat(sendBuffer,">\r\n",5);
				SMTP_SSL_Send(ssl,sendBuffer);

				if(SMTP_SSL_Reciev(ssl,"250"))
					return 1;
				else
					nuberOfRecipients++;
			}
		}
	}
	if(0==nuberOfRecipients)
		return 1;
	else
		return 0;
}

static int SMTP_SSL_DATA(mbedtls_ssl_context *ssl)
{
	SMTP_SSL_Send(ssl,"DATA\r\n");
	if(SMTP_SSL_Reciev(ssl,"354"))
		return 1;

	return 0;
}

static int SMTP_SSL_QUIT(mbedtls_ssl_context *ssl)
{
	SMTP_SSL_Send(ssl,"quit\r\n");
	if(SMTP_SSL_Reciev(ssl,"221"))
		return 1;

	return 0;
}

static int SMTP_SSL_Connect(mbedtls_ssl_context *ssl, s_smtp_sender server, mbedtls_entropy_context *entropy,
		mbedtls_ctr_drbg_context *ctr_drbg, mbedtls_ssl_config *conf, mbedtls_x509_crt *cacert, mbedtls_net_context *server_fd)
{
	int len=0, ret=0;
	char IP_buff[17]={0};
	char Port_buff[4]={0};
	uint32_t flags2=0;
	const uint8_t *pers = (uint8_t *)("ssl_client");


	len = strlen((char *)pers);
	if((ret = mbedtls_ctr_drbg_seed(ctr_drbg, mbedtls_entropy_func, entropy, (const unsigned char *) pers, len)) != 0)
		return 1;

	/* 1. Initialize certificates */
/*	ret = mbedtls_x509_crt_parse(cacert, (const unsigned char *) mbedtls_test_cas_pem, mbedtls_test_cas_pem_len);
	if(ret < 0)
		return 1;
*/
	/* 2. Start the connection */
	if(0==server.IP) 	 return 1;
	if(0==server.port) return 1;

	mini_snprintf(IP_buff,sizeof(IP_buff), "%d.%d.%d.%d", server.IP&0xFF, (server.IP>>8)&0xFF, (server.IP>>16)&0xFF, (server.IP>>24)&0xFF);
	mini_snprintf(Port_buff,sizeof(Port_buff), "%d", server.port);

	if((ret = mbedtls_net_connect(server_fd,IP_buff,Port_buff,MBEDTLS_NET_PROTO_TCP)) != 0)
		return 1;

	/* 3. Setup stuff */
	if((ret = mbedtls_ssl_config_defaults(conf, MBEDTLS_SSL_IS_CLIENT, MBEDTLS_SSL_TRANSPORT_STREAM, MBEDTLS_SSL_PRESET_DEFAULT)) != 0)
		return 1;
/*
	mbedtls_ssl_conf_ciphersuites(&conf, mbedtls_ssl_list_ciphersuites());
	mbedtls_ssl_conf_ciphersuites(&conf, my_non_rsa_ciphers);
*/
	mbedtls_ssl_conf_authmode(conf, MBEDTLS_SSL_VERIFY_OPTIONAL);
	mbedtls_ssl_conf_ca_chain(conf, cacert, NULL);
	mbedtls_ssl_conf_rng(conf, mbedtls_ctr_drbg_random, ctr_drbg);

	if((ret = mbedtls_ssl_setup(ssl,conf)) != 0)
		return 1;

	if((ret = mbedtls_ssl_set_hostname(ssl,server.login/*server.name*/)) != 0)
		return 1;

	mbedtls_ssl_set_bio(ssl, server_fd, mbedtls_net_send, mbedtls_net_recv, NULL);

	/* 4. Handshake */
	while((ret = mbedtls_ssl_handshake(ssl)) != 0)
	{
		if( ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE)
			return 1;
	}

	/* 5. Verify the server certificate */
	if((flags2 = mbedtls_ssl_get_verify_result(ssl)) != 0)
	{
		mbedtls_x509_crt_verify_info((char *)vrfy_buf, sizeof(vrfy_buf), "  ! ", flags2);
	}
	else
		return 1;

	return 0;
}

static void SMTP_SSL_Disconnect(mbedtls_ssl_context *ssl, mbedtls_entropy_context *entropy, mbedtls_ctr_drbg_context *ctr_drbg,
		mbedtls_ssl_config *conf, mbedtls_x509_crt *cacert, mbedtls_net_context *server_fd)
{
	mbedtls_net_free			(server_fd);
	mbedtls_x509_crt_free	(cacert);
	mbedtls_ssl_free			(ssl);
	mbedtls_ssl_config_free	(conf);
	mbedtls_ctr_drbg_free	(ctr_drbg);
	mbedtls_entropy_free		(entropy);
}

static void EMAIL_HeadTestReport(Email_Send_Param *par, s_smtp_sender *send, s_smtp_recipient client[], char *sendBuffer)
{
	int n = mini_snprintf(sendBuffer,SMTP_MAIL_BUFFER,"From: %s\r\n",send->login);

	switch(par->id)
	{
		case EMAIL_TEST:
			n += mini_snprintf(sendBuffer + n, SMTP_MAIL_BUFFER - n, "Subject: TEST\r\n");
			break;

		case EMAIL_MEASURE:
			n += mini_snprintf(sendBuffer + n, SMTP_MAIL_BUFFER - n, "Subject: MEASURE\r\n");
			break;
	}

	n += mini_snprintf(sendBuffer + n, SMTP_MAIL_BUFFER - n, "To: ");

	for(int i=0;i<MAX_EMAIL_RECIPIENTS;++i)
	{
		if ((par->recepientsMask>>i)&0x01)
		{
			if(client[i].email[0]!=' ' && client[i].email[1]!='\0')
				n += mini_snprintf(sendBuffer + n,SMTP_MAIL_BUFFER - n,"%s,",&client[i].email[0]);
		}
	}
	n += mini_snprintf(sendBuffer + n, SMTP_MAIL_BUFFER - n, "\r\n");
	n += mini_snprintf(sendBuffer + n, SMTP_MAIL_BUFFER - n, "Content-Transfer-Encoding: 8bit\r\n");
	n += mini_snprintf(sendBuffer + n, SMTP_MAIL_BUFFER - n, "Content-Type: text/html; charset=\"UTF-8\"");
	n += mini_snprintf(sendBuffer + n, SMTP_MAIL_BUFFER - n, "\r\n\r\n");
}

static void EMAIL_Content(Email_Send_Param *par, char *sendBuffer, char* txt)
{
	int n=0;

	switch(par->id)
	{
		case EMAIL_TEST:
			n = mini_snprintf(sendBuffer, SMTP_MAIL_BUFFER, "<html><style>table{width: 20%%;} td{ border-bottom:1px solid #ddd; padding:7px; font-family:Arial; text-align:center; white-space:nowrap;}</style>");
			n += mini_snprintf(sendBuffer + n, SMTP_MAIL_BUFFER - n, "<table><tr><td>MODEL</td><td>MODEL</td></tr>");
			n += mini_snprintf(sendBuffer + n, SMTP_MAIL_BUFFER - n, "<tr><td>Firmware</td><td>%s</td></tr>");
			n += mini_snprintf(sendBuffer + n, SMTP_MAIL_BUFFER - n, "<tr><td>AA</td><td>11</td></tr>");
			n += mini_snprintf(sendBuffer + n, SMTP_MAIL_BUFFER - n, "<tr><td>BB</td><td>22</td></tr>");
			n += mini_snprintf(sendBuffer + n, SMTP_MAIL_BUFFER - n, "<tr><td>CC</td><td>33</td></tr>");
			n += mini_snprintf(sendBuffer + n, SMTP_MAIL_BUFFER - n, "</table></html>\r\n");
			n += mini_snprintf(sendBuffer + n, SMTP_MAIL_BUFFER - n, "\r\n.\r\n");
			break;

		case EMAIL_MEASURE:
			n = mini_snprintf(sendBuffer, SMTP_MAIL_BUFFER, "%s\r\n.\r\n",txt);
			break;
	}
}

static void EMAIL_ContentTest(Email_Send_Param *par, char *sendBuffer, char* txt)
{
	LOOP_FOR(i,SMTP_MAIL_BUFFER-40){ sendBuffer[i]='A'; sendBuffer[i+1]=0;}
}

static void EMAIL_SSL_SendData(mbedtls_ssl_context *ssl, Email_Send_Param *par, s_smtp_sender *send, s_smtp_recipient client[], char* txt)
{
	EMAIL_HeadTestReport(par,send,client,sendBuffer);
	SMTP_SSL_Send(ssl,sendBuffer);

	LOOP_FOR(i,300){
		EMAIL_ContentTest(par,sendBuffer,txt);
		SMTP_SSL_Send(ssl,sendBuffer);
	}
	EMAIL_Content(par,sendBuffer,txt);
	SMTP_SSL_Send(ssl,sendBuffer);

	SMTP_SSL_Reciev(ssl,"250");
}

static void vtaskSMTPS(void *mes)  //W wysylaniu emaili korzystam ze zmiennych globalnych nie robie kopi !!!! - w trakcie wysylania nie wolno zapisywac nowych danych do parametrow email
{

	uint8_t connectionError = 0;
	ip_addr_t IP_server = {0};
	Email_Send_Param _param = EmailSendParam;
	int selNad = _param.whichSender;

	netconn_gethostbyname(Const.emailSend[selNad].server, &IP_server);
	Const.emailSend[selNad].IP = IP_server.addr;
	//VAR_SetTabVal(Const_emailSend_IP, 0, IP_server.addr);

	if(IP_server.addr == 0)
	{
		Dbg(1,"Connection error");
		vTaskDelete(NULL);
	}

	mbedtls_ssl_context ssl;
	mbedtls_ssl_config conf;
	mbedtls_x509_crt cert;
	mbedtls_ctr_drbg_context ctr_drbg;
	mbedtls_entropy_context entropy;
	mbedtls_net_context server_fd;

	mbedtls_ssl_init(&ssl);
	mbedtls_ssl_config_init(&conf);
	mbedtls_x509_crt_init(&cert);
	mbedtls_ctr_drbg_init(&ctr_drbg);
	mbedtls_entropy_init( &entropy );
	mbedtls_net_init(&server_fd);

	void _Close_SMTP_SSL(void){  SMTP_SSL_Disconnect(&ssl,&entropy,&ctr_drbg,&conf,&cert,&server_fd);  /*GiveMutex(Semphr_sdram);*/  }

	while(1)
	{
		if(SMTP_SSL_Connect(&ssl, Const.emailSend[selNad], &entropy, &ctr_drbg, &conf, &cert, &server_fd))
		{
			SMTP_SSL_Disconnect(&ssl, &entropy, &ctr_drbg, &conf, &cert, &server_fd);
			if(++connectionError >= 5)
			{
				Dbg(1,"Connection error");
				vTaskDelete(NULL);
			}
			vTaskDelay(500);
		}
		else
		{
			if(SMTP_SSL_EHLO(&ssl,Const.emailSend[selNad].name)){
				Dbg(1,"Authorization error"); _Close_SMTP_SSL();
				vTaskDelete(NULL);
			}

			if(SMTP_SSL_AuthLogin(&ssl,&Const.emailSend[selNad])){
				Dbg(1,"Authorization error"); _Close_SMTP_SSL();
				vTaskDelete(NULL);
			}

			if(SMTP_SSL_MailFrom(&ssl,Const.emailSend[selNad].login)){
				Dbg(1,"Authorization error"); _Close_SMTP_SSL();
				vTaskDelete(NULL);
			}

			if(SMTP_SSL_RecipientTo(&ssl,Const.emailRecv)){
				Dbg(1,"Wrong recipient address"); _Close_SMTP_SSL();
				vTaskDelete(NULL);
			}

			if(SMTP_SSL_DATA(&ssl)){
				Dbg(1,"Connection error"); _Close_SMTP_SSL();
				vTaskDelete(NULL);
			}

			EMAIL_SSL_SendData(&ssl, &_param, &Const.emailSend[selNad], Const.emailRecv, mes);
			SMTP_SSL_QUIT(&ssl);
			mbedtls_ssl_close_notify(&ssl);

			Dbg(1,"Successfully"); _Close_SMTP_SSL();
			vTaskDelete(NULL);
		}

		vTaskDelay(50);
	}

}


/*----------------- THREADs -------------- */

void https_server_netconn_init(void)
{
	vTaskHandleServer = sys_thread_new("HTTPS", SSL_Server, NULL, 1024, -2);
}


static StaticTask_t vtaskSMTPS_Buffer;

void CreateTestEMAILTask(char* mes)
{
	//TakeMutex(Semphr_sdram, 3000);

	xTaskCreateStatic(vtaskSMTPS, "SMTPS", 8192, (void*) mes, (unsigned portBASE_TYPE ) 2, (StackType_t*)GETVAL_ptr(0x500000), &vtaskSMTPS_Buffer);
}


/* USER CODE END 4 */

/**
  * @}
  */

/**
  * @}
  */

