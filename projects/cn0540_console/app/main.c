/*******************************************************************************
 *   @file   main.c
 *   @brief  Main module for CN0540 console application
********************************************************************************
Copyright 2025(c) Analog Devices, Inc.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of Analog Devices, Inc. nor the names of its
   contributors may be used to endorse or promote products derived from this
   software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY ANALOG DEVICES, INC. “AS IS” AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
EVENT SHALL ANALOG DEVICES, INC. BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*******************************************************************************/

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include <stdio.h>
#include "cn0540_console_app.h"
#include "app_config.h"

/******************************************************************************/
/***************************** Function Definitions ***************************/
/******************************************************************************/

/* @brief  Main function
 * @return 0 in case of success, negative error code otherwise
 */
int main()
{
	int ret;

	/* Initialize the stm32 peripherals */
#if (ACTIVE_PLATFORM == STM32_PLATFORM)
	stm32_system_init();
#endif

	ret = cn0540_app_initalization();
	if (ret) {
		printf("\r\nError during set-up (%d)\r\n", ret);
		adi_press_any_key_to_continue();
	}

	/* Infinite loop */
	while (1) {
		// Display the console menu for the AD7124 application
		adi_do_console_menu(&cn0540_main_menu);
	}

	// This line should not be reached
	return -1;
}