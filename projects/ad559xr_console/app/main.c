/*!
 *****************************************************************************
  @file:  main.c
  @brief: main module for AD5592R application interface
  @details: main module for AD5592R application interface

 -----------------------------------------------------------------------------
 Copyright (c) 2020 Analog Devices, Inc.
 All rights reserved.

 This software is proprietary to Analog Devices, Inc. and its licensors.
 By using this software you agree to the terms of the associated
 Analog Devices Software License Agreement.
*****************************************************************************/

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/
#include <stdio.h>
#include "ad5592r_console_app.h"

/******************************************************************************/
/***************************** Function Definitions **********************************/
/******************************************************************************/

/* @brief  Main function
 *
 * @param  None
 *
 * @return SUCCESS(0), FAILURE (Negative)
 */
int main()
{
	int32_t status;

	if((status = ad5592r_app_initalization()) < 0) {
		printf(EOL "Error setting up device (%d)" EOL, status);
		adi_press_any_key_to_continue();
	}

	/* Infinite loop */
	while (1) {
		// display the console menu for the AD7124 application
		adi_do_console_menu(&ad5592r_main_menu);
	}

	// this line should not be reached
	return - 1;
}
