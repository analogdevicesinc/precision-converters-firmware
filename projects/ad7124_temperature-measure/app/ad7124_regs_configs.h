/***************************************************************************//*
 * @file    ad7124_reg_app_config.h
 * @brief   Register configurations global defines
 * @details
******************************************************************************
 * Copyright (c) 2021 Analog Devices, Inc. All Rights Reserved.
 *
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 * By using this software you agree to the terms of the associated
 * Analog Devices Software License Agreement.
******************************************************************************/

#ifndef AD7124_REGS_CONFIGS_H_
#define AD7124_REGS_CONFIGS_H_

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include "app_config.h"
#include "ad7124.h"

/******************************************************************************/
/********************* Macros and Constants Definitions ***********************/
/******************************************************************************/

/* 2-wire RTD configurations */
#define	RTD1_2WIRE_IOUT0	0	// AIN0
#define	RTD2_2WIRE_IOUT0	1	// AIN1
#define	RTD3_2WIRE_IOUT0	8	// AIN8
#define	RTD4_2WIRE_IOUT0	11	// AIN11
#define	RTD5_2WIRE_IOUT0	14	// AIN14

#define	RTD1_2WIRE_AINP		2	// AIN2
#define	RTD2_2WIRE_AINP		4	// AIN4
#define	RTD3_2WIRE_AINP		6	// AIN6
#define	RTD4_2WIRE_AINP		9	// AIN9
#define	RTD5_2WIRE_AINP		12	// AIN12

#define	RTD1_2WIRE_AINM		3	// AIN3
#define	RTD2_2WIRE_AINM		5	// AIN5
#define	RTD3_2WIRE_AINM		7	// AIN7
#define	RTD4_2WIRE_AINM		10	// AIN10
#define	RTD5_2WIRE_AINM		13	// AIN13

#define RTD_2WIRE_GAIN_VALUE	4	// Gain=16

/* 3-wire RTD configurations */
#define	RTD1_3WIRE_IOUT0	0	// AIN0
#define	RTD2_3WIRE_IOUT0	6	// AIN6
#define	RTD3_3WIRE_IOUT0	10	// AIN10
#define	RTD4_3WIRE_IOUT0	14	// AIN14

#define	RTD1_3WIRE_IOUT1	1	// AIN1
#define	RTD2_3WIRE_IOUT1	7	// AIN7
#define	RTD3_3WIRE_IOUT1	11	// AIN11
#define	RTD4_3WIRE_IOUT1	15	// AIN15

#define	RTD1_3WIRE_AINP		2	// AIN2
#define	RTD2_3WIRE_AINP		4	// AIN4
#define	RTD3_3WIRE_AINP		8	// AIN8
#define	RTD4_3WIRE_AINP		12	// AIN12

#define	RTD1_3WIRE_AINM		3	// AIN3
#define	RTD2_3WIRE_AINM		5	// AIN5
#define	RTD3_3WIRE_AINM		9	// AIN9
#define	RTD4_3WIRE_AINM		13	// AIN13

#define SINGLE_3WIRE_RTD_GAIN	4	// Gain=16
#define MULTI_3WIRE_RTD_GAIN	5	// Gain=32
#define	RTD_3WIRE_EXC_MEASURE_GAIN	0	// Gain=1

/* 3-wire RTD excitation current measurement inputs (for calibration) */
#if defined(AD7124_8)
#define	RTD_3WIRE_EXC_MEASURE_AINP		14	// AIN14
#define	RTD_3WIRE_EXC_MEASURE_AINM		15	// AIN15
#else
#define	RTD_3WIRE_EXC_MEASURE_AINP		6	// AIN6
#define	RTD_3WIRE_EXC_MEASURE_AINM		7	// AIN7
#endif

/* 4-wire RTD configurations */
#define	RTD1_4WIRE_IOUT0	0	// AIN0
#define	RTD2_4WIRE_IOUT0	1	// AIN1
#define	RTD3_4WIRE_IOUT0	8	// AIN8
#define	RTD4_4WIRE_IOUT0	11	// AIN11
#define	RTD5_4WIRE_IOUT0	14	// AIN14

#define	RTD1_4WIRE_AINP		2	// AIN2
#define	RTD2_4WIRE_AINP		4	// AIN4
#define	RTD3_4WIRE_AINP		6	// AIN6
#define	RTD4_4WIRE_AINP		9	// AIN9
#define	RTD5_4WIRE_AINP		12	// AIN12

#define	RTD1_4WIRE_AINM		3	// AIN3
#define	RTD2_4WIRE_AINM		5	// AIN5
#define	RTD3_4WIRE_AINM		7	// AIN7
#define	RTD4_4WIRE_AINM		10	// AIN10
#define	RTD5_4WIRE_AINM		13	// AIN13

#define RTD_4WIRE_GAIN_VALUE	4	// Gain=16

/* RTD common configurations */
#define RTD_IOUT0_500UA_EXC		4	// 500uA IOUT0 excitation current
#define RTD_IOUT1_500UA_EXC		4	// 500uA IOUT1 excitation current
#define RTD_IOUT0_250UA_EXC		3	// 250uA IOUT0 excitation current
#define RTD_IOUT1_250UA_EXC		3	// 250uA IOUT1 excitation current
#define RTD_IOUT_EXC_OFF		0	// Excitation current is off


/* NTC Thermistor configurations */
#define NTC1_THERMISTOR_AINP	0	// AIN0
#define NTC2_THERMISTOR_AINP	2	// AIN2
#define NTC3_THERMISTOR_AINP	4	// AIN4
#define NTC4_THERMISTOR_AINP	6	// AIN6
#define NTC5_THERMISTOR_AINP	8	// AIN8
#define NTC6_THERMISTOR_AINP	10	// AIN10
#define NTC7_THERMISTOR_AINP	12	// AIN12
#define NTC8_THERMISTOR_AINP	14	// AIN14

#define NTC1_THERMISTOR_AINM	1	// AIN1
#define NTC2_THERMISTOR_AINM	3	// AIN3
#define NTC3_THERMISTOR_AINM	5	// AIN5
#define NTC4_THERMISTOR_AINM	7	// AIN7
#define NTC5_THERMISTOR_AINM	9	// AIN9
#define NTC6_THERMISTOR_AINM	11	// AIN11
#define NTC7_THERMISTOR_AINM	13	// AIN13
#define NTC8_THERMISTOR_AINM	15	// AIN15

/* NTC Thermistor common configurations */
#define THERMISTOR_GAIN_VALUE	0	// Gain=1


/* Thermocouple configurations */
#define THERMOCOUPLE1_AINP		2	// AIN2
#define THERMOCOUPLE2_AINP		6	// AIN6
#define THERMOCOUPLE3_AINP		8	// AIN8
#define THERMOCOUPLE4_AINP		10	// AIN10
#define THERMOCOUPLE5_AINP		12	// AIN12
#define THERMOCOUPLE6_AINP		14	// AIN14

#define THERMOCOUPLE1_AINM		3	// AIN3
#define THERMOCOUPLE2_AINM		7	// AIN7
#define THERMOCOUPLE3_AINM		9	// AIN9
#define THERMOCOUPLE4_AINM		11	// AIN11
#define THERMOCOUPLE5_AINM		13	// AIN13
#define THERMOCOUPLE6_AINM		15	// AIN15

/* Thermocouple common configurations */
#define THERMOCOUPLE_GAIN_VALUE		7	// Gain=128

/* Cold Junction configurations for thermocouple compensation */
/* Note: RTD to be used is either 2-wire or 4-wire */
#define CJC_RTD_AINP			4	// AIN4
#define CJC_RTD_AINM			5	// AIN5
#define	CJC_RTD_IOUT0			1	// AIN1
#define	CJC_RTD_IOUT0_EXC		RTD_IOUT0_500UA_EXC	// 500uA

#define CJC_PTC_THERMISTOR_AINP		4	// AIN4
#define CJC_PTC_THERMISTOR_AINM		5	// AIN5
#define CJC_PTC_THERMISTOR_IOUT0	1	// AIN1
#define	CJC_PTC_THERMISTOR_IOUT0_EXC 4	// 500uA

#define RTD_PT1000_GAIN_VALUE		0	// Gain=1


/* ADC internal calibration configurations */
#define ADC_CALIBRATION_GAIN		1	// Gain = 2
#define ADC_CALIBRATION_PWR_MODE	0	// Low power mode
#define ADC_CALIBRATION_REF_SRC		2	// Internal Vref

/* CJC sensor types supported for thermocouple measurement */
typedef enum {
	PT100_4WIRE_RTD,
	THERMISTOR_PTC_KY81_110,
	PT1000_2WIRE_RTD,
	NUM_OF_CJC_SENSORS
} cjc_sensor_type;

/******************************************************************************/
/********************** Public/Extern Declarations ****************************/
/******************************************************************************/

/*
 * Arrays holding the info for the AD7124 registers - address, initial value,
 * size and access type.
 */
extern const struct ad7124_st_reg ad7124_regs_config_2wire_rtd[AD7124_REG_NO];
extern const struct ad7124_st_reg ad7124_regs_config_3wire_rtd[AD7124_REG_NO];
extern const struct ad7124_st_reg ad7124_regs_config_4wire_rtd[AD7124_REG_NO];
extern const struct ad7124_st_reg ad7124_regs_config_thermistor[AD7124_REG_NO];
extern const struct ad7124_st_reg
	ad7124_regs_config_thermocouple[AD7124_REG_NO];

#endif /* AD7124_REGS_CONFIGS_H_ */
