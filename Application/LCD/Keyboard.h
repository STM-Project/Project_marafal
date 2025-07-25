/*
 * Keyboard.h
 *
 *  Created on: Sep 8, 2024
 *      Author: maraf
 */

#ifndef LCD_KEYBOARD_H_
#define LCD_KEYBOARD_H_

#include "stm32f7xx_hal.h"
#include "LCD_BasicGraphics.h"
#include "LCD_fonts_images.h"
#include "LCD_Common.h"
#include "common.h"
#include "timer.h"

#define NO_TOUCH	0
#define KEYBOARD_TYPE(type,key)						FILE_NAME(keyboard)(type,key,0,0,0,0,0,0,0,NoTouch,NoTouch,0)
#define KEYBOARD_TYPE_PARAM(type,key,a,b,c,d,e)	FILE_NAME(keyboard)(type,key,0,0,a,b,c,d,e,NoTouch,NoTouch,0)

#define INIT_KEYBOARD_PARAM	figureShape shape, uint8_t bold, uint16_t x, uint16_t y, uint16_t widthKey, uint16_t heightKey, uint8_t interSpace, uint16_t forTouchIdx, uint16_t startTouchIdx, uint8_t eraseOther
#define ARG_KEYBOARD_PARAM	  shape, bold, x, y, widthKey, heightKey, interSpace, forTouchIdx, startTouchIdx, eraseOther

#define N			0x80000000

typedef enum{
	XYsizeTxt,
	StringTxt,
	Color1Txt,
	Color2Txt,
	StringTxtAll,
	Color1TxtAll,
	Color2TxtAll,
}TXT_PARAM_KEY;

typedef enum{
	KeysAutoSize,		/* Keys auto size to fontPress size*/
	KeysNotDel,
	KeysDel
}KEYBOARD_ANOTHER_PARAM;

structSize KEYBOARD_GetSize(void);
void KEYBOARD_KeyParamSet(TXT_PARAM_KEY param, ...);
void KEYBOARD_KeyAllParamSet(uint16_t sizeX,uint16_t sizeY, ...);
void KEYBOARD_KeyAllParamSet2(uint16_t sizeX,uint16_t sizeY, COLORS_DEFINITION color1,COLORS_DEFINITION color2, ...);
void KEYBOARD_KeyAllParamSet3(uint16_t sizeX,uint16_t sizeY, COLORS_DEFINITION color1,COLORS_DEFINITION color2, char** txt);

void KEYBOARD_SetGeneral(int vFontID,int vFontID_descr,int vColorDescr,int vFrameMainColor,int vFillMainColor,int vFrameColor,int vFillColor,int vFramePressColor,int vFillPressColor,int vBkColor);
int KEYBOARD_StartUp(int type, figureShape shape, uint8_t bold, uint16_t x, uint16_t y, uint16_t widthKey, uint16_t heightKey, uint8_t interSpace, uint16_t forTouchIdx, uint16_t startTouchIdx, uint8_t eraseOther);

void KEYBOARD_Buttons(int k, int selBlockPress, INIT_KEYBOARD_PARAM, int TOUCH_Release, int TOUCH_Action, char* txtDescr);
void KEYBOARD_Select(int k, int selBlockPress, INIT_KEYBOARD_PARAM, int TOUCH_Release, char* txtDescr, int value);
void KEYBOARD_ServiceSizeStyle(int k, int selBlockPress, INIT_KEYBOARD_PARAM, int TOUCH_Release, int TOUCH_Action, char* txtDescr, int value);
void KEYBOARD_ServiceSliderRGB(int k, int selBlockPress, INIT_KEYBOARD_PARAM, int TOUCH_Release, int TOUCH_Action, char* txtDescr, int *value, VOID_FUNCTION *pfunc);
void KEYBOARD_ServiceCircleSliderRGB(int k, int selBlockPress, INIT_KEYBOARD_PARAM, int TOUCH_Release, int TOUCH_Action, int touchTimer, char* txtDescr, int *value, VOID_FUNCTION *pfunc, TIMER_ID timID);
void KEYBOARD_Service_SliderButtonRGB(int k, int selBlockPress, INIT_KEYBOARD_PARAM, int TOUCH_Release, int TOUCH_Action, char* txtDescr, int *value, VOID_FUNCTION *pfunc);
void KEYBOARD_ServiceSizeRoll(int k, int selBlockPress, INIT_KEYBOARD_PARAM, int touchRelease, int nrRoll, char* txtDescr, uint32_t colorDescr, int frameNmbVis, int value);

int KEYBOARD_ServiceLenOffsWin(int k, int selBlockPress, INIT_KEYBOARD_PARAM, int touchRelease, int touchAction, int touchAction2, int touchTimer, char* txtDescr, char* txtDescr2, char* txtDescr3, char* txtDescr4, uint32_t colorDescr,FUNC_MAIN *pfunc,FUNC_MAIN_INIT, TIMER_ID timID);
void KEYBOARD__ServiceSetTxt(int k, int selBlockPress, INIT_KEYBOARD_PARAM, int touchRelease, int touchAction, int tBig,int tBack,int tEnter,uint32_t colorDescr);


#endif /* LCD_KEYBOARD_H_ */
