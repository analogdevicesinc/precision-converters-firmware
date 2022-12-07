/***************************************************************************//**
 * @file    main.c
 * @brief   Main application interface module
********************************************************************************
* Copyright (c) 2022 Analog Devices, Inc.
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

#include "24xx32a_eeprom_iio.h"

/******************************************************************************/
/********************* Macros and Constants Definition ************************/
/******************************************************************************/

/******************************************************************************/
/******************** Variables and User Defined Data Types *******************/
/******************************************************************************/

/******************************************************************************/
/************************** Functions Declarations ****************************/
/******************************************************************************/

/******************************************************************************/
/************************** Functions Definitions *****************************/
/******************************************************************************/

/* @brief	Main function
 * @details	This is a main entry function for application
 */
int main(void)
{
	/* Initialize the IIO device interface */
	if (evb_discovery_iio_init()) {
		printf("IIO initialization failure!!\r\n");
	}

	while (1) {
		/* Monitor the IIO client events */
		evb_discovery_iio_event_handler();
	}
}
