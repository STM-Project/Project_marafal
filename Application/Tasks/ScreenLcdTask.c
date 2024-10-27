/*
 * ScreenLCDTask.c
 *
 *  Created on: Jun 2, 2024
 *      Author: mgrag
 */
#include "ScreenLcdTask.h"
#include "FreeRTOS.h"
#include "task.h"
#include "SCREEN_ReadPanel.h"

xTaskHandle vtask_ScreensSelectLCD_Handle;

void vtask_ScreensSelectLCD(void *pvParameters)
{
	while(1)
	{
		SCREEN_ReadPanel();
		vTaskDelay(20);
	}
}

void Create_ScreensSelectLCD_Task(void)
{
	xTaskCreate(vtask_ScreensSelectLCD, (char* )"vtask_ScreensSelectLCD", 2048, NULL, (unsigned portBASE_TYPE ) 1, &vtask_ScreensSelectLCD_Handle);
}
