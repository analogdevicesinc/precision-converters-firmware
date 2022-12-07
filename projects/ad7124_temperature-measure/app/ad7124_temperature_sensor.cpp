
/***************************************************************************//*
 * @file    ad7124_temperature_sensor.cpp
 * @brief   AD7124 temperature sensor functionality
 * @details
******************************************************************************
 * Copyright (c) 2021 Analog Devices, Inc. All Rights Reserved.
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
#include <ptxxx.h>
#include <ntc_10k_44031.h>
#include <ptc_ky81_110.h>

#include "ad7124_temperature_sensor.h"

#ifdef __cplusplus
extern "C"
{
#endif //  _cplusplus

#include "ad7124_regs_configs.h"
#include "ad7124_support.h"

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

/* PTC thermistor Ref Resistance value (in ohms) */
#define PTC_RREF			5110U

/******************************************************************************/
/******************** Variables and User Defined Data Types *******************/
/******************************************************************************/

/* Calibration constant (iout ratio) for RTD resistance measurement */
static float calibration_iout_ratio = 1;

/******************************************************************************/
/************************** Functions Definitions *****************************/
/******************************************************************************/

/*!
 * @brief	Convert the ADC raw value into equivalent RTD resistance
 * @param	adc_raw[in]- ADC raw sample
 * @param	gain[in] - RTD gain
 * @return	RTD resistance value
 * @note	RTD is biased with constant excitation current. Below formula
 *			is based on ratiometric measurement, where fixed value of RTD RREF
 *			(reference resistor) and gain is taken into account
 */
static float convert_adc_raw_into_rtd_resistance(int32_t adc_raw, uint8_t gain)
{
	float rtd_res;

	/* Below equation is for bipolar inputs as all ADC configurations for
	 * sensor measurement are having default bipolar mode */
	rtd_res = (((float)adc_raw - (1 << (AD7124_ADC_N_BITS - 1))) *
		   (calibration_iout_ratio * RTD_RREF)) / ((
					   AD7124_PGA_GAIN(gain)) * (1 << (AD7124_ADC_N_BITS - 1)));

	return rtd_res;
}


/*!
 * @brief	Store the RTD calibration Iout ratio for 3-wire RTD calibration
 *			based measurement
 * @param	iout_ratio[in]- Iout1/Iout0 ratio
 * @param	status[in] - Calibration ratio set/reset flag
 * @return	none
 */
void store_rtd_calibrated_iout_ratio(float iout_ratio, bool status)
{
	if (status) {
		calibration_iout_ratio = 1 + iout_ratio;
	} else {
		calibration_iout_ratio = 1;
	}
}


/*!
 * @brief	Convert the ADC raw value into equivalent PTC thermistor resistance
 * @param	adc_raw[in]- ADC raw sample
 * @return	PTC resistance value
 * @note	PTC is biased with constant excitation current. Below formula
 *			is based on ratiometric measurement, where fixed value of PTC RREF
 *			(reference resistor) and gain is taken into account
 */
static float convert_adc_raw_into_ptc_resistance(int32_t adc_raw)
{
	float ptc_res;

	/* Below equation is for bipolar inputs as all ADC configurations for
	 * sensor measurement are having default bipolar mode */
	ptc_res = (((float)adc_raw - (1 << (AD7124_ADC_N_BITS - 1))) * PTC_RREF) / ((
				AD7124_PGA_GAIN(THERMISTOR_GAIN_VALUE)) * (1 << (AD7124_ADC_N_BITS - 1)));

	return ptc_res;
}


/*!
 * @brief	Convert the ADC raw value into equivalent NTC thermistor voltage
 * @param	adc_raw[in]- ADC raw sample
 * @return	NTC Thermistor voltage value
 * @note	The NTC is biased with constant ADC reference voltage. Below formula
 *			is based on ratiometric measurement, where fixed value of ADC REF
 *			and gain is taken into account
 */
static float convert_adc_raw_into_ntc_voltage(int32_t adc_raw)
{
	float ntc_voltage;

	/* Below equation is for bipolar inputs as all ADC configurations for
	 * sensor measurement are having default bipolar mode */
	ntc_voltage = (((float)adc_raw - (1 << (AD7124_ADC_N_BITS - 1))) *
		       AD7124_REF_VOLTAGE) / ((
				       AD7124_PGA_GAIN(THERMISTOR_GAIN_VALUE)) * (1 << (AD7124_ADC_N_BITS - 1)));

	return ntc_voltage;
}


/*!
 * @brief	Convert the NTC thermistor voltage into equivalent resistance
 * @param	voltage[in]- NTC Thermistor voltage
 * @return	NTC Thermistor resistance value
 */
static float convert_ntc_voltage_into_resistance(float ntc_voltage)
{
	float ntc_resistance = (ntc_voltage * NTC_RSENSE) / (AD7124_REF_VOLTAGE -
			       ntc_voltage);
	return ntc_resistance;
}


/**
 * @brief  	Converts raw ADC code to millivolts
 * @param	raw_adc_code[in] Raw ADC code
 * @return	voltage in millivolts
 * @details	This converts the raw ADC code to millivolts for thermocouple channel,
 *          based on the AD7124 Eval board configuration
 */
static float convert_raw_adc_into_tc_mv(uint32_t raw_adc_code)
{
	return (((((float)raw_adc_code - (1 << (AD7124_ADC_N_BITS - 1))) /
		  (AD7124_PGA_GAIN(THERMOCOUPLE_GAIN_VALUE) *
		   (1 << (AD7124_ADC_N_BITS - 1)))) * AD7124_REF_VOLTAGE) * 1000);
}



/**
 * @brief  	Get the RTD reference resistor value
 * @return	RTD reference resistor value
 */
uint32_t get_rtd_rref(void)
{
	return RTD_RREF;
}


/**
 * @brief  	Convert ADC raw value into TC temperature
 * @param	tc_sample[in] Raw TC sample
 * @param	cjc_sample[in] Raw CJC sample
 * @param	cjc_sensor[in] CJC sensor type
 * @param	cjc_temp[in] CJC temperature value
 * @return	TC temperature
 */
float get_tc_temperature(float tc_sample, float cjc_sample,
			 cjc_sensor_type cjc_sensor, float *cjc_temp)
{
	Thermocouple_Type_T tcSensor;
	float tc_mv;
	float tc_temperature;
	float cjc_temperature;

	tc_mv = convert_raw_adc_into_tc_mv(tc_sample);
	tc_temperature = tcSensor.convert(tc_mv);

	if (cjc_sensor == PT100_4WIRE_RTD) {
		cjc_temperature = get_rtd_temperature(cjc_sample, RTD_4WIRE_GAIN_VALUE);
	} else if (cjc_sensor == THERMISTOR_PTC_KY81_110) {
		cjc_temperature = get_ptc_thermistor_temperature(cjc_sample);
	} else if (cjc_sensor == PT1000_2WIRE_RTD) {
		PT1000 rtd_sensor;
		float rtd_resistance;

		rtd_resistance = convert_adc_raw_into_rtd_resistance(cjc_sample,
				 RTD_PT1000_GAIN_VALUE);
		cjc_temperature = rtd_sensor.convertResistanceToTemperature(rtd_resistance);
	} else {
		return 0;
	}

	/* Get the CJC temperature */
	*cjc_temp = cjc_temperature;

	/* NOTE The simplest approach of adding the CJC temperature to TC temperature is taken here.
	 * A better method is to convert RTD back to thermocouple mV, and add that to TC value
	 * then do the thermocouple to degC conversion.
	 * */
	return (tc_temperature + cjc_temperature);
}


/**
 * @brief  	Convert ADC raw value into RTD temperature
 * @param	rtd_sample[in] Raw RTD sample
 * @pram	gain[in]  RTD gain
 * @return	RTD temperature
 * @note	Fixed PT100 RTD sensor is used
 */
float get_rtd_temperature(int32_t rtd_sample, uint8_t gain)
{
	PT100 rtd_sensor;
	float rtd_resistance;

	rtd_resistance = convert_adc_raw_into_rtd_resistance(rtd_sample, gain);

	return rtd_sensor.convertResistanceToTemperature(rtd_resistance);
}


/**
 * @brief  	Convert ADC raw value into NTC temperature
 * @param	ntc_sample[in] Raw NTC sample
 * @return	NTC temperature
 * @note	Fixed NTC 10K 44031RC sensor is used
 */
float get_ntc_thermistor_temperature(int32_t ntc_sample)
{
	ntc_10k_44031rc ntc_thermistor;
	float ntc_voltage;
	float ntc_resistance;

	ntc_voltage = convert_adc_raw_into_ntc_voltage(ntc_sample);
	ntc_resistance = convert_ntc_voltage_into_resistance(ntc_voltage);

	return ntc_thermistor.convert(ntc_resistance);
}


/**
 * @brief  	Convert ADC raw value into PTC temperature
 * @param	ptc_sample[in] Raw PTC sample
 * @return	PTC temperature
 * @note	Fixed PTC KY81/110 Thermistor sensor is used
 */
float get_ptc_thermistor_temperature(int32_t ptc_sample)
{
	ptc_ky81_110 ptc_thermistor;
	float ptc_resistance;

	ptc_resistance = convert_adc_raw_into_ptc_resistance(ptc_sample);

	return ptc_thermistor.convert(ptc_resistance);
}
