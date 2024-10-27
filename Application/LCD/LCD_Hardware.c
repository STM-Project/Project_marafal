/*
 * LCD_Hardware.c
 *
 *  Created on: 10.04.2021
 *      Author: Elektronika RM
 */

#include "stm32f7xx_hal.h"
#include "LCD_Hardware.h"
#include "ltdc.h"
#include "dma2d.h"

static uint32_t  ActiveLayer=0;

uint32_t LCD_GetXSize(void){
  return hltdc.LayerCfg[ActiveLayer].ImageWidth;
}
uint32_t LCD_GetYSize(void){
  return hltdc.LayerCfg[ActiveLayer].ImageHeight;
}
//static uint32_t LCD_GetAddress(uint32_t Xpos, uint32_t Ypos){
//  return hltdc.LayerCfg[ActiveLayer].FBStartAdress;
//}

static int LCD_GetBytesPerPixel(void)
{
	switch(hdma2d.Init.ColorMode)
	{
	case DMA2D_OUTPUT_RGB565:
		return 2;
	case DMA2D_OUTPUT_RGB888:
		return 3;
	default:
	case DMA2D_OUTPUT_ARGB8888:
		return 4;
	}
}

static uint32_t LCD_GetPositionAddress(uint32_t Xpos, uint32_t Ypos){
  return hltdc.LayerCfg[ActiveLayer].FBStartAdress + (((LCD_GetXSize()*Ypos) + Xpos)*LCD_GetBytesPerPixel());
}

static int LCD_SetOutputOffset(uint32_t width)
{
	hdma2d.Init.OutputOffset = LCD_GetXSize() - width;
	return HAL_DMA2D_Init(&hdma2d);
}

void LCD_DisplayBuff(uint32_t Xpos, uint32_t Ypos, uint32_t width, uint32_t height, uint32_t *pbmp)
{
	while(HAL_DMA2D_STATE_READY!=HAL_DMA2D_GetState(&hdma2d));
	LCD_SetOutputOffset(width);
	SCB_InvalidateDCache_by_Addr(pbmp, width*height*sizeof(uint32_t));
   HAL_DMA2D_Start_IT(&hdma2d, (uint32_t)pbmp, LCD_GetPositionAddress(Xpos,Ypos), width, height);
   while(HAL_DMA2D_STATE_READY!=HAL_DMA2D_GetState(&hdma2d));
}
