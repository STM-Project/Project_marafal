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

#define BK_COLOR_CIRCLESLIDER(p)					GetTransitionColor(p.param[0], p.param[1],0.5)
#define BK_COLOR_CIRCLEBUTTON(p)					GetTransitionColor(p.color[0].fill, p.color[1].fill,0.5)
#define POS_SIZE_CIRCLEBUTTONSLIDER(p,offs)	p.pos[0].x, p.pos[0].y, p.size[0].w+offs, p.size[0].h+offs

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
	NoDirect = -1,
	Up,
	Down,
	Down2,
	Right,
	Right2,
	RightDown,
	Left,
	Left2,
	LeftUp,
	outside,
	inside,
	inside2,
	DownUp,
	LeftLeft,
	RightRight,
	Horizontal,
	Vertical,
	Middle,
	Middle2,
	Midd_X,
	Midd_Y,
	Midd_X2,
	Midd_Y2,
	AllEdge,
	AllEdge2,
	Shade,
	Round,
	Center,
}DIRECTIONS;

#define NORMAL_SLIDER_PARAM	2,8,2,2
#define NORMAL_SLIDER_PARAM2	2,6,2,1
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
	ReadOutColor = 0x1,
}SHAPE_FLAGS;

#define K_TAB_SIZE 	4
typedef struct{
	u32 k[K_TAB_SIZE];
}structK;

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

void LCD_Circle(uint32_t posBuff,uint32_t BkpSizeX,uint32_t BkpSizeY, uint32_t x, uint32_t y, uint32_t _width, uint32_t height, uint32_t FrameColor, uint32_t FillColor, uint32_t BkpColor);
void LCD_HalfCircle(uint32_t posBuff,uint32_t BkpSizeX,uint32_t BkpSizeY, uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t FrameColor, uint32_t FillColor, uint32_t BkpColor);
SHAPE_PARAMS LCDSHAPE_Create(uint32_t posBuff,uint32_t BkpSizeX,uint32_t BkpSizeY, uint32_t x, uint32_t y, uint32_t _width, uint32_t height, uint32_t FrameColor, uint32_t FillColor, uint32_t BkpColor,u32 selFillColorFrom,u32 selFillColor,u32 selFillColorTo,u16 degree,DIRECTIONS fillDir, u32 outColorRead);

/* ------- Selected Figures ------------------*/
void LCD_SignStar(uint32_t posBuff,uint32_t BkpSizeX,uint32_t BkpSizeY, uint32_t x,uint32_t y, uint32_t width, uint32_t height, uint32_t FrameColor, uint32_t FillColor, uint32_t BkpColor);
void LCD_SimpleTriangle(uint32_t posBuff,uint32_t BkpSizeX, uint32_t x,uint32_t y, uint32_t halfWidth,uint32_t height, uint32_t FrameColor, uint32_t FillColor, uint32_t BkpColor, DIRECTIONS direct);
structPosition LCD_ShapeExample(uint32_t posBuff,uint32_t BkpSizeX, uint32_t x,uint32_t y, uint32_t lineLen, uint32_t FrameColor, uint32_t FillColor, uint32_t BkpColor, int angleInclination);
SHAPE_PARAMS LCD_Arrow(uint32_t posBuff,uint32_t bkpSizeX,uint32_t bkpSizeY, uint32_t x,uint32_t y, uint32_t width,uint32_t height, uint32_t frameColor, uint32_t fillColor, uint32_t bkpColor, DIRECTIONS direct);
SHAPE_PARAMS LCD_Enter(uint32_t posBuff,uint32_t bkpSizeX,uint32_t bkpSizeY, uint32_t x,uint32_t y, uint32_t width,uint32_t height, uint32_t frameColor, uint32_t fillColor, uint32_t bkpColor);
SHAPE_PARAMS LCD_Exit(uint32_t posBuff,uint32_t bkpSizeX,uint32_t bkpSizeY, uint32_t x,uint32_t y, uint32_t width,uint32_t height, uint32_t frameColor, uint32_t fillColor, uint32_t bkpColor);
SHAPE_PARAMS LCD_KeyBackspace(uint32_t posBuff,uint32_t BkpSizeX,uint32_t BkpSizeY, uint32_t x,uint32_t y, uint32_t width, uint32_t height, uint32_t FrameColor, uint32_t FillColor, uint32_t BkpColor);
SHAPE_PARAMS LCD_GradientCircleButton(u32 posBuff,u32 BkpSizeX,u32 BkpSizeY,u32 x,u32 y,u32 width,u32 height,u32 FrameColor,u32 FillColorGradStart,u32 FillColorGradStop,u32 BkpColor,u32 outColorRead);
SHAPE_PARAMS LCD_GradientCircleSlider(u32 posBuff,u32 BkpSizeX,u32 BkpSizeY,u32 x,u32 y,u32 width,u32 height,u32 FrameColorSlid,u32 FillColorSlid,u32 GradColorStartSlid,u32 GradColorSlid,u32 GradColorStopSlid,u32 FrameColorButt,u32 FillColorStartButt,u32 FillColorStopButt,u32 BkpColor,u16 degree,DIRECTIONS fillDirSlid,u32 outColorRead);

SHAPE_PARAMS LCD_SimpleSliderH(uint32_t posBuff, uint32_t BkpSizeX,uint32_t BkpSizeY, uint32_t x,uint32_t y, uint32_t widthParam, uint32_t heightParam, uint32_t ElementsColor, uint32_t LineColor, uint32_t LineSelColor, uint32_t BkpColor, uint32_t slidPos, int elemSel);
SHAPE_PARAMS LCD_SimpleSliderV(uint32_t posBuff, uint32_t BkpSizeX,uint32_t BkpSizeY, uint32_t x,uint32_t y, uint32_t widthParam, uint32_t heightParam, uint32_t ElementsColor, uint32_t LineColor, uint32_t LineSelColor, uint32_t BkpColor, uint32_t slidPos, int elemSel);
int ChangeElemSliderColor(SLIDER_PARAMS sel, uint32_t color);
uint32_t ChangeElemSliderSize(uint16_t width, uint8_t coeffHeightTriang, uint8_t coeffLineBold, uint8_t coeffHeightPtr, uint8_t coeffWidthPtr);
uint32_t SetSpaceTriangLineSlider(uint16_t height, uint16_t param);
uint32_t SetValType(uint16_t slidPos, uint16_t param);
uint32_t SetLineBold2Width(uint32_t width, uint8_t bold);
uint32_t SetTriangHeightCoeff2Height(uint32_t height, uint8_t coeff);

SHAPE_PARAMS LCDSHAPE_Window					(ShapeFunc pShape, uint32_t posBuff, SHAPE_PARAMS param);
SHAPE_PARAMS LCDSHAPE_Arrow					(uint32_t posBuff, SHAPE_PARAMS param);
SHAPE_PARAMS LCDSHAPE_Enter					(uint32_t posBuff, SHAPE_PARAMS param);
SHAPE_PARAMS LCDSHAPE_Exit						(uint32_t posBuff, SHAPE_PARAMS param);
SHAPE_PARAMS LCDSHAPE_KeyBackspace			(uint32_t posBuff, SHAPE_PARAMS param);
SHAPE_PARAMS LCDSHAPE_Rectangle				(uint32_t posBuff, SHAPE_PARAMS param);
SHAPE_PARAMS LCDSHAPE_RoundRectangle		(uint32_t posBuff, SHAPE_PARAMS param);
SHAPE_PARAMS LCDSHAPE_GradientCircleButton(uint32_t posBuff, SHAPE_PARAMS param);
SHAPE_PARAMS LCDSHAPE_GradientCircleSlider(uint32_t posBuff, SHAPE_PARAMS param);

void LCD_Arrow_Indirect						(uint32_t x,uint32_t y, uint32_t width,uint32_t height, uint32_t frameColor, uint32_t fillColor, uint32_t bkpColor, DIRECTIONS direct);
void LCD_Rectangle_Indirect				(u32 x,u32 y, u32 width,u32 height, u32 FrameColorStart,u32 FrameColorStop,u32 FillColorStart,u32 FillColorStop,u32 BkpColor,float ratioStart,DIRECTIONS direct);
void LCD_RoundRectangle_Indirect			(int rectFrame,u32 x,u32 y, u32 width,u32 height, u32 FrameColorStart,u32 FrameColorStop,u32 FillColorStart,u32 FillColorStop,u32 BkpColor,float ratioStart,DIRECTIONS direct);
void LCD_GradientCircleButton_Indirect	(u32 x,u32 y,u32 width,u32 height,u32 FrameColor,u32 FillColorGradStart,u32 FillColorGradStop,u32 BkpColor,u32 outColorRead);
void LCD_GradientCircleSlider_Indirect(u32 x,u32 y,u32 width,u32 height,u32 FrameColorSlid,u32 FillColorSlid,u32 GradColorStartSlid,u32 GradColorSlid,u32 GradColorStopSlid,u32 FrameColorButt,u32 FillColorStartButt,u32 FillColorStopButt,u32 BkpColor,u16 degree,DIRECTIONS fillDirSlid,u32 outColorRead);

void LCDSHAPE_Arrow_Indirect			 		 (SHAPE_PARAMS param);
void LCDSHAPE_Rectangle_Indirect		 		 (SHAPE_PARAMS param);
void LCDSHAPE_RoundRectangle_Indirect		 (SHAPE_PARAMS param);
void LCDSHAPE_GradientCircleButton_Indirect(SHAPE_PARAMS param);
void LCDSHAPE_GradientCircleSlider_Indirect(SHAPE_PARAMS param);
/* ------- End Selected Figures ------------------*/


#endif /* LCD_LCD_BASICGAPHICS_H_ */
