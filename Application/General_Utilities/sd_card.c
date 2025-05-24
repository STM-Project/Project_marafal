/*
 * sd_card.c
 *
 *  Created on: 03.04.2021
 *      Author: Elektronika RM
 */

#include "errors_service.h"
#include "sd_card.h"
#include "fatfs.h"

#define MAX_OPEN_FILE   10

static FIL myFile[MAX_OPEN_FILE];

void SDCard_Init(void)
{
	FRESULT result=f_mount(&SDFatFS, (TCHAR const*) SDPath, 1);
	if(FR_OK!=result)
		ERROR_SDcardMount((int)result);
}

int SDCardFileOpen(uint8_t id, char *fileName, uint8_t accessMode)
{
	FRESULT result=f_open(&myFile[id], fileName, accessMode);
	if(FR_OK!=result)
		ERROR_SDcardOpen((int)result);
	return result;
}

int SDCardFileRead(uint8_t id, char *buff, uint32_t size)
{
	int _result;
	uint32_t bytesRead;

	FRESULT result=f_read(&myFile[id], buff, size, (void *)&bytesRead);
	if(FR_OK!=result)
	{
		ERROR_SDcardRead((int)result);
		_result=result*(-1);
	}
	else
		_result=bytesRead;
	return _result;
}

int SDCardFileWrite(uint8_t id, char *buff, uint32_t size)
{
	int _result;
	uint32_t bytesWritten;

	FRESULT result=f_write(&myFile[id], buff, size, (void *)&bytesWritten);
	if(FR_OK!=result)
	{
		ERROR_SDcardWrite((int)result);
		_result=result*(-1);
	}
	else
		_result=bytesWritten;
	return _result;
}

int SDCardFileClose(uint8_t id)
{
	FRESULT result=f_close(&myFile[id]);
	if(FR_OK!=result)
		ERROR_SDcardClose((int)result);
	return result;
}

int SDCardFilePosition(uint8_t id, uint32_t offset)  // Warunek: wsk >= 4 !!!
{//	while((wsk%4)!=0)  //wyr�wnanie do 4bajt�w dla odczytu SDRAM
	//		wsk--;
	FRESULT result=f_lseek(&myFile[id], offset);
	if(FR_OK!=result)
		ERROR_SDcardLseek((int)result);
	return result;
}

int SDCardOpenFileSize(uint8_t id)
{
	return f_size(&myFile[id]);			/* f_size(&MyFile[id]) only after f_open */
}

int SDCardFileInfo(char *name, uint32_t *fileSize)
{
	FILINFO plikInfo;
	FRESULT result=f_stat(name, &plikInfo);
	if(FR_OK!=result)
		ERROR_SDcardInfo((int)result);
	else
		*fileSize=plikInfo.fsize;
	return result;
}

int SDCard_FileReadClose(uint8_t id, char *fileName, char *pReadBuff, int maxSize)
{
	uint32_t len;
	int result;

	result=SDCardFileOpen(0,fileName,FA_READ);
	if(FR_OK!=result)
		return result*(-1);

	result=SDCardFileRead(0, pReadBuff, maxSize);
	if(0>result)
		return result;
	len=result;

	result=SDCardFileClose(0);
	if(FR_OK!=result)
		return result*(-1);

   return len;
}
