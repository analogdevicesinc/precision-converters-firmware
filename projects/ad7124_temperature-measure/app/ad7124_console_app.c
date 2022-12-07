/***************************************************************************//**
 * @file     ad7124_console_app.c
 * @brief    AD7124 temperature measurement firmware console interface
 * @details  This file is specific to ad7124 console menu application handle.
 *           The functions defined in this file performs the action
 *           based on user selected console menu.
********************************************************************************
* Copyright (c) 2021-22 Analog Devices, Inc.
* All rights reserved.
*
* This software is proprietary to Analog Devices, Inc. and its licensors.
* By using this software you agree to the terms of the associated
* Analog Devices Software License Agreement.
*******************************************************************************/

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>

#include "app_config.h"

#include "mbed_platform_support.h"
#include "no_os_util.h"
#include "no_os_error.h"
#include "no_os_spi.h"

#include "ad7124.h"
#include "ad7124_regs.h"
#include "ad7124_support.h"
#include "ad7124_regs_configs.h"

#include "ad7124_console_app.h"
#include "ad7124_user_config.h"
#include "ad7124_temperature_sensor.h"

/******************************************************************************/
/********************* Macros and Constants Definitions ***********************/
/******************************************************************************/

/* Maximum number of ADC samples for the single/continuous measurement type */
#define MAX_ADC_SAMPLES			100

/* Maximum number of ADC samples for the average measurement type */
#define MAX_AVG_ADC_SAMPLES		8

/* The max number of sensors connected to any AD7124 device */
#define	MAX_RTD_SENSORS				5
#define MAX_THERMOCOUPLE_SENSORS	6
#define MAX_NTC_THERMISTOR_SENSORS	8

/* AD7124 max input count */
#if defined(AD7124_8)
#define AD7124_MAX_INPUTS			16
#else
#define AD7124_MAX_INPUTS			8
#endif

/* Default offset value for AD7124 */
#define AD7124_DEFAULT_OFFSET		0x800000

/* Max configurations (setups) used in the firmware (required for ADC calibration) */
#define AD7124_MAX_CONFIGS_USED		3	// TC + CJC1 + CJC2

/* @brief	ADC Conversion wait timeout period.
 * @Note	This value depends upon the core clock frequency.
 *			Below value is derived based on the code testing for SDP-K1 controller
 *			at default frequency and it ensures timeout period is more than 1 sec.
 **/
#define CONVERSION_TIMEOUT			10000

/******************************************************************************/
/******************** Variables and User Defined Data Types *******************/
/******************************************************************************/

/*
 * This is the 'live' AD7124 register map that is used by the driver
 * the other 'default' configs are used to populate this at init time
 */
static struct ad7124_st_reg ad7124_register_map[AD7124_REG_NO];

/* Pointer to the struct representing the AD7124 device */
static struct ad7124_dev *p_ad7124_dev = NULL;

/* Possible sensor configurations (mapped with enum sensor_config_ids) */
static const char *sensor_configs[NUMBER_OF_SENSOR_CONFIGS] = {
	"RESET",
	"2-Wire RTD",
	"3-Wire RTD",
	"4-Wire RTD",
	"Thermocouple",
	"Thermistor",
};

/* Maximum number of sensors connected to different AD7124 devices */
static uint8_t max_supported_sensors[NUMBER_OF_SENSOR_CONFIGS] = {
#if defined (AD7124_8)
	0,	// RESET config
	5,	// 2-wire RTDs
	4,	// 3-wire RTDs
	5,	// 4-wire RTDs
	6,	// Thermocouples
	8	// Thermistors
#else
	0,	// RESET config
	2,	// 2-wire RTDs
	2,	// 3-wire RTDs
	2,	// 4-wire RTDs
	2,	// Thermocouples
	4	// Thermistors
#endif
};

/* ADC channels assigned to sensors for the measurement (each channel per sensor) */
enum sensor_channels {
	SENSOR_CHANNEL0,
	SENSOR_CHANNEL1,
	SENSOR_CHANNEL2,
	SENSOR_CHANNEL3,
	SENSOR_CHANNEL4,
	SENSOR_CHANNEL5,
	SENSOR_CHANNEL6,
	SENSOR_CHANNEL7,
	NUM_OF_SENSOR_CHANNELS
};

/* Sensor measurement type */
typedef enum {
	/* Measurement with averaged ADC samples */
	AVERAGED_MEASUREMENT,
	/* Measurement with single ADC sample */
	SINGLE_MEASUREMENT,
	/* Continuous measurement with single ADC sample */
	CONTINUOUS_MEASUREMENT
} sensor_measurement_type;

/* Curent sensor configuration (pointer to sensor_configs string array)  */
static const char *current_sensor_config;
static enum sensor_config_ids current_sensor_config_id;

/* ADC raw data for n samples */
static int32_t n_sample_data[NUM_OF_SENSOR_CHANNELS][MAX_ADC_SAMPLES];

/* CJC sensor ADC raw data for n samples */
static int32_t n_cjc_sample_data[MAX_THERMOCOUPLE_SENSORS][MAX_ADC_SAMPLES];

/* Status info (true/false) */
static const char status_info[] = {
	'N',
	'Y'
};

/* For storing decimal value(s) in character form */
static char decimal_eqv_str[50] = "";
static char decimal_eqv_str_arr[50 * NUM_OF_SENSOR_CHANNELS] = "";

/* Sensor enable status */
static bool sensor_enable_status[NUM_OF_SENSOR_CHANNELS] = {
	true, false, false, false, false, false, false, false
};

/* CJC sensor names (for thermocouple measurement) */
static const char *cjc_sensor_names[NUM_OF_CJC_SENSORS] = {
	"PT100 4-Wire RTD",
	"Thermistor PTC KY81/110",
	"PT1000 2-Wire RTD",
};

/* Current selected CJC sensor */
static cjc_sensor_type current_cjc_sensor = PT100_4WIRE_RTD;

/* Below channels are configured through 'ad7124_regs_config_thermocouple' structure*/
#define	CJC_RTD_CHN			SENSOR_CHANNEL6
#define	CJC_THERMISTOR_CHN	SENSOR_CHANNEL7

/* ADC calibration type */
enum adc_calibration_type {
	INTERNAL_CALIBRATION,
	SYSTEM_CALIBRATION
};

/* ADC calibration configs */
typedef struct {
	int32_t power_mode;
	int32_t gain_before_calib[NUM_OF_SENSOR_CHANNELS];
	int32_t gain_after_calib[NUM_OF_SENSOR_CHANNELS];
	int32_t offset_after_calib[NUM_OF_SENSOR_CHANNELS];
	int32_t offset_before_calib[NUM_OF_SENSOR_CHANNELS];
	bool adc_calibration_done;
} adc_calibration_configs;

static adc_calibration_configs adc_calibration_config;

/* 3-wire RTD calibration types */
enum rtd_3wire_calibration_type {
	MEASURING_EXCITATION_CURRENT,
	CHOPPING_EXCITATION_CURRENT
};

/* Current RTD 3-wire calibration type */
enum rtd_3wire_calibration_type rtd_3wire_calibration_type =
	MEASURING_EXCITATION_CURRENT;

/* Below channel is configured through 'ad7124_regs_config_3wire_rtd' structure*/
#define	RTD_3WIRE_REF_MEASUREMENT_CHN	SENSOR_CHANNEL4

/* Forward declaration of console menus */
console_menu rtd_2wire_menu;
console_menu rtd_3wire_menu;
console_menu rtd_3wire_calibration_menu;
console_menu rtd_4wire_menu;
console_menu ntc_thermistor_menu;
console_menu thermocouple_menu;
console_menu adc_calibration_menu;

/******************************************************************************/
/************************** Functions Declarations ****************************/
/******************************************************************************/

/******************************************************************************/
/************************** Functions Definitions *****************************/
/******************************************************************************/

/*!
 * @brief	Initialize the AD7124 device and user configuration.
 * @param	config_id[in]- Register configuration to be load into device
 * @return	0 in case of success, negative error code otherwise
 * @details	This resets and then writes the default register map value to
 *			the device.  A call to init the SPI port is made, but may not
 *			actually do very much, depending on the platform.
 */
int32_t ad7124_app_initialize(uint8_t config_id)
{
	/*
	 * Copy one of the default/user configs to the live register memory map
	 * Requirement, not checked here, is that all the configs are the same size
	 */
	switch (config_id) {
	case AD7124_CONFIG_RESET:
		memcpy(ad7124_register_map, ad7124_regs,
		       sizeof(ad7124_register_map));
		break;

	case AD7124_CONFIG_2WIRE_RTD:
		memcpy(ad7124_register_map, ad7124_regs_config_2wire_rtd,
		       sizeof(ad7124_register_map));
		break;

	case AD7124_CONFIG_3WIRE_RTD:
		memcpy(ad7124_register_map, ad7124_regs_config_3wire_rtd,
		       sizeof(ad7124_register_map));
		break;

	case AD7124_CONFIG_4WIRE_RTD:
		memcpy(ad7124_register_map, ad7124_regs_config_4wire_rtd,
		       sizeof(ad7124_register_map));
		break;

	case AD7124_CONFIG_THERMISTOR:
		memcpy(ad7124_register_map, ad7124_regs_config_thermistor,
		       sizeof(ad7124_register_map));
		break;

	case AD7124_CONFIG_THERMOCOUPLE:
		memcpy(ad7124_register_map, ad7124_regs_config_thermocouple,
		       sizeof(ad7124_register_map));
		break;

	default:
		/* Not a defined configID */
		return -EINVAL;
	}

	/* Get the current sensor configuration */
	current_sensor_config = sensor_configs[config_id];
	current_sensor_config_id = config_id;

	/* Don't apply calibration coefficients when new config is selected */
	adc_calibration_config.adc_calibration_done = false;

	ad7124_init_params.regs = ad7124_register_map;

	return (ad7124_setup(&p_ad7124_dev, &ad7124_init_params));
}


/*!
 * @brief	Initialize the part with a specific configuration
 * @param	config_id[in] - Configuration ID
 * @return	Configuration status
 */
static int32_t init_with_configuration(uint8_t config_id)
{
	/* Free the device resources */
	(void)ad7124_remove(p_ad7124_dev);

	return ad7124_app_initialize(config_id);
}


/*!
 * @brief	determines if the Escape key was pressed
 * @return	bool- key press status
 */
static bool was_escape_key_pressed(void)
{
	char rxChar;
	bool wasPressed = false;

	/* Check for Escape key pressed */
	if ((rxChar = getchar_noblock()) > 0) {
		if (rxChar == ESCAPE_KEY_CODE) {
			wasPressed = true;
		}
	}

	return (wasPressed);
}


/*!
 * @brief	Enable the multiple sensors for measurement
 * @param	chn_number[in]- Channel number assigned for the sensor
 * @return	MENU_CONTINUE
 * @note	Each sensor has been assigned with unique channel number
 */
int32_t enable_disable_sensor(uint32_t chn_number)
{
	sensor_enable_status[chn_number] = !sensor_enable_status[chn_number];
	return MENU_CONTINUE;
}


/*!
 * @brief	Select the CJC sensor for thermocouple measurement
 * @param	cjc_sensor[in]- CJC sensor to be selected
 * @return	MENU_CONTINUE
 * @return	Only one CJC sensor is active at a time, therefore fixed
 *			channel value is selected for it during measurement
 */
int32_t select_cjc_sensor(uint32_t cjc_sensor)
{
	current_cjc_sensor = cjc_sensor;

	/* Change status of all CJC sensor channels to false at start */
	sensor_enable_status[CJC_RTD_CHN] = false;
	sensor_enable_status[CJC_THERMISTOR_CHN] = false;

	switch (current_cjc_sensor) {
	case PT100_4WIRE_RTD:
	case PT1000_2WIRE_RTD:
		sensor_enable_status[CJC_RTD_CHN] = true;
		break;

	case THERMISTOR_PTC_KY81_110:
		sensor_enable_status[CJC_THERMISTOR_CHN] = true;
		break;

	default:
		return -EINVAL;
	}

	return MENU_CONTINUE;
}


/*!
 * @brief	Perform the ADC data conversion for input channel
 * @param	chn[in]- Channel to be sampled
 * @param	data[in]- Array to store converted results
 * @param	measurement_type[in] - Temperature measurement and display type
 * @return	0 in case of success, negative error code otherwise
 * @note	This function gets the averged adc raw value for MAX_ADC_SAMPLES
 *			samples
 */
static int32_t perform_adc_conversion(uint8_t chn,
				      int32_t (*data)[MAX_ADC_SAMPLES],
				      sensor_measurement_type measurement_type)
{
	int32_t sample_data;
	int64_t avg_sample_data = 0;
	uint16_t samples_cnt;

	/* Enable the current channel */
	ad7124_register_map[AD7124_Channel_0 + chn].value |=
		AD7124_CH_MAP_REG_CH_ENABLE;
	if (ad7124_write_register(p_ad7124_dev,
				  ad7124_register_map[AD7124_Channel_0 + chn]) != 0) {
		return -EIO;
	}

	if (measurement_type == AVERAGED_MEASUREMENT) {
		samples_cnt = MAX_AVG_ADC_SAMPLES;
	} else {
		samples_cnt = MAX_ADC_SAMPLES;
	}

	/* Enter into continuous conversion mode */
	ad7124_register_map[AD7124_ADC_Control].value &= (~AD7124_ADC_CTRL_REG_MSK);
	ad7124_register_map[AD7124_ADC_Control].value |= AD7124_ADC_CTRL_REG_MODE(
				CONTINUOUS_CONV_MODE);
	if (ad7124_write_register(p_ad7124_dev,
				  ad7124_register_map[AD7124_ADC_Control]) != 0) {
		return -EIO;
	}

	/* Let the channel settle */
	no_os_mdelay(100);

	/* Read adc samples */
	for (uint16_t sample = 0; sample < samples_cnt; sample++) {
		/*
		 *  this polls the status register READY/ bit to determine when conversion is done
		 *  this also ensures the STATUS register value is up to date and contains the
		 *  channel that was sampled as well. No need to read STATUS separately
		 */
		if (ad7124_wait_for_conv_ready(p_ad7124_dev, CONVERSION_TIMEOUT) != 0) {
			break;
		}

		if (measurement_type == AVERAGED_MEASUREMENT) {
			if (ad7124_read_data(p_ad7124_dev, &sample_data) != 0) {
				break;
			}
			avg_sample_data += sample_data;
		} else {
			if (ad7124_read_data(p_ad7124_dev, data[0] + sample) != 0) {
				break;
			}
		}
	}

	/* Put ADC into Standby mode */
	ad7124_register_map[AD7124_ADC_Control].value &= (~AD7124_ADC_CTRL_REG_MSK);
	ad7124_register_map[AD7124_ADC_Control].value |= AD7124_ADC_CTRL_REG_MODE(
				STANDBY_MODE);
	if (ad7124_write_register(p_ad7124_dev,
				  ad7124_register_map[AD7124_ADC_Control]) != 0) {
		return -EIO;
	}

	/* Disable current channel */
	ad7124_register_map[AD7124_Channel_0 + chn].value &=
		~AD7124_CH_MAP_REG_CH_ENABLE;
	if (ad7124_write_register(p_ad7124_dev,
				  ad7124_register_map[AD7124_Channel_0 + chn]) != 0) {
		return -EIO;
	}

	if (measurement_type == AVERAGED_MEASUREMENT) {
		/* Calculate the averaged adc raw value */
		*data[0] = (avg_sample_data / samples_cnt);
	}

	return 0;
}


/*!
 * @brief	Perform the 3-wire RTD additional configurations
 * @param	multiple_rtd_enabled[in,out]- Multiple RTD enable status flag
 * @return	0 in case of success, negative error code otherwise
 */
static int32_t do_3wire_rtd_configs(bool *multiple_rtd_enabled)
{
	uint8_t sensor_enable_cnt = 0;
	uint8_t setup;

	*multiple_rtd_enabled = false;

	/* Check if multiple RTDs are enabled */
	for (uint8_t chn = SENSOR_CHANNEL0;
	     chn < max_supported_sensors[AD7124_CONFIG_3WIRE_RTD]; chn++) {
		if (sensor_enable_status[chn])
			sensor_enable_cnt++;

		if (sensor_enable_cnt > 1) {
			*multiple_rtd_enabled = true;
			break;
		}
	}

	for (uint8_t chn = SENSOR_CHANNEL0;
	     chn < max_supported_sensors[AD7124_CONFIG_3WIRE_RTD]; chn++) {
		if (sensor_enable_status[chn]) {
			setup = ad7124_get_channel_setup(p_ad7124_dev, chn);
			ad7124_register_map[AD7124_Config_0 + setup].value &= (~AD7124_CFG_REG_PGA_MSK);

			if (*multiple_rtd_enabled) {
				ad7124_register_map[AD7124_Config_0 + setup].value |= AD7124_CFG_REG_PGA(
							MULTI_3WIRE_RTD_GAIN);
			} else {
				ad7124_register_map[AD7124_Config_0 + setup].value |= AD7124_CFG_REG_PGA(
							SINGLE_3WIRE_RTD_GAIN);
			}

			if (ad7124_write_register(p_ad7124_dev,
						  ad7124_register_map[AD7124_Config_0 + setup]) != 0) {
				return -EIO;
			}
		}
	}

	return 0;
}


/*!
 * @brief	Select (enable/disable) excitation sources for RTD measurement
 * @param	enable_status[in] - Iout enable status
 * @param	rtd_config_id[in]- RTD type (2/3/4-wire)
 * @param	chn[in] - ADC channel assigned to given RTD sensor
 * @param	multiple_3wire_rtd_enabled[in] - Multiple RTD enable status
 * @return	0 in case of success, negative error code otherwise
 */
static int32_t select_rtd_excitation_sources(bool enable_status,
		uint32_t rtd_config_id,
		uint8_t chn, bool multiple_3wire_rtd_enabled)
{
	int32_t iout0_exc, iout1_exc;
	uint8_t arr_indx = 0;

	const uint8_t rtd_iout0_source[][MAX_RTD_SENSORS] = {
		{
			RTD1_2WIRE_IOUT0, RTD2_2WIRE_IOUT0, RTD3_2WIRE_IOUT0, RTD4_2WIRE_IOUT0,
			RTD5_2WIRE_IOUT0
		},
		{
			RTD1_3WIRE_IOUT0, RTD2_3WIRE_IOUT0, RTD3_3WIRE_IOUT0, RTD4_3WIRE_IOUT0
		},
		{
			RTD1_4WIRE_IOUT0, RTD2_4WIRE_IOUT0, RTD3_4WIRE_IOUT0, RTD4_4WIRE_IOUT0,
			RTD5_4WIRE_IOUT0
		}
	};

	const uint8_t rtd_3wire_iout1_source[] = {
		RTD1_3WIRE_IOUT1, RTD2_3WIRE_IOUT1, RTD3_3WIRE_IOUT1, RTD4_3WIRE_IOUT1,
	};

	/* Get the index mapped to RTD config ID */
	switch (rtd_config_id) {
	case AD7124_CONFIG_2WIRE_RTD:
		arr_indx = 0;
		break;

	case AD7124_CONFIG_3WIRE_RTD:
		arr_indx = 1;
		break;

	case AD7124_CONFIG_4WIRE_RTD:
		arr_indx = 2;
		break;

	default:
		break;
	}

	/* Select excitation source based on RTD configuration */
	if (multiple_3wire_rtd_enabled) {
		iout0_exc = RTD_IOUT0_250UA_EXC;
		iout1_exc = RTD_IOUT1_250UA_EXC;
	} else {
		iout0_exc = RTD_IOUT0_500UA_EXC;
		iout1_exc = RTD_IOUT1_500UA_EXC;
	}

	if (enable_status) {
		/* Enable and direct IOUT0 excitation current source for current RTD sensor measurement */
		ad7124_register_map[AD7124_IOCon1].value |= (AD7124_IO_CTRL1_REG_IOUT_CH0(
					rtd_iout0_source[arr_indx][chn]) | AD7124_IO_CTRL1_REG_IOUT0(
					iout0_exc));

		if (rtd_config_id == AD7124_CONFIG_3WIRE_RTD) {
			/* Enable and direct IOUT1 excitation current source for current RTD sensor measurement */
			ad7124_register_map[AD7124_IOCon1].value |= (AD7124_IO_CTRL1_REG_IOUT_CH1(
						rtd_3wire_iout1_source[chn]) | AD7124_IO_CTRL1_REG_IOUT1(
						iout1_exc));
		}
	} else {
		/* Turn off the excitation currents */
		ad7124_register_map[AD7124_IOCon1].value &= ((~AD7124_IO_CTRL1_REG_IOUT0_MSK)
				& (~AD7124_IO_CTRL1_REG_IOUT_CH0_MSK));

		if (rtd_config_id == AD7124_CONFIG_3WIRE_RTD) {
			ad7124_register_map[AD7124_IOCon1].value &= ((~AD7124_IO_CTRL1_REG_IOUT1_MSK)
					& (~AD7124_IO_CTRL1_REG_IOUT_CH1_MSK));
		}
	}

	if (ad7124_write_register(p_ad7124_dev,
				  ad7124_register_map[AD7124_IOCon1]) != 0) {
		return -EIO;
	}

	return 0;
}


/*!
 * @brief	Perform the ADC sampling for selected RTD sensor channel
 * @param	rtd_config_id[in]- RTD type (2/3/4-wire)
 * @param	chn[in] - ADC channel assigned to given RTD sensor
 * @param	adc_raw[out] - ADC raw result
 * @param	measurement_type[in] - Temperature measurement and display type
 * @param	multiple_3wire_rtd_enabled[in] - Multiple RTD enable status
 * @return	RTC sensor ADC sampling data
 */
static bool do_rtd_sensor_adc_sampling(uint32_t rtd_config_id, uint8_t chn,
				       int32_t (*adc_raw)[MAX_ADC_SAMPLES], sensor_measurement_type measurement_type,
				       bool multiple_3wire_rtd_enabled)
{
	bool adc_sampling_status = true;
	uint8_t setup =  ad7124_get_channel_setup(p_ad7124_dev, chn);

	do {
		/* Apply previous calibration coefficients while performing new measurement  */
		if (adc_calibration_config.adc_calibration_done) {
			ad7124_register_map[AD7124_Gain_0 + setup].value =
				adc_calibration_config.gain_after_calib[chn];
			if (ad7124_write_register(p_ad7124_dev,
						  ad7124_register_map[AD7124_Gain_0 + setup]) != 0) {
				adc_sampling_status = false;
				break;
			}

			ad7124_register_map[AD7124_Offset_0 + setup].value =
				adc_calibration_config.offset_after_calib[chn];
			if (ad7124_write_register(p_ad7124_dev,
						  ad7124_register_map[AD7124_Offset_0 + setup]) != 0) {
				adc_sampling_status = false;
				break;
			}
		}

		select_rtd_excitation_sources(true, rtd_config_id, chn,
					      multiple_3wire_rtd_enabled);

		if (ad7124_write_register(p_ad7124_dev,
					  ad7124_register_map[AD7124_IOCon1]) != 0) {
			adc_sampling_status = false;
			break;
		}

		if (perform_adc_conversion(chn, adc_raw, measurement_type) != 0) {
			adc_sampling_status = false;
			break;
		}

		select_rtd_excitation_sources(false, rtd_config_id, chn,
					      multiple_3wire_rtd_enabled);
	} while (0);

	return adc_sampling_status;
}


/*!
 * @brief	Perform the multiple RTD sensors measurement
 * @param	rtd_config_id[in]- RTD type (2/3/4-wire)
 * @param	measurement_type[in] - Temperature measurement and display type
 * @return	MENU_CONTINUE
 */
static int32_t perform_rtd_measurement(uint32_t rtd_config_id,
				       sensor_measurement_type measurement_type)
{
	bool adc_error = false;
	bool multiple_3wire_rtd_enabled = false;
	uint8_t rtd_gain;
	uint16_t sample_cnt;
	bool continue_measurement = false;
	float temperature;

	if (measurement_type == CONTINUOUS_MEASUREMENT) {
		printf(EOL"Press ESC key once to stop measurement..." EOL);
		no_os_mdelay(1000);
		continue_measurement = true;
	}

	/* Print display header */
	printf(EOL EOL EOL);
	for (uint8_t chn = SENSOR_CHANNEL0;
	     chn < max_supported_sensors[rtd_config_id]; chn++) {
		if (sensor_enable_status[chn]) {
			printf("\tRTD%d   ", chn+1);
		}
	}
	printf(EOL "\t-----------------------------------------------" EOL EOL);

	/* Perform additional configs for 3-wire RTD measurement */
	if (rtd_config_id == AD7124_CONFIG_3WIRE_RTD) {
		if (do_3wire_rtd_configs(&multiple_3wire_rtd_enabled) != 0)
			adc_error = true;
	}

	do {
		/* Sample and Read all enabled NTC channels in sequence */
		for (uint8_t chn = SENSOR_CHANNEL0;
		     (chn < max_supported_sensors[rtd_config_id]) & !adc_error; chn++) {
			if (sensor_enable_status[chn]) {
				if (!do_rtd_sensor_adc_sampling(rtd_config_id, chn, &n_sample_data[chn],
								measurement_type, multiple_3wire_rtd_enabled)) {
					adc_error = true;
					break;
				}
			}
		}

		if (adc_error) {
			printf(EOL EOL "\tError Performing Measurement" EOL);
			break;
		} else {
			if (multiple_3wire_rtd_enabled) {
				/* Store the Iout ratio as 1 (assumption is Iout0=Iout1) and no
				 * Iout calibration is performed */
				store_rtd_calibrated_iout_ratio(1, true);
				rtd_gain = MULTI_3WIRE_RTD_GAIN;
			} else {
				rtd_gain = RTD_2WIRE_GAIN_VALUE;
			}

			/* Calculate temperature and display result */
			if (measurement_type == AVERAGED_MEASUREMENT) {
				for (uint8_t chn = SENSOR_CHANNEL0; chn < max_supported_sensors[rtd_config_id];
				     chn++) {
					if (sensor_enable_status[chn]) {
						temperature = get_rtd_temperature(n_sample_data[chn][0], rtd_gain);
						sprintf(decimal_eqv_str, "%.4f  ", temperature);
						strcat(decimal_eqv_str_arr, decimal_eqv_str);
					}
				}
				printf("\t%s" EOL EOL, decimal_eqv_str_arr);
				decimal_eqv_str_arr[0] = '\0';
			} else {
				for (sample_cnt = 0; sample_cnt < MAX_ADC_SAMPLES; sample_cnt++) {
					for (uint8_t chn = SENSOR_CHANNEL0; chn < max_supported_sensors[rtd_config_id];
					     chn++) {
						if (sensor_enable_status[chn]) {
							temperature = get_rtd_temperature(n_sample_data[chn][sample_cnt], rtd_gain);
							sprintf(decimal_eqv_str, "%.4f  ", temperature);
							strcat(decimal_eqv_str_arr, decimal_eqv_str);
						}
					}
					printf("\t%s" EOL EOL, decimal_eqv_str_arr);
					decimal_eqv_str_arr[0] = '\0';
				}
			}
		}
	} while (continue_measurement && !was_escape_key_pressed());

	if (multiple_3wire_rtd_enabled) {
		/* Reset the calibration constant value after measurement */
		store_rtd_calibrated_iout_ratio(1, false);
	}

	/* Put ADC into standby mode */
	ad7124_register_map[AD7124_ADC_Control].value &= (~AD7124_ADC_CTRL_REG_MSK);
	ad7124_register_map[AD7124_ADC_Control].value |= AD7124_ADC_CTRL_REG_MODE(
				STANDBY_MODE);
	ad7124_write_register(p_ad7124_dev, ad7124_register_map[AD7124_ADC_Control]);

	adi_press_any_key_to_continue();
	return MENU_CONTINUE;
}


/*!
 * @brief	Perform the 2-wire RTD measurement
 * @param	measurement_type[in] - Temperature measurement and display type
 * @return	MENU_CONTINUE
 */
static int32_t perform_2wire_rtd_measurement(uint32_t measurement_type)
{
	return perform_rtd_measurement(AD7124_CONFIG_2WIRE_RTD, measurement_type);
}


/*!
 * @brief	Perform the 3-wire RTD measurement
 * @param	measurement_type[in] - Temperature measurement and display type
 * @return	MENU_CONTINUE
 */
static int32_t perform_3wire_rtd_measurement(uint32_t measurement_type)
{
	return perform_rtd_measurement(AD7124_CONFIG_3WIRE_RTD, measurement_type);
}


/*!
 * @brief	Perform the 4-wire RTD measurement
 * @param	measurement_type[in] - Temperature measurement and display type
 * @return	MENU_CONTINUE
 */
static int32_t perform_4wire_rtd_measurement(uint32_t measurement_type)
{
	return perform_rtd_measurement(AD7124_CONFIG_4WIRE_RTD, measurement_type);
}


/*!
 * @brief	Change the 3-wire RTD calibration type to user selected type
 * @param	calibration_type[in]- 3-wire RTD calibration type
 * @return	MENU_CONTINUE
 */
static int32_t change_3wire_rtd_calibration_type(uint32_t calibration_type)
{
	rtd_3wire_calibration_type = calibration_type;
	return MENU_CONTINUE;
}


/*!
 * @brief	Perform the 3-wire RTD calibration and measurement
 * @param	measurement_type[in] - Temperature measurement and display type
 * @return	MENU_CONTINUE
 */
static int32_t calibrate_and_measure_3wire_rtd(uint32_t measurement_type)
{
	int32_t sample_data[2][MAX_ADC_SAMPLES];
	bool adc_error = false;
	float rtd_calib_iout_avg[2]; 	// Iout0+Iout1
	float iout_ratio;
	float temperature;
	float voltage;
	bool multiple_3wire_rtd_enabled = false;
	uint16_t sample_cnt;
	bool continue_measurement = false;
	uint8_t first_active_rtd = SENSOR_CHANNEL0;
	uint8_t setup;

	const uint8_t rtd_3wire_iout0_source[] = {
		RTD1_3WIRE_IOUT0, RTD2_3WIRE_IOUT0, RTD3_3WIRE_IOUT0, RTD4_3WIRE_IOUT0
	};

	const uint8_t rtd_3wire_iout1_source[] = {
		RTD1_3WIRE_IOUT1, RTD2_3WIRE_IOUT1, RTD3_3WIRE_IOUT1, RTD4_3WIRE_IOUT1
	};

	do {
		/* Perform additional configurations for 3-wire RTD */
		if (do_3wire_rtd_configs(&multiple_3wire_rtd_enabled) != 0) {
			printf(EOL EOL "\tError Performing Measurement" EOL);
			adc_error = true;
			break;
		}

		if (!multiple_3wire_rtd_enabled) {
			printf(EOL EOL
			       "\tError in calibration!! Calibration is recommended only when multiple RTDs are connected"
			       EOL);

			adc_error = true;
			break;
		}
	} while (0);

	if (adc_error) {
		adi_press_any_key_to_continue();
		return MENU_CONTINUE;
	}

	if (measurement_type == CONTINUOUS_MEASUREMENT) {
		printf(EOL"Press ESC key once to stop measurement..." EOL);
		no_os_mdelay(1000);
		continue_measurement = true;
	}

	do {
		/* Get the calibrated Iout current avg for measurement by excitation method */
		if (rtd_3wire_calibration_type == MEASURING_EXCITATION_CURRENT) {

			/* Get the first RTD active (user enabled) channel to calibrate Iout */
			for (uint8_t chn = SENSOR_CHANNEL0;
			     chn < max_supported_sensors[AD7124_CONFIG_3WIRE_RTD]; chn++) {
				if (sensor_enable_status[chn]) {
					first_active_rtd = chn;
					break;
				}
			}

			/* Enable and direct IOUT0 excitation current source */
			ad7124_register_map[AD7124_IOCon1].value |= (AD7124_IO_CTRL1_REG_IOUT_CH0(
						rtd_3wire_iout0_source[first_active_rtd]) | AD7124_IO_CTRL1_REG_IOUT0(
						RTD_IOUT0_250UA_EXC));

			if (ad7124_write_register(p_ad7124_dev,
						  ad7124_register_map[AD7124_IOCon1]) != 0) {
				adc_error = true;
				break;
			}

			/* Read adc averaged sample result for Iout0 excitation */
			if (perform_adc_conversion(RTD_3WIRE_REF_MEASUREMENT_CHN,
						   &n_sample_data[RTD_3WIRE_REF_MEASUREMENT_CHN],
						   AVERAGED_MEASUREMENT) != 0) {
				adc_error = true;
				break;
			}

			/* Get the equivalent ADC voltage */
			voltage = ad7124_convert_sample_to_voltage(p_ad7124_dev,
					RTD_3WIRE_REF_MEASUREMENT_CHN,
					n_sample_data[RTD_3WIRE_REF_MEASUREMENT_CHN][0]);

			rtd_calib_iout_avg[0] = (voltage / get_rtd_rref());

			/* Turn off the Iout0 excitation current */
			ad7124_register_map[AD7124_IOCon1].value &= ((~AD7124_IO_CTRL1_REG_IOUT0_MSK)
					& (~AD7124_IO_CTRL1_REG_IOUT_CH0_MSK));

			/* Enable and direct IOUT1 excitation current source */
			ad7124_register_map[AD7124_IOCon1].value |= (AD7124_IO_CTRL1_REG_IOUT_CH1(
						rtd_3wire_iout1_source[first_active_rtd]) | AD7124_IO_CTRL1_REG_IOUT1(
						RTD_IOUT1_250UA_EXC));

			if (ad7124_write_register(p_ad7124_dev,
						  ad7124_register_map[AD7124_IOCon1]) != 0) {
				adc_error = true;
				break;
			}

			/* Read adc averaged sample result for Iout1 excitation */
			if (perform_adc_conversion(RTD_3WIRE_REF_MEASUREMENT_CHN,
						   &n_sample_data[RTD_3WIRE_REF_MEASUREMENT_CHN],
						   AVERAGED_MEASUREMENT) != 0) {
				adc_error = true;
				break;
			}

			/* Get the equivalent ADC voltage */
			voltage = ad7124_convert_sample_to_voltage(p_ad7124_dev,
					RTD_3WIRE_REF_MEASUREMENT_CHN,
					n_sample_data[RTD_3WIRE_REF_MEASUREMENT_CHN][0]);

			rtd_calib_iout_avg[1] = (voltage / get_rtd_rref());

			/* Turn off the Iout1 excitation current */
			ad7124_register_map[AD7124_IOCon1].value &= ((~AD7124_IO_CTRL1_REG_IOUT1_MSK)
					& (~AD7124_IO_CTRL1_REG_IOUT_CH1_MSK));

			if (ad7124_write_register(p_ad7124_dev,
						  ad7124_register_map[AD7124_IOCon1]) != 0) {
				adc_error = true;
				break;
			}

			printf(EOL EOL "\tIout0: %f" EOL, rtd_calib_iout_avg[0]);
			printf("\tIout1: %f" EOL, rtd_calib_iout_avg[1]);

			iout_ratio = rtd_calib_iout_avg[1] / rtd_calib_iout_avg[0];
			printf("\tIout_ratio: %f" EOL, iout_ratio);
		}
	} while (0);

	/* Print display header */
	printf(EOL EOL);
	for (uint8_t chn = SENSOR_CHANNEL0;
	     chn < max_supported_sensors[AD7124_CONFIG_3WIRE_RTD]; chn++) {
		if (sensor_enable_status[chn]) {
			printf("\tRTD%d   ", chn + 1);
		}
	}
	printf(EOL "\t-----------------------------------------------" EOL EOL);

	do {
		/* Calibrate, Sample and Read all enabled RTD channels in sequence */
		for (uint8_t chn = SENSOR_CHANNEL0;
		     (chn < max_supported_sensors[AD7124_CONFIG_3WIRE_RTD]) && (!adc_error);
		     chn++) {
			if (sensor_enable_status[chn]) {
				if (rtd_3wire_calibration_type == MEASURING_EXCITATION_CURRENT) {
					/* Perform the ADC sampling on Iout calibrated RTD sensor channel */
					if (!do_rtd_sensor_adc_sampling(AD7124_CONFIG_3WIRE_RTD,
									chn, &n_sample_data[chn], measurement_type, multiple_3wire_rtd_enabled)) {
						adc_error = true;
						break;
					}
				} else {
					/* Calibration by Iout excitation chopping.
					 * Part1: Direct the Iout excitation currents */

					/* Apply previous calibration coefficients while performing new measurement  */
					if (adc_calibration_config.adc_calibration_done) {
						setup =  ad7124_get_channel_setup(p_ad7124_dev, chn);

						ad7124_register_map[AD7124_Gain_0 + setup].value =
							adc_calibration_config.gain_after_calib[chn];
						if (ad7124_write_register(p_ad7124_dev,
									  ad7124_register_map[AD7124_Gain_0 + setup]) != 0) {
							adc_error = true;
							break;
						}

						ad7124_register_map[AD7124_Offset_0 + setup].value =
							adc_calibration_config.offset_after_calib[chn];
						if (ad7124_write_register(p_ad7124_dev,
									  ad7124_register_map[AD7124_Offset_0 + setup]) != 0) {
							adc_error = true;
							break;
						}
					}

					/* Enable and direct IOUT0 excitation current source */
					ad7124_register_map[AD7124_IOCon1].value |= (AD7124_IO_CTRL1_REG_IOUT_CH0(
								rtd_3wire_iout0_source[chn]) | AD7124_IO_CTRL1_REG_IOUT0(
								RTD_IOUT0_250UA_EXC));

					/* Enable and direct IOUT1 excitation current source */
					ad7124_register_map[AD7124_IOCon1].value |= (AD7124_IO_CTRL1_REG_IOUT_CH1(
								rtd_3wire_iout1_source[chn]) | AD7124_IO_CTRL1_REG_IOUT1(
								RTD_IOUT1_250UA_EXC));

					if (ad7124_write_register(p_ad7124_dev,
								  ad7124_register_map[AD7124_IOCon1]) != 0) {
						adc_error = true;
						break;
					}

					/* Read adc averaged sample result for selected RTD sensor channel */
					if (perform_adc_conversion(chn, &sample_data[0],
								   measurement_type) != 0) {
						adc_error = true;
						break;
					}

					/* Reset Iout registers for loading new configs */
					ad7124_register_map[AD7124_IOCon1].value &= ((~AD7124_IO_CTRL1_REG_IOUT0_MSK)
							& (~AD7124_IO_CTRL1_REG_IOUT_CH0_MSK) & (~AD7124_IO_CTRL1_REG_IOUT1_MSK)
							& (~AD7124_IO_CTRL1_REG_IOUT_CH1_MSK));

					/* Part2: Swap the Iout excitation sources and direct currents */

					/* Enable and direct IOUT0 excitation current source */
					ad7124_register_map[AD7124_IOCon1].value |= (AD7124_IO_CTRL1_REG_IOUT_CH0(
								rtd_3wire_iout1_source[chn]) | AD7124_IO_CTRL1_REG_IOUT0(
								RTD_IOUT0_250UA_EXC));

					/* Enable and direct IOUT1 excitation current source */
					ad7124_register_map[AD7124_IOCon1].value |= (AD7124_IO_CTRL1_REG_IOUT_CH1(
								rtd_3wire_iout0_source[chn]) | AD7124_IO_CTRL1_REG_IOUT1(
								RTD_IOUT1_250UA_EXC));

					if (ad7124_write_register(p_ad7124_dev,
								  ad7124_register_map[AD7124_IOCon1]) != 0) {
						adc_error = true;
						break;
					}

					/* Read adc averaged sample result for selected RTD sensor channel */
					if (perform_adc_conversion(chn, &sample_data[1],
								   measurement_type) != 0) {
						adc_error = true;
						break;
					}

					/* Turn off the excitation currents */
					ad7124_register_map[AD7124_IOCon1].value &= ((~AD7124_IO_CTRL1_REG_IOUT0_MSK)
							& (~AD7124_IO_CTRL1_REG_IOUT_CH0_MSK) & (~AD7124_IO_CTRL1_REG_IOUT1_MSK)
							& (~AD7124_IO_CTRL1_REG_IOUT_CH1_MSK));

					if (ad7124_write_register(p_ad7124_dev,
								  ad7124_register_map[AD7124_IOCon1]) != 0) {
						adc_error = true;
						break;
					}

					/* Get ADC averaged result */
					if (measurement_type == AVERAGED_MEASUREMENT) {
						n_sample_data[chn][0] = (sample_data[0][0] + sample_data[1][0]) / 2;
					} else {
						for (sample_cnt = 0; sample_cnt < MAX_ADC_SAMPLES; sample_cnt++) {
							n_sample_data[chn][sample_cnt] = (sample_data[0][sample_cnt] +
											  sample_data[1][sample_cnt]) / 2;
						}
					}
				}
			}
		}

		if (adc_error) {
			printf(EOL EOL "\tError Performing Measurement" EOL);
			break;
		} else {
			if (rtd_3wire_calibration_type == MEASURING_EXCITATION_CURRENT) {
				store_rtd_calibrated_iout_ratio(iout_ratio, true);
			} else {
				/* Store the Iout ratio as 1 (assumption is Iout0=Iout1) and no
				 * Iout calibration is performed */
				store_rtd_calibrated_iout_ratio(1, true);
			}

			/* Calculate temperature and display result */
			if (measurement_type == AVERAGED_MEASUREMENT) {
				for (uint8_t chn = SENSOR_CHANNEL0;
				     chn < max_supported_sensors[AD7124_CONFIG_3WIRE_RTD];
				     chn++) {
					if (sensor_enable_status[chn]) {
						temperature = get_rtd_temperature(n_sample_data[chn][0], MULTI_3WIRE_RTD_GAIN);
						sprintf(decimal_eqv_str, "%.4f  ", temperature);
						strcat(decimal_eqv_str_arr, decimal_eqv_str);
					}
				}
				printf("\t%s" EOL EOL, decimal_eqv_str_arr);
				decimal_eqv_str_arr[0] = '\0';
			} else {
				for (sample_cnt = 0; sample_cnt < MAX_ADC_SAMPLES; sample_cnt++) {
					for (uint8_t chn = SENSOR_CHANNEL0;
					     chn < max_supported_sensors[AD7124_CONFIG_3WIRE_RTD];
					     chn++) {
						if (sensor_enable_status[chn]) {
							temperature = get_rtd_temperature(n_sample_data[chn][sample_cnt],
											  MULTI_3WIRE_RTD_GAIN);
							sprintf(decimal_eqv_str, "%.4f  ", temperature);
							strcat(decimal_eqv_str_arr, decimal_eqv_str);
						}
					}
					printf("\t%s" EOL EOL, decimal_eqv_str_arr);
					decimal_eqv_str_arr[0] = '\0';
				}
			}
		}
	} while (continue_measurement && !was_escape_key_pressed());

	/* Reset the calibration constant value after measurement */
	store_rtd_calibrated_iout_ratio(1, false);

	/* Put ADC into standby mode */
	ad7124_register_map[AD7124_ADC_Control].value &= (~AD7124_ADC_CTRL_REG_MSK);
	ad7124_register_map[AD7124_ADC_Control].value |= AD7124_ADC_CTRL_REG_MODE(
				STANDBY_MODE);
	ad7124_write_register(p_ad7124_dev, ad7124_register_map[AD7124_ADC_Control]);

	adi_press_any_key_to_continue();
	return MENU_CONTINUE;
}


/*!
 * @brief	Perform the multiple NTC thermistor sensors measurement
 * @param	measurement_type[in]- Temperature measurement and display type
 * @return	MENU_CONTINUE
 */
int32_t perform_ntc_thermistor_measurement(uint32_t measurement_type)
{
	bool adc_error = false;
	uint16_t sample_cnt;
	bool continue_measurement = false;
	float temperature;
	uint8_t setup;

	if (measurement_type == CONTINUOUS_MEASUREMENT) {
		printf(EOL"Press ESC key once to stop measurement..." EOL);
		no_os_mdelay(1000);
		continue_measurement = true;
	}

	/* Print display header */
	printf(EOL EOL EOL);
	for (uint8_t chn = SENSOR_CHANNEL0;
	     chn < max_supported_sensors[AD7124_CONFIG_THERMISTOR]; chn++) {
		if (sensor_enable_status[chn]) {
			printf("\tNTC%d   ", chn + 1);
		}
	}
	printf(EOL "\t-----------------------------------------------" EOL EOL);

	do {
		/* Sample and Read all enabled NTC channels in sequence */
		for (uint8_t chn = SENSOR_CHANNEL0;
		     chn < max_supported_sensors[AD7124_CONFIG_THERMISTOR];
		     chn++) {
			if (sensor_enable_status[chn]) {
				/* Apply previous calibration coefficients while performing new measurement  */
				if (adc_calibration_config.adc_calibration_done) {
					setup =  ad7124_get_channel_setup(p_ad7124_dev, chn);

					ad7124_register_map[AD7124_Gain_0 + setup].value =
						adc_calibration_config.gain_after_calib[chn];
					if (ad7124_write_register(p_ad7124_dev,
								  ad7124_register_map[AD7124_Gain_0 + setup]) != 0) {
						adc_error = true;
						break;
					}

					ad7124_register_map[AD7124_Offset_0 + setup].value =
						adc_calibration_config.offset_after_calib[chn];
					if (ad7124_write_register(p_ad7124_dev,
								  ad7124_register_map[AD7124_Offset_0 + setup]) != 0) {
						adc_error = true;
						break;
					}
				}

				if (perform_adc_conversion(chn, &n_sample_data[chn],
							   measurement_type) != 0) {
					adc_error = true;
					break;
				}
			}
		}

		if (adc_error) {
			printf(EOL EOL "\tError Performing Measurement" EOL);
			break;
		} else {
			/* Calculate temperature and display result */
			if (measurement_type == AVERAGED_MEASUREMENT) {
				for (uint8_t chn = SENSOR_CHANNEL0;
				     chn < max_supported_sensors[AD7124_CONFIG_THERMISTOR];
				     chn++) {
					if (sensor_enable_status[chn]) {
						temperature = get_ntc_thermistor_temperature(n_sample_data[chn][0]);
						sprintf(decimal_eqv_str, "%.4f  ", temperature);
						strcat(decimal_eqv_str_arr, decimal_eqv_str);
					}
				}
				printf("\t%s" EOL EOL, decimal_eqv_str_arr);
				decimal_eqv_str_arr[0] = '\0';
			} else {
				for (sample_cnt = 0; sample_cnt < MAX_ADC_SAMPLES; sample_cnt++) {
					for (uint8_t chn = SENSOR_CHANNEL0;
					     chn < max_supported_sensors[AD7124_CONFIG_THERMISTOR];
					     chn++) {
						if (sensor_enable_status[chn]) {
							temperature = get_ntc_thermistor_temperature(n_sample_data[chn][sample_cnt]);
							sprintf(decimal_eqv_str, "%.4f  ", temperature);
							strcat(decimal_eqv_str_arr, decimal_eqv_str);
						}
					}
					printf("\t%s" EOL EOL, decimal_eqv_str_arr);
					decimal_eqv_str_arr[0] = '\0';
				}
			}
		}
	} while (continue_measurement && !was_escape_key_pressed());

	/* Put ADC into standby mode */
	ad7124_register_map[AD7124_ADC_Control].value &= (~AD7124_ADC_CTRL_REG_MSK);
	ad7124_register_map[AD7124_ADC_Control].value |= AD7124_ADC_CTRL_REG_MODE(
				STANDBY_MODE);
	ad7124_write_register(p_ad7124_dev, ad7124_register_map[AD7124_ADC_Control]);

	adi_press_any_key_to_continue();
	return MENU_CONTINUE;
}


/*!
 * @brief	Perform CJC sensor configurations
 * @param	input_chn[in] - Channel mapped to CJC sensor
 * @return	0 in case of success, negative error code otherwise
 */
static int32_t do_cjc_configs(int32_t input_chn)
{
	int32_t iout0_input, iout_exc;
	int32_t gain;
	uint8_t setup;

	switch (current_cjc_sensor) {
	case PT100_4WIRE_RTD:
		iout0_input = CJC_RTD_IOUT0;
		iout_exc = CJC_RTD_IOUT0_EXC;
		gain = RTD_4WIRE_GAIN_VALUE;
		break;

	case PT1000_2WIRE_RTD:
		iout0_input = CJC_RTD_IOUT0;
		iout_exc = CJC_RTD_IOUT0_EXC;
		gain = RTD_PT1000_GAIN_VALUE;
		break;

	case THERMISTOR_PTC_KY81_110:
		iout0_input = CJC_PTC_THERMISTOR_IOUT0;
		iout_exc = CJC_PTC_THERMISTOR_IOUT0_EXC;
		gain = THERMISTOR_GAIN_VALUE;
		break;

	default:
		return -EINVAL;
	}

	setup = ad7124_get_channel_setup(p_ad7124_dev, input_chn);

	/* Set the gain corresponding to selected CJC sensor */
	ad7124_register_map[AD7124_Config_0 + setup].value &= (~AD7124_CFG_REG_PGA_MSK);
	ad7124_register_map[AD7124_Config_0 + setup].value |= AD7124_CFG_REG_PGA(gain);
	if (ad7124_write_register(p_ad7124_dev,
				  ad7124_register_map[AD7124_Config_0 + setup]) != 0) {
		return -EIO;
	}

	/* Enable and direct IOUT0 excitation current source for CJ sensor measurement */
	ad7124_register_map[AD7124_IOCon1].value |= (AD7124_IO_CTRL1_REG_IOUT_CH0(
				iout0_input) | AD7124_IO_CTRL1_REG_IOUT0(iout_exc));
	if (ad7124_write_register(p_ad7124_dev,
				  ad7124_register_map[AD7124_IOCon1]) != 0) {
		return -EIO;
	}

	return 0;
}


/*!
 * @brief	Perform the cold junction compensation (CJC) measurement
 * @param	*data[out]- Pointer to array to read data into
 * @param	measurement_type[in]- Temperature measurement and display type
 * @return	0 in case of success, negative error code otherwise
 * @note	Both CJCs uses similar excitation and ratiometric measurement
 *			logic
 */
int32_t perform_cjc_measurement(int32_t (*data)[MAX_ADC_SAMPLES],
				sensor_measurement_type measurement_type)
{
	int32_t input_chn;
	uint8_t setup;

	switch (current_cjc_sensor) {
	case PT100_4WIRE_RTD:
		input_chn = CJC_RTD_CHN;
		break;

	case PT1000_2WIRE_RTD:
		input_chn = CJC_RTD_CHN;
		break;

	case THERMISTOR_PTC_KY81_110:
		input_chn = CJC_THERMISTOR_CHN;
		break;

	default:
		return -EINVAL;
	}

	/* Perform CJC configurations */
	if (do_cjc_configs(input_chn) != 0) {
		return -EIO;
	}

	/* Apply previous calibration coefficients while performing new measurement  */
	if (adc_calibration_config.adc_calibration_done) {
		setup =  ad7124_get_channel_setup(p_ad7124_dev, input_chn);

		ad7124_register_map[AD7124_Gain_0 + setup].value =
			adc_calibration_config.gain_after_calib[input_chn];
		if (ad7124_write_register(p_ad7124_dev,
					  ad7124_register_map[AD7124_Gain_0 + setup]) != 0) {
			return -EIO;
		}

		ad7124_register_map[AD7124_Offset_0 + setup].value =
			adc_calibration_config.offset_after_calib[input_chn];
		if (ad7124_write_register(p_ad7124_dev,
					  ad7124_register_map[AD7124_Offset_0 + setup]) != 0) {
			return -EIO;
		}
	}

	if (perform_adc_conversion(input_chn, data, measurement_type) != 0) {
		return -EIO;
	}

	/* Turn off the excitation current */
	ad7124_register_map[AD7124_IOCon1].value &= ((~AD7124_IO_CTRL1_REG_IOUT0_MSK)
			& (~AD7124_IO_CTRL1_REG_IOUT_CH0_MSK));
	if (ad7124_write_register(p_ad7124_dev,
				  ad7124_register_map[AD7124_IOCon1]) != 0) {
		return -EIO;
	}

	return 0;
}


/*!
 * @brief	Perform the multiple thermocouple sensors measurement
 * @param	measurement_type[in]- Temperature measurement and display type
 * @return	MENU_CONTINUE
 */
int32_t perform_thermocouple_measurement(uint32_t measurement_type)
{
	bool adc_error = false;
	uint8_t setup;
	uint16_t sample_cnt;
	bool continue_measurement = false;
	float tc_temperature;
	float cjc_temperature;

#if defined(AD7124_8)
	const int32_t tc_vbias_input[] = {
		AD7124_8_IO_CTRL2_REG_GPIO_VBIAS2, AD7124_8_IO_CTRL2_REG_GPIO_VBIAS6,
		AD7124_8_IO_CTRL2_REG_GPIO_VBIAS8, AD7124_8_IO_CTRL2_REG_GPIO_VBIAS10,
		AD7124_8_IO_CTRL2_REG_GPIO_VBIAS12, AD7124_8_IO_CTRL2_REG_GPIO_VBIAS14
	};
#else
	const int32_t tc_vbias_input[] = {
		AD7124_IO_CTRL2_REG_GPIO_VBIAS2, AD7124_IO_CTRL2_REG_GPIO_VBIAS6
	};
#endif

	if (measurement_type == CONTINUOUS_MEASUREMENT) {
		printf(EOL"Press ESC key once to stop measurement..." EOL);
		no_os_mdelay(1000);
		continue_measurement = true;
	}

	/* Print display header */
	printf(EOL EOL EOL);
	for (uint8_t chn = SENSOR_CHANNEL0;
	     chn < max_supported_sensors[AD7124_CONFIG_THERMOCOUPLE]; chn++) {
		if (sensor_enable_status[chn]) {
			sprintf(decimal_eqv_str, "TC%d  CJC   ", chn+1);
			strcat(decimal_eqv_str_arr, decimal_eqv_str);
		}
	}
	printf("\t%s" EOL EOL, decimal_eqv_str_arr);
	decimal_eqv_str_arr[0] = '\0';
	printf("\t----------------------------------------------------------------------------------------------"
	       EOL EOL);

	do {
		/* Sample and Read all enabled TC channels in sequence */
		for (uint8_t chn = SENSOR_CHANNEL0;
		     chn < max_supported_sensors[AD7124_CONFIG_THERMOCOUPLE];
		     chn++) {
			if (sensor_enable_status[chn]) {
				setup =  ad7124_get_channel_setup(p_ad7124_dev, chn);

				/* Apply previous calibration coefficients while performing new measurement  */
				if (adc_calibration_config.adc_calibration_done) {
					ad7124_register_map[AD7124_Gain_0 + setup].value =
						adc_calibration_config.gain_after_calib[chn];
					if (ad7124_write_register(p_ad7124_dev,
								  ad7124_register_map[AD7124_Gain_0 + setup]) != 0) {
						adc_error = true;
						break;
					}

					ad7124_register_map[AD7124_Offset_0 + setup].value =
						adc_calibration_config.offset_after_calib[chn];
					if (ad7124_write_register(p_ad7124_dev,
								  ad7124_register_map[AD7124_Offset_0 + setup]) != 0) {
						adc_error = true;
						break;
					}
				}

				/* Turn on the bias voltage for current thermocouple input (AINP) */
				ad7124_register_map[AD7124_IOCon2].value |= tc_vbias_input[chn];
				if (ad7124_write_register(p_ad7124_dev,
							  ad7124_register_map[AD7124_IOCon2]) != 0) {
					adc_error = true;
					break;
				}

				if (perform_adc_conversion(chn, &n_sample_data[chn],
							   measurement_type) != 0) {
					adc_error = true;
					break;
				}

				/* Turn off the bias voltage for all analog inputs */
				ad7124_register_map[AD7124_IOCon2].value = 0x0;
				if (ad7124_write_register(p_ad7124_dev,
							  ad7124_register_map[AD7124_IOCon2]) != 0) {
					adc_error = true;
					break;
				}

				/* Perform measurement for the cold junction compensation sensor */
				if (perform_cjc_measurement(&n_cjc_sample_data[chn],
							    measurement_type) != 0) {
					adc_error = true;
					break;
				}

				/* Change gain back to thermocouple sensor gain */
				ad7124_register_map[AD7124_Config_0 + setup].value &= (~AD7124_CFG_REG_PGA_MSK);
				ad7124_register_map[AD7124_Config_0 + setup].value |= AD7124_CFG_REG_PGA(
							THERMOCOUPLE_GAIN_VALUE);
				if (ad7124_write_register(p_ad7124_dev,
							  ad7124_register_map[AD7124_Config_0 + setup]) != 0) {
					adc_error = true;
					break;
				}
			}
		}

		if (adc_error) {
			printf(EOL EOL "\tError Performing Measurement" EOL);
			break;
		} else {
			/* Calculate temperature and display result */
			if (measurement_type == AVERAGED_MEASUREMENT) {
				for (uint8_t chn = SENSOR_CHANNEL0;
				     chn < max_supported_sensors[AD7124_CONFIG_THERMOCOUPLE];
				     chn++) {
					if (sensor_enable_status[chn]) {
						tc_temperature = get_tc_temperature(n_sample_data[chn][0],
										    n_cjc_sample_data[chn][0], current_cjc_sensor,
										    &cjc_temperature);
						sprintf(decimal_eqv_str, "%.4f  %.4f   ", tc_temperature, cjc_temperature);
						strcat(decimal_eqv_str_arr, decimal_eqv_str);
					}
				}
				printf("\t%s" EOL EOL, decimal_eqv_str_arr);
				decimal_eqv_str_arr[0] = '\0';
			} else {
				for (sample_cnt = 0; sample_cnt < MAX_ADC_SAMPLES; sample_cnt++) {
					for (uint8_t chn = SENSOR_CHANNEL0;
					     chn < max_supported_sensors[AD7124_CONFIG_THERMOCOUPLE];
					     chn++) {
						if (sensor_enable_status[chn]) {
							tc_temperature = get_tc_temperature(n_sample_data[chn][sample_cnt],
											    n_cjc_sample_data[chn][sample_cnt], current_cjc_sensor,
											    &cjc_temperature);
							sprintf(decimal_eqv_str, "%.4f  %.4f   ", tc_temperature, cjc_temperature);
							strcat(decimal_eqv_str_arr, decimal_eqv_str);
						}
					}
					printf("\t%s" EOL EOL, decimal_eqv_str_arr);
					decimal_eqv_str_arr[0] = '\0';
				}
			}
		}
	} while (continue_measurement && !was_escape_key_pressed());

	/* Put ADC into standby mode */
	ad7124_register_map[AD7124_ADC_Control].value &= (~AD7124_ADC_CTRL_REG_MSK);
	ad7124_register_map[AD7124_ADC_Control].value |= AD7124_ADC_CTRL_REG_MODE(
				STANDBY_MODE);
	ad7124_write_register(p_ad7124_dev, ad7124_register_map[AD7124_ADC_Control]);

	adi_press_any_key_to_continue();
	return MENU_CONTINUE;
}


/*!
 * @brief	Perform the device configurations required for ADC calibration
 * @return	adc calibration configuration status
 */
static int32_t do_adc_calibration_configs(void)
{
	int32_t adc_config_status = 0;

	do {
		/* Put ADC into standby mode */
		ad7124_register_map[AD7124_ADC_Control].value &= (~AD7124_ADC_CTRL_REG_MSK);
		ad7124_register_map[AD7124_ADC_Control].value |= AD7124_ADC_CTRL_REG_MODE(
					STANDBY_MODE);

		/* Get ADC power mode status for previous config */
		adc_calibration_config.power_mode = AD7124_ADC_CTRL_REG_POWER_MODE_RD(
				ad7124_register_map[AD7124_ADC_Control].value);

		/* Select low power ADC mode for ADC calibration */
		ad7124_register_map[AD7124_ADC_Control].value &=
			(~AD7124_ADC_CTRL_REG_POWER_MODE_MSK);
		ad7124_register_map[AD7124_ADC_Control].value |= AD7124_ADC_CTRL_REG_POWER_MODE(
					ADC_CALIBRATION_PWR_MODE);

		if (ad7124_write_register(p_ad7124_dev,
					  ad7124_register_map[AD7124_ADC_Control]) != 0) {
			adc_config_status = -EIO;
			break;
		}
	} while (0);

	return adc_config_status;
}


/*!
 * @brief	Reset the ADC configuration to previous demo mode configuration
 * @return	none
 */
static void reset_adc_calibration_configs(void)
{
	/* Put ADC into standby mode */
	ad7124_register_map[AD7124_ADC_Control].value &= (~AD7124_ADC_CTRL_REG_MSK);
	ad7124_register_map[AD7124_ADC_Control].value |= AD7124_ADC_CTRL_REG_MODE(
				STANDBY_MODE);

	/* Reset ADC power mode */
	ad7124_register_map[AD7124_ADC_Control].value &=
		(~AD7124_ADC_CTRL_REG_POWER_MODE_MSK);
	ad7124_register_map[AD7124_ADC_Control].value |= AD7124_ADC_CTRL_REG_POWER_MODE(
				adc_calibration_config.power_mode);

	ad7124_write_register(p_ad7124_dev,
			      ad7124_register_map[AD7124_ADC_Control]);
}


/*!
 * @brief	Perform the ADC calibration on selected channel
 * @param	calibration_mode[in] - ADC calibration mode
 * @param	chn[in] - ADC channel to be calibrated
 * @param	setup[in] - Setup mapped to selected ADC channel
 * @param	pos_analog_input[in] - Positive analog input mapped to selected ADC channel
 * @param	neg_analog_input[in] - Negative analog input mapped to selected ADC channel
 * @return	adc calibration status
 */
static int32_t do_adc_calibration(uint32_t calibration_mode, uint8_t chn,
				  uint8_t setup,
				  uint8_t pos_analog_input, uint8_t neg_analog_input)
{
	int32_t calibration_status = 0;
	uint8_t pga = AD7124_PGA_GAIN(ad7124_get_channel_pga(p_ad7124_dev, chn));

	do {
		if ((calibration_mode == INTERNAL_FULL_SCALE_CALIBRATE_MODE)
		    || (calibration_mode == INTERNAL_ZERO_SCALE_CALIBRATE_MODE)) {
			if (calibration_mode == INTERNAL_FULL_SCALE_CALIBRATE_MODE) {
				/* Write default offset register value before starting full-scale internal calibration */
				ad7124_register_map[AD7124_Offset_0 + setup].value =
					AD7124_DEFAULT_OFFSET;
				if (ad7124_write_register(p_ad7124_dev,
							  ad7124_register_map[AD7124_Offset_0 + setup]) != 0) {
					calibration_status = -EIO;
					break;
				}

				/* Don't continue further internal full-scale calibration at gain of 1 */
				if (pga == 1) {
					printf("\tDevice does not support internal full-scale calibration at Gain of 1!!"
					       EOL);
					break;
				} else {
					printf("\tRunning internal full-scale (gain) calibration..." EOL);
				}
			} else {
				printf("\tRunning internal zero-scale (offset) calibration..." EOL);
			}
		} else {
			if (calibration_mode == SYSTEM_FULL_SCALE_CALIBRATE_MODE) {
				printf(EOL
				       "\tApply full-scale voltage between AINP%d and AINM%d and press any key..."
				       EOL,
				       pos_analog_input,
				       neg_analog_input);
			} else {
				printf(EOL
				       "\tApply zero-scale voltage between AINP%d and AINM%d and press any key..."
				       EOL,
				       pos_analog_input,
				       neg_analog_input);
			}

			/* Wait for user input */
			getchar();
		}

		/* Get setup/configuration mapped to corresponding channel */
		setup = AD7124_CH_MAP_REG_SETUP_RD(
				ad7124_register_map[AD7124_Channel_0 + chn].value);

		if ((calibration_mode == INTERNAL_FULL_SCALE_CALIBRATE_MODE)
		    || (calibration_mode == SYSTEM_FULL_SCALE_CALIBRATE_MODE)) {
			/* Read the gain coefficient value */
			if (ad7124_read_register(p_ad7124_dev,
						 &ad7124_register_map[AD7124_Gain_0 + setup]) != 0) {
				calibration_status = -EIO;
				break;
			}
			adc_calibration_config.gain_before_calib[chn] =
				ad7124_register_map[AD7124_Gain_0 +
								  setup].value;
		}

		if ((calibration_mode == INTERNAL_ZERO_SCALE_CALIBRATE_MODE)
		    || (calibration_mode == SYSTEM_ZERO_SCALE_CALIBRATE_MODE)) {
			/* Read the offset coefficient value */
			if (ad7124_read_register(p_ad7124_dev,
						 &ad7124_register_map[AD7124_Offset_0 + setup]) != 0) {
				calibration_status = -EIO;
				break;
			}
			adc_calibration_config.offset_before_calib[chn] =
				ad7124_register_map[AD7124_Offset_0 +
								    setup].value;
		}

		ad7124_register_map[AD7124_ADC_Control].value =
			((ad7124_register_map[AD7124_ADC_Control].value & ~AD7124_ADC_CTRL_REG_MSK) | \
			 AD7124_ADC_CTRL_REG_MODE(calibration_mode));

		if (ad7124_write_register(p_ad7124_dev,
					  ad7124_register_map[AD7124_ADC_Control]) != 0) {
			calibration_status = -EIO;
			break;
		}

		/* Let the channel settle */
		no_os_mdelay(100);

		/* Wait for calibration (conversion) to finish */
		if (ad7124_wait_for_conv_ready(p_ad7124_dev,
					       p_ad7124_dev->spi_rdy_poll_cnt) != 0) {
			calibration_status = -EIO;
			break;
		}
	} while (0);

	return calibration_status;
}


/*!
 * @brief	Perform the AD7124 internal/system calibration
 * @param	calibration_type[in]- ADC calibration type (internal/system)
 * @return	MENU_CONTINUE
 * @note	This function performs both 'Internal/System Full-Scale' and
 *			'Internal/System Zero-Scale' calibration on all enabled ADC channels
 *			sequentially.
 */
int32_t perform_adc_calibration(uint32_t calibration_type)
{
	bool adc_error = false;
	uint8_t chn_cnt;
	uint8_t pos_analog_input, neg_analog_input;
	uint8_t setup;
	uint8_t pga;

	/* Load ADC configurations and perform the calibration */
	if (do_adc_calibration_configs() == 0) {
		/* Calibrate all the user enabled ADC channels sequentially */
		for (chn_cnt = 0; chn_cnt < NUM_OF_SENSOR_CHANNELS; chn_cnt++) {
			if (sensor_enable_status[chn_cnt]) {
				/* Read the channel map register */
				if (ad7124_read_register(p_ad7124_dev,
							 &ad7124_register_map[AD7124_Channel_0 + chn_cnt]) != 0) {
					adc_error = true;
					break;
				}

				/* Get the analog inputs mapped to corresponding channel */
				pos_analog_input = AD7124_CH_MAP_REG_AINP_RD(
							   ad7124_register_map[AD7124_Channel_0 + chn_cnt].value);
				neg_analog_input = AD7124_CH_MAP_REG_AINM_RD(
							   ad7124_register_map[AD7124_Channel_0 + chn_cnt].value);

				/* Make sure analog input number mapped to channel is correct */
				if (pos_analog_input > AD7124_MAX_INPUTS
				    || neg_analog_input > AD7124_MAX_INPUTS) {
					continue;
				}

				/* Get setup/configuration mapped to corresponding channel */
				setup = AD7124_CH_MAP_REG_SETUP_RD(
						ad7124_register_map[AD7124_Channel_0 + chn_cnt].value);

				/* Get the programmable gain mapped to corresponding channels setup */
				pga = AD7124_PGA_GAIN(ad7124_get_channel_pga(p_ad7124_dev, chn_cnt));

				printf(EOL "Calibrating Channel %d => " EOL, chn_cnt);

				/* Enable channel for calibration */
				ad7124_register_map[AD7124_Channel_0 + chn_cnt].value |=
					AD7124_CH_MAP_REG_CH_ENABLE;
				if (ad7124_write_register(p_ad7124_dev,
							  ad7124_register_map[AD7124_Channel_0 + chn_cnt]) != 0) {
					adc_error = true;
					break;
				}

				if ((current_sensor_config_id == AD7124_CONFIG_2WIRE_RTD) ||
				    (current_sensor_config_id == AD7124_CONFIG_3WIRE_RTD) ||
				    (current_sensor_config_id == AD7124_CONFIG_4WIRE_RTD)) {
					/* Enable the Iout source on channel */
					select_rtd_excitation_sources(true,
								      current_sensor_config_id,
								      chn_cnt,
								      true);
				} else if (current_sensor_config_id == AD7124_CONFIG_THERMOCOUPLE) {
					if ((chn_cnt == CJC_RTD_CHN) || (chn_cnt == CJC_THERMISTOR_CHN)) {
						do_cjc_configs(chn_cnt);
					}
				} else {
					/* do nothing */
				}

				if (calibration_type == INTERNAL_CALIBRATION) {
					/* Perform the internal full-scale (gain) calibration */
					if (do_adc_calibration(INTERNAL_FULL_SCALE_CALIBRATE_MODE,
							       chn_cnt,
							       setup,
							       pos_analog_input,
							       neg_analog_input) != 0) {
						adc_error = true;
						break;
					}

					/* Read the gain coefficient value (post calibrated) */
					ad7124_read_register(p_ad7124_dev,
							     &ad7124_register_map[AD7124_Gain_0 + setup]);
					adc_calibration_config.gain_after_calib[chn_cnt] =
						ad7124_register_map[AD7124_Gain_0 +
										  setup].value;

					/* Perform the internal zero-scale (offset) calibration */
					if (do_adc_calibration(INTERNAL_ZERO_SCALE_CALIBRATE_MODE,
							       chn_cnt,
							       setup,
							       pos_analog_input,
							       neg_analog_input) != 0) {
						adc_error = true;
						break;
					}

					/* Read the offset coefficient value (post calibrated) */
					ad7124_read_register(p_ad7124_dev,
							     &ad7124_register_map[AD7124_Offset_0 + setup]);
					adc_calibration_config.offset_after_calib[chn_cnt] =
						ad7124_register_map[AD7124_Offset_0 +
										    setup].value;

					/* Compare the pre and post adc calibration gain coefficients to check calibration status */
					if (pga > 1) {
						if (adc_calibration_config.gain_after_calib[chn_cnt] !=
						    adc_calibration_config.gain_before_calib[chn_cnt]) {
							printf("\tGain %d: 0x%lx" EOL,
							       setup,
							       adc_calibration_config.gain_after_calib[chn_cnt]);
						} else {
							printf(EOL "\tError in internal full-scale (gain) calibration!!" EOL);
							adc_calibration_config.gain_after_calib[chn_cnt] =
								adc_calibration_config.gain_before_calib[chn_cnt];
						}
					}

					/* Compare the pre and post adc calibration offset coefficients to check calibration status */
					if (adc_calibration_config.offset_after_calib[chn_cnt] !=
					    adc_calibration_config.offset_before_calib[chn_cnt]) {
						printf("\tOffset %d: 0x%lx" EOL,
						       setup,
						       adc_calibration_config.offset_after_calib[chn_cnt]);

					} else {
						printf(EOL "\tError in internal zero-scale (offset) calibration!!" EOL);
						adc_calibration_config.offset_after_calib[chn_cnt] =
							adc_calibration_config.offset_before_calib[chn_cnt];
					}
				} else {
					/* Perform the system zero-scale (offset) calibration */
					if (do_adc_calibration(SYSTEM_ZERO_SCALE_CALIBRATE_MODE,
							       chn_cnt,
							       setup,
							       pos_analog_input,
							       neg_analog_input) != 0) {
						adc_error = true;
						break;
					}

					/* Read the offset coefficient value (post calibrated) */
					ad7124_read_register(p_ad7124_dev,
							     &ad7124_register_map[AD7124_Offset_0 + setup]);
					adc_calibration_config.offset_after_calib[chn_cnt] =
						ad7124_register_map[AD7124_Offset_0 +
										    setup].value;

					/* Compare the pre and post adc calibration offset coefficients to detect calibration error */
					if (adc_calibration_config.offset_after_calib[chn_cnt] !=
					    adc_calibration_config.offset_before_calib[chn_cnt]) {
						printf("\tOffset %d: 0x%lx" EOL,
						       setup,
						       adc_calibration_config.offset_after_calib[chn_cnt]);
					} else {
						printf(EOL "\tError in system zero-scale (offset) calibration!!" EOL);
						adc_calibration_config.offset_after_calib[chn_cnt] =
							adc_calibration_config.offset_before_calib[chn_cnt];
					}

					/* Perform the system full-scale (gain) calibration */
					if (do_adc_calibration(SYSTEM_FULL_SCALE_CALIBRATE_MODE,
							       chn_cnt,
							       setup,
							       pos_analog_input,
							       neg_analog_input) != 0) {
						adc_error = true;
						break;
					}

					/* Read the gain coefficient value (post calibrated) */
					ad7124_read_register(p_ad7124_dev,
							     &ad7124_register_map[AD7124_Gain_0 + setup]);
					adc_calibration_config.gain_after_calib[chn_cnt] =
						ad7124_register_map[AD7124_Gain_0 +
										  setup].value;

					/* Compare the pre and post adc calibration gain coefficients to detect calibration error */
					if (adc_calibration_config.gain_after_calib[chn_cnt] !=
					    adc_calibration_config.gain_before_calib[chn_cnt]) {
						printf("\tGain %d: 0x%lx" EOL,
						       setup,
						       adc_calibration_config.gain_after_calib[chn_cnt]);
					} else {
						printf(EOL "\tError in system full-scale (gain) calibration!!" EOL);
						adc_calibration_config.gain_after_calib[chn_cnt] =
							adc_calibration_config.gain_before_calib[chn_cnt];
					}
				}

				if ((current_sensor_config_id == AD7124_CONFIG_2WIRE_RTD) ||
				    (current_sensor_config_id == AD7124_CONFIG_3WIRE_RTD) ||
				    (current_sensor_config_id == AD7124_CONFIG_4WIRE_RTD)) {
					/* Disable the Iout source on RTD channel */
					select_rtd_excitation_sources(false,
								      current_sensor_config_id,
								      chn_cnt,
								      true);
				} else {
					/* Turn off the Iout0 excitation current */
					ad7124_register_map[AD7124_IOCon1].value &= ((~AD7124_IO_CTRL1_REG_IOUT0_MSK)
							& (~AD7124_IO_CTRL1_REG_IOUT_CH0_MSK));
					ad7124_write_register(p_ad7124_dev, ad7124_register_map[AD7124_IOCon1]);
				}

				/* Disable current channel */
				ad7124_register_map[AD7124_Channel_0 + chn_cnt].value &=
					(~AD7124_CH_MAP_REG_CH_ENABLE);
				if (ad7124_write_register(p_ad7124_dev,
							  ad7124_register_map[AD7124_Channel_0 + chn_cnt]) != 0) {
					adc_error = true;
					break;
				}

				if (!adc_error) {
					printf(EOL "\tCalibration done..." EOL);
				}
			}
		}

		if (adc_error) {
			adc_calibration_config.adc_calibration_done = false;
		} else {
			adc_calibration_config.adc_calibration_done = true;
		}
	} else {
		printf(EOL "\tError in calibration!!" EOL);
		adc_calibration_config.adc_calibration_done = false;
	}

	/* Reset the ADC configs to previously enabled config to apply calibration
	 * offset and gain coefficients */
	reset_adc_calibration_configs();

	adi_press_any_key_to_continue();
	adi_clear_console();

	return MENU_CONTINUE;
}


/*!
 * @brief	Display header information for 2-wire RTD measurement menu
 * @return	none
 */
void rtd_2wire_menu_header(void)
{
	if (strcmp(current_sensor_config, sensor_configs[AD7124_CONFIG_2WIRE_RTD])) {
		/* Disable unused sensor channels */
		for (uint8_t chn = max_supported_sensors[AD7124_CONFIG_2WIRE_RTD];
		     chn < NUM_OF_SENSOR_CHANNELS; chn++) {
			sensor_enable_status[chn] = false;
		}

		/* Load the 2-wire RTD device configuration */
		init_with_configuration(AD7124_CONFIG_2WIRE_RTD);
	}

	printf("\t Sensor  Channel   IOUT0   AIN+    AIN-   Enable" EOL);
	printf("\t -----------------------------------------------" EOL);
	printf("\t  RTD1      %d      AIN%d    AIN%d    AIN%d     %c"
	       EOL,
	       SENSOR_CHANNEL0, RTD1_2WIRE_IOUT0, RTD1_2WIRE_AINP, RTD1_2WIRE_AINM,
	       status_info[sensor_enable_status[SENSOR_CHANNEL0]]);
	printf("\t  RTD2      %d      AIN%d    AIN%d    AIN%d     %c"
	       EOL,
	       SENSOR_CHANNEL1, RTD2_2WIRE_IOUT0, RTD2_2WIRE_AINP, RTD2_2WIRE_AINM,
	       status_info[sensor_enable_status[SENSOR_CHANNEL1]]);
#if defined(AD7124_8)
	printf("\t  RTD3      %d      AIN%d    AIN%d    AIN%d     %c" EOL,
	       SENSOR_CHANNEL2, RTD3_2WIRE_IOUT0, RTD3_2WIRE_AINP, RTD3_2WIRE_AINM,
	       status_info[sensor_enable_status[SENSOR_CHANNEL2]]);
	printf("\t  RTD4      %d      AIN%d   AIN%d    AIN%d    %c" EOL,
	       SENSOR_CHANNEL3, RTD4_2WIRE_IOUT0, RTD4_2WIRE_AINP, RTD4_2WIRE_AINM,
	       status_info[sensor_enable_status[SENSOR_CHANNEL3]]);
	printf("\t  RTD5      %d      AIN%d   AIN%d   AIN%d    %c" EOL,
	       SENSOR_CHANNEL4, RTD5_2WIRE_IOUT0, RTD5_2WIRE_AINP, RTD5_2WIRE_AINM,
	       status_info[sensor_enable_status[SENSOR_CHANNEL4]]);
#endif
}


/*!
 * @brief	Display header information for 3-wire RTD measurement menu
 * @return	none
 */
void rtd_3wire_menu_header(void)
{
	if (strcmp(current_sensor_config, sensor_configs[AD7124_CONFIG_3WIRE_RTD])) {
		/* Disable unused sensor channels */
		for (uint8_t chn = max_supported_sensors[AD7124_CONFIG_3WIRE_RTD];
		     chn < NUM_OF_SENSOR_CHANNELS; chn++) {
			sensor_enable_status[chn] = false;
		}

		/* Load the 3-wire RTD device configuration */
		init_with_configuration(AD7124_CONFIG_3WIRE_RTD);
	}

	printf("\t Sensor  Channel   IOUT0   IOUT1  AIN+    AIN-   Enable" EOL);
	printf("\t ------------------------------------------------------" EOL);
	printf("\t  RTD1      %d      AIN%d    AIN%d   AIN%d    AIN%d     %c"
	       EOL,
	       SENSOR_CHANNEL0, RTD1_3WIRE_IOUT0, RTD1_3WIRE_IOUT1, RTD1_3WIRE_AINP,
	       RTD1_3WIRE_AINM,
	       status_info[sensor_enable_status[SENSOR_CHANNEL0]]);
	printf("\t  RTD2      %d      AIN%d    AIN%d   AIN%d    AIN%d     %c"
	       EOL,
	       SENSOR_CHANNEL1, RTD2_3WIRE_IOUT0, RTD2_3WIRE_IOUT1, RTD2_3WIRE_AINP,
	       RTD2_3WIRE_AINM,
	       status_info[sensor_enable_status[SENSOR_CHANNEL1]]);
#if defined(AD7124_8)
	printf("\t  RTD3      %d      AIN%d   AIN%d  AIN%d    AIN%d     %c"
	       EOL,
	       SENSOR_CHANNEL2, RTD3_3WIRE_IOUT0, RTD3_3WIRE_IOUT1, RTD3_3WIRE_AINP,
	       RTD3_3WIRE_AINM,
	       status_info[sensor_enable_status[SENSOR_CHANNEL2]]);
	printf("\t  RTD4      %d      AIN%d   AIN%d  AIN%d   AIN%d    %c"
	       EOL,
	       SENSOR_CHANNEL3, RTD4_3WIRE_IOUT0, RTD4_3WIRE_IOUT1, RTD4_3WIRE_AINP,
	       RTD4_3WIRE_AINM,
	       status_info[sensor_enable_status[SENSOR_CHANNEL3]]);
#endif

	printf("\t -------------------------------------------------------------------"
	       EOL);
	printf("\tNote: For single   RTD measurement, connect Rref at the higher side"
	       EOL);
	printf("\t      For multiple RTD measurement, connect Rref at the lower side"
	       EOL);
}


/*!
 * @brief	Display header information for 3-wire RTD calibration menu
 * @return	none
 */
void rtd_3wire_calibration_menu_header(void)
{
	if (rtd_3wire_calibration_type == MEASURING_EXCITATION_CURRENT) {
		if (strcmp(current_sensor_config, sensor_configs[AD7124_CONFIG_3WIRE_RTD])) {
			/* Disable unused sensor channels */
			for (uint8_t chn = max_supported_sensors[AD7124_CONFIG_3WIRE_RTD];
			     chn < NUM_OF_SENSOR_CHANNELS; chn++) {
				sensor_enable_status[chn] = false;
			}

			/* Load the 3-wire RTD device configuration */
			init_with_configuration(AD7124_CONFIG_3WIRE_RTD);
		}

		/* For 'Iout measurement type calibration', the additional 2 analog inputs
		 * are needed for Ref measurement, which reduces number of allowed sensors
		 * interfaces by 1 */
#if defined(AD7124_8)
		/* Chn 0, 1 and 2 are active. Chn3 is disabled */
		sensor_enable_status[SENSOR_CHANNEL3] = false;
#else
		/* Chn0 is active. Chn1 is disabled */
		sensor_enable_status[SENSOR_CHANNEL1] = false;
#endif

		printf("\t Calibration Type: Measuring Excitation Current" EOL);
		printf("\t -------------------------------------------------------------------"
		       EOL);
		printf("\t Sensor  Channel   RTD    RTD     IOUT0   IOUT1  Ref    Ref   Enable"
		       EOL);
		printf("\t                   AIN+   AIN-                   AIN+   AIN-        "
		       EOL);
		printf("\t -------------------------------------------------------------------"
		       EOL);

		printf("\t  RTD1      %d      AIN%d   AIN%d    AIN%d    AIN%d   AIN%d  AIN%d  %c"
		       EOL,
		       SENSOR_CHANNEL0,
		       RTD1_3WIRE_AINP, RTD1_3WIRE_AINM, RTD1_3WIRE_IOUT0,
		       RTD1_3WIRE_IOUT1, RTD_3WIRE_EXC_MEASURE_AINP, RTD_3WIRE_EXC_MEASURE_AINM,
		       status_info[sensor_enable_status[SENSOR_CHANNEL0]]);

#if defined(AD7124_8)
		printf("\t  RTD2      %d      AIN%d   AIN%d    AIN%d    AIN%d   AIN%d  AIN%d  %c"
		       EOL,
		       SENSOR_CHANNEL1,
		       RTD2_3WIRE_AINP, RTD2_3WIRE_AINM, RTD2_3WIRE_IOUT0,
		       RTD2_3WIRE_IOUT1, RTD_3WIRE_EXC_MEASURE_AINP, RTD_3WIRE_EXC_MEASURE_AINM,
		       status_info[sensor_enable_status[SENSOR_CHANNEL1]]);

		printf("\t  RTD3      %d      AIN%d   AIN%d    AIN%d   AIN%d  AIN%d  AIN%d  %c"
		       EOL,
		       SENSOR_CHANNEL2,
		       RTD3_3WIRE_AINP, RTD3_3WIRE_AINM, RTD3_3WIRE_IOUT0,
		       RTD3_3WIRE_IOUT1, RTD_3WIRE_EXC_MEASURE_AINP, RTD_3WIRE_EXC_MEASURE_AINM,
		       status_info[sensor_enable_status[SENSOR_CHANNEL2]]);
#endif
	} else {
		printf("\t Calibration Type: Chopping Excitation Current " EOL);
		printf("\t ------------------------------------------------------" EOL);
		rtd_3wire_menu_header();
	}
}


/*!
 * @brief	Display header information for 4-wire RTD measurement menu
 * @return	none
 */
void rtd_4wire_menu_header(void)
{
	if (strcmp(current_sensor_config, sensor_configs[AD7124_CONFIG_4WIRE_RTD])) {
		/* Disable unused sensor channels */
		for (uint8_t chn = max_supported_sensors[AD7124_CONFIG_4WIRE_RTD];
		     chn < NUM_OF_SENSOR_CHANNELS; chn++) {
			sensor_enable_status[chn] = false;
		}

		/* Load the 4-wire RTD device configuration */
		init_with_configuration(AD7124_CONFIG_4WIRE_RTD);
	}

	printf("\t Sensor  Channel   IOUT0   AIN+    AIN-   Enable" EOL);
	printf("\t -----------------------------------------------" EOL);
	printf("\t  RTD1      %d      AIN%d    AIN%d    AIN%d     %c"
	       EOL,
	       SENSOR_CHANNEL0, RTD1_4WIRE_IOUT0, RTD1_4WIRE_AINP, RTD1_4WIRE_AINM,
	       status_info[sensor_enable_status[SENSOR_CHANNEL0]]);
	printf("\t  RTD2      %d      AIN%d    AIN%d    AIN%d     %c"
	       EOL,
	       SENSOR_CHANNEL1, RTD2_4WIRE_IOUT0, RTD2_4WIRE_AINP, RTD2_4WIRE_AINM,
	       status_info[sensor_enable_status[SENSOR_CHANNEL1]]);
#if defined(AD7124_8)
	printf("\t  RTD3      %d      AIN%d    AIN%d    AIN%d     %c" EOL,
	       SENSOR_CHANNEL2, RTD3_4WIRE_IOUT0, RTD3_4WIRE_AINP, RTD3_4WIRE_AINM,
	       status_info[sensor_enable_status[SENSOR_CHANNEL2]]);
	printf("\t  RTD4      %d      AIN%d   AIN%d    AIN%d    %c" EOL,
	       SENSOR_CHANNEL3, RTD4_4WIRE_IOUT0, RTD4_4WIRE_AINP, RTD4_4WIRE_AINM,
	       status_info[sensor_enable_status[SENSOR_CHANNEL3]]);
	printf("\t  RTD5      %d      AIN%d   AIN%d   AIN%d    %c" EOL,
	       SENSOR_CHANNEL4, RTD5_4WIRE_IOUT0, RTD5_4WIRE_AINP, RTD5_4WIRE_AINM,
	       status_info[sensor_enable_status[SENSOR_CHANNEL4]]);
#endif
}


/*!
 * @brief	Display header information for NTC thermistor measurement menu
 * @return	none
 */
void ntc_thermistor_menu_header(void)
{
	if (strcmp(current_sensor_config, sensor_configs[AD7124_CONFIG_THERMISTOR])) {
		/* Disable unused sensor channels */
		for (uint8_t chn = max_supported_sensors[AD7124_CONFIG_THERMISTOR];
		     chn < NUM_OF_SENSOR_CHANNELS; chn++) {
			sensor_enable_status[chn] = false;
		}

		/* Load the Thermistor device configuration */
		init_with_configuration(AD7124_CONFIG_THERMISTOR);
	}

	printf("\t Sensor  Channel   AIN+    AIN-   Enable" EOL);
	printf("\t ---------------------------------------" EOL);
	printf("\t  NTC1      %d       AIN%d    AIN%d     %c"
	       EOL,
	       SENSOR_CHANNEL0, NTC1_THERMISTOR_AINP, NTC1_THERMISTOR_AINM,
	       status_info[sensor_enable_status[SENSOR_CHANNEL0]]);
	printf("\t  NTC2      %d       AIN%d    AIN%d     %c"
	       EOL,
	       SENSOR_CHANNEL1, NTC2_THERMISTOR_AINP, NTC2_THERMISTOR_AINM,
	       status_info[sensor_enable_status[SENSOR_CHANNEL1]]);
	printf("\t  NTC3      %d       AIN%d    AIN%d     %c"
	       EOL,
	       SENSOR_CHANNEL2, NTC3_THERMISTOR_AINP, NTC3_THERMISTOR_AINM,
	       status_info[sensor_enable_status[SENSOR_CHANNEL2]]);
	printf("\t  NTC4      %d       AIN%d    AIN%d     %c"
	       EOL,
	       SENSOR_CHANNEL3, NTC4_THERMISTOR_AINP, NTC4_THERMISTOR_AINM,
	       status_info[sensor_enable_status[SENSOR_CHANNEL3]]);
#if defined(AD7124_8)
	printf("\t  NTC5      %d       AIN%d    AIN%d     %c" EOL,
	       SENSOR_CHANNEL4, NTC5_THERMISTOR_AINP, NTC5_THERMISTOR_AINM,
	       status_info[sensor_enable_status[SENSOR_CHANNEL4]]);
	printf("\t  NTC6      %d       AIN%d   AIN%d    %c" EOL,
	       SENSOR_CHANNEL5, NTC6_THERMISTOR_AINP, NTC6_THERMISTOR_AINM,
	       status_info[sensor_enable_status[SENSOR_CHANNEL5]]);
	printf("\t  NTC7      %d       AIN%d   AIN%d    %c" EOL,
	       SENSOR_CHANNEL6, NTC7_THERMISTOR_AINP, NTC7_THERMISTOR_AINM,
	       status_info[sensor_enable_status[SENSOR_CHANNEL6]]);
	printf("\t  NTC8      %d       AIN%d   AIN%d    %c" EOL,
	       SENSOR_CHANNEL7, NTC8_THERMISTOR_AINP, NTC8_THERMISTOR_AINM,
	       status_info[sensor_enable_status[SENSOR_CHANNEL7]]);
#endif
}


/*!
 * @brief	Display header information for NTC thermistor measurement menu
 * @return	none
 */
void thermocouple_menu_header(void)
{
	if (strcmp(current_sensor_config, sensor_configs[AD7124_CONFIG_THERMOCOUPLE])) {
		/* Disable unused sensor channels */
		for (uint8_t chn = max_supported_sensors[AD7124_CONFIG_THERMOCOUPLE];
		     chn < NUM_OF_SENSOR_CHANNELS; chn++) {
			sensor_enable_status[chn] = false;
		}

		/* Select CJC sensor */
		select_cjc_sensor(current_cjc_sensor);

		/* Load the Thermocouple device configuration */
		init_with_configuration(AD7124_CONFIG_THERMOCOUPLE);
	}

	printf("\t Sensor  Channel  IOUT0   AIN+    AIN-   Enable" EOL);
	printf("\t ----------------------------------------------" EOL);
	printf("\t  TC1      %d       -      AIN%d    AIN%d     %c"
	       EOL,
	       SENSOR_CHANNEL0, THERMOCOUPLE1_AINP, THERMOCOUPLE1_AINM,
	       status_info[sensor_enable_status[SENSOR_CHANNEL0]]);
	printf("\t  TC2      %d       -      AIN%d    AIN%d     %c"
	       EOL,
	       SENSOR_CHANNEL1, THERMOCOUPLE2_AINP, THERMOCOUPLE2_AINM,
	       status_info[sensor_enable_status[SENSOR_CHANNEL1]]);
#if defined(AD7124_8)
	printf("\t  TC3      %d       -      AIN%d    AIN%d     %c"
	       EOL,
	       SENSOR_CHANNEL2, THERMOCOUPLE3_AINP, THERMOCOUPLE3_AINM,
	       status_info[sensor_enable_status[SENSOR_CHANNEL2]]);
	printf("\t  TC4      %d       -      AIN%d   AIN%d    %c"
	       EOL,
	       SENSOR_CHANNEL3, THERMOCOUPLE4_AINP, THERMOCOUPLE4_AINM,
	       status_info[sensor_enable_status[SENSOR_CHANNEL3]]);
	printf("\t  TC5      %d       -      AIN%d   AIN%d    %c" EOL,
	       SENSOR_CHANNEL4, THERMOCOUPLE5_AINP, THERMOCOUPLE5_AINM,
	       status_info[sensor_enable_status[SENSOR_CHANNEL4]]);
	printf("\t  TC6      %d       -      AIN%d   AIN%d    %c" EOL,
	       SENSOR_CHANNEL5, THERMOCOUPLE6_AINP, THERMOCOUPLE6_AINM,
	       status_info[sensor_enable_status[SENSOR_CHANNEL5]]);
#endif

	printf(EOL "\t Current CJC: %s" EOL, cjc_sensor_names[current_cjc_sensor]);
	printf("\t ----------------------------------------------" EOL);

	switch (current_cjc_sensor) {
	case PT100_4WIRE_RTD:
	case PT1000_2WIRE_RTD:
		printf("\t  CJC      %d       AIN%d   AIN%d    AIN%d     Y"
		       EOL,
		       CJC_RTD_CHN,
		       CJC_RTD_IOUT0,
		       CJC_RTD_AINP,
		       CJC_RTD_AINM);
		break;

	case THERMISTOR_PTC_KY81_110:
		printf("\t  CJC      %d       AIN%d   AIN%d    AIN%d     Y"
		       EOL,
		       CJC_THERMISTOR_CHN,
		       CJC_PTC_THERMISTOR_IOUT0,
		       CJC_PTC_THERMISTOR_AINP,
		       CJC_PTC_THERMISTOR_AINM);
		break;

	default:
		printf("\t  No CJC selected!!" EOL);
		break;
	}
}


/*!
 * @brief	Display header information for ADC calibration menu
 * @return	none
 */
void adc_calibration_menu_header(void)
{
	printf("\tCurrent Config: %s" EOL, current_sensor_config);
	printf("\t----------------------------------------------------" EOL);
	printf("\t CHN0:  %c  |  CHN1:  %c  |  CHN2:  %c  |  CHN3:  %c" EOL
	       "\t CHN4:  %c  |  CHN5:  %c  |  CHN6:  %c  |  CHN7:  %c" EOL,
	       status_info[sensor_enable_status[SENSOR_CHANNEL0]],
	       status_info[sensor_enable_status[SENSOR_CHANNEL1]],
	       status_info[sensor_enable_status[SENSOR_CHANNEL2]],
	       status_info[sensor_enable_status[SENSOR_CHANNEL3]],
	       status_info[sensor_enable_status[SENSOR_CHANNEL4]],
	       status_info[sensor_enable_status[SENSOR_CHANNEL5]],
	       status_info[sensor_enable_status[SENSOR_CHANNEL6]],
	       status_info[sensor_enable_status[SENSOR_CHANNEL7]]);
	printf("\t----------------------------------------------------" EOL);
	printf("\t*Note: The AD7124 is factory calibrated at a gain of 1, and the resulting gain coefficient"
	       EOL
	       "\t       is the default gain coefficient on the device. The device does not support further"
	       EOL
	       "\t       internal full-scale calibrations at a gain of 1" EOL);
}


/*!
 * @brief	Display header information for main menu
 * @return	none
 */
void main_menu_header(void)
{
	printf("\tCurrent Config: %s,", current_sensor_config);
	printf("  Active Device: %s" EOL, ACTIVE_DEVICE);
}


/*!
 * @brief	Display 2-wire RTD measurement main menu
 * @param	menu_id[in]- Optional menu ID
 * @return	MENU_CONTINUE
 */
int32_t display_2wire_rtd_menu(uint32_t menu_id)
{
	return adi_do_console_menu(&rtd_2wire_menu);
}


/*!
 * @brief	Display 3-wire RTD measurement main menu
 * @param	menu_id[in]- Optional menu ID
 * @return	MENU_CONTINUE
 */
int32_t display_3wire_rtd_menu(uint32_t menu_id)
{
	return adi_do_console_menu(&rtd_3wire_menu);
}


/*!
 * @brief	Display 3-wire RTD calibration main menu
 * @param	menu_id[in]- Optional menu ID
 * @return	MENU_CONTINUE
 */
int32_t display_3wire_rtd_calibration_menu(uint32_t menu_id)
{
	return adi_do_console_menu(&rtd_3wire_calibration_menu);
}


/*!
 * @brief	Display 4-wire RTD measurement main menu
 * @param	menu_id[in]- Optional menu ID
 * @return	MENU_CONTINUE
 */
int32_t display_4wire_rtd_menu(uint32_t menu_id)
{
	return adi_do_console_menu(&rtd_4wire_menu);
}


/*!
 * @brief	Display thermoucouple measurement main menu
 * @param	menu_id[in]- Optional menu ID
 * @return	MENU_CONTINUE
 */
int32_t display_thermocouple_menu(uint32_t menu_id)
{
	return adi_do_console_menu(&thermocouple_menu);
}


/*!
 * @brief	Display thermistor measurement main menu
 * @param	menu_id[in]- Optional menu ID
 * @return	MENU_CONTINUE
 */
int32_t display_ntc_thermistor_menu(uint32_t menu_id)
{
	return adi_do_console_menu(&ntc_thermistor_menu);
}


/*!
 * @brief	Display ADC calibration main menu
 * @param	menu_id[in]- Optional menu ID
 * @return	MENU_CONTINUE
 */
int32_t display_adc_calibration_menu(uint32_t menu_id)
{
	return adi_do_console_menu(&adc_calibration_menu);
}


/*!
 * @brief	Display ADC calibration main menu
 * @param	menu_id[in]- Optional menu ID
 * @return	MENU_CONTINUE
 */
int32_t reset_device_config(uint32_t menu_id)
{
	if (init_with_configuration(AD7124_CONFIG_RESET) != 0) {
		printf(EOL "\t Error resetting config!!" EOL);
		adi_press_any_key_to_continue();
	} else {
		/* Disable all sensor channels except channel 0 */
		for (uint8_t chn = SENSOR_CHANNEL1; chn < NUM_OF_SENSOR_CHANNELS; chn++) {
			sensor_enable_status[chn] = false;
		}
	}

	return MENU_CONTINUE;
}


/* =========== Menu Declarations =========== */

static console_menu_item rtd_2wire_menu_items[] = {
	{ "Enable/Disable RTD1", '1', enable_disable_sensor, NULL, SENSOR_CHANNEL0 },
	{ "Enable/Disable RTD2", '2', enable_disable_sensor, NULL, SENSOR_CHANNEL1 },
#if defined(AD7124_8)
	{ "Enable/Disable RTD3", '3', enable_disable_sensor, NULL, SENSOR_CHANNEL2 },
	{ "Enable/Disable RTD4", '4', enable_disable_sensor, NULL, SENSOR_CHANNEL3 },
	{ "Enable/Disable RTD5", '5', enable_disable_sensor, NULL, SENSOR_CHANNEL4 },
#endif
	{ " " },
	{ "Perform Averaged Measurement", 'A', perform_2wire_rtd_measurement, NULL, AVERAGED_MEASUREMENT },
	{ "Perform Single Measurement", 'S', perform_2wire_rtd_measurement, NULL, SINGLE_MEASUREMENT },
	{ "Perform Continuous Measurement", 'C', perform_2wire_rtd_measurement, NULL, CONTINUOUS_MEASUREMENT },
};

console_menu rtd_2wire_menu = {
	.title = "2-Wire RTD Measurement",
	.items = rtd_2wire_menu_items,
	.itemCount = ARRAY_SIZE(rtd_2wire_menu_items),
	.headerItem = rtd_2wire_menu_header,
	.footerItem = NULL,
	.enableEscapeKey = true
};

static console_menu_item rtd_3wire_menu_items[] = {
	{ "Enable/Disable RTD1", '1', enable_disable_sensor, NULL, SENSOR_CHANNEL0 },
	{ "Enable/Disable RTD2", '2', enable_disable_sensor, NULL, SENSOR_CHANNEL1 },
#if defined(AD7124_8)
	{ "Enable/Disable RTD3", '3', enable_disable_sensor, NULL, SENSOR_CHANNEL2 },
	{ "Enable/Disable RTD4", '4', enable_disable_sensor, NULL, SENSOR_CHANNEL3 },
#endif
	{ " " },
	{ "Calibrate RTD and Perform Measurement",    'M', display_3wire_rtd_calibration_menu },
	{ " " },
	{ "No Calibration Measurement:" },
	{ "Perform Averaged Measurement", 'A', perform_3wire_rtd_measurement, NULL, AVERAGED_MEASUREMENT },
	{ "Perform Single Measurement", 'S', perform_3wire_rtd_measurement, NULL, SINGLE_MEASUREMENT },
	{ "Perform Continuous Measurement", 'C', perform_3wire_rtd_measurement, NULL, CONTINUOUS_MEASUREMENT },
};

console_menu rtd_3wire_menu = {
	.title = "3-Wire RTD Measurement",
	.items = rtd_3wire_menu_items,
	.itemCount = ARRAY_SIZE(rtd_3wire_menu_items),
	.headerItem = rtd_3wire_menu_header,
	.footerItem = NULL,
	.enableEscapeKey = true
};

static console_menu_item rtd_3wire_calibration_menu_items[] = {
	{ "Change type to Measuring Excitation Current", 'E', change_3wire_rtd_calibration_type, NULL, MEASURING_EXCITATION_CURRENT },
	{ "Change type to Chopping Excitation Current", 'P', change_3wire_rtd_calibration_type, NULL, CHOPPING_EXCITATION_CURRENT  },
	{ " " },
	{ "Perform Averaged Measurement", 'A', calibrate_and_measure_3wire_rtd, NULL, AVERAGED_MEASUREMENT },
	{ "Perform Single Measurement", 'S', calibrate_and_measure_3wire_rtd, NULL, SINGLE_MEASUREMENT },
	{ "Perform Continuous Measurement", 'C', calibrate_and_measure_3wire_rtd, NULL, CONTINUOUS_MEASUREMENT },
};

console_menu rtd_3wire_calibration_menu = {
	.title = "Calibrate 3-Wire RTD Excitation Source",
	.items = rtd_3wire_calibration_menu_items,
	.itemCount = ARRAY_SIZE(rtd_3wire_calibration_menu_items),
	.headerItem = rtd_3wire_calibration_menu_header,
	.footerItem = NULL,
	.enableEscapeKey = true
};

static console_menu_item rtd_4wire_menu_items[] = {
	{ "Enable/Disable RTD1", '1', enable_disable_sensor, NULL, SENSOR_CHANNEL0 },
	{ "Enable/Disable RTD2", '2', enable_disable_sensor, NULL, SENSOR_CHANNEL1 },
#if defined(AD7124_8)
	{ "Enable/Disable RTD3", '3', enable_disable_sensor, NULL, SENSOR_CHANNEL2 },
	{ "Enable/Disable RTD4", '4', enable_disable_sensor, NULL, SENSOR_CHANNEL3 },
	{ "Enable/Disable RTD5", '5', enable_disable_sensor, NULL, SENSOR_CHANNEL4 },
#endif
	{ " " },
	{ "Perform Averaged Measurement", 'A', perform_4wire_rtd_measurement, NULL, AVERAGED_MEASUREMENT },
	{ "Perform Single Measurement", 'S', perform_4wire_rtd_measurement, NULL, SINGLE_MEASUREMENT },
	{ "Perform Continuous Measurement", 'C', perform_4wire_rtd_measurement, NULL, CONTINUOUS_MEASUREMENT },
};

console_menu rtd_4wire_menu = {
	.title = "4-Wire RTD Measurement",
	.items = rtd_4wire_menu_items,
	.itemCount = ARRAY_SIZE(rtd_4wire_menu_items),
	.headerItem = rtd_4wire_menu_header,
	.footerItem = NULL,
	.enableEscapeKey = true
};

static console_menu_item ntc_thermistor_menu_items[] = {
	{ "Enable/Disable NTC1", '1', enable_disable_sensor, NULL, SENSOR_CHANNEL0 },
	{ "Enable/Disable NTC2", '2', enable_disable_sensor, NULL, SENSOR_CHANNEL1 },
	{ "Enable/Disable NTC3", '3', enable_disable_sensor, NULL, SENSOR_CHANNEL2 },
	{ "Enable/Disable NTC4", '4', enable_disable_sensor, NULL, SENSOR_CHANNEL3 },
#if defined(AD7124_8)
	{ "Enable/Disable NTC5", '5', enable_disable_sensor, NULL, SENSOR_CHANNEL4 },
	{ "Enable/Disable NTC6", '6', enable_disable_sensor, NULL, SENSOR_CHANNEL5 },
	{ "Enable/Disable NTC7", '7', enable_disable_sensor, NULL, SENSOR_CHANNEL6 },
	{ "Enable/Disable NTC8", '8', enable_disable_sensor, NULL, SENSOR_CHANNEL7 },
#endif
	{ " " },
	{ "Perform Averaged Measurement",  'A', perform_ntc_thermistor_measurement, NULL, AVERAGED_MEASUREMENT },
	{ "Perform Single Measurement",    'S', perform_ntc_thermistor_measurement, NULL, SINGLE_MEASUREMENT },
	{ "Perform Continuous Measurement", 'C', perform_ntc_thermistor_measurement, NULL, CONTINUOUS_MEASUREMENT },
};

console_menu ntc_thermistor_menu = {
	.title = "NTC Thermistor Measurement",
	.items = ntc_thermistor_menu_items,
	.itemCount = ARRAY_SIZE(ntc_thermistor_menu_items),
	.headerItem = ntc_thermistor_menu_header,
	.footerItem = NULL,
	.enableEscapeKey = true
};

static console_menu_item thermocouple_menu_items[] = {
	{ "Enable/Disable TC1", '1', enable_disable_sensor, NULL, SENSOR_CHANNEL0 },
	{ "Enable/Disable TC2", '2', enable_disable_sensor, NULL, SENSOR_CHANNEL1 },
#if defined(AD7124_8)
	{ "Enable/Disable TC3", '3', enable_disable_sensor, NULL, SENSOR_CHANNEL2 },
	{ "Enable/Disable TC4", '4', enable_disable_sensor, NULL, SENSOR_CHANNEL3 },
	{ "Enable/Disable TC5", '5', enable_disable_sensor, NULL, SENSOR_CHANNEL4 },
	{ "Enable/Disable TC6", '6', enable_disable_sensor, NULL, SENSOR_CHANNEL5 },
#endif
	{ " " },
	{ "Select CJC (PT100 4-wire RTD)", '7', select_cjc_sensor, NULL, PT100_4WIRE_RTD    },
	{ "Select CJC (PTC KY81/110 Thermistor)", '8', select_cjc_sensor, NULL, THERMISTOR_PTC_KY81_110 },
	{ "Select CJC (PT1000 2-wire RTD)", '9', select_cjc_sensor, NULL, PT1000_2WIRE_RTD   },
	{ " " },
	{ "Perform Averaged Measurement", 'A', perform_thermocouple_measurement, NULL, AVERAGED_MEASUREMENT },
	{ "Perform Single Measurement", 'S', perform_thermocouple_measurement, NULL, SINGLE_MEASUREMENT },
	{ "Perform Continuous Measurement", 'C', perform_thermocouple_measurement, NULL, CONTINUOUS_MEASUREMENT },
};

console_menu thermocouple_menu = {
	.title = "Thermocouple Measurement",
	.items = thermocouple_menu_items,
	.itemCount = ARRAY_SIZE(thermocouple_menu_items),
	.headerItem = thermocouple_menu_header,
	.footerItem = NULL,
	.enableEscapeKey = true
};

static console_menu_item adc_calibration_menu_items[] = {
	{ "Perform Internal Calibration", 'I', perform_adc_calibration, NULL, INTERNAL_CALIBRATION },
	{ "Perform System Calibration", 'S', perform_adc_calibration, NULL, SYSTEM_CALIBRATION },
};

console_menu adc_calibration_menu = {
	.title = "AD7124 Calibration",
	.items = adc_calibration_menu_items,
	.itemCount = ARRAY_SIZE(adc_calibration_menu_items),
	.headerItem = adc_calibration_menu_header,
	.footerItem = NULL,
	.enableEscapeKey = true
};

/*
 * Definition of the Main Menu Items and menu itself
 */
static console_menu_item main_menu_items[] = {
	{"2-Wire RTD",		'A',	display_2wire_rtd_menu       },
	{"3-Wire RTD",		'B',	display_3wire_rtd_menu       },
	{"4-Wire RTD",		'C',	display_4wire_rtd_menu       },
	{"Thermocouple",	'D',	display_thermocouple_menu    },
	{"Thermistor",		'E',	display_ntc_thermistor_menu  },
	{"Calibrate ADC",	'F',	display_adc_calibration_menu },
	{ " " },
	{"Reset Config", 'R', reset_device_config, NULL, AD7124_CONFIG_RESET },
};

console_menu ad7124_main_menu = {
	.title = "AD7124 Sensor Measurement Menu",
	.items = main_menu_items,
	.itemCount = ARRAY_SIZE(main_menu_items),
	.headerItem = main_menu_header,
	.footerItem = NULL,
	.enableEscapeKey = false
};
