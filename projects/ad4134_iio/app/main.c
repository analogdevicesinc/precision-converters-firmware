/***************************************************************************//**
 *   @file    main.c
 *   @brief   Main interface for IIO firmware application
********************************************************************************
 * Copyright (c) 2020-21, 2023, 2025 Analog Devices, Inc.
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

/******************************************************************************/
/********************** Macros and Constants Definitions **********************/
/******************************************************************************/

/******************************************************************************/
/********************** Variables and User Defined Data Types *****************/
/******************************************************************************/

/******************************************************************************/
/************************** Functions Declarations ****************************/
/******************************************************************************/
int32_t init_system(void);
int32_t iio_app_initialize(void);
void iio_app_event_handler(void);

/******************************************************************************/
/************************** Functions Definitions *****************************/
/******************************************************************************/
/**
 * @brief	Main entry point to application
 * @return	none
 */
int main(void)
{
	/* Initialize the system and peripherals */
	if (init_system()) {
		printf("System initialization failure!!\r\n");
		return -ENODEV;
	}

	/* Initialize the IIO interface */
	if (iio_app_initialize()) {
		printf("IIO initialization failure!!\r\n");
		return -ENODEV;
	}

	while (1) {
		/* Monitor the IIO client events */
		iio_app_event_handler();
	}
}
