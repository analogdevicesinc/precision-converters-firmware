/***************************************************************************//**
 * @file    ad717x_iio.c
 * @brief   Source file for the AD717x IIO Application
********************************************************************************
* Copyright (c) 2021-23,2025-26 Analog Devices, Inc.
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
#include "ad717x_user_config.h"
#include "app_config.h"
#include "ad717x_iio.h"
#include "ad717x.h"
#include "iio.h"
#include "no_os_util.h"
#include "no_os_error.h"
#include "no_os_alloc.h"
#include "common.h"
#include "iio_trigger.h"
#include "ad717x_support.h"
#include "ad717x_system_config.h"
#include "version.h"

/******************************************************************************/
/********************* Macros and Constants Definition ************************/
/******************************************************************************/

/* Bytes per sample (*Note: 4 bytes needed per sample for data range
 * of 0 to 32-bit) */
#define	BYTES_PER_SAMPLE	sizeof(uint32_t)

/* Number of data storage bits (needed for IIO client) */
#define CHN_STORAGE_BITS	(BYTES_PER_SAMPLE * 8)

/* AD717x attribute unique IDs */
enum ad717x_attribute_ids {
	/* IIO Channel Attributes */
	AD717x_RAW_ATTR_ID,
	AD717x_SCALE_ATTR_ID,
	AD717x_OFFSET_ATTR_ID,
	AD717x_ANALOG_INPUT_ID,
	AD717x_ANALOG_INPUT_P_ID,
	AD717x_ANALOG_INPUT_N_ID,

	/* IIO Device Attributes */
	AD717x_SAMPLING_FREQUENCY_ID,
	AD717x_OPEN_WIRE_ID,
	AD717x_STATUS_ATTR_ID,
	AD717x_TEMPERATURE_ATTR_ID,
	AD717x_CALIBRATE_ATTR_ID,
	AD717x_OPEN_WIRE_INPUT_ID,
};

/* Open wire detection ADC count threshold for determining open wire condition */
#define AD717x_OW_DETECT_THRESHOLD_IN_MV	300

/* Open wire detection channel indices for single-ended inputs */
#define AD717x_OW_DETECT_SE_CHN_A		0
#define AD717x_OW_DETECT_SE_CHN_B		15

/* Open wire detection channel indices for differential inputs */
#define AD717x_OW_DETECT_DIFF_CHN_A		1
#define AD717x_OW_DETECT_DIFF_CHN_B		2

/* Temperature sensor sensitivity in V/K */
#define AD717x_TEMP_SENSITIVITY		0.000477

/* Absolute zero in degree Celsius */
#define AD717x_ABSOLUTE_ZERO_IN_CELSIUS	273.15

/* Data Buffer for burst mode data capture */
#define AD717x_DATA_BUFFER_SIZE		(8192)

/* Scan type definition */
#define AD717x_SCAN {\
	.sign = 'u',\
	.storagebits = CHN_STORAGE_BITS,\
	.shift = 0,\
	.is_big_endian = false\
}

/* Channel attribute definition */
#define AD717x_CHANNEL(_name, _priv) {\
	.name = _name,\
	.priv = _priv,\
	.show = get_adc_attribute,\
	.store = set_adc_attribute\
}

/* Channel attribute definition */
#define AD717x_CHANNEL_AVAIL(_name, _priv) {\
	.name = _name,\
	.priv = _priv,\
	.show = get_adc_available_attribute,\
	.store = set_adc_available_attribute\
}

/* AD411x Channel Definition (uses input pair attributes) */
#define IIO_AD411x_CHANNEL(_idx) {\
	.name = "ch" # _idx,\
	.ch_type = IIO_VOLTAGE,\
	.channel = _idx,\
	.scan_index = _idx,\
	.indexed = true,\
	.scan_type = &ad717x_scan_type[_idx],\
	.ch_out = false,\
	.attributes = ad411x_channel_attributes,\
}

/* AD717x Channel Definition (uses separate pos/neg input attributes) */
#define IIO_AD717x_CHANNEL(_idx) {\
	.name = "ch" # _idx,\
	.ch_type = IIO_VOLTAGE,\
	.channel = _idx,\
	.scan_index = _idx,\
	.indexed = true,\
	.scan_type = &ad717x_scan_type[_idx],\
	.ch_out = false,\
	.attributes = ad717x_channel_attributes,\
}

/* ADC data buffer size */
#if defined(USE_SDRAM_CAPTURE_BUFFER)
#define adc_data_buffer				SDRAM_START_ADDRESS
#define DATA_BUFFER_SIZE			SDRAM_SIZE_BYTES
#else
#define DATA_BUFFER_SIZE			(32768)		// 32kbytes
int8_t adc_data_buffer[DATA_BUFFER_SIZE];
#endif

/* IIO trigger name */
#define AD717X_IIO_TRIGGER_NAME		"ad717x_iio_trigger"

/* Number of IIO devices */
#define NUM_OF_IIO_DEVICES	2

/* Timeout count to avoid stuck into potential infinite loop while checking
 * for new data into an acquisition buffer. The actual timeout factor is determined
 * through 'sampling_frequency' attribute of IIO app, but this period here makes sure
 * we are not stuck into a forever loop in case data capture is interrupted
 * or failed in between.
 * Note: This timeout factor is dependent upon the MCU clock frequency. Below timeout
 * is tested for SDP-K1 platform @180Mhz default core clock */
#define AD717x_CONV_TIMEOUT	10000

/******************************************************************************/
/******************** Variables and User Defined Data Types *******************/
/******************************************************************************/

/* IIO interface descriptor */
static struct iio_desc *p_ad717x_iio_desc;

/* AD717X IIO device descriptor */
static struct iio_device *ad717x_iio_desc[NUM_OF_IIO_DEVICES];

#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
static struct iio_trigger ad717x_iio_trig_desc = {
	.is_synchronous = true,
};

/* IIO trigger init parameters */
static struct iio_trigger_init iio_trigger_init_params = {
	.descriptor = &ad717x_iio_trig_desc,
	.name = AD717X_IIO_TRIGGER_NAME,
};
#endif

/* IIO interface init parameters */
static struct iio_init_param  iio_init_params = {
	.phy_type = USE_UART,
#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
	.trigs = &iio_trigger_init_params,
#endif
};

/* Pointer to the struct representing the AD717x IIO device */
ad717x_dev *p_ad717x_dev_inst = NULL;

/* Channel scale values */
static float attr_scale_val[AD717x_MAX_CHANNELS];

/* Channel offset values */
static int attr_offset_val[AD717x_MAX_CHANNELS];

/* AD717x channel scan type */
static struct scan_type ad717x_scan_type[] = {
	AD717x_SCAN,
	AD717x_SCAN,
	AD717x_SCAN,
	AD717x_SCAN,
	AD717x_SCAN,
	AD717x_SCAN,
	AD717x_SCAN,
	AD717x_SCAN,
	AD717x_SCAN,
	AD717x_SCAN,
	AD717x_SCAN,
	AD717x_SCAN,
	AD717x_SCAN,
	AD717x_SCAN,
	AD717x_SCAN,
	AD717x_SCAN
};

/* EVB HW validation status */
static bool hw_mezzanine_is_valid;

/* AD717x IIO hw trigger descriptor */
static struct iio_hw_trig *ad717x_hw_trig_desc;

/* Flag to indicate if size of the buffer is updated according to requested
 * number of samples for the multi-channel IIO buffer data alignment */
static volatile bool buf_size_updated = false;

/* Number of active channels requested by IIO Client */
static volatile uint8_t num_of_active_channels = 0;

/* Open wire detection states */
static const char *ad717x_open_wire_options[] = {
	"not_detected",
	"detected"
};

/* Calibration mode options */
static const char *ad717x_calibrate_options[] = {
	"calibrate"
};

/* Open wire input selection options */
static const char *ad717x_open_wire_input_options[] = {
	"vin0-vin1",
	"vin2-vin3",
	"vin4-vin5",
	"vin6-vin7",
	"vin0-vincom",
	"vin1-vincom",
	"vin2-vincom",
	"vin3-vincom",
	"vin4-vincom",
	"vin5-vincom",
	"vin6-vincom",
	"vin7-vincom"
};

/* Analog input pairs corresponding to each open wire input option */
static const enum ad717x_analog_input_pairs ad717x_ow_input_pairs[] = {
	VIN0_VIN1,
	VIN2_VIN3,
	VIN4_VIN5,
	VIN6_VIN7,
	VIN0_VINCOM,
	VIN1_VINCOM,
	VIN2_VINCOM,
	VIN3_VINCOM,
	VIN4_VINCOM,
	VIN5_VINCOM,
	VIN6_VINCOM,
	VIN7_VINCOM
};

/* Number of differential open wire input options (first 4 entries) */
#define AD717x_OW_NUM_DIFF_OPTIONS	4

/* Currently selected open wire input option index */
static uint8_t ow_input_selection = 0;

/* AD411x analog input pair option strings */
static const char *ad717x_analog_input_pair_options[] = {
	"vin0-vin1",
	"vin0-vincom",
	"vin1-vin0",
	"vin1-vincom",
	"vin2-vin3",
	"vin2-vincom",
	"vin3-vin2",
	"vin3-vincom",
	"vin4-vin5",
	"vin4-vincom",
	"vin5-vin4",
	"vin5-vincom",
	"vin6-vin7",
	"vin6-vincom",
	"vin7-vin6",
	"vin7-vincom",
	"vin8-vin9",
	"vin8-vincom",
	"vin9-vin8",
	"vin9-vincom",
	"vin10-vin11",
	"vin10-vincom",
	"vin11-vin10",
	"vin11-vincom",
	"vin12-vin13",
	"vin12-vincom",
	"vin13-vin12",
	"vin13-vincom",
	"vin14-vin15",
	"vin14-vincom",
	"vin15-vin14",
	"vin15-vincom",
	"adcin11-adcin12",
	"adcin12-adcin11",
	"adcin13-adcin14",
	"adcin14-adcin13",
	"adcin11-adcin15",
	"adcin12-adcin15",
	"adcin13-adcin15",
	"adcin14-adcin15",
	"iin3_p-iin3_m",
	"iin2_p-iin2_m",
	"iin1_p-iin1_m",
	"iin0_p-iin0_m",
	"temp_sensor",
	"reference"
};

/* Enum values corresponding to ad717x_analog_input_pair_options */
static const enum ad717x_analog_input_pairs ad717x_analog_input_pair_values[] =
{
	VIN0_VIN1,
	VIN0_VINCOM,
	VIN1_VIN0,
	VIN1_VINCOM,
	VIN2_VIN3,
	VIN2_VINCOM,
	VIN3_VIN2,
	VIN3_VINCOM,
	VIN4_VIN5,
	VIN4_VINCOM,
	VIN5_VIN4,
	VIN5_VINCOM,
	VIN6_VIN7,
	VIN6_VINCOM,
	VIN7_VIN6,
	VIN7_VINCOM,
	VIN8_VIN9,
	VIN8_VINCOM,
	VIN9_VIN8,
	VIN9_VINCOM,
	VIN10_VIN11,
	VIN10_VINCOM,
	VIN11_VIN10,
	VIN11_VINCOM,
	VIN12_VIN13,
	VIN12_VINCOM,
	VIN13_VIN12,
	VIN13_VINCOM,
	VIN14_VIN15,
	VIN14_VINCOM,
	VIN15_VIN14,
	VIN15_VINCOM,
	ADCIN11_ADCIN12,
	ADCIN12_ADCIN11,
	ADCIN13_ADCIN14,
	ADCIN14_ADCIN13,
	ADCIN11_ADCIN15,
	ADCIN12_ADCIN15,
	ADCIN13_ADCIN15,
	ADCIN14_ADCIN15,
	IIN3P_IIN3M,
	IIN2P_IIN2M,
	IIN1P_IIN1M,
	IIN0P_IIN0M,
	TEMPERATURE_SENSOR,
	REFERENCE
};

/* AD717x positive/negative analog input option strings */
static const char *ad717x_analog_input_options[] = {
	"ain0",
	"ain1",
	"ain2",
	"ain3",
	"ain4",
	"ain5",
	"ain6",
	"ain7",
	"ain8",
	"ain9",
	"ain10",
	"ain11",
	"ain12",
	"ain13",
	"ain14",
	"ain15",
	"ain16",
	"temp_sensor_p",
	"temp_sensor_m",
	"avdd_avss_p",
	"avdd_avss_m",
	"ref_p",
	"ref_m"
};

/* Enum values corresponding to ad717x_analog_input_options */
static const enum ad717x_analog_input ad717x_analog_input_values[] = {
	AIN0,
	AIN1,
	AIN2,
	AIN3,
	AIN4,
	AIN5,
	AIN6,
	AIN7,
	AIN8,
	AIN9,
	AIN10,
	AIN11,
	AIN12,
	AIN13,
	AIN14,
	AIN15,
	AIN16,
	TEMP_SENSOR_P,
	TEMP_SENSOR_M,
	AVDD_AVSS_P,
	AVDD_AVSS_M,
	REF_P,
	REF_M
};

/******************************************************************************/
/************************** Functions Declaration *****************************/
/******************************************************************************/

/******************************************************************************/
/************************** Functions Definition ******************************/
/******************************************************************************/

/*!
 * @brief	Getter/Setter for the attribute value
 * @param	device[in]- pointer to IIO device structure
 * @param	buf[in]- pointer to buffer holding attribute value
 * @param	len[in]- length of buffer string data
 * @param	channel[in]- pointer to IIO channel structure
 * @param	id[in]- Attribute ID
 * @return	Number of characters read/written in case of success, negative error code otherwise
 */
static int get_adc_attribute(void *device,
			     char *buf,
			     uint32_t len,
			     const struct iio_ch_info *channel,
			     intptr_t id)
{
	float sampling_freq;
	int32_t adc_raw_data = 0;
	int32_t adc_raw_data_2 = 0;
	ad717x_st_reg *reg;
	uint32_t ow_diff;
	union ad717x_analog_inputs inp;
	float conversion_result;
	float temp;
	uint8_t ow_chn_a;
	uint8_t ow_chn_b;
	union ad717x_analog_inputs ow_inp_saved_a;
	union ad717x_analog_inputs ow_inp_saved_b;
	uint8_t indx;

	switch (id) {
	case AD717x_RAW_ATTR_ID:
		if (ad717x_single_read(device, channel->ch_num, &adc_raw_data) < 0) {
			return -EINVAL;
		}

		return sprintf(buf, "%lu", adc_raw_data);

	case AD717x_SCALE_ATTR_ID:
		return sprintf(buf, "%f", attr_scale_val[channel->ch_num]);

	case AD717x_OFFSET_ATTR_ID:
		return sprintf(buf, "%d", attr_offset_val[channel->ch_num]);

	case AD717x_SAMPLING_FREQUENCY_ID:
		if (ad717x_get_sampling_frequency(device, &sampling_freq) < 0) {
			return -EINVAL;
		}

		return sprintf(buf, "%.2f", sampling_freq);

	case AD717x_OPEN_WIRE_ID:
		/* Select channels based on differential vs single-ended input */
		if (ow_input_selection < AD717x_OW_NUM_DIFF_OPTIONS) {
			ow_chn_a = AD717x_OW_DETECT_DIFF_CHN_A;
			ow_chn_b = AD717x_OW_DETECT_DIFF_CHN_B;
		} else {
			ow_chn_a = AD717x_OW_DETECT_SE_CHN_A;
			ow_chn_b = AD717x_OW_DETECT_SE_CHN_B;
		}

		/* Save current analog input configuration for the channels */
		ow_inp_saved_a = p_ad717x_dev_inst->chan_map[ow_chn_a].analog_inputs;
		ow_inp_saved_b = p_ad717x_dev_inst->chan_map[ow_chn_b].analog_inputs;

		/* Connect the selected open wire input pair to both channels */
		inp.analog_input_pairs = ad717x_ow_input_pairs[ow_input_selection];
		if (ad717x_connect_analog_input(device, ow_chn_a, inp) != 0) {
			return -EINVAL;
		}
		if (ad717x_connect_analog_input(device, ow_chn_b, inp) != 0) {
			return -EINVAL;
		}

		/* Enable open wire detection on voltage channels */
		reg = AD717X_GetReg(p_ad717x_dev_inst, AD717X_GPIOCON_REG);
		if (!reg) {
			return -EINVAL;
		}

		if (AD717X_ReadRegister(p_ad717x_dev_inst, AD717X_GPIOCON_REG) == 0) {
			reg->value |= AD4111_GPIOCON_REG_OW_EN;

			if (AD717X_WriteRegister(p_ad717x_dev_inst, AD717X_GPIOCON_REG) != 0) {
				return -EINVAL;
			}
		} else {
			return -EINVAL;
		}

		if (ad717x_single_read(device, ow_chn_a, &adc_raw_data) < 0) {
			return -EINVAL;
		}

		if (ad717x_single_read(device, ow_chn_b, &adc_raw_data_2) < 0) {
			return -EINVAL;
		}

		/* Disable open wire detection */
		reg->value &= ~AD4111_GPIOCON_REG_OW_EN;

		if (AD717X_WriteRegister(p_ad717x_dev_inst, AD717X_GPIOCON_REG) != 0) {
			return -EINVAL;
		}

		/* Restore original analog input configuration */
		ad717x_connect_analog_input(device, ow_chn_a, ow_inp_saved_a);
		ad717x_connect_analog_input(device, ow_chn_b, ow_inp_saved_b);

		ow_diff = abs(adc_raw_data - adc_raw_data_2);

		return sprintf(buf, "%s",
			       ad717x_open_wire_options[ow_diff > (AD717x_OW_DETECT_THRESHOLD_IN_MV
								/ attr_scale_val[ow_chn_a])]);

	case AD717x_STATUS_ATTR_ID:
		reg = AD717X_GetReg(p_ad717x_dev_inst, AD717X_STATUS_REG);
		if (!reg) {
			return -EINVAL;
		}

		if (AD717X_ReadRegister(p_ad717x_dev_inst, AD717X_STATUS_REG) != 0) {
			return -EINVAL;
		}

		return sprintf(buf, "%lu", reg->value);

	case AD717x_TEMPERATURE_ATTR_ID:
		if (device_map_table[p_ad717x_dev_inst->active_device].use_input_pairs) {
			inp.analog_input_pairs = TEMPERATURE_SENSOR;
		} else {
			inp.ainp.pos_analog_input = TEMP_SENSOR_P;
			inp.ainp.neg_analog_input = TEMP_SENSOR_M;
		}
		ad717x_connect_analog_input(device, 0, inp);

		if (ad717x_single_read(device, 0, &adc_raw_data) < 0) {
			return -EINVAL;
		}

		inp = ad717x_init_params.chan_map[0].analog_inputs;
		ad717x_connect_analog_input(device, 0, inp);

		conversion_result = (((float)adc_raw_data / (1 <<
				      (device_map_table[p_ad717x_dev_inst->active_device].resolution - 1))) - 1) *
				    AD717X_INTERNAL_REFERENCE;

		// Calculate the temparture in degree celcius
		temp = ((float)conversion_result / AD717x_TEMP_SENSITIVITY) -
		       AD717x_ABSOLUTE_ZERO_IN_CELSIUS;

		return sprintf(buf,
			       "%.2f Celcius",
			       temp);

	case AD717x_CALIBRATE_ATTR_ID:
		return sprintf(buf, "%s", ad717x_calibrate_options[0]);

	case AD717x_OPEN_WIRE_INPUT_ID:
		return sprintf(buf, "%s", ad717x_open_wire_input_options[ow_input_selection]);

	case AD717x_ANALOG_INPUT_ID:
		inp.analog_input_pairs =
			p_ad717x_dev_inst->chan_map[channel->ch_num].analog_inputs.analog_input_pairs;
		for (indx = 0; indx < NO_OS_ARRAY_SIZE(ad717x_analog_input_pair_values);
		     indx++) {
			if (ad717x_analog_input_pair_values[indx] == inp.analog_input_pairs) {
				return sprintf(buf, "%s", ad717x_analog_input_pair_options[indx]);
			}
		}
		return -EINVAL;

	case AD717x_ANALOG_INPUT_P_ID:
		inp.ainp.pos_analog_input =
			p_ad717x_dev_inst->chan_map[channel->ch_num].analog_inputs.ainp.pos_analog_input;
		for (indx = 0; indx < NO_OS_ARRAY_SIZE(ad717x_analog_input_values); indx++) {
			if (ad717x_analog_input_values[indx] == inp.ainp.pos_analog_input) {
				return sprintf(buf, "%s", ad717x_analog_input_options[indx]);
			}
		}
		return -EINVAL;

	case AD717x_ANALOG_INPUT_N_ID:
		inp.ainp.neg_analog_input =
			p_ad717x_dev_inst->chan_map[channel->ch_num].analog_inputs.ainp.neg_analog_input;
		for (indx = 0; indx < NO_OS_ARRAY_SIZE(ad717x_analog_input_values); indx++) {
			if (ad717x_analog_input_values[indx] == inp.ainp.neg_analog_input) {
				return sprintf(buf, "%s", ad717x_analog_input_options[indx]);
			}
		}
		return -EINVAL;

	default:
		break;
	}

	return -EINVAL;
}


static int set_adc_attribute(void *device,
			     char *buf,
			     uint32_t len,
			     const struct iio_ch_info *channel,
			     intptr_t id)
{
	uint8_t indx;
	union ad717x_analog_inputs inp;

	switch (id) {

	case AD717x_CALIBRATE_ATTR_ID:
		for (indx = 0; indx < NO_OS_ARRAY_SIZE(ad717x_calibrate_options); indx++) {
			if (!strcmp(buf, ad717x_calibrate_options[indx])) {
				break;
			}
		}

		if (indx >= NO_OS_ARRAY_SIZE(ad717x_calibrate_options)) {
			return -EINVAL;
		}

		for (indx = 0; indx < p_ad717x_dev_inst->num_channels; indx++) {
			/* Enable the channel */
			if (ad717x_set_channel_status(device, id, true) != 0) {
				return -EINVAL;
			}

			if (ad717x_set_adc_mode(p_ad717x_dev_inst,
						INTERNAL_GAIN_CALIB) != 0) {
				return -EINVAL;
			}

			/* Wait for Calibration completion */
			if (AD717X_WaitForReady(device, AD717X_CONV_TIMEOUT) != 0) {
				return -EINVAL;
			}

			if (ad717x_set_adc_mode(p_ad717x_dev_inst,
						INTERNAL_OFFSET_CALIB) != 0) {
				return -EINVAL;
			}

			/* Wait for Calibration completion */
			if (AD717X_WaitForReady(device, AD717X_CONV_TIMEOUT) != 0) {
				return -EINVAL;
			}

			/* Disable the channel */
			if (ad717x_set_channel_status(device, id, false) != 0) {
				return -EINVAL;
			}
		}

		return len;

	case AD717x_OPEN_WIRE_INPUT_ID:
		for (indx = 0; indx < NO_OS_ARRAY_SIZE(ad717x_open_wire_input_options);
		     indx++) {
			if (!strcmp(buf, ad717x_open_wire_input_options[indx])) {
				break;
			}
		}

		if (indx >= NO_OS_ARRAY_SIZE(ad717x_open_wire_input_options)) {
			return -EINVAL;
		}

		ow_input_selection = indx;

		return len;

	case AD717x_ANALOG_INPUT_ID:
		for (indx = 0; indx < NO_OS_ARRAY_SIZE(ad717x_analog_input_pair_options);
		     indx++) {
			if (!strcmp(buf, ad717x_analog_input_pair_options[indx])) {
				break;
			}
		}

		if (indx >= NO_OS_ARRAY_SIZE(ad717x_analog_input_pair_options)) {
			return -EINVAL;
		}

		inp.analog_input_pairs = ad717x_analog_input_pair_values[indx];
		if (ad717x_connect_analog_input(device, channel->ch_num, inp) != 0) {
			return -EINVAL;
		}

		return len;

	case AD717x_ANALOG_INPUT_P_ID:
		for (indx = 0; indx < NO_OS_ARRAY_SIZE(ad717x_analog_input_options); indx++) {
			if (!strcmp(buf, ad717x_analog_input_options[indx])) {
				break;
			}
		}

		if (indx >= NO_OS_ARRAY_SIZE(ad717x_analog_input_options)) {
			return -EINVAL;
		}

		inp = p_ad717x_dev_inst->chan_map[channel->ch_num].analog_inputs;
		inp.ainp.pos_analog_input = ad717x_analog_input_values[indx];
		if (ad717x_connect_analog_input(device, channel->ch_num, inp) != 0) {
			return -EINVAL;
		}

		return len;


	case AD717x_ANALOG_INPUT_N_ID:
		for (indx = 0; indx < NO_OS_ARRAY_SIZE(ad717x_analog_input_options); indx++) {
			if (!strcmp(buf, ad717x_analog_input_options[indx])) {
				break;
			}
		}

		if (indx >= NO_OS_ARRAY_SIZE(ad717x_analog_input_options)) {
			return -EINVAL;
		}

		inp = p_ad717x_dev_inst->chan_map[channel->ch_num].analog_inputs;
		inp.ainp.neg_analog_input = ad717x_analog_input_values[indx];
		if (ad717x_connect_analog_input(device, channel->ch_num, inp) != 0) {
			return -EINVAL;
		}

		return len;

	/* ADC Raw, Scale, Offset factors are constant for the firmware configuration */
	case AD717x_RAW_ATTR_ID:
	case AD717x_SCALE_ATTR_ID:
	case AD717x_OFFSET_ATTR_ID:
	case AD717x_SAMPLING_FREQUENCY_ID:
	case AD717x_OPEN_WIRE_ID:
	case AD717x_STATUS_ATTR_ID:
	case AD717x_TEMPERATURE_ATTR_ID:
	default:
		break;
	}

	return len;
}

/*!
 * @brief	Getter for the attribute available value
 * @param	device[in]- pointer to IIO device structure
 * @param	buf[in]- pointer to buffer holding attribute value
 * @param	len[in]- length of buffer string data
 * @param	channel[in]- pointer to IIO channel structure
 * @param	id[in]- Attribute ID
 * @return	Number of characters read/written in case of success, negative error code otherwise
 */
static int get_adc_available_attribute(void *device,
				       char *buf,
				       uint32_t len,
				       const struct iio_ch_info *channel,
				       intptr_t id)
{
	uint8_t indx;
	int32_t offset = 0;
	enum ad717x_device_type dev_id = p_ad717x_dev_inst->active_device;
	enum ad717x_analog_input_pairs pair;
	enum ad717x_analog_input ain;

	switch (id) {
	case AD717x_OPEN_WIRE_ID:
		return sprintf(buf,
			       "%s %s",
			       ad717x_open_wire_options[0],
			       ad717x_open_wire_options[1]);

	case AD717x_CALIBRATE_ATTR_ID:
		return sprintf(buf,
			       "%s",
			       ad717x_calibrate_options[0]);

	case AD717x_OPEN_WIRE_INPUT_ID:
		return sprintf(buf,
			       "%s %s %s %s %s %s %s %s %s %s %s %s",
			       ad717x_open_wire_input_options[0],
			       ad717x_open_wire_input_options[1],
			       ad717x_open_wire_input_options[2],
			       ad717x_open_wire_input_options[3],
			       ad717x_open_wire_input_options[4],
			       ad717x_open_wire_input_options[5],
			       ad717x_open_wire_input_options[6],
			       ad717x_open_wire_input_options[7],
			       ad717x_open_wire_input_options[8],
			       ad717x_open_wire_input_options[9],
			       ad717x_open_wire_input_options[10],
			       ad717x_open_wire_input_options[11]);

	case AD717x_ANALOG_INPUT_ID:
		for (indx = 0; indx < NO_OS_ARRAY_SIZE(ad717x_analog_input_pair_options);
		     indx++) {
			pair = ad717x_analog_input_pair_values[indx];

			/* Common inputs for all AD411x devices: VIN0-VIN7 pairs */
			if (pair == VIN0_VIN1 || pair == VIN0_VINCOM ||
			    pair == VIN1_VIN0 || pair == VIN1_VINCOM ||
			    pair == VIN2_VIN3 || pair == VIN2_VINCOM ||
			    pair == VIN3_VIN2 || pair == VIN3_VINCOM ||
			    pair == VIN4_VIN5 || pair == VIN4_VINCOM ||
			    pair == VIN5_VIN4 || pair == VIN5_VINCOM ||
			    pair == VIN6_VIN7 || pair == VIN6_VINCOM ||
			    pair == VIN7_VIN6 || pair == VIN7_VINCOM) {
				if (offset > 0) {
					offset += sprintf(buf + offset, " ");
				}
				offset += sprintf(buf + offset, "%s",
						  ad717x_analog_input_pair_options[indx]);
			}
			/* AD4111/AD4112: current inputs */
			else if ((dev_id == ID_AD4111 || dev_id == ID_AD4112) &&
				 (pair == IIN3P_IIN3M || pair == IIN2P_IIN2M ||
				  pair == IIN1P_IIN1M || pair == IIN0P_IIN0M)) {
				offset += sprintf(buf + offset, " %s",
						  ad717x_analog_input_pair_options[indx]);
			}
			/* AD4114/AD4115/AD4116: VIN8-VIN10 pairs */
			else if ((dev_id == ID_AD4114 || dev_id == ID_AD4115 || dev_id == ID_AD4116) &&
				 (pair == VIN8_VIN9 || pair == VIN8_VINCOM ||
				  pair == VIN9_VIN8 || pair == VIN9_VINCOM ||
				  pair == VIN10_VIN11 || pair == VIN10_VINCOM)) {
				offset += sprintf(buf + offset, " %s",
						  ad717x_analog_input_pair_options[indx]);
			}
			/* AD4114/AD4115: VIN11-VIN15 pairs */
			else if ((dev_id == ID_AD4114 || dev_id == ID_AD4115) &&
				 (pair == VIN11_VIN10 || pair == VIN11_VINCOM ||
				  pair == VIN12_VIN13 || pair == VIN12_VINCOM ||
				  pair == VIN13_VIN12 || pair == VIN13_VINCOM ||
				  pair == VIN14_VIN15 || pair == VIN14_VINCOM ||
				  pair == VIN15_VIN14 || pair == VIN15_VINCOM)) {
				offset += sprintf(buf + offset, " %s",
						  ad717x_analog_input_pair_options[indx]);
			}
			/* AD4116: ADCIN inputs */
			else if (dev_id == ID_AD4116 &&
				 (pair == ADCIN11_ADCIN12 || pair == ADCIN12_ADCIN11 ||
				  pair == ADCIN13_ADCIN14 || pair == ADCIN14_ADCIN13 ||
				  pair == ADCIN11_ADCIN15 || pair == ADCIN12_ADCIN15 ||
				  pair == ADCIN13_ADCIN15 || pair == ADCIN14_ADCIN15)) {
				offset += sprintf(buf + offset, " %s",
						  ad717x_analog_input_pair_options[indx]);
			}
			/* Temperature sensor and reference for all AD411x */
			else if (pair == TEMPERATURE_SENSOR || pair == REFERENCE) {
				offset += sprintf(buf + offset, " %s",
						  ad717x_analog_input_pair_options[indx]);
			}
		}
		return offset;

	case AD717x_ANALOG_INPUT_P_ID:
	case AD717x_ANALOG_INPUT_N_ID:
		for (indx = 0; indx < NO_OS_ARRAY_SIZE(ad717x_analog_input_options); indx++) {
			ain = ad717x_analog_input_values[indx];

			/* AIN0-AIN4: available on all AD717x devices */
			if (ain >= AIN0 && ain <= AIN4) {
				if (offset > 0) {
					offset += sprintf(buf + offset, " ");
				}
				offset += sprintf(buf + offset, "%s",
						  ad717x_analog_input_options[indx]);
			}
			/* AIN5-AIN8: AD7172-4, AD7173-8, AD7175-8 */
			else if (ain >= AIN5 && ain <= AIN8 &&
				 (dev_id == ID_AD7172_4 || dev_id == ID_AD7173_8 ||
				  dev_id == ID_AD7175_8)) {
				offset += sprintf(buf + offset, " %s",
						  ad717x_analog_input_options[indx]);
			}
			/* AIN9-AIN16: AD7173-8, AD7175-8 */
			else if (ain >= AIN9 && ain <= AIN16 &&
				 (dev_id == ID_AD7173_8 || dev_id == ID_AD7175_8)) {
				offset += sprintf(buf + offset, " %s",
						  ad717x_analog_input_options[indx]);
			}
			/* TEMP_SENSOR_P/M: AD7172-2, AD7175-2, AD7177-2, AD7173-8, AD7175-8 */
			else if ((ain == TEMP_SENSOR_P || ain == TEMP_SENSOR_M) &&
				 (dev_id == ID_AD7172_2 || dev_id == ID_AD7175_2 ||
				  dev_id == ID_AD7177_2 || dev_id == ID_AD7173_8 ||
				  dev_id == ID_AD7175_8)) {
				offset += sprintf(buf + offset, " %s",
						  ad717x_analog_input_options[indx]);
			}
			/* AVDD_AVSS_P/M: AD7172-2, AD7172-4, AD7175-2, AD7175-8, AD7177-2 */
			else if ((ain == AVDD_AVSS_P || ain == AVDD_AVSS_M) &&
				 (dev_id == ID_AD7172_2 || dev_id == ID_AD7172_4 ||
				  dev_id == ID_AD7175_2 || dev_id == ID_AD7175_8 ||
				  dev_id == ID_AD7177_2)) {
				offset += sprintf(buf + offset, " %s",
						  ad717x_analog_input_options[indx]);
			}
			/* REF_P/M: all AD717x devices */
			else if (ain == REF_P || ain == REF_M) {
				offset += sprintf(buf + offset, " %s",
						  ad717x_analog_input_options[indx]);
			}
		}
		return offset;

	default:
		break;
	}

	return len;
}

/*!
 * @brief	Setter for the attribute available value
 * @param	device[in]- pointer to IIO device structure
 * @param	buf[in]- pointer to buffer holding attribute value
 * @param	len[in]- length of buffer string data
 * @param	channel[in]- pointer to IIO channel structure
 * @param	id[in]- Attribute ID
 * @return	Number of characters read/written in case of success, negative error code otherwise
 */
static int set_adc_available_attribute(void *device,
				       char *buf,
				       uint32_t len,
				       const struct iio_ch_info *channel,
				       intptr_t id)
{
	return len;
}

/* AD411x Channel Attributes */
static struct iio_attribute ad411x_channel_attributes[] = {
	AD717x_CHANNEL("raw", AD717x_RAW_ATTR_ID),
	AD717x_CHANNEL("scale", AD717x_SCALE_ATTR_ID),
	AD717x_CHANNEL("offset", AD717x_OFFSET_ATTR_ID),
	AD717x_CHANNEL("analog_input", AD717x_ANALOG_INPUT_ID),
	AD717x_CHANNEL_AVAIL("analog_input_available", AD717x_ANALOG_INPUT_ID),
	END_ATTRIBUTES_ARRAY
};

/* AD717x Channel Attributes */
static struct iio_attribute ad717x_channel_attributes[] = {
	AD717x_CHANNEL("raw", AD717x_RAW_ATTR_ID),
	AD717x_CHANNEL("scale", AD717x_SCALE_ATTR_ID),
	AD717x_CHANNEL("offset", AD717x_OFFSET_ATTR_ID),
	AD717x_CHANNEL("analog_input_p", AD717x_ANALOG_INPUT_P_ID),
	AD717x_CHANNEL_AVAIL("analog_input_p_available", AD717x_ANALOG_INPUT_P_ID),
	AD717x_CHANNEL("analog_input_n", AD717x_ANALOG_INPUT_N_ID),
	AD717x_CHANNEL_AVAIL("analog_input_n_available", AD717x_ANALOG_INPUT_N_ID),
	END_ATTRIBUTES_ARRAY
};

/* AD717x Global Attributes (with open wire detection) */
static struct iio_attribute iio_ad717x_global_attributes_with_ow[] = {
	AD717x_CHANNEL("sampling_frequency", AD717x_SAMPLING_FREQUENCY_ID),
	AD717x_CHANNEL("open_wire", AD717x_OPEN_WIRE_ID),
	AD717x_CHANNEL_AVAIL("open_wire_available", AD717x_OPEN_WIRE_ID),
	AD717x_CHANNEL("open_wire_input", AD717x_OPEN_WIRE_INPUT_ID),
	AD717x_CHANNEL_AVAIL("open_wire_input_available", AD717x_OPEN_WIRE_INPUT_ID),
	AD717x_CHANNEL("status", AD717x_STATUS_ATTR_ID),
	AD717x_CHANNEL("temperature", AD717x_TEMPERATURE_ATTR_ID),
	AD717x_CHANNEL("calibrate", AD717x_CALIBRATE_ATTR_ID),
	AD717x_CHANNEL_AVAIL("calibrate_available", AD717x_CALIBRATE_ATTR_ID),

	END_ATTRIBUTES_ARRAY
};

/* AD717x Global Attributes (without open wire detection) */
static struct iio_attribute iio_ad717x_global_attributes[] = {
	AD717x_CHANNEL("sampling_frequency", AD717x_SAMPLING_FREQUENCY_ID),
	AD717x_CHANNEL("status", AD717x_STATUS_ATTR_ID),
	AD717x_CHANNEL("temperature", AD717x_TEMPERATURE_ATTR_ID),
	AD717x_CHANNEL("calibrate", AD717x_CALIBRATE_ATTR_ID),
	AD717x_CHANNEL_AVAIL("calibrate_available", AD717x_CALIBRATE_ATTR_ID),

	END_ATTRIBUTES_ARRAY
};

/* IIO channels for AD411x family (input pair selection) */
static struct iio_channel iio_ad411x_channels[] = {
	IIO_AD411x_CHANNEL(0),
	IIO_AD411x_CHANNEL(1),
	IIO_AD411x_CHANNEL(2),
	IIO_AD411x_CHANNEL(3),
	IIO_AD411x_CHANNEL(4),
	IIO_AD411x_CHANNEL(5),
	IIO_AD411x_CHANNEL(6),
	IIO_AD411x_CHANNEL(7),
	IIO_AD411x_CHANNEL(8),
	IIO_AD411x_CHANNEL(9),
	IIO_AD411x_CHANNEL(10),
	IIO_AD411x_CHANNEL(11),
	IIO_AD411x_CHANNEL(12),
	IIO_AD411x_CHANNEL(13),
	IIO_AD411x_CHANNEL(14),
	IIO_AD411x_CHANNEL(15)
};

/* IIO channels for AD717x family (separate pos/neg input selection) */
static struct iio_channel iio_ad717x_channels[] = {
	IIO_AD717x_CHANNEL(0),
	IIO_AD717x_CHANNEL(1),
	IIO_AD717x_CHANNEL(2),
	IIO_AD717x_CHANNEL(3),
	IIO_AD717x_CHANNEL(4),
	IIO_AD717x_CHANNEL(5),
	IIO_AD717x_CHANNEL(6),
	IIO_AD717x_CHANNEL(7),
	IIO_AD717x_CHANNEL(8),
	IIO_AD717x_CHANNEL(9),
	IIO_AD717x_CHANNEL(10),
	IIO_AD717x_CHANNEL(11),
	IIO_AD717x_CHANNEL(12),
	IIO_AD717x_CHANNEL(13),
	IIO_AD717x_CHANNEL(14),
	IIO_AD717x_CHANNEL(15)
};

/**
 * @brief Read the debug register value
 * @param desc[in,out] - Pointer to IIO device descriptor
 * @param reg[in]- Address of the register to be read
 * @param readval[out]- Pointer to the register data variable
 * @return 0 in case of success, negative error code
 */
static int32_t iio_ad717x_debug_reg_read(void *dev,
		uint32_t reg,
		uint32_t *readval)
{
	int32_t debug_read_status; // Status check variable

	/* Retrieve the pointer to the requested register */
	ad717x_st_reg *read_register =  AD717X_GetReg(p_ad717x_dev_inst, reg);
	if (!read_register) {
		return -EINVAL;
	}

	/* Read the register data, extract the value */
	debug_read_status = AD717X_ReadRegister(p_ad717x_dev_inst, reg);
	if (debug_read_status) {
		return debug_read_status;
	}
	*readval = read_register->value;

	return 0;
}


/**
 * @brief Write value to the debug register
 * @param desc[in,out] Pointer to IIO device descriptor
 * @param reg[in] Address of the register where the data is to be written
 * @param write_val[out] Pointer to the register data variable
 * @return 0 in case of success, negative error code otherwise
 */
static int32_t iio_ad717x_debug_reg_write(void *dev,
		uint32_t reg,
		uint32_t write_val)
{
	int32_t debug_write_status;	// Status check variable
	ad717x_st_reg *device_data_reg;  // Pointer to data register

	/* Retrieve the pointer to the requested registrer */
	device_data_reg = AD717X_GetReg(p_ad717x_dev_inst, reg);
	if (!device_data_reg) {
		return -EINVAL;
	}
	device_data_reg->value = write_val;

	/* Write value to the register */
	debug_write_status = AD717X_WriteRegister(p_ad717x_dev_inst, reg);
	if (debug_write_status) {
		return debug_write_status;
	}

	return 0;
}

/*!
 * @brief Function to prepare the ADC for continuous capture
 * @return 0 in case of success, negative error code otherwise
 */
int32_t ad717x_trigger_cont_data_capture(void)
{
	int32_t ret;

	/* Set ADC to Continuous conversion mode */
	ret = ad717x_set_adc_mode(p_ad717x_dev_inst, CONTINUOUS);
	if (ret) {
		return ret;
	}

	/* Enable Continuous read operation */
	ret = ad717x_enable_cont_read(p_ad717x_dev_inst, true);
	if (ret) {
		return ret;
	}

	/* Pull the cs line low to detect the EOC bit during data capture */
	ret = no_os_gpio_set_value(csb_gpio, NO_OS_GPIO_LOW);
	if (ret) {
		return ret;
	}

	return ret;
}

/*!
 * @brief Function to stop continuous data capture
 * @return 0 in case of success, negative error code otherwise
 */
int32_t ad717x_stop_cont_data_capture(void)
{
	int32_t ret = 0;
	int32_t adc_raw_data;
	uint8_t rdy_value = NO_OS_GPIO_HIGH;
	uint32_t timeout = AD717x_CONV_TIMEOUT;

	/* Read the data register when RDY is low to exit continuous read mode */
	do {
		ret = no_os_gpio_get_value(rdy_gpio, &rdy_value);
		if (ret) {
			return ret;
		}
		timeout--;
	} while ((rdy_value != NO_OS_GPIO_LOW) && (timeout > 0));

	if (timeout == 0) {
		return -ETIMEDOUT;
	}

	ret = AD717X_ReadData(p_ad717x_dev_inst, &adc_raw_data);
	if (ret) {
		return ret;
	}

	/* Disable continous read mode */
	ret = ad717x_enable_cont_read(p_ad717x_dev_inst, false);
	if (ret) {
		return ret;
	}

	return ret;
}

/**
 * @brief Prepare for ADC data capture (transfer from device to memory)
 * @param dev_instance[in] - IIO device instance
 * @param ch_mask[in] - Channels select mask
 * @return 0 in case of success or negative value otherwise
 */
static int32_t iio_ad717x_prepare_transfer(void *dev_instance,
		uint32_t chn_mask)
{
	int32_t ret;
	uint8_t ch_id;
	uint32_t mask = 0x1;
	num_of_active_channels = 0;
	buf_size_updated = false;

#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
#if defined(USE_VIRTUAL_COM_PORT)
	/* TODO : spikes being observed in VCOM
	 * Added as temporary fix to the above issue.
	 * Makes the UART Interrupt low prior than the GPIO Interrupt
	 */
	ret = no_os_irq_set_priority(trigger_irq_desc, IRQ_INT_ID, 0);
	if (ret) {
		return ret;
	}

	HAL_NVIC_SetPriority(OTG_HS_IRQn, 1, 1);
#else
	ret = no_os_irq_set_priority(trigger_irq_desc, IRQ_INT_ID, RDY_GPIO_PRIORITY);
	if (ret) {
		return ret;
	}
#endif
#endif

	/* Enable requested channels and Disable the remaining */
	for (ch_id = 0;
	     ch_id < p_ad717x_dev_inst->num_channels; ch_id++) {
		if (chn_mask & mask) {
			ret = ad717x_set_channel_status(p_ad717x_dev_inst, ch_id, true);
			if (ret) {
				return ret;
			}
			num_of_active_channels++;
		} else {
			ret = ad717x_set_channel_status(p_ad717x_dev_inst, ch_id, false);
			if (ret) {
				return ret;
			}
		}
		mask <<= 1;
	}

#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
	ret = ad717x_trigger_cont_data_capture();
	if (ret) {
		return ret;
	}

	/* Clear pending Interrupt before enabling back the trigger.
	 * Else , a spurious interrupt is observed after a legitimate interrupt,
	 * as SPI SDO is on the same pin and is mistaken for an interrupt event */
	ret = no_os_irq_clear_pending(trigger_irq_desc, IRQ_INT_ID);
	if (ret) {
		return ret;
	}

	ret = iio_trig_enable(ad717x_hw_trig_desc);
	if (ret) {
		return ret;
	}
#endif

	return ret;
}


/**
 * @brief Read buffer data corresponding to AD4170 IIO device
 * @param iio_dev_data[in] - Pointer to IIO device data structure
 * @return 0 in case of success or negative value otherwise
 */
static int32_t iio_ad717x_submit_buffer(struct iio_device_data *iio_dev_data)
{
#if (DATA_CAPTURE_MODE == BURST_DATA_CAPTURE)
	int32_t ret;
	uint32_t sample_index = 0;
	uint32_t nb_of_samples;
	uint32_t adc_raw_data = 0;

	nb_of_samples = (iio_dev_data->buffer->size / BYTES_PER_SAMPLE);
	if (!buf_size_updated) {
		/* Update total buffer size according to bytes per scan for proper
		 * alignment of multi-channel IIO buffer data */
		iio_dev_data->buffer->buf->size = iio_dev_data->buffer->size;
		buf_size_updated = true;
	}

	/* Set ADC to continuous conversion mode */
	ret = ad717x_set_adc_mode(p_ad717x_dev_inst, CONTINUOUS);
	if (ret) {
		return ret;
	}

	while (sample_index < nb_of_samples) {
		/* Wait for RDY Low */
		ret = AD717X_WaitForReady(p_ad717x_dev_inst, AD717x_CONV_TIMEOUT);
		if (ret) {
			return ret;
		}

		/* Read ADC Data */
		ret = AD717X_ReadData(p_ad717x_dev_inst, &adc_raw_data);
		if (ret) {
			return ret;
		}

		/* Write to CB */
		ret = no_os_cb_write(iio_dev_data->buffer->buf, &adc_raw_data,
				     BYTES_PER_SAMPLE);
		if (ret) {
			return ret;
		}
		sample_index++;
	}

	/* Set ADC to standby mode */
	ret = ad717x_set_adc_mode(p_ad717x_dev_inst, STANDBY);
	if (ret) {
		return ret;
	}
#endif
	return 0;
}


/**
 * @brief Perform tasks before end of current data transfer
 * @param dev[in] - IIO device instance
 * @return 0 in case of success or negative value otherwise
 */
static int32_t iio_ad717x_end_transfer(void *dev)
{
	int32_t ret;
	uint8_t ch_id;

#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
	ret = iio_trig_disable(ad717x_hw_trig_desc);
	if (ret) {
		return ret;
	}

	ret =  ad717x_stop_cont_data_capture();
	if (ret) {
		return ret;
	}
#endif

	/* Set ADC to Standby mode */
	ret = ad717x_set_adc_mode(p_ad717x_dev_inst, STANDBY);
	if (ret) {
		return ret;
	}

	/* Disable all channels */
	for (ch_id = 0; ch_id < p_ad717x_dev_inst->num_channels; ch_id++) {
		ret = ad717x_set_channel_status(p_ad717x_dev_inst, ch_id, false);
		if (ret) {
			return ret;
		}
	}

	return 0;
}

/**
 * @brief Initialization of AD717x IIO hardware trigger specific parameters
 * @param desc[in,out] - IIO hardware trigger descriptor
 * @return 0 in case of success, negative error code otherwise
 */
static int32_t ad717x_iio_trigger_param_init(struct iio_hw_trig **desc)
{
	struct iio_hw_trig_init_param ad717x_hw_trig_init_params;
	struct iio_hw_trig *hw_trig_desc;
	int32_t ret;

	ad717x_hw_trig_init_params.irq_id = IRQ_INT_ID;
	ad717x_hw_trig_init_params.name = AD717X_IIO_TRIGGER_NAME;
	ad717x_hw_trig_init_params.irq_trig_lvl = NO_OS_IRQ_EDGE_FALLING;
	ad717x_hw_trig_init_params.irq_ctrl = trigger_irq_desc;
	ad717x_hw_trig_init_params.cb_info.event = NO_OS_EVT_GPIO;
	ad717x_hw_trig_init_params.cb_info.peripheral = NO_OS_GPIO_IRQ;
	ad717x_hw_trig_init_params.cb_info.handle = trigger_gpio_handle;
	ad717x_hw_trig_init_params.iio_desc = p_ad717x_iio_desc;

	/* Initialize hardware trigger */
	ret = iio_hw_trig_init(&hw_trig_desc, &ad717x_hw_trig_init_params);
	if (ret) {
		return ret;
	}

	*desc = hw_trig_desc;

	return 0;
}

/*
 * @brief Push data into IIO buffer when trigger handler IRQ is invoked
 * @param iio_dev_data[in] - IIO device data instance
 * @return 0 in case of success or negative value otherwise
 */
int32_t ad717x_trigger_handler(struct iio_device_data *iio_dev_data)
{
	int32_t ret;
	uint32_t adc_read_back = 0;

	ret = iio_trig_disable(ad717x_hw_trig_desc);
	if (ret) {
		return ret;
	}

	if (!buf_size_updated) {
		/* Update total buffer size according to bytes per scan for proper
		 * alignment of multi-channel IIO buffer data */
		iio_dev_data->buffer->buf->size = ((uint32_t)(DATA_BUFFER_SIZE /
						   iio_dev_data->buffer->bytes_per_scan)) * iio_dev_data->buffer->bytes_per_scan;
		buf_size_updated = true;
	}

	ret = ad717x_adc_read_converted_sample(&adc_read_back);
	if (ret) {
		return ret;
	}

	ret = no_os_cb_write(iio_dev_data->buffer->buf,
			     &adc_read_back,
			     BYTES_PER_SAMPLE);
	if (ret) {
		return ret;
	}

	/* Clear pending Interrupt before enabling back the trigger.
	 * Else , a spurious interrupt is observed after a legitimate interrupt,
	 * as SPI SDO is on the same pin and is mistaken for an interrupt event */
	ret = no_os_irq_clear_pending(trigger_irq_desc, IRQ_INT_ID);
	if (ret) {
		return ret;
	}

	/* Enable back the external interrupts */
	ret = iio_trig_enable(ad717x_hw_trig_desc);
	if (ret) {
		return ret;
	}

	return 0;
}

/**
 * @brief Init for reading/writing and parameterization of a AD717x IIO device
 * @param desc[in,out] IIO device descriptor
 * @return 0 in case of success, negative error code otherwise
 */
int32_t iio_ad717x_init(struct iio_device **desc)
{
	struct iio_device *iio_ad717x_inst;  // IIO Device Descriptor for AD717x

	iio_ad717x_inst = no_os_calloc(1, sizeof(struct iio_device));
	if (!iio_ad717x_inst) {
		return -EINVAL;
	}

	iio_ad717x_inst->num_ch = p_ad717x_dev_inst->num_channels;

	/* Select appropriate channel array based on device family */
	if (device_map_table[p_ad717x_dev_inst->active_device].use_input_pairs) {
		iio_ad717x_inst->channels = iio_ad411x_channels;
	} else {
		iio_ad717x_inst->channels = iio_ad717x_channels;
	}

	if (device_map_table[p_ad717x_dev_inst->active_device].supports_open_wire) {
		iio_ad717x_inst->attributes = iio_ad717x_global_attributes_with_ow;
	} else {
		iio_ad717x_inst->attributes = iio_ad717x_global_attributes;
	}
	iio_ad717x_inst->buffer_attributes = NULL;
	iio_ad717x_inst->pre_enable = iio_ad717x_prepare_transfer;
	iio_ad717x_inst->post_disable = iio_ad717x_end_transfer;
	iio_ad717x_inst->submit = iio_ad717x_submit_buffer;
	iio_ad717x_inst->debug_reg_read = iio_ad717x_debug_reg_read;
	iio_ad717x_inst->debug_reg_write = iio_ad717x_debug_reg_write;
#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
	iio_ad717x_inst->trigger_handler = ad717x_trigger_handler;
#endif

	*desc = iio_ad717x_inst;

	return 0;
}


/**
 * @brief Function to update scale and offset values based on user selection
 * @param device[in] AD717x device descriptor
 * @return 0 in case of success, negative error code otherwise
 */
static int32_t ad717x_update_attr_parameters(ad717x_dev *device)
{
	float reference_value = 0; // Variable to hold the updated reference value
	float scale_factor = device_map_table[device->active_device].scale_factor;
	uint8_t dev_resolution = device_map_table[device->active_device].resolution;
	uint32_t adc_max_count_unipolar = (uint32_t)((1 << dev_resolution) - 1);
	uint32_t adc_max_count_bipolar = (uint32_t)(1 << (dev_resolution - 1));
	uint8_t i;

	for (i = 0; i < device->num_channels; i++) {
		/* Update reference value */
		switch (device->setups[device->chan_map[i].setup_sel].ref_source) {
		case INTERNAL_REF:
			reference_value = AD717X_INTERNAL_REFERENCE;
			break;
		case EXTERNAL_REF:
			reference_value = AD717x_EXTERNAL_REFERENCE;
			break;
		case AVDD_AVSS:
			reference_value = AD717X_AVDD_AVSS_REFERENCE;
			break;
		default:
			return -EINVAL;
		}

		/* Update channel attribute parameters */
		if (!(device->setups[device->chan_map[i].setup_sel].bi_unipolar)) {
			/* Settings for Unipolar mode */
			attr_scale_val[i] = ((reference_value / adc_max_count_unipolar) * 1000) /
					    scale_factor;
			attr_offset_val[i] = 0;
			ad717x_scan_type[i].sign = 'u';
		} else {
			/* Settings for Bipolar mode */
			attr_scale_val[i] = ((reference_value / adc_max_count_bipolar) * 1000) /
					    scale_factor;
			attr_offset_val[i] = -(1 << (dev_resolution - 1));
			ad717x_scan_type[i].sign = 'u';
		}
		ad717x_scan_type[i].realbits = dev_resolution;
	}

	return 0;
}

/**
 * @brief Assign device name and populate init parameters from the device map table
 * @param dev_id[in] - The device id (index into device_map_table)
 * @param dev_name[out] - Pointer to store the device name string
 * @return 0 in case of success, negative error code otherwise.
 */
static int32_t ad717x_assign_device(enum ad717x_device_type dev_id,
				    char** dev_name)
{
	uint8_t indx;

	/* Retrieve the device name from the lookup table */
	*dev_name = (char*)device_map_table[dev_id].name;

	/* Populate the init parameters with device-specific register map,
	 * channel count, and setup count from the device map table */
	ad717x_init_params.num_regs = device_map_table[dev_id].num_regs;
	ad717x_init_params.regs = device_map_table[dev_id].regs;
	ad717x_init_params.active_device = dev_id;
	ad717x_init_params.num_channels = device_map_table[dev_id].num_channels;
	ad717x_init_params.num_setups = device_map_table[dev_id].num_setups;

	/* Initialize all setups with default configuration and
	 * assign the selected ODR value to each filter configuration */
	for (indx = 0; indx < ad717x_init_params.num_setups; indx++) {
		ad717x_init_params.setups[indx] = default_setups[indx];
		ad717x_init_params.filter_configuration[indx].odr =
			default_ad717x_filtcons[indx].odr;

		/* AD7177-2 only supports ODR register values starting from index 7 */
		if (dev_id == ID_AD7177_2 &&
		    ad717x_init_params.filter_configuration[indx].odr < sps_10417) {
			ad717x_init_params.filter_configuration[indx].odr = sps_10417;
		}
	}

	/* Configure channel maps: AD411x family uses input pairs (voltage input
	 * mux), while AD717x family uses individual analog input selection */
	for (indx = 0; indx < ad717x_init_params.num_channels; indx++) {
		if (device_map_table[dev_id].use_input_pairs) {
			ad717x_init_params.chan_map[indx] = default_ad411x_chan_maps[indx];
		} else {
			ad717x_init_params.chan_map[indx] = default_ad717x_chan_maps[indx];
		}
	}

	return 0;
}


/**
 * @brief	Remove the IIO interface for AD717X IIO device
 * @return	0 in case of success,negative error code otherwise
 */
int32_t iio_app_remove(void)
{
#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
	iio_hw_trig_remove(ad717x_hw_trig_desc);
#endif

	/* Remove IIO */
	iio_remove(p_ad717x_iio_desc);

	/* Free System Config IIO device */
	iio_init_params.nb_devs--;
	iio_ad717x_system_config_remove(ad717x_iio_desc[iio_init_params.nb_devs]);

	/* Free AD717x IIO device */
	iio_init_params.nb_devs--;
	no_os_free(ad717x_iio_desc[iio_init_params.nb_devs]);
	ad717x_iio_desc[iio_init_params.nb_devs] = NULL;

	/* Remove AD717x descriptor */
	AD717X_remove(p_ad717x_dev_inst);

	/* Remove context attributes */
	remove_iio_context_attributes(iio_init_params.ctx_attrs);

	/* Reset the number of triggers */
	iio_init_params.nb_trigs = 0;

	return 0;
}

/**
 * @brief 	Initialize the AD717x IIO Interface
 * @return	0 in case of success, negative error code otherwise
 */
int32_t iio_app_initialize(void)
{
	int32_t iio_init_status;		    // Status check variable
	enum ad717x_device_type dev_type;
	uint8_t indx;

	/* Read context attributes */
	static const char *mezzanine_names[] = {
		"Eval-AD4111SDZ",
		"EVAL-AD4112SDZ",
		"EVAL-AD4113ARDZ",
		"EVAL-AD4114SDZ",
		"EVAL-AD4115SDZ",
		"EVAL-AD4115SDZ",
		"EVAL-AD7172-2SDZ",
		"EVAL-AD7172-4SDZ",
		"EVAL-AD7173-8SDZ",
		"EVAL-AD7175-2SDZ",
		"EVAL-AD7175-8SDZ",
		"EVAL-AD7176-2SDZ",
		"EVAL-AD7177-2SDZ"
	};

	/* IIOD init parameters */
	struct iio_device_init iio_device_init_params[NUM_OF_IIO_DEVICES] = {
		{
#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
			.trigger_id = "trigger0"
#endif
		}
	};

	/* Iterate twice to detect the correct attached board */
	for (indx = 0; indx < NO_OS_ARRAY_SIZE(mezzanine_names); indx++) {
		iio_init_status = get_iio_context_attributes_ex(&iio_init_params.ctx_attrs,
				  &iio_init_params.nb_ctx_attr,
				  eeprom_desc,
				  mezzanine_names[indx],
				  STR(HW_CARRIER_NAME),
				  &hw_mezzanine_is_valid,
				  FIRMWARE_VERSION);
		if (iio_init_status) {
			return iio_init_status;
		}

		if (hw_mezzanine_is_valid) {
			dev_type = indx;
			break;
		}

		if (indx != NO_OS_ARRAY_SIZE(mezzanine_names) - 1) {
			iio_init_status = remove_iio_context_attributes(iio_init_params.ctx_attrs);
			if (iio_init_status) {
				return iio_init_status;
			}
		}
	}

	if (hw_mezzanine_is_valid) {
		do {
			/* Initialize AD717X device and peripheral interface */
			iio_init_status = ad717x_assign_device(dev_type,
							       &iio_device_init_params[0].name);
			if (iio_init_status) {
				break;
			}

			/* Initialize AD717x device */
			iio_init_status = AD717X_Init(&p_ad717x_dev_inst, ad717x_init_params);
			if (iio_init_status) {
				break;
			}

			for (indx = 0; indx < ad717x_init_params.num_setups; indx++) {
				/* Update filter */
				iio_init_status = ad717x_write_filter_order(p_ad717x_dev_inst,
						  default_ad717x_filtcons[indx].oder,
						  indx);
				if (iio_init_status) {
					/* Free AD717x device */
					AD717X_remove(p_ad717x_dev_inst);
					break;
				}

				/* Update post filter */
				iio_init_status = ad717x_write_post_filter(p_ad717x_dev_inst,
						  default_ad717x_filtcons[indx].enhfilten,
						  default_ad717x_filtcons[indx].enhfilt,
						  indx);
				if (iio_init_status) {
					/* Free AD717x device */
					AD717X_remove(p_ad717x_dev_inst);
					break;
				}
			}

			/* Initialize the AD717x IIO Interface */
			iio_init_status = iio_ad717x_init(&ad717x_iio_desc[iio_init_params.nb_devs]);
			if (iio_init_status) {
				/* Free AD717x device */
				AD717X_remove(p_ad717x_dev_inst);
				break;
			}

			iio_device_init_params[iio_init_params.nb_devs].raw_buf = adc_data_buffer;
			iio_device_init_params[iio_init_params.nb_devs].raw_buf_len = DATA_BUFFER_SIZE;

			iio_device_init_params[iio_init_params.nb_devs].dev = p_ad717x_dev_inst;
			iio_device_init_params[iio_init_params.nb_devs].dev_descriptor =
				ad717x_iio_desc[iio_init_params.nb_devs];

			iio_init_params.nb_devs++;

			/* Initialize system config device */
			iio_init_status = iio_ad717x_system_config_init(
						  &ad717x_iio_desc[iio_init_params.nb_devs],
						  p_ad717x_dev_inst);
			if (iio_init_status) {
				iio_init_params.nb_devs--;
				no_os_free(ad717x_iio_desc[iio_init_params.nb_devs]);
				AD717X_remove(p_ad717x_dev_inst);
				break;
			}

			iio_device_init_params[1].name = "system_config";
			iio_device_init_params[1].dev = p_ad717x_dev_inst;
			iio_device_init_params[1].dev_descriptor =
				ad717x_iio_desc[iio_init_params.nb_devs];
			iio_init_params.nb_devs++;

#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
			iio_init_params.nb_trigs++;
#endif

			/* Update the ADC scale respective to the device settings */
			iio_init_status = ad717x_update_attr_parameters(p_ad717x_dev_inst);
			if (iio_init_status) {
				iio_init_params.nb_devs--;
				iio_ad717x_system_config_remove(ad717x_iio_desc[iio_init_params.nb_devs]);
				iio_init_params.nb_devs--;
				no_os_free(ad717x_iio_desc[iio_init_params.nb_devs]);
				AD717X_remove(p_ad717x_dev_inst);
				break;
			}
		} while (false);
	}

	/* Initialize the IIO Interface */
	iio_init_params.uart_desc = uart_desc;
	iio_init_params.devs = iio_device_init_params;
	iio_init_status = iio_init(&p_ad717x_iio_desc, &iio_init_params);
	if (iio_init_status) {
		return iio_init_status;
	}

#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
	/* Initialize the AD717x IIO trigger specific parameters */
	iio_init_status = ad717x_iio_trigger_param_init(&ad717x_hw_trig_desc);
	if (iio_init_status) {
		return iio_init_status;
	}
#endif

	return 0;
}

/**
 * @brief 	Run the AD717x IIO event handler
 * @return	None
 */
void iio_app_event_handler(void)
{
	if (iio_ad717x_is_system_reconfigured()) {
		/* Remove the resources allocated by previous init */
		iio_app_remove();

		/* Re-initialize IIO with updated hardware state */
		iio_app_initialize();

		return;
	}

	(void)iio_step(p_ad717x_iio_desc);
}
