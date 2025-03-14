/*************************************************************************//**
 *   @file   ad7134_user_config.c
 *   @brief  User configuration file for AD7134 device
******************************************************************************
* Copyright (c) 2020-21, 2023 Analog Devices, Inc.
* All rights reserved.
*
* This software is proprietary to Analog Devices, Inc. and its licensors.
* By using this software you agree to the terms of the associated
* Analog Devices Software License Agreement.
*****************************************************************************/

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include "ad7134_user_config.h"
#include "app_config.h"

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

/******************************************************************************/
/********************** Variables and User Defined Data Types *****************/
/******************************************************************************/

/* Initialize the ad7134 device init structure */
struct ad713x_init_param ad713x_init_params = {
	/* Define SPI init parameters structure */
	{
		.max_speed_hz = 20000000,   	// Max SPI Speed
		.chip_select = SPI_CSB,   		// Chip Select
		.mode = NO_OS_SPI_MODE_0,   			// CPOL = 0, CPHA =0
		.extra = &spi_extra_init_params,	// SPI extra configurations
		.platform_ops = &spi_ops,
		.device_id  = SPI_DEVICE_ID,
	},

	/* gpio_init_params */

	/* Optional - connected to IOVDD on AD7134 EVB for ASRC controller mode */
	.gpio_mode = NULL,

	/* Optional - connected to GND on AD7134 EVB for decimation ratio selection
	 * in ASRC controller mode */
	.gpio_dclkmode = NULL,

	/* Optional - connected to IOVDD on AD7134 EVB to set DCLK pin as output
	 * in ASRC controller mode */
	.gpio_dclkio = NULL,

	/* Optional - connected to IOVDD on AD7134 EVB */
	.gpio_resetn = NULL,

	.gpio_pnd = &pdn_init_param,

	/* gpio_init_values (optional) */
	.mode_master_nslave = NO_OS_GPIO_HIGH,
	.dclkmode_free_ngated = NO_OS_GPIO_HIGH,
	.dclkio_out_nin = NO_OS_GPIO_HIGH,
	.pnd = NO_OS_GPIO_HIGH,

	/* Define the device specific additional parameters */
	.dev_id = ACTIVE_DEVICE_ID,

	/* Only 16-bit Frame + No CRC is supported for data capturing */
	.adc_data_len = ADC_16_BIT_DATA, /* DO NOT CHANGE */
	.crc_header = NO_CRC,			 /* DO NOT CHANGE */

#if (INTERFACE_MODE != TDM_MODE)
	/* DOUT0 and DOUT1 are used in dual channel mode. Chn0 & Chn1 data is available
	 * on DOUT0 pin and Chn2 & Chn3 data is available on DOUT1 pin */
	.format = DUAL_CH_DC,	/* DO NOT CHANGE */
#else
	/* Only DOUT0 is used as the data out pin in case of TDM Mode */
	.format = SINGLE_CH_DC, /* DO NOT CHANGE */
#endif

	.clk_delay_en = false,
	.spi_common_dev = NULL
};
