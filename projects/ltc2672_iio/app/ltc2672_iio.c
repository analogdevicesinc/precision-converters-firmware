/***************************************************************************//**
 *   @file    ltc2672_iio.c
 *   @brief   Implementation of LTC2672 IIO application interfaces
********************************************************************************
 * Copyright (c) 2023 Analog Devices, Inc.
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

#include "app_config.h"
#include "ltc2672_iio.h"
#include "ltc2672_user_config.h"
#include "common.h"
#include "no_os_util.h"

/******** Forward declaration of getter/setter functions ********/
static int ltc2672_iio_attr_get(void *device,
				char *buf,
				uint32_t len,
				const struct iio_ch_info *channel,
				intptr_t priv);

static int ltc2672_iio_attr_set(void *device,
				char *buf,
				uint32_t len,
				const struct iio_ch_info *channel,
				intptr_t priv);

static int ltc2672_iio_attr_available_get(void *device,
		char *buf,
		uint32_t len,
		const struct iio_ch_info *channel,
		intptr_t priv);

static int ltc2672_iio_attr_available_set(void *device,
		char *buf,
		uint32_t len,
		const struct iio_ch_info *channel,
		intptr_t priv);

/******************************************************************************/
/************************ Macros and Constants ********************************/
/******************************************************************************/

#define LTC2672_CHN_ATTR(_name, _priv) {\
		.name = _name,\
		.priv = _priv,\
		.show = ltc2672_iio_attr_get,\
		.store = ltc2672_iio_attr_set\
}

#define LTC2672_CHN_AVAIL_ATTR(_name, _priv) {\
		.name = _name,\
		.priv = _priv,\
		.show = ltc2672_iio_attr_available_get,\
		.store = ltc2672_iio_attr_available_set\
}

#define LTC2672_CH(_name, _idx, _type) {\
	.name = _name, \
	.ch_type = _type,\
	.ch_out = true,\
	.indexed = true,\
	.channel = _idx,\
	.scan_index = _idx,\
	.scan_type = &ltc2672_iio_scan_type,\
	.attributes = ltc2672_iio_ch_attributes\
}

/*	Number of IIO devices */
#define NUM_OF_IIO_DEVICES	1

#define	BYTES_PER_SAMPLE	sizeof(uint16_t)

#define BYTE_SIZE	(uint32_t)8
#define	BYTE_MASK	(uint32_t)0xff

/* Number of data storage bits (needed for IIO client to send buffer of data) */
#define CHN_STORAGE_BITS	(BYTES_PER_SAMPLE * 8)

/* DAC data buffer size */
#if defined(USE_SDRAM)
#define dac_data_buffer				SDRAM_START_ADDRESS
#define DATA_BUFFER_SIZE			SDRAM_SIZE_BYTES
#else
#define DATA_BUFFER_SIZE			(32768)		// 32kbytes
static int8_t dac_data_buffer[DATA_BUFFER_SIZE] = {0};
#endif

/******************************************************************************/
/******************** Variables and User Defined Data Types *******************/
/******************************************************************************/

/* LTC2672 devices descriptor */
struct ltc2672_dev *ltc2672_dev_desc;

/* LTC2672 IIO descriptor */
static struct iio_desc *ltc2672_iio_desc;

/* Attribute IDs */
enum ltc2672_iio_attr_id {
	// Channels attributes
	DAC_CH_RAW,
	DAC_CH_OFFSET,
	DAC_CH_SCALE,
	DAC_CH_SPAN,
	DAC_CH_CURRENT,
	DAC_CH_POWERDOWN,

	// Device attributes
	DAC_RAW,
	DAC_SPAN,
	DAC_CURRENT,
	DAC_POWERDOWN,
	DAC_MUX,
	DAC_FAULT_REGISTER
};

/* IIO channels scan structure */
static struct scan_type ltc2672_iio_scan_type = {
	.sign = 'u',
	.realbits = DAC_RESOLUTION,
	.storagebits = CHN_STORAGE_BITS,
	.shift = 0,
	.is_big_endian = false
};

/* Current spans */
static const char *ltc2672_current_spans[] = {
	"off_mode",
	"3.125mA",
	"6.25mA",
	"12.5mA",
	"25mA",
	"50mA",
	"100mA",
	"200mA",
	"MVREF",
	"300mA"
};

/* Mux output select options */
static const char *ltc2672_mux_select[] = {
	"disable",
	"iout0",
	"iout1",
	"iout2",
	"iout3",
	"iout4",
	"vcc",
	"vref",
	"vref_lo",
	"die_temperature",
	"vdd0",
	"vdd1",
	"vdd2",
	"vdd3",
	"vdd4",
	"v_minus",
	"gnd",
	"vout0",
	"vout1",
	"vout2",
	"vout3",
	"vout4"
};

/* Mux command array */
static enum ltc2672_mux_commands ltc2672_mux_map[] =  {
	LTC2672_MUX_DISABLED,
	LTC2672_MUX_IOUT0,
	LTC2672_MUX_IOUT1,
	LTC2672_MUX_IOUT2,
	LTC2672_MUX_IOUT3,
	LTC2672_MUX_IOUT4,
	LTC2672_MUC_VCC,
	LTC2672_MUX_VREF,
	LTC2672_MUX_VREF_LO,
	LTC2672_MUX_DIE_TEMP,
	LTC2672_MUX_VDD0,
	LTC2672_MUX_VDD1,
	LTC2672_MUX_VDD2,
	LTC2672_MUX_VDD3,
	LTC2672_MUX_VDD4,
	LTC2672_MUX_VMINUS,
	LTC2672_MUX_GND,
	LTC2672_MUX_VOUT0,
	LTC2672_MUX_VOUT1,
	LTC2672_MUX_VOUT2,
	LTC2672_MUX_VOUT3,
	LTC2672_MUX_VOUT4
};

/* Fault detect flags */
static const char *ltc2672_fault_detect[] = {
	"no_fault",
	"open_circuit_ch0",
	"open_circuit_ch1",
	"open_circuit_ch2",
	"open_circuit_ch3",
	"open_circuit_ch4",
	"Over_temperature",
	"Invalid_SPI_seq_length",
	"multiple_faults"
};

/* IIO channels attributes list */
static struct iio_attribute ltc2672_iio_ch_attributes[] = {
	LTC2672_CHN_ATTR("raw", DAC_CH_RAW),
	LTC2672_CHN_ATTR("scale", DAC_CH_SCALE),
	LTC2672_CHN_ATTR("offset", DAC_CH_OFFSET),
	LTC2672_CHN_ATTR("current", DAC_CH_CURRENT),
	LTC2672_CHN_ATTR("span", DAC_CH_SPAN),
	LTC2672_CHN_AVAIL_ATTR("span_available", DAC_CH_SPAN),
	LTC2672_CHN_ATTR("powerdown", DAC_CH_POWERDOWN),
	LTC2672_CHN_AVAIL_ATTR("powerdown_available", DAC_CH_POWERDOWN),
	END_ATTRIBUTES_ARRAY
};

/* IIO global attributes list */
static struct iio_attribute ltc2672_iio_global_attributes[] = {
	LTC2672_CHN_ATTR("all_chns_raw", DAC_RAW),
	LTC2672_CHN_ATTR("all_chns_current", DAC_CURRENT),
	LTC2672_CHN_ATTR("all_chns_span", DAC_SPAN),
	LTC2672_CHN_AVAIL_ATTR("all_chns_span_available", DAC_SPAN),
	LTC2672_CHN_ATTR("all_chns_powerdown", DAC_POWERDOWN),
	LTC2672_CHN_AVAIL_ATTR("all_chns_powerdown_available", DAC_POWERDOWN),
	LTC2672_CHN_ATTR("mux", DAC_MUX),
	LTC2672_CHN_AVAIL_ATTR("mux_available", DAC_MUX),
	LTC2672_CHN_ATTR("fault_detect", DAC_FAULT_REGISTER),
	LTC2672_CHN_AVAIL_ATTR("fault_detect_available", DAC_FAULT_REGISTER),
	END_ATTRIBUTES_ARRAY
};

/* IIO channels info */
static struct iio_channel ltc2672_iio_channels[] = {
	LTC2672_CH("Chn0", 0, IIO_CURRENT),
	LTC2672_CH("Chn1", 1, IIO_CURRENT),
	LTC2672_CH("Chn2", 2, IIO_CURRENT),
	LTC2672_CH("Chn3", 3, IIO_CURRENT),
	LTC2672_CH("Chn4", 4, IIO_CURRENT)
};


/* IIO context attributes */
static struct iio_ctx_attr ctx_attrs[] = {
	{
		.name = "hw_carrier",
		.value = STR(HW_CARRIER_NAME)
	},
	{
		.name = "hw_mezzanine",
		.value = HW_MEZZANINE_NAME
	},
	{
		.name = "hw_name",
		.value = ACTIVE_DEVICE_NAME
	},
};

/* Channel wise dac code array */
static uint32_t ch_dac_codes[LTC2672_TOTAL_CHANNELS];

/* Variable to store mux output select */
static uint8_t mux_val;

/* Scale attribute value */
static float attr_scale_val[LTC2672_TOTAL_CHANNELS];

/* Offset attribute value */
static uint16_t attr_offset_val;

/******************************************************************************/
/************************ Functions Definitions *******************************/
/******************************************************************************/

/**
 * @brief	Get the IIO scale for input channel
 * @param 	chn[in] - Input channel
 * @param	scale[in, out] - Channel IIO scale value
 * @return	0 in case of success, negative error code otherwise
 */
static int ltc2672_get_scale(uint8_t chn, float *scale)
{
	if (!scale) {
		return -EINVAL;
	}

	if (ltc2672_dev_desc->id == LTC2672_12) {
		*scale = ((float)ltc2672_dev_desc->max_currents[chn] / LTC2672_12BIT_RESO)/1000;
	} else {
		*scale = ((float)ltc2672_dev_desc->max_currents[chn] / LTC2672_16BIT_RESO)/1000;
	}

	return 0;
}

/*!
* @brief	Getter function for LTC2672 attributes.
* @param	device[in, out]- Pointer to IIO device instance.
* @param	buf[in]- IIO input data buffer.
* @param	len[in]- Number of expected bytes.
* @param	channel[in] - input channel.
* @param	priv[in] - Attribute private ID.
* @return	len in case of success, negative error code otherwise.
*/
static int ltc2672_iio_attr_get(void *device,
				char *buf,
				uint32_t len,
				const struct iio_ch_info *channel,
				intptr_t priv)
{
	int ret;
	uint8_t read_val = 0;
	float current = 0;

	switch (priv) {
	case DAC_CH_RAW:
		return sprintf(buf, "%lu", ch_dac_codes[channel->ch_num]);

	case DAC_CH_OFFSET:
		return sprintf(buf, "%u", attr_offset_val);

	case DAC_CH_SCALE:
		return sprintf(buf, "%0.10f", attr_scale_val[channel->ch_num]);

	case DAC_CH_CURRENT:
		/* Convert the dac code to current in mA */
		current = (ch_dac_codes[channel->ch_num] + attr_offset_val)
			  *attr_scale_val[channel->ch_num];

		return sprintf(buf, "%5.4fmA", current);

	case DAC_CH_SPAN:
		read_val = ltc2672_dev_desc->out_spans[channel->ch_num];

		if (read_val == LTC2672_4800VREF) {
			read_val = LTC2672_NUM_CURRENT_SPANS-1;
		}

		return sprintf(buf,
			       "%s",
			       ltc2672_current_spans[read_val]);

	case DAC_CH_POWERDOWN:
		return sprintf(buf, "%s", "powerdown");

	case DAC_CURRENT:
		/* Convert the dac code to current in mA */
		current = (ch_dac_codes[0] + attr_offset_val)
			  *attr_scale_val[0];

		return sprintf(buf, "%5.4fmA", current);

	case DAC_RAW:
		return sprintf(buf, "%lu", ch_dac_codes[0]);

	case DAC_SPAN:
		read_val = ltc2672_dev_desc->out_spans[0];

		if (read_val == LTC2672_4800VREF) {
			read_val = LTC2672_NUM_CURRENT_SPANS - 1;
		}

		return sprintf(buf,
			       "%s",
			       ltc2672_current_spans[read_val]);

	case DAC_MUX:
		return sprintf(buf, "%s", ltc2672_mux_select[mux_val]);

	case DAC_POWERDOWN:
		return sprintf(buf, "%s", "powerdown");

	case DAC_FAULT_REGISTER:
		/* Do a NO-OP transaction to read the fault register contents of the previous transaction */
		ret = ltc2672_transaction(ltc2672_dev_desc,
					  LTC2672_COMMAND24_GENERATE(LTC2672_NO_OP, 0, LTC2672_DUMMY), false);
		if (ret) {
			return ret;
		}

		/* Get fault register bitfield from previous command */
		read_val = no_os_field_get(LTC2672_FAULT_REG_MASK,
					   ltc2672_dev_desc->prev_command);

		/* Check for number and type of faults detected */
		if (no_os_hweight8(read_val) > 1) {
			/* multiple faults detected */
			read_val = LTC2672_NUM_FAULTS + 1;
		} else if (read_val) {
			/* single fault detected */
			read_val = no_os_find_first_set_bit(read_val);

			if (read_val == LTC2672_UNUSED) {
				/* set this to no fault */
				read_val = 0;
			} else if (read_val != LTC2672_INV_LENGTH) {
				read_val += 1;
			}
		}

		return sprintf(buf, "%s", ltc2672_fault_detect[read_val]);

	default:
		return -EINVAL;
	}

	return len;
}

/*!
 * @brief	Setter function for LTC2672 attributes.
 * @param	device[in, out]- Pointer to IIO device instance.
 * @param	buf[in]- IIO input data buffer.
 * @param	len[in]- Number of expected bytes.
 * @param	channel[in] - input channel.
 * @param	priv[in] - Attribute private ID.
 * @return	len in case of success, negative error code otherwise.
 */
static int ltc2672_iio_attr_set(void *device,
				char *buf,
				uint32_t len,
				const struct iio_ch_info *channel,
				intptr_t priv)
{
	int ret;
	uint8_t val;
	uint32_t write_val;
	uint32_t current_val_ua; //current value in uA
	char *end;

	switch (priv) {
	case DAC_CH_SCALE:
	case DAC_CH_OFFSET:
	case DAC_FAULT_REGISTER:
		//read-only
		break;

	case DAC_CH_RAW:
		write_val = no_os_str_to_uint32(buf);

		ret = ltc2672_set_code_channel(ltc2672_dev_desc, write_val, channel->ch_num);
		if (ret) {
			return ret;
		}
		ch_dac_codes[channel->ch_num] = write_val;

		break;

	case DAC_CH_CURRENT:
		current_val_ua = (uint32_t)(strtof(buf, &end) * 1000);

		ret = ltc2672_set_current_channel(ltc2672_dev_desc, current_val_ua,
						  channel->ch_num);
		if (ret) {
			return ret;
		}

		/* Update ch-wise dac codes array */
		ch_dac_codes[channel->ch_num] = ltc2672_current_to_code(device, current_val_ua,
						channel->ch_num);

		break;

	case DAC_CH_SPAN:
		for (val = 0; val < LTC2672_NUM_CURRENT_SPANS; val++) {
			if (!strcmp(buf, ltc2672_current_spans[val])) {
				break;
			}
		}

		if (val == LTC2672_NUM_CURRENT_SPANS-1) {
			val = LTC2672_4800VREF;
		}

		ret = ltc2672_set_span_channel(ltc2672_dev_desc, val, channel->ch_num);
		if (ret) {
			return ret;
		}

		/* Calculate the scale from output span selected */
		ret = ltc2672_get_scale(channel->ch_num, &attr_scale_val[channel->ch_num]);
		if (ret) {
			return ret;
		}

		break;

	case DAC_CH_POWERDOWN:
		ret = ltc2672_power_down_channel(ltc2672_dev_desc, channel->ch_num);
		if (ret) {
			return ret;
		}

		break;

	case DAC_RAW:
		write_val = no_os_str_to_uint32(buf);

		ret = ltc2672_set_code_all_channels(ltc2672_dev_desc, write_val);
		if (ret) {
			return ret;
		}

		/* Update ch-wise dac array */
		for (val = 0; val < LTC2672_TOTAL_CHANNELS; val++) {
			ch_dac_codes[val] = write_val;
		}

		break;

	case DAC_CURRENT:
		current_val_ua = (uint32_t)(strtof(buf, &end) * 1000);

		ret = ltc2672_set_current_all_channels(ltc2672_dev_desc, current_val_ua);
		if (ret) {
			return ret;
		}

		write_val = ltc2672_current_to_code(device, current_val_ua,
						    channel->ch_num);

		/* Update ch-wise dac array */
		for (val = 0; val < LTC2672_TOTAL_CHANNELS; val++) {
			ch_dac_codes[val] = write_val;
		}

		break;

	case DAC_SPAN:
		for (val = 0; val < LTC2672_NUM_CURRENT_SPANS; val++) {
			if (!strcmp(buf, ltc2672_current_spans[val])) {
				break;
			}
		}

		if (val == LTC2672_NUM_CURRENT_SPANS-1) {
			val = LTC2672_4800VREF;
		}

		ret = ltc2672_set_span_all_channels(ltc2672_dev_desc, val);
		if (ret) {
			return ret;
		}

		/* Calculate the scale from output span selected */
		for (val = 0; val < LTC2672_TOTAL_CHANNELS; val++) {
			ret = ltc2672_get_scale(channel->ch_num, &attr_scale_val[val]);
			if (ret) {
				return ret;
			}
		}

		break;

	case DAC_MUX:
		for (val = 0; val < LTC2672_NUM_MUX_SELECTS; val++) {
			if (!strcmp(buf, ltc2672_mux_select[val])) {
				break;
			}
		}

		ret = ltc2672_monitor_mux(ltc2672_dev_desc, ltc2672_mux_map[val]);
		if (ret) {
			return ret;
		}
		mux_val = val;

		break;

	case DAC_POWERDOWN:
		ret = ltc2672_power_down_all_channels(ltc2672_dev_desc);
		if (ret) {
			return ret;
		}

		break;

	default:
		return -EINVAL;
	}

	return len;
}

/*!
 * @brief	Attribute available getter function for LTC2672 attributes
 * @param	device[in, out]- Pointer to IIO device instance
 * @param	buf[in]- IIO input data buffer
 * @param	len[in]- Number of input bytes
 * @param	channel[in] - input channel
 * @param	priv[in] - Attribute private ID
 * @return	len in case of SUCCESS, negative error code otherwise
 */
static int ltc2672_iio_attr_available_get(void *device,
		char *buf,
		uint32_t len,
		const struct iio_ch_info *channel,
		intptr_t priv)
{
	uint8_t val;
	buf[0] = '\0';

	switch (priv) {
	case DAC_CH_POWERDOWN:
	case DAC_POWERDOWN:
		return sprintf(buf, "%s", "powerdown");

	case DAC_CH_SPAN:
	case DAC_SPAN:
		for (val = 0; val < LTC2672_NUM_CURRENT_SPANS; val++) {
			strcat(buf, ltc2672_current_spans[val]);
			strcat(buf, " ");
		}
		break;

	case DAC_MUX:
		for(val = 0; val < LTC2672_NUM_MUX_SELECTS; val++) {
			strcat(buf, ltc2672_mux_select[val]);
			strcat(buf, " ");
		}
		break;

	case DAC_FAULT_REGISTER:
		for (val = 0; val < NO_OS_ARRAY_SIZE(ltc2672_fault_detect); val++) {
			strcat(buf, ltc2672_fault_detect[val]);
			strcat(buf, " ");
		}
		break;

	default:
		return -EINVAL;
	}

	/* Remove extra trailing space at the end of the buffer string */
	len = strlen(buf);
	buf[len - 1] = '\0';

	return len;
}

/*!
 * @brief	Attribute available setter function for LTC2672 attributes
 * @param	device[in, out]- Pointer to IIO device instance
 * @param	buf[in]- IIO input data buffer
 * @param	len[in]- Number of input bytes
 * @param	channel[in] - input channel
 * @param	priv[in] - Attribute private ID
 * @return	len in case of SUCCESS, negative error code otherwise
 */
static int ltc2672_iio_attr_available_set(void *device,
		char *buf,
		uint32_t len,
		const struct iio_ch_info *channel,
		intptr_t priv)
{
	return len;
}

/**
* @brief	Init for reading/writing and parametrization of an
* 			LTC2672 IIO device.
* @param 	desc[in,out] - IIO device descriptor.
* @return	0 in case of success, negative error code otherwise.
*/
static int32_t ltc2672_iio_param_init(struct iio_device **desc)
{
	struct iio_device *ltc2672_iio_inst;

	if (!desc) {
		return -EINVAL;
	}

	ltc2672_iio_inst = calloc(1, sizeof(struct iio_device));
	if (!ltc2672_iio_inst) {
		return -ENOMEM;
	}

	ltc2672_iio_inst->num_ch = NO_OS_ARRAY_SIZE(ltc2672_iio_channels);
	ltc2672_iio_inst->channels = ltc2672_iio_channels;
	ltc2672_iio_inst->attributes = ltc2672_iio_global_attributes;
	ltc2672_iio_inst->debug_attributes = NULL;

	ltc2672_iio_inst->submit = NULL;
	ltc2672_iio_inst->pre_enable = NULL;
	ltc2672_iio_inst->post_disable = NULL;
	ltc2672_iio_inst->read_dev = NULL;
	ltc2672_iio_inst->write_dev = NULL;
	ltc2672_iio_inst->debug_reg_read = NULL;
	ltc2672_iio_inst->debug_reg_write = NULL;
	ltc2672_iio_inst->trigger_handler = NULL;

	*desc = ltc2672_iio_inst;

	return 0;
}

/**
 * @brief	Initialize the IIO interface for LTC2672 IIO device.
 * @return	0 in case of success, negative error code otherwise.
 */
int32_t ltc2672_iio_init()
{
	int32_t ret;

	/* IIO device descriptor */
	struct iio_device *ltc2672_iio_dev;

	/* IIO interface init parameters */
	static struct iio_init_param iio_init_params = {
		.phy_type = USE_UART,
		.trigs = NULL,
		.nb_trigs = 0,
	};

	/* IIOD init parameters */
	static struct iio_device_init iio_device_init_params[NUM_OF_IIO_DEVICES];

	/* Initialize the system peripherals */
	ret = init_system();
	if (ret) {
		return ret;
	}

	/* Initialize LTC2672 no-os device driver interface */
	ret = ltc2672_init(&ltc2672_dev_desc, &ltc2672_init_params);
	if (ret) {
		return ret;
	}

	/* Initialize the LTC2672 IIO app specific parameters */
	ret = ltc2672_iio_param_init(&ltc2672_iio_dev);
	if (ret) {
		return ret;
	}
	iio_init_params.nb_devs++;

	/* LTC2672 IIO device init parameters */
	iio_device_init_params[0].name = ACTIVE_DEVICE_NAME;
	iio_device_init_params[0].raw_buf = dac_data_buffer;
	iio_device_init_params[0].raw_buf_len = DATA_BUFFER_SIZE;
	iio_device_init_params[0].dev = ltc2672_dev_desc;
	iio_device_init_params[0].dev_descriptor = ltc2672_iio_dev;
	iio_device_init_params[0].trigger_id = NULL;

	iio_init_params.ctx_attrs = ctx_attrs;
	iio_init_params.nb_ctx_attr = NO_OS_ARRAY_SIZE(ctx_attrs);

	/* Initialize the IIO interface */
	iio_init_params.devs = iio_device_init_params;
	iio_init_params.uart_desc = uart_iio_com_desc;
	ret = iio_init(&ltc2672_iio_desc, &iio_init_params);
	if (ret) {
		return ret;
	}

	return 0;
}

/**
 * @brief 	Run the LTC2672 IIO event handler.
 * @return	none.
 * @details	This function monitors the new IIO client event.
 */
void ltc2672_iio_event_handler(void)
{
	iio_step(ltc2672_iio_desc);
}