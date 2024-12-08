/*
 * ScreenLCDTask.c
 *
 *  Created on: Jun 2, 2024
 *      Author: mgrag
 */
#include "ScreenLcdTask.h"
#include "FreeRTOS.h"
#include "task.h"
#include "SCREEN_ReadPanel.h"

xTaskHandle vtask_ScreensSelectLCD_Handle;
xTaskHandle vtask_TEST_Handle;

void vtask_ScreensSelectLCD(void *pvParameters)
{
	while(1)
	{
		SCREEN_ReadPanel();
		vTaskDelay(20);
	}
}
double bufff[100];
void vtask_TEST(void *pvParameters)
{

	double dd=125.0145;


	while(1)
	{
		for(int i=0;i<100;++i){
			bufff[i] = dd*dd+(double)i*25.1236;
		}
		for(int i=0;i<100;++i){
			if(bufff[i]==125.125) bufff[i]=0.0;
		}
		vTaskDelay(20);
	}
}

void Create_ScreensSelectLCD_Task(void)
{
	xTaskCreate(vtask_ScreensSelectLCD, (char* )"vtask_ScreensSelectLCD", 1024, NULL, (unsigned portBASE_TYPE ) 1, &vtask_ScreensSelectLCD_Handle);
}

void Create_TEST_Task(void)
{
	xTaskCreate(vtask_TEST, (char* )"vtask_TEST", 4048, NULL, (unsigned portBASE_TYPE ) 7, &vtask_TEST_Handle);
}




/* ---------------- HTTP ---------------- */
#define WEBSERVER_THREAD_PRIO    ( tskIDLE_PRIORITY + 4 )

#include "api.h"
#include "netif.h"
#include "ethernetif.h"
#include "tcpip.h"
#include "dhcp.h"
#include "tcp.h"
#include "string.h"
#include "ff.h"
#include "sd_card.h"
#include "timer.h"

extern void Dbg(int on, char *txt);
extern char* GETVAL_ptr();

void http_server_serve(struct netconn *conn)
{
  struct netbuf *inbuf;
  err_t recv_err;
  char* buf;
  u16_t buflen;

  //conn->pcb.tcp->remote_ip.addr;

  /* Read the data from the port, blocking if nothing yet there.
   We assume the request (the part we care about) is in one netbuf */
  recv_err = netconn_recv(conn, &inbuf);

  if (recv_err == ERR_OK)  //http
  {
	  if (netconn_err(conn) == ERR_OK)
	  {
		  netbuf_data(inbuf, (void**)&buf, &buflen);

		  /* Is this an HTTP GET command? (only check the first 5 chars, since
      	there are other formats for GET, and we're keeping it very simple )*/
		  if(strstr(buf, "GET / "))   //http
		  {

//      	FILINFO fno;
//      	FIL graphicFile;
//      	f_open(&graphicFile, "0:image/blogo.dta", FA_READ);
//      	f_stat("0:image/blogo.dta",&fno);
//      	if( fno.fsize <= 768000)
//      			f_read(&graphicFile, bufferForLogoBigGraphic, fno.fsize, &bytesreadFromLogoBig);
//      	f_close(&graphicFile);




//				res = f_readdir(&dir, &fno); /* Read a directory item */
//				if (res != FR_OK || fno.fname[0] == 0)
//				{
//					strcpy(str+len-1,"]\0");
//					netconn_write(conn, data, (size_t )len, NETCONN_NOFLAG);
//					break; /* Break on error or end of dir */
//				}
//				if (fno.fname[0] == '.')
//					continue; /* Ignore dot entry */
//				if (AM_DIR != (fno.fattrib & AM_DIR))
//				{
//					dbl2stri(file_size, ((double) fno.fsize) / 1024, 2);
//					len += mini_snprintf(str + len,100,"[\"%s\",\"%02u-%02u-%02u %02u:%02u\",\"%s\"],",fno.fname,(fno.fdate >> 9) + 1980, fno.fdate >> 5 & 15, fno.fdate & 31, (fno.ftime >> 11),
//							((fno.ftime >> 5) & 63), file_size);
//				}




			  if(TakeMutex2(Semphr_pLcd,Semphr_cardSD,1000))
			  {
				  SDCardFileOpen(0,"aaa.htm",FA_READ);
				  int len = SDCardFileRead(0, GETVAL_ptr(0), 180000);
				  SDCardFileClose(0);
				  netconn_write(conn, GETVAL_ptr(0),len, NETCONN_NOCOPY);

				  GiveMutex2(Semphr_pLcd,Semphr_cardSD);
				  Dbg(1,"\r\nGET_GET_GET...");
			  }
			  else
				  netconn_write(conn, "1234567890",10, NETCONN_NOCOPY);


      }
      else
      {

      	 if(TakeMutex2(Semphr_pLcd,Semphr_cardSD,1000))
      	 {
      		 SDCardFileOpen(0,"aaa.htm",FA_READ);
      		 int len = SDCardFileRead(0, GETVAL_ptr(0), 180000);
      		 SDCardFileClose(0);
      		 netconn_write(conn, GETVAL_ptr(0),len, NETCONN_NOCOPY);

      		 GiveMutex2(Semphr_pLcd,Semphr_cardSD);
      		 Dbg(1,"\r\niii...");
      	 }
			  else
				  netconn_write(conn, "0000000000",10, NETCONN_NOCOPY);


      }
    }
  }
  /* Close the connection (server closes in HTTP) */
  netconn_close(conn);


  /* Delete the buffer (netconn_recv gives us ownership,
   so we have to make sure to deallocate the buffer) */
  netbuf_delete(inbuf);
}

static void http_server_netconn_thread(void *arg)
{
  struct netconn *conn, *newconn;
  err_t err, accept_err;

  /* Create a new TCP connection handle */
  conn = netconn_new(NETCONN_TCP);

  if (conn!= NULL)
  {
    /* Bind to port 80 (HTTP) with default IP address */
    err = netconn_bind(conn, NULL, 80);

    if (err == ERR_OK)
    {
      /* Put the connection into LISTEN state */
      netconn_listen(conn);

      while(1)
      {
        /* accept any icoming connection */
        accept_err = netconn_accept(conn, &newconn);
        if(accept_err == ERR_OK)
        {
          /* serve connection */
          http_server_serve(newconn);

          /* delete connection */
          netconn_delete(newconn);
        }
      }
    }
  }
}

void http_server_netconn_init(void)
{
	sys_thread_new("HTTP", http_server_netconn_thread, NULL, 1200, WEBSERVER_THREAD_PRIO);
}
/* -------------END HTTP ---------------- */
