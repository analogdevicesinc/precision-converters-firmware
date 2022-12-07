/*!
 *****************************************************************************
  @file:  main.c

  @brief: main module for nanodac application interface

  @details: main module for nanodac application interface

 -----------------------------------------------------------------------------
 Copyright (c) 2020, 2022 Analog Devices, Inc.
 All rights reserved.

 This software is proprietary to Analog Devices, Inc. and its licensors.
 By using this software you agree to the terms of the associated
 Analog Devices Software License Agreement.
******************************************************************************/

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include <stdio.h>
#include "nanodac_console_app.h"

/******************************************************************************/
/************************ Functions Definitions *******************************/
/******************************************************************************/

/* @brief  Main function
 *
 * @param  None
 *
 * @return SUCCESS(0), FAILURE (Negative)
 */
int main()
{
	int32_t setupResult;

	/* Initialize the nanodac application */
	if ((setupResult = nanodac_app_initialize()) < 0) {
		printf("Error setting up nanodac (%ld)\r\n\r\n", setupResult);
	}

	/* Infinite loop */
	while (1) {
		// display the console menu for the nanodac application
		adi_do_console_menu(&nanodac_main_menu);
	}

	// this line should not be reached
	return - 1;
}
