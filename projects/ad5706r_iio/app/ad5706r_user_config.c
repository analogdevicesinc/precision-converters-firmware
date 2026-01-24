/***************************************************************************//**
 *   @file    ad5706r_user_config.c
 *   @brief   User configuration source for the AD5706R IIO Application
********************************************************************************
 * Copyright (c) 2024-2026 Analog Devices, Inc.
 *
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 * By using this software you agree to the terms of the associated
 * Analog Devices Software License Agreement.
*******************************************************************************/

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include <stdint.h>
#include "ad5706r_user_config.h"
#include "app_config.h"
#include "no_os_spi.h"
#include "no_os_gpio.h"

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

/******************************************************************************/
/********************** Variables and User Defined Data Types *****************/
/******************************************************************************/

/* SPI Initialization parameters */
static struct no_os_spi_init_param spi_init_params = {
	.device_id = SPI_DEVICE_ID,
	.max_speed_hz = MAX_SPI_SCLK_FREQ,
	.mode = NO_OS_SPI_MODE_0,
	.chip_select = SPI_CSB,
	.bit_order = NO_OS_SPI_BIT_ORDER_MSB_FIRST,
	.platform_ops = &spi_ops,
	.extra = &spi_extra_init_params
};

/* GPIO Reset parameters */
struct no_os_gpio_init_param gpio_reset_params = {
	.port = RESET_PORT,
	.number = GPIO_RESET,
	.platform_ops = &gpio_ops,
	.extra = &gpio_reset_extra_params
};

/* Initialize the AD5706R device structure */
struct ad5706r_init_param ad5706r_init_params = {
	.spi_init_prm = &spi_init_params,
	.gpio_ldac_tgp = &gpio_ldac_tg_params,
	.gpio_reset = &gpio_reset_params,
	.vref_enable = AD5706R_EXTERNAL_VREF_PIN_INPUT,
	.op_mode = { AD5706R_SHUTDOWN_SW, AD5706R_SHUTDOWN_SW, AD5706R_SHUTDOWN_SW, AD5706R_SHUTDOWN_SW },
	.range = { AD5706R_50mA, AD5706R_50mA, AD5706R_50mA, AD5706R_50mA },
	.crc_en = false,
	.spi_cfg = { false, false, false, false, 1 },
	.dac_mode = { AD5706R_DIRECT_WRITE_REG, AD5706R_DIRECT_WRITE_REG, AD5706R_DIRECT_WRITE_REG, AD5706R_DIRECT_WRITE_REG },
	.ldac_cfg = {
		.func_en_mask = 0x0000,
		.func_mode = { AD5706R_TOGGLE, AD5706R_TOGGLE, AD5706R_TOGGLE, AD5706R_TOGGLE },
		.ldac_hw_sw_mask = 0x0000,
		.ldac_sync_async_mask = 0x0000,
		.multi_dac_ch_mask = 0x0000,
		.dither_period = { AD5706R_SAMPLES_4, AD5706R_SAMPLES_4, AD5706R_SAMPLES_4, AD5706R_SAMPLES_4 },
		.dither_phase = { AD5706R_DEGREES_0, AD5706R_DEGREES_0, AD5706R_DEGREES_0, AD5706R_DEGREES_0 },
		.edge_trig = { AD5706R_RISING_EDGE_TRIG, AD5706R_RISING_EDGE_TRIG, AD5706R_RISING_EDGE_TRIG, AD5706R_RISING_EDGE_TRIG }
	},
	.dev_addr = 0x0,
	.mux_out_mode = AD5706R_HIGH_Z,
	.mux_out_sel = AD5706R_AGND,
};
