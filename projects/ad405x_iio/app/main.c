/***************************************************************************//**
 * @file    main.c
 * @brief   Main interface for AD405X IIO firmware application
********************************************************************************
* Copyright (c) 2023 Analog Devices, Inc.
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

#include "no_os_error.h"
#include "ad405x_iio.h"

/******************************************************************************/
/********************* Macros and Constants Definition ************************/
/******************************************************************************/

/******************************************************************************/
/******************** Variables and User Defined Data Types *******************/
/******************************************************************************/

/******************************************************************************/
/************************** Functions Declaration *****************************/
/******************************************************************************/
int32_t init_system(void);

/******************************************************************************/
/************************** Functions Definition ******************************/
/******************************************************************************/
/**
 * @brief	Main entry point to application
 * @return	none
 */
int main(void)
{
	if (init_system()) {
		printf("System Initialization failure!!\r\n");
		return -ENODEV;
	}
	/* Initialize the AD405X IIO interface */
	if (iio_ad405x_initialize()) {
		printf("IIO initialization failure!!\r\n");
		return -ENODEV;
	}

	while (1) {
		/* Monitor the IIO client events */
		iio_ad405x_event_handler();
	}
}