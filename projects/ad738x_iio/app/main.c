/***************************************************************************//**
 *   @file    main.c
 *   @brief   Main module for AD738x IIO application
********************************************************************************
 * Copyright (c) 2022 Analog Devices, Inc.
 *
 * All rights reserved.
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

#include "ad738x_iio.h"

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
	int32_t ret;

	/* Initialize the AD738x IIO interface */
	ret = ad738x_iio_initialize();
	if (ret) {
		printf("IIO initialization failure!!\r\n");
	}

	while (1) {
		/* Monitor the IIO client events */
		ad738x_iio_event_handler();
	}
}
