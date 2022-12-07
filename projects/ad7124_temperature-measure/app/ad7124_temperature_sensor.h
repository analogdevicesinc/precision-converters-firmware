/***************************************************************************//*
 * @file    ad7124_temperature_sensor.h
 * @brief   AD7124 temperature sensor module global defines
 * @details
******************************************************************************
 * Copyright (c) 2021 Analog Devices, Inc. All Rights Reserved.
 *
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 * By using this software you agree to the terms of the associated
 * Analog Devices Software License Agreement.
******************************************************************************/

#ifndef AD7124_TEMPERATURE_SENSOR_H_
#define AD7124_TEMPERATURE_SENSOR_H_

#ifdef __cplusplus
extern "C"
{
#endif //  _cplusplus

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include <stdint.h>
#include "ad7124_regs_configs.h"

/******************************************************************************/
/********************* Macros and Constants Definitions ***********************/
/******************************************************************************/

/******************************************************************************/
/********************** Public/Extern Declarations ****************************/
/******************************************************************************/

float get_tc_temperature(float tc_sample, float cjc_sample,
			 cjc_sensor_type cjc_sensor, float *cjc_temp);
uint32_t get_rtd_rref(void);
float get_rtd_temperature(int32_t rtd_sample, uint8_t gain);
void store_rtd_calibrated_iout_ratio(float iout_ratio, bool status);
float get_ntc_thermistor_temperature(int32_t ntc_sample);
float get_ptc_thermistor_temperature(int32_t ntc_sample);

#ifdef __cplusplus  // Closing extern c
}
#endif //  _cplusplus

#endif	// end of AD7124_TEMPERATURE_SENSOR_H_
