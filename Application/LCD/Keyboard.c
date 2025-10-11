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

static struct TEXT_PARAM{
	int fontId, OnlyDigits, space, maxVal, constWidth;
	u32 shadeColor;
	u8 deep;
	DIRECTIONS dir;
} textParam = {0,fullHight,0,250,NoConstWidth,0,0,0};

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

static void SetTxtParamInit(int font_id){
	textParam.fontId = font_id;
	textParam.OnlyDigits = fullHight;
	textParam.space		= 0;
	textParam.maxVal		= 250;
	textParam.constWidth = NoConstWidth;
	textParam.shadeColor = 0;	/* 0x777777  */
	textParam.deep			= 0;	/* 3 			 */
	textParam.dir			= 0;	/* RightDown */
}
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
static void Str_bkColorRead(const char *txt, uint32_t color){
	LCD_StrDependOnColorsWindow(0,widthAll,heightAll,fontID, GET_X((char*)txt),GET_Y,(char*)txt, fullHight, 0, READ_BGCOLOR, color,FONT_COEFF, NoConstWidth);
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
static StructFieldPos StrDescrParam(int nr,XY_Touch_Struct pos, const char *txt, uint32_t color, LCD_STR_PARAM* par){
	StructFieldPos field = {0};
	LCD_Xmiddle(MIDDLE_NR+1,SetPos,SetPosAndWidth(pos.x,widthAll),NULL,0,NoConstWidth);
	field.x = LCD_Xmiddle(MIDDLE_NR+1,GetPos,fontID_descr,(char*)txt,0,NoConstWidth);
	field.y = s[nr].interSpace+pos.y;
	if(NULL!=par)
		*par 	 = LCD_SetStrDescrParam(s[nr].x+field.x, s[nr].y+field.y,  GetStrPxlWidth(fontID_descr,(char*)txt,NoConstWidth),GetHeightFont(fontID_descr), 0,0, unUsed,unUsed, fontID_descr, (char*)txt, fullHight, 0, bkColor, color,FONT_COEFF, NoConstWidth);
	field.len = LCD_StrDependOnColorsWindow(0,								  widthAll,														  	 heightAll,						   fontID_descr, field.x,field.y,	 (char*)txt, fullHight, 0, bkColor, color,FONT_COEFF, NoConstWidth);
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
static void KeyStr_alt(int nr,XY_Touch_Struct pos,const char *txt, uint32_t color){
	LCD_RoundRectangle2(0,PARAM32(Down,bold1,unUsed,Rectangle), widthAll,heightAll, pos.x,pos.y, s[nr].widthKey, s[nr].heightKey, BrightDecr(frameColor,0x20),BrightIncr(frameColor,0x65), 0xFF808080,BrightDecr(fillColor,0x44), bkColor, 0.0, Down);
	TxtPos(nr,pos);
	Str_bkColorRead(txt,color);
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

static void KeyStrPressDisp_oneBlock(int nr, XY_Touch_Struct pos, const char *txt, uint32_t colorTxt){		/* ..._oneBlock means indirect display */
	LCD_ShapeWindow( s[nr].shape, 0, s[nr].widthKey,s[nr].heightKey, 0,0, s[nr].widthKey,s[nr].heightKey, SetBold2Color(framePressColor,s[nr].bold),fillPressColor,bkColor);
	TxtPos(nr,(XY_Touch_Struct){0});
	LCD_StrDependOnColorsWindowIndirect(0, s[nr].x+pos.x, s[nr].y+pos.y, s[nr].widthKey, s[nr].heightKey,fontID, GET_X((char*)txt),GET_Y,(char*)txt, fullHight, 0, fillPressColor, colorTxt,FONT_COEFF, NoConstWidth);
}
static void KeyStrPressDisp_win(int nr, XY_Touch_Struct pos, const char *txt, uint32_t colorTxt){
	LCD_ShapeWindow( s[nr].shape, 0, widthAll,heightAll, pos.x,pos.y, s[nr].widthKey,s[nr].heightKey, SetBold2Color(framePressColor,s[nr].bold),fillPressColor,bkColor);
	TxtPos(nr,pos);
	LCD_StrDependOnColorsWindow(0, widthAll,heightAll, fontID, GET_X((char*)txt),GET_Y, (char*)txt, fullHight, 0, fillPressColor, colorTxt,FONT_COEFF, NoConstWidth);
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
static StructFieldPos TxtDescrParam(int nr, uint16_t xPos, uint16_t yPos, char* txtDescr, LCD_STR_PARAM* par){
	XY_Touch_Struct posHead = {xPos,yPos};
	return StrDescrParam(nr, posHead, txtDescr, colorDescr, par);
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

static void KeyStrDisp_Ind(int nr, XY_Touch_Struct pos, char *txt, u32 colorTxt, int releasePress){
	if(releasePress) LCD_ShapeWindow( s[nr].shape, 0, s[nr].widthKey,s[nr].heightKey, 0,0, s[nr].widthKey,s[nr].heightKey, SetBold2Color(framePressColor,s[nr].bold),fillPressColor,bkColor);
	else				  LCD_ShapeWindow( s[nr].shape, 0, s[nr].widthKey,s[nr].heightKey, 0,0, s[nr].widthKey,s[nr].heightKey, SetBold2Color(frameColor,	  s[nr].bold),fillColor,	  bkColor);
	TxtPos(nr,(XY_Touch_Struct){0});
	LCD_StrDependOnColorsWindowIndirect(0, s[nr].x+pos.x, s[nr].y+pos.y, s[nr].widthKey, s[nr].heightKey,fontID, GET_X((char*)txt),GET_Y,(char*)txt, fullHight, 0, READ_BGCOLOR, colorTxt,FONT_COEFF, NoConstWidth);
}
static void KeyStrDisp2_Ind(int nr, XY_Touch_Struct pos, char *txt, u32 colorTxt, int releasePress){
	if(releasePress) LCD_RoundRectangle2(0,PARAM32(Up,  bold2,unUsed,Rectangle), s[nr].widthKey,s[nr].heightKey, 0,0, s[nr].widthKey, s[nr].heightKey, BrightIncr(framePressColor,0x65),BrightDecr(framePressColor,0x20), 0xFF808080,BrightDecr(fillPressColor,0x44), bkColor, 0.0, Up  );
	else				  LCD_RoundRectangle2(0,PARAM32(Down,bold2,unUsed,Rectangle), s[nr].widthKey,s[nr].heightKey, 0,0, s[nr].widthKey, s[nr].heightKey, BrightDecr(frameColor,		0x20),BrightIncr(frameColor,		0x65), 0xFF808080,BrightDecr(fillColor,	  0x44), bkColor, 0.0, Down);
	TxtPos(nr,(XY_Touch_Struct){0});
	LCD_StrDependOnColorsWindowIndirect(0, s[nr].x+pos.x, s[nr].y+pos.y, s[nr].widthKey, s[nr].heightKey,fontID, GET_X((char*)txt),GET_Y,(char*)txt, fullHight, 0, READ_BGCOLOR, colorTxt,FONT_COEFF, NoConstWidth);
}

static void KeyStrDisp_Win(int nr, XY_Touch_Struct pos, char *txt, u32 colorTxt, int releasePress){
	if(releasePress) LCD_ShapeWindow( s[nr].shape, 0, widthAll,heightAll, pos.x,pos.y, s[nr].widthKey,s[nr].heightKey, SetBold2Color(framePressColor,s[nr].bold),fillPressColor,bkColor);
	else				  LCD_ShapeWindow( s[nr].shape, 0, widthAll,heightAll, pos.x,pos.y, s[nr].widthKey,s[nr].heightKey, SetBold2Color(frameColor,	    s[nr].bold),fillColor,	    bkColor);
	TxtPos(nr,pos);
	LCD_StrDependOnColorsWindow(0,widthAll,heightAll,fontID, GET_X((char*)txt),GET_Y,(char*)txt, fullHight, 0, READ_BGCOLOR, colorTxt,FONT_COEFF, NoConstWidth);	/* Str_bkColorRead(txt,colorTxt); */
}
static void KeyStrDisp2_Win(int nr, XY_Touch_Struct pos, char *txt, u32 colorTxt, int releasePress){
	if(releasePress) LCD_RoundRectangle2(0,PARAM32(Up,  bold2,unUsed,Rectangle), widthAll,heightAll, pos.x,pos.y, s[nr].widthKey, s[nr].heightKey, BrightIncr(framePressColor,0x65),BrightDecr(framePressColor,0x20), 0xFF808080,BrightDecr(fillPressColor,0x44), bkColor, 0.0, Up  );
	else				  LCD_RoundRectangle2(0,PARAM32(Down,bold2,unUsed,Rectangle), widthAll,heightAll, pos.x,pos.y, s[nr].widthKey, s[nr].heightKey, BrightDecr(frameColor,	  0x20),BrightIncr(frameColor,	  0x65), 0xFF808080,BrightDecr(fillColor,	    0x44), bkColor, 0.0, Down);
	TxtPos(nr,pos);
	LCD_StrDependOnColorsWindow(0,widthAll,heightAll,fontID, GET_X((char*)txt),GET_Y,(char*)txt, fullHight, 0, READ_BGCOLOR, colorTxt,FONT_COEFF, NoConstWidth);
}

static void KeyShapeDisp_Ind(int nr, XY_Touch_Struct pos, ShapeFunc pShape, SHAPE_PARAMS param, int releasePress){
	if(releasePress){ LCD_ShapeWindow( s[nr].shape, 0, s[nr].widthKey,s[nr].heightKey, 0,0, s[nr].widthKey,s[nr].heightKey, SetBold2Color(framePressColor,s[nr].bold),fillPressColor,bkColor); 	param.color[0].bk = fillPressColor; }	/* bkColor for pShape is fillPressColor */
	else				 { LCD_ShapeWindow( s[nr].shape, 0, s[nr].widthKey,s[nr].heightKey, 0,0, s[nr].widthKey,s[nr].heightKey, SetBold2Color(frameColor,	   s[nr].bold),fillColor,		bkColor); 	param.color[0].bk = fillColor; 		}	/* bkColor for pShape is fillColor */
	pShape(0,param);
	LCD_Display(0, s[nr].x+pos.x, s[nr].y+pos.y, s[nr].widthKey, s[nr].heightKey);
}
static void KeyShapeDisp2_Ind(int nr, XY_Touch_Struct pos, ShapeFunc pShape, SHAPE_PARAMS param, int releasePress){
	if(releasePress){ LCD_RoundRectangle2(0,PARAM32(Up,  bold2,unUsed,Rectangle), s[nr].widthKey,s[nr].heightKey, 0,0, s[nr].widthKey, s[nr].heightKey, BrightIncr(framePressColor,0x65),BrightDecr(framePressColor,0x20), 0xFF808080,BrightDecr(fillPressColor,0x44), bkColor, 0.0, Up  ); 	param.color[0].bk = GetTransitionColor(0xFF808080,BrightDecr(fillPressColor,0x44),0.5); }	/* bkColor for pShape is fillPressColor */
	else				 { LCD_RoundRectangle2(0,PARAM32(Down,bold2,unUsed,Rectangle), s[nr].widthKey,s[nr].heightKey, 0,0, s[nr].widthKey, s[nr].heightKey, BrightDecr(frameColor,		 0x20),BrightIncr(frameColor,		 0x65), 0xFF808080,BrightDecr(fillColor,		0x44), bkColor, 0.0, Down); 	param.color[0].bk = GetTransitionColor(0xFF808080,BrightDecr(fillColor,		 0x44),0.5); }	/* bkColor for pShape is fillColor */
	pShape(0,param);
	LCD_Display(0, s[nr].x+pos.x, s[nr].y+pos.y, s[nr].widthKey, s[nr].heightKey);
}
static void KeyShapeDisp_Win(int nr, XY_Touch_Struct pos, ShapeFunc pShape, SHAPE_PARAMS param, int releasePress){
	if(releasePress){ LCD_ShapeWindow( s[nr].shape, 0, param.bkSize.w, param.bkSize.h, pos.x,pos.y, s[nr].widthKey,s[nr].heightKey, SetBold2Color(framePressColor,s[nr].bold),fillPressColor,bkColor); 	param.color[0].bk = fillPressColor; }	/* bkColor for pShape is fillPressColor */
	else				 { LCD_ShapeWindow( s[nr].shape, 0, param.bkSize.w, param.bkSize.h, pos.x,pos.y, s[nr].widthKey,s[nr].heightKey, SetBold2Color(frameColor,	  s[nr].bold),fillColor,	  bkColor); 	param.color[0].bk = fillColor; 		}	/* bkColor for pShape is fillColor */
	pShape(0,param);
}
static void KeyShapeDisp2_Win(int nr, XY_Touch_Struct pos, ShapeFunc pShape, SHAPE_PARAMS param, int releasePress){
	if(releasePress){ LCD_RoundRectangle2(0,PARAM32(Up,  bold2,unUsed,Rectangle), widthAll,heightAll, pos.x,pos.y, s[nr].widthKey, s[nr].heightKey, BrightIncr(framePressColor,0x65),BrightDecr(framePressColor,0x20), 0xFF808080,BrightDecr(fillPressColor,0x44), bkColor, 0.0, Up  ); 	param.color[0].bk = GetTransitionColor(0xFF808080,BrightDecr(fillPressColor,0x44),0.5); }	/* bkColor for pShape is fillPressColor */
	else				 { LCD_RoundRectangle2(0,PARAM32(Down,bold2,unUsed,Rectangle), widthAll,heightAll, pos.x,pos.y, s[nr].widthKey, s[nr].heightKey, BrightDecr(frameColor,		0x20),BrightIncr(frameColor,		0x65), 0xFF808080,BrightDecr(fillColor,	  0x44), bkColor, 0.0, Down); 	param.color[0].bk = GetTransitionColor(0xFF808080,BrightDecr(fillColor,		 0x44),0.5); }	/* bkColor for pShape is fillColor */
	pShape(0,param);
}

static void KeyShapePressDisp_win(int nr, XY_Touch_Struct pos, ShapeFunc pShape, SHAPE_PARAMS param){
	LCD_ShapeWindow( s[nr].shape, 0, widthAll,heightAll, pos.x,pos.y, s[nr].widthKey,s[nr].heightKey, SetBold2Color(framePressColor,s[nr].bold),fillPressColor,bkColor);		/* draw some rectangle for background */
	pShape(0,param);		/* draw some shape */
}


static void KeyShapePressDisp_win_alt(int nr, XY_Touch_Struct pos, ShapeFunc pShape, SHAPE_PARAMS param, int altBk){
	if(altBk) LCD_RoundRectangle2(0,PARAM32(Up,	bold2,unUsed,Rectangle), widthAll,heightAll, pos.x,pos.y, s[nr].widthKey, s[nr].heightKey, BrightIncr(framePressColor,0x65),BrightDecr(framePressColor,0x20), 0xFF808080,BrightDecr(fillPressColor,0x44), bkColor, 0.0, Up  );
	else 		 LCD_RoundRectangle2(0,PARAM32(Down,bold2,unUsed,Rectangle), widthAll,heightAll, pos.x,pos.y, s[nr].widthKey, s[nr].heightKey, BrightDecr(framePressColor,0x20),BrightIncr(framePressColor,0x65), 0xFF808080,BrightDecr(fillPressColor,0x44), bkColor, 0.0, Down);		/* draw some rectangle for background */
	pShape(0,param);		/* draw some shape */
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

static SlidButtCircParam SlidButtCircChangeParam(int nrParam, int nrElement, u32 color, int brightStep){
	SlidButtCircParam slidParam={0};
	switch(nrParam){
	case 1:
		slidParam.dir = Center;
		slidParam.gradSlidColor = (u64)BrightDecr(color,brightStep)<<32 | color;
		slidParam.middSlidColor = 0;
		slidParam.gradButtColor = (u64)0xC0C0C0<<32 | 0x333333;
		break;
	case 2:
		slidParam.dir = Center;
		slidParam.gradSlidColor = (u64)BrightDecr(color,brightStep)<<32 | color;
		slidParam.middSlidColor = BrightDecr(color,brightStep);
		slidParam.gradButtColor = (u64)0xC0C0C0<<32 | 0x333333;
		break;
	case 3:
		slidParam.dir = Round;
		slidParam.gradSlidColor = (u64)BrightDecr(color,brightStep)<<32 | color;
		slidParam.middSlidColor = 0;
		slidParam.gradButtColor = (u64)0xC0C0C0<<32 | 0x333333;
		break;
	case 4:
		switch(nrElement){
		case 0:
			slidParam.dir = Round;
			slidParam.gradSlidColor = (u64)0x505050<<32 | 0xC0C0C0;
			slidParam.middSlidColor = 0;
			slidParam.gradButtColor = (u64)LIGHTRED<<32 | VERYDARKRED;
			break;
		case 1:
			slidParam.dir = Round;
			slidParam.gradSlidColor = (u64)0x505050<<32 | 0xC0C0C0;
			slidParam.middSlidColor = 0;
			slidParam.gradButtColor = (u64)GREEN<<32 | VERYDARKGREEN;
			break;
		case 2:
			slidParam.dir = Round;
			slidParam.gradSlidColor = (u64)0x505050<<32 | 0xC0C0C0;
			slidParam.middSlidColor = 0;
			slidParam.gradButtColor = (u64)LIGHTBLUE<<32 | DARKBLUE;
			break;
		}
		break;
	case 5:
		slidParam.dir = Round;
		slidParam.gradSlidColor = (u64)RED<<32 | DARKGREEN;
		slidParam.middSlidColor = BLUE;
		slidParam.gradButtColor = (u64)0xC0C0C0<<32 | 0x333333;
		break;
	}
	return slidParam;
}

static void KeysAllRelease_CircleSlider(int nr, XY_Touch_Struct posKeys[],int *value){
	XY_Touch_Struct posK;
	uint16_t _GetDegFromVal(int val){ return ((360*val)/255); }

	for(int i=0; i<dimKeys[0]*dimKeys[1]; ++i)
	{
		uint16_t deg[2] = {0, _GetDegFromVal(*(value+i)) };
		uint32_t degColor[2] = {0, CONDITION(deg[1]==deg[0],fillColor,colorTxtPressKey[i]) };
		char *pTxt= Int2Str(*(value+i),Zero,3,Sign_none);
		posK = posKeys[i];	posK.y -= VALPERC(GetHeightFontDescr(),40);		TxtDescrMidd_WidthKey(nr, posK, txtKey[i], colorTxtPressKey[i]);

		if(s[nr].param2){
			SHAPE_PARAMS par={0};
			int brightStep = 0;
			if(degColor[1]==fillColor) brightStep= 0;
			else								brightStep= (i==1) ? 0xB0/2 : 0xB0;
			SlidButtCircParam slidParam = SlidButtCircChangeParam(s[nr].param2,i,degColor[1],brightStep);

			if(s[nr].bold)
				par=LCD_GradientCircleSlider(0,widthAll,heightAll, posKeys[i].x, posKeys[i].y, s[nr].widthKey,s[nr].heightKey, SetBold2Color(frameColor,s[nr].bold),fillColor, slidParam.gradSlidColor>>32, slidParam.gradSlidColor, slidParam.middSlidColor, SetBold2Color(fillColor,11), slidParam.gradButtColor>>32, slidParam.gradButtColor, bkColor,deg[1],slidParam.dir,0);
			else
				par=LCD_GradientCircleSlider(0,widthAll,heightAll, posKeys[i].x, posKeys[i].y, s[nr].widthKey,s[nr].heightKey, SetBold2Color(frameColor,s[nr].bold),fillColor, slidParam.gradSlidColor>>32, slidParam.gradSlidColor, slidParam.middSlidColor, unUsed,							  	unUsed,							  unUsed,  					   bkColor,deg[1],slidParam.dir,0);
			if(IS_RANGE(s[nr].bold, 1, (LCD_GetCircleWidth()-GetStrPxlWidth(fontID_descr, StrAll(3," ",pTxt," "), ConstWidth))/2) ){
				LCD_BkFontTransparent(fontVar_40, fontID_descr);
				LCD_StrDependOnColorsWindowMidd(0,widthAll,heightAll,FONT_ID_VAR(fontID_descr,fontVar_40), POS_SIZE_CIRCLEBUTTONSLIDER(par,-2,-2), pTxt, fullHight,0, unUsed /*BK_COLOR_CIRCLESLIDER(par)*/, WHITE, 250, ConstWidth);
			}
		}
		else{
			LCD_SetCirclePercentParam(2,deg,(uint32_t*)degColor);
			LCD_Circle(0, widthAll,heightAll, posKeys[i].x, posKeys[i].y, SetParamWidthCircle(Percent_Circle,s[nr].widthKey),s[nr].heightKey, SetBold2Color(frameColor,s[nr].bold), fillColor, bkColor);
			if(IS_RANGE(s[nr].bold, 1, (LCD_GetCircleWidth()-GetStrPxlWidth(fontID_descr, StrAll(3," ",pTxt," "), ConstWidth))/2) ){
				XY_Touch_Struct posTemp={ posKeys[i].x-2, posKeys[i].y-2 };
				StrKeyMidd(nr, posTemp, pTxt, BrightIncr(colorDescr,0x30), fontID_descr,ConstWidth);
			}
		}
	}
	LCD_Display(0, s[nr].x, s[nr].y, widthAll, heightAll);
}

static void KeyPress_CircleSlider(int nr, uint16_t x,uint16_t y, XY_Touch_Struct posKeys, float radius, int *value, VOID_FUNCTION *pfunc, uint32_t colorPress, int nrElement)
{
	uint32_t _GetPosX(void){	return x-(s[nr].x+posKeys.x); }
	uint32_t _GetPosY(void){	return y-(s[nr].y+posKeys.y); }
	uint16_t _GetValFromDeg(uint16_t deg){
		return ((255*deg)/360);
	}
	uint16_t _GetDegFromPosX(void){    //to moze jako funkcje by latwo uzywac !!!!!
		uint16_t deg=0;
		float radi = sqrt( (radius-(float)_GetPosX())*(radius-(float)_GetPosX()) + (radius-(float)_GetPosY())*(radius-(float)_GetPosY()) );
		float stretch = radius-(float)_GetPosX();
		deg = (uint16_t)(57.295*acos(stretch/radi));
		if(_GetPosY() > (uint16_t)radius)
			deg = 360 - deg;
		return deg;
	}
	INIT(xPosFromMidd_Pow2, ((int)radius-(int)_GetPosX()) );		xPosFromMidd_Pow2 *= xPosFromMidd_Pow2;  //to moze jako funkcje by latwo uzywac !!!!!
	INIT(yPosFromMidd_Pow2, ((int)radius-(int)_GetPosY()) );		yPosFromMidd_Pow2 *= yPosFromMidd_Pow2;
	INIT(radiusTouch_Pow2, xPosFromMidd_Pow2 + yPosFromMidd_Pow2 );

	if( IS_RANGE(radiusTouch_Pow2, (int)(VALPERC(radius,45)*VALPERC(radius,45)), (int)(radius*radius)) )
	{
		int degPosX = _GetDegFromPosX();
		uint16_t deg[2] = {0, degPosX};
		uint32_t degColor[2] = {0,CONDITION(deg[1]==deg[0],fillColor,colorPress)};
		char *pTxt= Int2Str(*value,Zero,3,Sign_none);
		int _ShowTxtFunc(void){	 return IS_RANGE(s[nr].bold, 1, (LCD_GetCircleWidth()-GetStrPxlWidth(fontID_descr, StrAll(3," ",pTxt," "), ConstWidth))/2); }

		*value = _GetValFromDeg(degPosX);

		if(s[nr].param2){
			SHAPE_PARAMS par={0};
			int brightStep = 0;
			if(degColor[1]==fillColor) brightStep= 0;
			else								brightStep= (nrElement==1) ? 0xB0/2 : 0xB0;
			SlidButtCircParam slidParam = SlidButtCircChangeParam(s[nr].param2,nrElement,degColor[1],brightStep);

			if(s[nr].bold) par=LCD_GradientCircleSlider(ToStructAndReturn,s[nr].widthKey,s[nr].heightKey, 0,0, s[nr].widthKey,s[nr].heightKey,   SetBold2Color(frameColor,s[nr].bold),fillColor, slidParam.gradSlidColor>>32, slidParam.gradSlidColor, slidParam.middSlidColor, SetBold2Color(fillColor,11), slidParam.gradButtColor>>32, slidParam.gradButtColor, bkColor,deg[1],slidParam.dir,0);
			else				par=LCD_GradientCircleSlider(ToStructAndReturn,s[nr].widthKey,s[nr].heightKey, 0,0, s[nr].widthKey,s[nr].heightKey,   SetBold2Color(frameColor,s[nr].bold),fillColor, slidParam.gradSlidColor>>32, slidParam.gradSlidColor, slidParam.middSlidColor, unUsed,unUsed,unUsed,  								  												 bkColor,deg[1],slidParam.dir,0);
			ShapeBkClear(nr, par.bkSize.w, par.bkSize.h, bkColor);
			LCDSHAPE_GradientCircleSlider(0,par);

			if(_ShowTxtFunc()){
				LCD_BkFontTransparent(fontVar_40, fontID_descr);
				LCD_StrDependOnColorsWindowMidd(0, par.bkSize.w, par.bkSize.h, FONT_ID_VAR(fontID_descr,fontVar_40), POS_SIZE_CIRCLEBUTTONSLIDER(par,-2,-2), pTxt, fullHight,0, unUsed /*BK_COLOR_CIRCLESLIDER(par)*/, WHITE, 250, ConstWidth);
			}
			int correctXY = LCD_GradCircButtSlidCorrectXY(par,s[nr].widthKey);
			LCD_Display(0, s[nr].x+posKeys.x+correctXY, s[nr].y+posKeys.y+correctXY,  par.bkSize.w, par.bkSize.h);
		}
		else{
			LCD_SetCirclePercentParam(2,deg,(uint32_t*)degColor);
			ShapeBkClear(nr, s[nr].widthKey,s[nr].heightKey, bkColor);
			LCD_Circle(0, s[nr].widthKey,s[nr].heightKey, 0,0, SetParamWidthCircle(Percent_Circle,s[nr].widthKey),s[nr].heightKey, SetBold2Color(frameColor,s[nr].bold), fillColor, bkColor);

			if(_ShowTxtFunc())
				StrWinKeyMidd(nr, pTxt, BrightIncr(colorDescr,0x30), fontID_descr,ConstWidth);

			LCD_Display(0, s[nr].x+posKeys.x, s[nr].y+posKeys.y, s[nr].widthKey, s[nr].heightKey);
		}
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
	if(startTouchIdx){
	if(0 != field.width)
	{
		void _FuncTemp(void){
			touchTemp[0].x= s[nr].x + field.x;
			touchTemp[1].x= touchTemp[0].x + field.width;
			touchTemp[0].y= s[nr].y + field.y;
			touchTemp[1].y= touchTemp[0].y + field.height;
		}
		_FuncTemp();
		LCD_TOUCH_Set(ID_TOUCH_POINT_RELEASE_WITH_HOLD, s[nr].startTouchIdx + GetPosKeySize(), LCD_TOUCH_SetTimeParam_ms(400));
		s[nr].nmbTouch++;

		_FuncTemp();
		LCD_TOUCH_Set(ID_TOUCH_POINT_WITH_HOLD, s[nr].startTouchIdx + GetPosKeySize() +1, LCD_TOUCH_SetTimeParam_ms(500));
		s[nr].nmbTouch++;
}}}

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

static void DispTxtIndirect(char *txt, u32 xPos,u32 yPos, u32 fontColor,u32 bkColor){
	LCD_TxtShadowInit(fontVar_40, fontID, bkColor, BK_Rectangle);
	LCD_Txt(DisplayIndirect, NULL, xPos,yPos, BK_SIZE_IS_TXT_SIZE, textParam.fontId, fontVar_40, 0,0, txt, fontColor,bkColor, textParam.OnlyDigits, textParam.space, textParam.maxVal, textParam.constWidth, textParam.shadeColor, textParam.deep, textParam.dir);
}
static void DispTxt(char *txt, u32 Xpos,u32 Ypos, u32 fontColor,u32 bkColor, u32 BkpSizeX,u32 BkpSizeY){
	LCD_TxtShadowInit(fontVar_40, fontID, bkColor, BK_Rectangle);
	LCD_Txt(Display,NULL, unUsed,unUsed, BkpSizeX,BkpSizeY, textParam.fontId, fontVar_40, Xpos,Ypos, txt, fontColor,bkColor, textParam.OnlyDigits, textParam.space, textParam.maxVal, textParam.constWidth, textParam.shadeColor, textParam.deep, textParam.dir);
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

void KEYBOARD_ServiceCircleSliderRGB(int k, int selBlockPress, INIT_KEYBOARD_PARAM, int TOUCH_Release, int TOUCH_Action, int touchTimer, char* txtDescr, int *value, VOID_FUNCTION *pfunc, TIMER_ID timID)
{
	int head 	 = GetHeightHead(k);									/* 'Color1Txt' is no press color for: outline, pointer */
	int interSp  = VALPERC(s[k].interSpace,60);					/* 'Color2Txt' is 	press color for: pointer, lineSel */
	XY_Touch_Struct posKey[GetPosKeySize()];
	StructFieldPos fieldTxtDescr = {0};
	static LCD_STR_PARAM parTxt={0};

	LCD_SetCircleAA(0.0, 0.0);
	CorrectLineAA_on();

	SetPosKey(k,posKey,interSp,head);
	SetDimAll(k,interSp,head);

	void _ChangeFrameColor(int type){
		if(type){	if(s[k].bold)  frameColor= bkColor;	}
		else	  {	if(s[k].bold)  frameColor= BrightDecr(frameColor,0x39);	}
	}
	void _FuncAllRelease(int releasePress){
		ShapeWin(k,widthAll,heightAll);
		if(shape!=0) fieldTxtDescr = StrDescrParam(k, structXY_Zero, txtDescr, colorDescr, &parTxt);
		else			 fieldTxtDescr = StrDescrParam(k, structXY_Zero, txtDescr, CONDITION(press==releasePress,WHITE,colorDescr), NULL);
		_ChangeFrameColor(s[k].param);
		KeysAllRelease_CircleSlider(k, posKey,value);
	}

	bkColor = fillMainColor;
	if(TOUCH_Release == selBlockPress) _FuncAllRelease(release);
	else if(touchTimer+0 == selBlockPress){ LCD_StrDependOnColorsWindowIndirectParam(parTxt); }
	else
	{	INIT(nrCircSlid, selBlockPress-TOUCH_Action);
		float radius = ((float)LCD_GetCircleWidth())/2;

		if(nrCircSlid  < GetPosKeySize()){
			_ChangeFrameColor(s[k].param);
			KeyPress_CircleSlider(k, x,y, posKey[nrCircSlid], radius, value+nrCircSlid, pfunc, colorTxtPressKey[nrCircSlid], nrCircSlid);
		}
		else if(nrCircSlid == GetPosKeySize()){
			if(TOOGLE(s[k].param) || 0==s[k].bold) s[k].bold= LCD_IncrWrapPercCircleBold(radius, s[k].bold, 20,80, 10);
			_FuncAllRelease(press);
			vTimerService(timID+0,restart_time,noUse);
		}
		else if(nrCircSlid == GetPosKeySize()+1){
			INCR_WRAP(s[k].param2,1,0,5);
			_FuncAllRelease(press);
			vTimerService(timID+0,restart_time,noUse);
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
{
	#define _NMB2KEY	8
	const char *txtKey[]								= {"(.......)", "(...)", "(.......)",	"(.......)", " _ ", " _ ", "|  |", "||", "Info spaces", "Write spaces", "Reset spaces" };
	const COLORS_DEFINITION colorTxtKey[]		= {	WHITE,  	  WHITE,     WHITE,  		WHITE,  	 WHITE, WHITE,  WHITE, WHITE, 	WHITE,		 	WHITE,			  WHITE  };
	const COLORS_DEFINITION colorTxtPressKey[]= {  DARKRED,	 DARKRED,	DARKRED,		  DARKRED,	DARKRED,DARKRED,DARKRED,DARKRED,	DARKRED, 	  DARKRED,		  	 DARKRED };
	const uint16_t dimKeys[] = {4,3};

	extern uint32_t pLcd[];
	int retVal=0;
	int widthKeyCorrect = 0;
	int heightKeyCorrect = 5;
	if(shape!=0){		/* Do only once when creating Keyboard */
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
	void 	_SetFlagTest (void){	SET_bit(s[k].param,5); }
	void 	_RstFlagTest (void){	RST_bit(s[k].param,5); }
	int 	_IsFlagTest	 (void){ return CHECK_bit(s[k].param,5); }

	int heightUpDn = 17;
	int widthtUpDn = 26;
	int spaceBetweenArrows = 10;
	int spaceFromFrame = 10;
	int space2UpDn = spaceFromFrame + heightUpDn + spaceFromFrame;
	uint32_t bkColorSel = 0xFF232323;

	static LIST_TXT struct_ListTxt={0};

	static uint32_t 	posTxtTab[MAX_SCREENS_FOR_LCD_LIST]={0};
	static uint32_t 	isFilledWin	  =0;
	static int 			maxScreens	  =0;
	static uint32_t 	i_posTxtTab	  =0;
	static int 			nmbrLinesInWin=0;
	static int 			nrItemSel	  =-1;

	static LCD_STR_PARAM paramTxt		  ={0}, paramTxt2	  ={0};
	static SHAPE_PARAMS 	arrowUpParam  ={0}, arrowDnParam={0};
	static int 				paramCreateWin=0;
	static int 				calcWinWidth  =200;

	void _SetInitStaticVar(void){
		LOOP_FOR(i,MAX_SCREENS_FOR_LCD_LIST){ posTxtTab[i]=0; }
		struct_ListTxt=LIST_TXT_Zero;		 paramTxt	 	=LCD_STR_PARAM_Zero;
		arrowUpParam  =SHAPE_PARAMS_Zero, arrowDnParam	=SHAPE_PARAMS_Zero,
		maxScreens	  =0;						 paramCreateWin=0;
		i_posTxtTab	  =0;						 calcWinWidth	=200;
		nmbrLinesInWin=0;						 isFilledWin	=0;
		nrItemSel	  =-1;
	}

	POS_SIZE win = { .pos={ s[k].x+widthAll+1, s[k].y 				 	}, .size={calcWinWidth,heightAll} };
	POS_SIZE win2 ={ .pos={ 15, 					 s[k].y+heightAll+15 }, .size={600, 		  60} };

	if(shape!=0){}
	void _WinInfo(char* txt){
		WinInfo(txt, win2.pos.x, win2.pos.y, win2.size.w, win2.size.h, timID);
	}
	void _WindowSpacesInfo(uint16_t x,uint16_t y, uint16_t width,uint16_t height, int param, int spaceFromFrame, int nmbrStrip){
		int xPosD = width-spaceFromFrame-widthtUpDn;
		int xPosU = xPosD-spaceBetweenArrows-widthtUpDn;
		int yPosUD = height-spaceFromFrame-heightUpDn;

		uint32_t tabFontColor[]={BrightDecr(colorDescr,0x20),GREEN,CYAN,colorDescr,colorDescr,MYRED,ORANGE};

		uint32_t *pTabFontColor = (uint32_t*)pvPortMalloc(nmbrStrip*sizeof(uint32_t));
		LOOP_FOR(i,nmbrStrip){ pTabFontColor[i]=colorDescr; }
		LOOP_FOR(i,nmbrStrip){
			if(i >= STRUCT_TAB_SIZE(tabFontColor)) break;
			pTabFontColor[i] = tabFontColor[i];
		}

		if(NoDirect==param){
			i_posTxtTab=0;
		}
		else if(Up==param){
			CONDITION(1<i_posTxtTab && 0<isFilledWin, i_posTxtTab-=2, i_posTxtTab--);
		}
		else if((inside==param || inside2==param || Horizontal==param) && isFilledWin){		/* click change style and text test */
			if(0<i_posTxtTab) i_posTxtTab--;
		}

		LCD_ShapeWindow( s[k].shape, 0, width,height, 0,0, width,height, SetBold2Color(frameColor,s[k].bold), bkColor,bkColor );
		int nrLineSel = LCD_LIST_TXT_sel(nrItemSel,i_posTxtTab,nmbrLinesInWin);
		if(IS_RANGE(nrLineSel,0,nmbrLinesInWin-1))
			LCD_ShapeWindow( s[k].shape, 0, width,height, s[k].bold+1,spaceFromFrame+nrLineSel*GetHeightFontDescr(), width-2*(s[k].bold+1),GetHeightFontDescr(), bkColorSel, bkColorSel,bkColorSel );

	/* if(TakeMutex(Semphr_sdram,3000)){ */
			char *ptr = CONDITION(_IsFlagTest(), LCD_LIST_TXT_example(pCHAR_PLCD(width*height),NULL), LCD_DisplayRemeberedSpacesBetweenFonts(1,pCHAR_PLCD(width*height),NULL));
			isFilledWin = LCD_ListTxtWin(0,width,height,fontID_descr,spaceFromFrame,spaceFromFrame,ptr,posTxtTab[i_posTxtTab],nrLineSel,fullHight,0,bkColor,bkColorSel,colorDescr,FONT_COEFF,NoConstWidth, pTabFontColor, _GetTxtArrang(),space2UpDn,struct_ListTxt);
		/* GiveMutex(Semphr_sdram);
		} */
		vPortFree(pTabFontColor);
		if(isFilledWin) i_posTxtTab++;	/* if whole Win list is full */

		arrowUpParam.pos[0].x = 0;
		arrowDnParam.pos[0].x = 0;

		mini_snprintf(paramTxt.str, MAX_TXT_SIZE__LCD_STR_PARAM-1,"%s (%d)",Int2Str(CONDITION(isFilledWin,i_posTxtTab,i_posTxtTab+1),' ',3,Sign_none), maxScreens);
		mini_snprintf(paramTxt2.str,MAX_TXT_SIZE__LCD_STR_PARAM-1,"Test");
		int len_temp  = GetStrPxlWidth(fontID_descr,paramTxt.str, NoConstWidth);
		int len_temp2 = GetStrPxlWidth(fontID_descr,paramTxt2.str,NoConstWidth);
		int xPosTxt  = xPosU-2*spaceBetweenArrows-len_temp;
		int xPosTxt2 = spaceFromFrame;
		paramTxt =LCD_SetStrDescrParam(win.pos.x+xPosTxt,  win.pos.y+yPosUD,  len_temp,  GetHeightFont(fontID_descr), 0,0, unUsed,unUsed, fontID_descr,paramTxt.str, fullHight,0,bkColor,YELLOW,	 FONT_COEFF,NoConstWidth);
		paramTxt2=LCD_SetStrDescrParam(win.pos.x+xPosTxt2, win.pos.y+yPosUD,  len_temp2, GetHeightFont(fontID_descr), 0,0, unUsed,unUsed, fontID_descr,paramTxt2.str,fullHight,0,bkColor,colorDescr,FONT_COEFF,NoConstWidth);
		LCD_StrDependOnColorsWindow(0,width,height,paramTxt.fontId,  xPosTxt, yPosUD, paramTxt.str,  paramTxt.onlyDig, paramTxt.spac, paramTxt.bkCol, CONDITION(inside ==param,RED,paramTxt.fontCol), paramTxt.maxV, paramTxt.constW);
		LCD_StrDependOnColorsWindow(0,width,height,paramTxt2.fontId, xPosTxt2,yPosUD, paramTxt2.str, paramTxt2.onlyDig,paramTxt2.spac,paramTxt2.bkCol,CONDITION(_IsFlagTest(), RED,paramTxt2.fontCol),paramTxt2.maxV,paramTxt2.constW);

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

			touchTemp[0].x= win.pos.x + xPosTxt;
			touchTemp[1].x= touchTemp[0].x + len_temp;
			touchTemp[0].y= win.pos.y + yPosUD;
			touchTemp[1].y= touchTemp[0].y + heightUpDn;
			LCD_TOUCH_Set(ID_TOUCH_POINT,touchAction2+2,press);
			s[k].nmbTouch++;

			touchTemp[0].x= win.pos.x + VALPERC(win.size.w,80);
			touchTemp[1].x= win.pos.x + win.size.w;
			touchTemp[0].y= win.pos.y + spaceFromFrame;
			touchTemp[1].y= win.pos.y + height - space2UpDn;
			LCD_TOUCH_Set(ID_TOUCH_GET_ANY_POINT,touchAction2+3,TOUCH_GET_PER_ANY_PROBE);
			s[k].nmbTouch++;

			touchTemp[0].x= win.pos.x;
			touchTemp[1].x= win.pos.x + VALPERC(win.size.w,80);
			touchTemp[0].y= win.pos.y + spaceFromFrame;
			touchTemp[1].y= win.pos.y + height - space2UpDn;
			LCD_TOUCH_Set(ID_TOUCH_POINT,touchAction2+4,press);
			s[k].nmbTouch++;

			touchTemp[0].x= win.pos.x + xPosTxt2;
			touchTemp[1].x= touchTemp[0].x + len_temp2;
			touchTemp[0].y= win.pos.y + yPosUD;
			touchTemp[1].y= touchTemp[0].y + heightUpDn;
			LCD_TOUCH_Set(ID_TOUCH_POINT,touchAction2+5,release);
			s[k].nmbTouch++;

			LCD_TOUCH_SusspendTouch(s[k].forTouchIdx);
		}
		LCD_TOUCH_SusspendTouch(touchAction2);
		LCD_TOUCH_SusspendTouch(touchAction2+1);

		if(1 < i_posTxtTab || (1==i_posTxtTab && 0==isFilledWin)){
			uint32_t colorUp=colorDescr;
			if(Up==param) colorUp=BrightIncr(colorDescr,0x40);
			LCDSHAPE_Window(LCDSHAPE_Arrow,0,arrowUpParam=LCD_Arrow(ToStructAndReturn,width,height, xPosU,yPosUD, SetLineBold2Width(widthtUpDn,7), SetTriangHeightCoeff2Height(heightUpDn,3), colorUp,colorUp,bkColor, Up));
			arrowUpParam.pos[0].x += win.pos.x;
			arrowUpParam.pos[0].y += win.pos.y;
			arrowUpParam.bkSize.w = widthtUpDn;
			arrowUpParam.bkSize.h = heightUpDn;
			arrowUpParam.color[0].frame=colorDescr;
			arrowUpParam.color[0].fill=colorDescr;
			LCD_TOUCH_RestoreSusspendedTouch(touchAction2);
		}
		if(0 < isFilledWin){
			uint32_t colorDn=colorDescr;
			if(Down==param) colorDn=BrightIncr(colorDescr,0x40);
			LCDSHAPE_Window(LCDSHAPE_Arrow,0,arrowDnParam=LCD_Arrow(ToStructAndReturn,width,height, xPosD,yPosUD, SetLineBold2Width(widthtUpDn,7), SetTriangHeightCoeff2Height(heightUpDn,3), colorDn,colorDn,bkColor, Down));
			arrowDnParam.pos[0].x += win.pos.x;
			arrowDnParam.pos[0].y += win.pos.y;
			arrowDnParam.bkSize.w = widthtUpDn;
			arrowDnParam.bkSize.h = heightUpDn;
			arrowDnParam.color[0].frame=colorDescr;
			arrowDnParam.color[0].fill=colorDescr;
			LCD_TOUCH_RestoreSusspendedTouch(touchAction2+1);
		}
		LCD_Display(0, x,y, width,height);
	}

	void _CreateWindows(int nr,int param){
		paramCreateWin=param;
		switch(nr){
			case 0:
				_WindowSpacesInfo(win.pos.x ,win.pos.y, win.size.w, win.size.h, param, spaceFromFrame, struct_ListTxt.nmbrStrips); 	_SetFlagWin();
				break;
			case 1:
				break;
	}}
	void _DeleteAdditionalTouches(void){
		LCD_TOUCH_DeleteSelectAndSusspendTouch(touchAction2);
		LCD_TOUCH_DeleteSelectAndSusspendTouch(touchAction2+1);
		LCD_TOUCH_DeleteSelectTouch(touchAction2+2);
		LCD_TOUCH_DeleteSelectTouch(touchAction2+3);
		LCD_TOUCH_DeleteSelectTouch(touchAction2+4);
		LCD_TOUCH_DeleteSelectTouch(touchAction2+5);
		s[k].nmbTouch-=6;
	}
	void _DeleteWindows(void){		/* Use function only after displaying (not during) */
		pfunc(FUNC_MAIN_ARG);	_RstFlagWin(); _RstFlagTest(); LCD_DisplayPart(0,win.pos.x ,win.pos.y, win.size.w, win.size.h); retVal=1;
		_DeleteAdditionalTouches();	_SetInitStaticVar();
		LCD_TOUCH_RestoreSusspendedTouch(s[k].forTouchIdx);
	}
	int _TakeNewTxtList(void){
		_SetTxtArrang(TxtInRow);
	/* if(TakeMutex(Semphr_sdram,3000)){ */		 /* semaphores don`t give in internal functions ! */
			nmbrLinesInWin = LCD_LIST_TXT_len(CONDITION(_IsFlagTest(), LCD_LIST_TXT_example(pCHAR_PLCD(0),NULL), LCD_DisplayRemeberedSpacesBetweenFonts(1,pCHAR_PLCD(0),NULL)), TxtInRow, fontID_descr,0,NoConstWidth, &struct_ListTxt,posTxtTab,&maxScreens, win.size.h-spaceFromFrame-space2UpDn);
		/* GiveMutex(Semphr_sdram);
		} */
		if(struct_ListTxt.lenMaxWholeLine){
			win.size.w= struct_ListTxt.lenMaxWholeLine + 2*spaceFromFrame;
			calcWinWidth = win.size.w;
			return 0;
		}
		return 1;
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
			for(int i=0; i<_NMB2KEY; ++i){
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

			for(int i=_NMB2KEY; i<countKey; ++i){
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
	else if(IS_RANGE(selBlockPress,touchAction,touchAction+7)){
		switch(selBlockPress-touchAction){
			case 0:	_OverArrowTxt_oneBlockDisp(0,outside);		break;
			case 1:	_OverArrowTxt_oneBlockDisp(1,inside);		break;
			case 2:	_OverArrowTxt_oneBlockDisp(2,LeftLeft);	break;
			case 3:	_OverArrowTxt_oneBlockDisp(3,RightRight);	break;
			case 4:	_OverArrowTxt_oneBlockDisp(4,LeftLeft);	break;
			case 5:	_OverArrowTxt_oneBlockDisp(5,RightRight);	break;
			case 6:	_OverArrowTxt_oneBlockDisp(6,outside);		break;
			case 7:	_OverArrowTxt_oneBlockDisp(7,inside);		break;
	}}
	else if(touchAction+8 == selBlockPress){
		BKCOPY_VAL(c.widthKey,s[k].widthKey,s[k].widthKey2);   KeyStrPressDisp_oneBlock(k,posKey[8],txtKey[8],colorTxtPressKey[8]);	BKCOPY(s[k].widthKey,c.widthKey);
		if(_IsFlagWin()) _DeleteWindows();
		else{				  _TakeNewTxtList();	_CreateWindows(0,NoDirect); }
	}
	else if(touchAction+9  == selBlockPress){ BKCOPY_VAL(c.widthKey,s[k].widthKey,s[k].widthKey2);  KeyStrPressDisp_oneBlock(k,posKey[9],txtKey[9],colorTxtPressKey[9]);		BKCOPY(s[k].widthKey,c.widthKey); CONDITION(_IsFlagWin(),_DeleteWindows(),NULL);	 _WinInfo(txtDescr);}
	else if(touchAction+10 == selBlockPress){ BKCOPY_VAL(c.widthKey,s[k].widthKey,s[k].widthKey2);  KeyStrPressDisp_oneBlock(k,posKey[10],txtKey[10],colorTxtPressKey[10]);	BKCOPY(s[k].widthKey,c.widthKey); CONDITION(_IsFlagWin(),_DeleteWindows(),NULL);  _SetInitStaticVar(); _WinInfo(txtDescr2); }

	else if(touchAction+11 == selBlockPress){ 						  _CreateWindows(0,Up); 	 vTimerService(timID+1,restart_time,noUse); }
	else if(touchAction+12 == selBlockPress){ 						  _CreateWindows(0,Down);   vTimerService(timID+1,restart_time,noUse); }
	else if(touchAction+13 == selBlockPress){ _ToggleTxtArrang(); _CreateWindows(0,inside); vTimerService(timID+1,restart_time,noUse); }
	else if(touchAction+14 == selBlockPress){
		GetTouchToTemp(touchAction2+3);
		i_posTxtTab = ( ((y-touchTemp[0].y)* maxScreens) / (touchTemp[1].y-touchTemp[0].y) );
		_CreateWindows(0,DownUp);
	}
	else if(touchAction+15 == selBlockPress){
		GetTouchToTemp(touchAction2+4);
		int temp = (y-touchTemp[0].y)/GetHeightFont(fontID_descr);
		if(temp > nmbrLinesInWin-1) temp=nmbrLinesInWin-1;
		temp = temp + CONDITION(isFilledWin,i_posTxtTab-1,i_posTxtTab)*nmbrLinesInWin;
		if(temp==nrItemSel) nrItemSel=-1;
		else					  nrItemSel=temp;
		if(nrItemSel > struct_ListTxt.nmbrAllLines-1) nrItemSel=-1;
		 _CreateWindows(0,Horizontal);
	}
	else if(touchAction+16 == selBlockPress){
		int width_prev= win.size.w;
		int delta_width=0;
		_DeleteAdditionalTouches();  _SetInitStaticVar();
		if(_IsFlagTest()){
			_RstFlagTest();
			if(_TakeNewTxtList())
				win.size.w=calcWinWidth;
			delta_width= width_prev - win.size.w;
			if(delta_width > 0){
				pfunc(FUNC_MAIN_ARG); 	/* Semphr_sdram ? */
				LCD_DisplayPart(0,win.pos.x+width_prev-delta_width ,win.pos.y, delta_width, win.size.h);
			}
			_CreateWindows(0,NoDirect);
		}
		else{
			_SetFlagTest();
			if(_TakeNewTxtList())
				win.size.w=calcWinWidth;
			delta_width= width_prev - win.size.w;
			if(delta_width > 0){
				pfunc(FUNC_MAIN_ARG); 	/* Semphr_sdram ? */
				LCD_DisplayPart(0,win.pos.x+width_prev-delta_width ,win.pos.y, delta_width, win.size.h);
			}
			_CreateWindows(0,NoDirect);
		}
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

void KEYBOARD__ServiceSetTxt(int k, int selBlockPress, INIT_KEYBOARD_PARAM, int touchRelease, int touchAction, int tBig,int tBack,int tAlt,int tEnter,int tField,uint32_t colorDescr, char *charBuff,int charBuffSize) // klawiatura malutka i duza na caly LCD_X z liczbami
{
	#define TXTFIELD_COLOR		BrightDecr(frameColor,0x40)
	#define _UP		"|"
	#define _LF		"<--"
	#define _EN		"<-|"
	#define _EX		"exit"
	#define _AL		"alt"

	const char *txtKey[]								= {"q","w","e","r","t","y","u","i","o","p", \
																  "a","s","d","f","g","h","j","k","l", \
																_UP,"z","x","c","v","b","n","m",_LF, \
																_AL,_EX,"space",",",".",_EN };

	const char *txtBigKey[]							= {"Q","W","E","R","T","Y","U","I","O","P", \
																  "A","S","D","F","G","H","J","K","L", \
																_UP,"Z","X","C","V","B","N","M",_LF, \
																_AL,_EX,"space",",",".",_EN };

	const char *txtAltKey[]							= {" "," ", ę ," "," "," "," "," ", ó ," ", \
																   ą , ś ," "," "," "," "," "," ", ł , \
																_UP, ź , ż , ć ," "," ", ń ," ",_LF, \
																_AL,_EX,"space",",",".",_EN };
//
//	const char *txtAltBigKey[]						= {"Q","W","E","R","T","Y","U","I","O","P", \
//																  "A","S","D","F","G","H","J","K","L", \
//																_UP,"Z","X","C","V","B","N","M",_LF, \
//																"alt",_EX,"space",",",".",_EN };

//		const char *txtKey2[]							= {"/","1","2","3", \
//																	"*","4","5","6", \
//																	"-","7","8","9", \
//																	"+","=",".","0" };  //Sign 'S' to klawiatyra liczbnowa dla malej klawiatury

	const COLORS_DEFINITION colorTxtKey[]		= {WHITE,WHITE,WHITE,WHITE,WHITE,WHITE,WHITE,WHITE,WHITE,WHITE, \
																WHITE,WHITE,WHITE,WHITE,WHITE,WHITE,WHITE,WHITE,WHITE, \
																WHITE,WHITE,WHITE,WHITE,WHITE,WHITE,WHITE,WHITE,WHITE, \
																WHITE,WHITE,WHITE,WHITE,WHITE,WHITE };

	const COLORS_DEFINITION colorTxtPressKey[]= {DARKRED,DARKRED,DARKRED,DARKRED,DARKRED,DARKRED,DARKRED,DARKRED,DARKRED,DARKRED, \
																DARKRED,DARKRED,DARKRED,DARKRED,DARKRED,DARKRED,DARKRED,DARKRED,DARKRED, \
																DARKRED,DARKRED,DARKRED,DARKRED,DARKRED,DARKRED,DARKRED,DARKRED,DARKRED, \
																DARKRED,DARKRED,DARKRED,DARKRED,DARKRED,DARKRED };

	const uint8_t dimKeys[] = {10,9,9,6};
	static int charBuffOffs;  //to mozna dac do s[k].param2 !!!!
	char **pTxtKey = (char**) CONDITION( s[k].param & BIT_1, txtBigKey, txtKey );

	#define _PARAM_ARROW_UP		structSize 	size_UP = { (35*s[k].widthKey)/100,  (2*s[k].heightKey)/5 };		int bold_UP = 1;		int coeff_UP = 3
	#define _PARAM_ARROW_LF		structSize 	size_LF = { ( 2*s[k].widthKey)/4,    (2*s[k].heightKey)/7 };		int bold_LF = 1;		int coeff_LF = 0
	#define _PARAM_ARROW_EN		structSize 	size_EN = { ( 2*s[k].widthKey)/4,    (2*s[k].heightKey)/7 };		int bold_EN = 1;		int coeff_EN = 0
	#define _PARAM_ARROW_EX		structSize 	size_EX = { (35*s[k].heightKey)/100, (35*s[k].heightKey)/100 };

	if(shape!=0){		/* Do only once when creating Keyboard */
		charBuffOffs = 1;		/* 1 means cursor after text */
		s[k].param2 = 0;		/* keys style 0-1 */
		pTxtKey = (char**)txtKey;
		SetTxtParamInit(fontID);
		if(KeysAutoSize == widthKey){
			s[k].widthKey =  heightKey + LCD_GetWholeStrPxlWidth(fontID,(char*)pTxtKey[0],textParam.space,textParam.constWidth) + heightKey;
			s[k].heightKey = heightKey + LCD_GetFontHeight(fontID) + heightKey;
	}}

	int widthFieldTxt = 0;
	int heightFieldTxt = LCD_GetFontHeight(fontID) * 3;
	int head = s[k].interSpace + heightFieldTxt + s[k].interSpace;
	int colorFillBk = BrightDecr(bkColor,0x00);

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

	void _DispMainField 	 (void){	 LCD_ShapeWindow( s[k].shape,0, widthAll,	  	  heightAll, 	   0,					 0, 				   widthAll,	  heightAll, 		SetBold2Color(frameMainColor,s[k].bold), fillMainColor/*colorFillBk*/, bkColor 	  );	}
	void _DispTxtField  	 (void){	 LCD_ShapeWindow( s[k].shape,0, widthAll,	  	  heightAll, 	   s[k].interSpace,s[k].interSpace, widthFieldTxt,heightFieldTxt, SetBold2Color(TXTFIELD_COLOR,s[k].bold), TXTFIELD_COLOR,  				  colorFillBk );	}
	void _DispTxtFieldInd (void){	 LCD_ShapeWindow( s[k].shape,0, widthFieldTxt, heightFieldTxt, 0,					 0, 				   widthFieldTxt,heightFieldTxt, SetBold2Color(TXTFIELD_COLOR,s[k].bold), TXTFIELD_COLOR,  				  colorFillBk );  }

	void _RstIndxCharBuff(void)		{ charBuff[charBuffSize-1]=0; }
	void _SetIndxCharBuff(int indx)	{ if(IS_RANGE(indx,0,charBuffSize-1)) charBuff[charBuffSize-1]=indx; }
	int  _GetIndxCharBuff(void)		{ return charBuff[charBuffSize-1]; };
	void _IncIndxCharBuff(void)		{ if(IS_RANGE(charBuff[charBuffSize-1],0,charBuffSize-2)) charBuff[charBuffSize-1]++; }
	void _DecIndxCharBuff(void)		{ if(IS_RANGE(charBuff[charBuffSize-1],1,charBuffSize-1)) charBuff[charBuffSize-1]--; }
	void _SetCharBuff(char sign)		{ charBuff[_GetIndxCharBuff()]=sign; }
	char _GetCharBuff(int offs)		{ if(IS_RANGE(_GetIndxCharBuff()+offs,0,charBuff[charBuffSize-1])) return charBuff[_GetIndxCharBuff()+offs]; else return 0; }
	char *_GetPtrToCharBuff(int offs){ return charBuff+offs; }

	void _DispTxtInd(void){ DispTxtIndirect(_GetPtrToCharBuff(0), s[k].x+3+s[k].interSpace, s[k].x+3+s[k].interSpace, BLACK, TXTFIELD_COLOR); }  //zrob cos z tym 3 !!!!!!

	void _Cursor(int offs, u32 BkpSizeX,u32 BkpSizeY, int x,int y){
		int _char = _GetCharBuff(-1+offs);
		if(offs < 2)		/* offs=1 means cursor after text , offs<1 means cursor at text */
			LCD_ShapeWindow(LCD_Rectangle,0, BkpSizeX,BkpSizeY, \
					x + LCD_GetStrPxlWidth(fontID, _GetPtrToCharBuff(0), _GetIndxCharBuff()-1+offs, textParam.space, textParam.constWidth), \
					y + LCD_GetFontHeight(fontID) + 1, \
					LCD_GetFontWidth(fontID,CONDITION(offs<1,_char,' ')),1, BLACK,BLACK,BLACK);
	}

	void _ServiceCharBuff(int key){  //Wprowadz kursor migotajacy !!!!!!
		if		 (STRING_CmpTxt((char*)pTxtKey[key],"space"))  _SetCharBuff(' ');
		else 														 		  _SetCharBuff(*pTxtKey[key]);
		_IncIndxCharBuff();
	}

	void _DispTxtFieldWin(int cursorOffs){
		LCD_ShapeWindow( s[k].shape,0, widthFieldTxt, heightFieldTxt, 0,0, widthFieldTxt,heightFieldTxt, SetBold2Color(TXTFIELD_COLOR,s[k].bold), TXTFIELD_COLOR, colorFillBk );
		DispTxt(_GetPtrToCharBuff(0), 3,3, BLACK, TXTFIELD_COLOR, widthFieldTxt,heightFieldTxt);
		_Cursor(cursorOffs, widthFieldTxt, heightFieldTxt, 3,3);  //zrob cos z tym 3 !!!!!!
		LCD_Display(0, s[k].x+s[k].interSpace, s[k].y+s[k].interSpace, widthFieldTxt,heightFieldTxt);
	}

	// okreslic space od brzegow np tam gdzie jest +3 jako define !!!!



	void _KeyUP_ind(int nr){	_PARAM_ARROW_UP;
		u32 colorShape = CONDITION( s[k].param & BIT_1, colorTxtPressKey[nr], frameColor );
		int relPress 	= CONDITION( s[k].param & BIT_1, 1, 0 );
		if(0==MASK(s[k].param2,3)) KeyShapeDisp_Ind (k,posKey[nr], LCDSHAPE_Arrow, LCD_Arrow(ToStructAndReturn, s[k].widthKey,s[k].heightKey, MIDDLE(0,s[k].widthKey,size_UP.w),MIDDLE(0,s[k].heightKey,size_UP.h), SetLineBold2Width(size_UP.w,bold_UP), SetTriangHeightCoeff2Height(size_UP.h,coeff_UP), colorShape,colorShape,unUsed,Up), relPress);
		else								KeyShapeDisp2_Ind(k,posKey[nr], LCDSHAPE_Arrow, LCD_Arrow(ToStructAndReturn, s[k].widthKey,s[k].heightKey, MIDDLE(0,s[k].widthKey,size_UP.w),MIDDLE(0,s[k].heightKey,size_UP.h), SetLineBold2Width(size_UP.w,bold_UP), SetTriangHeightCoeff2Height(size_UP.h,coeff_UP), colorShape,colorShape,unUsed,Up), relPress);
	}
	void _KeyUP_win(int nr){	_PARAM_ARROW_UP;
		u32 colorShape = CONDITION( s[k].param & BIT_1, colorTxtPressKey[nr], frameColor );
		int relPress 	= CONDITION( s[k].param & BIT_1, 1, 0 );
		if(0==MASK(s[k].param2,3)) KeyShapeDisp_Win (k,posKey[nr], LCDSHAPE_Arrow, LCD_Arrow(ToStructAndReturn, widthAll, heightAll, posKey[nr].x+MIDDLE(0,s[k].widthKey,size_UP.w), posKey[nr].y+MIDDLE(0,s[k].heightKey, size_UP.h), SetLineBold2Width(size_UP.w,bold_UP), SetTriangHeightCoeff2Height(size_UP.h,coeff_UP), colorShape,colorShape,unUsed, Up), relPress);
		else								KeyShapeDisp2_Win(k,posKey[nr], LCDSHAPE_Arrow, LCD_Arrow(ToStructAndReturn, widthAll, heightAll, posKey[nr].x+MIDDLE(0,s[k].widthKey,size_UP.w), posKey[nr].y+MIDDLE(0,s[k].heightKey, size_UP.h), SetLineBold2Width(size_UP.w,bold_UP), SetTriangHeightCoeff2Height(size_UP.h,coeff_UP), colorShape,colorShape,unUsed, Up), relPress);
	}

	void _KeyBACK_ind(int nr){	_PARAM_ARROW_LF;
		u32 colorShape = colorTxtPressKey[nr];
		int relPress 	= 1;
		if(0==MASK(s[k].param2,3)) KeyShapeDisp_Ind (k,posKey[nr], LCDSHAPE_Arrow, LCD_Arrow(ToStructAndReturn, s[k].widthKey,s[k].heightKey, MIDDLE(0,s[k].widthKey,size_LF.w),MIDDLE(0,s[k].heightKey,size_LF.h), SetLineBold2Width(size_LF.w,bold_LF), SetTriangHeightCoeff2Height(size_LF.h,coeff_LF), colorShape,colorShape,unUsed, Left), relPress);
		else								KeyShapeDisp2_Ind(k,posKey[nr], LCDSHAPE_Arrow, LCD_Arrow(ToStructAndReturn, s[k].widthKey,s[k].heightKey, MIDDLE(0,s[k].widthKey,size_LF.w),MIDDLE(0,s[k].heightKey,size_LF.h), SetLineBold2Width(size_LF.w,bold_LF), SetTriangHeightCoeff2Height(size_LF.h,coeff_LF), colorShape,colorShape,unUsed, Left), relPress);
	}
	void _KeyBACK_win(int nr,int relPress){	_PARAM_ARROW_LF;
		u32 colorShape = CONDITION( relPress, colorTxtPressKey[nr], colorTxtKey[nr] );
		if(0==MASK(s[k].param2,3)) KeyShapeDisp_Win (k,posKey[nr], LCDSHAPE_Arrow, LCD_Arrow(ToStructAndReturn, widthAll, heightAll, posKey[nr].x+MIDDLE(0,s[k].widthKey,size_LF.w), posKey[nr].y+MIDDLE(0,s[k].heightKey, size_LF.h), SetLineBold2Width(size_LF.w,bold_LF), SetTriangHeightCoeff2Height(size_LF.h,coeff_LF), colorShape,colorShape,unUsed, Left), relPress);
		else								KeyShapeDisp2_Win(k,posKey[nr], LCDSHAPE_Arrow, LCD_Arrow(ToStructAndReturn, widthAll, heightAll, posKey[nr].x+MIDDLE(0,s[k].widthKey,size_LF.w), posKey[nr].y+MIDDLE(0,s[k].heightKey, size_LF.h), SetLineBold2Width(size_LF.w,bold_LF), SetTriangHeightCoeff2Height(size_LF.h,coeff_LF), colorShape,colorShape,unUsed, Left), relPress);
	}

	void _KeyENTER_ind(int nr){	_PARAM_ARROW_EN;
		u32 colorShape = colorTxtPressKey[nr];
		int relPress 	= 1;
		if(0==MASK(s[k].param2,3)) KeyShapeDisp_Ind (k,posKey[nr], LCDSHAPE_Enter, LCD_Enter(ToStructAndReturn,s[k].widthKey,s[k].heightKey, MIDDLE(0,s[k].widthKey,size_EN.w),MIDDLE(0,s[k].heightKey,size_EN.h), SetLineBold2Width(size_EN.w,bold_EN), SetTriangHeightCoeff2Height(size_EN.h,coeff_EN), colorShape,colorShape,unUsed), relPress);
		else								KeyShapeDisp2_Ind(k,posKey[nr], LCDSHAPE_Enter, LCD_Enter(ToStructAndReturn,s[k].widthKey,s[k].heightKey, MIDDLE(0,s[k].widthKey,size_EN.w),MIDDLE(0,s[k].heightKey,size_EN.h), SetLineBold2Width(size_EN.w,bold_EN), SetTriangHeightCoeff2Height(size_EN.h,coeff_EN), colorShape,colorShape,unUsed), relPress);
	}
	void _KeyENTER_win(int nr,int relPress){	_PARAM_ARROW_EN;
		u32 colorShape = CONDITION( relPress, colorTxtPressKey[nr], colorTxtKey[nr] );
		if(0==MASK(s[k].param2,3)) KeyShapeDisp_Win (k,posKey[nr], LCDSHAPE_Enter, LCD_Enter(ToStructAndReturn, widthAll, heightAll, posKey[nr].x+MIDDLE(0,s[k].widthKey,size_EN.w), posKey[nr].y+MIDDLE(0,s[k].heightKey,size_EN.h), SetLineBold2Width(size_EN.w,bold_EN), SetTriangHeightCoeff2Height(size_EN.h,coeff_EN), colorShape,colorShape,unUsed), relPress);
		else								KeyShapeDisp2_Win(k,posKey[nr], LCDSHAPE_Enter, LCD_Enter(ToStructAndReturn, widthAll, heightAll, posKey[nr].x+MIDDLE(0,s[k].widthKey,size_EN.w), posKey[nr].y+MIDDLE(0,s[k].heightKey,size_EN.h), SetLineBold2Width(size_EN.w,bold_EN), SetTriangHeightCoeff2Height(size_EN.h,coeff_EN), colorShape,colorShape,unUsed), relPress);
	}





	void _KeyStr_ind(int nr){
		u32 colorStr = colorTxtPressKey[nr];
		int relPress = 1;
		if(0==MASK(s[k].param2,3)) KeyStrDisp_Ind (k,posKey[nr],pTxtKey[nr],colorStr,relPress);
		else								KeyStrDisp2_Ind(k,posKey[nr],pTxtKey[nr],colorStr,relPress);
	}
	void _KeyStr_win(int nr,int relPress){
		u32 colorStr = CONDITION( relPress, colorTxtPressKey[nr], colorTxtKey[nr] );
		if(0==MASK(s[k].param2,3)) KeyStrDisp_Win (k,posKey[nr],pTxtKey[nr],colorStr,relPress);
		else								KeyStrDisp2_Win(k,posKey[nr],pTxtKey[nr],colorStr,relPress);
	}



	void _KeyQ2P(int nr, int act){
		if(release==act)	Key(k,posKey[nr]);
		else	 				KeyPressWin(k);

		char *ptrTxt = Int2Str(nr,None,1,Sign_none);
		int widthDescr = LCD_GetWholeStrPxlWidth(fontID_descr,ptrTxt,0,ConstWidth);
		int heightTxt = LCD_GetFontHeight(fontID);

		if(release==act){	 StrDescr_XYoffs(posKey[nr], 	 s[k].widthKey-VALPERC(widthDescr,180),  VALPERC(widthDescr,35), ptrTxt, 		BrightIncr(colorDescr,0x20));
								 Str_Xmidd_Yoffs(k,posKey[nr], s[k].heightKey-VALPERC(heightTxt,115), 							     pTxtKey[nr], colorTxtKey[nr]);
		}
		else{			BKCOPY_VAL(fillColor_c[0],fillColor,fillPressColor);
								StrDescrWin_XYoffs(k,s[k].widthKey-VALPERC(widthDescr,180), VALPERC(widthDescr,35), ptrTxt, 	    BrightIncr(colorDescr,0x20));
								StrWin_Xmidd_Yoffs(k,s[k].heightKey-VALPERC(heightTxt,115), 								pTxtKey[nr], colorTxtPressKey[nr]);
						BKCOPY(fillColor,fillColor_c[0]);
		}
		if(press==act)	LCD_Display(0,s[k].x+posKey[nr].x,s[k].y+posKey[nr].y,s[k].widthKey,s[k].heightKey);
	}

	void _ServiceTxtFieldTouch(void){
		XY_Touch_Struct touchFieldPos = LCD_TOUCH_GetPos();
		if(GetTouchToTemp( s[k].startTouchIdx + countKey )){
			int incTxt=0;
			int xStart = s[k].x + 3 + s[k].interSpace;	// z tym + 3 jako define daj !!!!!
			int xStop  = xStart  +  LCD_GetStrPxlWidth(fontID,_GetPtrToCharBuff(0), _GetIndxCharBuff()-1, textParam.space, textParam.constWidth);

			if(xStop < touchFieldPos.x){
				charBuffOffs = 1;
				_DispTxtFieldWin(charBuffOffs);
			}
			else{
				LOOP_FOR(i,_GetIndxCharBuff()){
					if(xStart > touchFieldPos.x){
						charBuffOffs = (incTxt+1) - _GetIndxCharBuff();
						_DispTxtFieldWin(charBuffOffs);
						break;
					}
					incTxt++;
					xStart = s[k].x + 3 + s[k].interSpace  +  LCD_GetStrPxlWidth(fontID,charBuff, incTxt, textParam.space, textParam.constWidth);
			}}
	}}

	void _DispAllReleaseKeyboard(void){
		_DispMainField();
		_DispTxtField();
		DispTxt(_GetPtrToCharBuff(0), 3+s[k].interSpace, 3+s[k].interSpace, BLACK, TXTFIELD_COLOR, widthAll, heightAll);

		BKCOPY_VAL(fillColor_c[0],fillColor,BrightIncr(fillColor,0x10));
		BKCOPY_VAL(bkColor_c[0],bkColor,colorFillBk);
		c.widthKey = s[k].widthKey;
		for(int i=0; i<countKey; ++i)
		{
			s[k].widthKey = wKey[i];

			if		 (STRING_CmpTxt((char*)pTxtKey[i],_UP)){	_KeyUP_win(i);	 }

			else if(STRING_CmpTxt((char*)pTxtKey[i],_LF)){	//Key(k,posKey[i]);
//				_PARAM_ARROW_LF;  //To zmienic
//				LCD_RoundRectangle2(0,PARAM32(Down,bold2,unUsed,Rectangle), widthAll,heightAll, posKey[i].x,posKey[i].y, s[k].widthKey, s[k].heightKey, BrightDecr(framePressColor,0x20),BrightIncr(framePressColor,0x65), 0xFF808080,BrightDecr(fillPressColor,0x44), bkColor, 0.0, Down);
//				LCD_Arrow(0,widthAll,heightAll, MIDDLE(posKey[i].x,s[k].widthKey,size_LF.w),MIDDLE(posKey[i].y,s[k].heightKey,size_LF.h), SetLineBold2Width(size_LF.w,bold_LF), SetTriangHeightCoeff2Height(size_LF.h,coeff_LF), frameColor,frameColor,bkColor, Left);
//
				_KeyBACK_win(i,0);

			}
			else if(STRING_CmpTxt((char*)pTxtKey[i],_EN)){
//				Key(k,posKey[i]);		_PARAM_ARROW_EN;  //To zmienic
//				LCD_RoundRectangle2(0,PARAM32(Down,bold2,unUsed,Rectangle), widthAll,heightAll, posKey[i].x,posKey[i].y, s[k].widthKey, s[k].heightKey, BrightDecr(framePressColor,0x20),BrightIncr(framePressColor,0x65), 0xFF808080,BrightDecr(fillPressColor,0x44), bkColor, 0.0, Down);
//				LCD_Enter(0,widthAll,heightAll, MIDDLE(posKey[i].x,s[k].widthKey,size_EN.w),MIDDLE(posKey[i].y,s[k].heightKey,size_EN.h), SetLineBold2Width(size_EN.w,bold_EN), SetTriangHeightCoeff2Height(size_EN.h,coeff_EN), frameColor,frameColor,bkColor);
//
				_KeyENTER_win(i,0);

			}
			else if(STRING_CmpTxt((char*)pTxtKey[i],_EX)){	KeyPress(k,posKey[i]);	_PARAM_ARROW_EX;  //To zmienic
				LCD_RoundRectangle2(0,PARAM32(Down,bold2,unUsed,Rectangle), widthAll,heightAll, posKey[i].x,posKey[i].y, s[k].widthKey, s[k].heightKey, BrightDecr(framePressColor,0x20),BrightIncr(framePressColor,0x65), 0xFF808080,BrightDecr(fillPressColor,0x44), bkColor, 0.0, Down);
				LCD_Exit(0,widthAll,heightAll, MIDDLE(posKey[i].x,s[k].widthKey,size_EX.w),MIDDLE(posKey[i].y,s[k].heightKey,size_EX.h), size_EX.w, size_EX.h, colorTxtPressKey[i],colorTxtPressKey[i],bkColor);
			}
			else{
				if(i<dimKeys[0]) _KeyQ2P(i,release);
				else{
					if(STRING_CmpTxt((char*)pTxtKey[i],_AL) && 0 < (s[k].param & BIT_2)) KeyStrPressDisp_win(k,posKey[i],pTxtKey[i],colorTxtPressKey[i]);
					else 																						_KeyStr_win(i,0);//KeyStr_alt( k,posKey[i],pTxtKey[i],colorTxtKey[i] );
			}}
		}
		s[k].widthKey = c.widthKey;
		BKCOPY(bkColor,bkColor_c[0]);
		BKCOPY(fillColor,fillColor_c[0]);

		_Cursor(charBuffOffs, widthAll,heightAll, 3+s[k].interSpace, 3+s[k].interSpace);
		LCD_Display(0, s[k].x, s[k].y, widthAll, heightAll);
	}


	if(touchRelease == selBlockPress) _DispAllReleaseKeyboard();

	else if(tBig == selBlockPress){		TOOGLE_BIT(s[k].param,BIT_1);
		nr = selBlockPress-touchAction;
		BKCOPY_VAL(c.widthKey,s[k].widthKey,wKey[nr]);
		BKCOPY_VAL(fillColor_c[0],fillColor,BrightIncr(fillColor,0x10));
		 _KeyUP_ind(nr);
		BKCOPY(fillColor,fillColor_c[0]);
		BKCOPY(s[k].widthKey,c.widthKey);
	}
	else if(tBack == selBlockPress){
		nr = selBlockPress-touchAction;
		BKCOPY_VAL(c.widthKey,s[k].widthKey,wKey[nr]);
		BKCOPY_VAL(fillColor_c[0],fillColor,BrightIncr(fillColor,0x10));
		 _KeyBACK_ind(nr);
		BKCOPY(fillColor,fillColor_c[0]);
		BKCOPY(s[k].widthKey,c.widthKey);
		_DecIndxCharBuff(); _SetCharBuff(0x0);	_DispTxtFieldWin(charBuffOffs);
	}
	else if(tEnter == selBlockPress){
		nr = selBlockPress-touchAction;
		BKCOPY_VAL(c.widthKey,s[k].widthKey,wKey[nr]);
		BKCOPY_VAL(fillColor_c[0],fillColor,BrightIncr(fillColor,0x10));
		 _KeyENTER_ind(nr);
		BKCOPY(fillColor,fillColor_c[0]);
		BKCOPY(s[k].widthKey,c.widthKey);
	}
	else if(tAlt == selBlockPress){		TOOGLE_BIT(s[k].param,BIT_2);
		INIT(nr,selBlockPress-touchAction);
		BKCOPY_VAL(c.widthKey,s[k].widthKey,wKey[nr]);
		_KeyStr_ind(nr);
		BKCOPY(s[k].widthKey,c.widthKey);
	}
	else if(tField == selBlockPress){		//tField to nie touch ale key !!!!!
		_ServiceTxtFieldTouch();
	}
	else if(tField + 1 == selBlockPress){		/*	KEY_style = tField + 1		change key style */

		TOOGLE_BIT(s[k].param2,BIT_1);
		_DispAllReleaseKeyboard();

	}
	else{
		if(IS_RANGE(selBlockPress,touchAction,touchAction+9))			/* press keys from 'q' to 'p' */
		{
			INIT(nr,selBlockPress-touchAction);
			BKCOPY_VAL(c.widthKey,s[k].widthKey,wKey[nr]);
			 _ServiceCharBuff(nr);
			 _DispTxtFieldWin(charBuffOffs);
			 _KeyQ2P(nr,press);
			BKCOPY(s[k].widthKey,c.widthKey);
		}
		else if(IS_RANGE(selBlockPress,touchAction+10,tEnter))		/* press rest of keys */
		{
			INIT(nr,selBlockPress-touchAction);
			BKCOPY_VAL(c.widthKey,s[k].widthKey,wKey[nr]);
			 _ServiceCharBuff(nr);
			 _DispTxtFieldWin(charBuffOffs);
			 _KeyStr_ind(nr);
			BKCOPY(s[k].widthKey,c.widthKey);
		}
	}


	if(startTouchIdx){
		int i;
		for(i=0; i<countKey; ++i){
			BKCOPY_VAL(c.widthKey,s[k].widthKey,wKey[i]);
			SetTouch(k,ID_TOUCH_POINT,s[k].startTouchIdx+i,press,posKey[i]);
			BKCOPY(s[k].widthKey,c.widthKey);
		}
		touchTemp[0].x= s[k].x+s[k].interSpace;
		touchTemp[1].x= touchTemp[0].x + widthFieldTxt;
		touchTemp[0].y= s[k].y+s[k].interSpace;
		touchTemp[1].y= touchTemp[0].y + heightFieldTxt;
		LCD_TOUCH_Set(ID_TOUCH_POINT, s[k].startTouchIdx+i,press);
		s[k].nmbTouch++;

		touchTemp[0].x= 700;   //tymczasowe dotyk zmiany stylu !!!!
		touchTemp[1].x= 800;
		touchTemp[0].y= 400;
		touchTemp[1].y= 480;
		LCD_TOUCH_Set(ID_TOUCH_POINT, s[k].startTouchIdx+i+1,press);
		s[k].nmbTouch++;
	}

	#undef P
	#undef F
	#undef W1
	#undef WS
	#undef WA
	#undef _UP
	#undef _LF
	#undef _EN
	#undef _EX
	#undef TXTFIELD_COLOR
}

/* ----- End User Function Definitions ----- */
