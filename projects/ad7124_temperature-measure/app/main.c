/***************************************************************************//**
 * @file     main.c
 * @brief    Main interface for AD7124 temperature measurement firmware example
 * @details
********************************************************************************
* Copyright (c) 2021-22, 2025 Analog Devices, Inc.
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
#include "app_config.h"
#include "ad7124_console_app.h"
#include "no_os_uart.h"
#include "ad7124_user_config.h"
/******************************************************************************/
/************************** Functions Definitions *****************************/
/******************************************************************************/

/* @brief	Main function
 * @details	This is a main entry function for firmware application
 */
int main(void)
{
	int32_t result;

	/* Initialize the stm32 peripherals */
#if (ACTIVE_PLATFORM == STM32_PLATFORM)
	stm32_system_init();
#endif

#if(ACTIVE_PLATFORM == STM32_PLATFORM)
	int ret;
	ret = no_os_uart_init(&uart_desc, &uart_init_params);
	if (ret) {
		return ret;
	}

	/* Set up the UART for standard I/O operations */
	no_os_uart_stdio(uart_desc);
#endif

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
