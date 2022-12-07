/***************************************************************************//**
 *   @file   main.c
 *   @brief  main module for ad590 console application interface
********************************************************************************
 * Copyright (c) 2021-22 Analog Devices, Inc.
 *
 * All rights reserved.
 *
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 * By using this software you agree to the terms of the associated
 * Analog Devices Software License Agreement.
*******************************************************************************/

#include <stdio.h>
#include "no_os_error.h"
#include "ad590_console_app.h"

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
	int32_t init_status;
	/* Initialize the ad590 device */
	init_status = ltc2488_app_initialize();
	if (init_status) {
		printf(EOL "\tError setting up ad590 (%ld)" EOL EOL, init_status);
	}
	
	while (1) {
		if (init_status == 0 ) {
			/* display the console menu for the ad590 application */
			adi_do_console_menu(&ad590_main_menu);
		}
	}
}