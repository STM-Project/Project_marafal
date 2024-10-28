/*
 * SCREEN_ReadPanel.c
 *
 *  Created on: 14.04.2021
 *      Author: RafalMar
 */

#include "SCREEN_ReadPanel.h"
#include "FreeRTOS.h"
#include "task.h"
#include "LCD_BasicGaphics.h"
#include "LCD_fonts_images.h"
#include "string_oper.h"
#include "LCD_Common.h"
#include "tim.h"
#include "timer.h"
#include "mini_printf.h"
#include <string.h>
#include "debug.h"
#include "FreeRTOS.h"
#include "common.h"
#include "sd_card.h"
#include "ff.h"
#include "lang.h"
#include "cpu_utils.h"
#include "SCREEN_CalibrationLCD.h"
#include "touch.h"
#include "SCREEN_FontsLCD.h"


#define FLOAT2STR(val)	Float2Str(val,' ',4,Sign_plusMinus,1)
#define INT2STR(val)		  Int2Str(val,'0',3,Sign_none)
#define INT2STR_TIME(val) Int2Str(val,' ',6,Sign_none)
#define ONEBIT(val)	     Int2Str(val,' ',0,Sign_none)

int argNmb = 0;
char **argVal = NULL;

int SCREEN_number=0, SCREEN_number_prev=-1;  //LOAD IMAGE !!!!!


StructTxtPxlLen lenStr;
int startScreen=0;

void LCD_ArrowTxt(uint32_t posBuff,uint32_t bkpSizeX,uint32_t bkpSizeY, uint32_t x,uint32_t y, uint32_t width,uint32_t height, uint32_t frameColor, uint32_t fillColor, uint32_t bkpColor, DIRECTIONS direct, int fontId, char *txt, uint32_t txtColor)
{
	int boldLine = 0;
	int heightTraingCoeff = 3;
	int spaceArrowTxt = 5;
	int spaceBetweenArrows = 4;

	if(height < LCD_GetFontHeight(fontId))
		height = LCD_GetFontHeight(fontId);

	int heightArrow = height - height/2;
	int xTxt,lineLen,temp=0;
	StructTxtPxlLen len;

	if(heightArrow > height)
		heightArrow = height;

	switch((int)direct)
	{
	case outside:
	case inside:
		len = LCD_StrDependOnColorsWindow(0,bkpSizeX,bkpSizeY,fontId, xTxt=MIDDLE(x,width,LCD_GetWholeStrPxlWidth(fontId,txt,0,NoConstWidth)), MIDDLE(y,height,LCD_GetFontHeight(fontId)),txt, fullHight, 0, bkpColor, txtColor,250, NoConstWidth);
		LCD_Arrow(0,bkpSizeX,bkpSizeY, x,										MIDDLE(y,height,heightArrow), SetLineBold2Width(xTxt-x-spaceArrowTxt,boldLine), SetTriangHeightCoeff2Height(heightArrow,heightTraingCoeff), frameColor,fillColor,bkpColor,  CONDITION(outside==direct,Left,Right));
		LCD_Arrow(0,bkpSizeX,bkpSizeY, xTxt+len.inPixel+spaceArrowTxt, MIDDLE(y,height,heightArrow), SetLineBold2Width(xTxt-x-spaceArrowTxt,boldLine), SetTriangHeightCoeff2Height(heightArrow,heightTraingCoeff), frameColor,fillColor,bkpColor, CONDITION(outside==direct,Right,Left));
		break;

	case RightRight:
		len = LCD_StrDependOnColorsWindow(0,bkpSizeX,bkpSizeY,fontId, x, MIDDLE(y,height,LCD_GetFontHeight(fontId)),txt, fullHight, 0, bkpColor, txtColor,250, NoConstWidth);
		lineLen = (width-(len.inPixel+spaceArrowTxt))/2;	if(lineLen>spaceBetweenArrows){ lineLen-=spaceBetweenArrows; temp=spaceBetweenArrows; }
		LCD_Arrow(0,bkpSizeX,bkpSizeY, xTxt=x+len.inPixel+spaceArrowTxt, MIDDLE(y,height,heightArrow), SetLineBold2Width(lineLen,boldLine), SetTriangHeightCoeff2Height(heightArrow,heightTraingCoeff), frameColor,fillColor,bkpColor,Right);
		LCD_Arrow(0,bkpSizeX,bkpSizeY, xTxt+=lineLen+temp,					  MIDDLE(y,height,heightArrow), SetLineBold2Width(lineLen,boldLine), SetTriangHeightCoeff2Height(heightArrow,heightTraingCoeff), frameColor,fillColor,bkpColor,Right);
		break;

	case LeftLeft:
		lineLen = (width-(LCD_GetWholeStrPxlWidth(fontId,txt,0,NoConstWidth)+spaceArrowTxt))/2;	if(lineLen>spaceBetweenArrows){ lineLen-=spaceBetweenArrows; temp=spaceBetweenArrows; }
		LCD_Arrow(0,bkpSizeX,bkpSizeY, xTxt=x,			 		MIDDLE(y,height,heightArrow), SetLineBold2Width(lineLen,boldLine), SetTriangHeightCoeff2Height(heightArrow,heightTraingCoeff), frameColor,fillColor,bkpColor,Left);
		LCD_Arrow(0,bkpSizeX,bkpSizeY, xTxt+=lineLen+temp, MIDDLE(y,height,heightArrow), SetLineBold2Width(lineLen,boldLine), SetTriangHeightCoeff2Height(heightArrow,heightTraingCoeff), frameColor,fillColor,bkpColor,Left);
		len = LCD_StrDependOnColorsWindow(0,bkpSizeX,bkpSizeY,fontId, xTxt+=lineLen+spaceArrowTxt, MIDDLE(y,height,LCD_GetFontHeight(fontId)),txt, fullHight, 0, bkpColor, txtColor,250, NoConstWidth);
		break;
	}
}

int SCREEN_ConfigTouchForStrVar(uint16_t ID_touch, uint16_t idx_touch, uint8_t param_touch, int idVar, int nrTouchIdx, StructTxtPxlLen lenStr){
 	LCD_SetStrVar_idxTouch(idVar,nrTouchIdx,idx_touch);
	touchTemp[0].x = LCD_GetStrVar_x(idVar);
	touchTemp[0].y = LCD_GetStrVar_y(idVar);
 	touchTemp[1].x = touchTemp[0].x + lenStr.inPixel;
 	touchTemp[1].y = touchTemp[0].y + lenStr.height;
 	return LCD_TOUCH_Set(ID_touch, idx_touch, param_touch);
}

int SCREEN_ConfigTouchForStrVar_2(uint16_t ID_touch, uint16_t idx_touch, uint8_t param_touch, int idVar, int nrTouchIdx, StructFieldPos field){
 	LCD_SetStrVar_idxTouch(idVar,nrTouchIdx,idx_touch);
	touchTemp[0].x = field.x;
	touchTemp[0].y = field.y;
 	touchTemp[1].x = touchTemp[0].x + field.width;
 	touchTemp[1].y = touchTemp[0].y + field.height;
 	return LCD_TOUCH_Set(ID_touch, idx_touch, param_touch);
}

int SCREEN_SetTouchForNewEndPos(int idVar, int nrTouchIdx, StructTxtPxlLen lenStr){
	return LCD_TOUCH_SetNewPos( LCD_GetStrVar_idxTouch(idVar,nrTouchIdx), LCD_GetStrVar_x(idVar), LCD_GetStrVar_y(idVar), lenStr.inPixel, lenStr.height );
}


void NOWY_1(void)  //dac mozliwosc zablokowania Dbg definem!!!
{
	int bkColor=MYGRAY;

	SCREEN_ResetAllParameters();
	LCD_Clear(bkColor);

	LCD_LoadFont_DarkgrayWhite(FONT_10, Arial, fontID_6);
	LCD_LoadFont_DarkgrayGreen(FONT_10, Arial, fontID_7);


	lenStr=LCD_StrRot(Rotate_0,fontID_6,        LCD_Xpos(lenStr,SetPos,5),LCD_Ypos(lenStr,SetPos,5), "123ABCabc", fullHight, 0,DARKRED,1,1);
	LCD_Frame(0,LCD_GetXSize(),LCD_GetYSize(),LCD_Xpos(lenStr,GetPos,-1),LCD_Ypos(lenStr,GetPos,-1), lenStr.inPixel+2,lenStr.height+2,WHITE,0,0);

	lenStr=LCD_StrChangeColorRot(Rotate_0,fontID_7,       LCD_Xpos(lenStr,IncPos,5),LCD_Ypos(lenStr,GetPos,0), "123ABCabc", fullHight, 0,GRAY,MYBLUE,254,1);
	LCD_Frame(0,LCD_GetXSize(),LCD_GetYSize(),LCD_Xpos(lenStr,GetPos,-1),LCD_Ypos(lenStr,GetPos,-1), lenStr.inPixel+2,lenStr.height+2,WHITE,0,0);

	lenStr=LCD_StrRotWin(Rotate_0,120,fontID_7,       LCD_Xpos(lenStr,IncPos,5),LCD_Ypos(lenStr,GetPos,0), "Rafa� Markielowski Agnieszaka Markielowska", fullHight, 0,bkColor,1,ConstWidth,0);
	LCD_Frame(0,LCD_GetXSize(),LCD_GetYSize(),LCD_Xpos(lenStr,GetPos,-1),LCD_Ypos(lenStr,GetPos,-1), lenStr.inPixel+2,lenStr.height+2,WHITE,0,0);

	lenStr=LCD_StrChangeColorRotWin(Rotate_0,120,fontID_7,       LCD_Xpos(lenStr,IncPos,5),LCD_Ypos(lenStr,GetPos,0), "Rafa� Markielowski Agnieszaka Markielowska", fullHight, 0,DARKRED,MAGENTA,244,ConstWidth,0);
	LCD_Frame(0,LCD_GetXSize(),LCD_GetYSize(),LCD_Xpos(lenStr,GetPos,-1),LCD_Ypos(lenStr,GetPos,-1), lenStr.inPixel+2,lenStr.height+2,WHITE,0,0);



	lenStr=LCD_StrRot(Rotate_90,fontID_6,     LCD_Xpos(lenStr,SetPos,5),LCD_Ypos(lenStr,IncPos,10), "123ABCabc", fullHight, 0,DARKRED,1,1);
	LCD_Frame(0,LCD_GetXSize(),LCD_GetYSize(),LCD_Xpos(lenStr,GetPos,-1),LCD_Ypos(lenStr,GetPos,-1), lenStr.inPixel+2,lenStr.height+2,WHITE,0,0);

	lenStr=LCD_StrChangeColorRot(Rotate_90,fontID_7, LCD_Xpos(lenStr,IncPos,5),LCD_Ypos(lenStr,GetPos,0), "123ABCabc", fullHight, 0,GRAY,MYBLUE,254,1);
	LCD_Frame(0,LCD_GetXSize(),LCD_GetYSize(),		 LCD_Xpos(lenStr,GetPos,-1),LCD_Ypos(lenStr,GetPos,-1), lenStr.inPixel+2,lenStr.height+2,WHITE,0,0);

	lenStr=LCD_StrRotWin(Rotate_90,70,fontID_7,       LCD_Xpos(lenStr,IncPos,5),LCD_Ypos(lenStr,GetPos,0), "Rafa� Markielowski Agnieszaka Markielowska", fullHight, 0,bkColor,1,1,0);
	LCD_Frame(0,LCD_GetXSize(),LCD_GetYSize(),		  LCD_Xpos(lenStr,GetPos,-1),LCD_Ypos(lenStr,GetPos,-1), lenStr.inPixel+2,lenStr.height+2,WHITE,0,0);

	lenStr=LCD_StrChangeColorRotWin(Rotate_90,70,fontID_7,LCD_Xpos(lenStr,IncPos,5),LCD_Ypos(lenStr,GetPos,0), "Rafa� Markielowski Agnieszaka Markielowska", fullHight, 0,DARKRED,MAGENTA,244,1,0);
	LCD_Frame(0,LCD_GetXSize(),LCD_GetYSize(),				LCD_Xpos(lenStr,GetPos,-1),LCD_Ypos(lenStr,GetPos,-1), lenStr.inPixel+2,lenStr.height+2,WHITE,0,0);



	lenStr=LCD_StrRot(Rotate_180,fontID_6,     LCD_Xpos(lenStr,SetPos,5),LCD_Ypos(lenStr,IncPos,10), "123ABCabc", fullHight, 0,DARKRED,1,1);
	LCD_Frame(0,LCD_GetXSize(),LCD_GetYSize(),LCD_Xpos(lenStr,GetPos,-1),LCD_Ypos(lenStr,GetPos,-1), lenStr.inPixel+2,lenStr.height+2,WHITE,0,0);

	lenStr=LCD_StrChangeColorRot(Rotate_180,fontID_7, LCD_Xpos(lenStr,IncPos,5),LCD_Ypos(lenStr,GetPos,0), "123ABCabc", fullHight, 0,GRAY,MYBLUE,254,1);
	LCD_Frame(0,LCD_GetXSize(),LCD_GetYSize(),		 LCD_Xpos(lenStr,GetPos,-1),LCD_Ypos(lenStr,GetPos,-1), lenStr.inPixel+2,lenStr.height+2,WHITE,0,0);

	lenStr=LCD_StrRotWin(Rotate_180,80,fontID_7,       LCD_Xpos(lenStr,IncPos,5),LCD_Ypos(lenStr,GetPos,0), "Rafa� Markielowski Agnieszaka Markielowska", fullHight, 0,bkColor,1,1,0);
	LCD_Frame(0,LCD_GetXSize(),LCD_GetYSize(),		  LCD_Xpos(lenStr,GetPos,-1),LCD_Ypos(lenStr,GetPos,-1), lenStr.inPixel+2,lenStr.height+2,WHITE,0,0);

	lenStr=LCD_StrChangeColorRotWin(Rotate_180,80,fontID_7,LCD_Xpos(lenStr,IncPos,5),LCD_Ypos(lenStr,GetPos,0), "Raf", fullHight, 0,DARKRED,MAGENTA,244,1,0);
	LCD_Frame(0,LCD_GetXSize(),LCD_GetYSize(),				LCD_Xpos(lenStr,GetPos,-1),LCD_Ypos(lenStr,GetPos,-1), lenStr.inPixel+2,lenStr.height+2,WHITE,0,0);


	LCD_Show();
	DisplayFontsStructState();

}

int test[6]={0,0,0,0,0,0};
void NOWY_2(void)  //dac mozliwosc zablokowania Dbg definem!!!
{
	int bkColor=MYGRAY;

	SCREEN_ResetAllParameters();
	LCD_Clear(bkColor);

	LCD_LoadFont_DarkgrayWhite(FONT_14, Arial, fontID_6);
	LCD_LoadFont_DarkgrayGreen(FONT_14, Arial, fontID_7);


	lenStr=LCD_StrRotVar(fontVar_1,Rotate_0,fontID_6,  LCD_Xpos(lenStr,SetPos,5),LCD_Ypos(lenStr,SetPos,5), INT2STR(test[0]), halfHight, 0,DARKRED,1,ConstWidth,bkColor);
	LCD_Frame(0,LCD_GetXSize(),LCD_GetYSize(),			LCD_Xpos(lenStr,GetPos,-1),LCD_Ypos(lenStr,GetPos,-1), lenStr.inPixel+2,lenStr.height+2,WHITE,0,0);

	lenStr=LCD_StrChangeColorRotVar(fontVar_2,Rotate_0,fontID_7, LCD_Xpos(lenStr,IncPos,5),LCD_Ypos(lenStr,GetPos,0), INT2STR(test[1]), halfHight, 0,GRAY,MYBLUE,249,ConstWidth,bkColor);
	LCD_Frame(0,LCD_GetXSize(),LCD_GetYSize(),						LCD_Xpos(lenStr,GetPos,-1),LCD_Ypos(lenStr,GetPos,-1), lenStr.inPixel+2,lenStr.height+2,WHITE,0,0);


	lenStr=LCD_StrRotVar(fontVar_3,Rotate_90,fontID_6, LCD_Xpos(lenStr,SetPos,5),LCD_Ypos(lenStr,IncPos,10), INT2STR(test[2]), halfHight, 0,DARKRED,1,ConstWidth,bkColor);
	LCD_Frame(0,LCD_GetXSize(),LCD_GetYSize(),			LCD_Xpos(lenStr,GetPos,-1),LCD_Ypos(lenStr,GetPos,-1), lenStr.inPixel+2,lenStr.height+2,WHITE,0,0);

	lenStr=LCD_StrChangeColorRotVar(fontVar_4,Rotate_90,fontID_7, LCD_Xpos(lenStr,IncPos,5),LCD_Ypos(lenStr,GetPos,0), INT2STR(test[3]), halfHight, 0,GRAY,MYBLUE,249,ConstWidth,bkColor);
	LCD_Frame(0,LCD_GetXSize(),LCD_GetYSize(),						 LCD_Xpos(lenStr,GetPos,-1),LCD_Ypos(lenStr,GetPos,-1), lenStr.inPixel+2,lenStr.height+2,WHITE,0,0);


	lenStr=LCD_StrRotVar(fontVar_5,Rotate_180,fontID_6,  LCD_Xpos(lenStr,SetPos,5),LCD_Ypos(lenStr,IncPos,10), INT2STR(test[4]), halfHight, 0,DARKRED,1,ConstWidth,bkColor);
	LCD_Frame(0,LCD_GetXSize(),LCD_GetYSize(),			LCD_Xpos(lenStr,GetPos,-1),LCD_Ypos(lenStr,GetPos,-1), lenStr.inPixel+2,lenStr.height+2,WHITE,0,0);

	lenStr=LCD_StrChangeColorRotVar(fontVar_6,Rotate_180,fontID_7, LCD_Xpos(lenStr,IncPos,5),LCD_Ypos(lenStr,GetPos,0), INT2STR(test[5]), halfHight, 0,GRAY,MYBLUE,249,ConstWidth,bkColor);
	LCD_Frame(0,LCD_GetXSize(),LCD_GetYSize(),						 LCD_Xpos(lenStr,GetPos,-1),LCD_Ypos(lenStr,GetPos,-1), lenStr.inPixel+2,lenStr.height+2,WHITE,0,0);


	lenStr=LCD_StrRotVar				 (fontVar_7,Rotate_180, fontID_6, LCD_Xpos(lenStr,SetPos,100),LCD_Ypos(lenStr,SetPos,50), "1234567890", halfHight, 0,DARKRED,5,NoConstWidth,bkColor);
	lenStr=LCD_StrChangeColorRotVar(fontVar_8,Rotate_90, fontID_7, LCD_Xpos(lenStr,IncPos,50),  LCD_Ypos(lenStr,GetPos,0),   "1234567890", halfHight, 0,GRAY,MYBLUE,249,ConstWidth,bkColor);

//	StartMeasureTime_us();
//	LCD_RoundRectangle(0,LCD_GetXSize(),LCD_GetYSize(),230,0, 100,45,LIGHTGRAY,MYGRAY,MYGRAY);
//	StopMeasureTime_us("\r\nLCD_DrawRoundRectangle:");
//
//	StartMeasureTime_us();
//	LCD_RoundFrame(0,LCD_GetXSize(),LCD_GetYSize(),230,50, 100,45,LIGHTGRAY,MYGRAY,MYGRAY);
//	StopMeasureTime_us("\r\nLCD_DrawRoundFrame:");
//
//	LCD_BoldRoundRectangle(0,LCD_GetXSize(),LCD_GetYSize(),230,100, 100,45,LIGHTGRAY,MYGRAY,MYGRAY);
//	LCD_BoldRoundFrame(0,LCD_GetXSize(),LCD_GetYSize(),230,150, 100,45,LIGHTGRAY,MYGRAY,MYGRAY);
//
//	StartMeasureTime_us();
//	LCD_BoldRoundRectangle_AA(0,LCD_GetXSize(),LCD_GetYSize(),350,0, 100,45,LIGHTGRAY,MYGRAY,MYGRAY);
//	StopMeasureTime_us("\r\nLCD_DrawBoldRoundRectangle_AA:");
//
//	StartMeasureTime_us();
//	LCD_BoldRoundFrame_AA(0,LCD_GetXSize(),LCD_GetYSize(),350,50, 100,45,LIGHTGRAY,MYGRAY,MYGRAY);
//	StopMeasureTime_us("\r\nLCD_DrawBoldRoundFrame_AA:");
//
//	StartMeasureTime_us();
//	LCD_RoundRectangle_AA(0,LCD_GetXSize(),LCD_GetYSize(),350,100, 100,45,LIGHTGRAY,MYGRAY,MYGRAY);
//	StopMeasureTime_us("\r\nLCD_DrawRoundRectangle_AA:");


	int frameColor=LIGHTGRAY;
	//int fillColor=GRAY;

	Set_AACoeff_RoundFrameRectangle(0.55, 0.73);
	//Set_AACoeff_RoundFrameRectangle(0, 0);
	//Set_AACoeff_RoundFrameRectangle(1, 1);


	StartMeasureTime_us();
	LCD_Shape(240,0,LCD_Frame,100,45,frameColor,bkColor,bkColor);
	StopMeasureTime_us("\r\nAAAAAAAAAAAA1:");
	StartMeasureTime_us();
	LCD_Shape(240,50,LCD_BoldFrame,100,45,frameColor,bkColor,bkColor);
	StopMeasureTime_us("\r\nAAAAAAAAAAAA1:");
	StartMeasureTime_us();
	LCD_Shape(240,100,LCD_BoldFrame,100,45,SetBold2Color(frameColor,4),bkColor,bkColor);
	StopMeasureTime_us("\r\nAAAAAAAAAAAA1:");
	StartMeasureTime_us();
	LCD_Shape(240,150,LCD_RoundFrame,100,45,frameColor,bkColor,bkColor);
	StopMeasureTime_us("\r\nAAAAAAAAAAAA1:");
	StartMeasureTime_us();
	LCD_Shape(240,200,LCD_BoldRoundFrame,100,45,SetBold2Color(frameColor,3),bkColor,bkColor);
	StopMeasureTime_us("\r\nAAAAAAAAAAAA3:");


	StartMeasureTime_us();
	LCD_Shape(350,0,LCD_Rectangle,100,45,frameColor,bkColor,bkColor);
	StopMeasureTime_us("\r\nAAAAAAAAAAAA1:");
	StartMeasureTime_us();
	LCD_Shape(350,50,LCD_BoldRectangle,100,45,frameColor,bkColor,bkColor);
	StopMeasureTime_us("\r\nAAAAAAAAAAAA1:");
	StartMeasureTime_us();
	LCD_Shape(350,100,LCD_BoldRectangle,100,45,SetBold2Color(GRAY,4),DARKYELLOW,bkColor);
	StopMeasureTime_us("\r\nAAAAAAAAAAAA1:");
	StartMeasureTime_us();
	LCD_Shape(350,150,LCD_RoundRectangle,100,45,GRAY,DARKRED,bkColor);  //zrobic jezsli Round to _AA !!!!
	StopMeasureTime_us("\r\nAAAAAAAAAAAA1:");
	StartMeasureTime_us();
	LCD_Shape(350,200,LCD_BoldRoundRectangle,100,45,SetBold2Color(frameColor,3),0x111111,bkColor);
	StopMeasureTime_us("\r\nAAAAAAAAAAAA3:");


 //Frame Indirect nie dziala !!!!!!!!!!!!!!!!!



//RotWin jako select rozwijane zanaczyc text w select !!!!!!!!!!!!!!!!!!!!!!!!



	LCD_Show();
	DisplayFontsStructState();


	LCD_ShapeIndirect(100,200,LCD_BoldRoundRectangle,100,45, SetBold2Color(frameColor,5),DARKGREEN,bkColor);

}

void NOWY_3(void)  //dac mozliwosc zablokowania Dbg definem!!!
{
	SCREEN_ResetAllParameters();
	LCD_Clear(MYGRAY);
	LCD_LoadFont_DarkgrayWhite(FONT_10, Arial, fontID_6);
	LCD_LoadFont_DarkgrayGreen(FONT_10, Arial, fontID_7);



	// w winHeight dac automatic zeby dopasowac wysokosc do textu
	//przed LCD_StrMovV dac obrys ramki np


// ZMIENIC lOGIKE width i heght gdy tot80 lub 90 !!!

	lenStr=LCD_StrMovH(fontVar_1,Rotate_0,0,109,   fontID_6,  100,0,"AAA11111111111222222222222222 33333333333333333333A",fullHight,0,DARKBLUE,1,1);
	lenStr=LCD_StrMovH(fontVar_2,Rotate_90,0,109,   fontID_6, 0,0,"BBB1111111111222222222222222 33333333333333333333B",fullHight,0,DARKBLUE,1,1);
	lenStr=LCD_StrMovH(fontVar_3,Rotate_180,0,109,   fontID_6, 50,0,"CCCC11111111111222222222222222 33333333333333333333C",fullHight,0,DARKBLUE,1,1);

	lenStr=LCD_StrChangeColorMovH(fontVar_4,Rotate_0,0,80,fontID_7, 100,50,"Zeszyty i ksi��ki to podr�czniki do szko�y dla ��o�A_XY",fullHight,0,DARKYELLOW,DARKRED,254,0);
	lenStr=LCD_StrChangeColorMovH(fontVar_5,Rotate_90,0,80,fontID_7, 400,0,"Zeszyty i ksi��ki to podr�czniki do szko�y dla ��o�A_XY",fullHight,0,DARKYELLOW,DARKRED,254,0);
	lenStr=LCD_StrChangeColorMovH(fontVar_6,Rotate_180,0,80,fontID_7, 435,0,"Zeszyty i ksi��ki to podr�czniki do szko�y dla ��o�A_XY",fullHight,0,DARKYELLOW,DARKRED,254,0);


	lenStr=LCD_StrChangeColorMovV(fontVar_7,Rotate_0,0,100,50,fontID_7,100,100,"Rafa� Markielowski jest ww p omieszczeniu gospodarczym lubi krasnale www doku na drzewie takie jego bojowe zadanie  SEX _XY",fullHight,0,LIGHTGRAY,DARKBLUE,249,0);
	lenStr=LCD_StrMovV           (fontVar_8,Rotate_0,0,100,50,fontID_6,210,100,"1234567890 abcdefghijklnn opqrstuvw ABCDEFGHIJKLMNOPQRTSUVWXYZ iiiiiiiiiiiijjjjjjjjjjjjSSSSSSSSSSEEEEEEEEEERRRRRRRRRA _XY",fullHight,0,MYGRAY,1,0);

	lenStr=LCD_StrChangeColorMovV(fontVar_9,Rotate_90,0,100,50,fontID_7,0,160,"Rafa� Markielowski jest ww p omieszczeniu gospodarczym lubi krasnale www doku na drzewie takie jego bojowe zadanie  SEX _XY",fullHight,0,LIGHTGRAY,DARKBLUE,249,0);
	lenStr=LCD_StrMovV           (fontVar_10,Rotate_90,0,100,50,fontID_6,110,160,"1234567890 abcdefghijklnn opqrstuvw ABCDEFGHIJKLMNOPQRTSUVWXYZ iiiiiiiiiiiijjjjjjjjjjjjSSSSSSSSSSEEEEEEEEEERRRRRRRRRA _XY",fullHight,0,MYGRAY,1,0);

	lenStr=LCD_StrChangeColorMovV(fontVar_11,Rotate_180,0,100,50,fontID_7,220,160,"Rafa� Markielowski jest ww p omieszczeniu gospodarczym lubi krasnale www doku na drzewie takie jego bojowe zadanie  SEX _XY",fullHight,0,LIGHTGRAY,DARKBLUE,249,0);
	lenStr=LCD_StrMovV           (fontVar_12,Rotate_180,0,100,50,fontID_6,330,160,"1234567890 abcdefghijklnn opqrstuvw ABCDEFGHIJKLMNOPQRTSUVWXYZ iiiiiiiiiiiijjjjjjjjjjjjSSSSSSSSSSEEEEEEEEEERRRRRRRRRA _XY",fullHight,0,MYGRAY,1,0);




	// wyzej funkcje pobieraja bufor w ktorym sa przewijane texty zastanowic sie ??




	LCD_Show();
	DisplayFontsStructState();
	LCD_DisplayStrMovBuffState();

}

/* ########### --- SCREEN_Test_Circle --- ############ */

#define MAX_WIDTH_CIRCLE 	(VALPERC(LCD_GetYSize(),96))
#define MIN_WIDTH_CIRCLE 	(20)

typedef struct{
	uint8_t startFlag;
	uint16_t width;
	uint8_t bk[3];
	uint8_t frame[3];
	uint8_t fill[3];
	float ratioBk;
	float ratioFill;
	uint32_t speed;
	uint32_t speedSum;
	uint8_t bold;
	uint8_t halfCircle;
	uint16_t deg[10];
	uint32_t degColor[10];
} CIRCLE_PARAM;
static CIRCLE_PARAM Circle ={0};

void Measure_Start(){
	StartMeasureTime_us();
}
void Measure_Stop(){
	uint32_t aaa = StopMeasureTime_us("");
	Circle.speedSum += aaa;
	Circle.speed = MAXVAL2(Circle.speed,aaa);
}

int START_SCREEN_Test_Circle(void){
	if(SCREEN_number_prev != SCREEN_number)
		return 1;
	return 0;
}

static void SCREEN_Test_Circle(void)
{
   uint32_t _BkColor		(void){ return RGB2INT( Circle.bk[0],	  Circle.bk[1],    Circle.bk[2]    ); }
   uint32_t _FillColor	(void){ return RGB2INT( Circle.fill[0],  Circle.fill[1],  Circle.fill[2]  ); }
   uint32_t _FrameColor	(void){ return RGB2INT( Circle.frame[0], Circle.frame[1], Circle.frame[2] ); }
/*
	void __Show_FrameAndCircle_Indirect(uint16_t x, uint16_t y, uint16_t width, uint8_t bold){
		int widthCalculated=LCD_CalculateCircleWidth(width);
		LCD_ClearPartScreen(3333,widthCalculated,widthCalculated,RGB2INT(Circle.bk[0],Circle.bk[1],Circle.bk[2]));
		LCD_SetCircleAA(Circle.ratioBk,Circle.ratioFill);
		LCD_ShapeWindow	         (LCD_Circle,3333,widthCalculated,widthCalculated, 0,0, width,          width,      		SetBold2Color(_FrameColor(),bold), _FillColor(),_BkColor());
		LCD_ShapeWindowIndirect(x,y,LCD_Frame, 3333,widthCalculated,widthCalculated, 0,0, widthCalculated,widthCalculated,              _FrameColor(),       _BkColor(),  _BkColor());
	}

	void __Show_Circle_Indirect(uint16_t x, uint16_t y, uint16_t width, uint8_t bold){
		LCD_ClearPartScreen(0,width,width,RGB2INT(Circle.bk[0],Circle.bk[1],Circle.bk[2]));
		LCD_SetCircleAA(Circle.ratioBk,Circle.ratioFill);
		LCD_ShapeIndirect(x,y,LCD_Circle, width,width, SetBold2Color(_FrameColor(),bold), _FillColor(), _BkColor());
	}
*/
	if(START_SCREEN_Test_Circle())
	{
		SCREEN_ResetAllParameters();
		LCD_LoadFont_DarkgrayWhite(FONT_10, 	  Arial, fontID_1);
		LCD_LoadFont_DarkgrayWhite(FONT_10_bold, Arial, fontID_2);
	}

	if(Circle.startFlag!=0x52)
	{
		Circle.startFlag=0x52;
		Circle.width=200;

		Circle.bk[0]=R_PART(MYGRAY);
		Circle.bk[1]=G_PART(MYGRAY);
		Circle.bk[2]=B_PART(MYGRAY);

		Circle.frame[0]=R_PART(WHITE);
		Circle.frame[1]=G_PART(WHITE);
		Circle.frame[2]=B_PART(WHITE);

		Circle.fill[0]=R_PART(MYGRAY3);
		Circle.fill[1]=G_PART(MYGRAY3);
		Circle.fill[2]=B_PART(MYGRAY3);

		Circle.ratioBk=0.0;
		Circle.ratioFill=0.0;
		Circle.bold=0;
		Circle.halfCircle=0;

		Circle.deg[0]=45;		Circle.degColor[0]=RED;  /* not use yet*/			/* example in future:  float Circle.deg[0]=45.4;*/
		Circle.deg[1]=125;	Circle.degColor[1]=MYRED;
		Circle.deg[2]=170;   Circle.degColor[2]=DARKYELLOW;
		Circle.deg[3]=225;   Circle.degColor[3]=DARKGREEN;
		Circle.deg[4]=280;   Circle.degColor[4]=DARKBLUE;
		Circle.deg[5]=320;   Circle.degColor[5]=DARKCYAN;
		Circle.deg[6]=360;   Circle.degColor[6]=BROWN;
	}

	CorrectLineAA_off();

/* --- Worse set for percent circle --- */
/*	LCD_SetCircleDegrees(4,Circle.deg[0],Circle.deg[1],Circle.deg[2],Circle.deg[3],Circle.deg[4],Circle.deg[5],Circle.deg[6]);
	LCD_SetCircleDegreesBuff(4,Circle.deg);
	LCD_SetCircleDegColors(4,Circle.degColor[0],Circle.degColor[1],Circle.degColor[2],Circle.degColor[3],Circle.degColor[4],Circle.degColor[5],Circle.degColor[6]);
	LCD_SetCircleDegColorsBuff(4,Circle.degColor);
*/
	LCD_SetCirclePercentParam(7,Circle.deg,Circle.degColor);
	LCD_Clear(RGB2INT(Circle.bk[0],Circle.bk[1],Circle.bk[2]));

	lenStr=LCD_Str(fontID_1, LCD_Xpos(lenStr,SetPos,0),  LCD_Ypos(lenStr,SetPos,0), StrAll(2,"Radius } | ",Int2Str(Circle.width/2,' ',3,Sign_none)), 	 fullHight,0,_BkColor(),1,1);
	lenStr=LCD_Str(fontID_2, LCD_Xpos(lenStr,IncPos,10), LCD_Ypos(lenStr,GetPos,0), StrAll(2,"angle:",INT2STR_TIME(Circle.deg[0])),fullHight,0,_BkColor(),1,0);
	lenStr=LCD_Str(fontID_1, LCD_Xpos(lenStr,SetPos,0), LCD_Ypos(lenStr,IncPos,8), StrAll(6,"Frame: ",INT2STR(Circle.frame[0])," ",INT2STR(Circle.frame[1])," ",INT2STR(Circle.frame[2])),  halfHight,0,_BkColor(),1,1);
	lenStr=LCD_Str(fontID_1, LCD_Xpos(lenStr,GetPos,0), LCD_Ypos(lenStr,IncPos,5), StrAll(6,"Fill:  ",INT2STR(Circle.fill[0]), " ",INT2STR(Circle.fill[1]), " ",INT2STR(Circle.fill[2])),   halfHight,0,_BkColor(),1,1);
	lenStr=LCD_Str(fontID_1, LCD_Xpos(lenStr,GetPos,0), LCD_Ypos(lenStr,IncPos,5), StrAll(6,"Backup:",INT2STR(Circle.bk[0]),   " ",INT2STR(Circle.bk[1]),   " ",INT2STR(Circle.bk[2])),   halfHight,0,_BkColor(),1,1);
	lenStr=LCD_Str(fontID_2, LCD_Xpos(lenStr,GetPos,0), LCD_Ypos(lenStr,IncPos,5), StrAll(4,"AA out:",Float2Str(Circle.ratioBk,' ',1,Sign_none,2),"  AA in:",Float2Str(Circle.ratioFill,' ',1,Sign_none,2)), 	 halfHight,0,_BkColor(),1,1);

	if(Circle.bold > Circle.width/2-1)
		Circle.bold= Circle.width/2-1;

	LCD_SetCircleAA(Circle.ratioBk,Circle.ratioFill);
	CorrectLineAA_on();
	StartMeasureTime_us();

/* --- Set points to draw own circle --- */
/*	LCD_SetCircleParam(0.01,0.01,14, 5,4,4,3,3,2,2,2,2,2, 2,2,1,1);
	LCD_Shape(60,160, LCD_Circle, 0,0, _FrameColor(), _FillColor(), _BkColor());	*/

	LCD_Shape(CIRCLE_POS_XY(Circle.width,10,10), LCD_Circle, SetParamWidthCircle(Percent_Circle,Circle.width),Circle.width, SetBold2Color(_FrameColor(),Circle.bold), _FillColor() /*TRANSPARENT*/, _BkColor());

/*	LCD_Shape(LCD_X-LCD_CalculateCircleWidth(Circle.width)/2-10, LCD_Y-LCD_CalculateCircleWidth(Circle.width)-10 , LCD_HalfCircle, SetParamWidthCircle(Half_Circle_270,Circle.width),Circle.width, SetBold2Color(_FrameColor(),Circle.bold), _FillColor(), _BkColor()); */

	Circle.speed=StopMeasureTime_us("");

	lenStr=LCD_Str(fontID_2, LCD_Xpos(lenStr,GetPos,0), LCD_Ypos(lenStr,IncPos,8), StrAll(5,"Speed Max: ",INT2STR_TIME(Circle.speed)," us    Speed Sum: ",INT2STR_TIME(Circle.speedSum)," us"), 	 halfHight,0,_BkColor(),1,1);
	lenStr=LCD_Str(fontID_2, LCD_Xpos(lenStr,GetPos,0), LCD_Ypos(lenStr,IncPos,8), StrAll(2,"Bold: ",Int2Str(Circle.bold,' ',2,Sign_none)), halfHight,0,_BkColor(),1,1);
	LCD_Show();

/*	__Show_FrameAndCircle_Indirect(CIRCLE_POS_XY(Circle.width,10,20), Circle.width, Circle.bold);
	__Show_Circle_Indirect(LCD_GetXSize()-50,200, 50, 0);
	LCD_ShapeIndirect(400,350,LCD_Circle, 50,50, SetBold2Color(WHITE,0), RED, MYGRAY);	*/

}

uint16_t param0 = 100;
uint16_t param1 = 80;
float param2 = 0.4;



void* TEST_function(void *a, void *b){
	static int aaS=0;
	int a_ = *((uint16_t*)a);
	int b_ = *((uint16_t*)b);

	 DbgVar(1,50,"\r\nAAAAKKUKUKUKU  !!!!@@ %d %d  ",a_,b_);
	 aaS=a_+b_;
	 return &aaS;
}

static void DBG_SCREEN_Test_Circle(void)
{
	int refresh_Screen=1;

		  if(DEBUG_RcvStr("]")) { if(Circle.width < MAX_WIDTH_CIRCLE) Circle.width=LCD_GetNextIncrCircleWidth(Circle.width); }
	else if(DEBUG_RcvStr("\\")){ if(Circle.width > MIN_WIDTH_CIRCLE) Circle.width=LCD_GetNextDecrCircleWidth(Circle.width); }

	else if(DEBUG_RcvStr("1")) INCR_WRAP(Circle.frame[0],1,0,255);
	else if(DEBUG_RcvStr("2")) INCR_WRAP(Circle.frame[1],1,0,255);
	else if(DEBUG_RcvStr("3")) INCR_WRAP(Circle.frame[2],1,0,255);
	else if(DEBUG_RcvStr("q")) DECR_WRAP(Circle.frame[0],1,0,255);
	else if(DEBUG_RcvStr("w")) DECR_WRAP(Circle.frame[1],1,0,255);
	else if(DEBUG_RcvStr("e")) DECR_WRAP(Circle.frame[2],1,0,255);
/*
	else if(DEBUG_RcvStr("a")) INCR_WRAP(Circle.bk[0],1,0,255);
	else if(DEBUG_RcvStr("s")) INCR_WRAP(Circle.bk[1],1,0,255);
	else if(DEBUG_RcvStr("d")) INCR_WRAP(Circle.bk[2],1,0,255);
	else if(DEBUG_RcvStr("z")) DECR_WRAP(Circle.bk[0],1,0,255);
	else if(DEBUG_RcvStr("x")) DECR_WRAP(Circle.bk[1],1,0,255);
	else if(DEBUG_RcvStr("c")) DECR_WRAP(Circle.bk[2],1,0,255);
*/
	else if(DEBUG_RcvStr("4")) INCR_WRAP(Circle.fill[0],1,0,255);
	else if(DEBUG_RcvStr("5")) INCR_WRAP(Circle.fill[1],1,0,255);
	else if(DEBUG_RcvStr("6")) INCR_WRAP(Circle.fill[2],1,0,255);
	else if(DEBUG_RcvStr("r")) DECR_WRAP(Circle.fill[0],1,0,255);
	else if(DEBUG_RcvStr("t")) DECR_WRAP(Circle.fill[1],1,0,255);
	else if(DEBUG_RcvStr("y")) DECR_WRAP(Circle.fill[2],1,0,255);

	else if(DEBUG_RcvStr("h")) INCR_FLOAT_WRAP(Circle.ratioBk,  0.10, 0.00, 1.00);
	else if(DEBUG_RcvStr("j")) INCR_FLOAT_WRAP(Circle.ratioFill,0.10, 0.00, 1.00);
	else if(DEBUG_RcvStr("n")) DECR_FLOAT_WRAP(Circle.ratioBk  ,0.10, 0.00, 1.00);
	else if(DEBUG_RcvStr("m")) DECR_FLOAT_WRAP(Circle.ratioFill,0.10, 0.00, 1.00);

	else if(DEBUG_RcvStr("g")) Circle.bold= LCD_IncrCircleBold(Circle.width,Circle.bold);
	else if(DEBUG_RcvStr("b")) Circle.bold= LCD_DecrCircleBold(Circle.width,Circle.bold);

	else if(DEBUG_RcvStr("o")) INCR_WRAP(Circle.halfCircle,1,0,3);

	else if(DEBUG_RcvStr("k")) DECR(Circle.deg[0],1,0);
	else if(DEBUG_RcvStr("l")) INCR(Circle.deg[0],1,Circle.deg[1]-1);
	else if(DEBUG_RcvStr(",")) DECR(Circle.deg[1],1,Circle.deg[0]+1);
	else if(DEBUG_RcvStr(".")) INCR(Circle.deg[1],1,Circle.deg[2]-1);
	else if(DEBUG_RcvStr("9")) DECR(Circle.deg[2],1,Circle.deg[1]+1);
	else if(DEBUG_RcvStr("0")) INCR(Circle.deg[2],1,Circle.deg[3]-1);
	else if(DEBUG_RcvStr("7")) DECR(Circle.deg[3],1,Circle.deg[2]+1);
	else if(DEBUG_RcvStr("8")) INCR(Circle.deg[3],1,360);

	else if(DEBUG_RcvStr("s")){ INCR(param0,1,MAX_WIDTH_CIRCLE); SETPARAM_Circle_1(&param0,&param1,&param2,(VOID_FUNCTION_TEST)TEST_function); DbgVar(1,50,"\r\nParam: %d  %d  %s",param0,param1,Float2Str(param2,' ',1,Sign_none,2)); }
	else if(DEBUG_RcvStr("x")){ DECR(param0,1,1); 					 SETPARAM_Circle_1(&param0,&param1,&param2,(VOID_FUNCTION_TEST)TEST_function); DbgVar(1,50,"\r\nParam: %d  %d  %s",param0,param1,Float2Str(param2,' ',1,Sign_none,2)); }

	else if(DEBUG_RcvStr("d")){ INCR(param1,5,95); SETPARAM_Circle_1(&param0,&param1,&param2,(VOID_FUNCTION_TEST)TEST_function); DbgVar(1,50,"\r\nParam: %d  %d  %s",param0,param1,Float2Str(param2,' ',1,Sign_none,2)); }
	else if(DEBUG_RcvStr("c")){ DECR(param1,5,20); SETPARAM_Circle_1(&param0,&param1,&param2,(VOID_FUNCTION_TEST)TEST_function); DbgVar(1,50,"\r\nParam: %d  %d  %s",param0,param1,Float2Str(param2,' ',1,Sign_none,2)); }

	else if(DEBUG_RcvStr("f")){ INCR(param2,0.1,3.0); SETPARAM_Circle_1(&param0,&param1,&param2,(VOID_FUNCTION_TEST)TEST_function); DbgVar(1,50,"\r\nParam: %d  %d  %s",param0,param1,Float2Str(param2,' ',1,Sign_none,2)); }
	else if(DEBUG_RcvStr("v")){ DECR(param2,0.1,0.0); SETPARAM_Circle_1(&param0,&param1,&param2,(VOID_FUNCTION_TEST)TEST_function); DbgVar(1,50,"\r\nParam: %d  %d  %s",param0,param1,Float2Str(param2,' ',1,Sign_none,2)); }

	else refresh_Screen=0;

	if(refresh_Screen) SCREEN_Test_Circle();

}
/* ########### --- END SCREEN_Test_Circle --- ############ */



void SCREEN_ReadPanel(void)
{
	if(startScreen==0)
	{
		switch(SCREEN_number)  //DYNAMICZNE ZARZADZANIE PAMIECIA FONT Z SDRAM zeby tyle nie zajmowalo !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		{
		case 0:
/*			SCREEN_Fonts_funcSet(FONTS_FONT_COLOR_FontColor, MYRED);
			SCREEN_Fonts_printInfo();	*/
			SCREEN_Fonts_main(argNmb,argVal);
			startScreen=1;
			break;
		case 1:
			NOWY_1();
			startScreen=1;
			break;
		case 2:
			NOWY_2();
			startScreen=1;
			break;
		case 3:
			NOWY_3();
			startScreen=1;
			break;
		case 4:
			SCREEN_Test_Circle();
			startScreen=1;
			break;
		case 5:
/*			SCREEN_Calibration_funcSet(CALIBRATION_FONT_SIZE_PosPhys, SCREEN_Calibration_funcGet(CALIBRATION_FONT_SIZE_Title));
			SCREEN_Calibration_funcSet(CALIBRATION_FONT_SIZE_PosLog, SCREEN_Calibration_funcGet(CALIBRATION_FONT_SIZE_CircleName));
			SCREEN_Calibration_funcSet(CALIBRATION_COLOR_BkScreen, MYGRAY);
			SCREEN_Calibration_funcSet(CALIBRATION_FONT_COLOR_CircleName, RED);
			SCREEN_Calibration_funcSet(CALIBRATION_COLOR_CircleFill, LIGHTRED);
			SCREEN_Calibration_funcSet(CALIBRATION_COEFF_COLOR_PosLog, 254);
			SCREEN_Calibration_printInfo();	*/
			SCREEN_Calibration_main(argNmb,argVal);
			startScreen=1;
			break;
		}
		SCREEN_number_prev = SCREEN_number;
	}
	else
	{
		switch(SCREEN_number)
		{
		case 0:
			SCREEN_Fonts_debugRcvStr();
			SCREEN_Fonts_setTouch();
			break;

		case 1:
			break;

		case 2:
			if(LCD_IsRefreshScreenTimeout(refresh_1,500)) LCD_StrRotVarIndirect				(fontVar_1,INT2STR(++test[0]));
			if(LCD_IsRefreshScreenTimeout(refresh_2,500)) LCD_StrChangeColorRotVarIndirect(fontVar_2,INT2STR(++test[1]));

			if(LCD_IsRefreshScreenTimeout(refresh_3,500)) LCD_StrRotVarIndirect				(fontVar_3,INT2STR(++test[2]));
		   if(LCD_IsRefreshScreenTimeout(refresh_4,500)) LCD_StrChangeColorRotVarIndirect(fontVar_4,INT2STR(++test[3]));

			if(LCD_IsRefreshScreenTimeout(refresh_5,500)) LCD_StrRotVarIndirect				(fontVar_5,INT2STR(++test[4]));
			if(LCD_IsRefreshScreenTimeout(refresh_6,500)) LCD_StrChangeColorRotVarIndirect(fontVar_6,INT2STR(++test[5]));

			     if(DEBUG_RcvStr("1")) LCD_StrRotVarIndirect(fontVar_7,"abAb");
			else if(DEBUG_RcvStr("2")) LCD_StrRotVarIndirect(fontVar_7,"Rafa� Markiel�wski hhhhX");
			else if(DEBUG_RcvStr("3")){ LCD_OffsStrVar_x(fontVar_7,10);  LCD_StrRotVarIndirect(fontVar_7,"XY");  }
			else if(DEBUG_RcvStr("4")){ LCD_OffsStrVar_x(fontVar_7,-10); LCD_StrRotVarIndirect(fontVar_7,"XY");  }
			else if(DEBUG_RcvStr("5")){ LCD_OffsStrVar_y(fontVar_7,10);  LCD_StrRotVarIndirect(fontVar_7,"SD");  }
			else if(DEBUG_RcvStr("6")){ LCD_OffsStrVar_y(fontVar_7,-10); LCD_StrRotVarIndirect(fontVar_7,"SD");  }

			else if(DEBUG_RcvStr("a")) LCD_StrChangeColorRotVarIndirect(fontVar_8,"abAb");
			else if(DEBUG_RcvStr("b")) LCD_StrChangeColorRotVarIndirect(fontVar_8,"Rafa� Markiel�wski hhhhX");
			else if(DEBUG_RcvStr("c")){ LCD_OffsStrVar_x(fontVar_8,10);  LCD_StrChangeColorRotVarIndirect(fontVar_8,"XY");  }
			else if(DEBUG_RcvStr("d")){ LCD_OffsStrVar_x(fontVar_8,-10); LCD_StrChangeColorRotVarIndirect(fontVar_8,"XY");  }
			else if(DEBUG_RcvStr("e")){ LCD_OffsStrVar_y(fontVar_8,10);  LCD_StrChangeColorRotVarIndirect(fontVar_8,"SD");  }
			else if(DEBUG_RcvStr("f")){ LCD_OffsStrVar_y(fontVar_8,-10); LCD_StrChangeColorRotVarIndirect(fontVar_8,"SD");  }
			break;

		case 3:
			if(LCD_IsRefreshScreenTimeout(refresh_1,60)) LCD_StrMovHIndirect(fontVar_1,1);
			if(LCD_IsRefreshScreenTimeout(refresh_2,60)) LCD_StrMovHIndirect(fontVar_2,1);
			if(LCD_IsRefreshScreenTimeout(refresh_3,20)) LCD_StrMovHIndirect(fontVar_3,1);

			if(LCD_IsRefreshScreenTimeout(refresh_4,60)) LCD_StrMovHIndirect(fontVar_4,1);
			if(LCD_IsRefreshScreenTimeout(refresh_5,60)) LCD_StrMovHIndirect(fontVar_5,1);
			if(LCD_IsRefreshScreenTimeout(refresh_6,20)) LCD_StrMovHIndirect(fontVar_6,1);

			if(LCD_IsRefreshScreenTimeout(refresh_7,60)) LCD_StrMovVIndirect(fontVar_7,1);
			if(LCD_IsRefreshScreenTimeout(refresh_8,20)) LCD_StrMovVIndirect(fontVar_8,1);

			if(LCD_IsRefreshScreenTimeout(refresh_9,60))  LCD_StrMovVIndirect(fontVar_9,1);
			if(LCD_IsRefreshScreenTimeout(refresh_10,20)) LCD_StrMovVIndirect(fontVar_10,1);

			if(LCD_IsRefreshScreenTimeout(refresh_11,60))  LCD_StrMovVIndirect(fontVar_11,1);
			if(LCD_IsRefreshScreenTimeout(refresh_12,20)) LCD_StrMovVIndirect(fontVar_12,1);
			break;

		case 4:
			DBG_SCREEN_Test_Circle();
			break;
//rozpisac wyswietlanie wszystkich degree i uatwic circleLinesLenCorrect  !!!!!!!!!!!!!!!!
		}
	}



	//if(IsRefreshScreenTimeout(refresh_10,2000)){ if(u==0) IncFontSize(); else DecFontSize();  startScreen++; if(startScreen>17){ startScreen=1; u=1-u; } }
	//Data2Refresh(PARAM_MOV_TXT);
//


	//if(IsRefreshScreenTimeout(refresh_7,40)) LCD_StrMovHIndirect_TEST(STR_ID_MovFonts_7,1);
	//if(IsRefreshScreenTimeout(refresh_8,50)) LCD_StrMovVI]]]]]]]\ndirect_TEST(STR_ID_MovFonts_8,1);




	if(DEBUG_RcvStr("=")){ startScreen=0; INCR_WRAP(SCREEN_number,1,0,5); }
	if(DEBUG_RcvStr("n")) LCD_DisplayStrMovBuffState();


}

/*
 Jak czytam zycie ludzi swietych kazdego dnia jak jest wspomnienie w kosciolacj to jest to heroizm, Aga mowi ze to luzie chorzy psychicznie i ma racje w 100% jesli myslimy wylacznie po ludzku

 Bo jak jest napisane "Głupstwem je st dla boga madrosc ludzka i odwrotnie dla ludzi madrosc boga"
 */



