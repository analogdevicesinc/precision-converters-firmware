/***************************************************************************//**
 * @file    main.c
 * @brief   Main interface for AD5710R IIO firmware application
 * @details
********************************************************************************
* Copyright (c) 2024-25 Analog Devices, Inc.
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

#include "ad5710r_iio.h"

/******************************************************************************/
/********************* Macros and Constants Definition ************************/
/******************************************************************************/

/******************************************************************************/
/******************** Variables and User Defined Data Types *******************/
/******************************************************************************/

/******************************************************************************/
/************************** Functions Declaration *****************************/
/******************************************************************************/

/******************************************************************************/
/************************** Functions Definition ******************************/
/******************************************************************************/
/**
 * @brief	Main entry point to application
 * @return	none
 */
int main(void)
{
	/* Initialize the system and peripherals */
	if (init_system()) {
		printf("System Initialization failure!!\r\n");
		return -ENODEV;
	}

	/* Initialize the AD5710R IIO interface */
	if (ad5710r_iio_initialize()) {
		printf("IIO initialization failure!!\r\n");
		return -ENODEV;
	}

	while (1) {
		/* Monitor the IIO client events */
		ad5710r_iio_event_handler();
	}
}

