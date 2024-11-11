/*
 * SCREEN_ReadPanel.h
 *
 *  Created on: 14.04.2021
 *      Author: RafalMar
 */

#ifndef LCD_SCREENS_SCREEN_READPANEL_H_
#define LCD_SCREENS_SCREEN_READPANEL_H_

#include "LCD_fonts_images.h"
#include "LCD_BasicGaphics.h"

#define ONEBIT(val)	     Int2Str(val,None,1,Sign_none)
#define INT2STR(val)		  Int2Str(val,'0',3,Sign_none)

extern int argNmb;
extern char **argVal;
extern int startScreen;

void SCREEN_ReadPanel(void);

void LCD_ArrowTxt(uint32_t posBuff,uint32_t bkpSizeX,uint32_t bkpSizeY, uint32_t x,uint32_t y, uint32_t width,uint32_t height, uint32_t frameColor, uint32_t fillColor, uint32_t bkpColor, DIRECTIONS direct, int fontId, char *txt, uint32_t txtColor);

int SCREEN_ConfigTouchForStrVar(uint16_t ID_touch, uint16_t idx_touch, uint8_t param_touch, int idVar, int nrTouchIdx, StructTxtPxlLen lenStr);
int SCREEN_ConfigTouchForStrVar_2(uint16_t ID_touch, uint16_t idx_touch, uint8_t param_touch, int idVar, int nrTouchIdx, StructFieldPos field);
int SCREEN_SetTouchForNewEndPos(int idVar, int touchIdx, StructTxtPxlLen lenStr);

void Measure_Start();
void Measure_Stop();

#endif /* LCD_SCREENS_SCREEN_READPANEL_H_ */
