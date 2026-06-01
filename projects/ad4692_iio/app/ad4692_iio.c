/***************************************************************************//**
 *   @file    ad4692_iio.c
 *   @brief   Implementation of AD4692 IIO application interfaces
 *   @details This module acts as an interface for AD4692 IIO application
********************************************************************************
 * Copyright (c) 2024, 2026 Analog Devices, Inc.
 *
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 * By using this software you agree to the terms of the associated
 * Analog Devices Software License Agreement.
*******************************************************************************/

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include <string.h>

#include "ad4692_user_config.h"
#include "ad4692_support.h"
#include "ad4692_iio.h"
#include "iio_trigger.h"
#include "no_os_error.h"
#include "no_os_delay.h"
#include "no_os_util.h"
#include "no_os_alloc.h"
#include "app_config.h"
#include "common.h"
#include "ad4692.h"
#include "version.h"
#include "ad4692_attrs.h"

/******************************************************************************/
/************************ Macros/Constants ************************************/
/******************************************************************************/

/* IIO trigger name */
#define AD4692_IIO_TRIGGER_NAME		"ad4692_iio_trigger"

/* ADC data buffer size */
#if defined(USE_SDRAM)
/* Offset the IIO buffer for 4 bytes to accommodate the 2-cycle command offset in manual mode */
#define adc_data_buffer				SDRAM_START_ADDRESS
#define DATA_BUFFER_SIZE			SDRAM_SIZE_BYTES - (N_CYCLE_OFFSET * BYTES_PER_SAMPLE)
#else
#define DATA_BUFFER_SIZE			(32768)		// 32kbytes
__attribute__((aligned(32))) static int8_t adc_data_buffer[DATA_BUFFER_SIZE +
				 (N_CYCLE_OFFSET * BYTES_PER_SAMPLE)];
#endif

#define DATA_BUFFER_SIZE_CONT		(32768)		// 32kbytes

/* AD4692 Channel Scan structure */
#define AD4692_DEFAULT_CHN_SCAN {\
	.sign = 'u',\
	.realbits = ADC_RESOLUTION,\
	.storagebits = CHN_STORAGE_BITS,\
	.shift = 0,\
	.is_big_endian = true\
}

/* IIOD channel scan configurations */
struct scan_type ad4692_iio_scan_type[NUM_OF_IIO_DEVICES][NO_OF_CHANNELS] = {
	{
		AD4692_DEFAULT_CHN_SCAN,
		AD4692_DEFAULT_CHN_SCAN,
		AD4692_DEFAULT_CHN_SCAN,
		AD4692_DEFAULT_CHN_SCAN,
		AD4692_DEFAULT_CHN_SCAN,
		AD4692_DEFAULT_CHN_SCAN,
		AD4692_DEFAULT_CHN_SCAN,
		AD4692_DEFAULT_CHN_SCAN,
		AD4692_DEFAULT_CHN_SCAN,
		AD4692_DEFAULT_CHN_SCAN,
		AD4692_DEFAULT_CHN_SCAN,
		AD4692_DEFAULT_CHN_SCAN,
		AD4692_DEFAULT_CHN_SCAN,
		AD4692_DEFAULT_CHN_SCAN,
		AD4692_DEFAULT_CHN_SCAN,
		AD4692_DEFAULT_CHN_SCAN
	},
	{
		AD4692_DEFAULT_CHN_SCAN,
		AD4692_DEFAULT_CHN_SCAN,
		AD4692_DEFAULT_CHN_SCAN,
		AD4692_DEFAULT_CHN_SCAN,
		AD4692_DEFAULT_CHN_SCAN,
		AD4692_DEFAULT_CHN_SCAN,
		AD4692_DEFAULT_CHN_SCAN,
		AD4692_DEFAULT_CHN_SCAN,
		AD4692_DEFAULT_CHN_SCAN,
		AD4692_DEFAULT_CHN_SCAN,
		AD4692_DEFAULT_CHN_SCAN,
		AD4692_DEFAULT_CHN_SCAN,
		AD4692_DEFAULT_CHN_SCAN,
		AD4692_DEFAULT_CHN_SCAN,
		AD4692_DEFAULT_CHN_SCAN,
		AD4692_DEFAULT_CHN_SCAN
	}
};

/* IIO Channel Definition */
#define AD4692_IIO_CH(_name, _dev, _idx) {\
	.name = _name,\
	.ch_type = IIO_VOLTAGE,\
	.ch_out = false,\
	.indexed = true,\
	.channel = _idx,\
	.scan_index = _idx,\
	.scan_type = ad4692_iio_scan_type[_dev],\
	.attributes = ad4692_iio_ch_attributes[_dev]\
}

/* Timeout count to avoid stuck into potential infinite loop while checking
 * for new data into an acquisition buffer. The actual timeout factor is determined
 * through 'sampling_frequency' attribute of IIO app, but this period here makes sure
 * we are not stuck into a forever loop in case data capture is interrupted
 * or failed in between.
 * Note: This timeout factor is dependent upon the MCU clock frequency. Below timeout
 * is tested for SDP-K1 platform @180Mhz default core clock */
#define BUF_READ_TIMEOUT 0xffffffff

/* Min and Max values for per channel accumulator count */
#define ACC_COUNT_MIN_VAL	0
#define ACC_COUNT_MAX_VAL	64

/* ADC max count (full scale value) for unipolar inputs */
#define ADC_MAX_COUNT_UNIPOLAR	(uint32_t) ((1 << ADC_RESOLUTION) - 1)

/* AD4692 Scale */
#define AD4692_SCALE  (((float) (AD4692_VREF) / 1000000.0f) / (float)(ADC_MAX_COUNT_UNIPOLAR) * 1000.0f)

/* AD4692 Offset */
#define AD4692_OFFSET	0

/* Maximum number of priorities supported in the FW */
#define AD4692_MAX_PRIORITIES       2

/* Supported resolutions- 16 and 24 bit */
#define AD4692_RES_16		    16
#define AD4692_RES_24		    24

/******************************************************************************/
/*************************** Types Declarations *******************************/
/******************************************************************************/

/* Pointer to the struct representing the AD4692 IIO device */
struct ad4692_desc *ad4692_dev = NULL;

/* IIO interface descriptor */
static struct iio_desc *ad4692_iio_desc;

/* AD4692 IIO hw trigger descriptor */
struct iio_hw_trig *ad4692_hw_trig_desc;

/* AD4692 Board level attribute unique IDs */
enum ad4692_board_attribute_ids {
	PRIORITY,
	PRIORITY_AVAILABLE,
};

/* AD4692 device channel attributes list */
static struct iio_attribute ad4692_iio_ch_attributes[NUM_OF_IIO_DEVICES][5] = {};

/* IIOD device (global) attributes list */
static struct iio_attribute ad4692_iio_global_attributes[NUM_OF_IIO_DEVICES][12]
		= {};

/* IIOD init parameters */
static struct iio_device_init iio_device_init_params[NUM_OF_IIO_DEVICES];

/* AD4692 IIO Channels */
static struct iio_channel
	ad4692_iio_channels[NUM_OF_IIO_DEVICES][NO_OF_CHANNELS] = {
	{
		AD4692_IIO_CH("Chn00", 0, 0),
		AD4692_IIO_CH("Chn01", 0, 1),
		AD4692_IIO_CH("Chn02", 0, 2),
		AD4692_IIO_CH("Chn03", 0, 3),
		AD4692_IIO_CH("Chn04", 0, 4),
		AD4692_IIO_CH("Chn05", 0, 5),
		AD4692_IIO_CH("Chn06", 0, 6),
		AD4692_IIO_CH("Chn07", 0, 7),
		AD4692_IIO_CH("Chn08", 0, 8),
		AD4692_IIO_CH("Chn09", 0, 9),
		AD4692_IIO_CH("Chn10", 0, 10),
		AD4692_IIO_CH("Chn11", 0, 11),
		AD4692_IIO_CH("Chn12", 0, 12),
		AD4692_IIO_CH("Chn13", 0, 13),
		AD4692_IIO_CH("Chn14", 0, 14),
		AD4692_IIO_CH("Chn15", 0, 15),
	},
	{
		AD4692_IIO_CH("Chn00", 1, 0),
		AD4692_IIO_CH("Chn01", 1, 1),
		AD4692_IIO_CH("Chn02", 1, 2),
		AD4692_IIO_CH("Chn03", 1, 3),
		AD4692_IIO_CH("Chn04", 1, 4),
		AD4692_IIO_CH("Chn05", 1, 5),
		AD4692_IIO_CH("Chn06", 1, 6),
		AD4692_IIO_CH("Chn07", 1, 7),
		AD4692_IIO_CH("Chn08", 1, 8),
		AD4692_IIO_CH("Chn09", 1, 9),
		AD4692_IIO_CH("Chn10", 1, 10),
		AD4692_IIO_CH("Chn11", 1, 11),
		AD4692_IIO_CH("Chn12", 1, 12),
		AD4692_IIO_CH("Chn13", 1, 13),
		AD4692_IIO_CH("Chn14", 1, 14),
		AD4692_IIO_CH("Chn15", 1, 15),
	}
};

/* List of channels to be captured */
uint8_t ad4692_active_channels[NO_OF_CHANNELS];

/* Number of channels enabled by the IIO Client */
uint8_t num_of_active_channels = 0;

/* Flag to check end of conversion */
volatile bool ad4692_conversion_flag = false;

/* ADC Mode values */
static const char *ad4692_adc_modes[] = {
	"cnv_clock",
	"cnv_burst",
	"autonomous",
	"spi_burst",
	"manual",
};

/* Channel mask to indicate the enabled channels */
uint16_t channel_mask = 0x1;

/* Accumulator count limit */
uint8_t ad4692_acc_count[NO_OF_CHANNELS] = { 0x0 };

/* Oscillator frequency values */
static const char *ad4692_osc_frequencies[] = {
	"1000kHz",
	"500kHz",
	"400kHz",
	"250kHz",
	"200kHz",
	"167kHz",
	"133kHz",
	"125kHz",
	"100kHz",
	"50kHz",
	"25kHz",
	"12P5kHz",
	"10kHz",
	"5kHz",
	"2P5kHz",
	"1P25kHz"
};

/* Restart IIO option */
static const char *restart_iio_options[] = {
	"Enable",
};

/* List of priorities */
static const char* priorities[] = {
	"disabled",
	"1",
	"2"
};

/* List of sequencer modes */
static const char *seq_modes[] = {
	"standard",
	"advanced"
};

/* List of interface modes */
static const char *interface_modes[] = {
	"spi_dma",
	"spi_interrupt"
};

/* List of data capture modes */
static const char *data_capture_modes[] = {
	"continuous_data_capture",
	"burst_data_capture"
};

/* List of readback modes */
static const char *readback_modes[] = {
	"averaged_data",
	"accumulator_data"
};

/* Selected sequencer mode. Default is standard */
enum ad4692_sequencer_modes ad4692_sequencer_mode = STANDARD_SEQUENCER;

/* Selected interface mode. Default is SPI DMA */
enum ad4692_interface_modes ad4692_interface_mode = SPI_DMA;

/* Selected data capture mode. Default is Burst */
enum ad4692_data_capture_modes ad4692_data_capture_mode = BURST_DATA_CAPTURE;

/* Selected readback option. Default is Averaged data */
enum ad4692_readback_options ad4692_readback_option = AVERAGED_DATA;

/* Restart IIO flag */
static bool restart_iio_flag = false;

/* Default oscillator frequency */
enum ad4692_int_osc_sel ad4692_osc_freq_id = AD4692_OSC_1MHZ;

/* Default channel sequencer length */
static uint8_t seq_len = 0;

/* Sampling frequency */
uint32_t ad4692_sampling_frequency;

/* Array to hold the channel priorities */
uint8_t channel_priorities[NO_OF_CHANNELS] = { 0x0 };

/* Array to hold the channel sequence */
uint8_t channel_sequence[AD4692_MAX_SLOTS_AS] = { 0x0 };

/* Number of slots used in the advanced sequencer */
uint8_t num_of_as_slots = 0;

/* Data number of bytes */
uint8_t n_data_bytes;

/* Number of bytes per SPI transaction for non manual modes */
uint8_t n_bytes_per_transaction = AD4692_N_BYTES_TXN_16BIT;

/* STM32 SPI Init params */
struct stm32_spi_init_param* spi_init_param;

/* IIO interface init parameters */
struct iio_init_param  iio_init_params = {
	.phy_type = USE_UART,
};

/* AD4692 IIO device descriptor */
static struct iio_device *ad4692_iio_dev[NUM_OF_IIO_DEVICES];

/******************************************************************************/
/************************ Functions Definitions *******************************/
/******************************************************************************/

/*!
 * @brief	Getter/Setter for the raw, offset and scale attribute value
 * @param	device[in, out]- Pointer to IIO device instance
 * @param	buf[in]- IIO input data buffer
 * @param	len[in]- Number of input bytes
 * @param	channel[in] - input channel
 * @param	priv[in] - Attribute private ID
 * @return	Number of characters read/written, negative error code otherwise
 */
int ad4692_iio_attr_get(void *device,
			char *buf,
			uint32_t len,
			const struct iio_ch_info *channel,
			intptr_t priv)
{
	int ret;
	uint32_t adc_data;
	uint32_t reg_val;
	uint8_t ch;

	switch (priv) {
	case ADC_RAW_ATTR_ID:
		channel_mask = NO_OS_BIT(channel->ch_num);
		ad4692_active_channels[0] = channel->ch_num;
		num_of_active_channels = 1;

		/* Read the ADC Sample */
		ret = ad4692_data_transfer_read_converted_data(ad4692_dev, channel->ch_num,
				&adc_data);
		if (ret) {
			return ret;
		}

		return sprintf(buf, "%lu", adc_data);

	case ADC_SCALE_ATTR_ID:

		return sprintf(buf, "%f", AD4692_SCALE);

	case ADC_OFFSET_ATTR_ID:
		return sprintf(buf, "%d", AD4692_OFFSET);

	case ADC_SAMPLING_FREQUENCY_ATTR_ID:
		return sprintf(buf, "%lu", ad4692_sampling_frequency);

	case ADC_MODE_ATTR_ID:
		return sprintf(buf, "%s", ad4692_adc_modes[ad4692_init_params.mode]);

	case SEQUENCER_MODE_ATTR_ID:
		return sprintf(buf, "%s", seq_modes[ad4692_sequencer_mode]);

	case INTERFACE_MODE_ATTR_ID:
		return sprintf(buf, "%s", interface_modes[ad4692_interface_mode]);

	case DATA_CAPTURE_MODE_ATTR_ID:
		return sprintf(buf, "%s", data_capture_modes[ad4692_data_capture_mode]);

	case READBACK_OPTION_ATTR_ID:
		return sprintf(buf, "%s", readback_modes[ad4692_readback_option]);

	case ACC_COUNT_ATTR_ID:
		if (ad4692_sequencer_mode == STANDARD_SEQUENCER) {
			ch = 0;
		} else {
			ch = channel->ch_num;
		}

		return sprintf(buf, "%u", ad4692_acc_count[ch]);

	case OSC_FREQUENCY_ATTR_ID:
		return sprintf(buf, "%s", ad4692_osc_frequencies[ad4692_osc_freq_id]);

	case SEQUENCE_LENGTH_ATTR_ID:
		ret = ad4692_reg_read(ad4692_dev, AD4692_SEQUENCER_CONTROL_REG, &reg_val);
		if (ret) {
			return ret;
		}
		seq_len = no_os_field_get(AD4692_ADV_SEQ_MODE_MASK, reg_val);

		return sprintf(buf, "%d", seq_len);

	case ADC_CHN_PRIORITY_ATTR_ID:
		return sprintf(buf, "%s", priorities[channel_priorities[channel->ch_num]]);

	case RESTART_IIO_ATTR_ID:
		return sprintf(buf, "%s", restart_iio_options[0]);

	default:
		return -EINVAL;
	}

	return len;
}

/*!
 * @brief	Setter function for AD4692 attributes
 * @param	device[in, out]- Pointer to IIO device instance
 * @param	buf[in]- IIO input data buffer
 * @param	len[in]- Number of expected bytes
 * @param	channel[in] - input channel
 * @param	priv[in] - Attribute private ID
 * @return	len in case of success, negative error code otherwise
 */
int ad4692_iio_attr_set(void *device,
			char *buf,
			uint32_t len,
			const struct iio_ch_info *channel,
			intptr_t priv)
{
	uint8_t id;
	int ret;
	uint32_t s_rate;
	uint8_t ch;

	switch (priv) {
	case ADC_MODE_ATTR_ID:
		for (id = AD4692_CNV_CLOCK; id <= AD4692_MANUAL_MODE; id++) {
			if (!strcmp(buf, ad4692_adc_modes[id])) {
				break;
			}
		}

		if (id >= NO_OS_ARRAY_SIZE(ad4692_adc_modes) ||
		    (id == AD4692_AUTONOMOUS)) {
			return -EINVAL;
		}

		ad4692_init_params.mode = id;

		break;

	case SEQUENCER_MODE_ATTR_ID:
		for (id = STANDARD_SEQUENCER; id <= ADVANCED_SEQUENCER; id++) {
			if (!strcmp(buf, seq_modes[id])) {
				ad4692_sequencer_mode = id;
				break;
			}
		}

		if (id >= NO_OS_ARRAY_SIZE(seq_modes)) {
			return -EINVAL;
		}

		break;

	case INTERFACE_MODE_ATTR_ID:
		for (id = SPI_DMA; id <= SPI_INTR; id++) {
			if (!strcmp(buf, interface_modes[id])) {
				ad4692_interface_mode = id;
				break;
			}
		}

		if (id >= NO_OS_ARRAY_SIZE(interface_modes)) {
			return -EINVAL;
		}

		break;

	case DATA_CAPTURE_MODE_ATTR_ID:
		for (id = CONTINUOUS_DATA_CAPTURE; id <= BURST_DATA_CAPTURE; id++) {
			if (!strcmp(buf, data_capture_modes[id])) {
				ad4692_data_capture_mode = id;
				break;
			}
		}

		if (id >= NO_OS_ARRAY_SIZE(data_capture_modes)) {
			return -EINVAL;
		}

		break;

	case ACC_COUNT_ATTR_ID:
		if (ad4692_sequencer_mode == STANDARD_SEQUENCER) {
			ch = 0;
		} else {
			ch = channel->ch_num;
		}

		if (no_os_str_to_uint32(buf) > ACC_COUNT_MAX_VAL) {
			return -EINVAL;
		}

		ret = ad4692_reg_write(ad4692_dev, AD4692_ACC_COUNT_LIMIT_REG(ch),
				       no_os_str_to_uint32(buf));
		if (ret) {
			return ret;
		}

		ad4692_acc_count[ch] = no_os_str_to_uint32(buf);

		break;

	case OSC_FREQUENCY_ATTR_ID:
		for (id = AD4692_OSC_1MHZ; id <= AD4692_OSC_1P25KHZ; id++) {
			if (!strcmp(buf, ad4692_osc_frequencies[id])) {
				break;
			}
		}

		ret = ad4692_set_osc(ad4692_dev, id);
		if (ret) {
			return ret;
		}
		ad4692_osc_freq_id = id;

		break;

	case SEQUENCE_LENGTH_ATTR_ID:
		break;

	case READBACK_OPTION_ATTR_ID:
		for (id = AVERAGED_DATA; id <= ACCUMULATOR_DATA; id++) {
			if (!strcmp(buf, readback_modes[id])) {
				ad4692_readback_option = id;
				break;
			}
		}

		if (id >= NO_OS_ARRAY_SIZE(readback_modes)) {
			return -EINVAL;
		}

		break;

	case ADC_SAMPLING_FREQUENCY_ATTR_ID:
		s_rate = no_os_str_to_uint32(buf);
		ret = ad4692_data_transfer_update_freq(&s_rate);
		if (ret) {
			return ret;
		}

		break;

	case ADC_CHN_PRIORITY_ATTR_ID:
		if (!strcmp(buf, priorities[0])) {
			id = 0;
		} else {
			id = no_os_str_to_uint32(buf);
		}

		if (id > AD4692_MAX_PRIORITIES) {
			return -EINVAL;
		}
		channel_priorities[channel->ch_num] = id;

		break;

	case RESTART_IIO_ATTR_ID:
		restart_iio_flag = true;
		break;

	default:
		return -EINVAL;
	}

	return len;
}

/*!
 * @brief	Attribute available getter function for AD4692 attributes
 * @param	device[in, out]- Pointer to IIO device instance
 * @param	buf[in]- IIO input data buffer
 * @param	len[in]- Number of input bytes
 * @param	channel[in] - input channel
 * @param	priv[in] - Attribute private ID
 * @return	len in case of SUCCESS, negative error code otherwise
 */
int ad4692_iio_attr_available_get(void *device,
				  char *buf,
				  uint32_t len,
				  const struct iio_ch_info *channel,
				  intptr_t priv)
{
	switch (priv) {
	case ADC_MODE_ATTR_ID:
		return sprintf(buf,
			       "%s %s %s %s",
			       ad4692_adc_modes[0],
			       ad4692_adc_modes[1],
			       ad4692_adc_modes[3],
			       ad4692_adc_modes[4]);

	case SEQUENCER_MODE_ATTR_ID:
		if (ad4692_init_params.mode == AD4692_MANUAL_MODE) {
			return sprintf(buf, "%s",
				       seq_modes[0]);
		} else {
			return sprintf(buf, "%s %s",
				       seq_modes[0],
				       seq_modes[1]);
		}

	case INTERFACE_MODE_ATTR_ID:
		if (ad4692_init_params.mode == AD4692_MANUAL_MODE) {
			return sprintf(buf, "%s %s",
				       interface_modes[0],
				       interface_modes[1]);
		} else {
			return sprintf(buf, "%s",
				       interface_modes[1]);
		}

	case DATA_CAPTURE_MODE_ATTR_ID:
		return sprintf(buf, "%s %s",
			       data_capture_modes[0],
			       data_capture_modes[1]);

	case OSC_FREQUENCY_ATTR_ID:
		return sprintf(buf,
			       "%s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s",
			       ad4692_osc_frequencies[0],
			       ad4692_osc_frequencies[1],
			       ad4692_osc_frequencies[2],
			       ad4692_osc_frequencies[3],
			       ad4692_osc_frequencies[4],
			       ad4692_osc_frequencies[5],
			       ad4692_osc_frequencies[6],
			       ad4692_osc_frequencies[7],
			       ad4692_osc_frequencies[8],
			       ad4692_osc_frequencies[9],
			       ad4692_osc_frequencies[10],
			       ad4692_osc_frequencies[11],
			       ad4692_osc_frequencies[12],
			       ad4692_osc_frequencies[13],
			       ad4692_osc_frequencies[14],
			       ad4692_osc_frequencies[15]);

	case ADC_CHN_PRIORITY_ATTR_ID:
		return sprintf(buf, "%s %s %s", priorities[0],
			       priorities[1],
			       priorities[2]);

	case READBACK_OPTION_ATTR_ID:
		return sprintf(buf, "%s %s",
			       readback_modes[0],
			       readback_modes[1]);

	case RESTART_IIO_ATTR_ID:
		return sprintf(buf, "%s", restart_iio_options[0]);

	default:
		return -EINVAL;
	}

	return len;
}

/*!
 * @brief Attribute available setter function for AD4692 attributes
 * @param device[in, out]- Pointer to IIO device instance
 * @param buf[in]- IIO input data buffer
 * @param len[in]- Number of input bytes
 * @param channel[in] - input channel
 * @param priv[in] - Attribute private ID
 * @return len in case of success, negative error code otherwise
 */
int ad4692_iio_attr_available_set(void *device,
				  char *buf,
				  uint32_t len,
				  const struct iio_ch_info *channel,
				  intptr_t priv)
{
	return len;
}

/*!
 * @brief Interrupt Service Routine to monitor end of conversion event.
 * @param ctx[in] - Callback context (unused)
 * @return none
 */
void ad4692_data_capture_callback(void *ctx)
{
	ad4692_conversion_flag = true;
}

/*!
 * @brief Read the debug register value
 * @param dev[in, out]- Pointer to IIO device instance
 * @param reg[in]- Register address to read from
 * @param readval[out]- Pointer to variable to read data into
 * @return 0 in case of success, negative value otherwise
 */
static int32_t ad4692_iio_debug_reg_read(void *dev,
		uint32_t reg,
		uint32_t *readval)
{
	int32_t ret;

	if (!readval || (reg > AD4692_ACC_STS_DATA_REG(NO_OF_CHANNELS))) {
		return -EINVAL;
	}

	ret =  ad4692_reg_read(ad4692_dev, reg, readval);
	if (NO_OS_IS_ERR_VALUE(ret)) {
		return ret;
	}

	return 0;
}

/*!
 * @brief Write to the debug register value
 * @param dev[in, out]- Pointer to IIO device instance
 * @param reg[in]- Register address to write
 * @param writeval[in]- Variable storing data to write
 * @return 0 in case of success, negative value otherwise
 */
static int32_t ad4692_iio_debug_reg_write(void *dev,
		uint32_t reg,
		uint32_t writeval)
{
	int32_t ret;

	if (reg > AD4692_ACC_STS_DATA_REG(NO_OF_CHANNELS)) {
		return -EINVAL;
	}

	ret = ad4692_reg_write(ad4692_dev, reg, writeval);
	if (NO_OS_IS_ERR_VALUE(ret)) {
		return ret;
	}

	return 0;
}

/**
 * @brief Init for reading/writing and parameterization of a AD4692 IIO device
 * @param desc[in,out] - IIO device descriptor
 * @param dev_indx[in] - IIO device number
 * @return 0 in case of success, negative error code otherwise
 */
static int ad4692_iio_init(struct iio_device **desc, uint8_t dev_indx)
{
	struct iio_device *ad4692_iio_inst;
	uint8_t channel_index;
	bool endianness;
	uint8_t realbits;
	uint8_t ch;
	static struct iio_channel channels[AD4692_MAX_CHANNELS];
	uint8_t total_enabled_channels = 0;

	if (!desc) {
		return -EINVAL;
	}

	ad4692_iio_inst = no_os_calloc(1, sizeof(struct iio_device));
	if (!ad4692_iio_inst) {
		return -ENOMEM;
	}

	if (ad4692_sequencer_mode == STANDARD_SEQUENCER) {
		for (ch = 0; ch < AD4692_MAX_CHANNELS; ch++) {
			channels[ch] = ad4692_iio_channels[dev_indx][ch];
			total_enabled_channels++;
		}

		switch (ad4692_init_params.mode) {
		case AD4692_MANUAL_MODE:
			memcpy(ad4692_iio_global_attributes[dev_indx],
			       ad4692_manual_global_attr[dev_indx],
			       sizeof(ad4692_manual_global_attr[dev_indx]));
			break;

		case AD4692_CNV_BURST:
		case AD4692_SPI_BURST:
			memcpy(ad4692_iio_global_attributes[dev_indx],
			       ad4692_std_seq_burst_global_attr[dev_indx],
			       sizeof(ad4692_std_seq_burst_global_attr[dev_indx]));
			break;

		case AD4692_CNV_CLOCK:
			memcpy(ad4692_iio_global_attributes[dev_indx],
			       ad4692_std_seq_cnv_clock_global_attr[dev_indx],
			       sizeof(ad4692_std_seq_cnv_clock_global_attr[dev_indx]));
			break;

		default:
			no_os_free(ad4692_iio_inst);
			return -EINVAL;
		}

		memcpy(ad4692_iio_ch_attributes[dev_indx], ad4692_std_seq_ch_attr[dev_indx],
		       sizeof(ad4692_std_seq_ch_attr[dev_indx]));
	} else {
		/* Include only channels that have a priority assigned */
		for (ch = 0; ch < AD4692_MAX_CHANNELS; ch++) {
			if (channel_priorities[ch] != 0) {
				channels[total_enabled_channels++] = ad4692_iio_channels[dev_indx][ch];
			}

			switch (ad4692_init_params.mode) {
			case AD4692_CNV_BURST:
			case AD4692_SPI_BURST:
				memcpy(ad4692_iio_global_attributes[dev_indx],
				       ad4692_adv_seq_burst_global_attr[dev_indx],
				       sizeof(ad4692_adv_seq_burst_global_attr[dev_indx]));
				break;

			case AD4692_CNV_CLOCK:
				memcpy(ad4692_iio_global_attributes[dev_indx],
				       ad4692_adv_seq_cnv_clock_global_attr[dev_indx],
				       sizeof(ad4692_adv_seq_cnv_clock_global_attr[dev_indx]));
				break;

			default:
				no_os_free(ad4692_iio_inst);
				return -EINVAL;
			}
		}

		memcpy(ad4692_iio_ch_attributes[dev_indx], ad4692_adv_seq_ch_attr[dev_indx],
		       sizeof(ad4692_adv_seq_ch_attr[dev_indx]));
	}

	ad4692_iio_inst->num_ch = total_enabled_channels;
	ad4692_iio_inst->channels = channels;
	ad4692_iio_inst->attributes = ad4692_iio_global_attributes[dev_indx];
	ad4692_iio_inst->submit = ad4692_data_transfer_submit;
	ad4692_iio_inst->pre_enable = ad4692_data_transfer_prepare;
	ad4692_iio_inst->post_disable = ad4692_data_transfer_end;
	ad4692_iio_inst->debug_reg_read = ad4692_iio_debug_reg_read;
	ad4692_iio_inst->debug_reg_write = ad4692_iio_debug_reg_write;

	if (ad4692_data_capture_mode == CONTINUOUS_DATA_CAPTURE) {
		ad4692_iio_inst->trigger_handler = ad4692_data_transfer_trigger_handler;
	}

	/* Configure the endianness in Channel scan structure */
	if (ad4692_init_params.mode == AD4692_MANUAL_MODE) {
		endianness = true;
		realbits = AD4692_RES_16;
		n_data_bytes = sizeof(uint16_t);
	} else {
		endianness = false;

		if (ad4692_readback_option == AVERAGED_DATA) {
			realbits = AD4692_RES_16;
			n_data_bytes = sizeof(uint16_t);
			n_bytes_per_transaction = AD4692_N_BYTES_TXN_16BIT;
		} else {
			realbits = AD4692_RES_24;
			n_data_bytes = sizeof(uint32_t);
			n_bytes_per_transaction = AD4692_N_BYTES_TXN_24BIT;
		}
	}

	for (channel_index = 0; channel_index < NO_OF_CHANNELS; channel_index++) {
		ad4692_iio_channels[0][channel_index].scan_type[0].is_big_endian = endianness;
		ad4692_iio_channels[0][channel_index].scan_type[0].realbits = realbits;
		ad4692_iio_channels[0][channel_index].scan_type[0].storagebits = n_data_bytes *
				8;
	}

	*desc = ad4692_iio_inst;

	return 0;
}

/**
 * @brief	Initialization of AD4692 IIO hardware trigger specific parameters
 * @param 	desc[in,out] - IIO hardware trigger descriptor
 * @return	0 in case of success, negative error code otherwise
 */
static int32_t ad4692_iio_trigger_param_init(struct iio_hw_trig **desc)
{
	int32_t ret;
	struct iio_hw_trig_init_param ad4692_hw_trig_init_params;
	struct iio_hw_trig *hw_trig_desc;

	if (!desc) {
		return -EINVAL;
	}

	if (ad4692_init_params.mode == AD4692_SPI_BURST) {
		ad4692_hw_trig_init_params.irq_id = SPI_BURST_PWM_ID;
	} else {
		ad4692_hw_trig_init_params.irq_id = TRIGGER_INT_ID;
	}

	ad4692_hw_trig_init_params.name = AD4692_IIO_TRIGGER_NAME;
	ad4692_hw_trig_init_params.irq_trig_lvl = NO_OS_IRQ_EDGE_FALLING;
	ad4692_hw_trig_init_params.irq_ctrl = trigger_irq_desc;
	ad4692_hw_trig_init_params.cb_info.event = NO_OS_EVT_GPIO;
	ad4692_hw_trig_init_params.cb_info.peripheral = NO_OS_GPIO_IRQ;
	ad4692_hw_trig_init_params.cb_info.handle = trigger_gpio_handle;
	ad4692_hw_trig_init_params.iio_desc = ad4692_iio_desc;

	/* Initialize hardware trigger */
	ret = iio_hw_trig_init(&hw_trig_desc, &ad4692_hw_trig_init_params);
	if (ret) {
		return ret;
	}

	*desc = hw_trig_desc;

	return 0;
}

/**
 * @brief Init for reading/writing and parameterization of a AD4692 Board IIO device
 * @param desc[in,out] - IIO device descriptor
 * @param dev_indx[in] - IIO Device index
 * @return 0 in case of success, negative error code otherwise
 */
static int board_iio_params_init(struct iio_device** desc,
				 uint8_t dev_indx)
{
	struct iio_device* iio_dev;

	if (!desc) {
		return -EINVAL;
	}

	iio_dev = no_os_calloc(1, sizeof(*iio_dev));
	if (!iio_dev) {
		return -ENOMEM;
	}

	if (ad4692_sequencer_mode == ADVANCED_SEQUENCER) {
		memcpy(ad4692_iio_ch_attributes[dev_indx], ad4692_adv_seq_ch_attr[dev_indx],
		       sizeof(ad4692_adv_seq_ch_attr[dev_indx]));
	} else {
		memcpy(ad4692_iio_ch_attributes[dev_indx], ad4692_std_seq_ch_attr[dev_indx],
		       sizeof(ad4692_std_seq_ch_attr[dev_indx]));
	}

	memcpy(ad4692_iio_global_attributes[dev_indx],
	       ad4692_manual_global_attr[dev_indx],
	       sizeof(ad4692_manual_global_attr[dev_indx]));

	iio_dev->num_ch = NO_OS_ARRAY_SIZE(ad4692_iio_channels[dev_indx]);
	iio_dev->channels = ad4692_iio_channels[dev_indx];
	iio_dev->attributes = ad4692_iio_global_attributes[dev_indx];

	*desc = iio_dev;

	return 0;
}

/**
 * @brief	DeInitialize the IIO parameters.
 */
void iio_params_deinit(void)
{
	uint8_t indx = 0;

	for (indx = 0 ; indx < iio_init_params.nb_devs; indx++) {
		if (ad4692_iio_dev[indx] != NULL) {
			no_os_free(ad4692_iio_dev[indx]);
			ad4692_iio_dev[indx] = NULL;
		}
	}

	iio_init_params.nb_devs = 0;
	iio_init_params.nb_trigs = 0;
}

/**
 * @brief	Remove the IIO application and free the allocated resources
 * @return  0
 */
int32_t iio_app_remove(void)
{
	/* Remove data transfer system */
	ad4692_data_transfer_remove(ad4692_dev);

	/* Remove hardware trigger if allocated (SPI_INTR + CONTINUOUS_DATA_CAPTURE) */
	if (ad4692_hw_trig_desc) {
		iio_hw_trig_remove(ad4692_hw_trig_desc);
		ad4692_hw_trig_desc = NULL;
	}

	/* Remove interrupt controller if allocated (SPI_INTR) */
	remove_interrupt();

	/* Remove SPI burst PWM if allocated (SPI_INTR + SPI_BURST) */
	remove_pwm();

	NO_OS_UNUSED_PARAM(ad4692_remove(ad4692_dev));
	ad4692_dev = NULL;

	iio_params_deinit();

	NO_OS_UNUSED_PARAM(iio_remove(ad4692_iio_desc));
	ad4692_iio_desc = NULL;

	remove_iio_context_attributes(iio_init_params.ctx_attrs);
	iio_init_params.ctx_attrs = NULL;

	return 0;
}

/**
 * @brief Initialize the AD4692 IIO Application
 * @return 0 in case of success, negative value otherwise
 */
int32_t iio_app_initialize(void)
{
	int32_t ret;

	/* EVB HW validation status */
	bool hw_mezzanine_is_valid;

	static struct iio_trigger ad4692_iio_trig_desc = {
		.is_synchronous = true,
		.enable = NULL,
		.disable = NULL
	};

	static struct iio_trigger_init iio_trigger_init_params = {
		.descriptor = &ad4692_iio_trig_desc,
		.name = AD4692_IIO_TRIGGER_NAME,
	};

	/* Configure interface modes.
	 * Only SPI interrupt permissible for modes other than manual */
	if (ad4692_init_params.mode == AD4692_MANUAL_MODE) {
		ad4692_interface_mode = SPI_DMA;
	} else {
		ad4692_interface_mode = SPI_INTR;
	}

	if ((ad4692_interface_mode == SPI_INTR)
	    && (ad4692_data_capture_mode == CONTINUOUS_DATA_CAPTURE)) {
		iio_init_params.trigs = &iio_trigger_init_params;
	}

	/* Configure to Standard Sequencer in manual Mode */
	if (ad4692_init_params.mode == AD4692_MANUAL_MODE) {
		ad4692_sequencer_mode = STANDARD_SEQUENCER;
	}

	/* Set the PWM period based on the mode */
	ad4692_sampling_frequency = ad4692_get_max_sampling_rate(
					    ad4692_init_params.mode);
	if (ad4692_sampling_frequency) {
		pwm_init_convst.period_ns = CONV_TRIGGER_PERIOD_NSEC(ad4692_sampling_frequency);
		pwm_init_convst.duty_cycle_ns = CNV_ON_TIME;
	}

	ret = init_interrupt();
	if (ret) {
		return ret;
	}

	/* Read context attributes */
	ret = get_iio_context_attributes_ex(&iio_init_params.ctx_attrs,
					    &iio_init_params.nb_ctx_attr,
					    eeprom_desc,
					    HW_MEZZANINE_NAME,
					    STR(HW_CARRIER_NAME),
					    &hw_mezzanine_is_valid,
					    FIRMWARE_VERSION);

	if (ret || !hw_mezzanine_is_valid) {
		goto iio_init;
	}

	do {
		ret = ad4692_init(&ad4692_dev, &ad4692_init_params);
		if (ret) {
			goto system_config_init;
		}

		/* Exit from manual mode if device is configured to manual mode */
		if (ad4692_dev->mode == AD4692_MANUAL_MODE) {
			ret = ad4692_exit_manual_mode(ad4692_dev);
			if (ret) {
				goto err_remove_ad4692_dev;
			}
		}

		/* Register and initialize the AD4692 device into IIO interface */
		ret = ad4692_iio_init(&ad4692_iio_dev[0], 0);
		if (ret) {
			goto err_remove_ad4692_dev;
		}

		/* Initialize the IIO interface */
		iio_device_init_params[0].name = ACTIVE_DEVICE_NAME;
		iio_device_init_params[0].raw_buf = (int8_t *)adc_data_buffer +
						    (N_CYCLE_OFFSET * BYTES_PER_SAMPLE);
		if (ad4692_data_capture_mode == CONTINUOUS_DATA_CAPTURE) {
			iio_device_init_params[0].raw_buf_len = DATA_BUFFER_SIZE_CONT;
		} else {
			iio_device_init_params[0].raw_buf_len = DATA_BUFFER_SIZE;
		}
		iio_device_init_params[0].dev = ad4692_dev;
		iio_device_init_params[0].dev_descriptor = ad4692_iio_dev[0];

		iio_init_params.nb_devs++;

		if ((ad4692_interface_mode == SPI_INTR)
		    && (ad4692_data_capture_mode == CONTINUOUS_DATA_CAPTURE)) {
			iio_device_init_params[0].trigger_id = "trigger0";
			iio_init_params.nb_trigs++;
		}
		break;
	} while (false);

	if (ad4692_dev) {
		/* Configure the channel priorities in advanced sequencer mode */
		if (ad4692_sequencer_mode == ADVANCED_SEQUENCER) {
			ret = ad4692_configure_channel_priorities(channel_priorities,
					channel_sequence,
					&num_of_as_slots,
					ad4692_acc_count);
			if (ret) {
				goto err_remove_ad4692;
			}
		} else {
			memset(ad4692_acc_count, 0x0, NO_OF_CHANNELS);
		}

		/* Initialize data transfer system for selected mode */
		/*
		 * Note: ad4692_init_params.mode is used instead of ad4692_dev->mode
		 * because when ad4692_init_params.mode is MANUAL the device moves to
		 * MANUAL mode only when data capture occurs. The ADC doesn't accept
		 * any register read/write when in MANUAL mode. Hence use CNV_CLOCK
		 * mode for all register read/write and use MANUAL mode only for data
		 * capture.
		 */
		ret = ad4692_data_transfer_init(ad4692_dev, ad4692_init_params.mode);
		if (ret) {
			goto err_remove_ad4692;
		}
	}

	/* Goto System Config initialization if success */
	goto system_config_init;

err_remove_ad4692:
	no_os_free(ad4692_iio_dev[0]);
	iio_init_params.nb_devs = 0;
	iio_init_params.nb_trigs = 0;
err_remove_ad4692_dev:
	ad4692_remove(ad4692_dev);
	ad4692_dev = NULL;

system_config_init:
	/* Initialize board IIO paramaters */
	ret = board_iio_params_init(&ad4692_iio_dev[iio_init_params.nb_devs], 1);
	if (ret) {
		goto iio_init;
	}

	iio_device_init_params[iio_init_params.nb_devs].name = "system_config";
	iio_device_init_params[iio_init_params.nb_devs].dev_descriptor =
		ad4692_iio_dev[iio_init_params.nb_devs];
	iio_init_params.nb_devs++;

iio_init:
	/* Initialize the IIO interface */
	iio_init_params.uart_desc = uart_iio_com_desc;
	iio_init_params.devs = iio_device_init_params;
	ret = iio_init(&ad4692_iio_desc, &iio_init_params);
	if (ret) {
		goto err_iio_init;
	}

	if ((ad4692_interface_mode == SPI_INTR)
	    && (ad4692_data_capture_mode == CONTINUOUS_DATA_CAPTURE)) {
		ret = ad4692_iio_trigger_param_init(&ad4692_hw_trig_desc);
		if (ret) {
			return ret;
		}
	}

	return 0;

err_iio_init:
	iio_app_remove();
	return ret;
}

/**
 * @brief 	Run the ad4692 IIO event handler
 * @return	None
 */
void iio_app_event_handler(void)
{
	if (restart_iio_flag) {

		iio_app_remove();

		/* Reset the restart_iio flag */
		restart_iio_flag = false;

		iio_app_initialize();
	}

	iio_step(ad4692_iio_desc);
}
