/*!
 *****************************************************************************
  @file:  main.c

  @brief: main module for AD7124 application interface

  @details: main module for AD7124 application interface

 -----------------------------------------------------------------------------
 Copyright (c) 2019, 2020 Analog Devices, Inc.
 All rights reserved.

 This software is proprietary to Analog Devices, Inc. and its licensors.
 By using this software you agree to the terms of the associated
 Analog Devices Software License Agreement.
/*****************************************************************************/

/*** includes ***/
#include <stdio.h>
#include "ad7124_console_app.h"

/*****************************************************************************/

/*** Function Prototypes ***/

/*****************************************************************************/

/* @brief  Main function
 *
 * @param  None
 *
 * @return SUCCESS(0), FAILURE (Negative)
 */
int main()
{
	int32_t setupResult;

	/* Initialize the AD7124 application before the main loop */
	if ((setupResult = ad7124_app_initialize(AD7124_CONFIG_A)) < 0) {
		printf("Error setting up AD7124 (%ld)\r\n\r\n", setupResult);
	}

	/* Infinite loop */
	while (1) {
		// display the console menu for the AD7124 application
		adi_do_console_menu(&ad7124_main_menu);
	}

	// this line should not be reached
	return - 1;
}
