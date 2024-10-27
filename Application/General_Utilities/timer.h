/*
 * timer.h
 *
 *  Created on: 03.04.2021
 *      Author: Elektronika RM
 */

#ifndef GENERAL_UTILITIES_TIMER_H_
#define GENERAL_UTILITIES_TIMER_H_

#include "stm32f7xx_hal.h"

void AllTimeReset(void);
void TimeReset(int nrTime);
int IsTimeout(int nrTime, int timeout);
void StartMeasureTime(int nr);
uint32_t StopMeasureTime(int nr, char *nameTime);

#endif /* GENERAL_UTILITIES_TIMER_H_ */
