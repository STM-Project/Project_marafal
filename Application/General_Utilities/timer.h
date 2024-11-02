/*
 * timer.h
 *
 *  Created on: 03.04.2021
 *      Author: Elektronika RM
 */

#ifndef GENERAL_UTILITIES_TIMER_H_
#define GENERAL_UTILITIES_TIMER_H_

#include "stm32f7xx_hal.h"
#include "freeRTOS.h"

typedef enum{
	TimerId_1,
	TimerId_2,
	TimerId_3,
	TimerId_4,
	TimerId_5,
	TimerId_6,
	TimerId_7,
	TimerId_8,
	TimerId_9,
}TIMER_ID;

typedef enum{
	start_time,
	stop_time,
	get_time,
	get_startTime,
	check_time,
	reset_time,
	restart_time,
	check_restart_time,
	check_stop_time,
	noUse
}TIMER_SERVICE;

void AllTimeReset(void);
void TimeReset(int nrTime);
int IsTimeout(int nrTime, int timeout);
void StartMeasureTime(int nr);
uint32_t StopMeasureTime(int nr, char *nameTime);

int _CheckTickCount(portTickType tim, int timeout);
uint16_t vTimerService(int nr, int cmd, int timeout);

#endif /* GENERAL_UTILITIES_TIMER_H_ */
