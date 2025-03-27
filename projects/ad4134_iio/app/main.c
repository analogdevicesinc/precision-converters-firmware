/***************************************************************************//**
 *   @file    main.cpp
 *   @brief   main module for AD7134 IIO interface
 *   @details This module invokes the AD7134 IIO interfaces
 *            through forever loop.
********************************************************************************
 * Copyright (c) 2020-21, 2023 Analog Devices, Inc.
 * All rights reserved.
 *
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 * By using this software you agree to the terms of the associated
 * Analog Devices Software License Agreement.
*******************************************************************************/

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include <stdint.h>
#include "ad4134_iio.h"

/******************************************************************************/
/********************** Macros and Constants Definitions **********************/
/******************************************************************************/

/******************************************************************************/
/************************ Functions Definitions *******************************/
/******************************************************************************/

/* @brief	Main function
 * @details	This is a main entry function for AD7134 IIO application
 */
int main(void)
{
	/* Initialize the AD7134 IIO interface */
	if (ad7134_iio_initialize() != 0) {
		printf("\r\n IIO Initialization Failure!\r\n");
	}

	while (1) {
		/* Monitor the IIO client events */
		ad7134_iio_event_handler();
	}
}
