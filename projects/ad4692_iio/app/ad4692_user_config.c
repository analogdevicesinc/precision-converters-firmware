/*************************************************************************//**
 *   @file   ad4692_user_config.c
 *   @brief  User configuration file for AD4692 device
******************************************************************************
* Copyright (c) 2024 Analog Devices, Inc.
*
* All rights reserved.
*
* This software is proprietary to Analog Devices, Inc. and its licensors.
* By using this software you agree to the terms of the associated
* Analog Devices Software License Agreement.
*****************************************************************************/

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include "ad4692_user_config.h"
#include "app_config.h"
#include "ad4692.h"
#include "no_os_pwm.h"

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

/******************************************************************************/
/********************** Variables and User Defined Data Types *****************/
/******************************************************************************/

/* SPI Init parameters */
struct no_os_spi_init_param ad4692_spi_init = {
	.device_id = SPI_DEVICE_ID,
	.max_speed_hz = MAX_SPI_BAUDRATE,
	.platform_ops = &spi_ops,
	.chip_select = SPI_CSB,
	.mode = NO_OS_SPI_MODE_3,
	.extra = &spi_extra_init_params,
};

/* PWM GPIO init parameters */
struct no_os_gpio_init_param cnv_pwm_gpio_params = {
	.port = CNV_PORT_NUM,
	.number = CNV_PIN_NUM,
	.platform_ops = &gpio_ops,
	.extra = &cnv_pwm_gpio_extra_init_params
};

/* PWM init parameters for conversion pulses */
struct no_os_pwm_init_param pwm_init_convst = {
	.id = CNV_TIMER_ID,
	.platform_ops = &pwm_ops,
	.pwm_gpio = &cnv_pwm_gpio_params,
	.extra = &pwm_cnv_extra_init_params
};

/* GPIO BSY Init parameters */
struct no_os_gpio_init_param gpio_init_busy = {
	.port = BSY_PORT_NUM,
	.number = BSY_PIN_NUM,
	.platform_ops = &gpio_ops,
	.extra = &bsy_extra_init_params
};

/* GPIO Reset Init parameters */
struct no_os_gpio_init_param gpio_init_reset = {
	.port = RESET_PORT_NUM,
	.number = RESET_PIN_NUM,
	.platform_ops = &gpio_ops,
	.extra = &reset_extra_init_params
};

/* AD4692 Init parameters */
struct ad4692_init_param ad4692_init_params = {
	.comm_param = &ad4692_spi_init,
	.conv_param = &pwm_init_convst,
	.gpio0_param = &gpio_init_busy,
	.reset_param = &gpio_init_reset,
	.id = ID_AD4692,
	.mode = AD4692_MANUAL_MODE,
	.vref = AD4692_VREF
};
