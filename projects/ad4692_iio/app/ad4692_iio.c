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

/******** Forward declaration of functions ********/

static int ad4692_read_converted_data(struct ad4692_desc *desc,
				      uint8_t chn,
				      uint32_t *adc_data);

static int ad4692_start_data_capture(struct ad4692_desc *desc);

static int ad4692_stop_data_capture(struct ad4692_desc *desc);

static void ad4692_get_tx_command(uint8_t* local_tx_data);

/******************************************************************************/
/************************ Macros/Constants ************************************/
/******************************************************************************/

/* IIO trigger name */
#define AD4692_IIO_TRIGGER_NAME		"ad4692_iio_trigger"

/* ADC data buffer size */
#if defined(USE_SDRAM)
/* Offset the IIO buffer for 4 bytes to accommodate the 2-cycle command offset in manual mode */
#define adc_data_buffer				SDRAM_START_ADDRESS + (N_CYCLE_OFFSET * BYTES_PER_SAMPLE)
#define DATA_BUFFER_SIZE			SDRAM_SIZE_BYTES
#else
#define DATA_BUFFER_SIZE			(32768)		// 32kbytes
static int8_t adc_data_buffer[DATA_BUFFER_SIZE];
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

/* Stop State mask */
#define AD4692_ADC_MODE_STOP_STATE_MASK  NO_OS_BIT(5)

/* Data Ready Stop state */
#define AD4692_STOP_STATE_DATA_READYb   0x1

/* ADC max count (full scale value) for unipolar inputs */
#define ADC_MAX_COUNT_UNIPOLAR	(uint32_t) ((1 << ADC_RESOLUTION) - 1)

/* AD4692 Scale */
#define AD4692_SCALE  (((float) (AD4692_VREF) / 1000000.0f) / (float)(ADC_MAX_COUNT_UNIPOLAR) * 1000.0f)

/* AD4692 Offset */
#define AD4692_OFFSET	0

/* Converts pwm period in nanoseconds to sampling frequency in samples per second */
#define PWM_PERIOD_TO_FREQUENCY(x)       (1000000000.0 / x)

/* Maximum number of priorities supported in the FW */
#define AD4692_MAX_PRIORITIES       2

/* Supported resolutions- 16 and 24 bit */
#define AD4692_RES_16		    16
#define AD4692_RES_24		    24

/* Number of bytes per transaction for accumulator data */
#define AD4692_N_BYTES_CNV_CLOCK_24BIT	5

/* Number of bytes per transaction for averaged data */
#define AD4692_N_BYTES_CNV_CLOCK_16BIT	4

/******************************************************************************/
/*************************** Types Declarations *******************************/
/******************************************************************************/

/* Pointer to the struct representing the AD4692 IIO device */
struct ad4692_desc *ad4692_dev = NULL;

/* IIO interface descriptor */
static struct iio_desc *ad4692_iio_desc;

/* AD4692 IIO hw trigger descriptor */
static struct iio_hw_trig *ad4692_hw_trig_desc;

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
static uint8_t ad4692_active_channels[NO_OF_CHANNELS];

/* Channel ID during data capture */
volatile uint8_t chan_id = 0;

/* Number of channels enabled by the IIO Client */
uint8_t num_of_active_channels = 0;

/* Data buffer */
uint8_t data_buff[BYTES_PER_SAMPLE] = { 0x0 };

/* SPI message for manual mode capture */
struct no_os_spi_msg ad4692_spi_msg_manual_mode = {
	.cs_change = CS_CHANGE,
	.tx_buff = data_buff,
	.rx_buff = data_buff,
	.bytes_number = BYTES_PER_SAMPLE
};

/* Flag to check end of conversion */
static volatile bool ad4692_conversion_flag = false;

/* Flag to indicate if size of the buffer is updated according to requested
 * number of samples for the multi-channel IIO buffer data alignment */
static volatile bool buf_size_updated = false;

/* Variable to store number of requested samples */
static uint32_t nb_of_samples;

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

/* Enum of readback options */
enum ad4692_readback_options {
	AVERAGED_DATA,
	ACCUMULATOR_DATA
};

/* Selected sequencer mode. Default is standard */
static enum ad4692_sequencer_modes ad4692_sequencer_mode = STANDARD_SEQUENCER;

/* Selected interface mode. Default is SPI DMA */
enum ad4692_interface_modes ad4692_interface_mode = SPI_DMA;

/* Selected data capture mode. Default is Burst */
enum ad4692_data_capture_modes ad4692_data_capture_mode = BURST;

/* Selected readback option. Default is Averaged data */
enum ad4692_readback_options ad4692_readback_option = AVERAGED_DATA;

/* Restart IIO flag */
static bool restart_iio_flag = false;

/* Default oscillator frequency */
static enum ad4692_int_osc_sel osc_freq_id = AD4692_OSC_1MHZ;

/* Default channel sequencer length */
static uint8_t seq_len = 0;

/* Sampling frequency */
uint32_t ad4692_sampling_frequency;

/* Maximum Sampling frequency */
uint32_t ad4692_sampling_frequency_max;

/* EVB HW validation status */
static bool hw_mezzanine_is_valid;

/* Array to hold the channel priorities */
uint8_t channel_priorities[NO_OF_CHANNELS] = { 0x0 };

/* Array to hold the channel sequence */
uint8_t channel_sequence[AD4692_MAX_SLOTS_AS] = { 0x0 };

/* Number of slots used in the advanced sequencer */
uint8_t num_of_as_slots = 0;

/* Data number of bytes */
uint8_t n_data_bytes;

/* STM32 SPI Init params */
struct stm32_spi_init_param* spi_init_param;

/* Global Pointer for IIO Device Data */
volatile struct iio_device_data* iio_dev_data_g;

/* Global variable for number of samples */
uint32_t nb_of_samples_g;

/* Global variable for data read from CB functions */
uint32_t data_read;

/* Flag to indicate if DMA has been configured for capture */
volatile bool dma_config_updated = false;

/* Flag for checking DMA buffer overflow */
volatile bool ad4692_dma_buff_full = false;

/* Variable to store start of buffer address */
volatile uint32_t *buff_start_addr;

/* Local buffer */
#define MAX_LOCAL_BUF_SIZE	65536
uint8_t local_buf[MAX_LOCAL_BUF_SIZE + (NO_OF_CHANNELS * N_CYCLE_OFFSET)];

/* Maximum value the DMA NDTR register can take */
#define MAX_DMA_NDTR		(no_os_min(65532, (MAX_LOCAL_BUF_SIZE)))

/* Variable to track the number of entries to the complete callback */
uint32_t callback_count = 0;

static bool dma_capture = false;

/* IIO interface init parameters */
struct iio_init_param  iio_init_params = {
	.phy_type = USE_UART,
};

/* AD4692 IIO device descriptor */
static struct iio_device *ad4692_iio_dev[NUM_OF_IIO_DEVICES];

/* Accumulator data buffer */
static uint8_t acc_data_buff[AD4692_N_BYTES_CNV_CLOCK_24BIT *
							    AD4692_MAX_CHANNELS] = { 0x0 };

/* Number of bytes per SPI transaction for non manual modes */
static uint8_t n_bytes_per_transaction = AD4692_N_BYTES_CNV_CLOCK_16BIT;

/******************************************************************************/
/************************ Functions Definitions *******************************/
/******************************************************************************/

/**
 * @brief Set the sampling rate and get the updated value
 *	  supported by MCU platform
 * @return 0 in case of success, negative error code otherwise
 */
int ad4692_update_sampling_frequency(uint32_t *sampling_rate)
{
	int ret;
	uint64_t pwm_period_ns;
	uint32_t period_ns_readback;
	uint32_t prescaler;

	if (*sampling_rate > ad4692_sampling_frequency_max) {
		*sampling_rate = ad4692_sampling_frequency_max;
	}

	if (ad4692_interface_mode == SPI_DMA) {
		ad4692_init_params.conv_param->period_ns = CONV_TRIGGER_PERIOD_NSEC(
					*sampling_rate);
		pwm_period_ns = CONV_TRIGGER_PERIOD_NSEC(*sampling_rate);

		/* Compute prescaler to fit the period value within respective register */
		ret = compute_optimal_prescaler(ad4692_dev->conv_desc->extra, pwm_period_ns,
						&prescaler);
		if (ret) {
			return ret;
		}

		/* Set prescaler for timer */
		ret = set_timer_prescaler(ad4692_dev->conv_desc, prescaler);
		if (ret) {
			return ret;
		}

		/* Initialize PWM with the updated rate */
		ret = no_os_pwm_set_period(ad4692_dev->conv_desc,
					   ad4692_init_params.conv_param->period_ns);
		if (ret) {
			return ret;
		}
	} else { // SPI_INTR

		if (ad4692_init_params.mode == AD4692_SPI_BURST) {
			pwm_period_ns = CONV_TRIGGER_PERIOD_NSEC(*sampling_rate);

			/* Compute prescaler to fit the period value within respective register */
			ret = compute_optimal_prescaler(spi_burst_pwm_desc->extra, pwm_period_ns,
							&prescaler);
			if (ret) {
				return ret;
			}

			/* Set prescaler for timer */
			ret = set_timer_prescaler(spi_burst_pwm_desc, prescaler);
			if (ret) {
				return ret;
			}

			ret = no_os_pwm_set_period(spi_burst_pwm_desc, pwm_period_ns);
			if (ret) {
				return ret;
			}
		} else {
			pwm_period_ns = CONV_TRIGGER_PERIOD_NSEC(*sampling_rate);

			/* Compute prescaler to fit the period value within respective register */
			ret = compute_optimal_prescaler(ad4692_dev->conv_desc->extra, pwm_period_ns,
							&prescaler);
			if (ret) {
				return ret;
			}

			/* Set prescaler for timer */
			ret = set_timer_prescaler(ad4692_dev->conv_desc, prescaler);
			if (ret) {
				return ret;
			}

			ret = no_os_pwm_set_period(ad4692_dev->conv_desc, pwm_period_ns);
			if (ret) {
				return ret;
			}

			ret = no_os_pwm_set_duty_cycle(ad4692_dev->conv_desc,
						       CONV_TRIGGER_DUTY_CYCLE_NSEC(pwm_period_ns));
			if (ret) {
				return ret;
			}
		}
	}

	if (ad4692_init_params.mode == AD4692_SPI_BURST) {
		/* Get the actual period of the PWM */
		ret = no_os_pwm_get_period(spi_burst_pwm_desc, &period_ns_readback);
		if (ret) {
			return ret;
		}
	} else {
		/* Get the actual period of the PWM */
		ret = no_os_pwm_get_period(ad4692_dev->conv_desc, &period_ns_readback);
		if (ret) {
			return ret;
		}
	}

	ad4692_sampling_frequency = PWM_PERIOD_TO_FREQUENCY(period_ns_readback);

	return 0;
}

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
	uint32_t data_read;
	uint8_t ch;

	switch (priv) {
	case ADC_RAW_ATTR_ID:
		dma_capture = false;

		/* Check if device is already in manual mode, exit from the same */
		if (ad4692_init_params.mode == AD4692_MANUAL_MODE) {
			ret = ad4692_stop_data_capture(ad4692_dev);
			if (ret) {
				return ret;
			}
		}

		channel_mask = NO_OS_BIT(channel->ch_num);
		ad4692_active_channels[0] = channel->ch_num;
		num_of_active_channels = 1;

		/* Start Data capture */
		ret = ad4692_start_data_capture(ad4692_dev);
		if (ret) {
			return ret;
		}

		/* Read the ADC Sample */
		ret = ad4692_read_converted_data(ad4692_dev, channel->ch_num, &data_read);
		if (ret) {
			return ret;
		}

		/* Stop Data capture */
		ret = ad4692_stop_data_capture(ad4692_dev);
		if (ret) {
			return ret;
		}

		return sprintf(buf, "%ld", data_read);

	case ADC_SCALE_ATTR_ID:

		return sprintf(buf, "%f", AD4692_SCALE);

	case ADC_OFFSET_ATTR_ID:
		return sprintf(buf, "%d", AD4692_OFFSET);

	case ADC_SAMPLING_FREQUENCY_ATTR_ID:
		return sprintf(buf, "%ld", ad4692_sampling_frequency);

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
		/* In Standard Sequencer Mode, the ACC_DEPTH_IN0 register sets
		 * the accumulator depth for all 16 channels. For Advanced Sequencer Mode,
		 * each channel's depth settings are independently programmed via the
		 * ACC_DEPTH_IN0, ACC_DEPTH_IN1, ... , ACC_DEPTH_IN15 registers */
		if (ad4692_sequencer_mode == STANDARD_SEQUENCER) {
			ch = 0;
		} else {
			ch = channel->ch_num;
		}

		return sprintf(buf, "%d", ad4692_acc_count[ch]);

	case OSC_FREQUENCY_ATTR_ID:
		return sprintf(buf, "%s", ad4692_osc_frequencies[osc_freq_id]);

	case SEQUENCE_LENGTH_ATTR_ID:
		ret = ad4692_reg_read(ad4692_dev, AD4692_SEQUENCER_CONTROL_REG, &data_read);
		if (ret) {
			return ret;
		}
		seq_len = no_os_field_get(AD4692_ADV_SEQ_MODE_MASK, data_read);

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

		/* Update the mode in the init params
		 * to include the updated mode during IIO re-initialization */
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
		for (id = CONTINUOUS; id <= BURST; id++) {
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
		/* In Standard Sequencer Mode, the ACC_DEPTH_IN0 register sets
		 * the accumulator depth for all 16 channels. For Advanced Sequencer Mode,
		 * each channel's depth settings are independently programmed via the
		 * ACC_DEPTH_IN0, ACC_DEPTH_IN1, ... , ACC_DEPTH_IN15 registers */
		if (ad4692_sequencer_mode == STANDARD_SEQUENCER) {
			ch = 0;
		} else {
			ch = channel->ch_num;
		}

		if (no_os_str_to_uint32(buf) > ACC_COUNT_MAX_VAL) {
			return -EINVAL;
		}

		/* Set the channel to the configured accumulator count */
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

		/* Configure the selected oscillator frequency */
		ret = ad4692_set_osc(ad4692_dev, id);
		if (ret) {
			return ret;
		}
		osc_freq_id = id;

		break;

	case SEQUENCE_LENGTH_ATTR_ID:
		/* Channel sequence length is a read-only attribute */
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
		ret = ad4692_update_sampling_frequency(&s_rate);
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
		/* Set flag to true */
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
		/* SPI DMA mode is supported only for manual mode */
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
 * @brief Read ADC converted data
 * @param desc[in, out]- AD4692 device descriptor
 * @param chn[in] - Channel ID (number)
 * @param adc_data[out] - ADC converted data
 * @return len in case of success, negative error code otherwise
 */
static int ad4692_read_converted_data(struct ad4692_desc *desc,
				      uint8_t chn,
				      uint32_t *adc_data)
{
	int ret;
	uint8_t eoc_status;
	uint32_t timeout = BUF_READ_TIMEOUT;

	switch (ad4692_init_params.mode) {
	case AD4692_MANUAL_MODE:
		data_buff[0] = AD4692_IN_COMMAND(chn);
		data_buff[1] = 0x0;
		eoc_status = NO_OS_GPIO_HIGH;

		ret = no_os_gpio_direction_input(desc->gpio0_desc);
		if (ret) {
			return ret;
		}

		/* Toggle CNV as a GPIO */
		ret = ad4692_toggle_cnv(cnv_gpio_desc);
		if (ret) {
			return ret;
		}

		/* Poll for BSY Low */
		do {
			ret = no_os_gpio_get_value(desc->gpio0_desc, &eoc_status);
			if (ret) {
				return ret;
			}
		} while ((eoc_status != NO_OS_GPIO_LOW) && (timeout-- > 0));

		if (timeout == 0) {
			return -ETIMEDOUT;
		}

		ret = no_os_spi_transfer(ad4692_dev->comm_desc, &ad4692_spi_msg_manual_mode, 1);
		if (ret) {
			return ret;
		}

		*adc_data = no_os_get_unaligned_be16(ad4692_spi_msg_manual_mode.rx_buff);

		break;

	case AD4692_CNV_CLOCK:
	case AD4692_CNV_BURST:
	case AD4692_SPI_BURST:

		/* Enable CNV PWM */
		ret = no_os_pwm_enable(desc->conv_desc);
		if (ret) {
			return ret;
		}

		if (ad4692_init_params.mode == AD4692_SPI_BURST) {
			/* Write to Convert Start register to trigger the next burst of conversion */
			ret = ad4692_reg_write(ad4692_dev,
					       AD4692_OSC_EN_REG,
					       AD4692_CONV_START_MASK);
			if (ret) {
				return ret;
			}
		}

		eoc_status = NO_OS_GPIO_HIGH;

		/* Poll for DATA_READYb Low */
		do {
			ret = no_os_gpio_get_value(desc->gpio0_desc, &eoc_status);
			if (ret) {
				return ret;
			}
		} while (eoc_status != NO_OS_GPIO_LOW && timeout-- > 0);

		if (timeout == 0) {
			return -ETIMEDOUT;
		}

		ret = ad4692_reg_read(ad4692_dev,
				      AD4692_AVG_IN_REG(chn),
				      adc_data);
		if (ret) {
			return ret;
		}
		*adc_data = no_os_get_unaligned_be16((uint8_t *)adc_data);

		/* Reset the state of accumulator to start a new burst of conversion */
		ret = ad4692_reg_write(ad4692_dev,
				       AD4692_STATE_RESET_REG,
				       AD4692_STATE_RESET_ALL);
		if (ret) {
			return ret;
		}

		/* Disable CNV PWM */
		ret = no_os_pwm_disable(desc->conv_desc);
		if (ret) {
			return ret;
		}

		break;

	default:

		return -EINVAL;
	}

	return 0;
}

/**
 * @brief Initialize device for data capture
 * @param desc[in, out] - AD4692 Device Descriptor
 * @return 0 in case of success, negative error code otherwise.
 */
int ad4692_start_data_capture(struct ad4692_desc *desc)
{
	int ret;
	uint8_t toggle_n;
	uint8_t ch_id;
	struct no_os_spi_msg ad4692_spi_msg = {
		.cs_change = CS_CHANGE,
		.tx_buff = data_buff,
		.rx_buff = data_buff,
		.bytes_number = BYTES_PER_SAMPLE
	};

	if (!desc) {
		return -EINVAL;
	}

	switch (ad4692_init_params.mode) {
	case AD4692_MANUAL_MODE:
		ret = init_gpio();
		if (ret) {
			return ret;
		}

		/* Configure GPIO0 as BUSY */
		ret = ad4692_gpio_set(desc,
				      AD4692_GPIO0,
				      AD4692_GPIO_OUTPUT_ADC_BUSY);
		if (ret) {
			return ret;
		}

		/* Enter Manual Mode */
		ret = ad4692_reg_write(desc,
				       AD4692_DEVICE_SETUP_REG,
				       AD4692_DEVICE_MANUAL);
		if (ret) {
			return ret;
		}

		if (!dma_capture) {
			/* Toggle CNV at a gap of 5us */
			for (toggle_n = 0; toggle_n < AD4692_N_CNV_TOGGLES; toggle_n++) {
				no_os_udelay(5);
				ret = ad4692_toggle_cnv(cnv_gpio_desc);
				if (ret) {
					return ret;
				}

				/* Register the channel ID */
				data_buff[0] = AD4692_IN_COMMAND(ad4692_active_channels[chan_id]);
				data_buff[1] = 0x0;

				/* Since manual mode operates with an offset of 2 CNV pulses,
				 * Iterate through the first two enabled channels if more
				 * than one channel is enabled. Else, register the only enabled channel- twice */
				if (num_of_active_channels > 1) {
					chan_id++;
				}

				ret = no_os_spi_transfer(desc->comm_desc, &ad4692_spi_msg, 1);
				if (ret) {
					return ret;
				}
			}
		}

		break;

	case AD4692_CNV_CLOCK:
		/* Reset manual mode bit */
		ad4692_reg_update(desc,
				  AD4692_DEVICE_SETUP_REG,
				  AD4692_MANUAL_MODE_MASK,
				  AD4692_EXIT_MANUAL_MODE);

		/* Enter CNV Clock Mode */
		ret = ad4692_reg_update(desc,
					AD4692_ADC_SETUP_REG,
					AD4692_MODE_MASK,
					AD4692_CNV_CLOCK);
		if (ret) {
			return ret;
		}

		/* Configure GPIO0 as DATA_READYb */
		ret = ad4692_reg_update(desc,
					AD4692_GPIO_MODE1_REG,
					AD4692_GPIO0_MASK,
					AD4692_GPIO_OUTPUT_DATA_READYb);

		if (ad4692_sequencer_mode == STANDARD_SEQUENCER) {
			/* Enable the desired channels */
			ret = ad4692_std_seq_ch(desc, channel_mask, true, 0);
			if (ret) {
				return ret;
			}

			/* Configure accumulator mask */
			ret = ad4692_configure_acc_mask(channel_mask, ad4692_sequencer_mode,
							channel_priorities);
			if (ret) {
				return ret;
			}
		}

		/* Configure the accumulator count limit */
		for (ch_id = 0; ch_id < num_of_active_channels; ch_id++) {
			ret = ad4692_reg_write(desc,
					       AD4692_ACC_COUNT_LIMIT_REG(ad4692_active_channels[ch_id]),
					       ad4692_acc_count[ad4692_active_channels[ch_id]]);
			if (ret) {
				return ret;
			}
		}

		break;

	case AD4692_CNV_BURST:
		/* Reset manual mode bit */
		ret = ad4692_reg_update(desc,
					AD4692_DEVICE_SETUP_REG,
					AD4692_MANUAL_MODE_MASK,
					AD4692_EXIT_MANUAL_MODE);
		if (ret) {
			return ret;
		}

		/* Reset the state of the accumulator */
		ret = ad4692_reg_write(desc,
				       AD4692_STATE_RESET_REG,
				       AD4692_STATE_RESET_ALL);
		if (ret) {
			return ret;
		}

		/* Set all channels to the configured accumulator count */
		for (ch_id = 0; ch_id < NO_OF_CHANNELS; ch_id++) {
			ret = ad4692_reg_write(ad4692_dev, AD4692_ACC_COUNT_LIMIT_REG(ch_id),
					       ad4692_acc_count[ch_id]);
			if (ret) {
				return ret;
			}
		}

		/* Configure GPIO0 as DATA_READYb */
		ret = ad4692_reg_update(desc,
					AD4692_GPIO_MODE1_REG,
					AD4692_GPIO0_MASK,
					AD4692_GPIO_OUTPUT_DATA_READYb);
		if (ret) {
			return ret;
		}

		/* Configure DATA_READYb as the stop state trigger */
		ret = ad4692_reg_update(desc,
					AD4692_ADC_SETUP_REG,
					AD4692_ADC_MODE_STOP_STATE_MASK,
					no_os_field_prep(AD4692_ADC_MODE_STOP_STATE_MASK,
							AD4692_STOP_STATE_DATA_READYb));
		if (ret) {
			return ret;
		}

		/* Enable the desired channels.*/
		if (ad4692_sequencer_mode == STANDARD_SEQUENCER) {
			ret = ad4692_std_seq_ch(desc, channel_mask, true, 0);
			if (ret) {
				return ret;
			}

			/* Configure accumulator mask */
			ret = ad4692_configure_acc_mask(channel_mask, ad4692_sequencer_mode,
							channel_priorities);
			if (ret) {
				return ret;
			}
		}

		/* Enter CNV Burst Mode */
		ret = ad4692_reg_update(desc,
					AD4692_ADC_SETUP_REG,
					AD4692_MODE_MASK,
					AD4692_CNV_BURST);
		if (ret) {
			return ret;
		}

		break;

	case AD4692_SPI_BURST:
		/* Reset manual mode bit */
		ret = ad4692_reg_update(desc,
					AD4692_DEVICE_SETUP_REG,
					AD4692_MANUAL_MODE_MASK,
					AD4692_EXIT_MANUAL_MODE);
		if (ret) {
			return ret;
		}

		/* Configure GPIO0 as DATA_READYb */
		ret = ad4692_reg_update(desc,
					AD4692_GPIO_MODE1_REG,
					AD4692_GPIO0_MASK,
					AD4692_GPIO_OUTPUT_DATA_READYb);
		if (ret) {
			return ret;
		}

		/* Configure DATA_READYb as the stop state trigger */
		ret = ad4692_reg_update(desc,
					AD4692_ADC_SETUP_REG,
					AD4692_ADC_MODE_STOP_STATE_MASK,
					no_os_field_prep(AD4692_ADC_MODE_STOP_STATE_MASK,
							AD4692_STOP_STATE_DATA_READYb));
		if (ret) {
			return ret;
		}

		/* Set all channels to the configured accumulator count */
		for (ch_id = 0; ch_id < NO_OF_CHANNELS; ch_id++) {
			ret = ad4692_reg_write(ad4692_dev, AD4692_ACC_COUNT_LIMIT_REG(ch_id),
					       ad4692_acc_count[ch_id]);
			if (ret) {
				return ret;
			}
		}

		if (ad4692_sequencer_mode == STANDARD_SEQUENCER) {
			ret = ad4692_std_seq_ch(desc, channel_mask, true, 0);
			if (ret) {
				return ret;
			}

			/* Configure accumulator mask */
			ret = ad4692_configure_acc_mask(channel_mask, ad4692_sequencer_mode,
							channel_priorities);
			if (ret) {
				return ret;
			}
		}

		/* Configure the oscillator frequency */
		ret = ad4692_set_osc(ad4692_dev, osc_freq_id);
		if (ret) {
			return ret;
		}

		/* Enter SPI Burst Mode */
		ret = ad4692_reg_update(desc,
					AD4692_ADC_SETUP_REG,
					AD4692_MODE_MASK,
					AD4692_SPI_BURST);
		if (ret) {
			return ret;
		}

		/* Write to Convert Start register */
		ret = ad4692_reg_write(desc,
				       AD4692_OSC_EN_REG,
				       AD4692_CONV_START_MASK);
		if (ret) {
			return ret;
		}

		break;

	default:
		return -EINVAL;
	}

	return 0;
}

/**
 * @brief Exit device from data capture
 * @param desc[in, out] - AD4692 Device Descriptor
 * @return 0 in case of success, negative error code otherwise.
 */
int ad4692_stop_data_capture(struct ad4692_desc *desc)
{
	int ret;

	if (!desc) {
		return -EINVAL;
	}

	switch (ad4692_init_params.mode) {
	case AD4692_MANUAL_MODE:
		/* Initialize CNV as a GPIO */
		ret = init_gpio();
		if (ret) {
			return ret;
		}

		ret = ad4692_exit_manual_mode(desc, cnv_gpio_desc);
		if (ret) {
			return ret;
		}

		ret = no_os_gpio_remove(cnv_gpio_desc);
		if (ret) {
			return ret;
		}
		cnv_gpio_desc = NULL;

		desc->mode = AD4692_CNV_CLOCK;

		break;

	case AD4692_CNV_CLOCK:
	case AD4692_CNV_BURST:
	case AD4692_SPI_BURST:
		break;

	default:
		return -EINVAL;
	}

	return 0;
}

/**
 * @brief Prepare for ADC data capture (transfer from device to memory)
 * @param dev_instance[in] - IIO device instance
 * @param chn_mask[in] - Channels select mask
 * @return 0 in case of success, negative error code otherwise
 */
static int32_t ad4692_iio_prepare_transfer(void* dev_instance, uint32_t ch_mask)
{
	int32_t ret;
	uint8_t ch_id;
	uint8_t index = 0;
	uint32_t mask = 0x1;
	num_of_active_channels = 0;
	chan_id = 0;
	memset(ad4692_active_channels, 0, NO_OF_CHANNELS);

	if (!dev_instance) {
		return -EINVAL;
	}

	if (ad4692_interface_mode == SPI_DMA) {
		dma_capture = true;
	}

	if (ad4692_sequencer_mode == STANDARD_SEQUENCER) {
		/* Updates the count of total number of active channels */
		for (ch_id = 0; ch_id < NO_OF_CHANNELS; ch_id++) {
			if (mask & ch_mask) {
				ad4692_active_channels[index++] = ch_id;
				num_of_active_channels++;
			}
			mask <<= 1;
		}
		channel_mask = ch_mask;
	} else {
		/* Updates the count of total number of active channels in advanced sequencer mode */
		for (ch_id = 0; ch_id < NO_OF_CHANNELS; ch_id++) {
			if (channel_priorities[ch_id] != 0) {
				ad4692_active_channels[index++] = ch_id;
				num_of_active_channels++;
				mask |= (1 << ch_id);
			}
		}
		channel_mask = mask;
	}

	if ((ad4692_data_capture_mode == CONTINUOUS)
	    || (ad4692_interface_mode == SPI_DMA)) {
		/* Start ADC Data capture */
		ret = ad4692_start_data_capture(ad4692_dev);
		if (ret) {
			return ret;
		}
	}
	if (ad4692_interface_mode == SPI_INTR) {
		ad4692_init_params.conv_param->period_ns = CONV_TRIGGER_PERIOD_NSEC(
					ad4692_sampling_frequency);
		ad4692_init_params.conv_param->duty_cycle_ns = CNV_ON_TIME;
		pwm_spi_burst_init.period_ns = CONV_TRIGGER_PERIOD_NSEC(
						       ad4692_sampling_frequency);

		ret = init_pwm();
		if (ret) {
			return ret;
		}

		if (ad4692_data_capture_mode == CONTINUOUS) {
#if	(ACTIVE_PLATFORM == STM32_PLATFORM)
			/* Clear any pending interrupts occured from a spurious falling edge of
			 * GPIO pin during toggling the CNV as a GPIO in the ad4692_start_data_capture()
			 * Note: EOC for SPI Burst is triggered in the ad4692_start_data_capture() when
			 * AD4692_CONV_START_REG is configured. Hence, no clearing of pending interrupt
			 * needed for SPI Burst */
			if (ad4692_init_params.mode != AD4692_SPI_BURST) {
				ret = no_os_irq_clear_pending(trigger_irq_desc, TRIGGER_INT_ID);
				if (ret) {
					return ret;
				}
			}
#endif // ACTIVE_PLATFORM
			ret = iio_trig_enable(ad4692_hw_trig_desc);
			if (ret) {
				return ret;
			}

			/* Configure CNV PWM and start */
			ret = ad4692_config_and_start_pwm(ad4692_dev);
			if (ret) {
				return ret;
			}
		}
	} else { // SPI DMA

		/* Pull the SPI CS line low to enable the data on SDO.*/
		ret = no_os_gpio_set_value(csb_gpio_desc, NO_OS_GPIO_LOW);
		if (ret) {
			return ret;
		}

		/* Initialize Tx Trigger Timer */
		ret = tx_trigger_init();
		if (ret) {
			return ret;
		}
	}

	return 0;
}

/**
 * @brief	Perform tasks before end of current data transfer
 * @param	dev[in] - IIO device instance
 * @return	0 in case of success, negative error code otherwise
 */
static int32_t ad4692_iio_end_transfer(void *dev)
{
	int32_t ret;

	if (!dev) {
		return -EINVAL;
	}

	if (ad4692_interface_mode == SPI_INTR) {
		if (ad4692_data_capture_mode == CONTINUOUS) {
			/* Stop timer */
			ad4692_stop_timer();

			/* Disable triggers */
			ret = iio_trig_disable(ad4692_hw_trig_desc);
			if (ret) {
				return ret;
			}
		}
	} else { // SPI_DMA
		/* Stop timers */
		ad4692_stop_timer();

		stm32_abort_dma_transfer();

		/* Pull the SPI CS line back high to enable reg Access */
		ret = no_os_gpio_set_value(csb_gpio_desc, NO_OS_GPIO_HIGH);
		if (ret) {
			return ret;
		}

		/* De initialize the Tx Trigger PWM */
		ret = no_os_pwm_disable(tx_trigger_desc);
		if (ret) {
			return ret;
		}

		dma_config_updated = false;
	}
	/* Stop ADC Data capture */
	ret = ad4692_stop_data_capture(ad4692_dev);
	if (ret) {
		return ret;
	}

	buf_size_updated = false;

	return 0;
}

/**
 * @brief Push data into IIO buffer when trigger handler IRQ is invoked
 * @param iio_dev_data[in] - IIO device data instance
 * @return 0 in case of success or negative value otherwise
 */
int32_t ad4692_trigger_handler(struct iio_device_data *iio_dev_data)
{
	int ret;
	uint32_t data_read = 0;
	uint8_t eoc_status;
	uint32_t timeout = BUF_READ_TIMEOUT;
	uint8_t id = 0;
	uint8_t bytes_offset = 2; // Offset to start reading the Rx word from SPI Data

	if (ad4692_init_params.mode == AD4692_MANUAL_MODE) {
		/* Reset the channel ID back to the first enabled channel in ascending order */
		if (chan_id >= num_of_active_channels) {
			chan_id = 0;
		}

		data_buff[0] = AD4692_IN_COMMAND(ad4692_active_channels[chan_id++]);
		data_buff[1] = 0x0;

		ret = no_os_spi_transfer(ad4692_dev->comm_desc, &ad4692_spi_msg_manual_mode, 1);
		if (ret) {
			return ret;
		}

		ret = no_os_cb_write(iio_dev_data->buffer->buf,
				     ad4692_spi_msg_manual_mode.rx_buff,
				     n_data_bytes);
		if (ret) {
			return ret;
		}
	} else {
		/* Populate the Tx command */
		ad4692_get_tx_command(acc_data_buff);

		if (ad4692_init_params.mode == AD4692_SPI_BURST) {
			/* Write to Convert Start register to trigger the next burst of conversion */
			ret = ad4692_reg_write(ad4692_dev,
					       AD4692_OSC_EN_REG,
					       AD4692_CONV_START_MASK);
			if (ret) {
				return ret;
			}
			/* Poll for BSY Low */
			do {
				ret = no_os_gpio_get_value(ad4692_dev->gpio0_desc, &eoc_status);
				if (ret) {
					return ret;
				}
			} while ((eoc_status != NO_OS_GPIO_LOW) && (timeout-- > 0));

			if (timeout == 0) {
				return -ETIMEDOUT;
			}
		}

		/* Pull CS low, read the data of all channels and Pull back CS High */
		ret = no_os_gpio_set_value(csb_gpio_desc, NO_OS_GPIO_LOW);
		if (ret) {
			return ret;
		}

		ret = no_os_spi_write_and_read(ad4692_dev->comm_desc, &acc_data_buff[0],
					       num_of_active_channels * n_bytes_per_transaction);
		if (ret) {
			return ret;
		}

		ret = no_os_gpio_set_value(csb_gpio_desc, NO_OS_GPIO_HIGH);
		if (ret) {
			return ret;
		}

		for (chan_id = 0; chan_id < num_of_active_channels; chan_id++) {
			id = (chan_id * n_bytes_per_transaction) + bytes_offset;
			if (ad4692_readback_option == ACCUMULATOR_DATA) {
				data_read = no_os_get_unaligned_be24(&acc_data_buff[id]);
			} else {
				data_read = no_os_get_unaligned_be16(&acc_data_buff[id]);
			}

			ret = no_os_cb_write(iio_dev_data->buffer->buf,
					     &data_read,
					     n_data_bytes);
			if (ret) {
				return ret;
			}
		}

		/* Reset the state of accumulator to start a new burst of conversion */
		ret = ad4692_reg_write(ad4692_dev,
				       AD4692_STATE_RESET_REG,
				       AD4692_STATE_RESET_ALL);
		if (ret) {
			return ret;
		}
	}

	return 0;
}

/*!
 * @brief Interrupt Service Routine to monitor end of conversion event.
 * @param ctx[in] - Callback context (unused)
 * @return none
 * @note Callback registered for the the DRDY interrupt to indicate
 * end of conversion in case of burst data capturing with SPI operation.
 */
void ad4692_data_capture_callback(void *ctx)
{
	ad4692_conversion_flag = true;
}

/**
 * @brief  Get the Tx buffer respective to the enabled channels
 * @param local_tx_data[out] Tx buffer
 * @return None
 */
void ad4692_get_tx_command(uint8_t* local_tx_data)
{
	uint8_t ch_id;
	uint8_t index;


	if (ad4692_init_params.mode == AD4692_MANUAL_MODE) {
		for (ch_id = 0; ch_id < num_of_active_channels; ch_id++) {
			index = ad4692_active_channels[ch_id];
			local_tx_data[ch_id * BYTES_PER_SAMPLE] = AD4692_IN_COMMAND(index);
			local_tx_data[(ch_id * BYTES_PER_SAMPLE) + 1] = 0;
		}
	} else {
		index = 0;
		for (ch_id = 0; ch_id < num_of_active_channels; ch_id++) {
			if (ad4692_readback_option == ACCUMULATOR_DATA) {
				acc_data_buff[index++] = AD4692_RW_ADDR_MASK | AD4692_MSB_MASK(
								 AD4692_ACC_IN_REG(
										 ad4692_active_channels[ch_id]));
				acc_data_buff[index++] = AD4692_LSB_MASK(AD4692_ACC_IN_REG(
								 ad4692_active_channels[ch_id]));
				acc_data_buff[index++] = 0x0;
				acc_data_buff[index++] = 0x0;
				acc_data_buff[index++] = 0x0;
			} else {
				acc_data_buff[index++] = AD4692_RW_ADDR_MASK | AD4692_MSB_MASK(
								 AD4692_AVG_IN_REG(
										 ad4692_active_channels[ch_id]));
				acc_data_buff[index++] = AD4692_LSB_MASK(AD4692_AVG_IN_REG(
								 ad4692_active_channels[ch_id]));
				acc_data_buff[index++] = 0x0;
				acc_data_buff[index++] = 0x0;
			}
		}
	}
}

/**
 * @brief Read data in burst mode via SPI DMA
 * @param nb_of_samples[in] - Number of samples requested by IIO
 * @param iio_dev_data[in] - IIO Device data instance
 * @return 0 in case of success or negative value otherwise
 */
static int32_t ad4692_read_data_spi_dma(uint32_t nb_of_samples,
					struct iio_device_data* iio_dev_data)
{
	int ret;
	uint32_t timeout = BUF_READ_TIMEOUT;
	uint32_t spirxdma_ndtr;
	static uint8_t local_tx_data[32] = { 0x0 };

	if (ad4692_data_capture_mode == BURST) {
		nb_of_samples = nb_of_samples * BYTES_PER_SAMPLE;

		ret = no_os_cb_prepare_async_write(iio_dev_data->buffer->buf,
						   nb_of_samples,
						   (void **) &buff_start_addr,
						   &data_read);
		if (ret) {
			return ret;
		}

		/* Manipulate the number of samples considering the 2-cycle offset in manual mode*/
		if (num_of_active_channels <= 2) {
			nb_of_samples += (N_CYCLE_OFFSET * BYTES_PER_SAMPLE);
		} else {
			nb_of_samples += (num_of_active_channels * BYTES_PER_SAMPLE);
		}

		if (!dma_config_updated) {
			ad4692_init_params.conv_param->period_ns = CONV_TRIGGER_PERIOD_NSEC(
						ad4692_sampling_frequency);
			ad4692_init_params.conv_param->duty_cycle_ns = CNV_ON_TIME;

			ret = init_pwm();
			if (ret) {
				return ret;
			}

			/* Build the Tx command with respect to the enabled channels */
			ad4692_get_tx_command(local_tx_data);

			/* Cap SPI RX DMA NDTR to MAX_DMA_NDTR. */
			spirxdma_ndtr = no_os_min(MAX_DMA_NDTR, nb_of_samples);
			rxdma_ndtr = spirxdma_ndtr;

			/* Register half complete callback, for ping-pong buffers implementation. */
			HAL_DMA_RegisterCallback(&hdma_spi1_rx,
						 HAL_DMA_XFER_HALFCPLT_CB_ID,
						 ad4692_spi_dma_rx_half_cplt_callback);

			struct no_os_spi_msg  ad4692_spi_msg = {
				.tx_buff = local_tx_data,
				.rx_buff = local_buf,
				.bytes_number = spirxdma_ndtr,
			};

			ret = no_os_spi_transfer_dma_async(ad4692_dev->comm_desc, &ad4692_spi_msg,
							   1, NULL, NULL);
			if (ret) {
				return ret;
			}

			/* DMA to be disabled while reconfiguring the NDTR register */
			DMA2_Stream2->CR &= ~1;
			DMA2_Stream2->NDTR = num_of_active_channels * 2;
			DMA2_Stream2->CR |= 1;

			dma_config_updated = true;
		}

		if (nb_of_samples == rxdma_ndtr) {
			dma_cycle_count = 1;
		} else {
			dma_cycle_count = ((nb_of_samples) / rxdma_ndtr) + 1;
		}
		callback_count = dma_cycle_count * 2;
		update_buff(local_buf, (uint8_t *)buff_start_addr);

		/* Configure CNV PWM and start */
		ret = ad4692_config_and_start_pwm(ad4692_dev);
		if (ret) {
			return ret;
		}

		while (ad4692_dma_buff_full != true && timeout > 0) {
			timeout--;
		}

		if (!timeout) {
			return -EIO;
		}

		ad4692_dma_buff_full = false;
		ret = no_os_cb_end_async_write(iio_dev_data->buffer->buf);
		if (ret) {
			return ret;
		}
		ad4692_stop_timer();
	} else { // CONTINUOUS_DATA_CAPTURE
		if (!dma_config_updated) {
			ad4692_init_params.conv_param->period_ns = CONV_TRIGGER_PERIOD_NSEC(
						ad4692_sampling_frequency);
			ad4692_init_params.conv_param->duty_cycle_ns = CNV_ON_TIME;

			ret = init_pwm();
			if (ret) {
				return ret;
			}

			/* Build the Tx command with respect to the enabled channels */
			ad4692_get_tx_command(local_tx_data);

			nb_of_samples = nb_of_samples * BYTES_PER_SAMPLE;

			/* Manipulate the number of samples considering the 2-cycle offset in manual mode*/
			if (num_of_active_channels <= 2) {
				nb_of_samples += (N_CYCLE_OFFSET * BYTES_PER_SAMPLE);
			} else {
				nb_of_samples += (num_of_active_channels * BYTES_PER_SAMPLE);
			}

			nb_of_samples_g = nb_of_samples;
			iio_dev_data_g = iio_dev_data;

			spirxdma_ndtr = no_os_min(MAX_DMA_NDTR, nb_of_samples);
			rxdma_ndtr = spirxdma_ndtr;

			/* SPI Message */
			struct no_os_spi_msg ad4692_spi_msg = {
				.tx_buff = local_tx_data,
				.rx_buff = local_buf,
				.bytes_number = spirxdma_ndtr
			};

			ret = no_os_cb_prepare_async_write(iio_dev_data_g->buffer->buf,
							   nb_of_samples,
							   (void **) &buff_start_addr,
							   &data_read);
			if (ret) {
				return ret;
			}

			ret = no_os_spi_transfer_dma_async(ad4692_dev->comm_desc, &ad4692_spi_msg, 1,
							   NULL, NULL);
			if (ret) {
				return ret;
			}

			/* DMA to be disabled while configuring the NDTR Register */
			DMA2_Stream2->CR &= ~1;
			DMA2_Stream2->NDTR = num_of_active_channels * BYTES_PER_SAMPLE;
			DMA2_Stream2->CR |= 1;

			dma_config_updated = true;

			if (nb_of_samples == rxdma_ndtr) {
				dma_cycle_count = 1;
			} else {
				dma_cycle_count = ((nb_of_samples) / rxdma_ndtr) + 1;
			}

			/* Update Buffer indices */
			update_buff(local_buf, (uint8_t *)buff_start_addr);

			/* Configure CNV PWM and start */
			ret = ad4692_config_and_start_pwm(ad4692_dev);
			if (ret) {
				return ret;
			}
		}
	}

	return 0;
}

/**
 * @brief Read buffer data corresponding to AD4692 IIO device.
 * @param [in, out] iio_dev_data - Device descriptor.
 * @return Number of samples read.
 */
static int32_t ad4692_iio_submit_samples(struct iio_device_data *iio_dev_data)
{
	int ret;
	uint32_t timeout = BUF_READ_TIMEOUT;
	uint32_t sample_index = 0;
	ad4692_conversion_flag = false;
	chan_id = 0;
	uint32_t data_read;
	uint8_t eoc_status;
	uint8_t id = 0;
	uint8_t bytes_offset = 2; // Offset to start reading the Rx word from SPI Data

	if (!iio_dev_data) {
		return -EINVAL;
	}

	nb_of_samples = iio_dev_data->buffer->size / n_data_bytes;

	if (!buf_size_updated) {
		/* Update total buffer size according to bytes per scan for proper
		 * alignment of multi-channel IIO buffer data */
		iio_dev_data->buffer->buf->size = iio_dev_data->buffer->size;
		buf_size_updated = true;
	}

	if (ad4692_interface_mode == SPI_INTR) {
		/* Start ADC data capture */
		ret = ad4692_start_data_capture(ad4692_dev);
		if (ret) {
			return ret;
		}

		/* Clear any pending interrupts occured from a spurious falling edge of
		* GPIO pin during toggling the CNV as a GPIO in the ad4692_start_data_capture()*/
		if (ad4692_init_params.mode != AD4692_SPI_BURST) {
			ret = no_os_irq_clear_pending(trigger_irq_desc, TRIGGER_INT_ID);
			if (ret) {
				return ret;
			}
		}

		ret = no_os_irq_enable(trigger_irq_desc, trigger_gpio_irq_params.irq_ctrl_id);
		if (ret) {
			return ret;
		}

		/* Config CNV PWM and start */
		ret = ad4692_config_and_start_pwm(ad4692_dev);
		if (ret) {
			return ret;
		}

		while (sample_index < nb_of_samples) {
			/* Build the Tx Command */
			ad4692_get_tx_command(acc_data_buff);

			if (ad4692_init_params.mode == AD4692_SPI_BURST) {
				/* Write to Convert Start register to trigger the next burst of conversion */
				ret = ad4692_reg_write(ad4692_dev,
						       AD4692_OSC_EN_REG,
						       AD4692_CONV_START_MASK);
				if (ret) {
					return ret;
				}
			}

			/* Check for status of conversion flag */
			while (!ad4692_conversion_flag && timeout > 0) {
				timeout--;
			}

			if (timeout == 0) {
				return -ETIMEDOUT;
			}
			timeout = BUF_READ_TIMEOUT;

			ad4692_conversion_flag = false;

			if (ad4692_init_params.mode == AD4692_MANUAL_MODE) {
				if (chan_id >= num_of_active_channels) {
					chan_id = 0;
				}

				data_buff[0] = AD4692_IN_COMMAND(ad4692_active_channels[chan_id++]);
				data_buff[1] = 0x0;

				ret = no_os_spi_transfer(ad4692_dev->comm_desc, &ad4692_spi_msg_manual_mode, 1);
				if (ret) {
					return ret;
				}

				ret = no_os_cb_write(iio_dev_data->buffer->buf,
						     ad4692_spi_msg_manual_mode.rx_buff,
						     n_data_bytes);
				if (ret) {
					return ret;
				}

			} else {
				if (ad4692_init_params.mode == AD4692_SPI_BURST) {
					/* Poll for BSY Low */
					do {
						ret = no_os_gpio_get_value(ad4692_dev->gpio0_desc, &eoc_status);
						if (ret) {
							return ret;
						}
					} while ((eoc_status != NO_OS_GPIO_LOW) && (timeout-- > 0));

					if (timeout == 0) {
						return -ETIMEDOUT;
					}
					timeout = BUF_READ_TIMEOUT;
				}

				/* Pull CS low, read the data of all channels and Pull back CS High */
				ret = no_os_gpio_set_value(csb_gpio_desc, NO_OS_GPIO_LOW);
				if (ret) {
					return ret;
				}

				ret = no_os_spi_write_and_read(ad4692_dev->comm_desc, acc_data_buff,
							       num_of_active_channels * n_bytes_per_transaction);
				if (ret) {
					return ret;
				}

				ret = no_os_gpio_set_value(csb_gpio_desc, NO_OS_GPIO_HIGH);
				if (ret) {
					return ret;
				}

				for (chan_id = 0; chan_id < num_of_active_channels; chan_id++) {
					id = (chan_id * n_bytes_per_transaction) + bytes_offset;
					if (ad4692_readback_option == ACCUMULATOR_DATA) {
						data_read = no_os_get_unaligned_be24(&acc_data_buff[id]);
					} else {
						data_read = no_os_get_unaligned_be16(&acc_data_buff[id]);
					}

					ret = no_os_cb_write(iio_dev_data->buffer->buf,
							     &data_read,
							     n_data_bytes);
					if (ret) {
						return ret;
					}
				}

				/* Reset the state of accumulator to start a new burst of conversion*/
				ret = ad4692_reg_write(ad4692_dev,
						       AD4692_STATE_RESET_REG,
						       AD4692_STATE_RESET_ALL);
				if (ret) {
					return ret;
				}
			}
			sample_index++;
		}

		/* Stop timer */
		ad4692_stop_timer();

		/* Disable triggers */
		ret = no_os_irq_disable(trigger_irq_desc, trigger_gpio_irq_params.irq_ctrl_id);
		if (ret) {
			return ret;
		}

		/* Stop ADC Data capture */
		ret = ad4692_stop_data_capture(ad4692_dev);
		if (ret) {
			return ret;
		}
	} else { // SPI_DMA
		ret = ad4692_read_data_spi_dma(nb_of_samples, iio_dev_data);
		if (ret) {
			return ret;
		}
	}

	return 0;
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
	int ret;

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
	int ret;

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
		return -EINVAL;
	}

	/* The acc_depth attribute is a global level attribute in case of Standard Sequencer
	 * and a channel level attribute in case of the Advanced sequencer */
	/* Update the channel map structure according to the sequencer configurations */
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
	ad4692_iio_inst->submit = ad4692_iio_submit_samples;
	ad4692_iio_inst->pre_enable = ad4692_iio_prepare_transfer;
	ad4692_iio_inst->post_disable = ad4692_iio_end_transfer;
	ad4692_iio_inst->debug_reg_read = ad4692_iio_debug_reg_read;
	ad4692_iio_inst->debug_reg_write = ad4692_iio_debug_reg_write;

	if (ad4692_data_capture_mode == CONTINUOUS) {
		ad4692_iio_inst->trigger_handler = ad4692_trigger_handler;
	}

	/* Configure the endianness in Channel scan structure */
	if (ad4692_init_params.mode == AD4692_MANUAL_MODE) {
		endianness = true;
		realbits = AD4692_RES_16;
		n_data_bytes = sizeof(uint16_t);
	} else {
		endianness = false;

		/* Update the scan structure according to the chosen readback option */
		if (ad4692_readback_option == AVERAGED_DATA) {
			realbits = AD4692_RES_16;
			n_data_bytes = sizeof(uint16_t);
			n_bytes_per_transaction = AD4692_N_BYTES_CNV_CLOCK_16BIT;
		} else {
			realbits = AD4692_RES_24;
			n_data_bytes = sizeof(uint32_t);
			n_bytes_per_transaction = AD4692_N_BYTES_CNV_CLOCK_24BIT;
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
static int ad4692_iio_trigger_param_init(struct iio_hw_trig **desc)
{
	int ret;
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

int32_t ad4692_configure_sampling_rate(void)
{
	dma_capture = false;

	switch (ad4692_init_params.mode) {
	case AD4692_MANUAL_MODE:
		if (ad4692_interface_mode == SPI_DMA) {
			dma_capture = true;
			ad4692_sampling_frequency_max = S_RATE_MANUAL_DMA;
		} else {
			ad4692_sampling_frequency_max = S_RATE_MANUAL_INTR;
		}

		break;

	case AD4692_CNV_CLOCK:
		if (ad4692_readback_option == AVERAGED_DATA) {
			ad4692_sampling_frequency_max = (ad4692_sequencer_mode == STANDARD_SEQUENCER) ?
							S_RATE_CNV_CLOCK_INTR_STD_AVG :
							S_RATE_CNV_CLOCK_INTR_ADV_AVG;
		} else {
			ad4692_sampling_frequency_max = (ad4692_sequencer_mode == STANDARD_SEQUENCER) ?
							S_RATE_CNV_CLOCK_INTR_STD_ACC :
							S_RATE_CNV_CLOCK_INTR_ADV_ACC;
		}

		break;

	case AD4692_CNV_BURST:
		if (ad4692_sequencer_mode == STANDARD_SEQUENCER) {
			if (ad4692_readback_option  == AVERAGED_DATA) {
				ad4692_sampling_frequency_max = S_RATE_CNV_BURST_STD_AVG;
			} else {
				ad4692_sampling_frequency_max = S_RATE_CNV_BURST_STD_ACC;
			}
		} else {
			if (ad4692_readback_option  == AVERAGED_DATA) {
				ad4692_sampling_frequency_max = S_RATE_CNV_BURST_ADV_AVG;
			} else {
				ad4692_sampling_frequency_max = S_RATE_CNV_BURST_ADV_ACC;
			}
		}

		break;

	case AD4692_SPI_BURST:
		if (ad4692_sequencer_mode == STANDARD_SEQUENCER) {
			if (ad4692_readback_option  == AVERAGED_DATA) {
				ad4692_sampling_frequency_max = S_RATE_SPI_BURST_STD_AVG;
			} else {
				ad4692_sampling_frequency_max = S_RATE_SPI_BURST_STD_ACC;
			}
		} else {
			if (ad4692_readback_option  == AVERAGED_DATA) {
				ad4692_sampling_frequency_max = S_RATE_SPI_BURST_ADV_AVG;
			} else {
				ad4692_sampling_frequency_max = S_RATE_SPI_BURST_ADV_ACC;
			}
		}

		break;

	default:
		return -EINVAL;
	}

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
 * @return  None
 */
int32_t iio_app_remove(void)
{
	/* Remove and free the pointers allocated during IIO init.
	 * Use NULL checks to determine which resources were allocated,
	 * since attribute setters may have changed mode globals before
	 * the restart flag was set. */

	/* Remove hardware trigger if allocated (SPI_INTR + CONTINUOUS) */
	if (ad4692_hw_trig_desc) {
		iio_hw_trig_remove(ad4692_hw_trig_desc);
		ad4692_hw_trig_desc = NULL;
	}

	/* Remove interrupt controller if allocated (SPI_INTR) */
	remove_interrupt();

	/* Remove SPI burst PWM if allocated (SPI_INTR + SPI_BURST) */
	remove_pwm();

	remove_gpio();

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
	int ret;

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
	    && (ad4692_data_capture_mode == CONTINUOUS)) {
		iio_init_params.trigs = &iio_trigger_init_params;
	}

	/* Configure max permissible data rate */
	ret = ad4692_configure_sampling_rate();
	if (ret) {
		return ret;
	}

	ad4692_sampling_frequency = ad4692_sampling_frequency_max;

	/* Configure to Standard Sequencer in manual Mode */
	if (ad4692_init_params.mode == AD4692_MANUAL_MODE) {
		ad4692_sequencer_mode = STANDARD_SEQUENCER;
	}

	ret = init_interrupt();
	if (ret) {
		return ret;
	}

	pwm_init_convst.period_ns = CONV_TRIGGER_PERIOD_NSEC(ad4692_sampling_frequency);
	pwm_spi_burst_init.period_ns = CONV_TRIGGER_PERIOD_NSEC(
					       ad4692_sampling_frequency);

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
		ret = ad4692_stop_data_capture(ad4692_dev);
		if (ret) {
			goto err_remove_ad4692;
		}

		/* Register and initialize the AD4692 device into IIO interface */
		ret = ad4692_iio_init(&ad4692_iio_dev[0], 0);
		if (ret) {
			goto err_remove_ad4692;
		}

		/* Initialize the IIO interface */
		iio_device_init_params[0].name = ACTIVE_DEVICE_NAME;
		iio_device_init_params[0].raw_buf = (int8_t *)adc_data_buffer;
		if (ad4692_data_capture_mode == CONTINUOUS) {
			iio_device_init_params[0].raw_buf_len = DATA_BUFFER_SIZE_CONT;
		} else {
			iio_device_init_params[0].raw_buf_len = DATA_BUFFER_SIZE;
		}
		iio_device_init_params[0].dev = ad4692_dev;
		iio_device_init_params[0].dev_descriptor = ad4692_iio_dev[0];

		iio_init_params.nb_devs++;

		if ((ad4692_interface_mode == SPI_INTR)
		    && (ad4692_data_capture_mode == CONTINUOUS)) {
			iio_device_init_params[0].trigger_id = "trigger0";
			iio_init_params.nb_trigs++;
		}

		break;

err_remove_ad4692:
		ad4692_remove(ad4692_dev);
		ad4692_dev = NULL;
		goto system_config_init;
	} while (false);

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
	    && (ad4692_data_capture_mode == CONTINUOUS)) {
		ret = ad4692_iio_trigger_param_init(&ad4692_hw_trig_desc);
		if (ret) {
			return ret;
		}
	}

	/* Initialize the PWM and channel priorities only if ADC initialization is proper */
	if (ad4692_dev) {
		ret = init_pwm();
		if (ret) {
			return ret;
		}
		ad4692_stop_timer();

		/* Configure the channel priorities in advanced sequencer mode */
		if (ad4692_sequencer_mode == ADVANCED_SEQUENCER) {
			ret = ad4692_configure_channel_priorities(channel_priorities,
					channel_sequence,
					&num_of_as_slots,
					ad4692_acc_count);
			if (ret) {
				return ret;
			}
		} else {
			memset(ad4692_acc_count, 0x0, NO_OF_CHANNELS);
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
