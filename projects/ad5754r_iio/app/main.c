/***************************************************************************//**
 *   @file    main.c
 *   @brief   Main interface for AD5754R IIO firmware application
********************************************************************************
 * Copyright (c) 2023 Analog Devices, Inc.
 *
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 * By using this software you agree to the terms of the associated
 * Analog Devices Software License Agreement.
*******************************************************************************/

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/
#include <stdio.h>
#include <stdint.h>
#include "ad5754r_iio.h"

/******************************************************************************/
/********************** Macros and Constants Definitions **********************/
/******************************************************************************/

/******************************************************************************/
/************************ Functions Definitions *******************************/
/******************************************************************************/

/**
 * @brief	Main entry point to application
 * @return	none
 */
int main(void)
{
	/* Initialize the AD5754R IIO interface */
	if (ad5754r_iio_init()) {
		printf("IIO initialization failure!!\r\n");
	}

	while (1) {
		/* Monitor the IIO client events */
		ad5754r_iio_event_handler();
	}
}