
#include "LCD_BasicGraphics.h"
#include "LCD_fonts_images.h"
#include "string_oper.h"
#include "LCD_Common.h"
#include "tim.h"
#include "timer.h"
#include "mini_printf.h"
#include <string.h>
#include "common.h"
#include "debug.h"
#include "touch.h"


void LCDEXAMPLE_RectangleGradient(u32 COLOR_FillFrame, u32 COLOR_Frame, u32 COLOR_BkScreen, int FONT_ID_Descr)
{
	SHAPE_PARAMS par1,par2,par3,par4,par5,par6;		/* for LCD_RoundRectangle2() for 'AllEdge' argument 'DIRECTIONS' has no effect */

	void _Str(int x,int y, char* txt){ LCD_StrDependOnColors(FONT_ID_VAR(FONT_ID_Descr,fontVar_40), x,y, txt, fullHight,0, unUsed, WHITE, 255, NoConstWidth); }

	LCD_BkFontTransparent(fontVar_40, FONT_ID_Descr);

	LCD_Rectangle2(0, LCD_X,LCD_Y, 20,240, 100, 40,  COLOR_FillFrame,BrightIncr(COLOR_FillFrame,0x55),   			  COLOR_FillFrame,ORANGE,  COLOR_BkScreen, 0.0, Down);  										_Str(60,250,"1");
	LCD_Rectangle2(0, LCD_X,LCD_Y, 160,240, 100, 40, BrightDecr(COLOR_Frame,0xA5),BrightDecr(COLOR_Frame,0xAE),   COLOR_FillFrame,YELLOW,  COLOR_BkScreen, 0.0, Up);  										_Str(200,250,"2");
	LCD_Rectangle2(0, LCD_X,LCD_Y, 270,240, 100, 40, BrightDecr(COLOR_Frame,0xA0),BrightIncr(COLOR_Frame,0x01),   COLOR_FillFrame,BrightDecr(COLOR_FillFrame,0x09),  COLOR_BkScreen, 0.0, Midd_Y);  _Str(310,250,"3");
	LCD_Rectangle2(0, LCD_X,LCD_Y, 380,240, 100, 40, BrightDecr(COLOR_Frame,0xA0),BrightIncr(COLOR_Frame,0x01),   COLOR_FillFrame,BrightDecr(COLOR_FillFrame,0x15),  COLOR_BkScreen, 0.0, Midd_X);  _Str(420,250,"4");

	LCD_Rectangle2(0, LCD_X,LCD_Y, 20,300, 100, 40,  0xFF202020,0xFF282828,   0xFF909090,BrightDecr(0xFF909090,0x20),  COLOR_BkScreen, 0.0, Midd_Y);  _Str(60,310,"5");
	//LCD_Rectangle2(0, LCD_X,LCD_Y, 160,300, 100, 40, 0xFF202020,0xFF282828,   0xFF909090,BrightDecr(0xFF909090,0x20),  v.COLOR_BkScreen, 0.0, Midd_Y2);
	LCD_Rectangle2(0, LCD_X,LCD_Y, 160,300, 100, 40, WHITE,(u32)bold8,   0xFF909090,BrightIncr(0xFF909090,0x20),  COLOR_BkScreen, 0.0, AllEdge); 	  _Str(200,310,"6");

	par3=LCD_Rectangle2(ToStructAndReturn, LCD_X,LCD_Y, 500,350, 100, 25, BrightDecr(COLOR_Frame,0xA0),BrightIncr(COLOR_Frame,0x01),  COLOR_FillFrame,BrightDecr(COLOR_FillFrame,0x35),  COLOR_BkScreen, 0.0, RightDown);
	par4=LCD_Rectangle2(ToStructAndReturn, LCD_X,LCD_Y, 610,360, 29, 100, BrightDecr(COLOR_Frame,0xA0),BrightIncr(COLOR_Frame,0x01),  COLOR_FillFrame,BrightDecr(COLOR_FillFrame,0x35),  COLOR_BkScreen, 0.0, RightDown);

	par1=LCD_RoundRectangle2(ToStructAndReturn,PARAM32(AllEdge, bold10,0x20,Rectangle),LCD_X,LCD_Y,35, 350,70,80 ,WHITE,BrightDecr(WHITE,0x20), 0xFFAAAAAA,BrightDecr(0xFFAAAAAA,0x20), COLOR_BkScreen, 0.0, Down);  /* 0x20 give effect 3D*/
		  LCD_RoundRectangle2(0,					 PARAM32(AllEdge2,bold10,0x20,Rectangle),LCD_X,LCD_Y,120,350,70,80 ,WHITE,BrightDecr(WHITE,0x40), 0xFFAAAAAA,BrightDecr(0xFFAAAAAA,0x40), COLOR_BkScreen, 0.0, Down);  /* 0x20 give effect 3D*/ 	  _Str(150,370,"7");

	par2=LCD_RoundRectangle2(ToStructAndReturn,PARAM32(Up,bold2,unUsed,Rectangle), 		LCD_X,LCD_Y,200,350,70,80, BrightDecr(COLOR_Frame,0x20),BrightIncr(COLOR_Frame,0x65), 0xFF808080,BrightDecr(0xFF808080,0x44), COLOR_BkScreen, 0.0, Left);
		  LCD_RoundRectangle2(0,					 PARAM32(Down,bold3,unUsed,Rectangle), 	LCD_X,LCD_Y,280,390,70,80, BrightDecr(COLOR_Frame,0x20),BrightIncr(COLOR_Frame,0x65), 0xFF808080,BrightDecr(0xFF808080,0x44), COLOR_BkScreen, 0.0, Down); 	  _Str(310,410,"8");

	LCDSHAPE_RoundRectangle(0,par1); 	  _Str(65,370,"9");
	LCDSHAPE_RoundRectangle(0,par2); 	  _Str(220,370,"10");

	par5=LCD_Rectangle2(ToStructAndReturn, LCD_X,LCD_Y, 570,240, 100, 40, COLOR_FillFrame,				   BrightIncr(COLOR_FillFrame,0x55),   COLOR_FillFrame,ORANGE,  COLOR_BkScreen, 0.25, Down);
	par6=LCD_Rectangle2(ToStructAndReturn, LCD_X,LCD_Y, 570,300, 100, 40, BrightDecr(COLOR_Frame,0xA5),BrightDecr(COLOR_Frame,0xAE), 		COLOR_FillFrame,YELLOW,  COLOR_BkScreen, 0.3, Up);

	LCDSHAPE_Rectangle(0,par3); 	  _Str(530,355,"11");
	LCDSHAPE_Rectangle(0,par4); 	  _Str(615,380,"12");
	LCDSHAPE_Rectangle(0,par5); 	  _Str(590,250,"13");
	LCDSHAPE_Rectangle(0,par6); 	  _Str(590,310,"14");

	LCD_RoundRectangle2(0, PARAM32(Down2,bold5,3,Rectangle), LCD_X,LCD_Y,710,240,70,80, BrightDecr(COLOR_Frame,0x40),0x202020, 0xFF808080,BrightDecr(0xFF808080,0x40), COLOR_BkScreen, 0.0, Down); 	 	  _Str(730,260,"15");
	LCD_RoundRectangle2(0, PARAM32(Down2,bold5,12,Rectangle), LCD_X,LCD_Y,710,340,70,80, BrightDecr(COLOR_Frame,0x40),0x202020, 0xFF808080,BrightDecr(0xFF808080,0x40), COLOR_BkScreen, 0.0, Down); 	  _Str(730,360,"16");

	LCD_RoundRectangle2(0,PARAM32(AllEdge,bold10,unUsed,Rectangle),LCD_X,LCD_Y,370,390,70,80 ,BrightDecr(WHITE,0x50), BrightDecr(WHITE,0x50), 0xFFA0A0A0,BrightDecr(0xFFA0A0A0,0x50), COLOR_BkScreen, 0.0, Down);
	LCD_RoundRectangle2(0,PARAM32(AllEdge,bold10,unUsed,Rectangle),LCD_X,LCD_Y,370,390,70,80 ,BrightDecr(WHITE,0x20),BrightDecr(WHITE,0x50),BrightDecr(0xFFAAAAAA,0x20) ,BrightDecr(0xFFAAAAAA,0x50), COLOR_BkScreen, 0.0, Down); 	 _Str(390,410,"17");

	LCD_RoundRectangle2(0,PARAM32(Shade,  bold7, unUsed,Frame),		LCD_X,LCD_Y,480,383,70,80, BrightDecr(0xFFA0A0A0,0x77),BrightDecr(0xFFA0A0A0,0x77),  unUsed,								unUsed, 							 COLOR_BkScreen, 0.0, Down);
	LCD_RoundRectangle2(0,PARAM32(AllEdge,bold10,unUsed,Rectangle),LCD_X,LCD_Y,480,383,70,80, 0xFFA0A0A0,						 0xFFA0A0A0,  	 					 BrightDecr(0xFFA0A0A0,0x30), BrightDecr(0xFFA0A0A0,0x10),COLOR_BkScreen, 0.0, Down); 	  _Str(500,400,"18");

/*	LCDSHAPE_Rectangle_Indirect(par3); 	  _Str(530,355,"11");
	LCDSHAPE_Rectangle_Indirect(par4); 	  _Str(615,380,"12");
	LCDSHAPE_Rectangle_Indirect(par5); 	  _Str(590,250,"13");
	LCDSHAPE_Rectangle_Indirect(par6);	  _Str(590,310,"14");
*/
	LCD_RoundRectangleTransp	 (0, LCD_X,LCD_Y,   5,230, 120,60, WHITE, 					  DARKYELLOW, READ_BGCOLOR, 0.5);		/* version of background transparent (mixed with 2 colors) */
	LCD_RoundFrameTransp			 (0, LCD_X,LCD_Y, 670,230, 120,60, WHITE, 					  unUsed, 	  READ_BGCOLOR, 0.5);
	LCD_BoldRoundRectangleTransp(0, LCD_X,LCD_Y,   5,230, 120,60, SetBold2Color(WHITE,6), DARKYELLOW, READ_BGCOLOR, 0.5);
	LCD_BoldRoundFrameTransp	 (0, LCD_X,LCD_Y, 670,230, 120,60, SetBold2Color(WHITE,6), unUsed, 	  READ_BGCOLOR, 0.5);
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

	LCDSHAPE_Create(0,LCD_X,LCD_Y,LCD_X-calcWidth-10-450, LCD_Y-calcWidth-30, SetParamWidthCircle(0,CircleWidth),CircleWidth, 					SetBold2Color(WHITE,30), 			COLOR_BkScreen, 		COLOR_BkScreen, RED,BLUE,0,360,RightDown,1);
	LCDSHAPE_Create(0,LCD_X,LCD_Y,LCD_X-calcWidth-10-150, LCD_Y-calcWidth-30, SetParamWidthCircle(Percent_Circle,CircleWidth),CircleWidth, SetBold2Color(COLOR_BkScreen,0), COLOR_FillMainFrame, COLOR_BkScreen, RED,BLUE,0,350,Left,1);
	LCDSHAPE_Create(0,LCD_X,LCD_Y,LCD_X-calcWidth-10, 	   LCD_Y-calcWidth-50, SetParamWidthCircle(Percent_Circle,CircleWidth),CircleWidth, SetBold2Color(COLOR_BkScreen,0), COLOR_FillMainFrame, COLOR_BkScreen, RED,BLUE,0,355,Right,0);
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
	LCD_STR_PARAM temp, new;		/* For all methods below 'new' =='temp' */
	new =  LCD_Txt(noDisplay, NULL, 0,0, LCD_X,LCD_Y, FONT_ID_Fonts, FONT_VAR_Fonts, 320,200, "12345", BLACK, COLOR_BkScreen, fullHight,0,250, NoConstWidth, 0x777777, 2, RightDown);	/* load parameters to 'new' */
	temp = LCD_Txt(noDisplay, &new, 0,0, LCD_X,LCD_Y, FONT_ID_Fonts, FONT_VAR_Fonts, 320,200, "12345", BLACK, COLOR_BkScreen, fullHight,0,250, NoConstWidth, 0x777777, 2, RightDown);	/* load parameters to 'new' and 'temp' */
	temp = LCD_Txt(Display,   NULL, 0,0, LCD_X,LCD_Y, FONT_ID_Fonts, FONT_VAR_Fonts, 320,200, "12345", BLACK, COLOR_BkScreen, fullHight,0,250, NoConstWidth, 0x777777, 2, RightDown);	/* 1 method */
	temp = LCD_Txt(Display,   &new, 0,0, LCD_X,LCD_Y, FONT_ID_Fonts, FONT_VAR_Fonts, 320,200, "12345", BLACK, COLOR_BkScreen, fullHight,0,250, NoConstWidth, 0x777777, 2, RightDown);	/* 2 method */

	LCD_Shape(new.txt.pos.x, new.txt.pos.y, LCD_Frame,  new.txt.size.w+new.shadow.deep, new.txt.size.h+new.shadow.deep,  RED,COLOR_BkScreen,COLOR_BkScreen);	/* test frame around the text */

	temp = LCD_Txt(DisplayViaStruct,&new,NO_TXT_ARGS);		/* 3 method */
	temp = LCD_TxtVar(&new,NULL);									/* 4 method */
	temp = LCD_TxtVar(&new,"new text");							/* 5 method */
	temp = temp;
	/* -------  END -------- */

	/* 5 Methods for Indirect */
	LCD_STR_PARAM temp2,new2;		/* For all methods below 'new2' =='temp2' */
	temp2 = LCD_Txt(noDisplay, &new2, 50,200, BK_SIZE_IS_TXT_SIZE, FONT_ID_Fonts, FONT_VAR_Fonts, 0,0, "12345", WHITE, COLOR_BkScreen, fullHight,0,250, NoConstWidth, 0x777777, 3, RightDown);	/* load parameters to 'new2' and 'temp2' */
	new2 =  LCD_Txt(noDisplay, NULL,  50,200, BK_SIZE_IS_TXT_SIZE, FONT_ID_Fonts, FONT_VAR_Fonts, 0,0, "12345", WHITE, COLOR_BkScreen, fullHight,0,250, NoConstWidth, 0x777777, 3, RightDown);	/* load parameters to 'new2' */
												/* {BkpSizeX,BkpSizeXY}=0 because is calculated as text length  */
	temp2 = LCD_Txt(DisplayIndirectViaStruct,&new2,NO_TXT_ARGS);	/* 1 method */
	temp2 = LCD_TxtVarInd(&new2, NULL);										/* 2 method */
	temp2 = LCD_TxtVarInd(&new2, "new text");								/* 3 method */
	temp2 = LCD_Txt(DisplayIndirect, &new2, 50,250, BK_SIZE_IS_TXT_SIZE, FONT_ID_Fonts, FONT_VAR_Fonts, 0,0, "abcde", WHITE, COLOR_BkScreen, fullHight,0,250, NoConstWidth, 0x777777, 6, RightDown);		/* 4 method */
	temp2 = LCD_Txt(DisplayIndirect, NULL,  50,250, BK_SIZE_IS_TXT_SIZE, FONT_ID_Fonts, FONT_VAR_Fonts, 0,0, "abcde", WHITE, COLOR_BkScreen, fullHight,0,250, NoConstWidth, 0x777777, 6, RightDown);		/* 5 method */
	temp2 = temp2;
	/* -------  END -------- */
}

void LCDEXAMPLE_DrawLine(u32 COLOR_Frame, u32 COLOR_BkScreen)
{
	/* 1 Show 'DrawLine()' for degree (45-11, 45) */
	CorrectLineAA_off();
	for(int i=0; i<15; i++)
		DrawLine(0,130,220+i*17, 150, 45-i, COLOR_Frame,LCD_X, 1.0, 1.0 ,COLOR_BkScreen,COLOR_BkScreen);

	CorrectLineAA_on();
	for(int i=0; i<15; i++)
		DrawLine(0,300,220+i*17, 150, 45-i, COLOR_Frame,LCD_X, 0.0, 0.0 ,COLOR_BkScreen,COLOR_BkScreen);		/* for i=12,13,14 we don`t have yet special correct (thicker line) */

	/* 2 Show 'DrawLine()' for degree (45, 45+11) */
	CorrectLineAA_off();
	for(int i=0; i<15; i++)
		DrawLine(0,430+i*10,350, 150, 45+i, COLOR_Frame,LCD_X, 1.0, 1.0 ,COLOR_BkScreen,COLOR_BkScreen);

	CorrectLineAA_on();
	for(int i=0; i<15; i++)
		DrawLine(0,620+i*10,350, 150, 45+i, COLOR_Frame,LCD_X, 0.0, 0.0 ,COLOR_BkScreen,COLOR_BkScreen);		/* for i=12,13,14 we don`t have yet special correct (thicker line) */
}

void MEMEXAMPLE_MemoryFunc(void)
{
	/* Common variables for all number of functions */
	int valueToMemory = 0x55;
	int retValue = 0;
	int retFlag = 0;
	int *pRetValue = NULL;
	char *pChar = NULL;
	char vChar = 0;
	char strBuff[] = "example";
	uint16_t buff16[] = {1,2,3};
	uint32_t buff32[] = {1,2,3};
	uint32_t lenBuff = 3;
	uint32_t freeMemorySize = 0;

	/* 1. Functions of volatile memory */
	retValue = FV(SetVal,0,valueToMemory);		/* retValue == valeToMemory */
	retValue = FV(SetVal,1,valueToMemory);		/* memory number 0,1,2,..  not exceed value _FV_MEMORY_SIZE */
	retValue = FV(SetVal,2,valueToMemory);

	retValue = FV(GetVal,0,NoUse);
	retValue = FV(GetVal,1,NoUse);
	retValue = FV(GetVal,2,NoUse);
	retValue = retValue;

	/* 2. Functions of volatile memory */
	pRetValue = _Int(valueToMemory);				/* address memory is auto-scroll when exceed value _BUFF_VAL_SIZE */
	pRetValue = (int*)_Uint8((uint8_t)valueToMemory);
	pRetValue = (int*)_Uint16((uint16_t)valueToMemory);
	pRetValue = (int*)_Uint32((uint32_t)valueToMemory);
	pRetValue = (int*)_Int16((int16_t)valueToMemory);
	pRetValue = (int*)_Int32((int32_t)valueToMemory);
	pRetValue = (int*)_Float((float)valueToMemory);
	pRetValue = pRetValue;

	/* 3. Functions of volatile memory */
	freeMemorySize = GETVAL_freeMemSize(0);
	pChar = GETVAL_ptr(0);		/* for this memory type use Semphr_sdram */
	pChar = GETVAL_ptr(1);
	pChar = GETVAL_ptr(2);
	vChar = GETVAL_char(0);
	retFlag = GETVAL_str(0, strBuff, strlen(strBuff));		/* When address memory 0,1,2,... exceeds return 1 and no-write otherwise return 0 and ok-write */
	valueToMemory = (int)GETVAL_int16(0);
	valueToMemory = (int)GETVAL_int32(0);
	retFlag = GETVAL_array16(0, buff16, lenBuff);
	retFlag = GETVAL_array32(0, buff32, lenBuff);

	retFlag = SETVAL_char(0, (char)valueToMemory);
	retFlag = SETVAL_char(1, (char)valueToMemory);
	retFlag = SETVAL_char(2, (char)valueToMemory);
	retFlag = SETVAL_str(0, strBuff, strlen(strBuff));
	retFlag = SETVAL_int16(0, (uint16_t)valueToMemory);
	retFlag = SETVAL_int32(0, (uint32_t)valueToMemory);
	retFlag = SETVAL_array16(0, buff16, lenBuff);
	retFlag = SETVAL_array32(0, buff32, lenBuff);

	_NOT_USED(freeMemorySize);
	_NOT_USED(retFlag);
	_NOT_USED(pChar);
	_NOT_USED(vChar);
}

void DBGEXAMPLE_KeyFunctions(void)
{
	double testVal = 0.0;
	void* FuncExample(void *arg1,void *arg2){ return NULL; }

	if(DEBUG_RcvStr("example")){ ; }

	/* 1. Debug option */
	_DBG_PARAM_NOWRAP("a",&testVal,_double,_Incr,_Double(0.1),_Double(20.0),"Test debug 1: ",FuncExample)		/* for 'a' increment value by step 0.1 */
	_DBG_PARAM_NOWRAP("b",&testVal,_double,_Decr,_Double(0.1),_Double( 1.0),"Test debug 1: ",FuncExample)		/* for 'b' decrement value by step 0.1 */

	/* 2. Debug option */
	_DBG2_PARAM_NOWRAP("a","A",&testVal,_double,_Incr,_Double(0.1),_Double(1.3),_Double(20.0),"Test debug 2: ",FuncExample,NULL)		/* for 'a' increment value by step 0.1 but for 'A' increment value by step 1.3 */
	_DBG2_PARAM_NOWRAP("b","B",&testVal,_double,_Decr,_Double(0.1),_Double(1.3),_Double( 1.0),"Test debug 2: ",FuncExample,NULL)		/* for 'b' decrement value by step 0.1 but for 'B' decrement value by step 1.3 */

	/* 3. Debug option */
	_DBG3_PARAM_NOWRAP("a","A","b","B",&testVal,_double,_Double(0.1),_Double(1.5),_Double(20.0),_Double(1.0),"Test debug 3: ",FuncExample,NULL)		/* for 'a' increment value by step 0.1 but for 'A' increment value by step 1.5 */
																																																	/* for 'b' decrement value by step 0.1 but for 'B' decrement value by step 1.5 */
}

void LCDEXAMPLE_Graph(int displayOption, USER_GRAPH_PARAM graph, int touchStart, int FONT_ID_Descr)
{
	switch(displayOption)
	{
	case 0:
		break;


	case 1:
		break;
	}

	/* 1 - GRAPH option */
	int offsMem = 0;
	int nrChart = 0;											/* must be always negative 	  must be always positive */
	int xPosChart = 50, 	yMiddPosChart = 250;			int yOffsToTopChart = -100,  yOffsToBottomChart = 100;				int widthChart = 700;
	int chartPtrPos = 70;

	u32 colorPosXY 	= RED;			int offsK_PosXY 	 = 20*LCD_X+0;
	u32 colorPosXYrep = DARKGREEN;	int offsK_PosXYrep = 40*LCD_X+0;

	u32 colorLine45degCorrectOFF = graph.colorLinePosXY;			 int offsK_45degCorrectOFF = graph.KoffsPosXY;
	u32 colorLineAAoff 			  = graph.colorLinePosXYrep;	 	 int offsK_AAoff 			   = graph.KoffsPosXYrep;

	void _ResetChartsTouchs(void){	LOOP_FOR(i,3){	LCD_TOUCH_DeleteSelectTouch(touchStart+i); }	 }

	switch(graph.grad.bkType)		/* Alternating display of charts */
	{																																																																								  			/* Draw line (colorLine) but OutColor and InColor (needed for AA) is read from bkColor */
																																																																																/* Alternative options:  0,0  ->  means READ_BK							 						Disp_no																															 							GRAD_None 																			 CHART_PTR_NONE */
	case 0:	GRAPH_GetSamplesAndDraw(0, NR_MEM(offsMem,nrChart), LCD_X, XYPOS_YMIN_YMAX(xPosChart,yMiddPosChart, yOffsToTopChart,yOffsToBottomChart), POINTS_STEP_XYSCALE(widthChart,1.0, graph.par.scaleX,graph.par.scaleY), FUNC_TYPE(graph.funcType), LINE_COLOR(WHITE,0,0), AA_VAL(graph.AAoutCoeff,graph.AAinCoeff), 	DRAW_OPT(Disp_AA, 								  colorLine45degCorrectOFF, colorLineAAoff,   offsK_45degCorrectOFF, offsK_AAoff	  ), 	GRAD_YmaxYmin(ORANGE), 		GRAD_COEFF(1.0,0.0), graph.corr45degAA, CHART_PTR_DEFAULT(							  chartPtrPos,												FONT_ID_Descr) );		if(touchStart){ _ResetChartsTouchs(); LCDTOUCH_Set(xPosChart, yMiddPosChart-ABS(yOffsToTopChart), widthChart, ABS(yOffsToTopChart)+yOffsToBottomChart, ID_TOUCH_GET_ANY_POINT, touchStart+0, pressRelease); }	break;
	case 1:	GRAPH_GetSamplesAndDraw(0, NR_MEM(offsMem,nrChart), LCD_X, XYPOS_YMIN_YMAX(xPosChart,yMiddPosChart, yOffsToTopChart,yOffsToBottomChart), POINTS_STEP_XYSCALE(widthChart,1.0, graph.par.scaleX,graph.par.scaleY), FUNC_TYPE(graph.funcType), LINE_COLOR(WHITE,0,0), AA_VAL(graph.AAoutCoeff,graph.AAinCoeff), 	DRAW_OPT(Disp_AA,  							     colorLine45degCorrectOFF, colorLineAAoff,   offsK_45degCorrectOFF, offsK_AAoff	  ), 	GRAD_Ystrip(DARKGREEN,51),	GRAD_COEFF(1.0,0.0), graph.corr45degAA, CHART_PTR_DEFAULT(							  chartPtrPos,												FONT_ID_Descr) );		if(touchStart){ _ResetChartsTouchs(); LCDTOUCH_Set(xPosChart, yMiddPosChart-ABS(yOffsToTopChart), widthChart, ABS(yOffsToTopChart)+yOffsToBottomChart, ID_TOUCH_GET_ANY_POINT, touchStart+1, pressRelease); }	break;
	case 2:  GRAPH_GetSamplesAndDraw(0, NR_MEM(offsMem,nrChart), LCD_X, XYPOS_YMIN_YMAX(xPosChart,yMiddPosChart, yOffsToTopChart,yOffsToBottomChart), POINTS_STEP_XYSCALE(widthChart,1.0, graph.par.scaleX,graph.par.scaleY), FUNC_TYPE(graph.funcType), LINE_COLOR(WHITE,0,0), AA_VAL(graph.AAoutCoeff,graph.AAinCoeff), 	DRAW_OPT(Disp_AA|Disp_posXY|Disp_posXYrep,  colorPosXY, 				  	 colorPosXYrep,    offsK_PosXY, 			 	offsK_PosXYrep), 	GRAD_Ycolor(RED,BLUE), 		GRAD_COEFF(1.0,0.0), graph.corr45degAA, CHART_PTR_PARAM  (LIGHTBLUE,DARKBLUE,18,chartPtrPos,1, WHITE,DARKYELLOW,80,30,10,10,1,FONT_ID_Descr) );		if(touchStart){ _ResetChartsTouchs(); LCDTOUCH_Set(xPosChart, yMiddPosChart-ABS(yOffsToTopChart), widthChart, ABS(yOffsToTopChart)+yOffsToBottomChart, ID_TOUCH_GET_ANY_POINT, touchStart+2, pressRelease); }	break;
																																																																																									 /* AA_VAL(0.0,0.0) and AA_ON  is the same 		DRAW_AA */			/*	Disp_all 																																 	GRAD_PARAM(...) */
																																																																																									 /* AA_VAL(1.0,1.0) and AA_OFF is the same 		DRAW_NO  only get samples without draw graph */
	}



	/* 2 - GRAPH option */

	int offsMem2[3] = { 0, 0, 0 };
	int nrChart2[3] = { 0, 1, 2 };																	/* must be always negative 	  must be always positive */
	int xPosChart2[3] = {50, 300, 0}, 	yMiddPosChart2[3] = {250, 250, 100};		int yOffsToTopChart2[3] = {-100, -80, -100},  yOffsToBottomChart2[3] = {100, 80, 100};			int widthChart2[3] = {200, 200, 240};
	int chartPtrPos2[3] = {70,70,70};

//
		GRAPH_GetSamples(NR_MEM(offsMem2[0],nrChart2[0]), XYPOS_YMIN_YMAX(xPosChart2[0],yMiddPosChart2[0], yOffsToTopChart2[0],yOffsToBottomChart2[0]), POINTS_STEP_XYSCALE(widthChart2[0],1.0, graph.par.scaleX,graph.par.scaleY), FUNC_TYPE(Func_sin));
		GRAPH_GetSamples(NR_MEM(offsMem2[1],nrChart2[1]), XYPOS_YMIN_YMAX(xPosChart2[1],yMiddPosChart2[1], yOffsToTopChart2[1],yOffsToBottomChart2[1]), POINTS_STEP_XYSCALE(widthChart2[1],1.0, graph.par.scaleX,graph.par.scaleY), FUNC_TYPE(Func_sin));
		GRAPH_GetSamples(NR_MEM(offsMem2[2],nrChart2[2]), XYPOS_YMIN_YMAX(xPosChart2[2],yMiddPosChart2[2], yOffsToTopChart2[2],yOffsToBottomChart2[2]), POINTS_STEP_XYSCALE(widthChart2[2],1.0, graph.par.scaleX,graph.par.scaleY), FUNC_TYPE(Func_sin));
	//
	//	USER_GRAPH_PARAM par1, par2, par3;				//Wazen wycieki pamieci (brak ogr) dla 	pomocnych krzywych Disp_posXY i Disp_posXYrep !!!!!!!!!!!!!!!
	//																								/* alternative: LINE_AACOLOR(WHITE,0,0) */
	//	par1 = LCD_Chart(ToStructAndReturn, 0, WIDTH_BK(LCD_X), LINE_AA_BKCOLOR(WHITE,0,0,v.COLOR_BkScreen), AA_VAL(testGraph.AAoutCoeff,testGraph.AAinCoeff), DRAW_OPT(Disp_AA/*|Disp_posXY|Disp_posXYrep*/, WHITE,WHITE, 0,0/*40*LCD_X-0, 80*LCD_X-0*/), /*GRAD_None*/GRAD_YmaxYmin(ORANGE), GRAD_COEFF(1.0,0.0),testGraph.corr45degAA, CHART_PTR_DEFAULT(chartPtrPos[0],v.FONT_ID_Descr));
	//	par2 = LCD_Chart(ToStructAndReturn, 1, WIDTH_BK(LCD_X), LINE_AA_BKCOLOR(WHITE,0,0,v.COLOR_BkScreen), AA_VAL(testGraph.AAoutCoeff,testGraph.AAinCoeff), DRAW_OPT(Disp_AA/*|Disp_posXY|Disp_posXYrep*/, WHITE,WHITE, 0,0/*40*LCD_X-0, 80*LCD_X-0*/), /*GRAD_None*/GRAD_YmaxYmin(ORANGE), GRAD_COEFF(1.0,0.0),testGraph.corr45degAA, CHART_PTR_DEFAULT(chartPtrPos[1],v.FONT_ID_Descr));
	//
	//	int widthBk = GRAPH_GetNmbrPoints(2);  ///naprawic wyciek gdy coraz wieksaz ampl !!!!! dla indirect
	//	par3 = LCD_Chart(ToStructAndReturn, 2, XY_WIN(550,250), LINE_AA_BKCOLOR(WHITE,0,0,v.COLOR_BkScreen), AA_VAL(testGraph.AAoutCoeff,testGraph.AAinCoeff), DRAW_OPT(Disp_AA/*|Disp_posXY|Disp_posXYrep*/, WHITE,WHITE, 0*widthBk-0, 0*widthBk-0), /*GRAD_None*/GRAD_YmaxYmin(ORANGE), GRAD_COEFF(1.0,0.0),testGraph.corr45degAA, CHART_PTR_DEFAULT(chartPtrPos[2],v.FONT_ID_Descr));
	//

	//	LCDTOUCH_Set( 50,250-100, 200,200, ID_TOUCH_GET_ANY_POINT, Touch_Chart_1, pressRelease);
	//	LCDTOUCH_Set(300,250- 80, 200,160, ID_TOUCH_GET_ANY_POINT, Touch_Chart_2, pressRelease);
	//	LCDTOUCH_Set(550,250-100, 240,200, ID_TOUCH_GET_ANY_POINT, Touch_Chart_3, pressRelease);

	//-----CHART  opt 1 --------  /* Simultaneous display of charts */
	//	LCD_Chart(0, MEM_3,				 LCD_X, LINE_AA_BKCOLOR(WHITE,0,0,v.COLOR_BkScreen), AA_VAL(testGraph.AAoutCoeff,testGraph.AAinCoeff), DRAW_OPT(Disp_AA|Disp_posXY|Disp_posXYrep, WHITE,WHITE, 50*LCD_X-0, 100*LCD_X-0), /*GRAD_None*/GRAD_YmaxYmin(ORANGE), GRAD_COEFF(1.0,0.0),testGraph.corr45degAA, CHART_PTR_DEFAULT(50,v.FONT_ID_Descr));
	//	LCD_Chart(0, NR_MEM(4000000,1),LCD_X, LINE_AA_BKCOLOR(WHITE,0,0,v.COLOR_BkScreen), AA_VAL(testGraph.AAoutCoeff,testGraph.AAinCoeff), DRAW_OPT(Disp_AA/*|Disp_posXY|Disp_posXYrep*/, WHITE,WHITE, 0,0/*50*LCD_X-0, 100*LCD_X-0*/), /*GRAD_None*/GRAD_YmaxYmin(ORANGE), GRAD_COEFF(1.0,0.0),testGraph.corr45degAA, CHART_PTR_DEFAULT(chartPtrPos[1],v.FONT_ID_Descr));
	//	LCD_Chart(0, NR_MEM(2000000,0),LCD_X, LINE_AA_BKCOLOR(WHITE,0,0,v.COLOR_BkScreen), AA_VAL(testGraph.AAoutCoeff,testGraph.AAinCoeff), DRAW_OPT(Disp_AA/*|Disp_posXY|Disp_posXYrep*/, WHITE,WHITE, 0,0/*50*LCD_X-0, 100*LCD_X-0*/), /*GRAD_None*/GRAD_YmaxYmin(ORANGE), GRAD_COEFF(1.0,0.0),testGraph.corr45degAA, CHART_PTR_DEFAULT(chartPtrPos[0],v.FONT_ID_Descr));

	//-----CHART  opt 2 --------
	//	LCDSHAPE_Chart(0,par1);
	//	LCDSHAPE_Chart(0,par2);
	//
	////-----CHART  opt 3 --------
	//	LCDSHAPE_Chart_Indirect(par3);








	/* Debug Test GRAPH		---	 use only if debug is necessary in function FILE_NAME(debugRcvStr)() */
	/*
 	 int startFuncType = Func_sin,	stopFuncType = Func_lines6;

	_DBG3_PARAM_NOWRAP("a","A","z","Z",&graph.par.scaleX,_float,_Float(0.1),_Float( 1.5),_Float( 20.0),_Float(1.0),"Test Graph scaleX: ",MainFuncRefresh,NULL)
	_DBG3_PARAM_NOWRAP("s","S","x","X",&graph.par.scaleY,_float,_Float(1.0),_Float(10.0),_Float(100.0),_Float(1.0),"Test Graph scaleY: ",MainFuncRefresh,NULL)

	_DBG_PARAM_NOWRAP("d",&graph.funcType,_uint8,_Incr,_Uint8(1),_Uint8(stopFuncType),"Test Graph funcType: ",MainFuncRefresh)
	_DBG_PARAM_NOWRAP("c",&graph.funcType,_uint8,_Decr,_Uint8(1),_Uint8(startFuncType),	 "Test Graph funcType: ",MainFuncRefresh)

	_DBG_PARAM_NOWRAP("f",&graph.AAoutCoeff,_float,_Incr,_Float(0.1),_Float(1.0),"Test Graph AA out: ",MainFuncRefresh)
	_DBG_PARAM_NOWRAP("v",&graph.AAoutCoeff,_float,_Decr,_Float(0.1),_Float(0.0),"Test Graph AA out: ",MainFuncRefresh)

	_DBG_PARAM_NOWRAP("g",&graph.AAinCoeff,_float,_Incr,_Float(0.1),_Float(1.0),"Test Graph AA in: ",MainFuncRefresh)
	_DBG_PARAM_NOWRAP("b",&graph.AAinCoeff,_float,_Decr,_Float(0.1),_Float(0.0),"Test Graph AA in: ",MainFuncRefresh)

	_DBG_PARAM_WRAP("y",&graph.corr45degAA,_int,_Wrap,_Int(1), _Int(0),_Int(1), "Test Graph AA 45deg: ",MainFuncRefresh)

	_DBG_PARAM_WRAP("q",&graph.grad.bkType,_int,_Wrap,_Int(1), _Int(Grad_YmaxYmin),_Int(Grad_Ycolor), "Test Graph grad type: ",MainFuncRefresh)

	 */

	/* Touch Test GRAPH		---	 use only if debug is necessary in function FILE_NAME(setTouch)() */
	/*
		case Touch_Chart_1:	if(GRAPH_IsMemReloaded(0)) FILE_NAME(main)(LoadPartScreen,(char**)ppMain); 	GRAPH_ptrTouchService(pos.x,pos.y,nrChart+0);	break;		// Attention:  Charts use memory for samples pointed by CounterBusyBytesForFontsImages what is changed by load fonts function...
		case Touch_Chart_2:	if(GRAPH_IsMemReloaded(1)) FILE_NAME(main)(LoadPartScreen,(char**)ppMain); 	GRAPH_ptrTouchService(pos.x,pos.y,nrChart+1);	break;		// 				... and you must reloaded charts if you have changed CounterBusyBytesForFontsImages before.
		case Touch_Chart_3:	if(GRAPH_IsMemReloaded(2)) FILE_NAME(main)(LoadPartScreen,(char**)ppMain); 	GRAPH_ptrTouchService(pos.x,pos.y,nrChart+2);	break;
	 */

}

void LCDEXAMPLE_CreateLoadDisplayFonts(u32 COLOR_BkScreen)
{
	/* 1. Creating all fonts from file.bmp to compressed file.ccf */
	LCD_CreateFileCFFfromAllFilesBMP();
	LCD_CkeckAllFontFilesCFF();
	LCD_DeleteAllFontAndImages();

	/* 2. Creating selected fonts from file.bmp to compressed file.ccf */
	LCD_DeleteAllFontAndImages();
	if( LCD_CreateFileCFFfromBMP(FONT_72, Comic_Saens_MS, RGB_RGB) < 0)
		_NOP;
	LCD_DeleteAllFontAndImages();
	if( LCD_CreateFileCFFfromBMP(FONT_72_bold, Comic_Saens_MS, RGB_RGB) < 0)
		_NOP;
	LCD_DeleteAllFontAndImages();
	if( LCD_CreateFileCFFfromBMP(FONT_72_italics, Comic_Saens_MS, RGB_RGB) < 0)
		_NOP;
	LCD_DeleteAllFontAndImages();

	/* 3. Loading selected fonts from compressed file.ccf to SDRAM */
	if( LCD_LoadFontFromFileCFF(FONT_72, Comic_Saens_MS, RGB_RGB, fontID_1) < 0)
		_NOP;
	if( LCD_LoadFontFromFileCFF(FONT_72_bold, Comic_Saens_MS, RGB_RGB, fontID_2) < 0)
		_NOP;
	if( LCD_LoadFontFromFileCFF(FONT_72_italics, Comic_Saens_MS, RGB_RGB, fontID_3) < 0)
		_NOP;

	/* 4. Displaying fonts from SDRAM before loading from compressed file.ccf */
	extern uint32_t pLcd[];
	LCD_DisplayTxt(0,pLcd,0,fontID_1, "+23.09 -76.9", 0,0, LCD_GetXSize(),LCD_GetYSize(), 5,0, 	COLOR_BkScreen,YELLOW, 0,NoConstWidth,fullHight,0);
	LCD_DisplayTxt(0,pLcd,0,fontID_2, "+23.09 -76.9", 0,0, LCD_GetXSize(),LCD_GetYSize(), 5,150, COLOR_BkScreen,YELLOW, 0,NoConstWidth,fullHight,0);
	LCD_DisplayTxt(0,pLcd,0,fontID_3, "+23.09 -76.9", 0,0, LCD_GetXSize(),LCD_GetYSize(), 5,300, COLOR_BkScreen,YELLOW, 0,ConstWidth,  fullHight,0);

	LCD_Show();
	while(1);
}

