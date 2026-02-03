/*******************************************************************************
 *   @file   stm32_gpio_irq_generated.c
 *   @brief  GPIO IRQ specific functions for STM32 platform
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

#include "no_os_error.h"
#include "stm32_hal.h"

/******************************************************************************/
/********************* Macros and Constants Definition ************************/
/******************************************************************************/

/******************************************************************************/
/******************** Variables and User Defined Data Types *******************/
/******************************************************************************/

/******************************************************************************/
/************************** Functions Declaration *****************************/
/******************************************************************************/

/******************************************************************************/
/************************** Functions Definition ******************************/
/******************************************************************************/

/**
 * @brief Get the IRQ ID
 * @param pin_nb[in] - Pin number
 * @param irq_id[out] - Interrupt ID
 * @return 0 if successful, negative error code otherwise.
 */
int stm32_get_exti_irq_id_from_pin(uint8_t pin_nb, IRQn_Type *irq_id)
{
	/* Note: The irq_id number used here are specific to STM32F469NI MCU on the SDP-K1 board
	 * The below parameters will change depending on the controller used.
	 * */
	switch (pin_nb) {
	case 4:
		*irq_id = EXTI4_IRQn;
		break;
	case 0:
		*irq_id = EXTI0_IRQn;
		break;
	case 2:
		*irq_id = EXTI2_IRQn;
		break;
	case 1:
		*irq_id = EXTI1_IRQn;
		break;
	case 3:
		*irq_id = EXTI3_IRQn;
		break;
	case 5:
	case 6:
	case 7:
	case 8:
	case 9:
		*irq_id = EXTI9_5_IRQn;
		break;
	case 10:
	case 11:
	case 12:
	case 13:
	case 14:
	case 15:
		*irq_id = EXTI15_10_IRQn;
		break;
	default:
		return -ENOSYS;
	}
	return 0;
}
