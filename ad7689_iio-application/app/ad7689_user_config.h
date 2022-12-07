/***************************************************************************//*
 * @file    ad7689_user_config.h
 * @brief   User configurations for AD7689 No-OS driver
******************************************************************************
 * Copyright (c) 2021 Analog Devices, Inc.
 * All rights reserved.
 *
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 * By using this software you agree to the terms of the associated
 * Analog Devices Software License Agreement.
******************************************************************************/

#ifndef _AD7689_USER_CONFIG_H_
#define _AD7689_USER_CONFIG_H_

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include "ad7689.h"

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

/* Select Input type (one at a time- default is unipolar) */
#define UNIPOLAR
//#define BIPOLAR

/* Default ADC channel config (for all channels)
 * Note: DO NOT SELECT TYPE AS 'AD7689_TEMPERATURE_SENSOR'. The input type is set to
 * to 'temperature' during temperature channel scanning at run-time.
 **/
#if defined(UNIPOLAR)
#define ADC_INPUT_TYPE_CFG			AD7689_UNIPOLAR_GND
#else
#define ADC_INPUT_TYPE_CFG			AD7689_BIPOLAR_COM
#endif

/* Default ADC reference voltage configurations (temperature sensor enabled by default) */
#define ADC_REF_VOLTAGE_CFG			AD7689_REF_EXTERNAL_TEMP_IBUF
#define ADC_DEFAULT_REF_VOLTAGE		5.0

/******************************************************************************/
/********************** Public/Extern Declarations ****************************/
/******************************************************************************/

extern struct ad7689_init_param ad7689_init_params;

#endif /* _AD7689_USER_CONFIG_H_ */
