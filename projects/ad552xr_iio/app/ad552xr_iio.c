/***************************************************************************//**
 *   @file    ad552xr_iio.c
 *   @brief   Implementation of AD552XR IIO Application Interface
 *   @details This module acts as an interface for AD552XR IIO device
********************************************************************************
 * Copyright (c) 2026 Analog Devices, Inc.
 *
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 * By using this software you agree to the terms of the associated
 * Analog Devices Software License Agreement.
*******************************************************************************/

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include "common.h"
#include "app_config.h"
#include "ad552xr.h"
#include "ad552xr_user_config.h"
#include "ad552xr_regs.h"
#include "ad552xr_support.h"
#include "no_os_error.h"
#include "no_os_delay.h"
#include "no_os_alloc.h"
#include "no_os_util.h"
#include "no_os_pwm.h"
#include "iio_trigger.h"
#include "version.h"

/******************************************************************************/
/************************** Functions Declarations ****************************/
/******************************************************************************/
static int iio_ad552xr_attr_get(void *device,
				char *buf,
				uint32_t len,
				const struct iio_ch_info *channel,
				intptr_t priv);

static int iio_ad552xr_attr_set(void *device,
				char *buf,
				uint32_t len,
				const struct iio_ch_info *channel,
				intptr_t priv);

static int iio_ad552xr_attr_available_get(void *device,
		char *buf,
		uint32_t len,
		const struct iio_ch_info *channel,
		intptr_t priv);

static int iio_ad552xr_attr_available_set(void *device,
		char *buf,
		uint32_t len,
		const struct iio_ch_info *channel,
		intptr_t priv);

/******************************************************************************/
/********************** Macros and Constants Definitions **********************/
/******************************************************************************/
/* Bytes per sample (Note: 4 bytes needed per sample for data range
 * of 0 to 32-bit) */
#define	BYTES_PER_SAMPLE	(AD552XR_DAC_RESOLUTION / 8)

/* Number of data storage bits (needed for IIO client) */
#define CHN_STORAGE_BITS	(BYTES_PER_SAMPLE * 8)

/* Channel attribute definition */
#define AD552XR_CHN_ATTR(_name, _priv) {\
	.name = _name,\
	.priv = _priv,\
	.show = iio_ad552xr_attr_get,\
	.store = iio_ad552xr_attr_set\
}

/* Channel attribute available definition */
#define AD552XR_CHN_AVAIL_ATTR(_name, _priv) {\
	.name = _name,\
	.priv = _priv,\
	.show = iio_ad552xr_attr_available_get,\
	.store = iio_ad552xr_attr_available_set\
}

/* AD552XR Channel Definition */
#define AD552XR_DAC_CH(_name, _dev, _idx) {\
	.name = strdup(_name),\
	.ch_type = IIO_VOLTAGE,\
	.channel = _idx,\
	.ch_out = true,\
	.scan_index = _idx,\
	.indexed = true,\
	.scan_type = &iio_ad552xr_scan_type[_dev][_idx],\
	.attributes = iio_ad552xr_channel_attributes[_dev],\
}

/* Scan type definition */
#define AD552XR_DEFAULT_CHN_SCAN {\
	.sign = 'u',\
	.realbits = AD552XR_DAC_RESOLUTION,\
	.storagebits = CHN_STORAGE_BITS,\
	.shift = 0,\
	.is_big_endian = false\
}

/* Number of bytes in a SPI transaction (Address phase + Data phase) */
#define AD552XR_NUM_BYTES_TRANSFER	(sizeof(uint16_t) + BYTES_PER_SAMPLE)

/* DAC data buffer size */
#if defined(USE_SDRAM)
/* Two buffers - one to store iio data and other to populate DAC data has to be created.
 * Hence partition SDRAM to accomodate both the buffers */
#define DATA_BUFFER_SIZE			(SDRAM_SIZE_BYTES / AD552XR_NUM_BYTES_TRANSFER)
#define dac_data_buffer				SDRAM_START_ADDRESS
#define populated_dac_data_buffer	SDRAM_START_ADDRESS + DATA_BUFFER_SIZE
#else
#define DATA_BUFFER_SIZE			32768
static int8_t dac_data_buffer[DATA_BUFFER_SIZE];
static uint8_t populated_dac_data_buffer[
	 (DATA_BUFFER_SIZE / BYTES_PER_SAMPLE) * AD552XR_NUM_BYTES_TRANSFER];
#endif

/* Timeout count to avoid stuck into potential infinite loop while checking
 * for new data into an acquisition buffer. The actual timeout factor is determined
 * through 'sampling_frequency' attribute of IIO app, but this period here makes sure
 * we are not stuck into a forever loop in case data capture is interrupted
 * or has failed in between.
 * Note: This timeout factor is dependent upon the MCU clock frequency. Below timeout
 * is tested for SDP-K1 platform @180Mhz default core clock
 */
#define BUF_READ_TIMEOUT	0xffffffff

/* AD552XR Maximum SPI frequency */
#define AD552XR_MAX_SPI_FREQUENCY		45E6

/******************************************************************************/
/********************** Variables and User Defined Data Types *****************/
/******************************************************************************/
/* Pointer to the structure representing the AD552XR IIO device */
static struct ad552xr_dev *ad552xr_dev_inst[AD552XR_IIO_NUM_DEVICES];

/* IIO interface descriptor */
static struct iio_desc *ad552xr_iio_desc;

/* AD552XR IIO device descriptor */
static struct iio_device *ad552xr_iio_dev[AD552XR_IIO_NUM_DEVICES];

/* IIOD channel scan configurations */
struct scan_type
	iio_ad552xr_scan_type[AD552XR_IIO_NUM_DEVICES][AD552XR_MAX_NUM_CH]
	= {
	[0 ... AD552XR_IIO_NUM_DEVICES - 1] = {
		[0 ... AD552XR_MAX_NUM_CH - 1] = AD552XR_DEFAULT_CHN_SCAN
	}
};

/* Permissible HW Mezzanine names */
static const char *mezzanine_names[] = {
	"EVAL-AD5529RARDZU1",
	"EV-AD5529R-ARDZ",
	"EVAL-AD5529RARDZ",
};

/* Device names */
static const char *ad552xr_device_names[] = {
	[ID_AD5529R] = "ad5529r"
};

/* AD552XR attribute unique IDs */
enum ad552xr_attribute_ids {
	/* IIO Device Attributes */
	DEV_ADDR_ATTR_ID,
	SAMPLE_RATE_ATTR_ID,
	SCK_FREQ_ATTR_ID,
	REF_SEL_ATTR_ID,
	STATUS_ATTR_ID,
	SINGLE_INSTR_ATTR_ID,

	/* IIO Channel Attributes */
	RAW_ATTR_ID,
	SCALE_ATTR_ID,
	OFFSET_ATTR_ID,
	INPUT_A_ATTR_ID,
	INPUT_B_ATTR_ID,
	OUTPUT_STATE_ATTR_ID,
	RANGE_SEL_ATTR_ID,
	HW_SW_SEL_ATTR_ID,
	FUNC_SEL_ATTR_ID,
	LDAC_HW_PIN_SEL_ATTR_ID,
	LDAC_HW_ACTIVE_EDGE_ATTR_ID,
	DITHER_PERIOD_FACTOR_ATTR_ID,
	DITHER_PHASE_ATTR_ID,
	RAMP_STEP_ATTR_ID,

	/* Number of IIO Device attributes */
	NUM_OF_DEV_ATTR = 6,
	NUM_OF_DEV_AVAIL_ATTR = 2,

	/* Number of IIO Channel attributes */
	NUM_OF_CH_ATTR = 14,
	NUM_OF_CH_AVAIL_ATTR = 8,
};

/* AD552XR Channel IIO attributes preset */
static struct iio_attribute iio_ad552xr_channel_attributes_preset[NUM_OF_CH_ATTR
			+ NUM_OF_CH_AVAIL_ATTR + 1] = {
	AD552XR_CHN_ATTR("raw", RAW_ATTR_ID),
	AD552XR_CHN_ATTR("scale", SCALE_ATTR_ID),
	AD552XR_CHN_ATTR("offset", OFFSET_ATTR_ID),
	AD552XR_CHN_ATTR("input_register_a", INPUT_A_ATTR_ID),
	AD552XR_CHN_ATTR("input_register_b", INPUT_B_ATTR_ID),
	AD552XR_CHN_ATTR("output_state", OUTPUT_STATE_ATTR_ID),
	AD552XR_CHN_AVAIL_ATTR("output_state_available", OUTPUT_STATE_ATTR_ID),
	AD552XR_CHN_ATTR("range_sel", RANGE_SEL_ATTR_ID),
	AD552XR_CHN_AVAIL_ATTR("range_sel_available", RANGE_SEL_ATTR_ID),
	AD552XR_CHN_ATTR("hw_sw_sel", HW_SW_SEL_ATTR_ID),
	AD552XR_CHN_AVAIL_ATTR("hw_sw_sel_available", HW_SW_SEL_ATTR_ID),
	AD552XR_CHN_ATTR("func_sel", FUNC_SEL_ATTR_ID),
	AD552XR_CHN_AVAIL_ATTR("func_sel_available", FUNC_SEL_ATTR_ID),
	AD552XR_CHN_ATTR("hw_ldac_pin_sel", LDAC_HW_PIN_SEL_ATTR_ID),
	AD552XR_CHN_AVAIL_ATTR("hw_ldac_pin_sel_available", LDAC_HW_PIN_SEL_ATTR_ID),
	AD552XR_CHN_ATTR("hw_ldac_edge_sel", LDAC_HW_ACTIVE_EDGE_ATTR_ID),
	AD552XR_CHN_AVAIL_ATTR("hw_ldac_edge_sel_available", LDAC_HW_ACTIVE_EDGE_ATTR_ID),
	AD552XR_CHN_ATTR("dither_period_factor", DITHER_PERIOD_FACTOR_ATTR_ID),
	AD552XR_CHN_AVAIL_ATTR("dither_period_factor_available", DITHER_PERIOD_FACTOR_ATTR_ID),
	AD552XR_CHN_ATTR("dither_phase", DITHER_PHASE_ATTR_ID),
	AD552XR_CHN_AVAIL_ATTR("dither_phase_available", DITHER_PHASE_ATTR_ID),
	AD552XR_CHN_ATTR("ramp_step_size", RAMP_STEP_ATTR_ID),

	END_ATTRIBUTES_ARRAY
};

/* AD552XR IIO Device (global) attributes preset */
static struct iio_attribute iio_ad552xr_global_attributes_preset[NUM_OF_DEV_ATTR
			+ NUM_OF_DEV_AVAIL_ATTR + 1] = {
	AD552XR_CHN_ATTR("dev_addr", DEV_ADDR_ATTR_ID),
	AD552XR_CHN_ATTR("sampling_frequency", SAMPLE_RATE_ATTR_ID),
	AD552XR_CHN_ATTR("sck_frequency", SCK_FREQ_ATTR_ID),
	AD552XR_CHN_ATTR("ref_sel", REF_SEL_ATTR_ID),
	AD552XR_CHN_AVAIL_ATTR("ref_sel_available", REF_SEL_ATTR_ID),
	AD552XR_CHN_ATTR("status", STATUS_ATTR_ID),
	AD552XR_CHN_ATTR("single_instr", SINGLE_INSTR_ATTR_ID),
	AD552XR_CHN_AVAIL_ATTR("single_instr_available", SINGLE_INSTR_ATTR_ID),

	END_ATTRIBUTES_ARRAY
};

/* AD552XR Channel IIO attributes */
static struct iio_attribute
	*iio_ad552xr_channel_attributes[AD552XR_IIO_NUM_DEVICES] = {
	iio_ad552xr_channel_attributes_preset,
	iio_ad552xr_channel_attributes_preset,
	iio_ad552xr_channel_attributes_preset,
	iio_ad552xr_channel_attributes_preset,
};

/* AD552XR IIO Device (global) attributes */
static struct iio_attribute
	*iio_ad552xr_global_attributes[AD552XR_IIO_NUM_DEVICES] = {
	iio_ad552xr_global_attributes_preset,
	iio_ad552xr_global_attributes_preset,
	iio_ad552xr_global_attributes_preset,
	iio_ad552xr_global_attributes_preset
};

/* IIOD channels configurations */
static struct iio_channel
	ad552xr_iio_channels[AD552XR_IIO_NUM_DEVICES][AD552XR_MAX_NUM_CH];

/* Output state of the channel */
static const char *ad552xr_output_state_attr_options[] = {
	"disable",
	"enable",
};

/* Operating ranges of the channel */
static const char *ad552xr_range_sel_attr_options[] = {
	"unipolar_5V",
	"unipolar_10V",
	"unipolar_20V",
	"unipolar_40V",
	"bipolar_5V",
	"bipolar_10V",
	"bipolar_15V",
	"bipolar_20V",
};

/* HW/SW selection */
static const char *ad552xr_hw_sw_sel_attr_options[] = {
	"hw",
	"sw",
};

/* Function selection */
static const char *ad552xr_func_sel_attr_options[] = {
	"disable",
	"ldac_toggle",
	"dither",
	"sawtooth",
	"triangular",
};

/* HW LDAC Pin Select */
static const char *ad552xr_ldac_hw_pin_sel_attr_options[] = {
	"ldac_toggle_0",
	"ldac_toggle_1",
	"ldac_toggle_2",
	"ldac_toggle_3",
};

/* HW LDAC Edge Select */
static const char *ad552xr_ldac_hw_active_edge_attr_options[] = {
	"rising_edge",
	"falling_edge",
	"any_edge",
};

/* Dither mode signal period factor */
static const char *ad552xr_dither_period_factor_attr_options[] = {
	"2",
	"4",
	"8",
	"16",
	"32",
	"64",
	"128",
};

/* Dither mode phase select */
static const char *ad552xr_dither_phase_attr_options[] = {
	"0",
	"90",
	"180",
	"270",
};

/* Reference selection */
static const char *ad552xr_ref_sel_attr_options[] = {
	"external",
	"internal",
};

/* SPI Register Read/Write mode */
static const char *ad552xr_single_instr_attr_options[] = {
	"streaming",
	"single_inst",
};

/* Scale factor to yield (raw + offset) * scale in millivolts. */
static const float scale_factor = 1000;

/* Offset value */
static int32_t offset = 0;

/* Output voltage ranges */
static const uint16_t ad552xr_voltage_ranges_v[] = {
	5, 10, 20, 40, // Unipolar full range
	10, 20, 30, 40 // Bipolar full range
};

#if (INTERFACE_MODE == SPI_INTERRUPT)
/* Descriptor to hold iio trigger details */
static struct iio_trigger ad552xr_iio_trig_desc = {
	.is_synchronous = true,
	.enable = NULL,
	.disable = NULL
};

/* IIO trigger name */
#define AD552XR_IIO_TRIGGER_NAME 		"ad552xr_iio_trigger"

/* AD552XR HW trigger descriptor */
static struct iio_hw_trig *ad552xr_hw_trig_desc;

/* IRQ ctrl descriptor for IIO trigger */
static struct no_os_irq_ctrl_desc *irq_iio_trigger_desc;
#endif

/* Pointer to store the device context attributes */
static struct iio_ctx_attr *context_attributes;

/* Function generation selection for channels */
static uint16_t func_gen_sel_mask = 0;

/******************************************************************************/
/************************** Functions Definitions *****************************/
/******************************************************************************/
/**
 * @brief	Initialize IIO channels structure. This will be called before main().
 * @return	None
 */
__attribute__((constructor))
static void init_ad552xr_iio_channels_constructor(void)
{
	uint8_t dev;
	uint8_t ch;

	for (dev = 0; dev < AD552XR_IIO_NUM_DEVICES; dev++) {
		for (ch = 0; ch < AD552XR_MAX_NUM_CH; ch++) {
			char name[10];
			sprintf(name, "Chn%d", ch);
			ad552xr_iio_channels[dev][ch] =
				(struct iio_channel)AD552XR_DAC_CH(name, dev, ch);
		}
	}
}

#if (INTERFACE_MODE == SPI_INTERRUPT)
/**
 * @brief	Pops one data-set from IIO buffer and writes into DAC
 * @param	iio_dev_data[in] - IIO device data instance.
 * @return	0 in case of success or negative value otherwise.
 */
static int32_t ad552xr_trigger_handler(struct iio_device_data *iio_dev_data)
{
	int32_t ret;

	/* Start the data transfer */
	ret = ad552xr_data_transfer_start(iio_dev_data, populated_dac_data_buffer);
	if (ret) {
		return ret;
	}

	return 0;
}

/**
 * @brief	Initialization of ad552xr IIO hardware trigger specific parameters
 * @param 	desc[in,out] - IIO hardware trigger descriptor
 * @return	0 in case of success, negative error code otherwise
 */
static int32_t ad552xr_iio_trigger_param_init(struct iio_hw_trig **desc)
{
	int32_t ret;
	struct iio_hw_trig_init_param ad552xr_hw_trig_init_params;
	struct iio_hw_trig *hw_trig_desc;

	if (!desc) {
		return -EINVAL;
	}

	/* Initialize the IIO Trigger IRQ ctrl descriptor */
	ret = no_os_irq_ctrl_init(&irq_iio_trigger_desc, &irq_iio_trigger_init_params);
	if (ret) {
		return ret;
	}

	ad552xr_hw_trig_init_params.irq_id = irq_iio_trigger_desc->irq_ctrl_id;
	ad552xr_hw_trig_init_params.name = AD552XR_IIO_TRIGGER_NAME;
	ad552xr_hw_trig_init_params.irq_trig_lvl = NO_OS_IRQ_EDGE_FALLING;
	ad552xr_hw_trig_init_params.irq_ctrl = irq_iio_trigger_desc;
	ad552xr_hw_trig_init_params.cb_info.event = NO_OS_EVT_TIM_PWM_PULSE_FINISHED;
	ad552xr_hw_trig_init_params.cb_info.peripheral = NO_OS_TIM_IRQ;
	ad552xr_hw_trig_init_params.cb_info.handle = &TIM_DAC_UPDATE_HANDLE;
	ad552xr_hw_trig_init_params.iio_desc = ad552xr_iio_desc;

	/* Initialize hardware trigger */
	ret = iio_hw_trig_init(&hw_trig_desc, &ad552xr_hw_trig_init_params);
	if (ret) {
		return ret;
	}

	*desc = hw_trig_desc;

	return 0;
}
#endif

/**
 * @brief Assign device info
 * @param read_id[in] - The mezzanine match id
 * @param dev_name[out] - The device name
 * @return 0 in case of success, negative error code otherwise.
 */
static int32_t ad552xr_assign_device(uint8_t read_id,
				     char** dev_name)
{
	switch (read_id) {
	case 0 ... 2:
		ad552xr_user_config.type = ID_AD5529R;
		*dev_name = (char *) ad552xr_device_names[ID_AD5529R];
		break;

	default:
		return -EINVAL;
	}

	return 0;

}

/*!
 * @brief	Getter function for AD552XR attributes
 * @param	device[in, out]- Pointer to IIO device instance.
 * @param	buf[in]- IIO input data buffer.
 * @param	len[in]- Number of expected bytes.
 * @param	channel[in] - input channel.
 * @param	priv[in] - Attribute private ID.
 * @return	len in case of success, negative error code otherwise
 */
static int iio_ad552xr_attr_get(void *device,
				char *buf,
				uint32_t len,
				const struct iio_ch_info *channel,
				intptr_t priv)
{
	int ret = 0;
	uint16_t val;
	float scale;
	struct ad552xr_dev *dev = device;
	uint32_t sample_rate;

	switch (priv) {
	case RAW_ATTR_ID:
		/* Read the DAC Data Readback register of the channel */
		ret = ad552xr_spi_reg_read(dev,
					   AD552XR_REG_DAC_DATA_READBACK_CH(channel->ch_num),
					   &val);
		if (ret) {
			return ret;
		}

		return sprintf(buf, "%d", val);

	case SCALE_ATTR_ID:
		/* Compute the scale value based on channel operating range */
		ret = ad552xr_spi_reg_read(dev,
					   AD552XR_REG_OUT_RANGE_CH(channel->ch_num),
					   &val);
		if (ret) {
			return ret;
		}

		val &= AD552XR_OUT_RANGE_CHn_MASK;
		scale = ((float)ad552xr_voltage_ranges_v[val]) /
			(NO_OS_BIT(AD552XR_DAC_RESOLUTION) - 1);

		return sprintf(buf, "%0.10f", scale * scale_factor);

	case OFFSET_ATTR_ID:
		/* Compute the offset value based on channel operating range */
		ret = ad552xr_spi_reg_read(dev,
					   AD552XR_REG_OUT_RANGE_CH(channel->ch_num),
					   &val);
		if (ret) {
			return ret;
		}

		val &= AD552XR_OUT_RANGE_CHn_MASK;
		if (val >= AD552XR_BIPOLAR_5V) {
			offset = (0 - (int32_t)NO_OS_BIT(AD552XR_DAC_RESOLUTION - 1));
		} else {
			offset = 0;
		}
		return sprintf(buf, "%ld", offset);

	case INPUT_A_ATTR_ID:
		/* Read DAC_INPUT_A_Chn */
		ret = ad552xr_spi_reg_read(dev,
					   AD552XR_REG_DAC_INPUT_A_CH(channel->ch_num),
					   &val);
		if (ret) {
			return ret;
		}

		return sprintf(buf, "%d", val);

	case INPUT_B_ATTR_ID:
		/* Read DAC_INPUT_B_Chn */
		ret = ad552xr_spi_reg_read(dev,
					   AD552XR_REG_FUNC_DAC_INPUT_B_CH(channel->ch_num),
					   &val);
		if (ret) {
			return ret;
		}

		return sprintf(buf, "%d", val);

	case OUTPUT_STATE_ATTR_ID:
		ret = ad552xr_spi_reg_read(dev,
					   AD552XR_REG_OUT_EN,
					   &val);
		if (ret) {
			return ret;
		}

		/* Get the bit of corresponding channel */
		val = ((val & AD552XR_CHANNEL_SEL(channel->ch_num)) != 0);
		return sprintf(buf, "%s", ad552xr_output_state_attr_options[val]);

	case RANGE_SEL_ATTR_ID:
		ret = ad552xr_spi_reg_read(dev,
					   AD552XR_REG_OUT_RANGE_CH(channel->ch_num),
					   &val);
		if (ret) {
			return ret;
		}

		val &= AD552XR_OUT_RANGE_CHn_MASK;

		return sprintf(buf, "%s", ad552xr_range_sel_attr_options[val]);

	case HW_SW_SEL_ATTR_ID:
		ret = ad552xr_spi_reg_read(dev,
					   AD552XR_REG_LDAC_HW_SW,
					   &val);
		if (ret) {
			return ret;
		}

		/* Get the bit of corresponding channel */
		val = ((val & AD552XR_CHANNEL_SEL(channel->ch_num)) != 0);
		return sprintf(buf, "%s", ad552xr_hw_sw_sel_attr_options[val]);

	case FUNC_SEL_ATTR_ID:
		ret = ad552xr_spi_reg_read(dev,
					   AD552XR_REG_FUNC_MODE_SEL_CH(channel->ch_num),
					   &val);
		if (ret) {
			return ret;
		}

		/* Check if function selection is configured */
		if (func_gen_sel_mask & AD552XR_CHANNEL_SEL(channel->ch_num)) {
			return sprintf(buf, "%s", ad552xr_func_sel_attr_options[val + 1]);
		} else {
			return sprintf(buf, "%s", ad552xr_func_sel_attr_options[0]);
		}

	case LDAC_HW_PIN_SEL_ATTR_ID:
		ret = ad552xr_spi_reg_read(dev,
					   AD552XR_REG_LDAC_HW_SRC_CH(channel->ch_num),
					   &val);
		if (ret) {
			return ret;
		}

		val &= AD552XR_LDAC_HW_SEL_CH_MASK;
		val >>= 8;

		return sprintf(buf, "%s", ad552xr_ldac_hw_pin_sel_attr_options[val]);

	case LDAC_HW_ACTIVE_EDGE_ATTR_ID:
		ret = ad552xr_spi_reg_read(dev,
					   AD552XR_REG_LDAC_HW_SRC_CH(channel->ch_num),
					   &val);
		if (ret) {
			return ret;
		}

		val &= AD552XR_LDAC_HW_EDGE_SEL_CH_MASK;

		return sprintf(buf, "%s", ad552xr_ldac_hw_active_edge_attr_options[val]);

	case DITHER_PERIOD_FACTOR_ATTR_ID:
		ret = ad552xr_spi_reg_read(dev,
					   AD552XR_REG_FUNC_DITHER_PERIOD_CH(channel->ch_num),
					   &val);
		if (ret) {
			return ret;
		}

		return sprintf(buf, "%s",
			       ad552xr_dither_period_factor_attr_options[NO_OS_ARRAY_SIZE(
						       ad552xr_dither_period_factor_attr_options) - val - 1]);

	case DITHER_PHASE_ATTR_ID:
		ret = ad552xr_spi_reg_read(dev,
					   AD552XR_REG_FUNC_DITHER_PHASE_CH(channel->ch_num),
					   &val);
		if (ret) {
			return ret;
		}

		return sprintf(buf, "%s", ad552xr_dither_phase_attr_options[val]);

	case RAMP_STEP_ATTR_ID:
		ret = ad552xr_spi_reg_read(dev,
					   AD552XR_REG_FUNC_RAMP_STEP_CH(channel->ch_num),
					   &val);
		if (ret) {
			return ret;
		}

		return sprintf(buf, "%d", val);

	case DEV_ADDR_ATTR_ID:
		return sprintf(buf, "%d", dev->dev_addr);

	case SAMPLE_RATE_ATTR_ID:
		/* Get the updated value set by hardware */
		ret = ad552xr_get_sampling_rate(&sample_rate);
		if (ret) {
			return ret;
		}

		return sprintf(buf, "%ld", sample_rate);

	case SCK_FREQ_ATTR_ID:
		return sprintf(buf, "%ld", dev->spi_desc->max_speed_hz);

	case REF_SEL_ATTR_ID:
		return sprintf(buf, "%s", ad552xr_ref_sel_attr_options[dev->vref]);

	case STATUS_ATTR_ID:
		/* Read the Interface_Status_A register */
		ret = ad552xr_spi_reg_read(dev, AD552XR_REG_INTERFACE_STATUS_A, &val);
		if (ret) {
			return ret;
		}

		return sprintf(buf, "%d", val);

	case SINGLE_INSTR_ATTR_ID:
		val = dev->spi_cfg.single_instr;

		return sprintf(buf, "%s", ad552xr_single_instr_attr_options[val]);

	default:
		break;
	}

	return len;
}

/*!
 * @brief	Setter function for AD552XR attributes.
 * @param	device[in, out]- Pointer to IIO device instance.
 * @param	buf[in]- IIO input data buffer.
 * @param	len[in]- Number of expected bytes.
 * @param	channel[in] - input channel.
 * @param	priv[in] - Attribute private ID.
 * @return	len in case of success, negative error code otherwise.
 */
static int iio_ad552xr_attr_set(void *device,
				char *buf,
				uint32_t len,
				const struct iio_ch_info *channel,
				intptr_t priv)
{
	int ret = 0;
	uint32_t val;
	struct ad552xr_dev *dev = device;
	bool found = false;

	switch (priv) {
	case RAW_ATTR_ID:
		val = no_os_str_to_uint32(buf);

		/* Set into async LDAC */
		ret = ad552xr_set_sync_async_ldac(device, channel->ch_num, false);
		if (ret) {
			return ret;
		}

		/* Write to DAC_INPUT_A_Chn */
		ret = ad552xr_set_dac_a_value(dev, channel->ch_num, val);
		if (ret) {
			return ret;
		}

		break;

	case SCALE_ATTR_ID:
		/* This is a read-only attribute */
		break;

	case OFFSET_ATTR_ID:
		/* This is a read-only attribute */
		break;

	case INPUT_A_ATTR_ID:
		val = no_os_str_to_uint32(buf);

		/* Write to DAC_INPUT_A_Chn */
		ret = ad552xr_set_dac_a_value(dev, channel->ch_num, val);
		if (ret) {
			return ret;
		}

		break;

	case INPUT_B_ATTR_ID:
		val = no_os_str_to_uint32(buf);

		/* Write to DAC_INPUT_B_Chn */
		ret = ad552xr_set_dac_b_value(dev, channel->ch_num, val);
		if (ret) {
			return ret;
		}

		break;

	case OUTPUT_STATE_ATTR_ID:
		for (val = 0; val < NO_OS_ARRAY_SIZE(ad552xr_output_state_attr_options);
		     val++) {
			if (!strcmp(buf, ad552xr_output_state_attr_options[val])) {
				found = true;
				break;
			}
		}

		if (!found) {
			return -EINVAL;
		}

		ret = ad552xr_channel_output_en(dev, channel->ch_num, val);
		if (ret) {
			return ret;
		}

		break;

	case RANGE_SEL_ATTR_ID:
		for (val = 0; val < NO_OS_ARRAY_SIZE(ad552xr_range_sel_attr_options); val++) {
			if (!strcmp(buf, ad552xr_range_sel_attr_options[val])) {
				found = true;
				break;
			}
		}

		if (!found) {
			return -EINVAL;
		}

		/* Set the channel output range */
		ret = ad552xr_set_ch_output_range(dev, channel->ch_num, val);
		if (ret) {
			return ret;
		}

		break;

	case HW_SW_SEL_ATTR_ID:
		for (val = 0; val < NO_OS_ARRAY_SIZE(ad552xr_hw_sw_sel_attr_options); val++) {
			if (!strcmp(buf, ad552xr_hw_sw_sel_attr_options[val])) {
				found = true;
				break;
			}
		}

		if (!found) {
			return -EINVAL;
		}

		/* Set the HW/SW ldac configuration. */
		ret = ad552xr_set_hw_sw_ldac(dev, channel->ch_num, val);
		if (ret) {
			return ret;
		}

		break;

	case FUNC_SEL_ATTR_ID:
		for (val = 0; val < NO_OS_ARRAY_SIZE(ad552xr_func_sel_attr_options); val++) {
			if (!strcmp(buf, ad552xr_func_sel_attr_options[val])) {
				found = true;
				break;
			}
		}

		if (!found) {
			return -EINVAL;
		}

		if (val != 0) {
			/* Configure the function */
			ret = ad552xr_func_mode_select(dev, channel->ch_num, val - 1);
			if (ret) {
				return ret;
			}

			/* Store the function generator selection in mask */
			func_gen_sel_mask |= AD552XR_CHANNEL_SEL(channel->ch_num);
		} else {
			/* Clear the function generator selection from mask */
			func_gen_sel_mask &= ~AD552XR_CHANNEL_SEL(channel->ch_num);
		}

		break;

	case LDAC_HW_PIN_SEL_ATTR_ID:
		for (val = 0; val < NO_OS_ARRAY_SIZE(ad552xr_ldac_hw_pin_sel_attr_options);
		     val++) {
			if (!strcmp(buf, ad552xr_ldac_hw_pin_sel_attr_options[val])) {
				found = true;
				break;
			}
		}

		if (!found) {
			return -EINVAL;
		}

		/* Set the HW toggle pin */
		ret = ad552xr_set_hw_ldac_toggle_pin(dev, channel->ch_num, val);
		if (ret) {
			return ret;
		}

		break;

	case LDAC_HW_ACTIVE_EDGE_ATTR_ID:
		for (val = 0; val < NO_OS_ARRAY_SIZE(ad552xr_ldac_hw_active_edge_attr_options);
		     val++) {
			if (!strcmp(buf, ad552xr_ldac_hw_active_edge_attr_options[val])) {
				found = true;
				break;
			}
		}

		if (!found) {
			return -EINVAL;
		}

		/* Set the HW active edge trigger */
		ret = ad552xr_set_hw_edge_trigger(dev, channel->ch_num, val);
		if (ret) {
			return ret;
		}

		break;

	case DITHER_PERIOD_FACTOR_ATTR_ID:
		for (val = 0; val < NO_OS_ARRAY_SIZE(ad552xr_dither_period_factor_attr_options);
		     val++) {
			if (!strcmp(buf, ad552xr_dither_period_factor_attr_options[val])) {
				found = true;
				break;
			}
		}

		if (!found) {
			return -EINVAL;
		}

		/* Set the Dither period */
		ret = ad552xr_set_dither_period(dev, channel->ch_num,
						NO_OS_ARRAY_SIZE(ad552xr_dither_period_factor_attr_options) - val - 1);
		if (ret) {
			return ret;
		}

		break;

	case DITHER_PHASE_ATTR_ID:
		for (val = 0; val < NO_OS_ARRAY_SIZE(ad552xr_dither_phase_attr_options);
		     val++) {
			if (!strcmp(buf, ad552xr_dither_phase_attr_options[val])) {
				found = true;
				break;
			}
		}

		if (!found) {
			return -EINVAL;
		}

		/* Set the Dither phase */
		ret = ad552xr_set_dither_phase(dev, channel->ch_num, val);
		if (ret) {
			return ret;
		}

		break;

	case RAMP_STEP_ATTR_ID:
		val = no_os_str_to_uint32(buf);

		/* Configure the ramp step size */
		ret = ad552xr_set_ramp_step_size(dev, channel->ch_num, val);
		if (ret) {
			return ret;
		}

		break;

	case DEV_ADDR_ATTR_ID:
		/* This is a read-only attribute */
		break;

	case SAMPLE_RATE_ATTR_ID:
		val = no_os_str_to_uint32(buf);

		/* Update the sampling rate */
		ret = ad552xr_set_sampling_rate(val);
		if (ret) {
			return ret;
		}

		break;

	case SCK_FREQ_ATTR_ID:
		val = no_os_str_to_uint32(buf);

		if (val > AD552XR_MAX_SPI_FREQUENCY) {
			return -EINVAL;
		}

		/* Update init params with new SCK frequency */
		spi_init_params.max_speed_hz = val;

		/* Remove the previous SPI descriptor */
		ret = no_os_spi_remove(dev->spi_desc);
		if (ret) {
			return ret;
		}

		/* Configure SPI with updated SCK frequency */
		ret = no_os_spi_init(&dev->spi_desc, &spi_init_params);
		if (ret) {
			return ret;
		}

		break;

	case REF_SEL_ATTR_ID:
		for (val = 0; val < NO_OS_ARRAY_SIZE(ad552xr_ref_sel_attr_options); val++) {
			if (!strcmp(buf, ad552xr_ref_sel_attr_options[val])) {
				found = true;
				break;
			}
		}

		if (!found) {
			return -EINVAL;
		}

		/* Set the voltage reference */
		ret = ad552xr_set_reference(dev, val);
		if (ret) {
			return ret;
		}

		break;

	case STATUS_ATTR_ID:
		val = no_os_str_to_uint32(buf);

		/* Write into Interface_Status_A register */
		ret = ad552xr_spi_reg_write(dev, AD552XR_REG_INTERFACE_STATUS_A, val);
		if (ret) {
			return ret;
		}

		break;

	case SINGLE_INSTR_ATTR_ID:
		for (val = 0; val < NO_OS_ARRAY_SIZE(ad552xr_single_instr_attr_options);
		     val++) {
			if (!strcmp(buf, ad552xr_single_instr_attr_options[val])) {
				found = true;
				break;
			}
		}

		if (!found) {
			return -EINVAL;
		}

		dev->spi_cfg.single_instr = (bool)val;

		ret = ad552xr_spi_write_mask(dev,
					     AD552XR_REG_INTERFACE_CONFIG_B,
					     AD552XR_INT_CONFIG_B_SINGLE_INSTR_MASK,
					     dev->spi_cfg.single_instr);
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
 * @brief	Attribute available getter function for AD552XR attributes.
 * @param	device[in, out]- Pointer to IIO device instance.
 * @param	buf[in]- IIO input data buffer.
 * @param	len[in]- Number of input bytes.
 * @param	channel[in] - input channel.
 * @param	priv[in] - Attribute private ID.
 * @return	len in case of success, negative error code otherwise.
 */
static int iio_ad552xr_attr_available_get(void *device,
		char *buf,
		uint32_t len,
		const struct iio_ch_info *channel,
		intptr_t priv)
{
	uint8_t val;

	switch (priv) {
	case OUTPUT_STATE_ATTR_ID :
		return sprintf(buf, "%s %s",
			       ad552xr_output_state_attr_options[0],
			       ad552xr_output_state_attr_options[1]);

	case RANGE_SEL_ATTR_ID :
		buf[0] = '\0';

		for (val = 0; val < NO_OS_ARRAY_SIZE(ad552xr_range_sel_attr_options); val++) {
			strcat(buf, ad552xr_range_sel_attr_options[val]);
			strcat(buf, " ");
		}

		/* Remove extra trailing space at the end of the buffer string */
		len = strlen(buf);
		buf[len - 1] = '\0';

		break;

	case HW_SW_SEL_ATTR_ID :
		return sprintf(buf, "%s %s",
			       ad552xr_hw_sw_sel_attr_options[0],
			       ad552xr_hw_sw_sel_attr_options[1]);

	case FUNC_SEL_ATTR_ID :

		buf[0] = '\0';

		for (val = 0; val < NO_OS_ARRAY_SIZE(ad552xr_func_sel_attr_options); val++) {
			strcat(buf, ad552xr_func_sel_attr_options[val]);
			strcat(buf, " ");
		}

		/* Remove extra trailing space at the end of the buffer string */
		len = strlen(buf);
		buf[len - 1] = '\0';

		break;

	case LDAC_HW_PIN_SEL_ATTR_ID :
		return sprintf(buf, "%s %s %s %s",
			       ad552xr_ldac_hw_pin_sel_attr_options[0],
			       ad552xr_ldac_hw_pin_sel_attr_options[1],
			       ad552xr_ldac_hw_pin_sel_attr_options[2],
			       ad552xr_ldac_hw_pin_sel_attr_options[3]);

	case LDAC_HW_ACTIVE_EDGE_ATTR_ID :
		return sprintf(buf, "%s %s %s",
			       ad552xr_ldac_hw_active_edge_attr_options[0],
			       ad552xr_ldac_hw_active_edge_attr_options[1],
			       ad552xr_ldac_hw_active_edge_attr_options[2]);

	case DITHER_PERIOD_FACTOR_ATTR_ID :
		buf[0] = '\0';

		for (val = 0; val < NO_OS_ARRAY_SIZE(ad552xr_dither_period_factor_attr_options);
		     val++) {
			strcat(buf, ad552xr_dither_period_factor_attr_options[val]);
			strcat(buf, " ");
		}

		/* Remove extra trailing space at the end of the buffer string */
		len = strlen(buf);
		buf[len - 1] = '\0';

		break;

	case DITHER_PHASE_ATTR_ID :
		return sprintf(buf, "%s %s %s %s",
			       ad552xr_dither_phase_attr_options[0],
			       ad552xr_dither_phase_attr_options[1],
			       ad552xr_dither_phase_attr_options[2],
			       ad552xr_dither_phase_attr_options[3]);

	case REF_SEL_ATTR_ID :
		return sprintf(buf, "%s %s",
			       ad552xr_ref_sel_attr_options[0],
			       ad552xr_ref_sel_attr_options[1]);

	case SINGLE_INSTR_ATTR_ID :
		return sprintf(buf, "%s %s",
			       ad552xr_single_instr_attr_options[0],
			       ad552xr_single_instr_attr_options[1]);

		break;

	default:
		break;
	}

	return len;
}

/*!
 * @brief	Attribute available setter function for AD552XR attributes
 * @param	device[in, out]- Pointer to IIO device instance
 * @param	buf[in]- IIO input data buffer
 * @param	len[in]- Number of input bytes
 * @param	channel[in] - input channel
 * @param	priv[in] - Attribute private ID
 * @return	len in case of success, negative error code otherwise
 */
static int iio_ad552xr_attr_available_set(void *device,
		char *buf,
		uint32_t len,
		const struct iio_ch_info *channel,
		intptr_t priv)
{
	return len;
}

/**
 * @brief	Prepares the device for data transfer.
 * @param	dev[in, out]- Application descriptor.
 * @param	mask[in]- Channels select mask.
 * @return 	0 in case of success, error code otherwise.
 */
static int32_t ad552xr_iio_prepare_transfer(void *dev, uint32_t mask)
{
	int32_t ret;
	uint8_t ch;
	struct ad552xr_dev *device = dev;

	if (!dev) {
		return -EINVAL;
	}

	for (ch = 0; ch < device->dev_info.num_channels; ch++) {
		/* Get the active channel */
		if (mask & AD552XR_CHANNEL_SEL(ch)) {
			/* Configure all active channels into sync ldac and write the
			 * function generator register with appropriate enablement */
			ret = ad552xr_set_sync_async_ldac(device, ch, true);
			if (ret) {
				return ret;
			}

			ret = ad552xr_func_en(device, ch,
					      (func_gen_sel_mask & AD552XR_CHANNEL_SEL(ch)));
			if (ret) {
				return ret;
			}

			/* Channel output enable */
			ret = ad552xr_channel_output_en(device, ch, true);
			if (ret) {
				return ret;
			}
		}
	}

#if (INTERFACE_MODE == SPI_INTERRUPT)
	/* Enable IIO trigger only if any channel is configured in SW Mode */
	if (device->ldac_cfg.ldac_hw_sw_mask) {
		ret = iio_trig_enable(ad552xr_hw_trig_desc);
		if (ret) {
			return ret;
		}
	}
#endif

	/* Prepare data transfer */
	ret = ad552xr_data_transfer_prepare(dev, mask);
	if (ret) {
		return ret;
	}

	return 0;
}

/**
 * @brief	Writes all the samples from the DAC buffer into the IIO buffer.
 * @param	iio_dev_data[in] - IIO device data instance.
 * @return	0 in case of success, negative error code otherwise
 */
static int32_t ad552xr_iio_submit_samples(struct iio_device_data *iio_dev_data)
{
	int32_t ret;

	/* Start the data transfer */
	ret = ad552xr_data_transfer_start(iio_dev_data, populated_dac_data_buffer);
	if (ret) {
		return ret;
	}

	return 0;
}

/**
 * @brief	End the device data transfer.
 * @param	dev[in, out]- Application descriptor.
 * @return	0 in case of success, error code otherwise.
 */
static int32_t ad552xr_iio_end_transfer(void *dev)
{
	int32_t ret;
	uint8_t ch;
	struct ad552xr_dev *device = dev;

#if (INTERFACE_MODE == SPI_INTERRUPT)
	/* Disable IIO trigger */
	ret = iio_trig_disable(ad552xr_hw_trig_desc);
	if (ret) {
		return ret;
	}
#endif

	/* Stop the data transfer */
	ret = ad552xr_data_transfer_stop(dev);
	if (ret) {
		return ret;
	}

	/* Disable channels output and function generation */
	for (ch = 0; ch < device->dev_info.num_channels; ch++) {
		if (device->ldac_cfg.output_en_mask & AD552XR_CHANNEL_SEL(ch)) {
			/* Channel output disable */
			ret = ad552xr_channel_output_en(device, ch, false);
			if (ret) {
				return ret;
			}

			ret = ad552xr_func_en(device, ch, false);
			if (ret) {
				return ret;
			}
		}
	}

	return 0;
}

/*!
 * @brief	Search for the base address for multi byte register
 * @param	addr[in] - Address as requested by the IIO client
 * @return	Register address
 */
uint32_t debug_reg_search(uint32_t addr)
{
	uint32_t i;
	uint32_t ch;

	/* Return register address if found else treat it as single-byte register */
	for (i = 0; i < AD552XR_NUM_REGS; i++) {
		if (addr == AD552XR_ADDR(ad552xr_regs[i])) {
			/* Remove the channel register identification */
			return ad552xr_regs[i] & (~AD552XR_RChn);
		}

		/* Check if address is any base address to a channel */
		if (AD552XR_RChn & ad552xr_regs[i]) {
			for (ch = 0; ch < AD552XR_MAX_NUM_CH; ch++) {
				if (addr == (AD552XR_ADDR(ad552xr_regs[i]) + AD552XR_REG_CH_OFFSET(ch))) {
					return (ad552xr_regs[i] & (~AD552XR_RChn)) | addr;
				}
			}
		}
	}

	return (AD552XR_R1B | addr);
}

/*!
 * @brief	Read the debug register value
 * @param	dev[in, out]- Pointer to IIO device instance
 * @param	reg[in]- Register address to read from
 * @param	read_val[in, out]- Pointer to variable to read data into
 * @return	0 in case of success, negative value otherwise
 */
static int32_t iio_ad552xr_debug_reg_read(void *dev,
		uint32_t reg,
		uint32_t *read_val)
{
	int32_t ret;
	uint32_t reg_addr;
	uint16_t reg_val = 0;

	if (!dev || !read_val || (reg > AD552XR_REG_INIT_CRC_ERR_STAT)) {
		return -EINVAL;
	}

	reg_addr = debug_reg_search(reg);

	/* Read the register contents */
	ret = ad552xr_spi_reg_read(dev, reg_addr, &reg_val);
	if (ret) {
		return ret;
	}

	*read_val = reg_val;

	return 0;
}

/*!
 * @brief	Write value to the debug register
 * @param	dev[in, out]- Pointer to IIO device instance
 * @param	reg[in]- Register address to write to
 * @param	writeval[in]- Variable to write data from
 * @return	0 in case of success, negative value otherwise
 */
static int32_t iio_ad552xr_debug_reg_write(void *dev,
		uint32_t reg,
		uint32_t writeval)
{
	int32_t ret;
	uint32_t reg_addr;

	if (!dev  || (reg > AD552XR_REG_INIT_CRC_ERR_STAT)) {
		return -EINVAL;
	}

	reg_addr = debug_reg_search(reg);

	/* Write data into device register */
	ret = ad552xr_spi_reg_write(dev, reg_addr, writeval);
	if (ret) {
		return ret;
	}

	return 0;
}

/**
* @brief	Init for reading/writing and parameterization of a
* 			AD552XR IIO device
* @param 	desc[in,out] - IIO device descriptor
* @param	dev_indx[in] - IIO Device index
* @return	0 in case of success, negative error code otherwise
*/
static int32_t iio_ad552xr_init(struct iio_device **desc, uint8_t dev_indx)
{
	struct iio_device *iio_ad552xr_inst;

	iio_ad552xr_inst = calloc(1, sizeof(struct iio_device));
	if (!iio_ad552xr_inst) {
		return -EINVAL;
	}

	iio_ad552xr_inst->num_ch = NO_OS_ARRAY_SIZE(ad552xr_iio_channels[dev_indx]);
	iio_ad552xr_inst->channels = ad552xr_iio_channels[dev_indx];
	iio_ad552xr_inst->attributes = iio_ad552xr_global_attributes[dev_indx];
	iio_ad552xr_inst->submit = ad552xr_iio_submit_samples;
	iio_ad552xr_inst->pre_enable = ad552xr_iio_prepare_transfer;
	iio_ad552xr_inst->post_disable = ad552xr_iio_end_transfer;
	iio_ad552xr_inst->read_dev = NULL;
	iio_ad552xr_inst->write_dev = NULL;
	iio_ad552xr_inst->debug_reg_read = iio_ad552xr_debug_reg_read;
	iio_ad552xr_inst->debug_reg_write = iio_ad552xr_debug_reg_write;
#if (INTERFACE_MODE == SPI_INTERRUPT)
	iio_ad552xr_inst->trigger_handler = ad552xr_trigger_handler;
#endif

	*desc = iio_ad552xr_inst;

	return 0;
}

/**
 * @brief	Remove the IIO resources for AD552XR IIO application
 * @return	none
 */
void iio_app_remove(void)
{
	uint8_t i;

	/* Remove IIO */
	iio_remove(ad552xr_iio_desc);

	/* Free AD552XR IIO devices */
	for (i = 0; i < AD552XR_IIO_NUM_DEVICES; i++) {
		no_os_free(ad552xr_iio_dev[i]);
	}

	/* Free AD552XR device descriptors */
	for (i = 0; i < AD552XR_IIO_NUM_DEVICES; i++) {
		ad552xr_remove(ad552xr_dev_inst[i]);
	}

	/* Remove the IIO context attributes */
	remove_iio_context_attributes(context_attributes);

	/* Remove the data transfer system */
	ad552xr_data_transfer_system_remove();
}

/**
 * @brief	Initialize the IIO interface for AD552XR IIO device
 * @return	0 in case of success,negative error code otherwise
 */
int32_t iio_app_initialize(void)
{
	int32_t ret;
	int dev_id = 0;
	bool hw_mezzanine_is_valid;
	uint8_t read_id = 0;

	/* IIO Device init params */
	struct iio_device_init iio_device_init_params[AD552XR_IIO_NUM_DEVICES];

#if (INTERFACE_MODE == SPI_INTERRUPT)
	/* IIO trigger init parameters */
	struct iio_trigger_init iio_trigger_init_params = {
		.descriptor = &ad552xr_iio_trig_desc,
		.name = AD552XR_IIO_TRIGGER_NAME,
	};
#endif

	/* IIO interface init parameters */
	struct iio_init_param iio_init_params = {
		.phy_type = USE_UART,
		.nb_devs = 0,
		.devs = iio_device_init_params,
#if (INTERFACE_MODE == SPI_INTERRUPT)
		.trigs = &iio_trigger_init_params,
#endif
	};

	/* Initialize the data transfer system */
	ret = ad552xr_data_transfer_system_init();
	if (ret) {
		return ret;
	}

	/* Read context attributes */
	for (read_id = 0; read_id < NO_OS_ARRAY_SIZE(mezzanine_names); read_id++) {
		ret = get_iio_context_attributes_ex(&context_attributes,
						    &iio_init_params.nb_ctx_attr,
						    eeprom_desc,
						    mezzanine_names[read_id],
						    STR(HW_CARRIER_NAME),
						    &hw_mezzanine_is_valid,
						    FIRMWARE_VERSION);
		if (ret || hw_mezzanine_is_valid) {
			break;
		}
	}

	/* Assign the context attributes */
	iio_init_params.ctx_attrs = context_attributes;

	/* If hardware is detected, then initialize it */
	if ((ret == 0) && hw_mezzanine_is_valid) {
		do {
			/* Update the device address for initialization */
			ad552xr_user_config.dev_addr = dev_id;

			/* Assign parameters and Initialize AD552xr device */
			ret = ad552xr_assign_device(read_id,
						    &iio_device_init_params[iio_init_params.nb_devs].name);
			if (ret) {
				return ret;
			}

			/* Initialize AD552XR device and peripheral interface */
			ret = ad552xr_init(&ad552xr_dev_inst[dev_id], &ad552xr_user_config);
			if (ret) {
				/* If init failed with current device address, continue */
				dev_id++;
				continue;
			}

			/* Board is initially configured with internal reference voltage
			 * where as the chip default value is external. Hence configure the
			 * reference voltage to be external */
			ret = ad552xr_set_reference(ad552xr_dev_inst[dev_id], AD552XR_INTERNAL_VREF);
			if (ret) {
				dev_id++;
				continue;
			}

			/* By default device will be in streaming mode, enter single instruction mode */
			ret = ad552xr_spi_write_mask(ad552xr_dev_inst[dev_id],
						     AD552XR_REG_INTERFACE_CONFIG_B,
						     AD552XR_INT_CONFIG_B_SINGLE_INSTR_MASK, 0x1);
			if (ret) {
				dev_id++;
				continue;
			}

			ad552xr_dev_inst[dev_id]->spi_cfg.single_instr = true;

			/* Initialize the AD552XR IIO device*/
			ret = iio_ad552xr_init(&ad552xr_iio_dev[dev_id], dev_id);
			if (ret) {
				ad552xr_remove(ad552xr_dev_inst[dev_id]);
				break;
			}

			/* Initialize the IIO interface */
			iio_device_init_params[iio_init_params.nb_devs].raw_buf = dac_data_buffer;
			iio_device_init_params[iio_init_params.nb_devs].raw_buf_len = DATA_BUFFER_SIZE;

			iio_device_init_params[iio_init_params.nb_devs].dev = ad552xr_dev_inst[dev_id];
			iio_device_init_params[iio_init_params.nb_devs].dev_descriptor =
				ad552xr_iio_dev[dev_id];

#if (INTERFACE_MODE == SPI_INTERRUPT)
			iio_device_init_params[iio_init_params.nb_devs].trigger_id = "trigger0";
			iio_init_params.nb_trigs++;
#endif

			/* Increment number of devs */
			iio_init_params.nb_devs++;
			break;
		} while (dev_id < AD552XR_IIO_NUM_DEVICES);
	}

	iio_init_params.uart_desc = uart_iio_comm_desc;
	ret = iio_init(&ad552xr_iio_desc, &iio_init_params);
	if (ret) {
		goto iio_fail;
	}

#if (INTERFACE_MODE == SPI_INTERRUPT)
	/* Initialize IIO trigger */
	ret = ad552xr_iio_trigger_param_init(&ad552xr_hw_trig_desc);
	if (ret) {
		return ret;
	}
#endif

	return 0;

iio_fail:
	/* Remove IIO application resources */
	iio_app_remove();

	return ret;
}

/**
 * @brief	Run the AD552XR IIO event handler
 * @return	none
 * @details	This function monitors the new IIO client event
 */
void iio_app_event_handler(void)
{
	(void)iio_step(ad552xr_iio_desc);
}
