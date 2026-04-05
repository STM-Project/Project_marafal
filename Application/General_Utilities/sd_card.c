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

	FRESULT result=f_write(&myFile[id], buff, size, (void *)&bytesWritten);			/* buffer file write, physical file write (buffer to SD) only after f_close() */
	/* f_sync(); */		/* physical file write */
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




char ASCII_to_hex (char kkk)
{
    if(kkk=='1') return 0x01;
    else if(kkk=='2') return 0x02;
	else if(kkk=='3') return 0x03;
	else if(kkk=='4') return 0x04;
	else if(kkk=='5') return 0x05;
	else if(kkk=='6') return 0x06;
	else if(kkk=='7') return 0x07;
	else if(kkk=='8') return 0x08;
	else if(kkk=='9') return 0x09;
	else if(kkk=='0') return 0x00;
	else if((kkk=='A')||(kkk=='a')) return 0x0A;
	else if((kkk=='B')||(kkk=='b')) return 0x0B;
	else if((kkk=='C')||(kkk=='c')) return 0x0C;
	else if((kkk=='D')||(kkk=='d')) return 0x0D;
	else if((kkk=='E')||(kkk=='e')) return 0x0E;
	else if((kkk=='F')||(kkk=='f')) return 0x0F;
	return 0;
}

int Plik_bin_Divide(char *Buff)
{
	uint32_t len;
	int result;


   char KOD[10];

	result=SDCardFileOpen(MAX_OPEN_FILE-1,"kod.txt",FA_READ);
	if(FR_OK!=result)
		return result*(-1);
	result=SDCardFileRead(MAX_OPEN_FILE-1, Buff, 16);
	if(0>result)
		return result;
	SDCardFileClose(MAX_OPEN_FILE-1);

   KOD[0] = (0xf0&(ASCII_to_hex(Buff[0])<<4)) | (0x0f&(ASCII_to_hex(Buff[1])));
   KOD[1] = (0xf0&(ASCII_to_hex(Buff[2])<<4)) | (0x0f&(ASCII_to_hex(Buff[3])));
   KOD[2] = (0xf0&(ASCII_to_hex(Buff[4])<<4)) | (0x0f&(ASCII_to_hex(Buff[5])));
   KOD[3] = (0xf0&(ASCII_to_hex(Buff[6])<<4)) | (0x0f&(ASCII_to_hex(Buff[7])));
   KOD[4] = (0xf0&(ASCII_to_hex(Buff[8])<<4)) | (0x0f&(ASCII_to_hex(Buff[9])));
   KOD[5] = (0xf0&(ASCII_to_hex(Buff[10])<<4)) | (0x0f&(ASCII_to_hex(Buff[11])));
   KOD[6] = (0xf0&(ASCII_to_hex(Buff[12])<<4)) | (0x0f&(ASCII_to_hex(Buff[13])));
   KOD[7] = (0xf0&(ASCII_to_hex(Buff[14])<<4)) | (0x0f&(ASCII_to_hex(Buff[15])));

/*
   KOD[0]=0x23;   //1507197966666600
   KOD[1]=0xAB;
   KOD[2]=0x09;
   KOD[3]=0x14;
   KOD[4]=0x00;
   KOD[5]=0xFF;
   KOD[6]=0xF5;
   KOD[7]=0xCD;
*/
     int i,j;


	result=SDCardFileOpen(MAX_OPEN_FILE-1,"plik.bin",FA_READ);
	if(FR_OK!=result)
		return result*(-1);

	result=SDCardFileOpen(MAX_OPEN_FILE-2,"nowy.bin",FA_CREATE_ALWAYS|FA_WRITE);
	if(FR_OK!=result)
		return result*(-1);


	 i=0;
	 wedwefrf:
	 HAL_Delay(10);


	result=SDCardFileRead(MAX_OPEN_FILE-1, &Buff[10000], 2048);
	if(0>result)
		return result;


	 for(j=0;j<2048;j++) Buff[j]=Buff[10000+j];

	 if(result==2048)
	 {
		 if(i==0){
		     i=1;

			 //for(j=0;j<8;j++) Buff[12000+j]=KOD[j];

			 Buff[11000]=KOD[0];
			 Buff[11011]=KOD[1];
			 Buff[11139]=KOD[2];
			 Buff[11987]=KOD[3];
			 Buff[11456]=KOD[4];
			 Buff[11238]=KOD[5];
			 Buff[11199]=KOD[6];
			 Buff[11691]=KOD[7];


			result=SDCardFileWrite(MAX_OPEN_FILE-2, &Buff[10000], 2048);
			if(0>result)
				return result;

			result=SDCardFileWrite(MAX_OPEN_FILE-2, Buff, 2048);
			if(0>result)
				return result;

	     }
		 else{
				result=SDCardFileWrite(MAX_OPEN_FILE-2, Buff, 2048);
				if(0>result)
					return result;
		 }
	     goto wedwefrf;
	 }
	 else
	 {
			result=SDCardFileWrite(MAX_OPEN_FILE-2, Buff, result);
			if(0>result)
				return result;

	 }

	 SDCardFileClose(MAX_OPEN_FILE-1);
	 SDCardFileClose(MAX_OPEN_FILE-2);

	 return 0;
}
