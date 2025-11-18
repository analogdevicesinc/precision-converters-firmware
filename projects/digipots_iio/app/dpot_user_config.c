/***************************************************************************//**
 * @file   dpot_user_config.c
 * @brief  User configurations for digipot No-OS drivers
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

#include "dpot_user_config.h"
#include "dpot.h"

/******************************************************************************/
/********************* Macros and Constants Definition ************************/
/******************************************************************************/

/******************************************************************************/
/******************** Variables and User Defined Data Types *******************/
/******************************************************************************/

/* Reset GPO init parameters */
struct no_os_gpio_init_param reset_gpio_init_params = {
	.port = RESET_GPIO_PORT,
	.number = RESET_GPIO_PIN,
	.platform_ops = &gpio_ops,
	.extra = &reset_gpio_extra_init_params
};

/* EEPROM write protect GPO init parameters */
struct no_os_gpio_init_param wp_gpio_init_params = {
	.port = WP_GPIO_PORT,
	.number = WP_GPIO_PIN,
	.platform_ops = &gpio_ops,
	.extra = &wp_gpio_extra_init_params
};

/* LRDAC GPO init parameters */
struct no_os_gpio_init_param lrdac_gpio_init_params = {
	.port = LRDAC_GPIO_PORT,
	.number = LRDAC_GPIO_PIN,
	.platform_ops = &gpio_ops,
	.extra = &lrdac_gpio_extra_init_params
};

/* Digital interface select GPO init parameters */
struct no_os_gpio_init_param dis_gpio_init_params = {
	.port = DIS_GPIO_PORT,
	.number = DIS_GPIO_PIN,
	.platform_ops = &gpio_ops,
	.extra = &dis_gpio_extra_init_params
};

/* INDEP GPO init parameters */
struct no_os_gpio_init_param indep_gpio_init_params = {
	.port = INDEP_GPIO_PORT,
	.number = INDEP_GPIO_PIN,
	.platform_ops = &gpio_ops,
	.extra = &indep_gpio_extra_init_params
};

/* INDEP GPO init parameters */
struct no_os_gpio_init_param shdn_gpio_init_params = {
	.port = INDEP_GPIO_PORT,
	.number = INDEP_GPIO_PIN,
	.platform_ops = &gpio_ops,
	.extra = &indep_gpio_extra_init_params
};
/* Digipot driver init parameters */
struct dpot_init_param dpot_init_params = {
	.device_id = DEFAULT_ACTIVE_DEVICE,
	.dpot_ops = &DEFAULT_DPOT_OPS,
	.extra = &DEFAULT_DPOT_EXTRA_PARAMS
};

