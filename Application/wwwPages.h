/*
 * wwwPages.h
 *
 *  Created on: 23.12.2020
 *      Author: RafalMar
 */

#ifndef HTTP_WWWPAGES_H_
#define HTTP_WWWPAGES_H_

#include "stm32f7xx_hal.h"
#include "common.h"

#define HTTP_BUFFER_SIZE 		300000
#define HTTP_BUFFER_SIZE_PRE 		300000

DATA_TO_SEND* GetPageWWW(char *getHttpRequest);
void ResetTestTab(void);

#endif /* HTTP_WWWPAGES_H_ */
