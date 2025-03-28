/*
 * examples.h
 *
 *  Created on: Feb 7, 2025
 *      Author: maraf
 */

#ifndef LCD_SCREENS_EXAMPLES_H_
#define LCD_SCREENS_EXAMPLES_H_

void LCDEXAMPLE_RectangleGradient(u32 COLOR_FillFrame, u32 COLOR_Frame, u32 COLOR_BkScreen, int FONT_ID_Descr);
void LCDEXAMPLE_GradientCircleButtonAndSlider(int FONT_ID_Title, int FONT_VAR_Title, u32 COLOR_FillMainFrame, u32 COLOR_Frame, u32 COLOR_BkScreen);
void LCDEXAMPLE_LcdTxt(int FONT_ID_Fonts, int FONT_VAR_Fonts, u32 COLOR_FillMainFrame, u32 COLOR_Frame, u32 COLOR_BkScreen);
void LCDEXAMPLE_DrawLine(u32 COLOR_Frame, u32 COLOR_BkScreen);

void MEMEXAMPLE_DrawLine(void);

#endif /* LCD_SCREENS_EXAMPLES_H_ */
