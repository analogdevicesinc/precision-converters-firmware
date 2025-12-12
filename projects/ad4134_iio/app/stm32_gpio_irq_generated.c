/***************************************************************************//**
 * @file    stm32_gpio_irq_generated.c
 * @brief   GPIO IRQ specific functions for STM32 platform
********************************************************************************
* Copyright (c) 2022-2023 Analog Devices, Inc.
* All rights reserved.
*
* This software is proprietary to Analog Devices, Inc. and its licensors.
* By using this software you agree to the terms of the associated
* Analog Devices Software License Agreement.
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
	/* Note: The irq_id number used here are specific to STM32H563ZIT6 MCU
	 * on the Nucleo-H563ZI Board.
	 * The below parameters will change depending on the controller used. */
#if defined(STM32H563xx)
	switch (pin_nb) {
	case 8:
		*irq_id = EXTI8_IRQn;
		break;
	case 12:
		*irq_id = EXTI12_IRQn;
		break;
	case 7:
		*irq_id = EXTI7_IRQn;
		break;
	case 5:
		*irq_id = EXTI5_IRQn;
		break;
	case 10:
		*irq_id = EXTI10_IRQn;
		break;
	case 15:
		*irq_id = EXTI15_IRQn;
		break;
	case 9:
		*irq_id = EXTI9_IRQn;
		break;
	case 6:
		*irq_id = EXTI6_IRQn;
		break;
	case 0:
		*irq_id = EXTI0_IRQn;
		break;
	case 4:
		*irq_id = EXTI4_IRQn;
		break;
	case 11:
		*irq_id = EXTI11_IRQn;
		break;
	case 14:
		*irq_id = EXTI14_IRQn;
		break;
	case 3:
		*irq_id = EXTI3_IRQn;
		break;
	case 2:
		*irq_id = EXTI2_IRQn;
		break;
	case 1:
		*irq_id = EXTI1_IRQn;
		break;
	case 13:
		*irq_id = EXTI13_IRQn;
		break;
	default:
		return -ENOSYS;
	}
#else
	switch (pin_nb) {
	case 4:
		*irq_id = EXTI4_IRQn;
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
	case 0:
		*irq_id = EXTI0_IRQn;
		break;
	case 10:
	case 11:
	case 12:
	case 13:
	case 14:
	case 15:
		*irq_id = EXTI15_10_IRQn;
		break;
	case 5:
	case 6:
	case 7:
	case 8:
	case 9:
		*irq_id = EXTI9_5_IRQn;
		break;
	default:
		return -ENOSYS;
	}
#endif
	return 0;
}
