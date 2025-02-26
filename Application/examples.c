
#include "LCD_BasicGaphics.h"
#include "LCD_fonts_images.h"
#include "string_oper.h"
#include "LCD_Common.h"
#include "tim.h"
#include "timer.h"
#include "mini_printf.h"
#include <string.h>
#include "common.h"


void LCDEXAMPLE_RectangleGradient(u32 COLOR_FillFrame, u32 COLOR_Frame, u32 COLOR_BkScreen)
{
	SHAPE_PARAMS par1,par2,par3,par4,par5,par6;		/* for LCD_RoundRectangle2() for 'AllEdge' argument 'DIRECTIONS' has no effect */

	LCD_Rectangle2(0, LCD_X,LCD_Y, 20,240, 100, 40,  COLOR_FillFrame,BrightIncr(COLOR_FillFrame,0x55),   			  COLOR_FillFrame,ORANGE,  COLOR_BkScreen, 0.0, Down);
	LCD_Rectangle2(0, LCD_X,LCD_Y, 160,240, 100, 40, BrightDecr(COLOR_Frame,0xA5),BrightDecr(COLOR_Frame,0xAE),   COLOR_FillFrame,YELLOW,  COLOR_BkScreen, 0.0, Up);
	LCD_Rectangle2(0, LCD_X,LCD_Y, 270,240, 100, 40, BrightDecr(COLOR_Frame,0xA0),BrightIncr(COLOR_Frame,0x01),   COLOR_FillFrame,BrightDecr(COLOR_FillFrame,0x09),  COLOR_BkScreen, 0.0, Midd_Y);
	LCD_Rectangle2(0, LCD_X,LCD_Y, 380,240, 100, 40, BrightDecr(COLOR_Frame,0xA0),BrightIncr(COLOR_Frame,0x01),   COLOR_FillFrame,BrightDecr(COLOR_FillFrame,0x15),  COLOR_BkScreen, 0.0, Midd_X);

	LCD_Rectangle2(0, LCD_X,LCD_Y, 20,300, 100, 40,  0xFF202020,0xFF282828,   0xFF909090,BrightDecr(0xFF909090,0x20),  COLOR_BkScreen, 0.0, Midd_Y);
	//LCD_Rectangle2(0, LCD_X,LCD_Y, 160,300, 100, 40, 0xFF202020,0xFF282828,   0xFF909090,BrightDecr(0xFF909090,0x20),  v.COLOR_BkScreen, 0.0, Midd_Y2);
	LCD_Rectangle2(0, LCD_X,LCD_Y, 160,300, 100, 40, WHITE,(u32)bold8,   0xFF909090,BrightIncr(0xFF909090,0x20),  COLOR_BkScreen, 0.0, AllEdge);

	par3=LCD_Rectangle2(ToStructAndReturn, LCD_X,LCD_Y, 500,350, 100, 25, BrightDecr(COLOR_Frame,0xA0),BrightIncr(COLOR_Frame,0x01),  COLOR_FillFrame,BrightDecr(COLOR_FillFrame,0x35),  COLOR_BkScreen, 0.0, RightDown);
	par4=LCD_Rectangle2(ToStructAndReturn, LCD_X,LCD_Y, 610,360, 29, 100, BrightDecr(COLOR_Frame,0xA0),BrightIncr(COLOR_Frame,0x01),  COLOR_FillFrame,BrightDecr(COLOR_FillFrame,0x35),  COLOR_BkScreen, 0.0, RightDown);

	par1=LCD_RoundRectangle2(ToStructAndReturn,PARAM32(AllEdge, bold10,0x20,Rectangle),LCD_X,LCD_Y,35, 350,70,80 ,WHITE,BrightDecr(WHITE,0x20), 0xFFAAAAAA,BrightDecr(0xFFAAAAAA,0x20), COLOR_BkScreen, 0.0, Down);  /* 0x20 give effect 3D*/
		  LCD_RoundRectangle2(0,					 PARAM32(AllEdge2,bold10,0x20,Rectangle),LCD_X,LCD_Y,120,350,70,80 ,WHITE,BrightDecr(WHITE,0x40), 0xFFAAAAAA,BrightDecr(0xFFAAAAAA,0x40), COLOR_BkScreen, 0.0, Down);  /* 0x20 give effect 3D*/

	par2=LCD_RoundRectangle2(ToStructAndReturn,PARAM32(Up,bold2,unUsed,Rectangle), 		LCD_X,LCD_Y,200,350,70,80, BrightDecr(COLOR_Frame,0x20),BrightIncr(COLOR_Frame,0x65), 0xFF808080,BrightDecr(0xFF808080,0x44), COLOR_BkScreen, 0.0, Left);
		  LCD_RoundRectangle2(0,					 PARAM32(Down,bold3,unUsed,Rectangle), 	LCD_X,LCD_Y,280,390,70,80, BrightDecr(COLOR_Frame,0x20),BrightIncr(COLOR_Frame,0x65), 0xFF808080,BrightDecr(0xFF808080,0x44), COLOR_BkScreen, 0.0, Down);

	LCDSHAPE_RoundRectangle(0,par1);
	LCDSHAPE_RoundRectangle(0,par2);

	par5=LCD_Rectangle2(ToStructAndReturn, LCD_X,LCD_Y, 570,240, 100, 40, COLOR_FillFrame,				   BrightIncr(COLOR_FillFrame,0x55),   COLOR_FillFrame,ORANGE,  COLOR_BkScreen, 0.25, Down);
	par6=LCD_Rectangle2(ToStructAndReturn, LCD_X,LCD_Y, 570,300, 100, 40, BrightDecr(COLOR_Frame,0xA5),BrightDecr(COLOR_Frame,0xAE), 		COLOR_FillFrame,YELLOW,  COLOR_BkScreen, 0.3, Up);

	LCDSHAPE_Rectangle(0,par3);
	LCDSHAPE_Rectangle(0,par4);
	LCDSHAPE_Rectangle(0,par5);
	LCDSHAPE_Rectangle(0,par6);

	LCD_RoundRectangle2(0, PARAM32(Down2,bold5,3,Rectangle), LCD_X,LCD_Y,710,240,70,80, BrightDecr(COLOR_Frame,0x40),0x202020, 0xFF808080,BrightDecr(0xFF808080,0x40), COLOR_BkScreen, 0.0, Down);
	LCD_RoundRectangle2(0, PARAM32(Down2,bold5,12,Rectangle), LCD_X,LCD_Y,710,340,70,80, BrightDecr(COLOR_Frame,0x40),0x202020, 0xFF808080,BrightDecr(0xFF808080,0x40), COLOR_BkScreen, 0.0, Down);

	LCD_RoundRectangle2(0,PARAM32(AllEdge,bold10,unUsed,Rectangle),LCD_X,LCD_Y,370,390,70,80 ,BrightDecr(WHITE,0x50), BrightDecr(WHITE,0x50), 0xFFA0A0A0,BrightDecr(0xFFA0A0A0,0x50), COLOR_BkScreen, 0.0, Down);
	LCD_RoundRectangle2(0,PARAM32(AllEdge,bold10,unUsed,Rectangle),LCD_X,LCD_Y,370,390,70,80 ,BrightDecr(WHITE,0x20),BrightDecr(WHITE,0x50),BrightDecr(0xFFAAAAAA,0x20) ,BrightDecr(0xFFAAAAAA,0x50), COLOR_BkScreen, 0.0, Down);

	LCD_RoundRectangle2(0,PARAM32(Shade,  bold7, unUsed,Frame),		LCD_X,LCD_Y,480,383,70,80, BrightDecr(0xFFA0A0A0,0x77),BrightDecr(0xFFA0A0A0,0x77),  unUsed,								unUsed, 							 COLOR_BkScreen, 0.0, Down);
	LCD_RoundRectangle2(0,PARAM32(AllEdge,bold10,unUsed,Rectangle),LCD_X,LCD_Y,480,383,70,80, 0xFFA0A0A0,						 0xFFA0A0A0,  	 					 BrightDecr(0xFFA0A0A0,0x30), BrightDecr(0xFFA0A0A0,0x10),COLOR_BkScreen, 0.0, Down);

//	LCDSHAPE_Rectangle_Indirect(par3);
//	LCDSHAPE_Rectangle_Indirect(par4);
//	LCDSHAPE_Rectangle_Indirect(par5);
//	LCDSHAPE_Rectangle_Indirect(par6);
}

void LCDEXAMPLE_GradientCircleButtonAndSlider(int FONT_ID_Title, int FONT_VAR_Title, u32 COLOR_FillMainFrame, u32 COLOR_Frame, u32 COLOR_BkScreen)
{
	/* Common for noIndirect an Indirect */
	uint32_t CircleWidth= 150;
	SHAPE_PARAMS par={0}, par2={0};
	uint16_t calcWidth = LCD_CalculateCircleWidth(CircleWidth);

	LCD_SetCircleAA(0.0, 0.0);
	CorrectLineAA_on();
	LCD_BkFontTransparent(FONT_VAR_Title, FONT_ID_Title);

	par=LCD_GradientCircleSlider(ToStructAndReturn,LCD_X,LCD_Y, LCD_X-calcWidth-10-625, LCD_Y-calcWidth-30,CircleWidth,CircleWidth,   SetBold2Color(COLOR_BkScreen,23),COLOR_FillMainFrame,0x666666,0xBBBBBB,0, SetBold2Color(COLOR_FillMainFrame,15),0xC0C0C0,0x333333, COLOR_BkScreen,270,Round,0);
	par2=par;
	/* -------  END -------- */

	/* Methods for noIndirect */
	LCDSHAPE_GradientCircleSlider(0,par);
	LCD_StrDependOnColorsMidd(FONT_ID_VAR(FONT_ID_Title,FONT_VAR_Title), POS_SIZE_CIRCLEBUTTONSLIDER(par,0,0), "23", fullHight,0, BK_COLOR_CIRCLESLIDER(par), WHITE, 250, ConstWidth);

	LCDSHAPE_Create(0,LCD_X,LCD_Y,LCD_X-calcWidth-10-450, LCD_Y-calcWidth-30, SetParamWidthCircle(0,CircleWidth),CircleWidth, SetBold2Color(WHITE,30), COLOR_BkScreen, COLOR_BkScreen, RED,BLUE,0,360,RightDown,1);
	LCDSHAPE_Create(0,LCD_X,LCD_Y,LCD_X-calcWidth-10-150, LCD_Y-calcWidth-30, SetParamWidthCircle(Percent_Circle,CircleWidth),CircleWidth, SetBold2Color(COLOR_BkScreen,0), COLOR_FillMainFrame, COLOR_BkScreen, RED,BLUE,0,350,Left,1);
	LCDSHAPE_Create(0,LCD_X,LCD_Y,LCD_X-calcWidth-10, LCD_Y-calcWidth-50,     SetParamWidthCircle(Percent_Circle,CircleWidth),CircleWidth, SetBold2Color(COLOR_BkScreen,0), COLOR_FillMainFrame, COLOR_BkScreen, RED,BLUE,0,355,Right,0);
	/* -------  END -------- */

	StartMeasureTime_us(); StopMeasureTime_us("Time:");
/*
	LOOP_FOR(i,10){
		LCD_GradientCircleButton(0,LCD_X,LCD_Y,LCD_X-calcWidth-100-i, LCD_Y-calcWidth-50-i, CircleWidth,CircleWidth, SetBold2Color(BrightDecr(0xFFA0A0A0,0x80),18),BrightDecr(0xFFA0A0A0,0x80), BrightDecr(0xFFA0A0A0,0x80),COLOR_BkScreen,1);
	}
	LCD_GradientCircleButton(0,LCD_X,LCD_Y,LCD_X-calcWidth-100-10, LCD_Y-calcWidth-50-10, CircleWidth,CircleWidth, SetBold2Color(BrightDecr(WHITE,0x90),18),BrightDecr(0xFFA0A0A0,0x30), BrightDecr(0xFFA0A0A0,0x70),COLOR_BkScreen,1);
*/

	/* 2 Methods for Indirect */
	par=LCD_GradientCircleButton(ToStructAndReturn,CircleWidth,CircleWidth,LCD_X-calcWidth-10, LCD_Y-calcWidth-30, CircleWidth,CircleWidth, SetBold2Color(COLOR_BkScreen,18),LIGHTBLUE,DARKBLUE,COLOR_BkScreen,0);
	LCDSHAPE_GradientCircleButton_Indirect(par);
	LCD_StrDependOnColorsWindowMiddIndirect(0, POS_SIZE_CIRCLEBUTTONSLIDER_indirect(par,0,0), FONT_ID_VAR(FONT_ID_Title,FONT_VAR_Title), "987", fullHight,0, unUsed /*BK_COLOR_CIRCLESLIDER(par2)*/, WHITE, 248, ConstWidth); 	/* bkColor unUsed because called function: LCD_BkFontTransparent() */

	LCDSHAPE_GradientCircleSlider_Indirect(par2);
	LCD_StrDependOnColorsWindowMiddIndirect(0, POS_SIZE_CIRCLEBUTTONSLIDER_indirect(par2,0,0), FONT_ID_VAR(FONT_ID_Title,FONT_VAR_Title), "23", fullHight,0, unUsed /*BK_COLOR_CIRCLESLIDER(par2)*/, WHITE, 248, ConstWidth);
	/* -------  END -------- */
}

void LCDEXAMPLE_LcdTxt(int FONT_ID_Fonts, int FONT_VAR_Fonts, u32 COLOR_FillMainFrame, u32 COLOR_Frame, u32 COLOR_BkScreen)
{
	/* 5 Methods for noIndirect */
	LCD_STR_PARAM temp, new;
	new =  LCD_Txt(noDisplay, NULL, 0,0, LCD_X,LCD_Y, FONT_ID_Fonts, FONT_VAR_Fonts, 320,200, "12345", BLACK, COLOR_BkScreen, fullHight,0,250, NoConstWidth, 0x777777, 2, RightDown);	/* load parameters to 'new' */
	temp = LCD_Txt(noDisplay, &new, 0,0, LCD_X,LCD_Y, FONT_ID_Fonts, FONT_VAR_Fonts, 320,200, "12345", BLACK, COLOR_BkScreen, fullHight,0,250, NoConstWidth, 0x777777, 2, RightDown);	/* load parameters to 'new' and 'temp' */
	temp = LCD_Txt(Display,   NULL, 0,0, LCD_X,LCD_Y, FONT_ID_Fonts, FONT_VAR_Fonts, 320,200, "12345", BLACK, COLOR_BkScreen, fullHight,0,250, NoConstWidth, 0x777777, 2, RightDown);	/* 1 method */
	temp = LCD_Txt(Display,   &new, 0,0, LCD_X,LCD_Y, FONT_ID_Fonts, FONT_VAR_Fonts, 320,200, "12345", BLACK, COLOR_BkScreen, fullHight,0,250, NoConstWidth, 0x777777, 2, RightDown);	/* 2 method */

	LCD_Shape(new.txt.pos.x, new.txt.pos.y, LCD_Frame,  new.txt.size.w+new.shadow.deep, new.txt.size.h+new.shadow.deep,  RED,COLOR_BkScreen,COLOR_BkScreen);	/* test frame around the text */

	temp = LCD_Txt(DisplayViaStruct,&new,NO_TXT_ARGS);		/* 3 method */
	temp = LCD_TxtVar(&new,NULL);									/* 4 method */
	temp = LCD_TxtVar(&new,"new text");							/* 5 method */
	/* -------  END -------- */

	/* 5 Methods for Indirect */
	LCD_STR_PARAM temp2,new2;
	temp2 = LCD_Txt(noDisplay, &new2, 50,200, BK_SIZE_IS_TXT_SIZE, FONT_ID_Fonts, FONT_VAR_Fonts, 0,0, "12345", WHITE, COLOR_BkScreen, fullHight,0,250, NoConstWidth, 0x777777, 3, RightDown);	/* load parameters to 'new2' and 'temp2' */
	new2 =  LCD_Txt(noDisplay, NULL,  50,200, BK_SIZE_IS_TXT_SIZE, FONT_ID_Fonts, FONT_VAR_Fonts, 0,0, "12345", WHITE, COLOR_BkScreen, fullHight,0,250, NoConstWidth, 0x777777, 3, RightDown);	/* load parameters to 'new2' */
												/* {BkpSizeX,BkpSizeXY}=0 because is calculated as text length  */
	temp2 = LCD_Txt(DisplayIndirectViaStruct,&new2,NO_TXT_ARGS);	/* 1 method */
	temp2 = LCD_TxtVarInd(&new2, NULL);										/* 2 method */
	temp2 = LCD_TxtVarInd(&new2, "new text");								/* 3 method */
	temp2 = LCD_Txt(DisplayIndirect, &new2, 50,250, BK_SIZE_IS_TXT_SIZE, FONT_ID_Fonts, FONT_VAR_Fonts, 0,0, "abcde", WHITE, COLOR_BkScreen, fullHight,0,250, NoConstWidth, 0x777777, 6, RightDown);		/* 4 method */
	temp2 = LCD_Txt(DisplayIndirect, NULL,  50,250, BK_SIZE_IS_TXT_SIZE, FONT_ID_Fonts, FONT_VAR_Fonts, 0,0, "abcde", WHITE, COLOR_BkScreen, fullHight,0,250, NoConstWidth, 0x777777, 6, RightDown);		/* 5 method */
	/* -------  END -------- */
}
