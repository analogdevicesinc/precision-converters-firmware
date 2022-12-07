/***************************************************************************//*
 * @file    ad7124_console_app.h
 * @brief   Defines the console menu structure for the AD7124 example code
 * @details
******************************************************************************
 * Copyright (c) 2021 Analog Devices, Inc. All Rights Reserved.
 *
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 * By using this software you agree to the terms of the associated
 * Analog Devices Software License Agreement.
******************************************************************************/

#ifndef AD7124_CONSOLE_APP_H_
#define AD7124_CONSOLE_APP_H_

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include "adi_console_menu.h"

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

enum sensor_config_ids {
	AD7124_CONFIG_RESET,
	AD7124_CONFIG_2WIRE_RTD,
	AD7124_CONFIG_3WIRE_RTD,
	AD7124_CONFIG_4WIRE_RTD,
	AD7124_CONFIG_THERMOCOUPLE,
	AD7124_CONFIG_THERMISTOR,
	NUMBER_OF_SENSOR_CONFIGS
};

/******************************************************************************/
/********************** Public/Extern Declarations ****************************/
/******************************************************************************/

int32_t ad7124_app_initialize(uint8_t configID);
extern console_menu ad7124_main_menu;

#endif /* AD7124_CONSOLE_APP_H_ */
