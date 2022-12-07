/***************************************************************************//**
 * @file     main.c
 * @brief    Main interface for AD7124 temperature measurement firmware example
 * @details
********************************************************************************
* Copyright (c) 2021-22 Analog Devices, Inc.
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
#include <stdio.h>

#include "ad7124_console_app.h"

/******************************************************************************/
/************************** Functions Definitions *****************************/
/******************************************************************************/

/* @brief	Main function
 * @details	This is a main entry function for firmware application
 */
int main(void)
{
	int32_t result;

	/* Initialize the AD7124 device and application */
	if ((result = ad7124_app_initialize(AD7124_CONFIG_RESET)) != 0) {
		printf("Error setting up AD7124 (%ld)" EOL EOL, result);
	}

	/* Infinite loop */
	while (1) {
		/* display the console menu for the AD7124 application */
		adi_do_console_menu(&ad7124_main_menu);
	}

	/* this line should never be reached */
	return (-1);
}
