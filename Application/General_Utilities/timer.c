/*
 * timer.c
 *
 *  Created on: 03.04.2021
 *      Author: Elektronika RM
 */

#include "timer.h"
#include "FreeRTOS.h"
#include "timers.h"
#include "debug.h"

#define MAX_COUNT_TIME		40
#define MAX_MEASURE_TIME	10

static portTickType timeVar[MAX_COUNT_TIME];
static portTickType measurTime[MAX_MEASURE_TIME];

void AllTimeReset(void)
{
	for(int i=0; i<MAX_COUNT_TIME; ++i)
		timeVar[i] = xTaskGetTickCount();
}
void TimeReset(int nrTime)
{
	if(nrTime<MAX_COUNT_TIME)
		timeVar[nrTime] = xTaskGetTickCount();
}
int IsTimeout(int nrTime, int timeout)
{
	if(nrTime<MAX_COUNT_TIME)
	{
		if((xTaskGetTickCount()-timeVar[nrTime])>timeout)
			return 1;
	}
	return 0;
}

void StartMeasureTime(int nr)
{
	if(nr<MAX_MEASURE_TIME)
		measurTime[nr] = xTaskGetTickCount();
}
uint32_t StopMeasureTime(int nr, char *nameTime)
{
	if(nr<MAX_MEASURE_TIME)
	{
		int measurTimeStop = xTaskGetTickCount();
		if(nameTime[0]>0)
			DbgVar(1,20,"%s%d ",nameTime,measurTimeStop-measurTime[nr]);
		return measurTimeStop-measurTime[nr];
	}
	return 0;
}
