/***************************************************************************//**
 *   @file    main.c
 *   @brief   Main interface for AD4080 IIO firmware application
********************************************************************************
 * Copyright (c) 2023-25 Analog Devices, Inc.
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
#include "ad4080_iio.h"
#include "app_config.h"

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
	/* Initialize the system and peripherals */
	if (init_system()) {
		printf("System initialization failure!!\r\n");
		return -ENODEV;
	}

	/* Initialize the AD4080 IIO interface */
	if (ad4080_iio_initialize()) {
		printf("IIO initialization failure!!\r\n");
		return -ENODEV;
	}

	while (1) {
		/* Monitor the IIO client events */
		ad4080_iio_event_handler();
	}
}
