/***************************************************************************//**
 * @file    stm32_pwm_ex.c
 * @brief   Extended support for STM32 PWM peripheral.
********************************************************************************
* Copyright (c) 2026 Analog Devices, Inc.
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
#include "no_os_error.h"
#include "no_os_util.h"
#include "stm32_pwm_ex.h"

/******************************************************************************/
/********************** Macros and Constants Definitions **********************/
/******************************************************************************/
#ifdef STM32F469xx
#define TIM_PSC_MAX_VAL				TIM_PSC_PSC
#define TIM_ARR_MAX_VAL				NO_OS_GENMASK(15, 0)
#endif

#ifdef STM32H563xx
#define TIM_PSC_MAX_VAL				(TIM_PSC_PSC >> TIM_PSC_PSC_Pos)
#define TIM_ARR_MAX_VAL				NO_OS_GENMASK(19, 0)
#define LPTIM_PSC_MAX_VAL			(LPTIM_CFGR_PRESC >> LPTIM_CFGR_PRESC_Pos)
#define LPTIM_ARR_MAX_VAL			(LPTIM_ARR_ARR >> LPTIM_ARR_ARR_Pos)
#endif

/**
 * @brief 	Calculate optimal prescaler value to fit the period in ARR register.
 * @param	desc[in] - STM32 PWM descriptor.
 * @param	period_ns[in] - Period to be set.
 * @return 	Prescaler value
 */
int32_t compute_optimal_prescaler(struct stm32_pwm_desc *desc,
				  uint64_t period_ns,
				  uint32_t *opt_prescaler)
{
	uint32_t prescaler = 0;
	uint64_t val;
	uint32_t timer_frequency_hz;
	uint32_t arr_max_val_ns;

	timer_frequency_hz = desc->get_timer_clock();
	timer_frequency_hz *= desc->clock_divider;

	/* Find the smallest prescaler such that period_ns fits in ARR size */
	switch (desc->pwm_timer) {
#ifdef HAL_TIM_MODULE_ENABLED
	case STM32_PWM_TIMER_TIM:
		arr_max_val_ns = TIM_ARR_MAX_VAL * 1E9 / timer_frequency_hz;

		while (prescaler < TIM_PSC_MAX_VAL) {
			val = period_ns / (prescaler + 1);
			if (val <= arr_max_val_ns)
				break;
			prescaler++;
		}

		if ((prescaler == TIM_PSC_MAX_VAL) && (val > arr_max_val_ns)) {
			/* If prescaler is at max value and period still does not fit,
			 * return the maximum prescaler value */
			return -ENOTSUP;
		}

		break;
#endif
#ifdef HAL_LPTIM_MODULE_ENABLED
	case STM32_PWM_TIMER_LPTIM:
		arr_max_val_ns = LPTIM_ARR_MAX_VAL * 1E9 / timer_frequency_hz;

		while (prescaler < LPTIM_PSC_MAX_VAL) {
			val = period_ns / (1 << prescaler);
			if (val <= arr_max_val_ns)
				break;
			prescaler++;
		}

		if ((prescaler == LPTIM_PSC_MAX_VAL) && (val > arr_max_val_ns)) {
			/* If prescaler is at max value and period still does not fit,
			 * return the maximum prescaler value */
			return -ENOTSUP;
		}

		prescaler = 1 << prescaler;

		break;
#endif
	default:
		prescaler = desc->prescaler;
	}

	*opt_prescaler = prescaler;

	return 0;
}
