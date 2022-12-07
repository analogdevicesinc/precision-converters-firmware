/***************************************************************************//**
 *   @file    ad4130_iio.c
 *   @brief   Implementation of AD4130 IIO application interfaces
 *   @details This module acts as an interface for AD4130 IIO application
********************************************************************************
 * Copyright (c) 2020-2022 Analog Devices, Inc.
 * All rights reserved.
 *
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 * By using this software you agree to the terms of the associated
 * Analog Devices Software License Agreement.
*******************************************************************************/

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#include "app_config.h"
#include "ad4130_iio.h"
#include "ad4130_support.h"
#include "ad4130_user_config.h"
#include "ad4130_temperature_sensor.h"
#include "ad4130_regs.h"
#include "no_os_error.h"
#include "board_info.h"
#include "iio_trigger.h"

/******** Forward declaration of getter/setter functions ********/
static int iio_ad4130_attr_get(void *device, char *buf, uint32_t len,
			       const struct iio_ch_info *channel, intptr_t priv);

static int iio_ad4130_attr_set(void *device, char *buf, uint32_t len,
			       const struct iio_ch_info *channel, intptr_t priv);

static int iio_ad4130_attr_available_get(void *device, char *buf,
		uint32_t len,
		const struct iio_ch_info *channel, intptr_t priv);

static int iio_ad4130_attr_available_set(void *device, char *buf,
		uint32_t len,
		const struct iio_ch_info *channel, intptr_t priv);

/******************************************************************************/
/************************ Macros/Constants ************************************/
/******************************************************************************/

#define AD4130_CHN_ATTR(_name, _priv) {\
	.name = _name,\
	.priv = _priv,\
	.show = iio_ad4130_attr_get,\
	.store = iio_ad4130_attr_set\
}

#define AD4130_CHN_AVAIL_ATTR(_name, _priv) {\
	.name = _name,\
	.priv = _priv,\
	.show = iio_ad4130_attr_available_get,\
	.store = iio_ad4130_attr_available_set\
}

#define AD4130_CH(_name, _idx, _type) {\
	.name = _name, \
	.ch_type = _type,\
	.ch_out = 0,\
	.indexed = true,\
	.channel = _idx,\
	.scan_index = _idx,\
	.scan_type = &chn_scan,\
	.attributes = ad4130_iio_ch_attributes\
}

/* Minimum sampling frequency supported/configured in the firmware.
 * Note: This is not an actual device sampling frequency.
 * It is just used for IIO oscilloscope timeout calculations. */
#define AD4130_MIN_SAMPLING_FREQ	(50 / ADC_USER_CHANNELS)

/* Default offset value for AD4130 */
#define AD4130_DEFAULT_OFFSET		0x800000

/*	Number of IIO devices */
#define NUM_OF_IIO_DEVICES	1

/* IIO trigger name */
#define AD4130_IIO_TRIGGER_NAME		"ad4130_iio_trigger"

/* Bytes per sample. This count should divide the total 256 bytes into 'n' equivalent
 * ADC samples as IIO library requests only 256bytes of data at a time in a given
 * data read query.
 * For 1 to 8-bit ADC, bytes per sample = 1 (2^0)
 * For 9 to 16-bit ADC, bytes per sample = 2 (2^1)
 * For 17 to 32-bit ADC, bytes per sample = 4 (2^2)
 **/
#define	BYTES_PER_SAMPLE	sizeof(uint32_t)	// For ADC resolution of 24-bits

/* Number of data storage bits (needed for IIO client to plot ADC data) */
#define CHN_STORAGE_BITS	(BYTES_PER_SAMPLE * 8)

/* Number of adc samples for loadcell calibration */
#define LOADCELL_SAMPLES_COUNT	10

/* CJC channel is 1 (common sensor for all Thermocouples).
 * Chn0 is used for TC connections */
#define CJC_CHANNEL		1

/* Shunt resistance (in ohms) for AVDD/IOVDD current calculation */
#define I_RSENSE	10

/* Multiplier for AVDD/IOVDD voltage calculation */
#define V_SCALE		6

/* Fifo depth limit (watermark count) for data capture */
#define FIFO_SIZE		256		// Range: 1-256

/* Timeout count to avoid stuck into potential infinite loop while checking
 * for new data into an acquisition buffer. The actual timeout factor is determined
 * through 'sampling_frequency' attribute of IIO app, but this period here makes sure
 * we are not stuck into a forever loop in case data capture is interrupted
 * or failed in between.
 * Note: This timeout factor is dependent upon the MCU clock frequency. Below timeout
 * is tested for SDP-K1 platform @180Mhz default core clock */
#define BUF_READ_TIMEOUT	0xffffffff

/* ADC data buffer size */
#if defined(USE_SDRAM)
#define adc_data_buffer				SDRAM_START_ADDRESS
#define DATA_BUFFER_SIZE			SDRAM_SIZE_BYTES
#else
#define DATA_BUFFER_SIZE			(32768)		// 32kbytes
static int8_t adc_data_buffer[DATA_BUFFER_SIZE] = { 0 };
#endif

/******************************************************************************/
/*************************** Types Declarations *******************************/
/******************************************************************************/

/* IIO interface descriptor */
static struct iio_desc *p_ad4130_iio_desc;

/**
 * Pointer to the struct representing the AD4130 IIO device
 */
struct ad413x_dev *ad4130_dev_inst = NULL;

/* IIO hw trigger descriptor */
static struct iio_hw_trig *ad4130_hw_trig_desc;

/* Number of active channels */
static volatile uint8_t num_of_active_channels;

/* Data buffer index */
static volatile uint8_t data_indx = 0;

static volatile bool fifo_data_available = false;
static volatile bool start_fifo_mode_data_capture = false;
static uint32_t fifo_data[FIFO_SIZE];

/* Flag to indicate if size of the buffer is updated according to requested
 * number of samples for the multi-channel IIO buffer data alignment */
static volatile bool buf_size_updated = false;

/* Device attributes with default values */

/* Scale attribute value per channel */
static float attr_scale_val[ADC_USER_CHANNELS];

/* IIOD channels scan structure */
static struct scan_type chn_scan;

/* AD4130 Attribute IDs */
enum ad4130_attribute_id {
	RAW_ATTR_ID,
	SCALE_ATTR_ID,
	OFFSET_ATTR_ID,
	SAMPLING_FREQ_ATTR_ID,
	DEMO_CONFIG_ATTR_ID,
	INTERNAL_CALIB_ID,
	SYSTEM_CALIB_ID,
	LOADCELL_GAIN_CALIB_ID,
	LOADCELL_OFFSET_CALIB_ID,
};

/* Calibration state */
enum calibration_state {
	FULL_SCALE_CALIB_STATE,
	ZERO_SCALE_CALIB_STATE,
	CALIB_COMPLETE_STATE
};

/* Calibration status */
enum calib_status {
	CALIB_NOT_DONE,
	CALIB_IN_PROGRESS,
	CALIB_DONE,
	CALIB_ERROR,
	CALIB_SKIPPED
};

/* Context attributes ID */
enum context_attr_ids {
	HW_MEZZANINE_ID,
	HW_CARRIER_ID,
	HW_NAME_ID,
	DEF_NUM_OF_CONTXT_ATTRS
};

/* ADC calibration configs */
typedef struct {
	uint32_t gain_before_calib;
	uint32_t gain_after_calib;
	uint32_t offset_before_calib;
	uint32_t offset_after_calib;
} adc_calibration_configs;

/* ADC calibration variables */
static enum calibration_state system_calibration_state = ZERO_SCALE_CALIB_STATE;
static enum calibration_state internal_calibration_state =
	FULL_SCALE_CALIB_STATE;
static enum calib_status adc_calibration_status[ADC_USER_CHANNELS];
static adc_calibration_configs adc_calibration_config[ADC_USER_CHANNELS];

/* IIOD channels attributes list */
static struct iio_attribute ad4130_iio_ch_attributes[] = {
	AD4130_CHN_ATTR("raw", RAW_ATTR_ID),
	AD4130_CHN_ATTR("scale", SCALE_ATTR_ID),
	AD4130_CHN_ATTR("offset", OFFSET_ATTR_ID),
	AD4130_CHN_ATTR("internal_calibration", INTERNAL_CALIB_ID),
	AD4130_CHN_ATTR("system_calibration", SYSTEM_CALIB_ID),
#if (ACTIVE_DEMO_MODE_CONFIG == LOADCELL_CONFIG)
	AD4130_CHN_ATTR("loadcell_gain_calibration", LOADCELL_GAIN_CALIB_ID),
	AD4130_CHN_ATTR("loadcell_offset_calibration", LOADCELL_OFFSET_CALIB_ID),
#endif
	END_ATTRIBUTES_ARRAY
};

/* IIOD device (global) attributes list */
static struct iio_attribute ad4130_iio_global_attributes[] = {
	AD4130_CHN_ATTR("sampling_frequency", SAMPLING_FREQ_ATTR_ID),
	AD4130_CHN_ATTR("demo_config", DEMO_CONFIG_ATTR_ID),
	END_ATTRIBUTES_ARRAY
};

static struct iio_channel ad4130_iio_channels[] = {
#if (ACTIVE_DEMO_MODE_CONFIG == THERMISTOR_CONFIG)
	AD4130_CH("Sensor1", SENSOR_CHANNEL0, IIO_TEMP)
#elif (ACTIVE_DEMO_MODE_CONFIG == RTD_3WIRE_CONFIG)
	AD4130_CH("Sensor1", SENSOR_CHANNEL0, IIO_TEMP)
#elif (ACTIVE_DEMO_MODE_CONFIG == RTD_2WIRE_CONFIG || \
	   ACTIVE_DEMO_MODE_CONFIG == RTD_4WIRE_CONFIG)
	AD4130_CH("Sensor1", SENSOR_CHANNEL0, IIO_TEMP)
#elif (ACTIVE_DEMO_MODE_CONFIG == THERMOCOUPLE_CONFIG)
	AD4130_CH("Sensor1", SENSOR_CHANNEL0, IIO_TEMP),
	AD4130_CH("CJC", CJC_CHANNEL, IIO_TEMP),
#elif (ACTIVE_DEMO_MODE_CONFIG == LOADCELL_CONFIG)
	/* Note: Channel type is considered as voltage as IIO
	 * oscilloscope doesn't support loadcell unit fomat of gram */
	AD4130_CH("Sensor1", SENSOR_CHANNEL0, IIO_VOLTAGE),
#elif (ACTIVE_DEMO_MODE_CONFIG == ECG_CONFIG)
	AD4130_CH("Sensor1", SENSOR_CHANNEL0, IIO_VOLTAGE),
#elif (ACTIVE_DEMO_MODE_CONFIG == NOISE_TEST_CONFIG)
	AD4130_CH("Chn0", 0, IIO_VOLTAGE),
#elif (ACTIVE_DEMO_MODE_CONFIG == POWER_TEST_CONFIG)
	AD4130_CH("V_AVDD", POWER_TEST_V_AVDD_CHN, IIO_VOLTAGE),
	AD4130_CH("V_IOVDD", POWER_TEST_V_IOVDD_CHN, IIO_VOLTAGE),
	AD4130_CH("I_AVDD", POWER_TEST_I_AVDD_CHN, IIO_CURRENT),
	AD4130_CH("I_IOVDD", POWER_TEST_I_IOVDD_CHN, IIO_CURRENT),
	AD4130_CH("V_AVSS-DGND", POWER_TEST_V_AVSS_DGND_CHN, IIO_VOLTAGE),
	AD4130_CH("V_REF", POWER_TEST_V_REF_CHN, IIO_VOLTAGE),
#else
	/* User default config */
	AD4130_CH("Chn0", 0, IIO_VOLTAGE),
	AD4130_CH("Chn1", 1, IIO_VOLTAGE),
	AD4130_CH("Chn2", 2, IIO_VOLTAGE),
	AD4130_CH("Chn3", 3, IIO_VOLTAGE),
	AD4130_CH("Chn4", 4, IIO_VOLTAGE),
	AD4130_CH("Chn5", 5, IIO_VOLTAGE),
	AD4130_CH("Chn6", 6, IIO_VOLTAGE),
	AD4130_CH("Chn7", 7, IIO_VOLTAGE),
#if (ADC_USER_CHANNELS > 8)
	AD4130_CH("Chn8", 8, IIO_VOLTAGE),
	AD4130_CH("Chn9", 9, IIO_VOLTAGE),
	AD4130_CH("Chn10", 10, IIO_VOLTAGE),
	AD4130_CH("Chn11", 11, IIO_VOLTAGE),
	AD4130_CH("Chn12", 12, IIO_VOLTAGE),
	AD4130_CH("Chn13", 13, IIO_VOLTAGE),
	AD4130_CH("Chn14", 14, IIO_VOLTAGE),
	AD4130_CH("Chn15", 15, IIO_VOLTAGE)
#endif
#endif
};

/* ADC raw averaged values from loadcell calibration */
static uint32_t adc_raw_offset;
static uint32_t adc_raw_gain;

/* EVB HW validation status */
static bool hw_mezzanine_is_valid;

/* Hardware board information */
static struct board_info board_info;

/******************************************************************************/
/************************ Functions Prototypes ********************************/
/******************************************************************************/

static void update_vltg_conv_scale_factor(uint8_t chn);
static void perform_sensor_measurement_and_update_scale(uint32_t adc_raw,
		uint16_t chn);
static int get_calibration_status(char *buf,
				  uint32_t len,
				  uint8_t chn,
				  intptr_t id);
static int set_calibration_routine(char *buf,
				   uint32_t len,
				   uint8_t chn,
				   intptr_t id);
static int get_loadcell_calibration_status(char *buf,
		uint32_t len,
		uint8_t chn,
		intptr_t id);
static int set_loadcell_calibration_status(char *buf,
		uint32_t len,
		uint8_t chn,
		intptr_t id);

/******************************************************************************/
/************************ Functions Definitions *******************************/
/******************************************************************************/

static char *get_demo_mode_config(void)
{
#if (ACTIVE_DEMO_MODE_CONFIG == RTD_2WIRE_CONFIG)
	return "2-Wire RTD";
#elif (ACTIVE_DEMO_MODE_CONFIG == RTD_3WIRE_CONFIG)
	return "3-Wire RTD";
#elif (ACTIVE_DEMO_MODE_CONFIG == RTD_4WIRE_CONFIG)
	return "4-Wire RTD";
#elif (ACTIVE_DEMO_MODE_CONFIG == THERMISTOR_CONFIG)
	return "Thermistor";
#elif (ACTIVE_DEMO_MODE_CONFIG == THERMOCOUPLE_CONFIG)
	return "Thermocouple";
#elif (ACTIVE_DEMO_MODE_CONFIG == LOADCELL_CONFIG)
	return "Loadcell";
#elif (ACTIVE_DEMO_MODE_CONFIG == ECG_CONFIG)
	return "ECG";
#elif (ACTIVE_DEMO_MODE_CONFIG == NOISE_TEST_CONFIG)
	return "Noise Test";
#elif (ACTIVE_DEMO_MODE_CONFIG == POWER_TEST_CONFIG)
	return "Power Test";
#else
	return "User Default";
#endif
}

/*!
 * @brief	Getter functions for AD4130 attributes
 * @param	device[in]- Pointer to IIO device instance
 * @param	buf[in]- IIO input data buffer
 * @param	len[in]- Number of input bytes
 * @param	channel[in] - Input channel
 * @param	priv[in] - Attribute private ID
 * @return	0 in case of success, negative error code otherwise
 * @Note	This sampling_frequency attribute is used to define the
 *			timeout period in IIO client during data capture.
 *			Timeout (1 chn) = (requested samples * sampling frequency) + 1sec
 *			Timeout (n chns) = ((requested samples * n) / sampling frequency) + 1sec
 *			e.g. If sampling frequency = 31.5KSPS, requested samples = 4000, n=1min or 8max
 *			Timeout (1 chn) = (4000 / 315000) + 1 = ~1.13sec
 *			Timeout (8 chns) = ((4000 * 8) / 315000) + 1 = ~2.01sec
 */
static int iio_ad4130_attr_get(void *device,
			       char *buf,
			       uint32_t len,
			       const struct iio_ch_info *channel,
			       intptr_t priv)
{
	uint32_t adc_data_raw;
	int32_t offset = 0;
	uint8_t bipolar;
	uint32_t val;
	int32_t	 ret;
	uint8_t preset = ad4130_dev_inst->ch[channel->ch_num].preset;

	val = no_os_str_to_uint32(buf);

	switch (priv) {
	case RAW_ATTR_ID:
		/* Apply calibrated coefficients before new sampling */
		if (adc_calibration_status[channel->ch_num] == CALIB_DONE) {
			ret = ad413x_reg_write(ad4130_dev_inst,
					       AD413X_REG_OFFSET(preset),
					       adc_calibration_config[channel->ch_num].offset_after_calib);
			if (ret) {
				break;
			}

			ret = ad413x_reg_write(ad4130_dev_inst,
					       AD413X_REG_GAIN(preset),
					       adc_calibration_config[channel->ch_num].gain_after_calib);
			if (ret) {
				break;
			}
		}

		/* Capture the raw adc data */
		ret = ad413x_read_single_sample(ad4130_dev_inst, (uint8_t)channel->ch_num,
						&adc_data_raw);
		if (ret) {
			break;
		}

		perform_sensor_measurement_and_update_scale(adc_data_raw, channel->ch_num);
		return sprintf(buf, "%d", adc_data_raw);

	case SCALE_ATTR_ID:
		return snprintf(buf, len, "%.10f", attr_scale_val[channel->ch_num]);

	case OFFSET_ATTR_ID:
#if (ACTIVE_DEMO_MODE_CONFIG == USER_DEFAULT_CONFIG || \
	 ACTIVE_DEMO_MODE_CONFIG == LOADCELL_CONFIG || \
	 ACTIVE_DEMO_MODE_CONFIG == ECG_CONFIG || \
	 ACTIVE_DEMO_MODE_CONFIG == NOISE_TEST_CONFIG || \
	 ACTIVE_DEMO_MODE_CONFIG == POWER_TEST_CONFIG)
		/* Note: For temperature type channels, the offset
		 * is ignored, as signed conversion needed for IIO client
		 * is done through perform_sensor_measurement_and_update_scale() */
		bipolar = ad4130_dev_inst->bipolar;
		if (bipolar) {
			/* For IIO clients, the raw to voltage conversion happens
			 * using formula: voltage = (adc_raw + offset) * scale
			 * Offset is determined based on the coding scheme of device.
			 * 'Offset binary coding' is used in bipolar mode while
			 * 'Streight binary coding' is used in unipolar mode.
			 * */
			offset = -ADC_MAX_COUNT_BIPOLAR;
		}
#endif
		return sprintf(buf, "%d", offset);

	case SAMPLING_FREQ_ATTR_ID:
		/* Sampling frequency for IIO oscilloscope timeout purpose.
		 * Does not indicate an actual sampling rate of device.
		 * Refer the 'note' in function description above for timeout calculations */
		return sprintf(buf, "%d", AD4130_MIN_SAMPLING_FREQ);

	case DEMO_CONFIG_ATTR_ID:
		return sprintf(buf, "%s", get_demo_mode_config());

	case INTERNAL_CALIB_ID:
	case SYSTEM_CALIB_ID:
		return get_calibration_status(buf, len, channel->ch_num, priv);

	case LOADCELL_GAIN_CALIB_ID:
	case LOADCELL_OFFSET_CALIB_ID:
		return get_loadcell_calibration_status(buf, len, channel->ch_num, priv);

	default:
		break;
	}

	return len;
}

/*!
 * @brief	Setter functions for AD4130 attributes
 * @param	device[in]- Pointer to IIO device instance
 * @param	buf[in]- IIO input data buffer
 * @param	len[in]- Number of input bytes
 * @param	channel[in] - Input channel
 * @param	priv[in] - Attribute private ID
 * @return	0 in case of success, negative error code otherwise
 */
static int iio_ad4130_attr_set(void *device,
			       char *buf,
			       uint32_t len,
			       const struct iio_ch_info *channel,
			       intptr_t priv)
{
	switch (priv) {
	case RAW_ATTR_ID:
	case SCALE_ATTR_ID:
	case SAMPLING_FREQ_ATTR_ID:
	case DEMO_CONFIG_ATTR_ID:
		/* All are read-only attributes */
		break;

	case INTERNAL_CALIB_ID:
	case SYSTEM_CALIB_ID:
		return set_calibration_routine(buf, len, channel->ch_num, priv);

	case LOADCELL_GAIN_CALIB_ID:
	case LOADCELL_OFFSET_CALIB_ID:
		return set_loadcell_calibration_status(buf, len, channel->ch_num, priv);

	default:
		break;
	}

	return len;
}

/*!
 * @brief	Perform the ADC internal/system calibration
 * @param	chn[in] - ADC channel
 * @param	calib_mode[in] - ADC calibration mode
 * @return	0 in case of success, negative error code otherwise
 */
static int32_t perform_adc_calibration(uint8_t chn,
				       enum ad413x_adc_mode calib_mode)
{
	int32_t ret;
	uint32_t data;
	uint8_t preset = ad4130_dev_inst->ch[chn].preset;
	uint8_t pga = ad4130_dev_inst->preset[preset].gain;

	/* Put ADC into standby mode */
	ret = ad413x_set_adc_mode(ad4130_dev_inst, AD413X_STANDBY_MODE);
	if (ret) {
		return ret;
	}

	/* Read the gain/offset coefficient values before calibration */
	if ((calib_mode == AD413X_INT_GAIN_CAL)
	    || (calib_mode == AD413X_SYS_GAIN_CAL)) {
		if (calib_mode == AD413X_INT_GAIN_CAL) {
			/* Write offset default value before internal gain calibration
			 * as internal offset calibration is performed after internal
			 * gain calibration */
			ret = ad413x_reg_write(ad4130_dev_inst,
					       AD413X_REG_OFFSET(preset),
					       AD4130_DEFAULT_OFFSET);
			if (ret) {
				return ret;
			}
		}

		ret = ad413x_reg_read(ad4130_dev_inst,
				      AD413X_REG_GAIN(preset),
				      &data);
		if (ret) {
			return ret;
		}
		adc_calibration_config[chn].gain_before_calib = data;
	} else {
		ret = ad413x_reg_read(ad4130_dev_inst,
				      AD413X_REG_OFFSET(preset),
				      &data);

		if (ret) {
			return ret;
		}
		adc_calibration_config[chn].offset_before_calib = data;
	}

	/* Enable channel for calibration */
	ret = ad413x_ch_en(ad4130_dev_inst, chn, 1);
	if (ret) {
		return ret;
	}

	if ((calib_mode == AD413X_INT_GAIN_CAL)
	    || (calib_mode == AD413X_SYS_GAIN_CAL)) {
		if ((calib_mode == AD413X_INT_GAIN_CAL) && (pga == AD413X_GAIN_1)) {
			/* Internal gain calibration is not supported at gain of 1 */
			adc_calibration_config[chn].gain_after_calib =
				adc_calibration_config[chn].gain_before_calib;
			adc_calibration_status[chn] = CALIB_SKIPPED;
			return 0;
		}

		/* Perform internal/system gain (full-scale) calibration */
		ret = ad413x_set_adc_mode(ad4130_dev_inst, calib_mode);
		if (ret) {
			return ret;
		}

		/* Wait for conversion to finish */
		no_os_mdelay(200);

		/* Read the gain coefficient value (post calibrated) */
		ret = ad413x_reg_read(ad4130_dev_inst,
				      AD413X_REG_GAIN(preset),
				      &data);
		if (ret) {
			return ret;
		}
		adc_calibration_config[chn].gain_after_calib = data;

		/* Compare the pre and post adc calibration gain coefficients to check calibration status */
		if (adc_calibration_config[chn].gain_after_calib ==
		    adc_calibration_config[chn].gain_before_calib) {
			/* Error in gain calibration */
			return -EINVAL;
		}
	} else {
		/* Perform internal/system offset (zero-scale) calibration */
		ret = ad413x_set_adc_mode(ad4130_dev_inst, calib_mode);
		if (ret) {
			return ret;
		}

		/* Wait for conversion to finish */
		no_os_mdelay(200);

		/* Read the coefficient value (post calibrated) */
		ret = ad413x_reg_read(ad4130_dev_inst,
				      AD413X_REG_OFFSET(preset),
				      &data);
		if (ret) {
			return ret;
		}
		adc_calibration_config[chn].offset_after_calib = data;

		/* Compare the pre and post adc calibration offset coefficients to check calibration status */
		if (adc_calibration_config[chn].offset_after_calib ==
		    adc_calibration_config[chn].offset_before_calib) {
			/* Error in offset calibration */
			return -EINVAL;
		}
	}

	/* Disable previously enabled channel */
	ret = ad413x_ch_en(ad4130_dev_inst, chn, 0);
	if (ret) {
		return ret;
	}

	return 0;
}

/*!
 * @brief	Getter for the ADC internal/system calibration
 * @param	buf[in]- pointer to buffer holding attribute value
 * @param	len[in]- length of buffer string data
 * @param	chn[in]- ADC channel
 * @param	id[in]- Attribute ID
 * @return	Number of characters read/written
 */
static int get_calibration_status(char *buf,
				  uint32_t len,
				  uint8_t chn,
				  intptr_t id)
{
	uint8_t buf_offset = 0;

	switch (id) {
	case SYSTEM_CALIB_ID:
	case INTERNAL_CALIB_ID:
		if (id == SYSTEM_CALIB_ID && system_calibration_state == CALIB_COMPLETE_STATE) {
			system_calibration_state = ZERO_SCALE_CALIB_STATE;
		} else if (id == INTERNAL_CALIB_ID
			   && internal_calibration_state == CALIB_COMPLETE_STATE) {
			internal_calibration_state = FULL_SCALE_CALIB_STATE;
		} else {
			if (adc_calibration_status[chn] != CALIB_ERROR
			    && adc_calibration_status[chn] != CALIB_SKIPPED
			    && adc_calibration_status[chn] != CALIB_IN_PROGRESS) {
				/* Return NA to indicate that system calibration is not supported
				 * using IIO oscilloscope. Pyadi-iio script needs to be executed
				 * to perform a system calibration due to manual intervention
				 **/
				return snprintf(buf, len, "%s", "NA");
			}
		}

		sprintf(buf + buf_offset, "%08x",
			adc_calibration_config[chn].gain_before_calib);
		buf_offset += 8;
		sprintf(buf + buf_offset, "%08x",
			adc_calibration_config[chn].gain_after_calib);
		buf_offset += 8;
		sprintf(buf + buf_offset, "%08x",
			adc_calibration_config[chn].offset_before_calib);
		buf_offset += 8;
		sprintf(buf + buf_offset, "%08x",
			adc_calibration_config[chn].offset_after_calib);
		buf_offset += 8;

		if (adc_calibration_status[chn] == CALIB_ERROR) {
			sprintf(buf + buf_offset, "%s", "calibration_failed");
			buf_offset += (strlen("calibration_failed") + 1);
			adc_calibration_status[chn] = CALIB_NOT_DONE;
		} else if (adc_calibration_status[chn] == CALIB_SKIPPED) {
			sprintf(buf + buf_offset, "%s", "calibration_skipped");
			buf_offset += (strlen("calibration_skipped") + 1);
			adc_calibration_status[chn] = CALIB_NOT_DONE;
		} else {
			sprintf(buf + buf_offset, "%s", "calibration_done");
			buf_offset += (strlen("calibration_done") + 1);
		}

		return buf_offset;

	default:
		return -EINVAL;
	}

	return len;
}

/*!
 * @brief	Setter for the ADC internal/system calibration
 * @param	buf[in]- pointer to buffer holding attribute value
 * @param	len[in]- length of buffer string data
 * @param	chn[in]- ADC channel
 * @param	id[in]- Attribute ID
 * @return	Number of characters read/written
 */
static int set_calibration_routine(char *buf,
				   uint32_t len,
				   uint8_t chn,
				   intptr_t id)
{
	switch (id) {
	case INTERNAL_CALIB_ID:
		if (!strncmp(buf, "start_calibration", strlen(buf))) {
			switch (internal_calibration_state) {
			case FULL_SCALE_CALIB_STATE:
				adc_calibration_status[chn] = CALIB_IN_PROGRESS;
				if (perform_adc_calibration(chn,
							    AD413X_INT_GAIN_CAL)) {
					adc_calibration_status[chn] = CALIB_ERROR;
				}
				internal_calibration_state = ZERO_SCALE_CALIB_STATE;
				break;

			case ZERO_SCALE_CALIB_STATE:
				if (perform_adc_calibration(chn,
							    AD413X_INT_OFFSET_CAL)) {
					adc_calibration_status[chn] = CALIB_ERROR;
					internal_calibration_state = FULL_SCALE_CALIB_STATE;
					break;
				}
				adc_calibration_status[chn] = CALIB_DONE;
				internal_calibration_state = CALIB_COMPLETE_STATE;
				break;

			case CALIB_COMPLETE_STATE:
			default:
				internal_calibration_state = FULL_SCALE_CALIB_STATE;
				break;
			}
		}
		break;

	case SYSTEM_CALIB_ID:
		if (!strncmp(buf, "start_calibration", strlen(buf))) {
			switch (system_calibration_state) {
			case ZERO_SCALE_CALIB_STATE:
				adc_calibration_status[chn] = CALIB_IN_PROGRESS;
				if (perform_adc_calibration(chn,
							    AD413X_SYS_OFFSET_CAL)) {
					adc_calibration_status[chn] = CALIB_ERROR;
				}
				system_calibration_state = FULL_SCALE_CALIB_STATE;
				break;

			case FULL_SCALE_CALIB_STATE:
				if (perform_adc_calibration(chn,
							    AD413X_SYS_GAIN_CAL)) {
					adc_calibration_status[chn] = CALIB_ERROR;
					system_calibration_state = ZERO_SCALE_CALIB_STATE;
					break;
				}
				adc_calibration_status[chn] = CALIB_DONE;
				system_calibration_state = CALIB_COMPLETE_STATE;
				break;

			case CALIB_COMPLETE_STATE:
			default:
				system_calibration_state = ZERO_SCALE_CALIB_STATE;
				break;
			}
		}
		break;

	default:
		return -EINVAL;
	}

	return len;
}

/*!
 * @brief	Getter for the Loadcell offset/gain calibration
 * @param	buf[in]- pointer to buffer holding attribute value
 * @param	len[in]- length of buffer string data
 * @param	chn[in]- ADC channel
 * @param	id[in]- Attribute ID
 * @return	Number of characters read/written
 */
static int get_loadcell_calibration_status(char *buf,
		uint32_t len,
		uint8_t chn,
		intptr_t id)
{
	switch (id) {
	case LOADCELL_OFFSET_CALIB_ID:
		return sprintf(buf, "%d", adc_raw_offset);

	case LOADCELL_GAIN_CALIB_ID:
		return sprintf(buf, "%d", adc_raw_gain);

	default:
		return -EINVAL;
	}

	return len;
}

/*!
 * @brief	Setter for the Loadcell offset/gain calibration
 * @param	buf[in]- pointer to buffer holding attribute value
 * @param	len[in]- length of buffer string data
 * @param	chn[in]- ADC channel
 * @param	id[in]- Attribute ID
 * @return	Number of characters read/written
 */
static int set_loadcell_calibration_status(char *buf,
		uint32_t len,
		uint8_t chn,
		intptr_t id)
{
	uint32_t adc_raw;
	uint8_t sample_cnt;
	uint64_t adc_raw_avg = 0;

	if (!strncmp(buf, "start_calibration", strlen(buf))) {
		switch (id) {
		case LOADCELL_OFFSET_CALIB_ID:
			for (sample_cnt = 0; sample_cnt < LOADCELL_SAMPLES_COUNT; sample_cnt++) {
				ad413x_read_single_sample(ad4130_dev_inst, chn, &adc_raw);
				adc_raw_avg += adc_raw;
			}

			adc_raw_avg /= LOADCELL_SAMPLES_COUNT;
			adc_raw_offset = (uint32_t)adc_raw_avg;
			break;

		case LOADCELL_GAIN_CALIB_ID:
			for (sample_cnt = 0; sample_cnt < LOADCELL_SAMPLES_COUNT; sample_cnt++) {
				ad413x_read_single_sample(ad4130_dev_inst, chn, &adc_raw);
				adc_raw_avg += adc_raw;
			}

			adc_raw_avg /= LOADCELL_SAMPLES_COUNT;
			adc_raw_gain = (uint32_t)adc_raw_avg;
			break;

		default:
			return -EINVAL;
		}
	}

	return len;
}

/*!
 * @brief	Read the debug register value
 * @param	dev[in]- Pointer to IIO device instance
 * @param	reg[in]- Register address to read from
 * @param	readval[in,out]- Pointer to variable to read data into
 * @return	0 in case of success or negative value otherwise
 */
int32_t debug_reg_read(void *dev, uint32_t reg, uint32_t *readval)
{
	int32_t ret;

	if (!dev || !readval || (reg > MAX_REGISTER_ADDRESS)) {
		return -EINVAL;
	}

	ret = ad413x_reg_read(dev, ad413x_regs[reg], readval);
	if (ret) {
		return ret;
	}

	return 0;
}

/*!
 * @brief	Write into the debug register
 * @param	dev[in]- Pointer to IIO device instance
 * @param	reg[in]- Register address to write into
 * @param	writeval[in]- Register value to write
 * @return	0 in case of success or negative value otherwise
 */
int32_t debug_reg_write(void *dev, uint32_t reg, uint32_t writeval)
{
	int32_t ret;

	if (!dev || (reg > MAX_REGISTER_ADDRESS)) {
		return -EINVAL;
	}

	ret = ad413x_reg_write(dev, ad413x_regs[reg], writeval);
	if (ret) {
		return ret;
	}

	return 0;
}

/*!
 * @brief	Start the burst mode data capture
 * @return	0 in case of success, negative error code otherwise
 */
int32_t start_burst_mode_capture(void)
{
	int32_t ret;

	/* Stop any previous conversion */
	ret = ad413x_set_adc_mode(ad4130_dev_inst, AD413X_STANDBY_MODE);
	if (ret) {
		return ret;
	}

	/* Trigger new conversion */
	ret = ad413x_set_adc_mode(ad4130_dev_inst, AD413X_CONTINOUS_CONV_MODE);
	if (ret) {
		return ret;
	}

	return 0;
}

/*!
 * @brief	Start the FIFO mode data capture
 * @return	0 in case of success, negative error code otherwise
 */
static int32_t start_fifo_mode_capture(void)
{
	int32_t ret;
	uint32_t fifo_control_reg_val;

	/* Read FIFO control register */
	ret = ad413x_reg_read(ad4130_dev_inst, AD413X_REG_FIFO_CTRL,
			      &fifo_control_reg_val);
	if (ret) {
		return ret;
	}

	/* Store the watermark count in FIFO */
	fifo_control_reg_val = (fifo_control_reg_val & ~AD413X_WATERMARK_MSK) |
			       AD413X_WATERMARK(FIFO_SIZE);

	/* Select the FIFO mode to enable FIFO and enable watermark interrupt */
	fifo_control_reg_val = (fifo_control_reg_val & ~AD4130_FIFO_MODE_MSK) |
			       AD413X_FIFO_MODE(FIFO_OLDEST_SAVE_MODE) |
			       AD413X_WATERMARK_INT_EN;

	/* Disable the FIFO header and status (FIFO status and header is not appended to data) */
	fifo_control_reg_val &= ~(AD413X_ADD_FIFO_HEADER | AD413X_ADD_FIFO_STATUS);

	/* Write to ADC fifo_ctrl register */
	ret = ad413x_reg_write(ad4130_dev_inst, AD413X_REG_FIFO_CTRL,
			       fifo_control_reg_val);
	if (ret) {
		return ret;
	}

	start_fifo_mode_data_capture = true;

	/* Stop any previous conversion */
	ret = ad413x_set_adc_mode(ad4130_dev_inst, AD413X_STANDBY_MODE);
	if (ret) {
		return ret;
	}

	/* Trigger new conversion */
	ret = ad413x_set_adc_mode(ad4130_dev_inst, AD413X_CONTINOUS_CONV_MODE);
	if (ret) {
		return ret;
	}

	return 0;
}

/*!
 * @brief	Capture requested number of ADC samples in FIFO mode
 * @param	iio_dev_data[in] - IIO device data instance
 * @param	nb_of_samples[in] - Number of samples to read
 * @return	0 in case of success, negative error code otherwise
 */
static int32_t read_fifo_data(struct iio_device_data *iio_dev_data,
			      uint32_t nb_of_samples)
{
	uint32_t remaining_samples = nb_of_samples;
	uint32_t timeout = BUF_READ_TIMEOUT;
	int32_t ret;

	fifo_data_available = false;

	ret = start_fifo_mode_capture();
	if (ret) {
		return ret;
	}

	/* Read all requeted samples into acquisition buffer */
	do {
		/* Wait for new FIFO event */
		timeout = BUF_READ_TIMEOUT;
		do {
			timeout--;
		} while ((!fifo_data_available) && (timeout > 0));

		if (timeout == 0) {
			return -EIO;
		}

		fifo_data_available = false;

		if (remaining_samples > FIFO_SIZE) {
			nb_of_samples = FIFO_SIZE;
			remaining_samples -= nb_of_samples;
		} else {
			nb_of_samples = remaining_samples;
			remaining_samples = 0;
		}

		/* Read data from FIFO and store into local buffer */
		ret = ad4130_read_fifo(ad4130_dev_inst, fifo_data, nb_of_samples);
		if (ret) {
			return ret;
		}

		ret = no_os_cb_write(iio_dev_data->buffer->buf, fifo_data, (nb_of_samples * BYTES_PER_SAMPLE));
		if (ret) {
			return ret;
		}
	} while (remaining_samples > 0);

	/* Stop any previous conversion */
	ret = ad413x_set_adc_mode(ad4130_dev_inst, AD413X_STANDBY_MODE);
	if (ret) {
		return ret;
	}

	start_fifo_mode_data_capture = false;

	return 0;
}

/*!
 * @brief	Capture requested number of ADC samples in burst (one-shot) mode
 * @param	iio_dev_data[in] - IIO device data instance
 * @param	nb_of_samples[in] - Number of samples to read
 * @return	0 in case of success, negative error code otherwise
 */
static int32_t read_burst_data(struct iio_device_data *iio_dev_data,
			       uint32_t nb_of_samples)
{
	uint32_t sample_index = 0;
	uint32_t adc_raw;
	int32_t ret;

	ret = start_burst_mode_capture();
	if (ret) {
		return ret;
	}

	while (sample_index < nb_of_samples) {
		ret = ad413x_mon_conv_and_read_data(ad4130_dev_inst, &adc_raw);
		if (ret) {
			return ret;
		}

		ret = no_os_cb_write(iio_dev_data->buffer->buf, &adc_raw, BYTES_PER_SAMPLE);
		if (ret) {
			return ret;
		}

		sample_index++;
	}

	/* Stop any previous conversion */
	ret = ad413x_set_adc_mode(ad4130_dev_inst, AD413X_STANDBY_MODE);
	if (ret) {
		return ret;
	}

	return 0;
}

/**
 * @brief	Read buffered data corresponding to AD4130 ADC IIO device
 * @param	iio_dev_data[in] - IIO device data instance
 * @return	0 in case of success or negative value otherwise
 */
static int32_t iio_ad4130_submit_buffer(struct iio_device_data *iio_dev_data)
{
#if ((DATA_CAPTURE_MODE == BURST_DATA_CAPTURE) || (DATA_CAPTURE_MODE == FIFO_DATA_CAPTURE))
	int32_t ret;
	uint32_t nb_of_samples;

	nb_of_samples = iio_dev_data->buffer->size / BYTES_PER_SAMPLE;
	data_indx = 0;

	if (!buf_size_updated) {
		/* Update total buffer size according to bytes per scan for proper
		 * alignment of multi-channel IIO buffer data */
		iio_dev_data->buffer->buf->size = iio_dev_data->buffer->size;
		buf_size_updated = true;
	}

#if (DATA_CAPTURE_MODE == BURST_DATA_CAPTURE)
	ret = read_burst_data(iio_dev_data, nb_of_samples);
	if (ret) {
		return ret;
	}
#else
	ret = read_fifo_data(iio_dev_data, nb_of_samples);
	if (ret) {
		return ret;
	}
#endif
#endif

	return 0;
}

/**
 * @brief	Prepare for ADC data capture (transfer from device to memory)
 * @param	dev_instance[in] - IIO device instance
 * @param	chn_mask[in] - Channels select mask
 * @return	0 in case of success, negative error code otherwise
 */
static int32_t iio_ad4130_prepare_transfer(void *dev,
		uint32_t chn_mask)
{
	uint8_t mask = 0x1;
	uint8_t chn;
	int32_t ret;

	num_of_active_channels = 0;
	data_indx = 0;
	buf_size_updated = false;

	/* Enable/Disable channels based on channel mask set in the IIO client */
	for (chn = 0; chn < ADC_USER_CHANNELS; chn++) {
		if (chn_mask & mask) {
			num_of_active_channels++;

			/* Enable the selected channel */
			ret = ad413x_ch_en(ad4130_dev_inst, chn, 1);
		} else {
			/* Disable the selected channel */
			ret = ad413x_ch_en(ad4130_dev_inst, chn, 0);
		}

		if (ret) {
			return ret;
		}

		mask <<= 1;
	}

#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
	/* Stop any previous conversion */
	ret = ad413x_set_adc_mode(ad4130_dev_inst, AD413X_STANDBY_MODE);
	if (ret) {
		return ret;
	}

	/* Trigger new conversion */
	ret = ad413x_set_adc_mode(ad4130_dev_inst, AD413X_CONTINOUS_CONV_MODE);
	if (ret) {
		return ret;
	}

	ret = iio_trig_enable(ad4130_hw_trig_desc);
	if (ret) {
		return ret;
	}
#endif

	return 0;
}

/**
 * @brief	Terminate current data transfer
 * @param	dev[in] - IIO device instance
 * @return	0 in case of success or negative value otherwise
 */
static int32_t iio_ad4130_end_transfer(void *dev)
{
	int32_t ret;

	/* Stop any active conversion */
	ret = ad413x_set_adc_mode(ad4130_dev_inst, AD413X_STANDBY_MODE);
	if (ret) {
		return ret;
	}

#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
	ret = iio_trig_disable(ad4130_hw_trig_desc);
	if (ret) {
		return ret;
	}
#endif

	return 0;
}

/**
 * @brief Push data into IIO buffer when trigger handler IRQ is invoked
 * @param iio_dev_data[in] - IIO device data instance
 * @return 0 in case of success or negative value otherwise
 */
int32_t iio_ad4130_trigger_handler(struct iio_device_data *iio_dev_data)
{
	int32_t ret;
	uint32_t adc_raw;

	if (!buf_size_updated) {
		/* Update total buffer size according to bytes per scan for proper
		 * alignment of multi-channel IIO buffer data */
		iio_dev_data->buffer->buf->size = ((uint32_t)(DATA_BUFFER_SIZE /
						   iio_dev_data->buffer->bytes_per_scan)) * iio_dev_data->buffer->bytes_per_scan;
		buf_size_updated = true;
	}

	/* Read adc conversion data for previously enabled channel in the sequencer */
	ret = ad413x_reg_read(ad4130_dev_inst, AD413X_REG_DATA, &adc_raw);
	if (ret) {
		return ret;
	}

	ret = no_os_cb_write(iio_dev_data->buffer->buf, &adc_raw, BYTES_PER_SAMPLE);
	if (ret) {
		return ret;
	}

	return 0;
}

/*!
 * @brief	This is an ISR (Interrupt Service Routine) to monitor FIFO data available event.
 *			This function is expected to be called asynchronously when data from internal device
 *			FIFO is available to read.
 * @param	ctx[in] - Callback context (unused)
 * @return	none
 */
void ad4130_fifo_event_handler(void *ctx)
{
	if (start_fifo_mode_data_capture) {
		fifo_data_available = true;
	}
}

/**
 * @brief	Perform the sensor measurement as per current demo config and update
 *			the adc_raw value to sensor conversion scale factor for IIO client
 * @param	adc_raw[in] - ADC raw value
 * @param	chn[in] -  ADC channel
 * @return	none
 */
static void perform_sensor_measurement_and_update_scale(uint32_t adc_raw,
		uint16_t chn)
{
	float temperature = 0;
	int32_t cjc_raw_data;
	float cjc_temp;

#if (ACTIVE_DEMO_MODE_CONFIG == THERMISTOR_CONFIG)
	temperature = get_ntc_thermistor_temperature(ad4130_dev_inst, adc_raw, chn);
	attr_scale_val[chn] = (temperature / adc_raw) * 1000.0;
#elif ((ACTIVE_DEMO_MODE_CONFIG == RTD_2WIRE_CONFIG) || \
	(ACTIVE_DEMO_MODE_CONFIG == RTD_3WIRE_CONFIG) || \
	(ACTIVE_DEMO_MODE_CONFIG == RTD_4WIRE_CONFIG))
	temperature = get_rtd_temperature(ad4130_dev_inst, adc_raw, chn);
	attr_scale_val[chn] = (temperature / adc_raw) * 1000.0;
#elif (ACTIVE_DEMO_MODE_CONFIG == THERMOCOUPLE_CONFIG)
	if (chn != CJC_CHANNEL) {
		/* Sample the CJC channel (TC channel is already sampled through
		 * get_raw() function) */
		if (ad413x_read_single_sample(ad4130_dev_inst, CJC_CHANNEL,
					      (uint32_t *)&cjc_raw_data)) {
			return;
		}
	} else {
		/* For calculating CJC temperature, TC raw data is not used */
		chn = SENSOR_CHANNEL0;
		cjc_raw_data = adc_raw;
		adc_raw = 0;
	}

	/* Calculate the TC and CJC temperature and update scale factor */
	temperature = get_tc_temperature(ad4130_dev_inst, adc_raw,
					 cjc_raw_data, chn, CJC_CHANNEL, &cjc_temp);
	attr_scale_val[chn] = (temperature / adc_raw) * 1000.0;
	attr_scale_val[CJC_CHANNEL] = (cjc_temp / cjc_raw_data) * 1000.0;
#endif
}

/*!
 * @brief	Update scale factor for adc data to voltage conversion
 *			for IIO client
 * @param	chn[in] - Input channel
 * @return	none
 */
static void update_vltg_conv_scale_factor(uint8_t chn)
{
	enum ad413x_gain pga;
	uint8_t preset;
	uint8_t bipolar;
	float vref;

	preset = ad4130_dev_inst->ch[chn].preset;
	pga = ad4130_dev_inst->preset[preset].gain;
	bipolar = ad4130_dev_inst->bipolar;

	vref = ad4130_get_reference_voltage(ad4130_dev_inst, chn);

	/* Get the scale factor for voltage conversion */
	if (bipolar) {
		attr_scale_val[chn] = (vref / (ADC_MAX_COUNT_BIPOLAR *
					       (1 << pga))) * 1000;
	} else {
		attr_scale_val[chn] = (vref / (ADC_MAX_COUNT_UNIPOLAR *
					       (1 << pga))) * 1000;
	}

#if (ACTIVE_DEMO_MODE_CONFIG == POWER_TEST_CONFIG)
	switch (chn) {
	case POWER_TEST_I_AVDD_CHN:
	case POWER_TEST_I_IOVDD_CHN:
		attr_scale_val[chn] /= I_RSENSE;
		break;

	case POWER_TEST_V_AVDD_CHN:
	case POWER_TEST_V_IOVDD_CHN:
		attr_scale_val[chn] *= V_SCALE;
		break;

	default:
		break;
	}
#endif
}

/**
 * @brief	Read IIO context attributes
 * @param 	params[in,out] - Pointer to IIO context attributes init param
 * @param	attrs_cnt[in,out] - IIO contxt attributes count
 * @return	0 in case of success, negative error code otherwise
 */
static int32_t get_iio_context_attributes(struct iio_cntx_attr_init *params,
		uint32_t *attrs_cnt)
{
	int32_t ret;
	struct iio_context_attribute *context_attributes;
	const char *board_status;
	uint8_t num_of_context_attributes = DEF_NUM_OF_CONTXT_ATTRS;
	uint8_t cnt = 0;

	if (!params || !attrs_cnt) {
		return -EINVAL;
	}

	if (is_eeprom_valid_dev_addr_detected()) {
		/* Read the board information from EEPROM */
		ret = read_board_info(eeprom_desc, &board_info);
		if (!ret) {
			if (!strcmp(board_info.board_id, HW_MEZZANINE_NAME)) {
				hw_mezzanine_is_valid = true;
			} else {
				hw_mezzanine_is_valid = false;
				board_status = "mismatch";
				num_of_context_attributes++;
			}
		} else {
			hw_mezzanine_is_valid = false;
			board_status = "not_detected";
			num_of_context_attributes++;
		}
	} else {
		hw_mezzanine_is_valid = false;
		board_status = "not_detected";
		num_of_context_attributes++;
	}

#if defined(FIRMWARE_VERSION)
	num_of_context_attributes++;
#endif

	/* Allocate dynamic memory for context attributes based on number of attributes
	 * detected/available */
	context_attributes = (struct iio_context_attribute *)calloc(
				     num_of_context_attributes,
				     sizeof(*context_attributes));
	if (!context_attributes) {
		return -ENOMEM;
	}

#if defined(FIRMWARE_VERSION)
	(context_attributes + cnt)->name = "fw_version";
	(context_attributes + cnt)->value = FIRMWARE_VERSION;
	cnt++;
#endif

	(context_attributes + cnt)->name = "hw_carrier";
	(context_attributes + cnt)->value = HW_CARRIER_NAME;
	cnt++;

	if (board_info.board_id[0] != '\0') {
		(context_attributes + cnt)->name = "hw_mezzanine";
		(context_attributes + cnt)->value = board_info.board_id;
		cnt++;
	}

	if (board_info.board_name[0] != '\0') {
		(context_attributes + cnt)->name = "hw_name";
		(context_attributes + cnt)->value = board_info.board_name;
		cnt++;
	}

	if (!hw_mezzanine_is_valid) {
		(context_attributes + cnt)->name = "hw_mezzanine_status";
		(context_attributes + cnt)->value = board_status;
		cnt++;
	}

	num_of_context_attributes = cnt;
	params->descriptor = context_attributes;
	*attrs_cnt = num_of_context_attributes;

	return 0;
}

/*!
 * @brief	Function to init the data capture for AD4130 device
 * @return	0 in case of success, negative error code otherwise
 */
int32_t ad4130_data_capture_init(void)
{
	int32_t ret;
	uint8_t preset;
	adc_conv_int_source_e conv_int_source;

	/* Stop any previous conversion */
	ret = ad413x_set_adc_mode(ad4130_dev_inst, AD413X_STANDBY_MODE);
	if (ret) {
		return ret;
	}

	/* Select and enable the interupt pin source for data conversion monitor */
#if defined(AD4130_WLCSP_PACKAGE_TYPE)
	conv_int_source = INT_PIN;
#else
	conv_int_source = CLK_PIN;
#endif

	ret = ad413x_set_int_source(ad4130_dev_inst, conv_int_source);
	if (ret) {
		return ret;
	}

	/* Set the filter FS value (same for all setups/channels for
	 * consistant ODR/sample rate) */
	for (preset = 0; preset <= ADC_PRESETS; preset++) {
		ret = ad413x_set_filter_fs(ad4130_dev_inst, AD4130_FS_CONFIG, preset);
		if (ret) {
			return ret;
		}
	}

	return 0;
}

/**
 * @brief Initialization of AD4130 IIO hardware trigger specific parameters
 * @param desc[in,out] - IIO hardware trigger descriptor
 * @return 0 in case of success, negative error code otherwise
 */
static int32_t ad4130_iio_trigger_param_init(struct iio_hw_trig **desc)
{
	struct iio_hw_trig_init_param ad4130_hw_trig_init_params;
	struct iio_hw_trig *hw_trig_desc;
	int32_t ret;

	hw_trig_desc = calloc(1, sizeof(struct iio_hw_trig));
	if (!hw_trig_desc) {
		return -ENOMEM;
	}

	ad4130_hw_trig_init_params.irq_id = TRIGGER_INT_ID;
	ad4130_hw_trig_init_params.name = AD4130_IIO_TRIGGER_NAME;
	ad4130_hw_trig_init_params.irq_trig_lvl = NO_OS_IRQ_EDGE_RISING;
	ad4130_hw_trig_init_params.irq_ctrl = trigger_irq_desc;
	ad4130_hw_trig_init_params.cb_info.event = NO_OS_EVT_GPIO;
	ad4130_hw_trig_init_params.cb_info.peripheral = NO_OS_GPIO_IRQ;
	ad4130_hw_trig_init_params.cb_info.handle = trigger_gpio_handle;
	ad4130_hw_trig_init_params.iio_desc = &p_ad4130_iio_desc;

	/* Initialize hardware trigger */
	ret = iio_hw_trig_init(&hw_trig_desc, &ad4130_hw_trig_init_params);
	if (ret) {
		return ret;
	}

	*desc = hw_trig_desc;

	return 0;
}

/**
 * @brief	Init for reading/writing and parameterization of a
 * 			ad4130 IIO device
 * @param 	desc[in,out] - IIO device descriptor
 * @return	0 in case of success, negative error code otherwise
 */
int32_t ad4130_iio_init(struct iio_device **desc)
{
	struct iio_device *iio_ad4130_inst;
	uint8_t chn;
	uint8_t bipolar;

	iio_ad4130_inst = calloc(1, sizeof(struct iio_device));
	if (!iio_ad4130_inst) {
		return -ENOMEM;
	}

	/* Update IIO device init parameters */
	for (chn = 0; chn < ADC_USER_CHANNELS; chn++) {
		update_vltg_conv_scale_factor(chn);
	}

	/* Get the polarity of device */
	bipolar = ad4130_dev_inst->bipolar;

	if (bipolar) {
		/* Using offset-binary coding for bipolar mode */
		chn_scan.sign = 's';
		chn_scan.realbits = CHN_STORAGE_BITS;
	} else {
		/* Using streight-binary coding for bipolar mode */
		chn_scan.sign = 'u';
		chn_scan.realbits = ADC_RESOLUTION;
	}

	chn_scan.storagebits = CHN_STORAGE_BITS;
	chn_scan.shift = 0;
	chn_scan.is_big_endian = false;

	iio_ad4130_inst->num_ch = NO_OS_ARRAY_SIZE(ad4130_iio_channels);
	iio_ad4130_inst->channels = ad4130_iio_channels;
	iio_ad4130_inst->attributes = ad4130_iio_global_attributes;

	iio_ad4130_inst->submit = iio_ad4130_submit_buffer;
	iio_ad4130_inst->pre_enable = iio_ad4130_prepare_transfer;
	iio_ad4130_inst->post_disable = iio_ad4130_end_transfer;
#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
	iio_ad4130_inst->trigger_handler = iio_ad4130_trigger_handler;
#endif

	iio_ad4130_inst->debug_reg_read = debug_reg_read;
	iio_ad4130_inst->debug_reg_write = debug_reg_write;

	*desc = iio_ad4130_inst;

	return 0;
}

/**
 * @brief Release resources allocated for IIO device
 * @param desc[in] - IIO device descriptor
 * @return 0 in case of success, negative error code otherwise
 */
int32_t ad4130_iio_remove(struct iio_desc *desc)
{
	int32_t status;

	if (!desc) {
		return -ENOMEM;
	}

	status = iio_remove(desc);
	if (status) {
		return status;
	}

	return 0;
}

/**
 * @brief	Initialize the IIO interface for AD4130 IIO device
 * @return	0 in case of success, negative error code otherwise
 */
int32_t ad4130_iio_initialize(void)
{
	int32_t init_status;

	/* IIO device descriptor */
	struct iio_device *p_iio_ad4130_dev;

#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
	static struct iio_trigger ad4130_iio_trig_desc = {
		.is_synchronous = true,
	};

	/* IIO trigger init parameters */
	static struct iio_trigger_init iio_trigger_init_params = {
		.descriptor = &ad4130_iio_trig_desc,
		.name = AD4130_IIO_TRIGGER_NAME,
	};
#endif

	/* IIO context attributes */
	static struct iio_cntx_attr_init iio_cntx_attr_init_params;

	/* IIO interface init parameters */
	static struct iio_init_param iio_init_params = {
		.phy_type = USE_UART,
#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
		.trigs = &iio_trigger_init_params,
#endif
	};

	/* IIOD init parameters */
	struct iio_device_init iio_device_init_params[NUM_OF_IIO_DEVICES] = {
		{
#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
			.trigger_id = "trigger0",
#endif
		}
	};

	/* Init the system peripherals */
	init_status = init_system();
	if (init_status) {
		return init_status;
	}

	/* Initialize AD4130 device and peripheral interface */
	init_status = ad413x_init(&ad4130_dev_inst, ad4130_init_params);
	if (init_status) {
		return init_status;
	}

	/* Read context attributes */
	init_status = get_iio_context_attributes(&iio_cntx_attr_init_params,
			&iio_init_params.nb_cntx_attrs);
	if (init_status) {
		return init_status;
	}

	if (hw_mezzanine_is_valid) {
		/* Initialize the device if HW mezzanine status is valid */
		init_status = ad4130_iio_init(&p_iio_ad4130_dev);
		if (init_status) {
			return init_status;
		}

		iio_device_init_params[0].name = ACTIVE_DEVICE_NAME;
		iio_device_init_params[0].raw_buf = adc_data_buffer;
		iio_device_init_params[0].raw_buf_len = DATA_BUFFER_SIZE;

		iio_device_init_params[0].dev = ad4130_dev_inst;
		iio_device_init_params[0].dev_descriptor = p_iio_ad4130_dev;

		iio_init_params.nb_devs++;

#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
		/* Initialize the IIO trigger specific parameters */
		init_status = ad4130_iio_trigger_param_init(&ad4130_hw_trig_desc);
		if (init_status) {
			return init_status;
		}

		iio_init_params.nb_trigs++;
#endif
	}

	/* Initialize the IIO interface */
	iio_init_params.uart_desc = uart_desc;
	iio_init_params.devs = iio_device_init_params;
	iio_init_params.cntx_attrs = &iio_cntx_attr_init_params;
	init_status = iio_init(&p_ad4130_iio_desc, &iio_init_params);
	if (init_status) {
		ad4130_iio_remove(p_ad4130_iio_desc);
		return init_status;
	}

	/* Perform data capture initialization */
	init_status = ad4130_data_capture_init();
	if (init_status) {
		return init_status;
	}

	return 0;
}

/**
 * @brief 	Run the AD4130 IIO event handler
 * @return	none
 * @details	This function monitors the new IIO client event
 */
void ad4130_iio_event_handler(void)
{
	(void)iio_step(p_ad4130_iio_desc);
}
