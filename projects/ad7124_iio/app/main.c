/**************************************************************************//**
 *   @file    main.c
 *   @brief   Main module for AD7124 IIO application
 *   @details This module invokes the AD7124 IIO interfaces
 *            through forever loop.
*******************************************************************************
 * Copyright (c) 2023 Analog Devices, Inc.
 * All rights reserved.
 *
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 * By using this software you agree to the terms of the associated
 * Analog Devices Software License Agreement.
******************************************************************************/

/*****************************************************************************/
/***************************** Include Files *********************************/
/*****************************************************************************/

#include <stdio.h>

#include "ad7124_iio.h"

/*****************************************************************************/
/********************** Macros and Constants Definitions *********************/
/*****************************************************************************/

/*****************************************************************************/
/************************ Functions Definitions ******************************/
/*****************************************************************************/

/**
 * @brief Main entry point to application
 * @return none
 */
int main(void)
{
	int ret;

	/* Initialize the IIO interface */
	ret = ad7124_iio_initialize();
	if (ret) {
		printf("IIO initialization failure!!\r\n");
	}

	while (1) {
		/* Monitor the IIO client events */
		ad7124_iio_event_handler();
	}
}
