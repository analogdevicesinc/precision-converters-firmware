/***************************************************************************//*
 * @file    ad4130_temperature_sensor.cpp
 * @brief   AD4130 temperature sensor measurement functionality
******************************************************************************
 * Copyright (c) 2022 Analog Devices, Inc.
 * All Rights Reserved.
 *
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 * By using this software you agree to the terms of the associated
 * Analog Devices Software License Agreement.
******************************************************************************/

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include <stdint.h>
#include <math.h>

#include <thermocouple.h>
#include <ntc_10k_44031.h>
#include <ptxxx.h>
#include "ad4130_temperature_sensor.h"

#ifdef __cplusplus
extern "C"
{
#endif //  _cplusplus

#include "app_config.h"
#include "ad4130_support.h"

#ifdef __cplusplus  // Closing extern c
}
#endif //  _cplusplus

/******************************************************************************/
/********************* Macros and Constants Definitions ***********************/
/******************************************************************************/

/* NTC thermistor Rsense value (in ohms) */
#define NTC_RSENSE			10000U

/* RTD Rref Resistance value (in ohms) */
#define RTD_RREF			5110U

/******************************************************************************/
/******************** Variables and User Defined Data Types *******************/
/******************************************************************************/

/******************************************************************************/
/************************** Functions Definitions *****************************/
/******************************************************************************/

/*!
 * @brief	Convert the NTC thermistor voltage into equivalent resistance
 * @param	ntc_voltage[in] - NTC Thermistor voltage
 * @return	NTC Thermistor resistance value
 * @note	The NTC is biased with constant ADC reference voltage. Below formula
 *			is based on ratiometric measurement, where fixed value of ADC REF
 *			and gain is taken into account
 */
static float convert_ntc_voltage_into_resistance(float ntc_voltage)
{
	return ((ntc_voltage * NTC_RSENSE) / (AD4170_1_25V_INT_REF_VOLTAGE -
					      ntc_voltage));
}

/**
 * @brief  	Convert ADC raw value into equivalent NTC temperature
 * @param	dev[in] - Device instance
 * @param	ntc_sample[in] - Raw ADC sample for NTC sensor
 * @param	chn[in] - ADC channel
 * @return	NTC temperature
 * @note	Fixed NTC 10K 44031RC sensor is used
 */
float get_ntc_thermistor_temperature(void *dev, uint32_t ntc_sample,
				     uint8_t chn)
{
	ntc_10k_44031rc ntc_thermistor;
	float ntc_voltage;
	float ntc_resistance;

	ntc_voltage = convert_adc_sample_into_voltage(dev, ntc_sample, chn);
	ntc_resistance = convert_ntc_voltage_into_resistance(ntc_voltage);

	return ntc_thermistor.convert(ntc_resistance);
}

/**
 * @brief  	Convert ADC raw value into equivalent RTD temperature
 * @param	dev[in] - Device instance
 * @param	rtd_sample[in] - Raw ADC sample for RTD sensor
 * @pram	chn[in] - ADC channel
 * @return	RTD temperature
 * @note	Fixed PT100 RTD sensor is used
 */
float get_rtd_temperature(void *dev, uint32_t rtd_sample, uint8_t chn)
{
	PT100 rtd_sensor;
	float rtd_resistance;

	rtd_resistance = convert_adc_raw_into_rtd_resistance(dev, rtd_sample, RTD_RREF,
			 chn);
	return rtd_sensor.convertResistanceToTemperature(rtd_resistance);
}

/**
 * @brief  	Convert ADC raw value into TC temperature
 * @param	dev[in] - Device instance
 * @param	tc_sample[in] - Raw TC sample
 * @param	cjc_sample[in] - Raw CJC sample
 * @param	tc_chn[in] - TC ADC channel
 * @param	cjc_chn[in] - CJC ADC channel
 * @param	cjc_temp[in, out] - CJC temperature value
 * @return	TC temperature
 * @note	T type thermocouple is used as default. For CJC, PT1000
 *			RTD sensor is used as default.
 */
float get_tc_temperature(void *dev,
			 uint32_t tc_sample, uint32_t cjc_sample,
			 uint8_t tc_chn, uint8_t cjc_chn,
			 float *cjc_temp)
{
	Thermocouple_Type_T tcSensor;
	float tc_mv;
	float tc_temperature;
	float cjc_temperature;
	PT1000 rtd_sensor;
	float rtd_resistance;

	tc_mv = convert_adc_sample_into_voltage(dev, tc_sample, tc_chn) * 1000;
	tc_temperature = tcSensor.convert(tc_mv);

#if defined(USE_CJC_AS_RTD)
	rtd_resistance = convert_adc_raw_into_rtd_resistance(dev, cjc_sample, RTD_RREF,
			 cjc_chn);
	cjc_temperature = rtd_sensor.convertResistanceToTemperature(rtd_resistance);
#else
	cjc_temperature = get_ntc_thermistor_temperature(dev, cjc_sample, cjc_chn);
#endif

	/* Get the CJC temperature */
	*cjc_temp = cjc_temperature;

	/* NOTE The simplest approach of adding the CJC temperature to TC temperature is taken here.
	 * A better method is to convert RTD back to thermocouple mV, and add that to TC value
	 * then do the thermocouple to degC conversion.
	 * */
	return (tc_temperature + cjc_temperature);
}
