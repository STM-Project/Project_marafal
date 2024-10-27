/*
 * TouchTask.c
 *
 *  Created on: Apr 10, 2024
 *      Author: maraf
 */
#include "TouchLcdTask.h"
#include "FreeRTOS.h"
#include "task.h"
#include <string.h>
#include <stdio.h>
#include "touch.h"

xTaskHandle vtask_TouchLcd_Handle;

void vtask_TouchLcd(void *pvParameters)
{
	portTickType xLastExecutionTime;

	xLastExecutionTime = xTaskGetTickCount();
	vTaskDelayUntil(&xLastExecutionTime, 100);

	while(1)
	{
		LCD_TOUCH_Service();
		vTaskDelayUntil(&xLastExecutionTime, SERVICE_TOUCH_PROB_TIME_MS);
	}
}

void Create_TouchLcd_Task(void)
{
	xTaskCreate(vtask_TouchLcd, (char* )"vtask_TouchLcd", 1024, NULL, (unsigned portBASE_TYPE ) 1, &vtask_TouchLcd_Handle);
}

void Delete_TouchLcd_Task(void)
{
	vTaskDelete(vtask_TouchLcd_Handle);
	/* vTaskSuspend(vtask_TouchLcd_Handle); */
	/* xTaskResumeFromISR(vtask_TouchLcd_Handle); */
}
