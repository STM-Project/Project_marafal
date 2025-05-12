/*
 * LCD_BasicGaphics.h
 *
 *  Created on: 11.04.2021
 *      Author: Elektronika RM
 */

#ifndef LCD_LCD_BASICGAPHICS_H_
#define LCD_LCD_BASICGAPHICS_H_

#include "stm32f7xx_hal.h"
#include "common.h"

#define RATIO_AA_VALUE_MIN 	1.0
#define RATIO_AA_VALUE_MAX 	0.0

#define LCD_X 	LCD_GetXSize()
#define LCD_Y 	LCD_GetYSize()

#define NMB_SLIDER_ELEMENTS	3

#define CIRCLE_POS_XY(width,offsX,offsY)		LCD_GetXSize()-LCD_CalculateCircleWidth(width)-(offsX), LCD_GetYSize()-LCD_CalculateCircleWidth(width)-(offsY)
#define SetBold2Color(frameColor,thickness)		(((frameColor)&0xFFFFFF) | (((((uint32_t)(thickness))<<24)&0xFF000000)))

#define NORMAL_SLIDER_PARAM	2,8,2,2
#define NORMAL_SLIDER_PARAM2	2,6,2,1

#define BK_COLOR_CIRCLESLIDER(p)					GetTransitionColor(p.param[0], p.param[1],0.5)					/* this is yet NOT imported !!!  we read bkColor for LCD_StrChangeColor() */
#define BK_COLOR_CIRCLEBUTTON(p)					GetTransitionColor(p.color[0].fill, p.color[1].fill,0.5)		/* this is yet NOT imported !!!  we read bkColor for LCD_StrChangeColor() */
#define POS_SIZE_CIRCLEBUTTONSLIDER(p,offsX,offsY)					p.pos[0].x+(offsX), p.pos[0].y+(offsY), p.size[0].w, p.size[0].h
#define POS_SIZE_CIRCLEBUTTONSLIDER_indirect(p,offsX,offsY)		p.pos[0].x+(offsX), p.pos[0].y+(offsY), p.size[0].w+2, p.size[0].h+2			/*	'+2' because in LCD_GradientCircleButton_Indirect() and LCD_GradientCircleSlider_Indirect() we have '+2' for 'bkSizeX' and 'bkSizeY' */

#define POINTS_STEP_XYSCALE(p1,p2,p3,p4)	p1,p2,p3,p4
#define FUNC_TYPE(func)	 func
#define LINE_COLOR(line,outLine,inLine)	 				 line,outLine,inLine
#define LINE_AA_BKCOLOR(line,outLine,inLine,bkRect)	 line,outLine,inLine,bkRect
#define LINE_AACOLOR(line,outLine,inLine)	 	 		 	 line,outLine,inLine,0
#define READ_BK	 0,0
#define OUT_IN_COLOR(out,in)	 out,in
#define AA_ON	 0.0,0.0
#define AA_OFF	 1.0,1.0
#define AA_VAL(outRatio,inRatio)	 outRatio,inRatio
#define DRAW_OPT(flag,color1,color2,offs1,offs2)		flag,color1,color2,offs1,offs2
#define WIDTH_BK(w)		w
#define XY_WIN(x,y)		(x)<<16|(y)

#define GRAD_PARAM(bkGradType,fromGradColor,toGradColor,gradStripY,amplTrans,offsTrans)		bkGradType,fromGradColor,toGradColor,gradStripY,amplTrans,offsTrans
#define GRAD_YmaxYmin(gradColor)							Grad_YmaxYmin,gradColor,unUsed,unUsed
#define GRAD_Ystrip(gradColor,stripLen)				Grad_Ystrip,gradColor,unUsed,stripLen
#define GRAD_Ycolor(fromGradColor,toGradColor)		Grad_Ycolor,fromGradColor,toGradColor,unUsed
#define GRAD_None												-1,unUsed,unUsed,unUsed
#define GRAD_COEFF(amplTrans,offsTrans)				amplTrans,offsTrans

#define DRAW_NO	 0,0,0,0,0
#define DRAW_AA	 Disp_AA,0,0,0,0
#define XYPOS_YMIN_YMAX(x,y,yMIn,yMax) 	x,y,yMIn,yMax
#define NR_MEM(offs,nr) 		offs,nr

typedef enum{
	Disp_no,
	Disp_AA,
	Disp_posXY,				 /* display posXY */
	Disp_posXYrep = 4,	 /* display repetition redundancy of posXY */
	Disp_all	=	Disp_AA | Disp_posXY | Disp_posXYrep
}DISP_OPTION;

enum CircleParam{
	Half_Circle_0,
	Half_Circle_90,
	Half_Circle_180,
	Half_Circle_270,
	Percent_Circle,
};

enum COPY_TO_SHAPE_STRUCT{
	ToStructAndReturn = 0x80000001,
	ToStructAndDisplay = 0x80000000
};

typedef enum{
	Func_sin,
	Func_sin1,
	Func_sin2,
	Func_sin3,
	Func_cos,
	Func_log,
	Func_tan,
	Func_noise,
	Func_noise2,
	Func_lines1,
	Func_lines2,
	Func_lines3,
	Func_lines4,
	Func_lines5,
	Func_lines6,
	Func_owner,
}GRAPH_FUNC;

typedef enum{
	DelTriang = -1,
	NoSel,
	LeftSel,
	PtrSel,
	RightSel,
	Percent,
	PosXY,
}SLIDER_PARAMS;

typedef enum{
	bold0,bold1,bold2,bold3,bold4,bold5,bold6,bold7,bold8,bold9,bold10,bold11,bold12,bold13,bold14,bold15
}BOLD_PARAMS;

typedef enum{
	Frame,
	Rectangle
}SHAPE_TYPES;

typedef enum{
	NoReadOutColor,
	ReadOutColor
}SHAPE_FLAGS;

typedef enum{
	Grad_YmaxYmin,
	Grad_Ystrip,
	Grad_Ycolor,
}GRADIENT_GRAPH_TYPE;

#define K_TAB_SIZE 	4
typedef struct{
	u32 k[K_TAB_SIZE];
}structK;

typedef struct{
	DIRECTIONS dir;
	u64 gradSlidColor;
	u32 middSlidColor;
	u64 gradButtColor;
}SlidButtCircParam;

typedef struct{
	u16 x,y;
	int16_t rx,ry;
}structRepPos;

typedef struct{
	int startX; int startY; int yMin; int yMax; int nmbrPoints; float precision; float scaleX; float scaleY; int funcPatternType;			/* ! Attention !  'structGetSmpl' is located in SDRAM (32 bit data bus) so write to 'u64' or 'double' variable generates hard-fault */
	int len_posXY;  int len_posXYrep;
}structGetSmpl;

typedef struct{
	GRADIENT_GRAPH_TYPE bkType;
	u32 fromColor;
	u32 toColor;
	u8 stripY;
	float amplTrans;
	float offsTrans;
}structGradParam;

typedef struct{
	u32 offsMem; u32 nrMem;		/* only for GRAPH_MEMORY_SDRAM2 */
	structGetSmpl par;
	u32 widthBk;					/* for Indirect Display used as window position x,y */
	u8 funcType;
	u32 bkRectColor;
	u32 lineColor;
	u32 AAoutColor;
	u32 AAinColor;
	float AAoutCoeff;
	float AAinCoeff;
	DISP_OPTION dispOpt;
	u32 colorLinePosXY;
	u32 colorLinePosXYrep;
	int KoffsPosXY;
	int KoffsPosXYrep;
	u8 corr45degAA;
	structGradParam grad;
}USER_GRAPH_PARAM;

extern USER_GRAPH_PARAM USER_GRAPH_PARAM_Zero;

extern uint32_t LCD_GetXSize(void);
extern uint32_t LCD_GetYSize(void);

uint16_t* GET_CIRCLE_correctForWidth(void);
uint16_t* GET_CIRCLE_correctPercDeg(int nr);
float* 	 GET_CIRCLE_errorDecision(int nr);

void SET_CIRCLE_errorDecision(int nr, float decis);
void CIRCLE_errorDecision(int nr, ON_OFF action);

void CorrectLineAA_on(void);
void CorrectLineAA_off(void);
void Set_AACoeff_RoundFrameRectangle(float outCoeff, float inCoeff);

void Set_AACoeff_Draw(int pixelsInOneSide, uint32_t colorFrom, uint32_t colorTo, float ratioStart);

void LCD_LineH(uint32_t BkpSizeX, uint16_t x, uint16_t y, uint16_t width,  uint32_t color, uint16_t bold);
void LCD_LineV(uint32_t BkpSizeX, uint16_t x, uint16_t y, uint16_t width,  uint32_t color, uint16_t bold);

void LCD_Display(uint32_t posBuff, uint32_t Xpos, uint32_t Ypos, uint32_t width, uint32_t height);
void LCD_DisplayPart(uint32_t posBuff, uint32_t Xpos, uint32_t Ypos, uint32_t width, uint32_t height);
void LCD_Show(void);

void LCD_Shape(uint32_t x,uint32_t y,figureShape pShape,uint32_t width,uint32_t height,uint32_t FrameColor,uint32_t FillColor,uint32_t BkpColor);
void LCD_ShapeWindow(figureShape pShape,uint32_t posBuff,uint32_t BkpSizeX,uint32_t BkpSizeY, uint32_t x,uint32_t y, uint32_t width, uint32_t height, uint32_t FrameColor, uint32_t FillColor, uint32_t BkpColor);
void LCD_ShapeIndirect(uint32_t xPos,uint32_t yPos,figureShape pShape, uint32_t width, uint32_t height, uint32_t FrameColor, uint32_t FillColor, uint32_t BkpColor);
void LCD_ShapeWindowIndirect(uint32_t xPos,uint32_t yPos,figureShape pShape,uint32_t posBuff,uint32_t BkpSizeX,uint32_t BkpSizeY, uint32_t x,uint32_t y, uint32_t width, uint32_t height, uint32_t FrameColor, uint32_t FillColor, uint32_t BkpColor);

void LCD_Clear(uint32_t color);
void LCD_ClearPartScreen(uint32_t posBuff, uint32_t BkpSizeX, uint32_t BkpSizeY, uint32_t color);

void LCD_Rectangle		(uint32_t posBuff,uint32_t BkpSizeX,uint32_t BkpSizeY, uint32_t x,uint32_t y, uint32_t width, uint32_t height, uint32_t FrameColor, uint32_t FillColor, uint32_t BkpColor);
void LCD_Frame				(uint32_t posBuff,uint32_t BkpSizeX,uint32_t BkpSizeY, uint32_t x,uint32_t y, uint32_t width, uint32_t height, uint32_t FrameColor, uint32_t FillColor, uint32_t BkpColor);
void LCD_BoldRectangle	(uint32_t posBuff, uint32_t BkpSizeX,uint32_t BkpSizeY, uint32_t x,uint32_t y, uint32_t width, uint32_t height, uint32_t FrameColor, uint32_t FillColor, uint32_t BkpColor);
void LCD_BoldFrame		(uint32_t posBuff, uint32_t BkpSizeX,uint32_t BkpSizeY, uint32_t x,uint32_t y, uint32_t width, uint32_t height, uint32_t FrameColor, uint32_t FillColor, uint32_t BkpColor);

SHAPE_PARAMS LCD_Rectangle2(u32 posBuff,u32 BkpSizeX,u32 BkpSizeY,u32 x,u32 y,u32 width,u32 height,u32 FrameColorStart,u32 FrameColorStop,u32 FillColorStart,u32 FillColorStop,u32 BkpColor,float ratioStart,DIRECTIONS param);
SHAPE_PARAMS LCD_RoundRectangle2(u32 posBuff,int rectangleFrame,u32 BkpSizeX,u32 BkpSizeY,u32 x,u32 y,u32 width,u32 height,u32 FrameColorStart,u32 FrameColorStop,u32 FillColorStart,u32 FillColorStop,u32 BkpColor,float ratioStart,DIRECTIONS direct);

void LCD_RoundFrame			(uint32_t posBuff, uint32_t BkpSizeX,uint32_t BkpSizeY, uint32_t x,uint32_t y, uint32_t width, uint32_t height, uint32_t FrameColor, uint32_t FillColor, uint32_t BkpColor);
void LCD_RoundRectangle		(uint32_t posBuff, uint32_t BkpSizeX,uint32_t BkpSizeY, uint32_t x,uint32_t y, uint32_t width, uint32_t height, uint32_t FrameColor, uint32_t FillColor, uint32_t BkpColor);
void LCD_BoldRoundRectangle(uint32_t posBuff, uint32_t BkpSizeX,uint32_t BkpSizeY, uint32_t x,uint32_t y, uint32_t width, uint32_t height, uint32_t FrameColor, uint32_t FillColor, uint32_t BkpColor);
void LCD_BoldRoundFrame		(uint32_t posBuff, uint32_t BkpSizeX,uint32_t BkpSizeY, uint32_t x,uint32_t y, uint32_t width, uint32_t height, uint32_t FrameColor, uint32_t FillColor, uint32_t BkpColor);

void LCD_LittleRoundRectangle(uint32_t posBuff, uint32_t BkpSizeX,uint32_t BkpSizeY, uint32_t x,uint32_t y, uint32_t width, uint32_t height, uint32_t FrameColor, uint32_t FillColor, uint32_t BkpColor);
void LCD_LittleRoundFrame(uint32_t posBuff, uint32_t BkpSizeX,uint32_t BkpSizeY, uint32_t x,uint32_t y, uint32_t width, uint32_t height, uint32_t FrameColor, uint32_t FillColor, uint32_t BkpColor);

structPosition DrawLine(uint32_t posBuff,uint16_t x, uint16_t y, uint16_t len, uint16_t degree, uint32_t lineColor,uint32_t BkpSizeX, float ratioAA1, float ratioAA2 ,uint32_t bk1Color, uint32_t bk2Color);

void LCD_SetCircleParam(float outRatio, float inRatio, int len, ...);
void LCD_SetCircleDegrees(int len, ...);
void LCD_SetCircleDegreesBuff(int len, uint16_t *buf);
void LCD_SetCircleDegColors(int len, ...);
void LCD_SetCircleDegColorsBuff(int len, uint32_t *buf);
void LCD_SetCirclePercentParam(int degLen, uint16_t *degBuff, uint32_t *degColorBuff);
void LCD_SetCircleDegree(uint8_t degNr, uint8_t deg);
uint16_t LCD_GetCircleDegree(uint8_t degNr);
uint32_t SetParamWidthCircle(uint16_t param, uint32_t width);
uint16_t CenterOfCircle(uint16_t xy, uint16_t width);
void LCD_SetCircleLine(uint8_t lineNr, uint8_t val);
void LCD_OffsCircleLine(uint8_t lineNr, int offs);
void LCD_SetCircleAA(float outRatio, float inRetio);
void LCD_CopyCircleAA(void);
void LCD_SetCopyCircleAA(void);
uint16_t LCD_GetCircleWidth(void);
void LCD_CopyCircleWidth(void);
void LCD_SetCopyCircleWidth(void);
uint16_t LCD_CalculateCircleWidth(uint32_t width);
uint16_t LCD_IncrCircleBold(uint16_t width, uint16_t bold);
uint16_t LCD_DecrCircleBold(uint16_t width, uint16_t bold);
uint16_t LCD_IncrWrapPercCircleBold(uint16_t radius, uint16_t bold, uint8_t minPerc, uint8_t maxPerc, uint8_t stepPerc);
uint16_t LCD_GetNextIncrCircleWidth(uint32_t width);
uint16_t LCD_GetNextDecrCircleWidth(uint32_t width);
structPosition GetCircleMiddPoint(uint16_t *radius);
int LCD_GradCircButtSlidCorrectXY(SHAPE_PARAMS param, u16 bkWidth);

void LCD_Circle(uint32_t posBuff,uint32_t BkpSizeX,uint32_t BkpSizeY, uint32_t x, uint32_t y, uint32_t _width, uint32_t height, uint32_t FrameColor, uint32_t FillColor, uint32_t BkpColor);
void LCD_HalfCircle(uint32_t posBuff,uint32_t BkpSizeX,uint32_t BkpSizeY, uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t FrameColor, uint32_t FillColor, uint32_t BkpColor);
SHAPE_PARAMS LCDSHAPE_Create(uint32_t posBuff,uint32_t BkpSizeX,uint32_t BkpSizeY, uint32_t x, uint32_t y, uint32_t _width, uint32_t height, uint32_t FrameColor, uint32_t FillColor, uint32_t BkpColor,u32 selFillColorFrom,u32 selFillColor,u32 selFillColorTo,u16 degree,DIRECTIONS fillDir, u32 outColorRead);

/* ------- Selected Figures ------------------*/
void 					LCD_SignStar					(uint32_t posBuff,uint32_t BkpSizeX,uint32_t BkpSizeY, uint32_t x,uint32_t y, uint32_t width, uint32_t height, uint32_t FrameColor, uint32_t FillColor, uint32_t BkpColor);
void 					LCD_SimpleTriangle			(uint32_t posBuff,uint32_t BkpSizeX, uint32_t x,uint32_t y, uint32_t halfWidth,uint32_t height, uint32_t FrameColor, uint32_t FillColor, uint32_t BkpColor, DIRECTIONS direct);
structPosition 	LCD_ShapeExample				(uint32_t posBuff,uint32_t BkpSizeX, uint32_t x,uint32_t y, uint32_t lineLen, uint32_t FrameColor, uint32_t FillColor, uint32_t BkpColor, int angleInclination);
SHAPE_PARAMS 		LCD_Arrow						(uint32_t posBuff,uint32_t bkpSizeX,uint32_t bkpSizeY, uint32_t x,uint32_t y, uint32_t width,uint32_t height, uint32_t frameColor, uint32_t fillColor, uint32_t bkpColor, DIRECTIONS direct);
SHAPE_PARAMS 		LCD_Enter						(uint32_t posBuff,uint32_t bkpSizeX,uint32_t bkpSizeY, uint32_t x,uint32_t y, uint32_t width,uint32_t height, uint32_t frameColor, uint32_t fillColor, uint32_t bkpColor);
SHAPE_PARAMS 		LCD_Exit							(uint32_t posBuff,uint32_t bkpSizeX,uint32_t bkpSizeY, uint32_t x,uint32_t y, uint32_t width,uint32_t height, uint32_t frameColor, uint32_t fillColor, uint32_t bkpColor);
SHAPE_PARAMS 		LCD_KeyBackspace				(uint32_t posBuff,uint32_t BkpSizeX,uint32_t BkpSizeY, uint32_t x,uint32_t y, uint32_t width, uint32_t height, uint32_t FrameColor, uint32_t FillColor, uint32_t BkpColor);
SHAPE_PARAMS 		LCD_GradientCircleButton	(u32 posBuff,u32 BkpSizeX,u32 BkpSizeY,u32 x,u32 y,u32 width,u32 height,u32 FrameColor,u32 FillColorGradStart,u32 FillColorGradStop,u32 BkpColor,u32 outColorRead);
SHAPE_PARAMS 		LCD_GradientCircleSlider	(u32 posBuff,u32 BkpSizeX,u32 BkpSizeY,u32 x,u32 y,u32 width,u32 height,u32 FrameColorSlid,u32 FillColorSlid,u32 GradColorStartSlid,u32 GradColorSlid,u32 GradColorStopSlid,u32 FrameColorButt,u32 FillColorStartButt,u32 FillColorStopButt,u32 BkpColor,u16 degree,DIRECTIONS fillDirSlid,u32 outColorRead);
USER_GRAPH_PARAM 	LCD_Chart						(int posBuff, int offsMem,int nrMem, u32 widthBk, u32 colorLineAA, u32 colorOut, u32 colorIn, u32 bkRectColor, float outRatioStart, float inRatioStart, DISP_OPTION dispOption, u32 color1, u32 color2, int offsK1, int offsK2, GRADIENT_GRAPH_TYPE bkGradType,u32 gradColor1,u32 gradColor2,u8 gradStripY,float amplTrans,float offsTrans, int corr45degAA);

SHAPE_PARAMS LCD_SimpleSliderH(uint32_t posBuff, uint32_t BkpSizeX,uint32_t BkpSizeY, uint32_t x,uint32_t y, uint32_t widthParam, uint32_t heightParam, uint32_t ElementsColor, uint32_t LineColor, uint32_t LineSelColor, uint32_t BkpColor, uint32_t slidPos, int elemSel);
SHAPE_PARAMS LCD_SimpleSliderV(uint32_t posBuff, uint32_t BkpSizeX,uint32_t BkpSizeY, uint32_t x,uint32_t y, uint32_t widthParam, uint32_t heightParam, uint32_t ElementsColor, uint32_t LineColor, uint32_t LineSelColor, uint32_t BkpColor, uint32_t slidPos, int elemSel);
int ChangeElemSliderColor(SLIDER_PARAMS sel, uint32_t color);
uint32_t ChangeElemSliderSize(uint16_t width, uint8_t coeffHeightTriang, uint8_t coeffLineBold, uint8_t coeffHeightPtr, uint8_t coeffWidthPtr);
uint32_t SetSpaceTriangLineSlider(uint16_t height, uint16_t param);
uint32_t SetValType(uint16_t slidPos, uint16_t param);
uint32_t SetLineBold2Width(uint32_t width, uint8_t bold);
uint32_t SetTriangHeightCoeff2Height(uint32_t height, uint8_t coeff);

SHAPE_PARAMS 		LCDSHAPE_Window						(ShapeFunc pShape, uint32_t posBuff, SHAPE_PARAMS param);
SHAPE_PARAMS 		LCDSHAPE_Arrow							(uint32_t posBuff, SHAPE_PARAMS param);
SHAPE_PARAMS 		LCDSHAPE_Enter							(uint32_t posBuff, SHAPE_PARAMS param);
SHAPE_PARAMS 		LCDSHAPE_Exit							(uint32_t posBuff, SHAPE_PARAMS param);
SHAPE_PARAMS 		LCDSHAPE_KeyBackspace				(uint32_t posBuff, SHAPE_PARAMS param);
SHAPE_PARAMS 		LCDSHAPE_Rectangle					(uint32_t posBuff, SHAPE_PARAMS param);
SHAPE_PARAMS 		LCDSHAPE_RoundRectangle				(uint32_t posBuff, SHAPE_PARAMS param);
SHAPE_PARAMS 		LCDSHAPE_GradientCircleButton		(uint32_t posBuff, SHAPE_PARAMS param);
SHAPE_PARAMS 		LCDSHAPE_GradientCircleSlider		(uint32_t posBuff, SHAPE_PARAMS param);
USER_GRAPH_PARAM 	LCDSHAPE_Chart							(uint32_t posBuff, USER_GRAPH_PARAM param);

void 	LCD_Arrow_Indirect						(uint32_t x,uint32_t y, uint32_t width,uint32_t height, uint32_t frameColor, uint32_t fillColor, uint32_t bkpColor, DIRECTIONS direct);
void 	LCD_Rectangle_Indirect					(u32 x,u32 y, u32 width,u32 height, u32 FrameColorStart,u32 FrameColorStop,u32 FillColorStart,u32 FillColorStop,u32 BkpColor,float ratioStart,DIRECTIONS direct);
void 	LCD_RoundRectangle_Indirect			(int rectFrame,u32 x,u32 y, u32 width,u32 height, u32 FrameColorStart,u32 FrameColorStop,u32 FillColorStart,u32 FillColorStop,u32 BkpColor,float ratioStart,DIRECTIONS direct);
void 	LCD_GradientCircleButton_Indirect	(u32 x,u32 y,u32 width,u32 height,u32 FrameColor,u32 FillColorGradStart,u32 FillColorGradStop,u32 BkpColor,u32 outColorRead);
void 	LCD_GradientCircleSlider_Indirect	(u32 x,u32 y,u32 width,u32 height,u32 FrameColorSlid,u32 FillColorSlid,u32 GradColorStartSlid,u32 GradColorSlid,u32 GradColorStopSlid,u32 FrameColorButt,u32 FillColorStartButt,u32 FillColorStopButt,u32 BkpColor,u16 degree,DIRECTIONS fillDirSlid,u32 outColorRead);
void 	LCD_Chart_Indirect						(int offsMem, int nrMem, u32 widthBk, u32 colorLineAA, u32 colorOut, u32 colorIn, u32 bkRectColor, float outRatioStart, float inRatioStart, DISP_OPTION dispOption, u32 color1, u32 color2, int offsK1, int offsK2, GRADIENT_GRAPH_TYPE bkGradType,u32 gradColor1,u32 gradColor2,u8 gradStripY,float amplTrans,float offsTrans, int corr45degAA);

void 	LCDSHAPE_Arrow_Indirect			 		 		(SHAPE_PARAMS param);
void 	LCDSHAPE_Rectangle_Indirect		 		 	(SHAPE_PARAMS param);
void 	LCDSHAPE_RoundRectangle_Indirect		 		(SHAPE_PARAMS param);
void 	LCDSHAPE_GradientCircleButton_Indirect		(SHAPE_PARAMS param);
void 	LCDSHAPE_GradientCircleSlider_Indirect		(SHAPE_PARAMS param);
void 	LCDSHAPE_Chart_Indirect							(USER_GRAPH_PARAM param);
/* ------- End Selected Figures ------------------*/

int 	GRAPH_GetNmbrPoints		(int offsMem, int nrMem);
int	GRAPH_GetSamples		  	(				 int offsMem,int nrMem, 				 int startX,int startY, int yMin,int yMax, int nmbrPoints,float precision, float scaleX,float scaleY, int funcPatternType);
void	GRAPH_Draw				  	(int posBuff,int offsMem,int nrMem, u32 widthBk,																																						 			u32 colorLineAA, u32 colorOut, u32 colorIn, float outRatioStart, float inRatioStart, DISP_OPTION dispOption, u32 color1, u32 color2, int offsK1, int offsK2, GRADIENT_GRAPH_TYPE bkGradType,u32 gradColor1,u32 gradColor2,u8 gradStripY,float amplTrans,float offsTrans, int corr45degAA);
void	GRAPH_GetSamplesAndDraw (int posBuff,int offsMem,int nrMem, u32 widthBk, int startX,int startY, int yMin,int yMax, int nmbrPoints,float precision, float scaleX,float scaleY, int funcPatternType, u32 colorLineAA, u32 colorOut, u32 colorIn, float outRatioStart, float inRatioStart, DISP_OPTION dispOption, u32 color1, u32 color2, int offsK1, int offsK2, GRADIENT_GRAPH_TYPE bkGradType,u32 gradColor1,u32 gradColor2,u8 gradStripY,float amplTrans,float offsTrans, int corr45degAA);



#endif /* LCD_LCD_BASICGAPHICS_H_ */
