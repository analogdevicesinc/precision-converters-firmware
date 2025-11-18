/*************************************************************************//**
 * @file   dpot_user_config.h
 * @brief  User configurations for digipot No-OS drivers
******************************************************************************
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
*****************************************************************************/
#ifndef DPOT_USER_CONFIG_H
#define DPOT_USER_CONFIG_H

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/
#include <stdint.h>
#include "app_config.h"
#include "ad5141.h"
#include "ad5142.h"
#include "ad5143.h"
#include "ad5144.h"
#include "ad5259.h"
#include "ad5161.h"
#include "ad5246.h"
#include "ad5242.h"
#include "ad5171.h"


/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

/******************************************************************************/
/********************** Variables and User Defined Data Types *****************/
/******************************************************************************/

/******************************************************************************/
/************************ Public Declarations *********************************/
/******************************************************************************/



/* ===========================================================
 * DPOT USER DEFAULT CONFIGURATIONS
 * =========================================================== */
/* Default operating mode */
#define DPOT_DEFAULT_OPERATING_MODE  DPOT_POTENTIOMETER_MODE

/* Default channel shutdown status */
#define DPOT_RDAC1_DEFAULT_SHUTDOWN  false
#define DPOT_RDAC2_DEFAULT_SHUTDOWN  false
#define DPOT_RDAC3_DEFAULT_SHUTDOWN  false
#define DPOT_RDAC4_DEFAULT_SHUTDOWN  false
#define DPOT_RAW1_DEFAULT_SHUTDOWN   false
#define DPOT_RWB1_DEFAULT_SHUTDOWN   false
#define DPOT_RAW2_DEFAULT_SHUTDOWN   false
#define DPOT_RWB2_DEFAULT_SHUTDOWN   false
#define DPOT_RAW3_DEFAULT_SHUTDOWN   false
#define DPOT_RWB3_DEFAULT_SHUTDOWN   false
#define DPOT_RAW4_DEFAULT_SHUTDOWN   false
#define DPOT_RWB4_DEFAULT_SHUTDOWN   false

#define DEFAULT_DPOT_OPS	ad5144_dpot_ops
#define DEFAULT_DPOT_EXTRA_PARAMS	ad5144_init_params

extern struct dpot_init_param dpot_init_params;
extern struct no_os_gpio_init_param reset_gpio_init_params;
extern struct no_os_gpio_init_param wp_gpio_init_params;
extern struct no_os_gpio_init_param lrdac_gpio_init_params;
extern struct no_os_gpio_init_param dis_gpio_init_params;
extern struct no_os_gpio_init_param indep_gpio_init_params;

extern struct no_os_gpio_init_param shdn_gpio_init_params;

extern struct no_os_gpio_init_param down_gpio_init_params;

extern struct no_os_gpio_init_param up_gpio_init_params;
extern struct ad5143_dpot_init_param ad5143_init_params;
extern struct ad5141_dpot_init_param ad5141_init_params;
extern struct ad5144_dpot_init_param ad5144_init_params;
extern struct ad5242_dpot_init_param ad5242_init_params;
#endif // DPOT_USER_CONFIG_H
