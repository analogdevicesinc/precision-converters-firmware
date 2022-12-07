/*!
 *****************************************************************************
 * @file: main.c
 * @brief:
 *-----------------------------------------------------------------------------
 *
Copyright (c) 2020-2022 Analog Devices, Inc. All Rights Reserved.

This software is proprietary to Analog Devices, Inc. and its licensors.
By using this software you agree to the terms of the associated
Analog Devices Software License Agreement.
 ******************************************************************************/

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include <stdio.h>
#include <ctype.h>

#include "adi_console_menu.h"
#include "ad5770r_console_app.h"

/******************************************************************************/
/************************ Functions Definitions *******************************/
/******************************************************************************/

int main (void)
{
	int32_t setupResult;

	/* Initialize the AD7124 application before the main loop */
	if ((setupResult = ad5770r_app_initialize()) != 0) {
		printf("Error setting up AD5770R (%d)" EOL EOL, setupResult);
	}

	while(1) {
		adi_do_console_menu(&ad5770r_main_menu);
	}

	// this line should never be reached
	return (-1);
}
