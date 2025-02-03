/*************************************************************************//**
 *   @file   ad4170_rtd_config.c
 *   @brief  RTD (2/3/4-wire) user configurations module for AD4170 IIO firmware
******************************************************************************
* Copyright (c) 2021-22,2024-25 Analog Devices, Inc.
* All rights reserved.
*
* This software is proprietary to Analog Devices, Inc. and its licensors.
* By using this software you agree to the terms of the associated
* Analog Devices Software License Agreement.
*****************************************************************************/

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include <stdint.h>

#include "app_config.h"
#include "ad4170_rtd_config.h"

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

#define	AD4170_RTD_CONFIG_OFFSET_RESET_VAL		0x0
#define AD4170_RTD_CONFIG_GAIN_RESET_VAL		0x555555

/******************************************************************************/
/********************** Variables and User Defined Data Types *****************/
/******************************************************************************/

/* Initialize the AD4170 device structure */
struct ad4170_init_param ad4170_rtd_config_params = {
	/* Active device selection */
	.id = ACTIVE_DEVICE_ID,
	/* Note: Max supported SPI frequency can vary from one platform to other */
	.spi_init = {
		.max_speed_hz = AD4170_MAX_SPI_SPEED,	// Max SPI Speed
		.chip_select = SPI_CSB,		// Chip Select
		.mode = NO_OS_SPI_MODE_3,		// CPOL = 1, CPHA = 1
		.platform_ops = &spi_ops,
		.extra = &spi_extra_init_params // SPI extra configurations
	},

	.spi_settings = {
		.short_instruction = false,		// 14-bit instruction mode to access full register range
		.crc_enabled = false,			// CRC Disabled for faster data access
		/* Use during 3-wire Isolated SPI mode (no CSB) -Not supported with firmware */
		.sync_loss_detect = false
	},

	.rdy_conv_timeout = 10000000,

	.config = {
		.pin_muxing = {
			.chan_to_gpio = AD4170_CHANNEL_NOT_TO_GPIO,
#if (INTERFACE_MODE == SPI_INTERRUPT_MODE)
#if defined(DEV_AD4190)
			.dig_aux2_ctrl = AD4170_DIG_AUX2_SYNC, // Used as START Input.
#else
			.dig_aux2_ctrl = AD4170_DIG_AUX2_LDAC,	// Used as h/w LDACB
#endif // DEV_AD4190
			.dig_aux1_ctrl = AD4170_DIG_AUX1_RDY,	// Used as RDY (end of conversion)
#elif (INTERFACE_MODE == TDM_MODE)
			.dig_aux1_ctrl = AD4170_DIG_AUX1_DISABLED,
			.dig_aux2_ctrl = AD4170_DIG_AUX2_DISABLED,
#endif
			.sync_ctrl = AD4170_SYNC_STANDARD,			// sync_ctrl pin must be high (deasserted)
			.dig_out_str = AD4170_DIG_STR_DEFAULT,
			.sdo_rdby_dly = AD4170_SDO_RDY_SCLK
		},

		/* Note: MCLCK is default set to 16Mhz using below configs. Changing MCLK
		 * from default value can result into a failure of data capture through IIO client */
		.clock_ctrl = {
			.dclk_divide = AD4170_DCLKDIVBY1,
			.clockdiv = AD4170_CLKDIVBY1,
			.clocksel = AD4170_INTERNAL_OSC
		},

		.standby_ctrl = 0xff, 	// All blocks active during standby
		.powerdown_sw = 0,
		.error_en = 0xff,

		.adc_ctrl = {
			.parallel_filt_en = false,
			.multi_data_reg_sel = true,		// Data register shared b/w all channels
			.cont_read_status_en = false,
			.cont_read = AD4170_CONT_READ_OFF,
			.mode = AD4170_CONT_CONV_MODE_CONFIG
		},

		/* Enabled Channel0 (channel must be enabled to apply init
		 * configurations on it such as setup, pin mapping, etc)
		 **/
		.channel_en = AD4170_CHANNEL(0) | AD4170_CHANNEL(1) | AD4170_CHANNEL(2),

		/* Channel setup */
		.setup = {
			{ .repeat_n = 0, .delay_n = AD4170_DLY_0, .setup_n = 0 },	// Chn0
			{ .repeat_n = 0, .delay_n = AD4170_DLY_0, .setup_n = 0 },	// Chn1
			{ .repeat_n = 0, .delay_n = AD4170_DLY_0, .setup_n = 0 },	// Chn2
		},

		/* Channel input mapping */
		.map = {
			{ . ainp = AD4170_AIN0, .ainm = AD4170_AIN1 },	// Chn0
			{ . ainp = AD4170_AIN3, .ainm = AD4170_AIN4 },	// Chn1
			{ . ainp = AD4170_AIN7, .ainm = AD4170_AIN8 },	// Chn2
		},

		/* Setup configurations */
		.setups = {
			// Setup0
			{
				/* NOTE: chop_adc =  AD4170_CHOP_IEXC_CD and AD4170_CHOP_IEXC_ABCD
				 * options are not available on the AD4190 */
				.misc = {
					.chop_iexc = AD4170_CHOP_IEXC_OFF,
					.chop_adc = AD4170_CHOP_OFF,
					.burnout = AD4170_BURNOUT_OFF
				},
				.afe = {
					.ref_buf_m = AD4170_REF_BUF_FULL,
					.ref_buf_p = AD4170_REF_BUF_FULL,
					.ref_select = AD4170_REFIN_REFIN1,
					.bipolar = true,
					.pga_gain = AD4170_PGA_GAIN_16
				},
				.filter = {
					.post_filter_sel = AD4170_POST_FILTER_NONE,
					.filter_type = AD4170_FILTER_CONFIG
				},
				.filter_fs = AD4170_FS_CONFIG,
				.offset = AD4170_RTD_CONFIG_OFFSET_RESET_VAL,
				.gain = AD4170_RTD_CONFIG_GAIN_RESET_VAL,
			},
		},

		{ .ref_en = true },		// Enable internal reference (needed to activate excitation currents)
		.v_bias = 0,			// No Vbias enabled on any input
		.i_pullup = 0,			// No pull-up enabled on any input

		.current_source = {
			/* Note: The excitation current is applied and removed during conversion.
			 * After power-up initialization, the excitation currents are 0. They
			 * are set to specific value only when conversion is in progress through
			 * ad4170_set_excitation_sources() function in ad4170_support.c file */
#if (ACTIVE_DEMO_MODE_CONFIG == RTD_3WIRE_CONFIG)
			// Chn0
			{ .i_out_pin = AD4170_I_OUT_GPIO3, .i_out_val = AD4170_I_OUT_0UA },
			{ .i_out_pin = AD4170_I_OUT_GPIO2, .i_out_val = AD4170_I_OUT_0UA },

			// Chn1
			{ .i_out_pin = AD4170_I_OUT_AIN7, .i_out_val = AD4170_I_OUT_0UA },
			{ .i_out_pin = AD4170_I_OUT_AIN8, .i_out_val = AD4170_I_OUT_0UA }
#else
			{ .i_out_pin = AD4170_I_OUT_GPIO3, .i_out_val = AD4170_I_OUT_0UA },	// Chn0
			{ .i_out_pin = AD4170_I_OUT_GPIO2, .i_out_val = AD4170_I_OUT_0UA },	// Chn1
			{ .i_out_pin = AD4170_I_OUT_AIN2, .i_out_val = AD4170_I_OUT_0UA },	// Chn2
			{ .i_out_pin = AD4170_I_OUT_AIN0, .i_out_val = AD4170_I_OUT_0UA },
#endif
		},
#if !defined(DEV_AD4190)
		.fir_control = {
			.fir_mode = AD4170_FIR_DEFAULT,
			.coeff_set = AD4170_FIR_COEFF_SET0,
			.fir_length = 0,
			.fir_coefficients = NULL
		},
		.dac = {
			.enabled = false,
			.gain = AD4170_DAC_GAIN_1,
			.hw_toggle = false,
			.hw_ldac = false
		}
#endif
	},

	&gpio_init_sync_inb,
	&gpio_init_rdy,		// DIAG_AUX1
	&gpio_init_ldac_n	// DIAG_AUX2
};
