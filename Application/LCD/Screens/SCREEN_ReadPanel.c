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
#define INT2STR_TIME(val) Int2Str(val,' ',6,Sign_none)

int argNmb = 0;
char **argVal = NULL;

int SCREEN_number=0, SCREEN_number_prev=-1;  //LOAD IMAGE !!!!!


StructTxtPxlLen lenStr;
int startScreen=0;

/*   //TO w osobnym pliku np examples.c !!!!!!!!!!!!!!!
void LCDEXAMPLE_RectangleGradient(u32 COLOR_FillFrame, u32 COLOR_Frame, u32 COLOR_BkScreen)
{
	SHAPE_PARAMS par1,par2,par3,par4,par5,par6;

	LCD_Rectangle2(0, LCD_X,LCD_Y, 20,240, 100, 40,  COLOR_FillFrame,BrightIncr(COLOR_FillFrame,0x55),   			  COLOR_FillFrame,ORANGE,  COLOR_BkScreen, 0.0, Down);
	LCD_Rectangle2(0, LCD_X,LCD_Y, 160,240, 100, 40, BrightDecr(COLOR_Frame,0xA5),BrightDecr(COLOR_Frame,0xAE),   COLOR_FillFrame,YELLOW,  COLOR_BkScreen, 0.0, Up);
	LCD_Rectangle2(0, LCD_X,LCD_Y, 270,240, 100, 40, BrightDecr(COLOR_Frame,0xA0),BrightIncr(COLOR_Frame,0x01),   COLOR_FillFrame,BrightDecr(COLOR_FillFrame,0x09),  COLOR_BkScreen, 0.0, Midd_Y);
	LCD_Rectangle2(0, LCD_X,LCD_Y, 380,240, 100, 40, BrightDecr(COLOR_Frame,0xA0),BrightIncr(COLOR_Frame,0x01),   COLOR_FillFrame,BrightDecr(COLOR_FillFrame,0x15),  COLOR_BkScreen, 0.0, Midd_X);

	LCD_Rectangle2(0, LCD_X,LCD_Y, 20,300, 100, 40,  0xFF202020,0xFF282828,   0xFF909090,BrightDecr(0xFF909090,0x20),  COLOR_BkScreen, 0.0, Midd_Y);
	//LCD_Rectangle2(0, LCD_X,LCD_Y, 160,300, 100, 40, 0xFF202020,0xFF282828,   0xFF909090,BrightDecr(0xFF909090,0x20),  v.COLOR_BkScreen, 0.0, Midd_Y2);
	LCD_Rectangle2(0, LCD_X,LCD_Y, 160,300, 100, 40, WHITE,(u32)bold8,   0xFF909090,BrightIncr(0xFF909090,0x20),  COLOR_BkScreen, 0.0, AllEdge);

	par3=LCD_Rectangle2(ToStructAndReturn, LCD_X,LCD_Y, 500,360, 100, 25, BrightDecr(COLOR_Frame,0xA0),BrightIncr(COLOR_Frame,0x01),  COLOR_FillFrame,BrightDecr(COLOR_FillFrame,0x35),  COLOR_BkScreen, 0.0, RightDown);
	par4=LCD_Rectangle2(ToStructAndReturn, LCD_X,LCD_Y, 610,360, 29, 100, BrightDecr(COLOR_Frame,0xA0),BrightIncr(COLOR_Frame,0x01),  COLOR_FillFrame,BrightDecr(COLOR_FillFrame,0x35),  COLOR_BkScreen, 0.0, RightDown);

	par1=LCD_RoundRectangle2(ToStructAndReturn,PARAM32(AllEdge,bold10,unUsed,Rectangle),LCD_X,LCD_Y,70,350,70,80 ,WHITE, WHITE, 0xFFA0A0A0,BrightDecr(0xFFA0A0A0,0x30), COLOR_BkScreen, 0.0, Down);
	par2=LCD_RoundRectangle2(ToStructAndReturn,PARAM32(Up,bold2,unUsed,Rectangle), 		LCD_X,LCD_Y,180,350,70,80, BrightDecr(COLOR_Frame,0x20),BrightIncr(COLOR_Frame,0x65), 0xFF808080,BrightDecr(0xFF808080,0x44), COLOR_BkScreen, 0.0, Left);
		  LCD_RoundRectangle2(0,					 PARAM32(Down,bold3,unUsed,Rectangle), 	LCD_X,LCD_Y,280,390,70,80, BrightDecr(COLOR_Frame,0x20),BrightIncr(COLOR_Frame,0x65), 0xFF808080,BrightDecr(0xFF808080,0x44), COLOR_BkScreen, 0.0, Down);

	LCDSHAPE_RoundRectangle(0,par1);
	LCDSHAPE_RoundRectangle(0,par2);

	par5=LCD_Rectangle2(ToStructAndReturn, LCD_X,LCD_Y, 570,240, 100, 40, COLOR_FillFrame,				   BrightIncr(COLOR_FillFrame,0x55),   COLOR_FillFrame,ORANGE,  COLOR_BkScreen, 0.25, Down);
	par6=LCD_Rectangle2(ToStructAndReturn, LCD_X,LCD_Y, 570,300, 100, 40, BrightDecr(COLOR_Frame,0xA5),BrightDecr(COLOR_Frame,0xAE), 		COLOR_FillFrame,YELLOW,  COLOR_BkScreen, 0.3, Up);

	LCDSHAPE_Rectangle(0,par3);
	LCDSHAPE_Rectangle(0,par4);
	LCDSHAPE_Rectangle(0,par5);
	LCDSHAPE_Rectangle(0,par6);

//	LCDSHAPE_Rectangle_Indirect(par3);
//	LCDSHAPE_Rectangle_Indirect(par4);
//	LCDSHAPE_Rectangle_Indirect(par5);
//	LCDSHAPE_Rectangle_Indirect(par6);
} */

void LCDEXAMPLE_GradientCircleButtonAndSlider(int FONT_ID_Title, int FONT_VAR_Title, u32 COLOR_FillMainFrame, u32 COLOR_Frame, u32 COLOR_BkScreen)
{
	uint32_t CircleWidth= 160;
	SHAPE_PARAMS par={0}, par2={0};
	uint16_t calcWidth = LCD_CalculateCircleWidth(CircleWidth);

	LCD_SetCircleAA(0.0, 0.0);
	CorrectLineAA_on();
	LCD_BkFontTransparent(FONT_VAR_Title, FONT_ID_Title);

	StartMeasureTime_us();

	par=LCD_GradientCircleSlider(ToStructAndReturn,LCD_X,LCD_Y, LCD_X-calcWidth-10-625, LCD_Y-calcWidth-50,CircleWidth,CircleWidth,   SetBold2Color(COLOR_BkScreen,23),COLOR_FillMainFrame,0x666666,0xBBBBBB,0x666666,   SetBold2Color(COLOR_FillMainFrame,15),0xC0C0C0,0x333333,  COLOR_BkScreen,270,Center,0);
	par2=par;
	LCDSHAPE_GradientCircleSlider(0,par);
	LCD_StrDependOnColorsMidd(FONT_ID_Title, POS_SIZE_CIRCLEBUTTONSLIDER(par,0), "23", fullHight,0, BK_COLOR_CIRCLESLIDER(par), WHITE, 250, ConstWidth);

	LCDSHAPE_Create(0,LCD_X,LCD_Y,LCD_X-calcWidth-10-450, LCD_Y-calcWidth-50, SetParamWidthCircle(0,CircleWidth),CircleWidth, SetBold2Color(WHITE,30), COLOR_BkScreen, COLOR_BkScreen, RED,BLUE,0,360,RightDown,0);
	LCDSHAPE_Create(0,LCD_X,LCD_Y,LCD_X-calcWidth-10-150, LCD_Y-calcWidth-50, SetParamWidthCircle(Percent_Circle,CircleWidth),CircleWidth, SetBold2Color(COLOR_BkScreen,0), COLOR_FillMainFrame, COLOR_BkScreen, RED,BLUE,0,350,Left,0);
	LCDSHAPE_Create(0,LCD_X,LCD_Y,LCD_X-calcWidth-10, LCD_Y-calcWidth-50,     SetParamWidthCircle(Percent_Circle,CircleWidth),CircleWidth, SetBold2Color(COLOR_BkScreen,0), COLOR_FillMainFrame, COLOR_BkScreen, RED,BLUE,0,355,Right,0);
	StopMeasureTime_us("Time:");

	//---- INDIRECT option----
	par=LCD_GradientCircleButton(ToStructAndReturn,CircleWidth,CircleWidth,LCD_X-calcWidth-10, LCD_Y-calcWidth-50, CircleWidth,CircleWidth, SetBold2Color(COLOR_BkScreen,18),LIGHTBLUE,DARKBLUE,COLOR_BkScreen,0);
	LCDSHAPE_GradientCircleButton_Indirect(par);
	LCD_StrDependOnColorsWindowMiddIndirect(0, POS_SIZE_CIRCLEBUTTONSLIDER(par,2), FONT_ID_Title, "987", fullHight,0, BK_COLOR_CIRCLEBUTTON(par), WHITE, 248, ConstWidth);

	LCDSHAPE_GradientCircleSlider_Indirect(par2);
	LCD_StrDependOnColorsWindowMiddIndirect(0, POS_SIZE_CIRCLEBUTTONSLIDER(par2,2), FONT_ID_Title, "23", fullHight,0, BK_COLOR_CIRCLESLIDER(par2), WHITE, 248, ConstWidth);
}

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
		LCD_Arrow(0,bkpSizeX,bkpSizeY, x,										MIDDLE(y,height,heightArrow), SetLineBold2Width(xTxt-x-spaceArrowTxt,boldLine), SetTriangHeightCoeff2Height(heightArrow,heightTraingCoeff), frameColor,fillColor,bkpColor, CONDITION(outside==direct,Left,Right));
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


	lenStr=LCD_StrRotVar(fontVar_1,Rotate_0,fontID_6,  LCD_Xpos(lenStr,SetPos,5),LCD_Ypos(lenStr,SetPos,5), INT2STR(test[0]), halfHight, 0,DARKRED,0,ConstWidth,bkColor);
	LCD_Frame(0,LCD_GetXSize(),LCD_GetYSize(),			LCD_Xpos(lenStr,GetPos,-1),LCD_Ypos(lenStr,GetPos,-1), lenStr.inPixel+2,lenStr.height+2,WHITE,0,0);

	lenStr=LCD_StrChangeColorRotVar(fontVar_2,Rotate_0,fontID_7, LCD_Xpos(lenStr,IncPos,5),LCD_Ypos(lenStr,GetPos,0), INT2STR(test[1]), halfHight, 0,GRAY,MYBLUE,249,ConstWidth,bkColor);
	LCD_Frame(0,LCD_GetXSize(),LCD_GetYSize(),						LCD_Xpos(lenStr,GetPos,-1),LCD_Ypos(lenStr,GetPos,-1), lenStr.inPixel+2,lenStr.height+2,WHITE,0,0);


	lenStr=LCD_StrRotVar(fontVar_3,Rotate_90,fontID_6, LCD_Xpos(lenStr,SetPos,5),LCD_Ypos(lenStr,IncPos,10), INT2STR(test[2]), halfHight, 0,DARKRED,0,ConstWidth,bkColor);
	LCD_Frame(0,LCD_GetXSize(),LCD_GetYSize(),			LCD_Xpos(lenStr,GetPos,-1),LCD_Ypos(lenStr,GetPos,-1), lenStr.inPixel+2,lenStr.height+2,WHITE,0,0);

	lenStr=LCD_StrChangeColorRotVar(fontVar_4,Rotate_90,fontID_7, LCD_Xpos(lenStr,IncPos,5),LCD_Ypos(lenStr,GetPos,0), INT2STR(test[3]), halfHight, 0,GRAY,MYBLUE,249,ConstWidth,bkColor);
	LCD_Frame(0,LCD_GetXSize(),LCD_GetYSize(),						 LCD_Xpos(lenStr,GetPos,-1),LCD_Ypos(lenStr,GetPos,-1), lenStr.inPixel+2,lenStr.height+2,WHITE,0,0);


	lenStr=LCD_StrRotVar(fontVar_5,Rotate_180,fontID_6,  LCD_Xpos(lenStr,SetPos,5),LCD_Ypos(lenStr,IncPos,10), INT2STR(test[4]), halfHight, 0,DARKRED,0,ConstWidth,bkColor);
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

	lenStr=LCD_StrMovH(fontVar_1,Rotate_0,0,109,   fontID_6,  100,0,"AAA11111111111222222222222222 33333333333333333333A",fullHight,0,DARKBLUE,0,1);
	lenStr=LCD_StrMovH(fontVar_2,Rotate_90,0,109,   fontID_6, 0,0,"BBB1111111111222222222222222 33333333333333333333B",fullHight,0,DARKBLUE,0,1);
	lenStr=LCD_StrMovH(fontVar_3,Rotate_180,0,109,   fontID_6, 50,0,"CCCC11111111111222222222222222 33333333333333333333C",fullHight,0,DARKBLUE,0,1);

	lenStr=LCD_StrChangeColorMovH(fontVar_4,Rotate_0,0,80,fontID_7, 100,50,"Zeszyty i ksi��ki to podr�czniki do szko�y dla ��o�A_XY",fullHight,0,DARKYELLOW,DARKRED,254,0);
	lenStr=LCD_StrChangeColorMovH(fontVar_5,Rotate_90,0,80,fontID_7, 400,0,"Zeszyty i ksi��ki to podr�czniki do szko�y dla ��o�A_XY",fullHight,0,DARKYELLOW,DARKRED,254,0);
	lenStr=LCD_StrChangeColorMovH(fontVar_6,Rotate_180,0,80,fontID_7, 435,0,"Zeszyty i ksi��ki to podr�czniki do szko�y dla ��o�A_XY",fullHight,0,DARKYELLOW,DARKRED,254,0);


	lenStr=LCD_StrChangeColorMovV(fontVar_7,Rotate_0,0,100,50,fontID_7,100,100,"Rafa� Markielowski jest ww p omieszczeniu gospodarczym lubi krasnale www doku na drzewie takie jego bojowe zadanie  SEX _XY",fullHight,0,LIGHTGRAY,DARKBLUE,249,0);
	lenStr=LCD_StrMovV           (fontVar_8,Rotate_0,0,100,50,fontID_6,210,100,"1234567890 abcdefghijklnn opqrstuvw ABCDEFGHIJKLMNOPQRTSUVWXYZ iiiiiiiiiiiijjjjjjjjjjjjSSSSSSSSSSEEEEEEEEEERRRRRRRRRA _XY",fullHight,0,MYGRAY,0,0);

	lenStr=LCD_StrChangeColorMovV(fontVar_9,Rotate_90,0,100,50,fontID_7,0,160,"Rafa� Markielowski jest ww p omieszczeniu gospodarczym lubi krasnale www doku na drzewie takie jego bojowe zadanie  SEX _XY",fullHight,0,LIGHTGRAY,DARKBLUE,249,0);
	lenStr=LCD_StrMovV           (fontVar_10,Rotate_90,0,100,50,fontID_6,110,160,"1234567890 abcdefghijklnn opqrstuvw ABCDEFGHIJKLMNOPQRTSUVWXYZ iiiiiiiiiiiijjjjjjjjjjjjSSSSSSSSSSEEEEEEEEEERRRRRRRRRA _XY",fullHight,0,MYGRAY,0,0);

	lenStr=LCD_StrChangeColorMovV(fontVar_11,Rotate_180,0,100,50,fontID_7,220,160,"Rafa� Markielowski jest ww p omieszczeniu gospodarczym lubi krasnale www doku na drzewie takie jego bojowe zadanie  SEX _XY",fullHight,0,LIGHTGRAY,DARKBLUE,249,0);
	lenStr=LCD_StrMovV           (fontVar_12,Rotate_180,0,100,50,fontID_6,330,160,"1234567890 abcdefghijklnn opqrstuvw ABCDEFGHIJKLMNOPQRTSUVWXYZ iiiiiiiiiiiijjjjjjjjjjjjSSSSSSSSSSEEEEEEEEEERRRRRRRRRA _XY",fullHight,0,MYGRAY,0,0);




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

static int START_SCREEN_Test_Circle(void){
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
		Circle.deg[2]=180;   Circle.degColor[2]=DARKYELLOW;
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

	lenStr=LCD_Str(fontID_1, LCD_Xpos(lenStr,SetPos,0),  LCD_Ypos(lenStr,SetPos,0), StrAll(2,"Radius } | ",Int2Str(Circle.width/2,' ',3,Sign_none)), 	 fullHight,0,_BkColor(),0,1);
	lenStr=LCD_Str(fontID_2, LCD_Xpos(lenStr,IncPos,10), LCD_Ypos(lenStr,GetPos,0), StrAll(2,"angle:",INT2STR_TIME(Circle.deg[0])),fullHight,0,_BkColor(),1,0);
	lenStr=LCD_Str(fontID_1, LCD_Xpos(lenStr,SetPos,0), LCD_Ypos(lenStr,IncPos,8), StrAll(6,"Frame: ",INT2STR(Circle.frame[0])," ",INT2STR(Circle.frame[1])," ",INT2STR(Circle.frame[2])),  halfHight,0,_BkColor(),0,1);
	lenStr=LCD_Str(fontID_1, LCD_Xpos(lenStr,GetPos,0), LCD_Ypos(lenStr,IncPos,5), StrAll(6,"Fill:  ",INT2STR(Circle.fill[0]), " ",INT2STR(Circle.fill[1]), " ",INT2STR(Circle.fill[2])),   halfHight,0,_BkColor(),0,1);
	lenStr=LCD_Str(fontID_1, LCD_Xpos(lenStr,GetPos,0), LCD_Ypos(lenStr,IncPos,5), StrAll(6,"Backup:",INT2STR(Circle.bk[0]),   " ",INT2STR(Circle.bk[1]),   " ",INT2STR(Circle.bk[2])),   halfHight,0,_BkColor(),1,0);
	lenStr=LCD_Str(fontID_2, LCD_Xpos(lenStr,GetPos,0), LCD_Ypos(lenStr,IncPos,5), StrAll(4,"AA out:",Float2Str(Circle.ratioBk,' ',1,Sign_none,2),"  AA in:",Float2Str(Circle.ratioFill,' ',1,Sign_none,2)), 	 halfHight,0,_BkColor(),0,1);

	if(Circle.bold > Circle.width/2-1)
		Circle.bold= Circle.width/2-1;

	LCD_SetCircleAA(Circle.ratioBk,Circle.ratioFill);
	CorrectLineAA_on();
	StartMeasureTime_us();

/* --- Set points to draw own circle --- */
/*	LCD_SetCircleParam(0.01,0.01,14, 5,4,4,3,3,2,2,2,2,2, 2,2,1,1);
	LCD_Shape(60,160, LCD_Circle, 0,0, _FrameColor(), _FillColor(), _BkColor());	*/

	/* CIRCLE_POS_XY(Circle.width,10,10) */
	uint16_t calcWidth = LCD_CalculateCircleWidth(Circle.width);
	LCD_Shape(LCD_X-calcWidth-10, LCD_Y-calcWidth-10, LCD_Circle, SetParamWidthCircle(Percent_Circle,Circle.width),Circle.width, SetBold2Color(_FrameColor(),Circle.bold), _FillColor() /*TRANSPARENT*/, _BkColor());

	/* LCD_Circle(0,LCD_X,LCD_Y, LCD_X-calcWidth-10, LCD_Y-calcWidth-10, SetParamWidthCircle(Percent_Circle,Circle.width),Circle.width, SetBold2Color(_FrameColor(),Circle.bold), _FillColor(), _BkColor()); */

/*	LCD_Shape(LCD_X-LCD_CalculateCircleWidth(Circle.width)/2-10, LCD_Y-LCD_CalculateCircleWidth(Circle.width)-10 , LCD_HalfCircle, SetParamWidthCircle(Half_Circle_270,Circle.width),Circle.width, SetBold2Color(_FrameColor(),Circle.bold), _FillColor(), _BkColor()); */

	Circle.speed=StopMeasureTime_us("");

	lenStr=LCD_Str(fontID_2, LCD_Xpos(lenStr,GetPos,0), LCD_Ypos(lenStr,IncPos,8), StrAll(5,"Speed Max: ",INT2STR_TIME(Circle.speed)," us    Speed Sum: ",INT2STR_TIME(Circle.speedSum)," us"), 	 halfHight,0,_BkColor(),0,1);
	lenStr=LCD_Str(fontID_2, LCD_Xpos(lenStr,GetPos,0), LCD_Ypos(lenStr,IncPos,8), StrAll(2,"Bold: ",Int2Str(Circle.bold,' ',2,Sign_none)), halfHight,0,_BkColor(),0,1);
	LCD_Show();

/*	__Show_FrameAndCircle_Indirect(CIRCLE_POS_XY(Circle.width,10,20), Circle.width, Circle.bold);
	__Show_Circle_Indirect(LCD_GetXSize()-50,200, 50, 0);
	LCD_ShapeIndirect(400,350,LCD_Circle, 50,50, SetBold2Color(WHITE,0), RED, MYGRAY);	*/

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

	_DEBUG_RCV_CHAR("s",GET_CIRCLE_correctForWidth(),_uint16,_Incr,_Uint16(1),_Uint16(MAX_WIDTH_CIRCLE),"Width",NULL)
	_DEBUG_RCV_CHAR("x",GET_CIRCLE_correctForWidth(),_uint16,_Decr,_Uint16(1),_Uint16(1),"Width",NULL)

	_DEBUG_RCV_CHAR("d",GET_CIRCLE_correctPercDeg(0),_uint16,_Incr,_Uint16(5),_Uint16(95),"PercDeg",NULL)
	_DEBUG_RCV_CHAR("c",GET_CIRCLE_correctPercDeg(0),_uint16,_Decr,_Uint16(5),_Uint16(20),"PercDeg",NULL)

	_DEBUG_RCV_CHAR("f",GET_CIRCLE_errorDecision(0),_float,_Incr,_Float(0.1),_Float(3.0),"ErrDeci",NULL)
	_DEBUG_RCV_CHAR("v",GET_CIRCLE_errorDecision(0),_float,_Decr,_Float(0.1),_Float(0.0),"ErrDeci",NULL)

	else refresh_Screen=0;

	if(refresh_Screen) SCREEN_Test_Circle();
}
/* ########### --- END SCREEN_Test_Circle --- ############ */



void SCREEN_ReadPanel(void)  // przeniesc do Task.c !!!!!!!!!!!!!!!!!!
{
	if(startScreen==0)
	{
		switch(SCREEN_number)
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
		}
	}



	//if(IsRefreshScreenTimeout(refresh_10,2000)){ if(u==0) IncFontSize(); else DecFontSize();  startScreen++; if(startScreen>17){ startScreen=1; u=1-u; } }
	//Data2Refresh(PARAM_MOV_TXT);
//


	//if(IsRefreshScreenTimeout(refresh_7,40)) LCD_StrMovHIndirect_TEST(STR_ID_MovFonts_7,1);
	//if(IsRefreshScreenTimeout(refresh_8,50)) LCD_StrMovVI]]]]]]]\ndirect_TEST(STR_ID_MovFonts_8,1);




	if(DEBUG_RcvStr("=")){ startScreen=0; INCR_WRAP(SCREEN_number,1,0,5); }
	if(DEBUG_RcvStr("?")) LCD_DisplayStrMovBuffState();


}

/*
 Jak czytam zycie ludzi swietych kazdego dnia jak jest wspomnienie w kosciolacj to jest to heroizm, Aga mowi ze to luzie chorzy psychicznie i ma racje w 100% jesli myslimy wylacznie po ludzku

 Bo jak jest napisane "Głupstwem je st dla boga madrosc ludzka i odwrotnie dla ludzi madrosc boga"
 */

//zrobic zmiane TOUCH_RESOLUTION!!

