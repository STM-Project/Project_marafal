/*
 * Keyboard.c
 *
 *  Created on: Sep 8, 2024
 *      Author: maraf
 */

#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#include "Keyboard.h"
#include "touch.h"
#include "SCREEN_ReadPanel.h"
#include "common.h"
#include "math.h"
#include "timer.h"
#include "mini_printf.h"

#define FONT_COEFF	252

#define MAX_NUMBER_OPENED_KEYBOARD_SIMULTANEOUSLY		20
#define KEYBOARD_NONE	0
#define MAXNMB_widthKey	5

#define MAX_WIN_X		50
#define MAX_WIN_Y		100

#define MIDDLE_NR		1
#define GET_X(txt)	LCD_Xmiddle(MIDDLE_NR,GetPos,fontID,txt,0,NoConstWidth)
#define GET_Y			LCD_Ymiddle(MIDDLE_NR,GetPos,fontID)

typedef enum{
	KEBOARD_1,
	KEBOARD_2,
	KEBOARD_3,
	KEBOARD_4,
	KEBOARD_5,
	KEBOARD_6,
	KEBOARD_7,
	KEBOARD_8,
	KEBOARD_9,
}KEYBOARD_NUMBER;

typedef enum{
	DispYes,
	DispNo,
}DISPLAY_YES_NO;

static char 				 txtKey			   [MAX_WIN_Y] [MAX_WIN_X] = {0};
static COLORS_DEFINITION colorTxtKey		[MAX_WIN_Y] = {0};
static COLORS_DEFINITION colorTxtPressKey [MAX_WIN_Y] = {0};
static uint16_t dimKeys[2] = {0};

static struct KEYBOARD_SETTINGS{
	figureShape shape;
	uint8_t bold;
	uint16_t x;
	uint16_t y;
	uint16_t widthKey;
	uint16_t heightKey;
	uint16_t widthKey2;
	uint16_t heightKey2;
	uint16_t wKey[MAXNMB_widthKey];
	uint16_t hKey[MAXNMB_widthKey];
	uint8_t interSpace;
	uint8_t forTouchIdx;			/* touch for enter to keyboard */
	uint8_t startTouchIdx;
	uint8_t nmbTouch;
	uint8_t param;
	uint32_t param2;
} s[MAX_NUMBER_OPENED_KEYBOARD_SIMULTANEOUSLY]={0}, c={0};

static int fontID = 0;
static int fontID_descr	= 0;
static int colorDescr	= 0;
static int frameMainColor = 0;
static int fillMainColor = 0;
static int frameColor = 0;
static int fillColor = 0;
static int framePressColor = 0;
static int fillPressColor = 0;
static int bkColor = 0;

static int frameColor_c[5]={0}, fillColor_c[5]={0}, bkColor_c[5]={0};
static uint16_t widthAll = 0, widthAll_c = 0;
static uint16_t heightAll = 0, heightAll_c = 0;

static void _deleteAllTouchs(void){
	for(int j=0; j<MAX_NUMBER_OPENED_KEYBOARD_SIMULTANEOUSLY; ++j){
		for(int i=0; i<s[j].nmbTouch; ++i)
			LCD_TOUCH_DeleteSelectTouch(s[j].startTouchIdx+i);
	}
}
static void _deleteAllTouchParams(void){
	for(int j=0; j<MAX_NUMBER_OPENED_KEYBOARD_SIMULTANEOUSLY; ++j){
		s[j].forTouchIdx = NO_TOUCH;
		s[j].nmbTouch = 0;
	}
}
static void _deleteTouchs(int nr){
	for(int i=0; i<s[nr].nmbTouch; ++i)
		LCD_TOUCH_DeleteSelectTouch(s[nr].startTouchIdx+i);
}
static void _deleteTouchParams(int nr){
	s[nr].forTouchIdx = NO_TOUCH;
	s[nr].nmbTouch = 0;
}
static int GetHeightHead(int nr){
	return s[nr].interSpace + LCD_GetFontHeight(fontID_descr) + s[nr].interSpace;
}
static int GetHeightFontDescr(void){
	return LCD_GetFontHeight(fontID_descr);
}
static int GetHeightFont(int IdFont){
	return LCD_GetFontHeight(IdFont);
}
static int GetStrPxlWidth(int IdFont, char* str, int constWidth){
	return LCD_GetWholeStrPxlWidth(IdFont,str,0,constWidth);
}
static void Str(const char *txt, uint32_t color){
	LCD_StrDependOnColorsWindow(0,widthAll,heightAll,fontID, GET_X((char*)txt),GET_Y,(char*)txt, fullHight, 0, fillColor, color,FONT_COEFF, NoConstWidth);
}
static void StrDescr_XYoffs(XY_Touch_Struct pos,int offsX,int offsY, const char *txt, uint32_t color){
	LCD_StrDependOnColorsWindow(0,widthAll,heightAll,fontID_descr, pos.x+offsX, pos.y+offsY,(char*)txt, fullHight, 0, fillColor, color,FONT_COEFF, NoConstWidth);
}
static void StrDescrWin_XYoffs(int nr,int offsX,int offsY, const char *txt, uint32_t color){
	LCD_StrDependOnColorsWindow(0,s[nr].widthKey,s[nr].heightKey,fontID_descr, offsX,offsY,(char*)txt, fullHight, 0, fillColor, color,FONT_COEFF, NoConstWidth);
}
static void Str_Xmidd_Yoffs(int nr,XY_Touch_Struct pos,int offsY, const char *txt, uint32_t color){
	LCD_Xmiddle(MIDDLE_NR,SetPos,SetPosAndWidth(pos.x,s[nr].widthKey),NULL,0,NoConstWidth);
	LCD_StrDependOnColorsWindow(0,widthAll,heightAll,fontID, GET_X((char*)txt), pos.y+offsY,(char*)txt, fullHight, 0, fillColor, color,FONT_COEFF, NoConstWidth);
}
static void StrWin_Xmidd_Yoffs(int nr,int offsY, const char *txt, uint32_t color){
	LCD_Xmiddle(MIDDLE_NR,SetPos,SetPosAndWidth(0,s[nr].widthKey),NULL,0,NoConstWidth);
	LCD_StrDependOnColorsWindow(0,s[nr].widthKey, s[nr].heightKey,fontID, GET_X((char*)txt),offsY,(char*)txt, fullHight, 0, fillColor, color,FONT_COEFF, NoConstWidth);
}
static void StrLeft(int nr, const char *txt, XY_Touch_Struct pos, uint32_t color){
	LCD_Ymiddle(MIDDLE_NR,SetPos,SetPosAndWidth(pos.y,s[nr].heightKey));
	LCD_StrDependOnColorsWindow(0,widthAll,heightAll,fontID, pos.x+LCD_GetFontHeight(fontID)/2, GET_Y, (char*)txt, fullHight, 0, fillColor, color,FONT_COEFF, NoConstWidth);
}
static void StrDescr_Xmidd_Yoffs(XY_Touch_Struct pos,int offsY, const char *txt, uint32_t color){  //to usunac a nizej jest lepsze i zawiera sie w tym
	LCD_Xmiddle(MIDDLE_NR+1,SetPos,SetPosAndWidth(0,widthAll),NULL,0,NoConstWidth);
	LCD_StrDependOnColorsWindow(0,widthAll,heightAll,fontID_descr,		LCD_Xmiddle(MIDDLE_NR+1,GetPos,fontID_descr,(char*)txt,0,NoConstWidth),	pos.y+offsY, 	(char*)txt, fullHight, 0, bkColor, color,FONT_COEFF, NoConstWidth);
}
static StructFieldPos StrDescr(int nr,XY_Touch_Struct pos, const char *txt, uint32_t color){
	StructFieldPos field = {0};
	LCD_Xmiddle(MIDDLE_NR+1,SetPos,SetPosAndWidth(pos.x,widthAll),NULL,0,NoConstWidth);
	field.len = LCD_StrDependOnColorsWindow(0,widthAll,heightAll,fontID_descr,		field.x=LCD_Xmiddle(MIDDLE_NR+1,GetPos,fontID_descr,(char*)txt,0,NoConstWidth),		field.y=s[nr].interSpace+pos.y,		(char*)txt, fullHight, 0, bkColor, color,FONT_COEFF, NoConstWidth);
	field.width =  field.len.inPixel;
	field.height = field.len.height;
	return field;
}
static void StrPress(const char *txt, uint32_t color){
	LCD_StrDependOnColorsWindow(0,widthAll,heightAll,fontID, GET_X((char*)txt),GET_Y,(char*)txt, fullHight, 0, fillPressColor, color,FONT_COEFF, NoConstWidth);
}
static void StrPressLeft(int nr,const char *txt, XY_Touch_Struct pos, uint32_t color){
	LCD_Ymiddle(MIDDLE_NR,SetPos,SetPosAndWidth(pos.y,s[nr].heightKey));
	LCD_StrDependOnColorsWindow(0,widthAll,heightAll,fontID, pos.x+10,GET_Y,(char*)txt, fullHight, 0, fillPressColor, color,FONT_COEFF, NoConstWidth);
}
static void StrDisp(int nr,const char *txt, uint32_t color){
	LCD_StrDependOnColorsWindowIndirect(0, s[nr].x, s[nr].y, widthAll,heightAll,fontID, GET_X((char*)txt),GET_Y,(char*)txt, fullHight, 0, fillColor, color,FONT_COEFF, NoConstWidth);
}
static void StrPressDisp(int nr,const char *txt, uint32_t color){
	LCD_StrDependOnColorsWindowIndirect(0, s[nr].x, s[nr].y, widthAll,heightAll,fontID, GET_X((char*)txt),GET_Y,(char*)txt, fullHight, 0, fillPressColor, color,FONT_COEFF, NoConstWidth);
}

static void TxtPos(int nr,XY_Touch_Struct pos){
	LCD_Xmiddle(MIDDLE_NR,SetPos,SetPosAndWidth(pos.x,s[nr].widthKey),NULL,0,NoConstWidth);
	LCD_Ymiddle(MIDDLE_NR,SetPos,SetPosAndWidth(pos.y,s[nr].heightKey));
}
static void _TxtPos(int nr,XY_Touch_Struct pos, int nrWH){
	LCD_Xmiddle(MIDDLE_NR,SetPos,SetPosAndWidth(pos.x, s[nr].wKey[nrWH]),NULL,0,NoConstWidth);
	LCD_Ymiddle(MIDDLE_NR,SetPos,SetPosAndWidth(pos.y, s[nr].hKey[nrWH]));
}

static void Key(int nr,XY_Touch_Struct pos){
	LCD_ShapeWindow( s[nr].shape, 0, widthAll,heightAll, pos.x,pos.y, s[nr].widthKey, s[nr].heightKey, SetBold2Color(frameColor,s[nr].bold),fillColor,bkColor);
}
static void _Key(int nr,XY_Touch_Struct pos,int nrWH){
	LCD_ShapeWindow( s[nr].shape, 0, widthAll,heightAll, pos.x,pos.y, s[nr].wKey[nrWH], s[nr].hKey[nrWH], SetBold2Color(frameColor,s[nr].bold),fillColor,bkColor);
}

static void KeyPressWin(int nr){
	LCD_ShapeWindow( s[nr].shape, 0, s[nr].widthKey,s[nr].heightKey, 0,0, s[nr].widthKey, s[nr].heightKey, SetBold2Color(framePressColor,s[nr].bold),fillPressColor,bkColor);
}

static void KeyStr(int nr,XY_Touch_Struct pos,const char *txt, uint32_t color){
	Key(nr,pos);
	TxtPos(nr,pos);
	Str(txt,color);
}
static void _KeyStr(int nr,XY_Touch_Struct pos,const char *txt, uint32_t color, int nrWH){
	_Key(nr,pos,nrWH);
	_TxtPos(nr,pos,nrWH);
	Str(txt,color);
}

static void KeyStrleft(int nr,XY_Touch_Struct pos,const char *txt, uint32_t color){
	Key(nr,pos);
	StrLeft(nr,txt,pos,color);
}

static void StrWinKeyMidd(int nr, const char *txt, uint32_t color, int idFont, int constWidth){
	LCD_Xmiddle(MIDDLE_NR,SetPos,SetPosAndWidth(0,s[nr].widthKey),NULL,0,constWidth);
	LCD_Ymiddle(MIDDLE_NR,SetPos,SetPosAndWidth(0,s[nr].heightKey));
	LCD_StrDependOnColorsWindow(0,s[nr].widthKey,s[nr].heightKey,idFont, LCD_Xmiddle(MIDDLE_NR,GetPos,idFont,(char*)txt,0,constWidth), LCD_Ymiddle(MIDDLE_NR,GetPos,idFont), (char*)txt, fullHight, 0, fillColor, color,FONT_COEFF, constWidth);
}
static void StrKeyMidd(int nr, XY_Touch_Struct pos, const char *txt, uint32_t color, int idFont, int constWidth){
	LCD_Xmiddle(MIDDLE_NR,SetPos,SetPosAndWidth(pos.x,s[nr].widthKey),NULL,0,constWidth);
	LCD_Ymiddle(MIDDLE_NR,SetPos,SetPosAndWidth(pos.y,s[nr].heightKey));
	LCD_StrDependOnColorsWindow(0,widthAll,heightAll,idFont, LCD_Xmiddle(MIDDLE_NR,GetPos,idFont,(char*)txt,0,constWidth), LCD_Ymiddle(MIDDLE_NR,GetPos,idFont), (char*)txt, fullHight, 0, fillColor, color,FONT_COEFF, constWidth);
}

static void KeyStrDisp(int nr,XY_Touch_Struct pos,const char *txt, uint32_t color){
	Key(nr, pos);
	TxtPos(nr,pos);
	StrDisp(nr,txt,color);
}
static void _KeyStrDisp(int nr,XY_Touch_Struct pos,const char *txt, uint32_t color, int nrWH){
	_Key(nr,pos,nrWH);
	_TxtPos(nr,pos,nrWH);
	StrDisp(nr,txt,color);
}

static void KeyPress(int nr, XY_Touch_Struct pos){
	LCD_ShapeWindow( s[nr].shape, 0, widthAll,heightAll, pos.x,pos.y, s[nr].widthKey,s[nr].heightKey, SetBold2Color(framePressColor,s[nr].bold),fillPressColor,bkColor);
}
static void _KeyPress(int nr, XY_Touch_Struct pos, int nrWH){
	LCD_ShapeWindow( s[nr].shape, 0, widthAll,heightAll, pos.x,pos.y, s[nr].wKey[nrWH],s[nr].hKey[nrWH], SetBold2Color(framePressColor,s[nr].bold),fillPressColor,bkColor);
}

static void KeyStrPress(int nr, XY_Touch_Struct pos, const char *txt, uint32_t colorTxt){
	KeyPress(nr,pos);
	TxtPos(nr,pos);
	StrPress(txt,colorTxt);
}
static void _KeyStrPress(int nr, XY_Touch_Struct pos, const char *txt, uint32_t colorTxt, int nrWH){
	_KeyPress(nr,pos,nrWH);
	_TxtPos(nr,pos,nrWH);
	StrPress(txt,colorTxt);
}

static void KeyStrPressLeft(int nr, XY_Touch_Struct pos, const char *txt, uint32_t colorTxt){
	KeyPress(nr,pos);
	StrPressLeft(nr,txt,pos,colorTxt);
}

static void KeyStrPressDisp(int nr, XY_Touch_Struct pos, const char *txt, uint32_t colorTxt){
	KeyPress(nr,pos);
	TxtPos(nr,pos);
	StrPressDisp(nr,txt,colorTxt);
}
static void _KeyStrPressDisp(int nr, XY_Touch_Struct pos, const char *txt, uint32_t colorTxt, int nrWH){
	_KeyPress(nr,pos,nrWH);
	_TxtPos(nr,pos,nrWH);
	StrPressDisp(nr,txt,colorTxt);
}

static void KeyStrPressDisp_oneBlock(int nr, XY_Touch_Struct pos, const char *txt, uint32_t colorTxt){
	LCD_ShapeWindow( s[nr].shape, 0, s[nr].widthKey,s[nr].heightKey, 0,0, s[nr].widthKey,s[nr].heightKey, SetBold2Color(framePressColor,s[nr].bold),fillPressColor,bkColor);
	TxtPos(nr,(XY_Touch_Struct){0});
	LCD_StrDependOnColorsWindowIndirect(0, s[nr].x+pos.x, s[nr].y+pos.y, s[nr].widthKey, s[nr].heightKey,fontID, GET_X((char*)txt),GET_Y,(char*)txt, fullHight, 0, fillPressColor, colorTxt,FONT_COEFF, NoConstWidth);
}
static void _KeyStrPressDisp_oneBlock(int nr, XY_Touch_Struct pos, const char *txt, uint32_t colorTxt, int nrWH){
	LCD_ShapeWindow( s[nr].shape, 0, s[nr].wKey[nrWH], s[nr].hKey[nrWH], 0,0, s[nr].wKey[nrWH], s[nr].hKey[nrWH], SetBold2Color(framePressColor,s[nr].bold),fillPressColor,bkColor);
	_TxtPos(nr,(XY_Touch_Struct){0},nrWH);
	LCD_StrDependOnColorsWindowIndirect(0, s[nr].x+pos.x, s[nr].y+pos.y, s[nr].wKey[nrWH], s[nr].hKey[nrWH],fontID, GET_X((char*)txt),GET_Y,(char*)txt, fullHight, 0, fillPressColor, colorTxt,FONT_COEFF, NoConstWidth);
}
static void KeyStrPressDisp_oneKey(int nr, XY_Touch_Struct pos, int nrTab){
	KeyStrPressDisp_oneBlock(nr, pos, txtKey[nrTab], colorTxtPressKey[nrTab]);
}
static void _KeyStrPressDisp_oneKey(int nr, XY_Touch_Struct pos, int nrTab, char *txtKeys[], uint32_t colorTxtPressKeys[], int nrWH){
	_KeyStrPressDisp_oneBlock(nr, pos, txtKeys[nrTab], colorTxtPressKeys[nrTab],nrWH);
}

static void TxtDescrMidd_WidthKey(int nr, XY_Touch_Struct pos, char *txtKeys, uint32_t colorDescr){
	LCD_StrDependOnColorsWindow(0,widthAll,heightAll,fontID_descr,		MIDDLE(pos.x, s[nr].widthKey, LCD_GetWholeStrPxlWidth(fontID_descr,txtKeys,0,NoConstWidth)),   pos.y-GetHeightFontDescr(), 	 txtKeys, fullHight, 0, bkColor, colorDescr,FONT_COEFF, NoConstWidth);
}
static void ShapeBkClear(int nr, int width_all, int height_all, uint32_t bkColor){
	LCD_ShapeWindow( s[nr].shape,0,width_all,height_all, 0,0, width_all,height_all, bkColor, bkColor,bkColor );
}
static StructFieldPos TxtDescr(int nr, uint16_t xPos, uint16_t yPos, char* txtDescr){
	XY_Touch_Struct posHead = {xPos,yPos};
	return StrDescr(nr, posHead, txtDescr, colorDescr);
}
static void TxtDescrMidd(int nr, int head, char* txtDescr){
	LCD_StrDependOnColorsWindow(0,widthAll,heightAll,fontID_descr,		MIDDLE(0,widthAll,LCD_GetWholeStrPxlWidth(fontID_descr,txtDescr,0,NoConstWidth)),  MIDDLE(0,head,GetHeightFontDescr()), 	 txtDescr, fullHight, 0, bkColor, colorDescr,FONT_COEFF, NoConstWidth);
}
static void ShapeWin(int nr, int width_all, int height_all){
	LCD_ShapeWindow( s[nr].shape,0,width_all,height_all, 0,0, width_all,height_all, SetBold2Color(frameMainColor,s[nr].bold), fillMainColor,bkColor );
}

static int GetPosKeySize(void){
	int countKey = dimKeys[0]*dimKeys[1];		if(countKey > MAX_WIN_Y-1)  countKey= MAX_WIN_Y;
	return countKey;
}
static int _GetPosKeySize(uint16_t dimKey[]){
	int countKey = dimKey[0]*dimKey[1];		if(countKey > MAX_WIN_Y-1)  countKey= MAX_WIN_Y;
	return countKey;
}

static void KeyShapePressDisp_oneBlock(int nr, XY_Touch_Struct pos, ShapeFunc pShape, SHAPE_PARAMS param){
	LCD_ShapeWindow( s[nr].shape, 0, s[nr].widthKey,s[nr].heightKey, 0,0, s[nr].widthKey,s[nr].heightKey, SetBold2Color(framePressColor,s[nr].bold),fillPressColor,bkColor);
	pShape(0,param);
	LCD_Display(0, s[nr].x+pos.x, s[nr].y+pos.y, s[nr].widthKey, s[nr].heightKey);
}

static void SetTouchSlider(int nr,uint16_t idx, SHAPE_PARAMS posElemSlider){
	for(int i=0; i<NMB_SLIDER_ELEMENTS; ++i){
		touchTemp[0].x= s[nr].x + posElemSlider.pos[i].x;
		touchTemp[1].x= touchTemp[0].x + CONDITION(Horizontal==s[nr].param, posElemSlider.size[i].w, posElemSlider.size[i].h);
		touchTemp[0].y= s[nr].y + posElemSlider.pos[i].y;
		touchTemp[1].y= touchTemp[0].y + CONDITION(Horizontal==s[nr].param, posElemSlider.size[i].h, posElemSlider.size[i].w);

		switch(i){ case 0: case 2: LCD_TOUCH_Set(ID_TOUCH_GET_ANY_POINT_WITH_WAIT, idx + s[nr].nmbTouch++, TOUCH_GET_PER_X_PROBE   ); break;
					  case 1:			LCD_TOUCH_Set(ID_TOUCH_GET_ANY_POINT,				idx + s[nr].nmbTouch++, TOUCH_GET_PER_ANY_PROBE ); break; }
	}
}
static void KeysAllRelease_Slider(int nr, XY_Touch_Struct posKeys[],int *value, int maxSliderValue, uint32_t lineUnSelColor, uint32_t spaceTriangLine, SHAPE_PARAMS *elemSliderPos){
	for(int i=0; i<dimKeys[0]*dimKeys[1]; ++i){
		if		 (Vertical == s[nr].param)
			elemSliderPos[i] = LCD_SimpleSliderV(0, widthAll,heightAll, posKeys[i].x, posKeys[i].y, ChangeElemSliderSize(s[nr].heightKey,NORMAL_SLIDER_PARAM), SetSpaceTriangLineSlider(s[nr].widthKey,spaceTriangLine), colorTxtKey[i], CONDITION(0==lineUnSelColor,colorTxtKey[i],lineUnSelColor), colorTxtPressKey[i], bkColor, SetValType(PERCENT_SCALE(*(value+i)+1,maxSliderValue+1),Percent), NoSel);
		else if(Horizontal == s[nr].param)
			elemSliderPos[i] = LCD_SimpleSliderH(0, widthAll,heightAll, posKeys[i].x, posKeys[i].y, ChangeElemSliderSize(s[nr].widthKey,NORMAL_SLIDER_PARAM), SetSpaceTriangLineSlider(s[nr].heightKey,spaceTriangLine), colorTxtKey[i], CONDITION(0==lineUnSelColor,colorTxtKey[i],lineUnSelColor), colorTxtPressKey[i], bkColor, SetValType(PERCENT_SCALE(*(value+i)+1,maxSliderValue+1),Percent), NoSel);
		TxtDescrMidd_WidthKey(nr, posKeys[i], txtKey[i], colorTxtPressKey[i]);
	}
	LCD_Display(0, s[nr].x, s[nr].y, widthAll, heightAll);
}

static void _KeysAllRelease_Slider(int nr, XY_Touch_Struct posKeys[],int *value, int maxSliderValue, uint32_t lineUnSelColor, uint32_t spaceTriangLine, SHAPE_PARAMS *elemSliderPos, \
			uint16_t dimKey[], char *txtKeys[], uint32_t colorTxtKeys[], uint32_t colorTxtPressKeys[], int nrWH, DISPLAY_YES_NO disp){
	for(int i=0; i<dimKey[0]*dimKey[1]; ++i){
		if		 (Vertical == s[nr].param)
			elemSliderPos[i] = LCD_SimpleSliderV(0, widthAll,heightAll, posKeys[i].x, posKeys[i].y, ChangeElemSliderSize(s[nr].hKey[nrWH],NORMAL_SLIDER_PARAM), SetSpaceTriangLineSlider(s[nr].wKey[nrWH],spaceTriangLine), colorTxtKeys[i], CONDITION(0==lineUnSelColor,colorTxtKeys[i],lineUnSelColor), colorTxtPressKeys[i], bkColor, SetValType(PERCENT_SCALE(*(value+i)+1,maxSliderValue+1),Percent), NoSel);
		else if(Horizontal == s[nr].param)
			elemSliderPos[i] = LCD_SimpleSliderH(0, widthAll,heightAll, posKeys[i].x, posKeys[i].y, ChangeElemSliderSize(s[nr].wKey[nrWH],NORMAL_SLIDER_PARAM), SetSpaceTriangLineSlider(s[nr].hKey[nrWH],spaceTriangLine), colorTxtKeys[i], CONDITION(0==lineUnSelColor,colorTxtKeys[i],lineUnSelColor), colorTxtPressKeys[i], bkColor, SetValType(PERCENT_SCALE(*(value+i)+1,maxSliderValue+1),Percent), NoSel);
		TxtDescrMidd_WidthKey(nr, posKeys[i], txtKeys[i], colorTxtPressKeys[i]);
	}
	if(DispYes==disp) LCD_Display(0, s[nr].x, s[nr].y, widthAll, heightAll);
}

static void ElemSliderPressDisp_oneBlock(int nr, uint16_t x,uint16_t y, XY_Touch_Struct posSlider, uint32_t spaceTringLine, int selElem, uint32_t lineColor, uint32_t lineSelColor, int *pVal, int valType, int maxSlidVal, VOID_FUNCTION *pfunc){
	int value = 0;
	SHAPE_PARAMS slid = {0};
	if		 (Vertical == s[nr].param){
		switch(valType){
			case PosXY:		value = SetValType(CONDITION(0>y-s[nr].y-posSlider.y, 0, y-s[nr].y-posSlider.y), PosXY);  break;
			case Percent:	value = SetValType(PERCENT_SCALE(*pVal+1,maxSlidVal+1),Percent);  			  				   break;
		}
		LCD_ShapeWindow(LCD_Rectangle,0,s[nr].widthKey,s[nr].heightKey, 0,0, s[nr].widthKey,s[nr].heightKey, bkColor, bkColor,bkColor);
		slid = LCD_SimpleSliderV(0, s[nr].widthKey, s[nr].heightKey, 0,0, ChangeElemSliderSize(s[nr].heightKey,NORMAL_SLIDER_PARAM), SetSpaceTriangLineSlider(s[nr].widthKey,spaceTringLine), lineColor, lineSelColor ,selElem|0xFF000000, bkColor, value, selElem);
	}
	else if(Horizontal == s[nr].param){
		switch(valType){
			case PosXY:		value = SetValType(CONDITION(0>x-s[nr].x-posSlider.x, 0, x-s[nr].x-posSlider.x), PosXY);  break;
			case Percent:	value = SetValType(PERCENT_SCALE(*pVal+1,maxSlidVal+1),Percent);  			  				   break;
		}
		LCD_ShapeWindow(LCD_Rectangle,0,s[nr].widthKey,s[nr].heightKey, 0,0, s[nr].widthKey,s[nr].heightKey, bkColor, bkColor,bkColor);
		slid = LCD_SimpleSliderH(0, s[nr].widthKey, s[nr].heightKey, 0,0, ChangeElemSliderSize(s[nr].widthKey,NORMAL_SLIDER_PARAM), SetSpaceTriangLineSlider(s[nr].heightKey,spaceTringLine), lineColor, lineSelColor ,selElem|0xFF000000, bkColor, value, selElem);
	}
	LCD_Display(0, s[nr].x+posSlider.x, s[nr].y+posSlider.y, s[nr].widthKey, s[nr].heightKey);
	if(PtrSel==SHIFT_RIGHT(selElem,24,F)){
		*pVal = SET_NEW_RANGE( ((maxSlidVal+1)*slid.param[0])/slid.param[1], slid.param[2],maxSlidVal-slid.param[2], 0,maxSlidVal );
		if(pfunc) pfunc();
	}
}

static void _ElemSliderPressDisp_oneBlock(int nr, uint16_t x,uint16_t y, XY_Touch_Struct posSlider, uint32_t spaceTringLine, int selElem, uint32_t lineColor, uint32_t lineSelColor, int *pVal, int valType, int maxSlidVal, VOID_FUNCTION *pfunc, int nrWH){
	int value = 0;
	SHAPE_PARAMS slid = {0};
	if		 (Vertical == s[nr].param){
		switch(valType){
			case PosXY:		value = SetValType(CONDITION(0>y-s[nr].y-posSlider.y, 0, y-s[nr].y-posSlider.y), PosXY);  break;
			case Percent:	value = SetValType(PERCENT_SCALE(*pVal+1,maxSlidVal+1),Percent);  			  				  break;
		}
		LCD_ShapeWindow(LCD_Rectangle,0,s[nr].wKey[nrWH],s[nr].hKey[nrWH], 0,0, s[nr].widthKey,s[nr].hKey[nrWH], bkColor, bkColor,bkColor);
		slid = LCD_SimpleSliderV(0, s[nr].wKey[nrWH], s[nr].hKey[nrWH], 0,0, ChangeElemSliderSize(s[nr].hKey[nrWH],NORMAL_SLIDER_PARAM), SetSpaceTriangLineSlider(s[nr].wKey[nrWH],spaceTringLine), lineColor, lineSelColor ,selElem|0xFF000000, bkColor, value, selElem);
	}
	else if(Horizontal == s[nr].param){
		switch(valType){
			case PosXY:		value = SetValType(CONDITION(0>x-s[nr].x-posSlider.x, 0, x-s[nr].x-posSlider.x), PosXY);  break;
			case Percent:	value = SetValType(PERCENT_SCALE(*pVal+1,maxSlidVal+1),Percent);  			  				  break;
		}
		LCD_ShapeWindow(LCD_Rectangle,0,s[nr].wKey[nrWH],s[nr].hKey[nrWH], 0,0, s[nr].widthKey,s[nr].hKey[nrWH], bkColor, bkColor,bkColor);
		slid = LCD_SimpleSliderH(0, s[nr].wKey[nrWH], s[nr].hKey[nrWH], 0,0, ChangeElemSliderSize(s[nr].wKey[nrWH],NORMAL_SLIDER_PARAM), SetSpaceTriangLineSlider(s[nr].hKey[nrWH],spaceTringLine), lineColor, lineSelColor ,selElem|0xFF000000, bkColor, value, selElem);
	}
	LCD_Display(0, s[nr].x+posSlider.x, s[nr].y+posSlider.y, s[nr].wKey[nrWH], s[nr].hKey[nrWH]);
	if(PtrSel==SHIFT_RIGHT(selElem,24,F))
		*pVal = SET_NEW_RANGE( ((maxSlidVal+1)*slid.param[0])/slid.param[1], slid.param[2],maxSlidVal-slid.param[2], 0,maxSlidVal );
	if(pfunc) pfunc();
}

static void KeysAllRelease_CircleSlider(int nr, XY_Touch_Struct posKeys[],int *value){
	XY_Touch_Struct posK;
	uint16_t _GetDegFromVal(int val){
		return ((360*val)/255);
	}
	for(int i=0; i<dimKeys[0]*dimKeys[1]; ++i)
	{
		uint16_t deg[2] = {0, _GetDegFromVal(*(value+i)) };
		uint32_t degColor[2] = {0, CONDITION(deg[1]==deg[0],fillColor,colorTxtPressKey[i]) };

		posK = posKeys[i];	posK.y -= VALPERC(GetHeightFontDescr(),40);		TxtDescrMidd_WidthKey(nr, posK, txtKey[i], colorTxtPressKey[i]);
		LCD_SetCirclePercentParam(2,deg,(uint32_t*)degColor);
		LCD_Circle(0, widthAll,heightAll, posKeys[i].x, posKeys[i].y, SetParamWidthCircle(Percent_Circle,s[nr].widthKey),s[nr].heightKey, SetBold2Color(frameColor,s[nr].bold), fillColor, bkColor);

		char *pTxt= Int2Str(*(value+i),Zero,3,Sign_none);
		if(IS_RANGE(s[nr].bold, 1, (LCD_GetCircleWidth()-GetStrPxlWidth(fontID_descr, StrAll(3," ",pTxt," "), ConstWidth))/2) )
			StrKeyMidd(nr, posKeys[i], pTxt, BrightIncr(colorDescr,0x30), fontID_descr,ConstWidth);
	}
	LCD_Display(0, s[nr].x, s[nr].y, widthAll, heightAll);
}
static void KeyPress_CircleSlider(int nr, uint16_t x,uint16_t y, XY_Touch_Struct posKeys, float radius, int *value, VOID_FUNCTION *pfunc, uint32_t colorPress)
{
	uint32_t _GetPosX(void){	return x-(s[nr].x+posKeys.x); }
	uint32_t _GetPosY(void){	return y-(s[nr].y+posKeys.y); }
	uint16_t _GetValFromDeg(uint16_t deg){
		return ((255*deg)/360);
	}
	uint16_t _GetDegFromPosX(void){
		uint16_t deg=0;
		float radi = sqrt( (radius-(float)_GetPosX())*(radius-(float)_GetPosX()) + (radius-(float)_GetPosY())*(radius-(float)_GetPosY()) );
		float stretch = radius-(float)_GetPosX();
		deg = (uint16_t)(57.295*acos(stretch/radi));
		if(_GetPosY() > (uint16_t)radius)
			deg = 360 - deg;
		return deg;
	}
	INIT(xPosFromMidd_Pow2, ((int)radius-(int)_GetPosX()) );		xPosFromMidd_Pow2 *= xPosFromMidd_Pow2;
	INIT(yPosFromMidd_Pow2, ((int)radius-(int)_GetPosY()) );		yPosFromMidd_Pow2 *= yPosFromMidd_Pow2;
	INIT(radiusTouch_Pow2, xPosFromMidd_Pow2 + yPosFromMidd_Pow2 );

	if( IS_RANGE(radiusTouch_Pow2, (int)(VALPERC(radius,45)*VALPERC(radius,45)), (int)(radius*radius)) )
	{
		int degPosX = _GetDegFromPosX();
		uint16_t deg[2] = {0, degPosX};
		uint32_t degColor[2] = {0,CONDITION(deg[1]==deg[0],fillColor,colorPress)};

		ShapeBkClear(nr, s[nr].widthKey,s[nr].heightKey, bkColor);
		*value = _GetValFromDeg(degPosX);

		LCD_SetCirclePercentParam(2,deg,(uint32_t*)degColor);
		LCD_Circle(0, s[nr].widthKey,s[nr].heightKey, 0,0, SetParamWidthCircle(Percent_Circle,s[nr].widthKey),s[nr].heightKey, SetBold2Color(frameColor,s[nr].bold), fillColor, bkColor);

		char *pTxt= Int2Str(*value,Zero,3,Sign_none);
		if(IS_RANGE(s[nr].bold, 1, (LCD_GetCircleWidth()-GetStrPxlWidth(fontID_descr, StrAll(3," ",pTxt," "), ConstWidth))/2) )
			StrWinKeyMidd(nr, pTxt, BrightIncr(colorDescr,0x30), fontID_descr,ConstWidth);

		LCD_Display(0, s[nr].x+posKeys.x, s[nr].y+posKeys.y, s[nr].widthKey, s[nr].heightKey);
		if(pfunc) pfunc();
	}
}

static void SetTouch_Slider(int nr, uint16_t startTouchIdx, SHAPE_PARAMS *elemSliderPos){
	if(startTouchIdx){
		for(int i=0; i<dimKeys[0]*dimKeys[1]; ++i)
			SetTouchSlider(nr,s[nr].startTouchIdx, elemSliderPos[i]);
	}
}
static void _SetTouch_Slider(int nr, uint16_t startTouchIdx, SHAPE_PARAMS *elemSliderPos, uint16_t dimKey[], int nrWH){
	uint8_t touchCnt = s[nr].nmbTouch;
	if(startTouchIdx){
		for(int i=0; i<dimKey[0]*dimKey[1]; ++i)
			SetTouchSlider(nr, s[nr].startTouchIdx + touchCnt, elemSliderPos[i]);
	}
}

static void SetTouch(int nr, uint16_t ID, uint16_t idx, uint8_t paramTouch, XY_Touch_Struct pos){
	touchTemp[0].x= s[nr].x + pos.x;
	touchTemp[1].x= touchTemp[0].x + s[nr].widthKey;
	touchTemp[0].y= s[nr].y + pos.y;
	touchTemp[1].y= touchTemp[0].y + s[nr].heightKey;
	LCD_TOUCH_Set(ID,idx,paramTouch);
	s[nr].nmbTouch++;
}
static void _SetTouch(int nr, uint16_t ID, uint16_t idx, uint8_t paramTouch, XY_Touch_Struct pos, int nrWH){
	touchTemp[0].x= s[nr].x + pos.x;
	touchTemp[1].x= touchTemp[0].x + s[nr].wKey[nrWH];
	touchTemp[0].y= s[nr].y + pos.y;
	touchTemp[1].y= touchTemp[0].y + s[nr].hKey[nrWH];
	LCD_TOUCH_Set(ID,idx,paramTouch);
	s[nr].nmbTouch++;
}

static void SetPosKey(int nr, XY_Touch_Struct pos[], int interSpace, int head){
	int d=0;
	for(int j=0; j<dimKeys[1]; ++j){
		for(int i=0; i<dimKeys[0]; ++i){
			pos[d].x = s[nr].interSpace + i*(s[nr].widthKey  + interSpace);
			pos[d].y = s[nr].interSpace + j*(s[nr].heightKey + interSpace) + head;
			d++;
	}}
}
static void _SetPosKey(int nr, XY_Touch_Struct pos[], int interSpace, int head, uint16_t dimKey[], int offsX, int nrWH){
	int d=0;
	for(int j=0; j<dimKey[1]; ++j){
		for(int i=0; i<dimKey[0]; ++i){
			pos[d].x = s[nr].interSpace + i*(s[nr].wKey[nrWH] + interSpace) + offsX;
			pos[d].y = s[nr].interSpace + j*(s[nr].hKey[nrWH] + interSpace) + head;
			d++;
	}}
}

static void SetDimKey(int nr, figureShape shape, uint16_t width, uint16_t height){
	if(0!=shape && KeysAutoSize==width){
		int count = dimKeys[0]*dimKeys[1];
		int tab[count];

		for(int i=0; i<count; ++i)
			tab[i] = LCD_GetWholeStrPxlWidth(fontID,(char*)txtKey[i],0,NoConstWidth);
		INIT_MAXVAL(tab,STRUCT_TAB_SIZE(tab),0,maxVal);

		s[nr].widthKey =  height + maxVal + height;		/*	space + text + space */
		s[nr].heightKey = height + LCD_GetFontHeight(fontID) + height;
	}
}
static void _SetDimKey(int nr, figureShape shape, uint16_t width, uint16_t height, uint16_t dimKey[], char *txtKeys[], int nrWH){
	if(0!=shape){
		if(KeysAutoSize==width){
			int count = dimKey[0]*dimKey[1];
			int tab[count];

			for(int i=0; i<count; ++i)
				tab[i] = LCD_GetWholeStrPxlWidth(fontID,txtKeys[i],0,NoConstWidth);
			INIT_MAXVAL(tab,STRUCT_TAB_SIZE(tab),0,maxVal);

			s[nr].wKey[nrWH] = height + maxVal + height;		/*	space + text + space */
			s[nr].hKey[nrWH] = height + LCD_GetFontHeight(fontID) + height;
		}
		else{
			s[nr].wKey[nrWH] = width;
			s[nr].hKey[nrWH] = height;
	}}
}
static void _SetDimKey_slider(int nr, figureShape shape, uint16_t width, uint16_t height, int nrWH){
	if(0!=shape){
		s[nr].wKey[nrWH] = width;
		s[nr].hKey[nrWH] = height;
	}
}

static void SetDimAll(int nr, int interSpace, int head){
	widthAll =  s[nr].interSpace + dimKeys[0]*s[nr].widthKey  + (dimKeys[0]-1)*interSpace + s[nr].interSpace;
	heightAll = s[nr].interSpace + dimKeys[1]*s[nr].heightKey + (dimKeys[1]-1)*interSpace + s[nr].interSpace + head;
}
static structSize _SetDimAll(int nr, int interSpace, int head, uint16_t dimKey[], int nrWH){
	structSize temp = { s[nr].interSpace + dimKey[0]*s[nr].wKey[nrWH] + (dimKey[0]-1)*interSpace + s[nr].interSpace,\
							  s[nr].interSpace + dimKey[1]*s[nr].hKey[nrWH] + (dimKey[1]-1)*interSpace + s[nr].interSpace + head};
	return temp;
}

static void KeysAllRelease(int nr, XY_Touch_Struct posKeys[]){
	int i, countKey = dimKeys[0]*dimKeys[1];
	for(i=0; i<countKey-1; ++i)
		KeyStr(nr,posKeys[i],txtKey[i],colorTxtKey[i]);
	KeyStrDisp(nr,posKeys[i],txtKey[i],colorTxtKey[i]);
}
static void _KeysAllRelease(int nr, XY_Touch_Struct posKeys[], uint16_t dimKey[], char *txtKeys[], uint32_t colorTxtKeys[], int nrWH, DISPLAY_YES_NO disp){
	int i, countKey = dimKey[0]*dimKey[1];
	for(i=0; i<countKey-1; ++i)
		_KeyStr(nr,posKeys[i],txtKeys[i],colorTxtKeys[i],nrWH);
	if(DispYes==disp)	_KeyStrDisp(nr,posKeys[i],txtKeys[i],colorTxtKeys[i],nrWH);
	else					_KeyStr	  (nr,posKeys[i],txtKeys[i],colorTxtKeys[i],nrWH);
}

static void KeysSelectOne(int nr, XY_Touch_Struct posKeys[], int value){
	typedef void FUNC_KEYSTR(int,XY_Touch_Struct,const char*,uint32_t);
	int i, countKey = dimKeys[0]*dimKeys[1];
	void _KeyStrFunc(FUNC_KEYSTR pFunc1,FUNC_KEYSTR pFunc2){
		if(i == value)	pFunc1(nr,posKeys[i],txtKey[i],colorTxtPressKey[i]);
		else				pFunc2(nr,posKeys[i],txtKey[i],colorTxtKey[i]);
	}
	for(i=0; i<countKey-1; ++i)
		_KeyStrFunc( KeyStrPress, KeyStr );
	_KeyStrFunc( KeyStrPressDisp, KeyStrDisp );
}
static void _KeysSelectOne(int nr, XY_Touch_Struct posKeys[], int value, uint16_t dimKey[], char *txtKeys[], uint32_t colorTxtKeys[], uint32_t colorTxtPressKeys[], int nrWH, DISPLAY_YES_NO disp){
	typedef void FUNC_KEYSTR(int,XY_Touch_Struct,const char*,uint32_t,int);
	int i, countKey = dimKey[0]*dimKey[1];
	void _KeyStrFunc(FUNC_KEYSTR pFunc1,FUNC_KEYSTR pFunc2){
		if(i == value)	pFunc1(nr,posKeys[i],txtKeys[i],colorTxtPressKeys[i],nrWH);
		else				pFunc2(nr,posKeys[i],txtKeys[i],colorTxtKeys[i],nrWH);
	}
	for(i=0; i<countKey-1; ++i)
		_KeyStrFunc(_KeyStrPress,_KeyStr );
	if(DispYes==disp) _KeyStrFunc(_KeyStrPressDisp,_KeyStrDisp );
	else					_KeyStrFunc(_KeyStrPress, 	  _KeyStr );
}

static void SetTouch_Button(int nr, uint16_t startTouchIdx, XY_Touch_Struct* posKeys){
	if(startTouchIdx){
		for(int i=0; i<GetPosKeySize(); ++i)
			SetTouch(nr,ID_TOUCH_GET_ANY_POINT_WITH_WAIT, s[nr].startTouchIdx + i, TOUCH_GET_PER_X_PROBE, posKeys[i]);
}}
static void _SetTouch_Button(int nr, uint16_t startTouchIdx, XY_Touch_Struct* posKeys, uint16_t dimKey[], int nrWH){
	uint8_t touchCnt = s[nr].nmbTouch;
	if(startTouchIdx){
		for(int i=0; i<dimKey[0]*dimKey[1]; ++i)
			_SetTouch(nr,ID_TOUCH_GET_ANY_POINT_WITH_WAIT, s[nr].startTouchIdx + touchCnt + i, TOUCH_GET_PER_X_PROBE, posKeys[i],nrWH);
}}

static void SetTouch_Select(int nr, uint16_t startTouchIdx, XY_Touch_Struct* posKeys){
	if(startTouchIdx){
		for(int i=0; i<GetPosKeySize(); ++i)
			SetTouch(nr,ID_TOUCH_POINT, s[nr].startTouchIdx + i, press, posKeys[i]);
}}
static void _SetTouch_Select(int nr, uint16_t startTouchIdx, XY_Touch_Struct* posKeys, uint16_t dimKey[], int nrWH){
	uint8_t touchCnt = s[nr].nmbTouch;
	if(startTouchIdx){
		for(int i=0; i<dimKey[0]*dimKey[1]; ++i)
			_SetTouch(nr,ID_TOUCH_POINT, s[nr].startTouchIdx + touchCnt + i, press, posKeys[i],nrWH);
}}


static void SetTouch_CircleSlider(int nr, uint16_t startTouchIdx, XY_Touch_Struct* posKeys){
	if(startTouchIdx){
		for(int i=0; i<GetPosKeySize(); ++i)
			SetTouch(nr,ID_TOUCH_GET_ANY_POINT, s[nr].startTouchIdx + i, TOUCH_GET_PER_ANY_PROBE, posKeys[i]);
}}

static void SetTouch_Additional(int nr, uint16_t startTouchIdx, StructFieldPos field){
	if(0 != field.width){//OPISAC cze wszystko prawie zero jako funkcja !!!!
		touchTemp[0].x= s[nr].x + field.x;
		touchTemp[1].x= touchTemp[0].x + field.width;
		touchTemp[0].y= s[nr].y + field.y;
		touchTemp[1].y= touchTemp[0].y + field.height;
		LCD_TOUCH_Set(ID_TOUCH_POINT, s[nr].startTouchIdx + GetPosKeySize(), press);
		s[nr].nmbTouch++;
}}

static void WinInfo(char* txt, int x,int y, int w,int h, TIMER_ID tim){
	uint32_t fillCol = BrightIncr(fillColor,0x1A);
	LCD_ShapeWindow( LCD_RoundRectangle, 0, w,h, 0,0, w,h, SetBold2Color(frameColor,0), fillCol,bkColor );
	LCD_Xmiddle(MIDDLE_NR,SetPos,SetPosAndWidth(0,w),NULL,0,NoConstWidth);
	LCD_Ymiddle(MIDDLE_NR,SetPos,SetPosAndWidth(0,h));
	LCD_StrDependOnColorsWindowIndirect(0,MIDDLE(0,LCD_X,w), y, w,h,fontID, GET_X(txt),GET_Y,txt, fullHight, 0, fillCol, DARKRED,FONT_COEFF, NoConstWidth);
	vTimerService(tim,start_time,noUse);
}

static void SetSliderDirect(int nr, figureShape shape, uint16_t width, uint16_t height){
	if(shape!=0)
		s[nr].param = CONDITION(width>height,Horizontal,Vertical);
}

static void ScrollSel_Preparation(int nr, figureShape shape, int nrScroll, int frameNmbVisib, uint16_t *sel, uint16_t *roll){
	if(shape!=0){
		if(dimKeys[1]-*sel <= frameNmbVisib/2)	*roll = dimKeys[1]-frameNmbVisib-1;
		else if(		  *sel <= frameNmbVisib/2)	*roll = 0;
		else												*roll = *sel - frameNmbVisib/2;

		uint16_t roll_copy = *roll;
		*roll *= s[nr].heightKey;
		*roll -= roll_copy;
		LCD_TOUCH_ScrollSel_SetCalculate(nrScroll, roll, sel, 0,0,0,dimKeys[1]/frameNmbVisib);
	}
}
static void SetPosKey_Scroll(int nr, XY_Touch_Struct pos[]){
	for(int i=0; i<dimKeys[1]; ++i){
		pos[i].x = s[nr].interSpace;
		pos[i].y = (i+1)*s[nr].interSpace + i*s[nr].heightKey - i;
	}
}
static void SetDimAll_Scroll(int nr){
	widthAll  = dimKeys[0]*s[nr].widthKey  + (dimKeys[0]+1)*s[nr].interSpace;
	heightAll = dimKeys[1]*s[nr].heightKey + (dimKeys[1]+1)*s[nr].interSpace - (dimKeys[1]-1);
}
static void SetTouch_Scroll(int nr, uint16_t startTouchIdx, XY_Touch_Struct* posKeys, int visiblWin){
	if(startTouchIdx){
		touchTemp[0].x= s[nr].x + posKeys[0].x;
		touchTemp[1].x= touchTemp[0].x + s[nr].widthKey;
		touchTemp[0].y= s[nr].y + posKeys[0].y;
		touchTemp[1].y= touchTemp[0].y + visiblWin;
		LCD_TOUCH_Set(ID_TOUCH_GET_ANY_POINT, s[nr].startTouchIdx, TOUCH_GET_PER_ANY_PROBE);
		s[nr].nmbTouch++;
	}
}
static void ScrollSel_SetNoneBk(int nr, figureShape shape){
	if(shape!=0)
		LCD_ShapeWindow( LCD_RoundRectangle,0,widthAll,heightAll, 0,0, widthAll,heightAll, SetBold2Color(frameMainColor,s[nr].bold), fillMainColor,bkColor );
}
static void ScrollSel_SetBk(int nr, figureShape shape, XY_Touch_Struct posHead, char* txtDescr, uint32_t colorDescr,uint16_t spaceFrame2Roll,int win){
	if(shape!=0){
		int head = posHead.y + LCD_GetFontHeight(fontID_descr) + posHead.y;

		BKCOPY_VAL(heightAll_c,heightAll,head+win+spaceFrame2Roll);
		BKCOPY_VAL(widthAll_c,widthAll,widthAll+2*spaceFrame2Roll);

		LCD_ShapeWindow( LCD_RoundRectangle,0,widthAll,heightAll, 0,0, widthAll,heightAll, SetBold2Color(frameMainColor,s[nr].bold), fillMainColor,bkColor );
	 	StrDescr(nr,posHead, txtDescr, colorDescr);
	 	LCD_Display(0, s[nr].x-spaceFrame2Roll, s[nr].y-head, widthAll, heightAll);

	 	BKCOPY(widthAll,widthAll_c);
	 	BKCOPY(heightAll,heightAll_c);
	}
}
static void ScrollSel_Calculate(int nr, figureShape shape, int nrScroll, uint16_t *sel, uint16_t *roll, int visiblWin){
	if(shape==0)
		LCD_TOUCH_ScrollSel_SetCalculate(nrScroll, roll, sel, s[nr].y, heightAll, s[nr].heightKey, visiblWin);
}
static void ScrollSel_Draw(int nr, XY_Touch_Struct* posKeys, uint16_t selFrame, uint16_t roll, int win){
	int fillColor_copy = fillColor;
	for(int i=0; i<dimKeys[1]; ++i){
		if(i == selFrame)
			KeyStrPressLeft(nr,posKeys[i],txtKey[i],(uint32_t)colorTxtPressKey[i]);		/* _KeyStrPress(posKey[i],txtKey[i],colorTxtPressKey); */
		else{
			fillColor = (i%2) ? BrightDecr(fillColor_copy,0x10) : fillColor_copy;
			KeyStrleft(nr,posKeys[i],txtKey[i],colorTxtKey[i]);								/*	_KeyStr(posKey[i],txtKey[i],colorTxtKey[i]); */
		}
	}
	LCD_Display(0 + roll * widthAll, s[nr].x, s[nr].y, widthAll, win);
}
static int ScrollSel_SetVisiblWin(int nr, int frameNmbVis){
	return frameNmbVis * s[nr].heightKey - (frameNmbVis-1);
}

/*	-----------	General Functions -----------------*/

structSize KEYBOARD_GetSize(void){
	structSize temp = {widthAll,heightAll};
	return temp;
}

void KEYBOARD_KeyParamSet(TXT_PARAM_KEY param, ...){
	char *ptr= NULL;
	COLORS_DEFINITION temp= 0;
	int countParam= MINVAL2( dimKeys[0]*dimKeys[1], MAX_WIN_Y );
	va_list va;
	va_start(va,0);
	switch(param){
		case StringTxt:
			for(int i=0; i<countParam; ++i){
				TXT_CUTTOFF(ptr=va_arg(va,char*),MAX_WIN_X);
				strcpy(txtKey[i],ptr); }
			break;
		case Color1Txt:
			for(int i=0; i<countParam; ++i)
				colorTxtKey[i]= va_arg(va,COLORS_DEFINITION);
			break;
		case Color2Txt:
			for(int i=0; i<countParam; ++i)
				colorTxtPressKey[i]= va_arg(va,COLORS_DEFINITION);
			break;
		case XYsizeTxt:
			dimKeys[0]= va_arg(va,int);
			dimKeys[1]= va_arg(va,int);
			break;
		case StringTxtAll:
			TXT_CUTTOFF(ptr=va_arg(va,char*),MAX_WIN_X);
			for(int i=0; i<countParam; ++i)
				strcpy(txtKey[i],ptr);
			break;
		case Color1TxtAll:
			temp = va_arg(va,COLORS_DEFINITION);
			for(int i=0; i<countParam; ++i)
				colorTxtKey[i]= temp;
			break;
		case Color2TxtAll:
			temp = va_arg(va,COLORS_DEFINITION);
			for(int i=0; i<countParam; ++i)
				colorTxtPressKey[i]= temp;
			break;
	}
	va_end(va);
}
void KEYBOARD_KeyAllParamSet(uint16_t sizeX,uint16_t sizeY, ...){
	char *ptr= NULL;
	int countParam= MINVAL2(sizeX*sizeY,MAX_WIN_Y);
	va_list va;
	va_start(va,0);
	dimKeys[0]= sizeX;
	dimKeys[1]= sizeY;
	for(int i=0; i<countParam; ++i){
		TXT_CUTTOFF(ptr=va_arg(va,char*),MAX_WIN_X);
		strcpy(txtKey[i],ptr); }
	for(int i=0; i<countParam; ++i)
		colorTxtKey[i]= va_arg(va,COLORS_DEFINITION);
	for(int i=0; i<countParam; ++i)
		colorTxtPressKey[i]= va_arg(va,COLORS_DEFINITION);
}
void KEYBOARD_KeyAllParamSet2(uint16_t sizeX,uint16_t sizeY, COLORS_DEFINITION color1,COLORS_DEFINITION color2, ...){
	char *ptr= NULL;
	va_list va;
	va_start(va,0);
	dimKeys[0]= sizeX;
	dimKeys[1]= sizeY;
	for(int i=0; i<MINVAL2(sizeX*sizeY,MAX_WIN_Y); ++i){
		colorTxtKey[i]= color1;
		colorTxtPressKey[i]= color2;
		TXT_CUTTOFF(ptr=va_arg(va,char*),MAX_WIN_X);
		strcpy(txtKey[i],ptr);
	}
}
void KEYBOARD_KeyAllParamSet3(uint16_t sizeX,uint16_t sizeY, COLORS_DEFINITION color1,COLORS_DEFINITION color2, char** txt){
	char *ptr= NULL;
	dimKeys[0]= sizeX;
	dimKeys[1]= sizeY;
	for(int i=0; i<MINVAL2(sizeX*sizeY,MAX_WIN_Y); ++i){
		colorTxtKey[i]= color1;
		colorTxtPressKey[i]= color2;
		TXT_CUTTOFF(ptr=txt[i],MAX_WIN_X);
		strcpy(txtKey[i],ptr);
	}
}

void KEYBOARD_SetGeneral(int vFontID,int vFontID_descr,int vColorDescr,int vFrameMainColor,int vFillMainColor,int vFrameColor,int vFillColor,int vFramePressColor,int vFillPressColor,int vBkColor){
	if(vFontID 			  !=N) fontID 			  = vFontID;
	if(vFontID_descr 	  !=N) fontID_descr 	  = vFontID_descr;
	if(vColorDescr 	  !=N) colorDescr 	  = vColorDescr;

	if(vFrameMainColor  !=N) frameMainColor  = vFrameMainColor;
	if(vFillMainColor   !=N) fillMainColor   = vFillMainColor;

	if(vFrameColor 	  !=N) frameColor 	  = vFrameColor;
	if(vFillColor 		  !=N) fillColor 		  = vFillColor;

	if(vFramePressColor !=N) framePressColor = vFramePressColor;
	if(vFillPressColor  !=N) fillPressColor  = vFillPressColor;
	if(vBkColor 		  !=N) bkColor 		  = vBkColor;
}

int KEYBOARD_StartUp(int type, figureShape shape, uint8_t bold, uint16_t x, uint16_t y, uint16_t widthKey, uint16_t heightKey, uint8_t interSpace, uint16_t forTouchIdx, uint16_t startTouchIdx, uint8_t eraseOther){
	int k = type-1;
	if(KEYBOARD_NONE == type){
		_deleteAllTouchs();
		_deleteAllTouchParams();
		return 1;
	}
	if(shape!=0){
		if(KeysDel == eraseOther){
			_deleteAllTouchs();
			_deleteAllTouchParams();
		}
		else{
			_deleteTouchs(k);
			_deleteTouchParams(k);
		}

		s[k].shape = shape;
		s[k].bold = bold;
		s[k].x = x;
		s[k].y = y;
		s[k].widthKey = widthKey;
		s[k].heightKey = heightKey;
		s[k].interSpace = interSpace;
		s[k].forTouchIdx = forTouchIdx;
		s[k].startTouchIdx = startTouchIdx;
		s[k].nmbTouch = 0;
		s[k].param = 0;
		s[k].param2 = 0;
	}
	return 0;
}

/* ----- User Function Definitions ----- */

void KEYBOARD_Buttons(int k, int selBlockPress, INIT_KEYBOARD_PARAM, int TOUCH_Release, int TOUCH_Action, char* txtDescr)
{
	int head = GetHeightHead(k);
	int interSpaceForButtons = 4;
	XY_Touch_Struct posKey[GetPosKeySize()];

	SetDimKey(k,shape,widthKey,heightKey);
	SetPosKey(k,posKey,interSpaceForButtons,head);
	SetDimAll(k,interSpaceForButtons,head);

	bkColor = fillMainColor;
	if(TOUCH_Release == selBlockPress){
		ShapeWin(k,widthAll,heightAll);
		TxtDescr(k, 0,0, txtDescr);
		KeysAllRelease(k, posKey);
	}
	else{
		INIT(nr,selBlockPress-TOUCH_Action);
		KeyStrPressDisp_oneKey(k,posKey[nr],nr);
	}
	SetTouch_Button(k, startTouchIdx, posKey);
}

void KEYBOARD_Select(int k, int selBlockPress, INIT_KEYBOARD_PARAM, int TOUCH_Release, char* txtDescr, int value)
{
	int head = CONDITION( NULL==txtDescr, 0, VALPERC(GetHeightHead(k),150) );
	int interSpaceForSelect = -1;
	XY_Touch_Struct posKey[GetPosKeySize()];

	SetDimKey(k,shape,widthKey,heightKey);
	SetPosKey(k,posKey,interSpaceForSelect,head);
	SetDimAll(k,interSpaceForSelect,head);

	bkColor = fillMainColor;
	if(TOUCH_Release == selBlockPress){
		if(NULL !=txtDescr){
			ShapeWin(k,widthAll,heightAll);
			TxtDescrMidd(k,head,txtDescr);
		}
		KeysSelectOne(k, posKey, value);
	}
	SetTouch_Select(k, startTouchIdx, posKey);
}

void KEYBOARD_ServiceSizeStyle(int k, int selBlockPress, INIT_KEYBOARD_PARAM, int TOUCH_Release, int TOUCH_Action, char* txtDescr, int value)
{
	const char *txtKey1[]							 = {"Size +",	"Size -"};
	const COLORS_DEFINITION colorTxtKey1[]		 = {WHITE,	  	WHITE};
	const COLORS_DEFINITION colorTxtPressKey1[]= {DARKRED,	BLACK};
	const uint16_t dimKeys1[] = {1,2};

	const char *txtKey2[]								= {"Normal", "Bold", "Italic"};
	const COLORS_DEFINITION colorTxtKey2[]			= {WHITE,	  WHITE,  WHITE};
	const COLORS_DEFINITION colorTxtPressKey2[]	= {BLACK,	  BROWN,  ORANGE};
	const uint16_t dimKeys2[] = {1,3};

	int interSpaceForSelect = -1;

	XY_Touch_Struct posKey1[_GetPosKeySize((uint16_t*)dimKeys1)];
	XY_Touch_Struct posKey2[_GetPosKeySize((uint16_t*)dimKeys2)];
	int head = GetHeightHead(k);

	_SetDimKey(k,shape,widthKey,heightKey,(uint16_t*)dimKeys1,(char**)txtKey1,KEBOARD_1);
	_SetDimKey(k,shape,widthKey,heightKey,(uint16_t*)dimKeys2,(char**)txtKey2,KEBOARD_2);

	structSize allSize1 = _SetDimAll(k,s[k].interSpace,	 head,(uint16_t*)dimKeys1,KEBOARD_1);
	structSize allSize2 = _SetDimAll(k,interSpaceForSelect,head,(uint16_t*)dimKeys2,KEBOARD_2);

	_SetPosKey(k,posKey1,s[k].interSpace, 		head, (uint16_t*)dimKeys1, 0,									 	KEBOARD_1);
	_SetPosKey(k,posKey2,interSpaceForSelect, head, (uint16_t*)dimKeys2, allSize1.w - s[k].interSpace, KEBOARD_2);

	widthAll  = (allSize1.w - s[k].interSpace) + allSize2.w;
	heightAll = MAXVAL2(allSize1.h, allSize2.h);

	bkColor = fillMainColor;
	if(TOUCH_Release == selBlockPress){
		ShapeWin(k,widthAll,heightAll);
		TxtDescr(k, 0,0, txtDescr);

		_KeysAllRelease(k, posKey1, 		  (uint16_t*)dimKeys1, (char**)txtKey1, (uint32_t*)colorTxtKey1, 										  KEBOARD_1, DispNo);

		BKCOPY_VAL(c.shape, s[k].shape, LCD_Rectangle);
		_KeysSelectOne (k, posKey2, value, (uint16_t*)dimKeys2, (char**)txtKey2, (uint32_t*)colorTxtKey2, (uint32_t*)colorTxtPressKey2, KEBOARD_2, DispYes);
		BKCOPY(s[k].shape, c.shape);
	}
	else{
		INIT(nr,selBlockPress-TOUCH_Action);
		_KeyStrPressDisp_oneBlock(k, posKey1[nr], txtKey1[nr], colorTxtPressKey1[nr],KEBOARD_1);
	}
	_SetTouch_Button(k, startTouchIdx, posKey1, (uint16_t*)dimKeys1, KEBOARD_1);
	_SetTouch_Select(k, startTouchIdx, posKey2, (uint16_t*)dimKeys2, KEBOARD_2);
}

void KEYBOARD_Service_SliderButtonRGB(int k, int selBlockPress, INIT_KEYBOARD_PARAM, int TOUCH_Release, int TOUCH_Action, char* txtDescr, int *value, VOID_FUNCTION *pfunc)
{
	#define _SET_SLIDERS_HORIZONTAL
	#define _SET_SLIDERSVERTICAL

	const char *txtKey1[]							 = {"Red",				  "Green",			   "Blue"};
	const COLORS_DEFINITION colorTxtKey1[]		 = {COLOR_GRAY(0xA0),  COLOR_GRAY(0xA0),  COLOR_GRAY(0xA0)}; 	/* Color noPress: sides, pointers, lineUnSel(alternative) */
	const COLORS_DEFINITION colorTxtPressKey1[]= {RED,  				  GREEN,  			   BLUE};					/* Color Press :  sides, pointers, lineSel */
	#ifdef _SET_SLIDERS_HORIZONTAL
		const uint16_t dimKeys1[] = {3,1};
	#else
		const uint16_t dimKeys1[] = {1,3};
	#endif

	const char *txtKey2[]								= {"R-","R+", "G-","G+", "B-","B+"};
	const COLORS_DEFINITION colorTxtKey2[]			= {RED,RED,	GREEN,GREEN, BLUE,BLUE};
	const COLORS_DEFINITION colorTxtPressKey2[]	= {DARKRED,DARKRED, LIGHTGREEN,LIGHTGREEN, DARKBLUE,DARKBLUE};
	#ifdef _SET_SLIDERS_HORIZONTAL
		const uint16_t dimKeys2[] = {6,1};
	#else
	 	const uint16_t dimKeys2[] = {2,3};
	#endif

	uint32_t spaceTriangLine = 5;	/* DelTriang */
	int maxSliderValue = 255;
	uint32_t lineUnSelColor = COLOR_GRAY(0x40);
	int countKey1 = dimKeys1[0]*dimKeys1[1];
	SHAPE_PARAMS elemSliderPos[countKey1];

	XY_Touch_Struct posKey1[_GetPosKeySize((uint16_t*)dimKeys1)];
	XY_Touch_Struct posKey2[_GetPosKeySize((uint16_t*)dimKeys2)];
	int head = GetHeightHead(k);

	SetSliderDirect(k,shape, widthKey,heightKey);
	_SetDimKey_slider(k,shape,widthKey,		heightKey,												   KEBOARD_1);			/* For slider has no 'KeysAutoSize' */
	_SetDimKey		  (k,shape,KeysAutoSize,10,		  (uint16_t*)dimKeys2,(char**)txtKey2, KEBOARD_2);

	#ifdef _SET_SLIDERS_HORIZONTAL
		structSize allSize1 = _SetDimAll(k,s[k].interSpace,head,(uint16_t*)dimKeys1,KEBOARD_1);
		structSize allSize2 = _SetDimAll(k,s[k].interSpace,0,	  (uint16_t*)dimKeys2,KEBOARD_2);

		widthAll  = CONDITION(Horizontal==s[k].param, allSize1.w, allSize2.w);
		heightAll = allSize1.h + allSize2.h;

		_SetPosKey(k,posKey1,s[k].interSpace, head, 		  (uint16_t*)dimKeys1, 0, 										 												KEBOARD_1);
		_SetPosKey(k,posKey2,s[k].interSpace, allSize1.h, (uint16_t*)dimKeys2, CONDITION(Horizontal==s[k].param, MIDDLE(0, widthAll, allSize2.w), 0), KEBOARD_2);
	#else
		structSize allSize1 = _SetDimAll(k,s[k].interSpace,head,(uint16_t*)dimKeys1,KEBOARD_1);
		structSize allSize2 = _SetDimAll(k,s[k].interSpace,head,(uint16_t*)dimKeys2,KEBOARD_2);

		widthAll  = (allSize1.w - s[k].interSpace) + allSize2.w;
		heightAll = MAXVAL2(allSize1.h, allSize2.h);

		_SetPosKey(k,posKey1,s[k].interSpace, head, (uint16_t*)dimKeys1, 0,			  						  KEBOARD_1);
		_SetPosKey(k,posKey2,s[k].interSpace, head, (uint16_t*)dimKeys2, allSize1.w - s[k].interSpace, KEBOARD_2);
	#endif

	void _ElemSliderBlock(int nrSlid,SLIDER_PARAMS param){
		_ElemSliderPressDisp_oneBlock(k,x,y,posKey1[nrSlid],spaceTriangLine, ChangeElemSliderColor(param, colorTxtPressKey1[nrSlid]),colorTxtKey1[nrSlid], CONDITION(0==lineUnSelColor,colorTxtKey1[nrSlid],lineUnSelColor), value+nrSlid, CONDITION(param==PtrSel,PosXY,Percent) ,maxSliderValue,pfunc,KEBOARD_1); }

	bkColor = fillMainColor;
	if(TOUCH_Release == selBlockPress){
		ShapeWin(k,widthAll,heightAll);
		TxtDescr(k, 0,0, txtDescr);

		_KeysAllRelease_Slider(k, posKey1, value, maxSliderValue, lineUnSelColor, spaceTriangLine, elemSliderPos,(uint16_t*)dimKeys1, (char**)txtKey1, (uint32_t*)colorTxtKey1, (uint32_t*)colorTxtPressKey1, KEBOARD_1, DispNo);
		_KeysAllRelease		 (k, posKey2, 																								(uint16_t*)dimKeys2, (char**)txtKey2, (uint32_t*)colorTxtKey2, 										KEBOARD_2, DispYes);
	}
	else{
		if(IS_RANGE(selBlockPress-TOUCH_Action, 0, countKey1*NMB_SLIDER_ELEMENTS-1))
		{
			INIT(nrElemSlid, selBlockPress-TOUCH_Action);	INIT(nrSlid, nrElemSlid / NMB_SLIDER_ELEMENTS);
			switch(nrElemSlid % NMB_SLIDER_ELEMENTS){
				case 0: _ElemSliderBlock(nrSlid,LeftSel);  break;
				case 1: _ElemSliderBlock(nrSlid,PtrSel);   break;
				case 2: _ElemSliderBlock(nrSlid,RightSel); break;
			}
		}
		else{
			INIT(nr,selBlockPress-(countKey1*NMB_SLIDER_ELEMENTS)-TOUCH_Action);
			_KeyStrPressDisp_oneBlock(k, posKey2[nr], txtKey2[nr], colorTxtPressKey2[nr], KEBOARD_2);
			if(0==nr%2) _ElemSliderBlock(nr/2, LeftSel);
			else			_ElemSliderBlock(nr/2, RightSel);
		}
	}
	_SetTouch_Slider(k, startTouchIdx, elemSliderPos, (uint16_t*)dimKeys1, KEBOARD_1);
	_SetTouch_Button(k, startTouchIdx, posKey2, 		  (uint16_t*)dimKeys2, KEBOARD_2);

	#undef _SET_SLIDERS_HORIZONTAL
	#undef _SET_SLIDERSVERTICAL
}

void KEYBOARD_ServiceSliderRGB(int k, int selBlockPress, INIT_KEYBOARD_PARAM, int TOUCH_Release, int TOUCH_Action, char* txtDescr, int *value, VOID_FUNCTION *pfunc)
{
	uint32_t spaceTriangLine = 5;	/* DelTriang */									/* 'Color1Txt' is no press color for: sides, pointers, lineUnSel(alternative) */
	int maxSliderValue = 255;																/* 'Color2Txt' is 	press color for: sides, pointers, lineSel */
	uint32_t lineUnSelColor = COLOR_GRAY(0x40);

	int head = GetHeightHead(k);
	XY_Touch_Struct posKey[GetPosKeySize()];

	SetSliderDirect(k,shape, widthKey,heightKey);
	SetPosKey(k,posKey, s[k].interSpace, head);		/* For slider has no 'KeysAutoSize' */
	SetDimAll(k, s[k].interSpace, head);

	int countKey = dimKeys[0]*dimKeys[1];
	SHAPE_PARAMS elemSliderPos[countKey];

	bkColor = fillMainColor;
	if(TOUCH_Release == selBlockPress){
		ShapeWin(k,widthAll,heightAll);
		TxtDescr(k, 0,0, txtDescr);
		KeysAllRelease_Slider(k, posKey, value, maxSliderValue, lineUnSelColor, spaceTriangLine, elemSliderPos);
	}
	else{
		INIT(nrElemSlid, selBlockPress-TOUCH_Action);	INIT(nrSlid, nrElemSlid / NMB_SLIDER_ELEMENTS);
		switch(nrElemSlid % NMB_SLIDER_ELEMENTS){
		 case 0: ElemSliderPressDisp_oneBlock(k,x,y,posKey[nrSlid],spaceTriangLine, ChangeElemSliderColor(LeftSel, colorTxtPressKey[nrSlid]),colorTxtKey[nrSlid], CONDITION(0==lineUnSelColor,colorTxtKey[nrSlid],lineUnSelColor), value+nrSlid,Percent,maxSliderValue,pfunc); break;
		 case 1: ElemSliderPressDisp_oneBlock(k,x,y,posKey[nrSlid],spaceTriangLine, ChangeElemSliderColor(PtrSel,  colorTxtPressKey[nrSlid]),colorTxtKey[nrSlid], CONDITION(0==lineUnSelColor,colorTxtKey[nrSlid],lineUnSelColor), value+nrSlid,PosXY,	maxSliderValue,pfunc); break;
		 case 2: ElemSliderPressDisp_oneBlock(k,x,y,posKey[nrSlid],spaceTriangLine, ChangeElemSliderColor(RightSel,colorTxtPressKey[nrSlid]),colorTxtKey[nrSlid], CONDITION(0==lineUnSelColor,colorTxtKey[nrSlid],lineUnSelColor), value+nrSlid,Percent,maxSliderValue,pfunc); break;
		}
	}
	SetTouch_Slider(k, startTouchIdx, elemSliderPos);
}

void KEYBOARD_ServiceCircleSliderRGB(int k, int selBlockPress, INIT_KEYBOARD_PARAM, int TOUCH_Release, int TOUCH_Action, char* txtDescr, int *value, VOID_FUNCTION *pfunc)
{
	int head 	 = GetHeightHead(k);									/* 'Color1Txt' is no press color for: outline, pointer */
	int interSp  = VALPERC(s[k].interSpace,60);					/* 'Color2Txt' is 	press color for: pointer, lineSel */
	XY_Touch_Struct posKey[GetPosKeySize()];
	StructFieldPos fieldTxtDescr = {0};

	LCD_SetCircleAA(0.0, 0.0);
	CorrectLineAA_on();

	SetPosKey(k,posKey,interSp,head);
	SetDimAll(k,interSp,head);

	void _ChangeFrameColor(int type){
		if(type){	if(s[k].bold)  frameColor= bkColor;	}
		else	  {	if(s[k].bold)  frameColor= BrightDecr(frameColor,0x39);	}
	}
	void _FuncAllRelease(void){
		ShapeWin(k,widthAll,heightAll);
		fieldTxtDescr = TxtDescr(k, 0,0, txtDescr);
		_ChangeFrameColor(s[k].param);
		KeysAllRelease_CircleSlider(k, posKey,value);
	}

	bkColor = fillMainColor;
	if(TOUCH_Release == selBlockPress) _FuncAllRelease();
	else
	{	INIT(nrCircSlid, selBlockPress-TOUCH_Action);
		float radius = ((float)LCD_GetCircleWidth())/2;

		if(nrCircSlid  < GetPosKeySize()){
			_ChangeFrameColor(s[k].param);
			KeyPress_CircleSlider(k, x,y, posKey[nrCircSlid], radius, value+nrCircSlid, pfunc, colorTxtPressKey[nrCircSlid]);
		}
		else if(nrCircSlid == GetPosKeySize()){
			if(TOOGLE(s[k].param) || 0==s[k].bold) s[k].bold= LCD_IncrWrapPercCircleBold(radius, s[k].bold, 20,80, 10);
			_FuncAllRelease();
		}
	}
	SetTouch_CircleSlider(k, startTouchIdx, posKey);
	SetTouch_Additional(k, startTouchIdx, fieldTxtDescr);
}

void KEYBOARD_ServiceSizeRoll(int k, int selBlockPress, INIT_KEYBOARD_PARAM, int touchRelease, int nrRoll, char* txtDescr, uint32_t colorDescr, int frameNmbVis, int value)
{
/*	#define _FRAME2ROLL */
	XY_Touch_Struct posKey[GetPosKeySize()];
	uint16_t selFrame = value;
	uint16_t roll = 0;

	#ifdef _FRAME2ROLL
		XY_Touch_Struct posHead = {0,5};
		uint16_t spaceFrame2Roll = 10;
	#endif

	SetDimKey(k,shape,widthKey,heightKey);
	SetPosKey_Scroll(k,posKey);
	SetDimAll_Scroll(k);

	int win = ScrollSel_SetVisiblWin(k,frameNmbVis);
	ScrollSel_Preparation(k,shape,nrRoll,frameNmbVis,&selFrame,&roll);
	framePressColor = frameColor;

	if(touchRelease == selBlockPress){
		#ifdef _FRAME2ROLL
			ScrollSel_SetBk(k,shape,posHead,txtDescr,colorDescr,spaceFrame2Roll,win);
		#else
		 	ScrollSel_SetNoneBk(k,shape);
		#endif
		 ScrollSel_Calculate(k,shape,nrRoll,&selFrame,&roll,win);
		 ScrollSel_Draw(k,posKey,selFrame,roll,win);
	}
	SetTouch_Scroll(k,startTouchIdx,posKey,win);
	#undef _FRAME2ROLL
}






















int KEYBOARD_ServiceLenOffsWin(int k, int selBlockPress, INIT_KEYBOARD_PARAM, int touchRelease, int touchAction, int touchAction2, int touchTimer, char* txtDescr, char* txtDescr2, char* txtDescr3, char* txtDescr4, uint32_t colorDescr,FUNC_MAIN *pfunc,FUNC_MAIN_INIT, TIMER_ID timID)
{extern uint32_t pLcd[]; //!!!!!!!!!!!!!!!!!!!!!!!!!!
	#define _NMB2KEY	8
	const char *txtKey[]								= {"(.......)", "(...)", "(.......)",	"(.......)", " _ ", " _ ", "|  |", "||", "Info spaces", "Write spaces", "Reset spaces"};
	const COLORS_DEFINITION colorTxtKey[]		= {WHITE,  WHITE,  WHITE,  WHITE,  WHITE,  WHITE,  WHITE,	 WHITE,		WHITE,		 WHITE,			  WHITE};
	const COLORS_DEFINITION colorTxtPressKey[]= {DARKRED,DARKRED,DARKRED,DARKRED,DARKRED,DARKRED,DARKRED,  DARKRED,	DARKRED, 	 DARKRED,		  DARKRED};
	const uint16_t dimKeys[] = {4,3};

	int widthKeyCorrect = 0;
	int heightKeyCorrect = 5;
	if(shape!=0){
		if(KeysAutoSize == widthKey){
			s[k].widthKey  = heightKey + LCD_GetWholeStrPxlWidth(fontID,(char*)txtKey[ 			 STRING_GetTheLongestTxt(_NMB2KEY-1,(char**)txtKey) 				],0,NoConstWidth) + heightKey -widthKeyCorrect;		/*	space + text + space */
			s[k].widthKey2 = heightKey + LCD_GetWholeStrPxlWidth(fontID,(char*)txtKey[_NMB2KEY + STRING_GetTheLongestTxt(2,			(char**)(txtKey+_NMB2KEY)) ],0,NoConstWidth) + heightKey -widthKeyCorrect;
			s[k].heightKey = heightKey +heightKeyCorrect + LCD_GetFontHeight(fontID) + heightKey +heightKeyCorrect;

			int diff;
			if		 (0 < (diff = dimKeys[0]*(s[k].widthKey+s[k].interSpace) - dimKeys[1]*(s[k].widthKey2+s[k].interSpace)))		s[k].widthKey2 += diff 		 / dimKeys[1];
			else if( 0 > diff)																																s[k].widthKey  += ABS(diff) / dimKeys[0];
	}}

	int countKey = dimKeys[0]*dimKeys[1] - 1;
	int head = s[k].interSpace + 2*LCD_GetFontHeight(fontID_descr) + s[k].interSpace;
	int edgeSpace = s[k].interSpace + s[k].interSpace/2;

	int widthShape1 =  s[k].widthKey - s[k].widthKey/4;
	int heightShape1 = LCD_GetFontHeight(fontID) + LCD_GetFontHeight(fontID)/3;  /* s[k].heightKey - s[k].heightKey/3; */

	XY_Touch_Struct posHead={0, s[k].interSpace};
	XY_Touch_Struct posKey[]=
	  {{edgeSpace		 					 + 0*s[k].widthKey,	1*s[k].interSpace + 0*s[k].heightKey + head},
		{edgeSpace + 1*s[k].interSpace + 1*s[k].widthKey, 	1*s[k].interSpace + 0*s[k].heightKey + head},
		{edgeSpace + 2*s[k].interSpace + 2*s[k].widthKey, 	1*s[k].interSpace + 0*s[k].heightKey + head},
		{edgeSpace + 3*s[k].interSpace + 3*s[k].widthKey, 	1*s[k].interSpace + 0*s[k].heightKey + head},
		\
		{edgeSpace		 					 + 0*s[k].widthKey,	2*s[k].interSpace + 1*s[k].heightKey + head},
		{edgeSpace + 1*s[k].interSpace + 1*s[k].widthKey, 	2*s[k].interSpace + 1*s[k].heightKey + head},
		{edgeSpace + 2*s[k].interSpace + 2*s[k].widthKey, 	2*s[k].interSpace + 1*s[k].heightKey + head},
		{edgeSpace + 3*s[k].interSpace + 3*s[k].widthKey, 	2*s[k].interSpace + 1*s[k].heightKey + head},
		\
		{edgeSpace		 					 + 0*s[k].widthKey2,	3*s[k].interSpace + 2*s[k].heightKey + head + s[k].interSpace-s[k].interSpace/3},
		{edgeSpace + 1*s[k].interSpace + 1*s[k].widthKey2, 3*s[k].interSpace + 2*s[k].heightKey + head + s[k].interSpace-s[k].interSpace/3},
		{edgeSpace + 2*s[k].interSpace + 2*s[k].widthKey2, 3*s[k].interSpace + 2*s[k].heightKey + head + s[k].interSpace-s[k].interSpace/3}};

	widthAll 	=   dimKeys[0]	  *s[k].widthKey   + (dimKeys[0]+1-2)*s[k].interSpace + 2*edgeSpace;
	widthAll_c 	=  (dimKeys[0]-1)*s[k].widthKey2  + (dimKeys[0]+0-2)*s[k].interSpace + 2*edgeSpace;
	if(widthAll_c > widthAll)	widthAll = widthAll_c;
	heightAll = head + dimKeys[1]*s[k].heightKey + (dimKeys[1]+1-1)*s[k].interSpace + s[k].interSpace-s[k].interSpace/3 + edgeSpace;

	void 	_SetFlagWin	 (void){	SET_bit(s[k].param,7); }
	int 	_IsFlagWin	 (void){ return CHECK_bit(s[k].param,7); }
	void 	_RstFlagWin	 (void){	RST_bit(s[k].param,7); }

	void _SetTxtArrang(TEXT_ARRANGEMENT type){
		if(type==TxtInRow) SET_bit(s[k].param,6);
		else					 RST_bit(s[k].param,6);
	}
	void _ToggleTxtArrang(void){
		if(CHECK_bit(s[k].param,6)) RST_bit(s[k].param,6);
		else								 SET_bit(s[k].param,6);
	}
	TEXT_ARRANGEMENT _GetTxtArrang(void){
		if(CHECK_bit(s[k].param,6)) return TxtInRow;
		else								 return TxtInSeq;
	}
	int heightUpDn = 17;
	int widthtUpDn = 26;
	int spaceBetweenArrows = 10;
	int spaceFromFrame = 10;
	int space2UpDn = spaceFromFrame + heightUpDn + spaceFromFrame;
	uint32_t bkColorSel = 0xFF202020;

	static uint32_t 	posTxtTab[MAX_SCREENS_FOR_LCD_LIST]={0};
	static int 			maxScreens=0;
	static uint32_t 	i_posTxtTab=0;
	static int 			nmbrLinesInWin=0;

	static LCD_STR_PARAM paramTxt;
	static int 				paramCreateWin=0;
	static int 				calcWinWidth=200;
	static SHAPE_PARAMS 	arrowUpParam={0}, arrowDnParam={0};
	static StructTxtPxlLen listTxtStruct={0};

	POS_SIZE win = { .pos={ s[k].x+widthAll+1, s[k].y 				 	}, .size={calcWinWidth,390} };
	POS_SIZE win2 ={ .pos={ 15, 					 s[k].y+heightAll+15 }, .size={600, 		  60} };

	if(shape!=0){
		_SetTxtArrang(TxtInRow);
		if(TakeMutex(Semphr_sdram,3000)){
			listTxtStruct = LCD_LIST_TXT_len( LCD_LIST_TXT_example(pCHAR_PLCD(0),NULL),TxtInRow, fontID_descr,0,NoConstWidth, NULL,posTxtTab,&maxScreens,win.size.h,space2UpDn);  //LCD_DisplayRemeberedSpacesBetweenFonts(1,pCHAR_PLCD(0),NULL);		/* LCD_LIST_TXT_example(pCHAR_PLCD(0)) */
			GiveMutex(Semphr_sdram);
		}
		if(listTxtStruct.inPixel){
			win.size.w= listTxtStruct.inPixel + 2*spaceFromFrame;
			calcWinWidth = win.size.w;
			nmbrLinesInWin = listTxtStruct.inChar;
	}}

	void _WinInfo(char* txt){
		WinInfo(txt, win2.pos.x, win2.pos.y, win2.size.w, win2.size.h, timID);
	}
	void _WindowSpacesInfo(uint16_t x,uint16_t y, uint16_t width,uint16_t height, int param, int spaceFromFrame, int nmbrStrip){
		int xPosU = width-2*(widthtUpDn+spaceBetweenArrows);
		int xPosD = width-(widthtUpDn+spaceBetweenArrows);
		int yPosUD = height-spaceFromFrame-heightUpDn;

		uint32_t tabFontColor[]={BrightDecr(colorDescr,0x20),MYGREEN,CYAN,colorDescr,colorDescr,MYRED,ORANGE};

		uint32_t *pTabFontColor = (uint32_t*)pvPortMalloc(nmbrStrip*sizeof(uint32_t));
		LOOP_FOR(i,nmbrStrip){ pTabFontColor[i]=colorDescr; }
		LOOP_FOR(i,nmbrStrip){
			if(i >= STRUCT_TAB_SIZE(tabFontColor)) break;
			pTabFontColor[i] = tabFontColor[i];
		}

		static uint32_t posTxt_temp=0;
		StructTxtPxlLen winTxtStruct={0};

		if(NoDirect==param){
			touchTemp[0].x= win.pos.x + xPosU;
			touchTemp[1].x= touchTemp[0].x + widthtUpDn;
			touchTemp[0].y= win.pos.y + yPosUD;
			touchTemp[1].y= touchTemp[0].y + heightUpDn;
			LCD_TOUCH_Set(ID_TOUCH_GET_ANY_POINT_WITH_WAIT,touchAction2,TOUCH_GET_PER_X_PROBE);
			s[k].nmbTouch++;

			touchTemp[0].x= win.pos.x + xPosD;
			touchTemp[1].x= touchTemp[0].x + widthtUpDn;
			touchTemp[0].y= win.pos.y + yPosUD;
			touchTemp[1].y= touchTemp[0].y + heightUpDn;
			LCD_TOUCH_Set(ID_TOUCH_GET_ANY_POINT_WITH_WAIT,touchAction2+1,TOUCH_GET_PER_X_PROBE);
			s[k].nmbTouch++;

			touchTemp[0].x= win.pos.x + spaceBetweenArrows;
			touchTemp[1].x= win.pos.x + (xPosU-spaceBetweenArrows);
			touchTemp[0].y= win.pos.y + yPosUD;
			touchTemp[1].y= touchTemp[0].y + heightUpDn;
			LCD_TOUCH_Set(ID_TOUCH_POINT,touchAction2+2,press);
			s[k].nmbTouch++;

			touchTemp[0].x= win.pos.x + VALPERC(win.size.w,75);
			touchTemp[1].x= win.pos.x + win.size.w;
			touchTemp[0].y= win.pos.y;
			touchTemp[1].y= win.pos.y + height - space2UpDn;
			LCD_TOUCH_Set(ID_TOUCH_GET_ANY_POINT,touchAction2+3,TOUCH_GET_PER_ANY_PROBE);
			s[k].nmbTouch++;

			i_posTxtTab=0;
			LCD_TOUCH_SusspendTouch(s[k].forTouchIdx);
		}
		else if(Up==param){
			CONDITION(1<i_posTxtTab && 0<posTxt_temp, i_posTxtTab-=2, i_posTxtTab--);
		}
		else if(inside==param && posTxt_temp){		/* click change style */
			if(0<i_posTxtTab) i_posTxtTab--;
		}

		LCD_ShapeWindow( s[k].shape, 0, width,height, 0,0, width,height, SetBold2Color(frameColor,s[k].bold), bkColor,bkColor );
		LCD_ShapeWindow( s[k].shape, 0, width,height, s[k].bold+1,spaceFromFrame+6*GetHeightFontDescr(), width-2*(s[k].bold+1),GetHeightFontDescr(), bkColorSel, bkColorSel,bkColorSel );
		if(TakeMutex(Semphr_sdram,3000)){
			int nmbLines=0;
			char *ptr = LCD_LIST_TXT_example(pCHAR_PLCD(width*height),&nmbLines);//LCD_DisplayRemeberedSpacesBetweenFonts(1,pCHAR_PLCD(width*height),NULL);	/* LCD_LIST_TXT_example(pCHAR_PLCD(width*height)) */
			winTxtStruct = LCD_ListTxtWin(0,width,height,fontID_descr,spaceFromFrame,spaceFromFrame,ptr+posTxtTab[i_posTxtTab],6,fullHight,0,bkColor,bkColorSel,colorDescr,FONT_COEFF,NoConstWidth, pTabFontColor, _GetTxtArrang(),space2UpDn);
			posTxt_temp = winTxtStruct.inChar;
		/*	if(posTxt_temp){
				maxScreens= nmbLines/winTxtStruct.height;
				if(nmbLines%winTxtStruct.height)	maxScreens++;
			} */
			GiveMutex(Semphr_sdram);
		}
		vPortFree(pTabFontColor);

		if(posTxt_temp) i_posTxtTab++;	/* if whole Win list is full */

		LCD_TOUCH_SusspendTouch(touchAction2);
		LCD_TOUCH_SusspendTouch(touchAction2+1);

		arrowUpParam.pos[0].x = 0;
		arrowDnParam.pos[0].x = 0;

		mini_snprintf(paramTxt.str,MAX_TXT_SIZE__LCD_STR_PARAM-1,"Site %d (max %d)",CONDITION(posTxt_temp,i_posTxtTab,i_posTxtTab+1), maxScreens);
		paramTxt=LCD_SetStrDescrParam(win.pos.x+spaceBetweenArrows, win.pos.y+yPosUD, GetStrPxlWidth(fontID_descr,paramTxt.str,NoConstWidth), GetHeightFont(fontID_descr), fontID_descr,paramTxt.str,fullHight,0,bkColor,YELLOW,FONT_COEFF,NoConstWidth);
		uint32_t fontColorInfo = CONDITION(inside==param,RED,paramTxt.fontCol);
		LCD_StrDependOnColorsWindow(0,width,height,paramTxt.fontId, 10,yPosUD, paramTxt.str, paramTxt.onlyDig,paramTxt.spac,paramTxt.bkCol,fontColorInfo,paramTxt.maxV,paramTxt.constW);

		if(1 < i_posTxtTab || (1==i_posTxtTab && 0==posTxt_temp)){
			uint32_t colorUp=colorDescr;
			if(Up==param) colorUp=BrightIncr(colorDescr,0x40);
			LCDSHAPE_Window(LCDSHAPE_Arrow,0,arrowUpParam=LCD_Arrow(ToStructAndReturn,width,height, xPosU,yPosUD, SetLineBold2Width(widthtUpDn,7), SetTriangHeightCoeff2Height(heightUpDn,3), colorUp,colorUp,bkColor, Up));
			arrowUpParam.pos[0].x += win.pos.x;
			arrowUpParam.pos[0].y += win.pos.y;
			arrowUpParam.bkSize.x = widthtUpDn;
			arrowUpParam.bkSize.y = heightUpDn;
			arrowUpParam.color[0].frame=colorDescr;
			arrowUpParam.color[0].fill=colorDescr;
			LCD_TOUCH_RestoreSusspendedTouch(touchAction2);
		}
		if(0 < posTxt_temp){
			uint32_t colorDn=colorDescr;
			if(Down==param) colorDn=BrightIncr(colorDescr,0x40);
			LCDSHAPE_Window(LCDSHAPE_Arrow,0,arrowDnParam=LCD_Arrow(ToStructAndReturn,width,height, xPosD,yPosUD, SetLineBold2Width(widthtUpDn,7), SetTriangHeightCoeff2Height(heightUpDn,3), colorDn,colorDn,bkColor, Down));
			arrowDnParam.pos[0].x += win.pos.x;
			arrowDnParam.pos[0].y += win.pos.y;
			arrowDnParam.bkSize.x = widthtUpDn;
			arrowDnParam.bkSize.y = heightUpDn;
			arrowDnParam.color[0].frame=colorDescr;
			arrowDnParam.color[0].fill=colorDescr;
			LCD_TOUCH_RestoreSusspendedTouch(touchAction2+1);
		}
		LCD_Display(0, x,y, width,height);
	}
	int retVal=0;
	void _CreateWindows(int nr,int param){
		paramCreateWin=param;
		switch(nr){
			case 0:
				_WindowSpacesInfo(win.pos.x ,win.pos.y, win.size.w, win.size.h, param, spaceFromFrame, listTxtStruct.height); 	_SetFlagWin();
				break;
			case 1:
				break;
		}
	}
	void _DeleteWindows(void){		/* Use function only after displaying (not during) */
		pfunc(FUNC_MAIN_ARG);	_RstFlagWin();  LCD_DisplayPart(0,win.pos.x ,win.pos.y, win.size.w, win.size.h); retVal=1;
		LCD_TOUCH_DeleteSelectAndSusspendTouch(touchAction2);
		LCD_TOUCH_DeleteSelectAndSusspendTouch(touchAction2+1);
		LCD_TOUCH_DeleteSelectTouch(touchAction2+2);
		LCD_TOUCH_DeleteSelectTouch(touchAction2+3);
		s[k].nmbTouch-=4;
		LCD_TOUCH_RestoreSusspendedTouch(s[k].forTouchIdx);
	}
	void _OverArrowTxt(int nr, DIRECTIONS direct){
		LCD_ArrowTxt(0,widthAll,heightAll, MIDDLE(posKey[nr].x, s[k].widthKey, widthShape1), MIDDLE(posKey[nr].y, s[k].heightKey, heightShape1), widthShape1,heightShape1, frameColor,frameColor,fillColor, direct,fontID,(char*)txtKey[nr],colorTxtKey[nr]);
	}
	void _OverArrowTxt_oneBlockDisp(int nr, DIRECTIONS direct){
		LCD_ShapeWindow( s[k].shape, 0, s[k].widthKey,s[k].heightKey, 0,0, s[k].widthKey,s[k].heightKey, SetBold2Color(framePressColor,s[k].bold),fillPressColor,bkColor);
		LCD_ArrowTxt(0,s[k].widthKey,s[k].heightKey, MIDDLE(0, s[k].widthKey, widthShape1), MIDDLE(0, s[k].heightKey, heightShape1), widthShape1,heightShape1, colorTxtPressKey[nr],colorTxtPressKey[nr],fillPressColor, direct,fontID,(char*)txtKey[nr],colorTxtPressKey[nr]);
		LCD_Display(0, s[k].x+posKey[nr].x, s[k].y+posKey[nr].y, s[k].widthKey, s[k].heightKey);
		if(_IsFlagWin())	_DeleteWindows();
	}

	if(touchRelease == selBlockPress)
	{
			BKCOPY_VAL(frameColor_c[0],frameColor,WHITE);
				LCD_ShapeWindow( s[k].shape,0,widthAll,heightAll, 0,0, widthAll,heightAll, SetBold2Color(frameMainColor,s[k].bold), fillMainColor,bkColor );		/* s[k].shape(0,widthAll,heightAll, 0,0, widthAll,heightAll, SetColorBoldFrame(frameColor,s[k].bold), bkColor,bkColor); */
			BKCOPY(frameColor,frameColor_c[0]);

			BKCOPY_VAL(bkColor_c[0],bkColor,fillMainColor);
		/*	posHead.y = s[k].interSpace/2; */											StrDescr_Xmidd_Yoffs(posHead, 0, txtDescr3, colorDescr);		/* _StrDescr(k,posHead, SL(LANG_LenOffsWin), v.FONT_COLOR_Descr); */
			posHead.y += LCD_GetFontHeight(fontID_descr)+s[k].interSpace/3;	StrDescr_Xmidd_Yoffs(posHead, 0, txtDescr4, colorDescr);
			BKCOPY(bkColor,bkColor_c[0]);

			BKCOPY_VAL(fillColor_c[0],fillColor,BrightIncr(fillColor,0xE));
			for(int i=0; i<_NMB2KEY; ++i)
			{
				Key(k,posKey[i]);
				switch(i){
					case 0: 	_OverArrowTxt(i,outside); 	  break;
					case 1: 	_OverArrowTxt(i,inside); 	  break;
					case 2: 	_OverArrowTxt(i,LeftLeft);   break;
					case 3: 	_OverArrowTxt(i,RightRight); break;
					case 4: 	_OverArrowTxt(i,LeftLeft);   break;
					case 5: 	_OverArrowTxt(i,RightRight); break;
					case 6: 	_OverArrowTxt(i,outside);	  break;
					case 7: 	_OverArrowTxt(i,inside); 	  break;
				}
			}
			BKCOPY(fillColor,fillColor_c[0]);

			BKCOPY_VAL(fillColor_c[0],fillColor,BrightIncr(fillColor,0x6));
			BKCOPY_VAL(c.widthKey,s[k].widthKey,s[k].widthKey2);
			for(int i=_NMB2KEY; i<countKey; ++i)
			{
				if(i<countKey-1){
					if(_IsFlagWin() && _NMB2KEY==i){		/* find key to hold press */
						BKCOPY_VAL(fillColor_c[1],fillColor,fillPressColor);
						BKCOPY_VAL(frameColor_c[0],frameColor,framePressColor);
						 KeyStr(k,posKey[i],txtKey[i],colorTxtPressKey[i]);
						BKCOPY(fillColor,fillColor_c[1]);
						BKCOPY(frameColor,frameColor_c[0]);
					}
					else KeyStr(k,posKey[i],txtKey[i],colorTxtKey[i]);
				}
				else KeyStrDisp(k,posKey[i],txtKey[i],colorTxtKey[i]);
			}
			BKCOPY(s[k].widthKey,c.widthKey);
			BKCOPY(fillColor,fillColor_c[0]);
	}
	else if(touchAction+0 == selBlockPress) 		_OverArrowTxt_oneBlockDisp(0,outside);
	else if(touchAction+1 == selBlockPress) 		_OverArrowTxt_oneBlockDisp(1,inside);
	else if(touchAction+2 == selBlockPress)  	_OverArrowTxt_oneBlockDisp(2,LeftLeft);
	else if(touchAction+3 == selBlockPress) 	_OverArrowTxt_oneBlockDisp(3,RightRight);
	else if(touchAction+4 == selBlockPress)    _OverArrowTxt_oneBlockDisp(4,LeftLeft);
	else if(touchAction+5 == selBlockPress) 	_OverArrowTxt_oneBlockDisp(5,RightRight);
	else if(touchAction+6 == selBlockPress)  _OverArrowTxt_oneBlockDisp(6,outside);
	else if(touchAction+7 == selBlockPress) _OverArrowTxt_oneBlockDisp(7,inside);

	else if(touchAction+8 == selBlockPress){	 BKCOPY_VAL(c.widthKey,s[k].widthKey,s[k].widthKey2);  KeyStrPressDisp_oneBlock(k,posKey[8],txtKey[8],colorTxtPressKey[8]);		BKCOPY(s[k].widthKey,c.widthKey); CONDITION(_IsFlagWin(),_DeleteWindows(),_CreateWindows(0,NoDirect)); }
	else if(touchAction+9 == selBlockPress){ BKCOPY_VAL(c.widthKey,s[k].widthKey,s[k].widthKey2);  KeyStrPressDisp_oneBlock(k,posKey[9],txtKey[9],colorTxtPressKey[9]);		BKCOPY(s[k].widthKey,c.widthKey); CONDITION(_IsFlagWin(),_DeleteWindows(),NULL);	 _WinInfo(txtDescr);}
	else if(touchAction+10 == selBlockPress){ BKCOPY_VAL(c.widthKey,s[k].widthKey,s[k].widthKey2);  KeyStrPressDisp_oneBlock(k,posKey[10],txtKey[10],colorTxtPressKey[10]);	BKCOPY(s[k].widthKey,c.widthKey); CONDITION(_IsFlagWin(),_DeleteWindows(),NULL);  _WinInfo(txtDescr2); }

	else if(touchAction+11 == selBlockPress){ 						  _CreateWindows(0,Up); 	 vTimerService(timID+1,restart_time,noUse); }
	else if(touchAction+12 == selBlockPress){ 						  _CreateWindows(0,Down);   vTimerService(timID+1,restart_time,noUse); }
	else if(touchAction+13 == selBlockPress){ _ToggleTxtArrang(); _CreateWindows(0,inside); vTimerService(timID+1,restart_time,noUse); }
	else if(touchAction+14 == selBlockPress){
		GetTouchToTemp(touchAction2+3);
		i_posTxtTab = ( ((y-touchTemp[0].y)* maxScreens) / (touchTemp[1].y-touchTemp[0].y) );
		_CreateWindows(0,DownUp); vTimerService(timID+1,restart_time,noUse);
	}

	else if(touchTimer+0 == selBlockPress){
		pfunc(FUNC_MAIN_ARG);
		LCD_DisplayPart(0, MIDDLE(0,LCD_X,win2.size.w)/* win2.pos.x */, win2.pos.y, win2.size.w, win2.size.h);
	}
	else if(touchTimer+1 == selBlockPress){
		switch(paramCreateWin){
		case Up: case Down:
			if(arrowUpParam.pos[0].x) LCDSHAPE_Arrow_Indirect(arrowUpParam);
			if(arrowDnParam.pos[0].x) LCDSHAPE_Arrow_Indirect(arrowDnParam);
			break;
		case inside:
			LCD_StrDependOnColorsWindowIndirectParam(paramTxt);
			break;
	}}

	if(startTouchIdx){
		for(int i=0; i<_NMB2KEY; ++i)
			SetTouch(k,ID_TOUCH_GET_ANY_POINT_WITH_WAIT, s[k].startTouchIdx + i, TOUCH_GET_PER_X_PROBE, posKey[i]);

		BKCOPY_VAL(c.widthKey,s[k].widthKey,s[k].widthKey2);
		 for(int i=_NMB2KEY; i<countKey; ++i)
			 SetTouch(k,ID_TOUCH_POINT, s[k].startTouchIdx + i, press, posKey[i]);
		BKCOPY(s[k].widthKey,c.widthKey);
	}
	return retVal;
	#undef _NMB2KEY
}
















void KEYBOARD__ServiceSetTxt(int k, int selBlockPress, INIT_KEYBOARD_PARAM, int touchRelease, int touchAction, int tBig,int tBack,int tEnter,uint32_t colorDescr) // klawiatura malutka i duza na caly LCD_X z liczbami
{
	#define _UP		"|"
	#define _LF		"<--"
	#define _EN		"<-|"
	#define _EX		"X"

	const char *txtKey[]								= {"q","w","e","r","t","y","u","i","o","p", \
																  "a","s","d","f","g","h","j","k","l", \
																_UP,"z","x","c","v","b","n","m",_LF, \
																"alt",_EX,"space",",",".",_EN };  //Sign 'S' to klawiatyra liczbnowa dla malej klawiatury

//		const char *txtKey2[]							= {"/","1","2","3", \
//																	"*","4","5","6", \
//																	"-","7","8","9", \
//																	"+","=",".","0" };

	const COLORS_DEFINITION colorTxtKey[]		= {WHITE,WHITE,WHITE,WHITE,WHITE,WHITE,WHITE,WHITE,WHITE,WHITE, \
																WHITE,WHITE,WHITE,WHITE,WHITE,WHITE,WHITE,WHITE,WHITE, \
																WHITE,WHITE,WHITE,WHITE,WHITE,WHITE,WHITE,WHITE,WHITE, \
																WHITE,WHITE,WHITE,WHITE,WHITE,WHITE };

	const COLORS_DEFINITION colorTxtPressKey[]= {DARKRED,DARKRED,DARKRED,DARKRED,DARKRED,DARKRED,DARKRED,DARKRED,DARKRED,DARKRED, \
																DARKRED,DARKRED,DARKRED,DARKRED,DARKRED,DARKRED,DARKRED,DARKRED,DARKRED, \
																DARKRED,DARKRED,DARKRED,DARKRED,DARKRED,DARKRED,DARKRED,DARKRED,DARKRED, \
																DARKRED,DARKRED,DARKRED,DARKRED,DARKRED,DARKRED };

	const uint8_t dimKeys[] = {10,9,9,6};

	#define _PARAM_ARROW_UP		structSize 	size_UP = { (35*s[k].widthKey)/100,  (2*s[k].heightKey)/5 };		int bold_UP = 1;		int coeff_UP = 3
	#define _PARAM_ARROW_LF		structSize 	size_LF = { ( 2*s[k].widthKey)/4,    (2*s[k].heightKey)/7 };		int bold_LF = 1;		int coeff_LF = 0
	#define _PARAM_ARROW_EN		structSize 	size_EN = { ( 2*s[k].widthKey)/4,    (2*s[k].heightKey)/7 };		int bold_EN = 1;		int coeff_EN = 0
	#define _PARAM_ARROW_EX		structSize 	size_EX = { (35*s[k].heightKey)/100, (35*s[k].heightKey)/100 };

	if(shape!=0){
		if(KeysAutoSize == widthKey){
			s[k].widthKey =  heightKey + LCD_GetWholeStrPxlWidth(fontID,(char*)txtKey[0],0,NoConstWidth) + heightKey;
			s[k].heightKey = heightKey + LCD_GetFontHeight(fontID) + heightKey;
	}}

	int widthFieldTxt = 0;
	int heightFieldTxt = LCD_GetFontHeight(fontID) * 3;
	int head = s[k].interSpace + heightFieldTxt + s[k].interSpace;

	#define W1	s[k].widthKey
	#define WS	4*W1 + 3*s[k].interSpace
	#define WA	W1 + (W1 + s[k].interSpace)/2

	uint16_t wKey[]= {W1,W1,W1,W1,W1,W1,W1,W1,W1,W1,\
								W1,W1,W1,W1,W1,W1,W1,W1,W1,\
								WA,W1,W1,W1,W1,W1,W1,W1,WA,\
								WA,W1,WS,W1,W1,WA};

	INIT(nr,0);		INIT(width_c,0);
	#define P(x,y,offs)	{(x+1)*s[k].interSpace + offs + CONDITION(0==x, width_c=_ReturnVal(0,CONDITION(0==y,nr=0,nr++)), width_c += wKey[nr++]),		(y+1)*s[k].interSpace + y*s[k].heightKey + head}
	#define F	(s[k].widthKey+s[k].interSpace)/2

	XY_Touch_Struct posKey[]= {P(0,0,0),P(1,0,0),P(2,0,0),P(3,0,0),P(4,0,0),P(5,0,0),P(6,0,0),P(7,0,0),P(8,0,0),P(9,0,0),\
										P(0,1,F),P(1,1,F),P(2,1,F),P(3,1,F),P(4,1,F),P(5,1,F),P(6,1,F),P(7,1,F),P(8,1,F),\
										P(0,2,0),P(1,2,0),P(2,2,0),P(3,2,0),P(4,2,0),P(5,2,0),P(6,2,0),P(7,2,0),P(8,2,0),\
										P(0,3,0),P(1,3,0),P(2,3,0),P(3,3,0),P(4,3,0),P(5,3,0)};

	INIT_INCVAL(sizeof(dimKeys),countKey,dimKeys);
	INIT_MAXVAL(dimKeys,sizeof(dimKeys),0,maxVal);

	widthAll =  maxVal*s[k].widthKey  + (maxVal+1)*s[k].interSpace;
	heightAll = sizeof(dimKeys)*s[k].heightKey + (sizeof(dimKeys)+1)*s[k].interSpace + head;
	widthFieldTxt = widthAll - 2*s[k].interSpace;

	void _KeyQ2P(int nr, int act){
		if(release==act)	Key(k,posKey[nr]);
		else	 				KeyPressWin(k);

		char *ptrTxt = Int2Str(nr,None,1,Sign_none);
		int widthDescr = LCD_GetWholeStrPxlWidth(fontID_descr,ptrTxt,0,ConstWidth);
		int heightTxt = LCD_GetFontHeight(fontID);

		if(release==act){	 StrDescr_XYoffs(posKey[nr], s[k].widthKey-VALPERC(widthDescr,180),  VALPERC(widthDescr,35), ptrTxt, 		BrightIncr(colorDescr,0x20));
								 Str_Xmidd_Yoffs(k,posKey[nr], s[k].heightKey-VALPERC(heightTxt,115), 							   txtKey[nr], colorTxtKey[nr]);
		}
		else{			BKCOPY_VAL(fillColor_c[0],fillColor,fillPressColor);
								StrDescrWin_XYoffs(k,s[k].widthKey-VALPERC(widthDescr,180), VALPERC(widthDescr,35), ptrTxt, 	   BrightIncr(colorDescr,0x20));
								StrWin_Xmidd_Yoffs(k,s[k].heightKey-VALPERC(heightTxt,115), 								txtKey[nr], colorTxtPressKey[nr]);
						BKCOPY(fillColor,fillColor_c[0]);
		}
		if(press==act) LCD_Display(0,s[k].x+posKey[nr].x,s[k].y+posKey[nr].y,s[k].widthKey,s[k].heightKey);
	}

	int colorFillBk = BrightDecr(bkColor,0x10);
	if(touchRelease == selBlockPress)
	{
			LCD_ShapeWindow( s[k].shape,0,widthAll,heightAll, 0,0, widthAll,heightAll, SetBold2Color(frameMainColor,s[k].bold), fillMainColor/*colorFillBk*/,bkColor );
			LCD_ShapeWindow( s[k].shape,0,widthAll,heightAll, s[k].interSpace,s[k].interSpace, widthFieldTxt,heightFieldTxt, SetBold2Color(BrightDecr(frameColor,0x40),s[k].bold), BrightDecr(frameColor,0x40),colorFillBk );

			fillColor = BrightIncr(fillColor,0x10);
			bkColor = colorFillBk;

			c.widthKey = s[k].widthKey;
			for(int i=0; i<countKey; ++i)
			{
				s[k].widthKey = wKey[i];
				if(STRING_CmpTxt((char*)txtKey[i],_UP)){	 		Key(k,posKey[i]);	_PARAM_ARROW_UP;
					LCD_Arrow(0,widthAll,heightAll, MIDDLE(posKey[i].x,s[k].widthKey,size_UP.w),MIDDLE(posKey[i].y,s[k].heightKey,size_UP.h), SetLineBold2Width(size_UP.w,bold_UP), SetTriangHeightCoeff2Height(size_UP.h,coeff_UP), frameColor,frameColor,bkColor, Up);
				}
				else if(STRING_CmpTxt((char*)txtKey[i],_LF)){	Key(k,posKey[i]);	_PARAM_ARROW_LF;
					LCD_Arrow(0,widthAll,heightAll, MIDDLE(posKey[i].x,s[k].widthKey,size_LF.w),MIDDLE(posKey[i].y,s[k].heightKey,size_LF.h), SetLineBold2Width(size_LF.w,bold_LF), SetTriangHeightCoeff2Height(size_LF.h,coeff_LF), frameColor,frameColor,bkColor, Left);
				}
				else if(STRING_CmpTxt((char*)txtKey[i],_EN)){	Key(k,posKey[i]);	_PARAM_ARROW_EN;
					LCD_Enter(0,widthAll,heightAll, MIDDLE(posKey[i].x,s[k].widthKey,size_EN.w),MIDDLE(posKey[i].y,s[k].heightKey,size_EN.h), SetLineBold2Width(size_EN.w,bold_EN), SetTriangHeightCoeff2Height(size_EN.h,coeff_EN), frameColor,frameColor,bkColor);
					LCD_Display(0, s[k].x, s[k].y, widthAll, heightAll);
				}
				else if(STRING_CmpTxt((char*)txtKey[i],_EX)){	KeyPress(k,posKey[i]);	_PARAM_ARROW_EX;
					LCD_Exit(0,widthAll,heightAll, MIDDLE(posKey[i].x,s[k].widthKey,size_EX.w),MIDDLE(posKey[i].y,s[k].heightKey,size_EX.h), size_EX.w, size_EX.h, colorTxtPressKey[i],colorTxtPressKey[i],bkColor);
				}
				else{
					if(i<dimKeys[0]) _KeyQ2P(i,release);
					else 				  KeyStr(k,posKey[i],txtKey[i],colorTxtKey[i]);
				}
			}
			s[k].widthKey = c.widthKey;
	}
	else if(tBig == selBlockPress){
		nr = selBlockPress-touchAction;
		BKCOPY_VAL(c.widthKey,s[k].widthKey,wKey[nr]);		_PARAM_ARROW_UP;
		KeyShapePressDisp_oneBlock(k,posKey[nr], LCDSHAPE_Arrow, LCD_Arrow(ToStructAndReturn,s[k].widthKey,s[k].heightKey, MIDDLE(0,s[k].widthKey,size_UP.w),MIDDLE(0,s[k].heightKey,size_UP.h), SetLineBold2Width(size_UP.w,bold_UP), SetTriangHeightCoeff2Height(size_UP.h,coeff_UP), colorTxtPressKey[nr],colorTxtPressKey[nr],bkColor, Up));
		BKCOPY(s[k].widthKey,c.widthKey);
	}
	else if(tBack == selBlockPress){
		nr = selBlockPress-touchAction;
		BKCOPY_VAL(c.widthKey,s[k].widthKey,wKey[nr]);		_PARAM_ARROW_LF;
		KeyShapePressDisp_oneBlock(k,posKey[nr], LCDSHAPE_Arrow, LCD_Arrow(ToStructAndReturn,s[k].widthKey,s[k].heightKey, MIDDLE(0,s[k].widthKey,size_LF.w),MIDDLE(0,s[k].heightKey,size_LF.h), SetLineBold2Width(size_LF.w,bold_LF), SetTriangHeightCoeff2Height(size_LF.h,coeff_LF), colorTxtPressKey[nr],colorTxtPressKey[nr],bkColor, Left));
		BKCOPY(s[k].widthKey,c.widthKey);
	}
	else if(tEnter == selBlockPress){
		nr = selBlockPress-touchAction;
		BKCOPY_VAL(c.widthKey,s[k].widthKey,wKey[nr]);		_PARAM_ARROW_EN;
		KeyShapePressDisp_oneBlock(k,posKey[nr], LCDSHAPE_Enter, LCD_Enter(ToStructAndReturn,s[k].widthKey,s[k].heightKey, MIDDLE(0,s[k].widthKey,size_EN.w),MIDDLE(0,s[k].heightKey,size_EN.h), SetLineBold2Width(size_EN.w,bold_EN), SetTriangHeightCoeff2Height(size_EN.h,coeff_EN), colorTxtPressKey[nr],colorTxtPressKey[nr],bkColor));
		BKCOPY(s[k].widthKey,c.widthKey);
	}
	else{
		  	  if(IS_RANGE(selBlockPress,touchAction,touchAction+9)){	 INIT(nr,selBlockPress-touchAction); BKCOPY_VAL(c.widthKey,s[k].widthKey,wKey[nr]);   _KeyQ2P(nr,press);  																		BKCOPY(s[k].widthKey,c.widthKey); }
		else if(IS_RANGE(selBlockPress,touchAction+10,tEnter)){ 	    INIT(nr,selBlockPress-touchAction); BKCOPY_VAL(c.widthKey,s[k].widthKey,wKey[nr]);   KeyStrPressDisp_oneBlock(k,posKey[nr],txtKey[nr],colorTxtPressKey[nr]); 	BKCOPY(s[k].widthKey,c.widthKey); }
	}




	if(startTouchIdx){
		for(int i=0; i<countKey; ++i){
			BKCOPY_VAL(c.widthKey,s[k].widthKey,wKey[i]);
			SetTouch(k,ID_TOUCH_POINT,s[k].startTouchIdx+i,press,posKey[i]);
			BKCOPY(s[k].widthKey,c.widthKey);
	}}
	#undef P
	#undef F
	#undef W1
	#undef WS
	#undef WA
	#undef _UP
	#undef _LF
	#undef _EN
	#undef _EX
}

/* ----- End User Function Definitions ----- */
