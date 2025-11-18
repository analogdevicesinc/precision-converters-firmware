/***************************************************************************//**
 * @file   ad514x_user_config.c
 * @brief  User configurations for AD514x No-OS drivers
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
/********************* Macros and Constants Definition ************************/
/******************************************************************************/

/******************************************************************************/
/******************** Variables and User Defined Data Types *******************/
/******************************************************************************/

/* AD5141 No-OS driver init parameters */
struct ad5141_dpot_init_param ad5141_init_params = {
	/* Select interface type */
	.eintf_type = AD_SPI_INTERFACE,
	/* Define interface init parameters */
	.spi_init = &spi_mode2_init_params,
	.i2c_init = &i2c_init_params,

	/* Define gpio init parameters */
	.reset_gpio_init = &reset_gpio_init_params,
	.dis_gpio_init = &dis_gpio_init_params,
	.lrdac_gpio_init = &lrdac_gpio_init_params,
	.wp_gpio_init = &wp_gpio_init_params,
	.indep_gpio_init = &indep_gpio_init_params,
	.eoperating_mode = DPOT_DEFAULT_OPERATING_MODE,
	.shutdown_enable_pt = {
		DPOT_RDAC1_DEFAULT_SHUTDOWN
	},
	.shutdown_enable_lg = {
		DPOT_RAW1_DEFAULT_SHUTDOWN,
		DPOT_RWB1_DEFAULT_SHUTDOWN
	}
};

/* AD5142 No-OS driver init parameters */
struct ad5142_dpot_init_param ad5142_init_params = {
	.eintf_type = AD_SPI_INTERFACE,
	/* Define interface init parameters */
	.spi_init = &spi_mode2_init_params,
	.i2c_init = &i2c_init_params,
	/* Define gpio init parameters */
	.reset_gpio_init = &reset_gpio_init_params,
	.dis_gpio_init = &dis_gpio_init_params,
	.lrdac_gpio_init = &lrdac_gpio_init_params,
	.wp_gpio_init = &wp_gpio_init_params,
	.indep_gpio_init = &indep_gpio_init_params,
	.eoperating_mode = DPOT_DEFAULT_OPERATING_MODE,
	.shutdown_enable_pt = {
		DPOT_RDAC1_DEFAULT_SHUTDOWN,
		DPOT_RDAC2_DEFAULT_SHUTDOWN
	},
	.shutdown_enable_lg = {
		DPOT_RAW1_DEFAULT_SHUTDOWN,
		DPOT_RWB1_DEFAULT_SHUTDOWN,
		DPOT_RAW2_DEFAULT_SHUTDOWN,
		DPOT_RWB2_DEFAULT_SHUTDOWN
	}
};

/* AD5144 No-OS driver init parameters */
struct ad5144_dpot_init_param ad5144_init_params = {
	/* Define interface init parameters */
	.eintf_type = AD_SPI_INTERFACE,
	.spi_init = &spi_mode2_init_params,
	.i2c_init = &i2c_init_params,
	/* Define gpio init parameters */
	.reset_gpio_init = &reset_gpio_init_params,
	.indep_gpio_init = &indep_gpio_init_params,
	.dis_gpio_init  = &dis_gpio_init_params,
	.eoperating_mode = DPOT_DEFAULT_OPERATING_MODE,
	.shutdown_enable_pt = {
		DPOT_RDAC1_DEFAULT_SHUTDOWN,
		DPOT_RDAC2_DEFAULT_SHUTDOWN,
		DPOT_RDAC3_DEFAULT_SHUTDOWN,
		DPOT_RDAC4_DEFAULT_SHUTDOWN
	},
	.shutdown_enable_lg = {
		DPOT_RAW1_DEFAULT_SHUTDOWN,
		DPOT_RWB1_DEFAULT_SHUTDOWN,
		DPOT_RAW2_DEFAULT_SHUTDOWN,
		DPOT_RWB2_DEFAULT_SHUTDOWN,
		DPOT_RAW3_DEFAULT_SHUTDOWN,
		DPOT_RWB3_DEFAULT_SHUTDOWN,
		DPOT_RAW4_DEFAULT_SHUTDOWN,
		DPOT_RWB4_DEFAULT_SHUTDOWN
	}
};

/* AD5143 No-OS driver init parameters */
struct ad5143_dpot_init_param ad5143_init_params = {
	/* Define interface init parameters */
	.i2c_init = &i2c_init_params,
	.eoperating_mode = DPOT_DEFAULT_OPERATING_MODE,
	.shutdown_enable_pt = {
		DPOT_RDAC1_DEFAULT_SHUTDOWN,
		DPOT_RDAC2_DEFAULT_SHUTDOWN,
		DPOT_RDAC3_DEFAULT_SHUTDOWN,
		DPOT_RDAC4_DEFAULT_SHUTDOWN
	},
	.shutdown_enable_lg = {
		DPOT_RAW1_DEFAULT_SHUTDOWN,
		DPOT_RWB1_DEFAULT_SHUTDOWN,
		DPOT_RAW2_DEFAULT_SHUTDOWN,
		DPOT_RWB2_DEFAULT_SHUTDOWN,
		DPOT_RAW3_DEFAULT_SHUTDOWN,
		DPOT_RWB3_DEFAULT_SHUTDOWN,
		DPOT_RAW4_DEFAULT_SHUTDOWN,
		DPOT_RWB4_DEFAULT_SHUTDOWN,
	},
};

