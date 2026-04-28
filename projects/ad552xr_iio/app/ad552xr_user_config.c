/***************************************************************************//**
 *   @file    ad552xr_user_config.c
 *   @brief   User configuration source for the AD552XR IIO Application
********************************************************************************
 * Copyright (c) 2026 Analog Devices, Inc.
 *
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 * By using this software you agree to the terms of the associated
 * Analog Devices Software License Agreement.
*******************************************************************************/

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/
#include "ad552xr_user_config.h"
#include "app_config.h"

/******************************************************************************/
/********************** Macros and Constants Definitions **********************/
/******************************************************************************/

/******************************************************************************/
/********************** Variables and User Defined Data Types *****************/
/******************************************************************************/
/* Initialize the AD552XR device structure */
struct ad552xr_init_param ad552xr_user_config = {
	.dev_addr = 0,
	.gpio_alarmb = &gpio_alarm_n_init_params,
	.gpio_clearb = &gpio_clear_n_init_params,
	.gpio_ldac_tgp = {
		&gpio_ldac_tgpx_init_params[0],
		&gpio_ldac_tgpx_init_params[1],
		&gpio_ldac_tgpx_init_params[2],
		&gpio_ldac_tgpx_init_params[3],
	},
	.gpio_md_addr = {&gpio_md_addr0_init_params, &gpio_md_addr1_init_params},
	.gpio_resetb = &gpio_reset_n_init_params,
	.spi_init_prm = &spi_init_params,
};

/******************************************************************************/
/************************** Functions Declarations ****************************/
/******************************************************************************/

/******************************************************************************/
/************************** Functions Definitions *****************************/
/******************************************************************************/
