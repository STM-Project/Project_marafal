/*
 * debug.h
 *
 *  Created on: 02.04.2021
 *      Author: Elektronika RM
 */

#ifndef GENERAL_UTILITIES_DEBUG_H_
#define GENERAL_UTILITIES_DEBUG_H_

#include "stm32f7xx_hal.h"

typedef enum{
	font,
	bkg
}FONT_BKG_COLOR;

#define Clr_	"\x1B\x5B\x32\x4A"

#define Red_	"\x1b[31m"
#define Gre_	"\x1b[32m"
#define Yel_	"\x1b[33m"
#define Blu_	"\x1b[34m"
#define Mag_	"\x1b[35m"
#define Cya_	"\x1b[36m"

#define CoBl_	"\x1b[38;2;0;0;0m"
#define CoGr_	"\x1b[38;2;140;140;140m"

#define CoR_	"\x1b[38;2;255;130;110m"
#define CoG_	"\x1b[38;2;0;205;115m"
#define CoG2_	"\x1b[38;2;100;255;100m"
#define CoB_	"\x1b[38;2;50;190;255m"

#define BkR_	"\x1b[48;2;160;70;45m"
#define BkW_	"\x1b[48;2;255;255;255m"

#define Bol_	"\x1b[1m"
#define Ita_	"\x1b[4m"
#define _X		"\x1b[0m"

/* ---- User Colors Definitions -----*/
#define Co1_	"\x1b[38;2;255;255;255m"
#define Co2_	"\x1b[38;2;255;255;255m"
#define Co3_	"\x1b[38;2;255;255;255m"

#define Bk1_	"\x1b[48;2;255;255;255m"
#define Bk2_	"\x1b[48;2;255;255;255m"
#define Bk3_	"\x1b[48;2;255;255;255m"
/* ----------------------------------*/

#define Col_	"%s"
char* _Col(FONT_BKG_COLOR background, uint8_t red, uint8_t green, uint8_t blue);

void DEBUG_Init(void);
void Dbg(int on, char *txt);
void DbgMulti(int on, char *startTxt, char *txt, char *endTxt);
void DbgVar(int on, unsigned int buffLen, const char *fmt, ...);
void DbgVar2(int on, unsigned int buffLen, const char *fmt, ...);
void DEBUG_RxFullBuffService(void);
int DEBUG_RcvStr(char *txt);

#endif /* GENERAL_UTILITIES_DEBUG_H_ */
