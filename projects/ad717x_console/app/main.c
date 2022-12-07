/*!
 *****************************************************************************
  @file:  main.c

  @brief: main module for AD717x/AD411x application interface

  @details: This module initialize the device and display the console menus

 -----------------------------------------------------------------------------
 Copyright (c) 2020,2022 Analog Devices, Inc.
 All rights reserved.

 This software is proprietary to Analog Devices, Inc. and its licensors.
 By using this software you agree to the terms of the associated
 Analog Devices Software License Agreement.
******************************************************************************/

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include <stdio.h>
#include "ad717x_console_app.h"

/******************************************************************************/
/************************ Functions Definitions *******************************/
/******************************************************************************/

/* @brief  Main function
 * @param  None
 * @return 0 in case of success, negative error code otherwise
 */
int main()
{
	int32_t setupResult;

	/* Initialize the AD717x/AD411x application */
	if ((setupResult = ad717x_app_initialize()) < 0) {
		printf("Error setting up AD717x (%ld)\r\n\r\n", setupResult);
	}

	/* Infinite loop */
	while (1) {
		// display the console menu for the AD717x/AD411x application
		adi_do_console_menu(&ad717x_main_menu);
	}

	// this line should not be reached
	return - 1;
}
