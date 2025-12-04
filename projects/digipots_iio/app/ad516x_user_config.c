/***************************************************************************//**
 * @file   ad516x_user_config.c
 * @brief  User configurations for AD5161 No-OS drivers
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

/******************************************************************************/
/******************** Variables and User Defined Data Types *******************/
/******************************************************************************/

/* AD5161 No-OS driver init parameters */
struct ad516x_dpot_init_param ad5161_init_params = {
	/* Select interface type */
	.eintf_type = AD_I2C_INTERFACE,
	/* I2C Define interface init parameters */
	.i2c_init = &i2c_init_params,
	/* SPI Define interface init parameters */
	.spi_init = &spi_mode0_init_params,
	/* GPIO for Digital interface selection. */
	.dis_gpio_init = &dis_gpio_init_params,
};

/* AD5165 No-OS driver init parameters */
struct ad516x_dpot_init_param ad5165_init_params = {
	/* Select interface type */
	.eintf_type = AD_I2C_INTERFACE,
	/* SPI Define interface init parameters */
	.spi_init = &spi_mode0_init_params,
	/* GPIO for Digital interface selection. */
	.dis_gpio_init = &dis_gpio_init_params,
};
