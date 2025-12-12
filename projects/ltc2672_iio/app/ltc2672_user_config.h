/***************************************************************************//*
 * @file    ltc2672_user_config.h
 * @brief   User configurations for LTC2672 No-OS driver
******************************************************************************
 * Copyright (c) 2023-25 Analog Devices, Inc.
 * All rights reserved.
 *
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 * By using this software you agree to the terms of the associated
 * Analog Devices Software License Agreement.
******************************************************************************/

#ifndef LTC2672_USER_CONFIG_H_
#define LTC2672_USER_CONFIG_H_

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include "ltc2672.h"

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

/******************************************************************************/
/********************** Public/Extern Declarations ****************************/
/******************************************************************************/

extern struct no_os_gpio_init_param gpio_ldac_params;
extern struct no_os_gpio_init_param gpio_clear_params;
extern struct no_os_gpio_init_param gpio_toggle_params;
extern struct ltc2672_init_param ltc2672_init_params;

#endif /* LTC2672_USER_CONFIG_H_ */