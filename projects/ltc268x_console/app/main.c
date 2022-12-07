/***************************************************************************//**
 *   @file   main.c
 *   @brief  main module for ltc268x console application interface
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
#include <assert.h>
#include "ltc268x_console_app.h"
#include "no_os_error.h"

/******************************************************************************/
/********************** Macros and Constants Definitions **********************/
/******************************************************************************/

/******************************************************************************/
/************************ Functions Definitions *******************************/
/******************************************************************************/
/* @brief	Main function
 * @details	This is a main entry function for firmware application
 */
int main(void)
{
	int32_t init_status;

	/* Initialize the LTC268X device */
	init_status = ltc268x_app_initialize();
	if (init_status) {
		printf(EOL "\tError setting up the device (%ld)" EOL, init_status);
		assert(false);
	}

	while (1) {
		/* Display the console menu for the LTC268X application */
		adi_do_console_menu(&ltc268x_main_menu);
	}
}