/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    hardware_rng.c
  * @author  MCD Application Team
  * @version V1.2.1
  * @date    14-April-2017
  * @brief   mbedtls alternate entropy data function.
  *          the mbedtls_hardware_poll() is customized to use the STM32 RNG
  *          to generate random data, required for TLS encryption algorithms.
  *
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

#include "mbedtls_config.h"

#ifdef MBEDTLS_ENTROPY_HARDWARE_ALT

#include "main.h"
#include "string.h"
#include "stm32f7xx_hal.h"
#include "mbedtls/entropy_poll.h"
#include "mbedtls/entropy.h"

extern RNG_HandleTypeDef hrng;

int mbedtls_hardware_poll( void *Data, unsigned char *Output, size_t Len, size_t *oLen )
{
  uint32_t randomValue;
  (void)Data;
  *oLen = 0;
  while (*oLen < Len)
  {
    if (HAL_RNG_GenerateRandomNumber(&hrng, &randomValue) == HAL_OK)
    {
      size_t count = Len - *oLen;
      if (count > sizeof(randomValue)) count = sizeof(randomValue);
      memcpy(Output + *oLen, &randomValue, count);
      *oLen += count;
    }
    else
    {
      return MBEDTLS_ERR_ENTROPY_SOURCE_FAILED;
    }
  }

  return 0;
}

#endif /*MBEDTLS_ENTROPY_HARDWARE_ALT*/
