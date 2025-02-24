/***************************************************************************//**
 *   @file    ad4170_iio.c
 *   @brief   Implementation of AD4170 IIO application interfaces
 *   @details This module acts as an interface for AD4170 IIO application
********************************************************************************
 * Copyright (c) 2021-2025 Analog Devices, Inc.
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

#include "ad4170_iio.h"
#include "app_config.h"
#include "ad4170_regs.h"
#include "ad4170_support.h"
#include "ad4170_temperature_sensor.h"
#include "no_os_error.h"
#include "no_os_irq.h"
#include "no_os_delay.h"
#include "no_os_util.h"
#include "common.h"
#include "iio_trigger.h"
#include "no_os_gpio.h"

#if (INTERFACE_MODE == TDM_MODE)
#include "stm32_tdm_support.h"
#endif

#if (ACTIVE_IIO_CLIENT == IIO_CLIENT_LOCAL)
#include "pl_gui_views.h"
#include "pl_gui_events.h"
#include "adi_fft.h"
#endif

/******** Forward declaration of functions ********/
static int iio_ad4170_local_backend_event_read(void *conn, uint8_t *buf,
		uint32_t len);
static int iio_ad4170_local_backend_event_write(void *conn, uint8_t *buf,
		uint32_t len);

static float ad4170_data_to_voltage_without_vref(int32_t data, uint8_t chn);
static float ad4170_data_to_voltage_wrt_vref(int32_t data, uint8_t chn);
static int32_t ad4170_code_to_straight_binary(uint32_t code, uint8_t chn);

/******************************************************************************/
/************************ Macros/Constants ************************************/
/******************************************************************************/

/* Number of adc samples for loadcell calibration */
#define LOADCELL_SAMPLES_COUNT	10

/* CJC channel is 2 (common sensor for all Thermocouples).
 * Chn0 and Chn1 are used for multiple TC connections */
#define CJC_CHANNEL		2

/*	Number of IIO devices */
#define NUM_OF_IIO_DEVICES	1

/* IIO trigger name */
#define IIO_TRIGGER_NAME	"ad4170_iio_trigger"

/* Number of data storage bits (needed for IIO client to plot ADC data) */
#define CHN_REAL_BITS		(ADC_RESOLUTION)
#define CHN_STORAGE_BITS	(BYTES_PER_SAMPLE * 8)

#define	LED_TOGGLE_TIME			(500)		// 500msec
#define	LED_TOGGLE_TICK_CNTR	(LED_TOGGLE_TIME / (TICKER_INTERRUPT_PERIOD_uSEC / 1000))

#define		BYTE_SIZE		(uint32_t)8
#define		BYTE_MASK		(uint32_t)0xff

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
/* SDRAM configs for SDP-K1 */
#define adc_data_buffer				SDRAM_START_ADDRESS
#define DATA_BUFFER_SIZE			SDRAM_SIZE_BYTES
#else
#if (ACTIVE_IIO_CLIENT == IIO_CLIENT_LOCAL)
/* Note: Setting lower size due to memory constraints on MCU */
#define DATA_BUFFER_SIZE			(16384)
#else
#if (INTERFACE_MODE == TDM_MODE)
#define DATA_BUFFER_SIZE			(128000)
#else
#define DATA_BUFFER_SIZE			(131072)
#endif
#endif	// ACTIVE_IIO_CLIENT
static int8_t adc_data_buffer[DATA_BUFFER_SIZE] = { 0 };
#endif

/* Local backend buffer (for storing IIO commands and responses) */
#if (ACTIVE_IIO_CLIENT == IIO_CLIENT_LOCAL)
#define APP_LOCAL_BACKEND_BUF_SIZE	0x1000	// min 4096 bytes required
static char app_local_backend_buff[APP_LOCAL_BACKEND_BUF_SIZE];
#endif

/* Max number of cached registers */
#define N_REGISTERS_CACHED ADC_REGISTER_COUNT

/******************************************************************************/
/*************************** Types Declarations *******************************/
/******************************************************************************/

/* IIO interface descriptor */
static struct iio_desc *p_ad4170_iio_desc;

/**
 * Pointer to the struct representing the AD4170 IIO device
 */
struct ad4170_dev *p_ad4170_dev_inst = NULL;

/* IIO device descriptor */
struct iio_device *p_iio_ad4170_dev;

/* IIO hw trigger descriptor */
static struct iio_hw_trig *ad4170_hw_trig_desc;

#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
static struct iio_trigger ad4170_iio_trig_desc = {
	.is_synchronous = true,
};

/* IIO trigger init parameters */
static struct iio_trigger_init iio_trigger_init_params = {
	.descriptor = &ad4170_iio_trig_desc,
	.name = IIO_TRIGGER_NAME,
};
#endif

#if (ACTIVE_IIO_CLIENT == IIO_CLIENT_LOCAL)
/* IIO local backend init parameters */
static struct iio_local_backend local_backend_init_params = {
	.local_backend_event_read = iio_ad4170_local_backend_event_read,
	.local_backend_event_write = iio_ad4170_local_backend_event_write,
	.local_backend_buff = app_local_backend_buff,
	.local_backend_buff_len = APP_LOCAL_BACKEND_BUF_SIZE,
};
#endif

/* IIO interface init parameters */
static struct iio_init_param iio_init_params = {
#if (ACTIVE_IIO_CLIENT == IIO_CLIENT_REMOTE)
	.phy_type = USE_UART,
#else
	.phy_type = USE_LOCAL_BACKEND,
	.local_backend = &local_backend_init_params,
#endif
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

/* Number of active channels */
uint8_t num_of_active_channels;

/* List of channels to be captured */
static volatile uint8_t active_channels[AD4170_NUM_CHANNELS];

/* Polarity of channels */
static bool bipolar[AD4170_NUM_CHANNELS];

/* Previously active channels of device */
static uint32_t prev_active_channels = 0;

/* Flag to indicate if size of the buffer is updated according to requested
 * number of samples for the multi-channel IIO buffer data alignment */
static volatile bool buf_size_updated = false;

/* Device attributes with default values */

/* Scale attribute value per channel */
static float attr_scale_val[AD4170_NUM_CHANNELS];

/* IIOD channels scan parameters */
static struct scan_type chn_scan[AD4170_NUM_CHANNELS];

/* Diagnostic errors */
static const char *diagnostic_errors[] = {
	"ROM CRC Err", "Memory Map CRC Err", "SPI Err", "ADC Conv Err",
	"AINP OV/UV Err", "AINM OV/UV Err", "Ref OV/UV Err", "Ref Diff Min Err",
	"IOUT0 Compl Err", "IOUT1 Compl Err", "IOUT2 Compl Err", "IOUT3 Compl Err",
	"ALDO PSM Err", "DLDO PSM Err", "RES", "Device Init Err"
};

static const char *adc_modes[] = {
	"Continuous_Conversion",
	"Continuous_Conversion_FIR",
	"Continuous_Conversion_IIR",
	"RESERVED",
	"Single_Conversion",
	"Standby",
	"Power_Down",
	"Idle",
	"System_Offset_Calibration",
	"System_Gain_Calibration",
	"Self_Offset_Calibration",
};

/* Flag to trigger new data capture */
static bool adc_data_capture_started = false;

/* Diagnostic error status */
static uint16_t diag_err_status = 0;

/* To store the ADC register values during power-down */
static uint32_t adc_reg_data[ADC_REGISTER_COUNT];

/* Attribute IDs */
enum ad4170_attr_id {
	IIO_RAW_ATTR_ID,
	IIO_SCALE_ATTR_ID,
	IIO_OFFSET_ATTR_ID,
	INTERNAL_CALIB_ID,
	SYSTEM_CALIB_ID,
	LOADCELL_OFFSET_CALIB_ID,
	LOADCELL_GAIN_CALIB_ID,
	FILTER_ATTR_ID,
	REF_SELECT_ATTR_ID
};

/* Calibration state */
enum calibration_state {
	FULL_SCALE_CALIB_STATE,
	ZERO_SCALE_CALIB_STATE,
	CALIB_COMPLETE_STATE
};

/* ADC calibration configs */
typedef struct {
	uint32_t gain_before_calib;
	uint32_t gain_after_calib;
	uint32_t offset_after_calib;
	uint32_t offset_before_calib;
} adc_calibration_configs;

/* Calibration status */
enum calib_status {
	CALIB_NOT_DONE,
	CALIB_IN_PROGRESS,
	CALIB_DONE,
	CALIB_ERROR,
	CALIB_SKIPPED
};

/* Filter values */
static char *ad4170_filter_values[] = {
	"sinc5_avg",
	"",
	"",
	"",
	"sinc5",
	"",
	"sinc3",
};

/* Clock Ctrl Values */
static char *ad4170_clock_ctrl_values[] = {
	"internal_osc",
	"internal_osc_output",
	"external_osc",
	"external_xtal"
};

/* Reference select values */
static char *ad4170_ref_select_values[] = {
	"refin1p_refin1m",
	"refin2p_refin2m",
	"refout_avss",
	"avdd_avss"
};

static enum calibration_state system_calibration_state = ZERO_SCALE_CALIB_STATE;
static enum calibration_state internal_calibration_state =
	FULL_SCALE_CALIB_STATE;
static enum calib_status adc_calibration_status[AD4170_NUM_CHANNELS];
static adc_calibration_configs adc_calibration_config[AD4170_NUM_CHANNELS];

/* ADC raw averaged values from loadcell calibration */
static uint32_t adc_raw_offset;
static uint32_t adc_raw_gain;

/* Number of channels used in the application */
static uint8_t num_of_channels;

/* EVB HW validation status */
static bool hw_mezzanine_is_valid;

/* Global pointer to copy the private iio_device_data
 * structure from ad4170_trigger_handler() */
struct iio_device_data *ad4170_iio_dev_data;

/* Flag to indicate if ad4170_trigger_handler() has been
 * accessed once */
static volatile bool is_triggered;

/* Flag to indicate if continuous mode is set */
static bool data_capture_started;

/* Flag to check if no_os_tdm_read() has been called once
 * during data capture operation */
volatile bool tdm_read_started = false;

/* Number of samples to be ignored initially during TDM-DMA read */
static uint32_t num_samples_ignore = 0;

/* Flag to indicate if data read request is for raw read Operation
 * or data capture operation */
bool data_capture_operation = false;

#if (ACTIVE_IIO_CLIENT == IIO_CLIENT_LOCAL)
/* Pocket lab GUI views init parameters */
struct pl_gui_views pocket_lab_gui_views[] = {
	PL_GUI_ADD_ABOUT_DEF_VIEW,
	PL_GUI_ADD_ATTR_EDIT_DEF_VIEW,
	PL_GUI_ADD_REG_DEBUG_DEF_VIEW,
	PL_GUI_ADD_DMM_DEF_VIEW,
	PL_GUI_ADD_CAPTURE_DEF_VIEW,
	PL_GUI_ADD_ANALYSIS_DEF_VIEW,
	{ NULL }
};

/* FFT init parameters */
struct adi_fft_init_params fft_init_params = {
	.vref = AD4170_REFIN_REFOUT_VOLTAGE,
	.sample_rate = AD4170_DEFLT_SAMPLING_FREQUENCY,
	.samples_count = ADI_FFT_MAX_SAMPLES,
	.input_data_zero_scale = ADC_MAX_COUNT_BIPOLAR,
	.input_data_full_scale = ADC_MAX_COUNT_UNIPOLAR,
	.convert_data_to_volt_without_vref = &ad4170_data_to_voltage_without_vref,
	.convert_data_to_volt_wrt_vref = &ad4170_data_to_voltage_wrt_vref,
	.convert_code_to_straight_binary = &ad4170_code_to_straight_binary
};

/* Pocket lab GUI device init parameters */
struct pl_gui_device_param pl_gui_device_params = {
	.fft_params = &fft_init_params
};

/* Pocket lab GUI init parameters */
static struct pl_gui_init_param pocket_lab_gui_init_params = {
	.views = pocket_lab_gui_views,
	.device_params = &pl_gui_device_params
};

struct pl_gui_desc *pocket_lab_gui_desc;
#endif

#if (INTERFACE_MODE == SPI_DMA_MODE)
/* STM32 SPI Init params */
struct stm32_spi_init_param* spi_init_param;

/* Global Pointer for IIO Device Data */
volatile struct iio_device_data* iio_dev_data_g;

/* Global variable for number of samples */
uint32_t nb_of_samples_g;

/* Global variable for data read from CB functions */
int32_t data_read;

/* Flag to indicate if DMA has been configured for capture */
volatile bool dma_config_updated = false;

/* Flag for checking DMA buffer overflow */
volatile bool ad4170_dma_buff_full = false;

/* Variable to store start of buffer address */
volatile uint32_t* buff_start_addr;

/* Local buffer */
#define MAX_LOCAL_BUF_SIZE	8000
uint8_t local_buf[MAX_LOCAL_BUF_SIZE];

/* Maximum value the DMA NDTR register can take */
#define MAX_DMA_NDTR		(no_os_min(65535, MAX_LOCAL_BUF_SIZE))
#endif

/* Serial interface reset command */
static const uint8_t ad4170_serial_intf_reset[24] = {
	0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFE,
	0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFE,
	0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFE
};

/**
 * @struct ad4170_cached_reg
 * @brief Cached register
 */
struct ad4170_cached_reg {
	uint32_t addr;
	uint32_t value
};

/* Cached registers */
struct ad4170_cached_reg reg_values[N_REGISTERS_CACHED];

/* Register index */
uint8_t read_reg_id = 0;

/* Permissible HW Mezzanine names */
static const char *mezzanine_names[] = {
	"EVAL-AD4170-4ARDZ",
	"EVAL-AD4170-ASDZ",
	"EVAL-AD4190-4ARDZ"
};

/* Active device available options */
static const char* active_dev[] = {
	"ad4170",
	"ad4190",
};

/* Effective sampling rate of the device */
static uint32_t sampling_rate = AD4170_MAX_SAMPLING_RATE;

/******************************************************************************/
/************************ Functions Prototypes ********************************/
/******************************************************************************/

static void perform_sensor_measurement_and_update_scale(uint32_t adc_raw,
		uint16_t chn);
static void update_vltg_conv_scale_factor(uint8_t chn);
static int32_t ad4170_stop_data_capture(void);

/******************************************************************************/
/************************ Functions Definitions *******************************/
/******************************************************************************/

/*!
 * @brief	Getter/Setter for the demo config attribute value
 * @param	device- pointer to IIO device structure
 * @param	buf- pointer to buffer holding attribute value
 * @param	len- length of buffer string data
 * @param	channel- pointer to IIO channel structure
 * @param	id- Attribute ID (optional)
 * @return	Number of characters read/written
 */
static int get_demo_config(void *device,
			   char *buf,
			   uint32_t len,
			   const struct iio_ch_info *channel,
			   intptr_t id)
{
#if (ACTIVE_DEMO_MODE_CONFIG == RTD_2WIRE_CONFIG)
	return sprintf(buf, "%s", "2-Wire RTD");
#elif (ACTIVE_DEMO_MODE_CONFIG == RTD_3WIRE_CONFIG)
	return sprintf(buf, "%s", "3-Wire RTD");
#elif (ACTIVE_DEMO_MODE_CONFIG == RTD_4WIRE_CONFIG)
	return sprintf(buf, "%s", "4-Wire RTD");
#elif (ACTIVE_DEMO_MODE_CONFIG == THERMISTOR_CONFIG)
	return sprintf(buf, "%s", "Thermistor");
#elif (ACTIVE_DEMO_MODE_CONFIG == THERMOCOUPLE_CONFIG)
	return sprintf(buf, "%s", "Thermocouple");
#elif (ACTIVE_DEMO_MODE_CONFIG == ACCELEROMETER_CONFIG)
	return sprintf(buf, "%s", "Accelerometer");
#elif (ACTIVE_DEMO_MODE_CONFIG == LOADCELL_CONFIG)
	return sprintf(buf, "%s", "Loadcell");
#else
	return sprintf(buf, "%s", "User Default");
#endif
}

static int set_demo_config(void *device,
			   char *buf,
			   uint32_t len,
			   const struct iio_ch_info *channel,
			   intptr_t id)
{
	/* Demo mode config is selected only at compiler time */
	return -EINVAL;
}


/*!
 * @brief	Getter/Setter for the sampling frequency attribute value
 * @param	device- pointer to IIO device structure
 * @param	buf- pointer to buffer holding attribute value
 * @param	len- length of buffer string data
 * @param	channel- pointer to IIO channel structure
 * @param	id- Attribute ID (optional)
 * @return	Number of characters read/written
 * @Note	This attribute is used to define the timeout period in IIO
 *			client during data capture.
 *			Timeout (1 chn) = (requested samples * sampling frequency) + 1sec
 *			Timeout (n chns) = ((requested samples * n) / sampling frequency) + 1sec
 *			e.g. If sampling frequency = 31.5KSPS, requested samples = 4000, n=1min or 8max
 *			Timeout (1 chn) = (4000 / 315000) + 1 = ~1.13sec
 *			Timeout (8 chns) = ((4000 * 8) / 315000) + 1 = ~2.01sec
 */
static int get_sampling_frequency(void *device,
				  char *buf,
				  uint32_t len,
				  const struct iio_ch_info *channel,
				  intptr_t id)
{
	/* The application set sampling frequency is divided by number of channels.
	 * This is just used for IIO client timeout purpose and does not
	 * indicate an actual sampling rate of device.
	 * Refer the 'note' in function description above for timeout calculations */
	return sprintf(buf, "%d",
		       (sampling_rate / num_of_channels));
}

static int set_sampling_frequency(void *device,
				  char *buf,
				  uint32_t len,
				  const struct iio_ch_info *channel,
				  intptr_t id)
{
	/* NA - Sampling frequency is fixed in the firmware */
	return -EINVAL;
}


/*!
 * @brief	Getter/Setter for the raw, offset and scale attribute value
 * @param	device- pointer to IIO device structure
 * @param	buf- pointer to buffer holding attribute value
 * @param	len- length of buffer string data
 * @param	channel- pointer to IIO channel structure
 * @param	id- Attribute ID
 * @return	Number of characters read/written
 */
static int get_adc_raw(void *device,
		       char *buf,
		       uint32_t len,
		       const struct iio_ch_info *channel,
		       intptr_t id)
{
	int32_t ret;
	static uint32_t adc_data_raw = 0;
	int32_t offset = 0;
	uint8_t setup = p_ad4170_dev_inst->config.setup[channel->ch_num].setup_n;
	bool bipolar = p_ad4170_dev_inst->config.setups[setup].afe.bipolar;

	switch (id) {
	case IIO_RAW_ATTR_ID:
		/* Apply calibrated coefficients before new sampling */
		if (adc_calibration_status[channel->ch_num] == CALIB_DONE) {
			ret = ad4170_spi_reg_write(p_ad4170_dev_inst,
						   AD4170_REG_ADC_SETUPS_OFFSET(setup),
						   adc_calibration_config[channel->ch_num].offset_after_calib);
			if (ret) {
				break;
			}

			ret = ad4170_spi_reg_write(p_ad4170_dev_inst,
						   AD4170_REG_ADC_SETUPS_GAIN(setup),
						   adc_calibration_config[channel->ch_num].gain_after_calib);
			if (ret) {
				break;
			}
		}

		/* Capture the raw adc data */
		ret = ad4170_read_single_sample((uint8_t)channel->ch_num, &adc_data_raw);
		if (ret) {
			break;
		}

		perform_sensor_measurement_and_update_scale(adc_data_raw, channel->ch_num);
		return sprintf(buf, "%d", adc_data_raw);

	case IIO_SCALE_ATTR_ID:
		return snprintf(buf, len, "%.10f", attr_scale_val[channel->ch_num]);

	case IIO_OFFSET_ATTR_ID:
		if (bipolar) {
#if (ACTIVE_DEMO_MODE_CONFIG == USER_DEFAULT_CONFIG || \
	ACTIVE_DEMO_MODE_CONFIG == ACCELEROMETER_CONFIG || \
	ACTIVE_DEMO_MODE_CONFIG == LOADCELL_CONFIG)
			/* Note: For configurations other than these 3, the offset
			 * is ignored, as signed conversion needed for IIO client
			 * is done through perform_sensor_measurement_and_update_scale() */
			if (adc_data_raw >= ADC_MAX_COUNT_BIPOLAR) {
				offset = -ADC_MAX_COUNT_UNIPOLAR;
			} else {
				offset = 0;
			}
#endif
		}

		return sprintf(buf, "%d", offset);

	default:
		break;
	}

	return len;
}

static int set_adc_raw(void *device,
		       char *buf,
		       uint32_t len,
		       const struct iio_ch_info *channel,
		       intptr_t id)
{
	/* NA- Can't set raw value */
	return len;
}


/*!
 * @brief	Getter/Setter for the diagnostic error attribute value
 * @param	device- pointer to IIO device structure
 * @param	buf- pointer to buffer holding attribute value
 * @param	len- length of buffer string data
 * @param	channel- pointer to IIO channel structure
 * @param	id- Attribute ID (optional)
 * @return	Number of characters read/written
 */
static int get_diag_error(void *device,
			  char *buf,
			  uint32_t len,
			  const struct iio_ch_info *channel,
			  intptr_t id)
{
	int32_t ret;
	uint16_t mask = 0x1;
	uint8_t err_cnt;

	/* Get the ADC error status bit (whichever found first starting from LSB) */
	ret = ad4170_get_error(device, &diag_err_status);
	if (ret) {
		return len;
	}

	for (err_cnt = 0; err_cnt < NO_OS_ARRAY_SIZE(diagnostic_errors); err_cnt++) {
		if (diag_err_status & mask)
			break;
		mask <<= 1;
	}

	if (diag_err_status) {
		return sprintf(buf, "%s", diagnostic_errors[err_cnt]);
	} else {
		strcpy(buf, "No Error");
	}

	return len;
}

static int set_diag_error(void *device,
			  char *buf,
			  uint32_t len,
			  const struct iio_ch_info *channel,
			  intptr_t id)
{
	/* NA- Can't set error value */
	return len;
}


/*!
 * @brief	Getter/Setter for the ADC mode available values
 * @param	device- pointer to IIO device structure
 * @param	buf- pointer to buffer holding attribute value
 * @param	len- length of buffer string data
 * @param	channel- pointer to IIO channel structure
 * @param	id- Attribute ID (optional)
 * @return	Number of characters read/written
 */
static int get_adc_mode_available(void *device,
				  char *buf,
				  uint32_t len,
				  const struct iio_ch_info *channel,
				  intptr_t id)
{
	return sprintf(buf,
		       "%s",
		       "Continuous_Conversion Continuous_Conversion_FIR Continuous_Conversion_IIR Standby Power_Down Idle");
}

static int set_adc_mode_available(void *device,
				  char *buf,
				  uint32_t len,
				  const struct iio_ch_info *channel,
				  intptr_t id)
{
	/* NA- Can't set available mode value */
	return len;
}

/*!
 * @brief	Getter/Setter for the Filter mode available values
 * @param	device- pointer to IIO device structure
 * @param	buf- pointer to buffer holding attribute value
 * @param	len- length of buffer string data
 * @param	channel- pointer to IIO channel structure
 * @param	id- Attribute ID (optional)
 * @return	Number of characters read/written
 */
static int get_filter_available(void *device,
				char *buf,
				uint32_t len,
				const struct iio_ch_info *channel,
				intptr_t id)
{
	return sprintf(buf,
		       "%s %s %s",
		       ad4170_filter_values[0],
		       ad4170_filter_values[4],
		       ad4170_filter_values[6]);
}

static int set_filter_available(void *device,
				char *buf,
				uint32_t len,
				const struct iio_ch_info *channel,
				intptr_t id)
{
	/* NA- Can't set available mode value */
	return len;
}

/*!
 * @brief	Getter/Setter for the reference available values
 * @param	device- pointer to IIO device structure
 * @param	buf- pointer to buffer holding attribute value
 * @param	len- length of buffer string data
 * @param	channel- pointer to IIO channel structure
 * @param	id- Attribute ID (optional)
 * @return	Number of characters read/written
 */
static int get_reference_available(void *device,
				   char *buf,
				   uint32_t len,
				   const struct iio_ch_info *channel,
				   intptr_t id)
{
	return sprintf(buf,
		       "%s %s %s %s",
		       ad4170_ref_select_values[0],
		       ad4170_ref_select_values[1],
		       ad4170_ref_select_values[2],
		       ad4170_ref_select_values[3]);
}

static int set_reference_available(void *device,
				   char *buf,
				   uint32_t len,
				   const struct iio_ch_info *channel,
				   intptr_t id)
{
	/* NA- Can't set available mode value */
	return len;
}


/*!
 * @brief	Getter/Setter for the Clock source available values
 * @param	device- pointer to IIO device structure
 * @param	buf- pointer to buffer holding attribute value
 * @param	len- length of buffer string data
 * @param	channel- pointer to IIO channel structure
 * @param	id- Attribute ID (optional)
 * @return	Number of characters read/written
 */
static int get_clock_available(void *device,
			       char *buf,
			       uint32_t len,
			       const struct iio_ch_info *channel,
			       intptr_t id)
{
	return sprintf(buf,
		       "%s %s %s %s",
		       ad4170_clock_ctrl_values[0],
		       ad4170_clock_ctrl_values[1],
		       ad4170_clock_ctrl_values[2],
		       ad4170_clock_ctrl_values[3]);
}

static int set_clock_available(void *device,
			       char *buf,
			       uint32_t len,
			       const struct iio_ch_info *channel,
			       intptr_t id)
{
	/* NA- Can't set available mode value */
	return len;
}

/*!
 * @brief	Getter/Setter for the ADC mode attribute value
 * @param	device- pointer to IIO device structure
 * @param	buf- pointer to buffer holding attribute value
 * @param	len- length of buffer string data
 * @param	channel- pointer to IIO channel structure
 * @param	id- Attribute ID (optional)
 * @return	Number of characters read/written
 */
static int get_adc_mode(void *device,
			char *buf,
			uint32_t len,
			const struct iio_ch_info *channel,
			intptr_t id)
{
	int32_t ret;
	uint32_t reg_val;
	uint8_t adc_mode;
	struct ad4170_adc_ctrl adc_ctrl = p_ad4170_dev_inst->config.adc_ctrl;

	/* Do not read ADC mode when in power-down mode */
	if (adc_ctrl.mode != AD4170_MODE_POWER_DOWN) {
		ret = ad4170_spi_reg_read(device, AD4170_REG_ADC_CTRL, &reg_val);
		if (ret) {
			return len;
		}

		adc_mode = (reg_val & AD4170_REG_CTRL_MODE_MSK);
	} else {
		adc_mode = AD4170_MODE_POWER_DOWN;
	}

	return sprintf(buf, "%s", adc_modes[adc_mode]);
}

static int set_adc_mode(void *device,
			char *buf,
			uint32_t len,
			const struct iio_ch_info *channel,
			intptr_t id)
{
	bool found = false;
	uint32_t reg;
	uint8_t new_adc_mode_indx;
	uint8_t current_adc_mode;
	struct ad4170_adc_ctrl adc_ctrl = p_ad4170_dev_inst->config.adc_ctrl;

	/* Search for valid adc mode by comparing mode string passed through input
	 * buffer with the mode string stored into RAM */
	for (new_adc_mode_indx = 0; new_adc_mode_indx < NO_OS_ARRAY_SIZE(adc_modes);
	     new_adc_mode_indx++) {
		if (!strncmp(buf,
			     adc_modes[new_adc_mode_indx],
			     strlen(buf))) {
			found = true;
			break;
		}
	}

	/* Get the current ADC mode */
	current_adc_mode = adc_ctrl.mode;

	if (found) {
		/* Check for old and new control mode */
		if ((new_adc_mode_indx != AD4170_MODE_POWER_DOWN)
		    && (adc_ctrl.mode == AD4170_MODE_POWER_DOWN)) {
			/* Reset SPI interface to exit out from power-down mode */
			if (ad4170_reset_spi_interface(device)) {
				return -EINVAL;
			}

			/* Allow LDO to wake-up */
			no_os_mdelay(1000);

			/* Restore all the registers upon exit from power-down mode */
			for (reg = 0; reg < ADC_REGISTER_COUNT; reg++) {
				if (ad4170_spi_reg_write(device, ad4170_regs[reg],
							 adc_reg_data[reg])) {
					return -EINVAL;
				}
			}
		} else if ((new_adc_mode_indx == AD4170_MODE_POWER_DOWN)
			   && (current_adc_mode != AD4170_MODE_POWER_DOWN)) {
			/* Store all ADC registers before entering into power down mode */
			for (reg = 0; reg < ADC_REGISTER_COUNT; reg++) {
				if (ad4170_spi_reg_read(device, ad4170_regs[reg],
							&adc_reg_data[reg])) {
					return -EINVAL;
				}
			}

			/* Place ADC into standby mode first before entering into power-down mode */
			adc_ctrl.mode = AD4170_MODE_STANDBY;
			ad4170_set_adc_ctrl(device, adc_ctrl);
		} else {
			/* do nothing */
		}

		if ((new_adc_mode_indx != AD4170_MODE_CONT) && adc_data_capture_started) {
			adc_data_capture_started = false;
			ad4170_stop_data_capture();
		}

		/* Write new ADC mode */
		adc_ctrl.mode = new_adc_mode_indx;
		ad4170_set_adc_ctrl(device, adc_ctrl);
	}

	return len;
}

/*!
 * @brief	Getter/Setter for the Filter attribute value
 * @param	device- pointer to IIO device structure
 * @param	buf- pointer to buffer holding attribute value
 * @param	len- length of buffer string data
 * @param	channel- pointer to IIO channel structure
 * @param	id- Attribute ID (optional)
 * @return	Number of characters read/written
 */
static int get_filter(void *device,
		      char *buf,
		      uint32_t len,
		      const struct iio_ch_info *channel,
		      intptr_t id)
{
	enum ad4170_filter_type filter_selected;

	filter_selected =
		p_ad4170_dev_inst->config.setups[channel->ch_num].filter.filter_type;

	return sprintf(buf, "%s", ad4170_filter_values[filter_selected]);
}

static int set_filter(void *device,
		      char *buf,
		      uint32_t len,
		      const struct iio_ch_info *channel,
		      intptr_t id)
{
	uint8_t filter_id;
	int ret;

	for (filter_id = AD4170_FILT_SINC5_AVG; filter_id <= AD4170_FILT_SINC3;
	     filter_id++) {
		if (!strcmp(buf, ad4170_filter_values[filter_id])) {
			break;
		}
	}

	ret = ad4170_set_filter(p_ad4170_dev_inst, channel->ch_num, filter_id);
	if (ret) {
		return ret;
	}

	return 0;
}

/*!
 * @brief	Getter/Setter for the Reference attribute value
 * @param	device- pointer to IIO device structure
 * @param	buf- pointer to buffer holding attribute value
 * @param	len- length of buffer string data
 * @param	channel- pointer to IIO channel structure
 * @param	id- Attribute ID (optional)
 * @return	Number of characters read/written
 */
static int get_reference(void *device,
			 char *buf,
			 uint32_t len,
			 const struct iio_ch_info *channel,
			 intptr_t id)
{
	enum ad4170_ref_select reference_selected;

	reference_selected =
		p_ad4170_dev_inst->config.setups[p_ad4170_dev_inst->config.setup[channel->ch_num].setup_n].afe.ref_select;

	return sprintf(buf, "%s", ad4170_ref_select_values[reference_selected]);
}

static int set_reference(void *device,
			 char *buf,
			 uint32_t len,
			 const struct iio_ch_info *channel,
			 intptr_t id)
{
	uint8_t ref_id;
	int ret;

	for (ref_id = AD4170_REFIN_REFIN1; ref_id <= AD4170_REFIN_AVDD;
	     ref_id++) {
		if (!strcmp(buf, ad4170_ref_select_values[ref_id])) {
			break;
		}
	}

	ret = ad4170_set_reference(p_ad4170_dev_inst, channel->ch_num,
				   ref_id);
	if (ret) {
		return ret;
	}

	return 0;
}

/*!
 * @brief	Getter/Setter for the Clock Ctrl attribute value
 * @param	device- pointer to IIO device structure
 * @param	buf- pointer to buffer holding attribute value
 * @param	len- length of buffer string data
 * @param	channel- pointer to IIO channel structure
 * @param	id- Attribute ID (optional)
 * @return	Number of characters read/written
 */
static int get_clock(void *device,
		     char *buf,
		     uint32_t len,
		     const struct iio_ch_info *channel,
		     intptr_t id)
{
	enum ad4170_clocksel clock_ctrl;

	clock_ctrl = p_ad4170_dev_inst->config.clock_ctrl.clocksel;

	return sprintf(buf, "%s", ad4170_clock_ctrl_values[clock_ctrl]);
}

static int set_clock(void *device,
		     char *buf,
		     uint32_t len,
		     const struct iio_ch_info *channel,
		     intptr_t id)
{
	uint8_t clock_sel_id;
	int ret;

	for (clock_sel_id = AD4170_INTERNAL_OSC; clock_sel_id <= AD4170_EXTERNAL_XTAL;
	     clock_sel_id++) {
		if (!strcmp(buf, ad4170_clock_ctrl_values[clock_sel_id])) {
			break;
		}
	}

	ret = ad4170_set_clocksel(p_ad4170_dev_inst, clock_sel_id);
	if (ret) {
		return ret;
	}

	return 0;
}

/*!
 * @brief	Getter/Setter for the FS attribute value
 * @param	device- pointer to IIO device structure
 * @param	buf- pointer to buffer holding attribute value
 * @param	len- length of buffer string data
 * @param	channel- pointer to IIO channel structure
 * @param	id- Attribute ID (optional)
 * @return	Number of characters read/written
 */
static int get_fs(void *device,
		  char *buf,
		  uint32_t len,
		  const struct iio_ch_info *channel,
		  intptr_t id)
{
	uint16_t fs;

	fs = p_ad4170_dev_inst->config.setups[p_ad4170_dev_inst->config.setup[channel->ch_num].setup_n].filter_fs;

	return sprintf(buf, "%d", fs);
}

static int set_fs(void *device,
		  char *buf,
		  uint32_t len,
		  const struct iio_ch_info *channel,
		  intptr_t id)
{
	int ret;

	ret = ad4170_set_fs(p_ad4170_dev_inst,
			    p_ad4170_dev_inst->config.setup[channel->ch_num].setup_n, channel->ch_num,
			    no_os_str_to_uint32(buf));
	if (ret) {
		return ret;
	}

	return 0;
}

/*!
 * @brief	Perform the ADC internal/system calibration
 * @param	chn[in] - ADC channel
 * @param	calib_mode[in] - Calibration mode
 * @return	0 in case of success, negative error code otherwise
 */
int32_t perform_adc_calibration(uint8_t chn,
				enum ad4170_mode calib_mode)
{
	int32_t status;
	uint32_t data;
	struct ad4170_adc_ctrl adc_ctrl;
	uint8_t setup = p_ad4170_dev_inst->config.setup[chn].setup_n;
	uint8_t pga = p_ad4170_dev_inst->config.setups[setup].afe.pga_gain;

	/* Put ADC into standby mode */
	adc_ctrl = p_ad4170_dev_inst->config.adc_ctrl;
	adc_ctrl.mode = AD4170_MODE_STANDBY;
	status = ad4170_set_adc_ctrl(p_ad4170_dev_inst, adc_ctrl);
	if (status) {
		return status;
	}

	/* Read the gain/offset coefficient value (pre calibrated) */
	if ((calib_mode == AD4170_MODE_SELF_GAIN_CAL)
	    || (calib_mode == AD4170_MODE_SYS_GAIN_CAL)) {
		status = ad4170_spi_reg_read(p_ad4170_dev_inst,
					     AD4170_REG_ADC_SETUPS_GAIN(setup),
					     &data);
		if (status) {
			return status;
		}
		adc_calibration_config[chn].gain_before_calib = data;
	} else {
		status = ad4170_spi_reg_read(p_ad4170_dev_inst,
					     AD4170_REG_ADC_SETUPS_OFFSET(setup),
					     &data);
		if (status) {
			return status;
		}
		adc_calibration_config[chn].offset_before_calib = data;
	}

	/* Enable channel for calibration */
	status = ad4170_enable_input_chn(chn);
	if (status) {
		return status;
	}

	/* Apply excitation (for RTD sensor config) */
	status = ad4170_apply_excitation(chn);
	if (status) {
		return status;
	}

	if ((calib_mode == AD4170_MODE_SELF_GAIN_CAL)
	    || (calib_mode == AD4170_MODE_SYS_GAIN_CAL)) {
		if ((calib_mode == AD4170_MODE_SELF_GAIN_CAL)
		    && (pga == AD4170_PGA_GAIN_1 || pga == AD4170_PGA_GAIN_1_PRECHARGE)) {
			/* Internal gain calibration is not supported at gain of 1 */
			adc_calibration_config[chn].gain_after_calib =
				adc_calibration_config[chn].gain_before_calib;
			adc_calibration_status[chn] = CALIB_SKIPPED;
			return 0;
		}

		/* Perform internal/system gain (full-scale) calibration */
		adc_ctrl = p_ad4170_dev_inst->config.adc_ctrl;
		adc_ctrl.mode = calib_mode;
		status = ad4170_set_adc_ctrl(p_ad4170_dev_inst, adc_ctrl);
		if (status) {
			return status;
		}

		/* Wait for conversion to finish */
		no_os_mdelay(100);

		/* Read the gain coefficient value (post calibrated) */
		status = ad4170_spi_reg_read(p_ad4170_dev_inst,
					     AD4170_REG_ADC_SETUPS_GAIN(setup), &data);
		if (status) {
			return status;
		}
		adc_calibration_config[chn].gain_after_calib = data;

		/* Compare the pre and post adc calibration gain coefficients
		 * to check calibration status */
		if (adc_calibration_config[chn].gain_after_calib ==
		    adc_calibration_config[chn].gain_before_calib) {
			/* Error in gain calibration */
			return -EINVAL;
		}
	} else {
		/* Perform internal/system offset (zero-scale) calibration */
		adc_ctrl = p_ad4170_dev_inst->config.adc_ctrl;
		adc_ctrl.mode = calib_mode;
		status = ad4170_set_adc_ctrl(p_ad4170_dev_inst, adc_ctrl);
		if (status) {
			return status;
		}

		/* Wait for conversion to finish */
		no_os_mdelay(100);

		/* Read the coefficient value (post calibrated) */
		status = ad4170_spi_reg_read(p_ad4170_dev_inst,
					     AD4170_REG_ADC_SETUPS_OFFSET(setup), &data);
		if (status) {
			return status;
		}
		adc_calibration_config[chn].offset_after_calib = data;

		/* Compare the pre and post adc calibration offset coefficients to check calibration status */
		if (adc_calibration_config[chn].offset_after_calib ==
		    adc_calibration_config[chn].offset_before_calib) {
			/* Error in offset calibration */
			return -EINVAL;
		}
	}

	/* Remove excitation (for RTD sensor config) */
	status = ad4170_remove_excitation(chn);
	if (status) {
		return status;
	}

	/* Disable previously enabled channel */
	status = ad4170_disable_input_chn(chn);
	if (status) {
		return status;
	}

	return 0;
}

/*!
 * @brief	Getter/Setter for the ADC internal/system calibration
 * @param	device- pointer to IIO device structure
 * @param	buf- pointer to buffer holding attribute value
 * @param	len- length of buffer string data
 * @param	channel- pointer to IIO channel structure
 * @param	id- Attribute ID
 * @return	Number of characters read/written
 */
static int get_calibration_status(void *device,
				  char *buf,
				  uint32_t len,
				  const struct iio_ch_info *channel,
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
			if (adc_calibration_status[channel->ch_num] != CALIB_ERROR
			    && adc_calibration_status[channel->ch_num] != CALIB_SKIPPED
			    && adc_calibration_status[channel->ch_num] != CALIB_IN_PROGRESS) {
				/* Return NA to indicate that system calibration is not supported
				 * using IIO oscilloscope. Pyadi-iio script needs to be executed
				 * to perform a system calibration due to manual intervention
				 **/
				return snprintf(buf, len, "%s", "NA");
			}
		}

		sprintf(buf + buf_offset, "%08x",
			adc_calibration_config[channel->ch_num].gain_before_calib);
		buf_offset += 8;
		sprintf(buf + buf_offset, "%08x",
			adc_calibration_config[channel->ch_num].gain_after_calib);
		buf_offset += 8;
		sprintf(buf + buf_offset, "%08x",
			adc_calibration_config[channel->ch_num].offset_before_calib);
		buf_offset += 8;
		sprintf(buf + buf_offset, "%08x",
			adc_calibration_config[channel->ch_num].offset_after_calib);
		buf_offset += 8;

		if (adc_calibration_status[channel->ch_num] == CALIB_ERROR) {
			sprintf(buf + buf_offset, "%s", "calibration_failed");
			buf_offset += (strlen("calibration_failed") + 1);
			adc_calibration_status[channel->ch_num] = CALIB_NOT_DONE;
		} else if (adc_calibration_status[channel->ch_num] == CALIB_SKIPPED) {
			sprintf(buf + buf_offset, "%s", "calibration_skipped");
			buf_offset += (strlen("calibration_skipped") + 1);
			adc_calibration_status[channel->ch_num] = CALIB_NOT_DONE;
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

static int set_calibration_routine(void *device,
				   char *buf,
				   uint32_t len,
				   const struct iio_ch_info *channel,
				   intptr_t id)
{
	switch (id) {
	case INTERNAL_CALIB_ID:
		if (!strncmp(buf, "start_calibration", strlen(buf))) {
			switch (internal_calibration_state) {
			case FULL_SCALE_CALIB_STATE:
				adc_calibration_status[channel->ch_num] = CALIB_IN_PROGRESS;
				if (perform_adc_calibration(channel->ch_num,
							    AD4170_MODE_SELF_GAIN_CAL)) {
					adc_calibration_status[channel->ch_num] = CALIB_ERROR;
				}
				internal_calibration_state = ZERO_SCALE_CALIB_STATE;
				break;

			case ZERO_SCALE_CALIB_STATE:
				if (perform_adc_calibration(channel->ch_num,
							    AD4170_MODE_SELF_OFFSET_CAL)) {
					adc_calibration_status[channel->ch_num] = CALIB_ERROR;
					internal_calibration_state = FULL_SCALE_CALIB_STATE;
					break;
				}
				adc_calibration_status[channel->ch_num] = CALIB_DONE;
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
				adc_calibration_status[channel->ch_num] = CALIB_IN_PROGRESS;
				if (perform_adc_calibration(channel->ch_num,
							    AD4170_MODE_SYS_OFFSET_CAL)) {
					adc_calibration_status[channel->ch_num] = CALIB_ERROR;
				}
				system_calibration_state = FULL_SCALE_CALIB_STATE;
				break;

			case FULL_SCALE_CALIB_STATE:
				if (perform_adc_calibration(channel->ch_num,
							    AD4170_MODE_SYS_GAIN_CAL)) {
					adc_calibration_status[channel->ch_num] = CALIB_ERROR;
					system_calibration_state = ZERO_SCALE_CALIB_STATE;
					break;
				}
				adc_calibration_status[channel->ch_num] = CALIB_DONE;
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
 * @brief	Getter/Setter for the Loadcell offset/gain calibration
 * @param	device- pointer to IIO device structure
 * @param	buf- pointer to buffer holding attribute value
 * @param	len- length of buffer string data
 * @param	channel- pointer to IIO channel structure
 * @param	id- Attribute ID
 * @return	Number of characters read/written
 */
static int get_loadcell_calibration_status(void *device,
		char *buf,
		uint32_t len,
		const struct iio_ch_info *channel,
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

static int set_loadcell_calibration_status(void *device,
		char *buf,
		uint32_t len,
		const struct iio_ch_info *channel,
		intptr_t id)
{
	uint32_t adc_raw;
	uint8_t sample_cnt;
	uint64_t adc_raw_avg = 0;

	if (!strncmp(buf, "start_calibration", strlen(buf))) {
		switch (id) {
		case LOADCELL_OFFSET_CALIB_ID:
			for (sample_cnt = 0; sample_cnt < LOADCELL_SAMPLES_COUNT; sample_cnt++) {
				ad4170_read_single_sample(channel->ch_num, &adc_raw);
				adc_raw_avg += adc_raw;
			}

			adc_raw_avg /= LOADCELL_SAMPLES_COUNT;
			adc_raw_offset = (uint32_t)adc_raw_avg;
			break;

		case LOADCELL_GAIN_CALIB_ID:
			for (sample_cnt = 0; sample_cnt < LOADCELL_SAMPLES_COUNT; sample_cnt++) {
				ad4170_read_single_sample(channel->ch_num, &adc_raw);
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
 * @brief	Search the debug register address in look-up table Or registers array
 * @param	addr- Register address to search for
 * @param	reg_addr_offset - Offset of register address from its base address for
 *			multi-byte register entity
 * @return	Index to register address from look-up detect
 */
static uint32_t debug_reg_search(uint32_t addr, uint32_t *reg_addr_offset)
{
	uint32_t curr_indx; 	// Indexing to registers array (look-up table)
	uint32_t reg_base_add; 	// Base register address
	bool found = false;		// Address found status flag

	/* Search for valid input register address in registers array */
	for (curr_indx = 0; curr_indx < ADC_REGISTER_COUNT; curr_indx++) {
		if (addr == AD4170_ADDR(ad4170_regs[curr_indx])) {
			*reg_addr_offset = 0;
			found = true;
			break;
		} else if (addr < AD4170_ADDR(ad4170_regs[curr_indx])) {
			/* Get the input address offset from its base address for
			 * multi-byte register entity and break the loop indicating input
			 * address is located somewhere in the previous indexed register */
			if (AD4170_TRANSF_LEN(ad4170_regs[curr_indx - 1]) > 1) {
				*reg_addr_offset = addr - AD4170_ADDR(ad4170_regs[curr_indx - 1]);
				found = true;
			}
			break;
		}
	}

	/* Get the base address of register entity (single or multi byte) */
	if (found) {
		if (*reg_addr_offset > 0) {
			reg_base_add = ad4170_regs[curr_indx - 1];
		} else {
			reg_base_add = ad4170_regs[curr_indx];
		}
	} else {
		reg_base_add = addr | AD4170_R1B;
	}

	return reg_base_add;
}

/*!
 * @brief	Read the debug register value
 * @param	dev- Pointer to IIO device instance
 * @param	reg- Register address to read from
 * @param	readval- Pointer to variable to read data into
 * @return	0 in case of success or negative value otherwise
 */
int32_t debug_reg_read(void *dev, uint32_t reg, uint32_t *readval)
{
	int32_t ret;
	uint32_t reg_base_add; 		// Base register address
	uint32_t reg_addr_offset;	// Offset of input register address from its base

	if (!dev || !readval || (reg > MAX_REGISTER_ADDRESS)) {
		return -EINVAL;
	}

	reg_base_add = debug_reg_search(reg, &reg_addr_offset);

	if (p_ad4170_dev_inst->id == ID_AD4190) {
		if ((reg_base_add >= AD4170_REG_FIR_CONTROL)
		    && (reg_base_add <= AD4170_REG_DAC_INPUTB(0))) {
			return -EINVAL;
		}
	}

	/* Read data from device register */
	ret = ad4170_spi_reg_read(dev, reg_base_add, readval);
	if (ret) {
		return ret;
	}

	/* Extract the specific byte location for register entity */
	*readval = (*readval >> (reg_addr_offset * BYTE_SIZE)) & BYTE_MASK;

	return 0;
}

/*!
 * @brief	Write into the debug register
 * @param	dev- Pointer to IIO device instance
 * @param	reg- Register address to write into
 * @param	writeval- Register value to write
 * @return	0 in case of success or negative value otherwise
 */
int32_t debug_reg_write(void *dev, uint32_t reg, uint32_t writeval)
{
	int32_t ret;
	uint32_t reg_base_add; 		// Base register address
	uint32_t reg_addr_offset; 	// Offset of input register address from its base
	uint32_t data;				// Register data

	if (!dev || (reg > MAX_REGISTER_ADDRESS)) {
		return -EINVAL;
	}

	reg_base_add = debug_reg_search(reg, &reg_addr_offset);

	if (p_ad4170_dev_inst->id == ID_AD4190) {
		if ((reg_base_add >= AD4170_REG_FIR_CONTROL)
		    && (reg_base_add <= AD4170_REG_DAC_INPUTB(0))) {
			return -EINVAL;
		}
	}

	/* Read the register contents */
	ret = ad4170_spi_reg_read(dev, reg_base_add, &data);
	if (ret) {
		return ret;
	}

	/* Modify the register contents to write user data at specific
	 * reister entity location */
	data &= ~(BYTE_MASK << (reg_addr_offset * BYTE_SIZE));
	data |= (uint32_t)((writeval & BYTE_MASK) << (reg_addr_offset * BYTE_SIZE));

	/* Write data into device register */
	ret = ad4170_spi_reg_write(dev, reg_base_add, data);
	if (ret) {
		return ret;
	}

	return 0;
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
	temperature = get_ntc_thermistor_temperature(adc_raw, chn);
	attr_scale_val[chn] = (temperature / adc_raw) * 1000.0;
#elif ((ACTIVE_DEMO_MODE_CONFIG == RTD_2WIRE_CONFIG) || \
	(ACTIVE_DEMO_MODE_CONFIG == RTD_3WIRE_CONFIG) || (ACTIVE_DEMO_MODE_CONFIG == RTD_4WIRE_CONFIG))
	temperature = get_rtd_temperature(adc_raw, chn);
	attr_scale_val[chn] = (temperature / adc_raw) * 1000.0;
#elif (ACTIVE_DEMO_MODE_CONFIG == THERMOCOUPLE_CONFIG)
	if (chn != CJC_CHANNEL) {
		/* Sample the CJC channel (TC channel is already sampled through get_raw() function) */
		if (ad4170_read_single_sample(CJC_CHANNEL, (uint32_t *)&cjc_raw_data)) {
			return;
		}
	} else {
		/* For calculating CJC value, TC raw value does not matter  */
		chn = SENSOR_CHANNEL0;
		cjc_raw_data = adc_raw;
		adc_raw = 0;
	}

	/* Calculate the TC and CJC temperature and update scale factor */
	temperature = get_tc_temperature(adc_raw, cjc_raw_data,
					 chn, CJC_CHANNEL, &cjc_temp);
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
	float pga;
	float vref;
	uint8_t setup = p_ad4170_dev_inst->config.setup[chn].setup_n;
	bool bipolar = p_ad4170_dev_inst->config.setups[setup].afe.bipolar;

	pga = ad4170_get_gain_value(chn);
	vref = ad4170_get_reference_voltage(chn);

	/* Get the scale factor for voltage conversion */
	if (bipolar) {
		attr_scale_val[chn] = (vref / (ADC_MAX_COUNT_BIPOLAR * pga)) * 1000;
	} else {
		attr_scale_val[chn] = (vref / (ADC_MAX_COUNT_UNIPOLAR * pga)) * 1000;
	}
}

/**
 * @brief	Convert ADC data to voltage without Vref
 * @param	data[in] - ADC data in straight binary format (signed)
 * @param	chn[in] - ADC channel
 * @return	voltage
 */
static float ad4170_data_to_voltage_without_vref(int32_t data, uint8_t chn)
{
	return convert_adc_data_to_voltage_without_vref(data, chn);
}

/**
 * @brief	Convert ADC data to voltage with respect to Vref
 * @param	data[in] - ADC data in straight binary format (signed)
 * @param	chn[in] - ADC channel
 * @return	voltage
 */
static float ad4170_data_to_voltage_wrt_vref(int32_t data, uint8_t chn)
{
	return convert_adc_data_to_voltage_wrt_vref(data, chn);
}

/**
 * @brief	Convert ADC code to straight binary data
 * @param	code[in] - ADC code (unsigned)
 * @param	chn[in] - ADC channel
 * @return	ADC straight binary data (signed)
 */
static int32_t ad4170_code_to_straight_binary(uint32_t code, uint8_t chn)
{
	return perform_sign_conversion(code, chn);
}

/*!
 * @brief	Trigger a data capture in continuous/burst mode
 * @return	0 in case of success, negative error code otherwise
 */
static int32_t ad4170_start_data_capture(void)
{
	int32_t ret;
	uint8_t chn;
	struct ad4170_adc_ctrl adc_ctrl = p_ad4170_dev_inst->config.adc_ctrl;

	/* Disable ADC conversion */
	ret = ad4170_disable_conversion();
	if (ret) {
		return ret;
	}

	/* Apply excitation sources (demo config specific) */
	for (chn = 0; chn < num_of_active_channels; chn++) {
		ret = ad4170_apply_excitation(active_channels[chn]);
		if (ret) {
			return ret;
		}
	}

#if (INTERFACE_MODE == SPI_INTERRUPT_MODE)
#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
	/* Select continuous conversion mode */
	adc_ctrl.mode = AD4170_CONT_CONV_MODE_CONFIG;

	/* Enable continuous read mode for faster data read (this mode is
	 * only allowed in continuous conversion) */
	adc_ctrl.cont_read = AD4170_CONT_READ_ON;
	adc_ctrl.cont_read_status_en = false;
#else
	/* Select continuous conversion mode */
	adc_ctrl.mode = AD4170_CONT_CONV_MODE_CONFIG;
	adc_ctrl.cont_read = AD4170_CONT_READ_OFF;
#endif
	ret = ad4170_set_adc_ctrl(p_ad4170_dev_inst, adc_ctrl);
	if (ret) {
		return ret;
	}
#elif (INTERFACE_MODE == SPI_DMA_MODE)
	/* Select continuous conversion mode */
	adc_ctrl.mode = AD4170_CONT_CONV_MODE_CONFIG;

	/* Enable continuous read mode for faster data read */
	adc_ctrl.cont_read = AD4170_CONT_READ_ON;
	adc_ctrl.cont_read_status_en = false;

	ret = ad4170_set_adc_ctrl(p_ad4170_dev_inst, adc_ctrl);
	if (ret) {
		return ret;
	}
#else // TDM_MODE
	/* Set to continuous transmit mode*/
	adc_ctrl.mode =
		AD4170_CONT_CONV_MODE_CONFIG; /* ADC in continuous conversion mode */
	adc_ctrl.cont_read =
		AD4170_CONT_TRANSMIT_ON; /* Turn ON Continuous transmit Mode for TDM */
	adc_ctrl.cont_read_status_en = false;

	ret = ad4170_set_adc_ctrl(p_ad4170_dev_inst, adc_ctrl);
	if (ret) {
		return ret;
	}

	/* Pull the SPI CS line low to enable the data on SDO.
	 * This also ensures that the DIG_AUX1 and DIG_AUX2 signals are sent out
	 * via the pins on the Zio connector during continuous transmit mode */
	ret = no_os_gpio_set_value(csb_gpio_desc, NO_OS_GPIO_LOW);
	if (ret) {
		return ret;
	}
#endif // INTERFACE_MODE

	data_capture_started = true;

	return 0;
}

/*!
 * @brief	Stop a data capture from continuous/burst mode
 * @return	0 in case of success, negative error code otherwise
 */
static int32_t ad4170_stop_data_capture(void)
{
	int32_t ret;
	uint8_t chn;
	struct ad4170_adc_ctrl adc_ctrl = p_ad4170_dev_inst->config.adc_ctrl;
	adc_ctrl.cont_read = AD4170_CONT_READ_OFF;
	adc_ctrl.mode = AD4170_MODE_STANDBY;

#if (INTERFACE_MODE == TDM_MODE)
	ret = no_os_tdm_stop(ad4170_tdm_desc);
	if (ret) {
		return ret;
	}

	/* Pull the SPI CS line high to stop streaming data on SDO */
	ret = no_os_gpio_set_value(csb_gpio_desc, NO_OS_GPIO_HIGH);
	if (ret) {
		return ret;
	}

	/* Exit continuous transmit mode */
	ret = ad4170_continuous_transmit_exit(p_ad4170_dev_inst);
	if (ret) {
		return ret;
	}
#endif

#if (INTERFACE_MODE != SPI_DMA_MODE)
	/* Disable ADC conversion */
	ret = ad4170_disable_conversion();
	if (ret) {
		return ret;
	}
#else
	ret = no_os_spi_write_and_read(p_ad4170_dev_inst->spi_desc,
				       ad4170_serial_intf_reset, sizeof(ad4170_serial_intf_reset));
	if (ret) {
		return ret;
	}

	/* Serial interface reset delay */
	no_os_mdelay(5);

	p_ad4170_dev_inst->config.adc_ctrl.cont_read = AD4170_CONT_READ_OFF;
	ad4170_set_adc_ctrl(p_ad4170_dev_inst, adc_ctrl);

	/* Re-initialize the Device to apply the user config params */
	ret = ad4170_init(&p_ad4170_dev_inst, &ad4170_init_params);
	if (ret) {
		return ret;
	}

	/* Restore cached reg values */
	ret = ad4170_restore_cache();
	if (ret) {
		return ret;
	}
#endif
	/* Remove excitation sources (demo config specific) */
	for (chn = 0; chn < num_of_active_channels; chn++) {
		ret = ad4170_remove_excitation(active_channels[chn]);
		if (ret) {
			return ret;
		}
	}

	return 0;
}

/**
 * @brief Read data in burst mode via TDM-DMA
 * @param nb_of_bytes[in] - Number of samples requested by IIO
 * @param iio_dev_data[in] - IIO Device data instance
 * @return 0 in case of success or negative value otherwise
 */
static int32_t ad4170_read_burst_data_tdm(uint32_t nb_of_bytes,
		struct iio_device_data *iio_dev_data)
{
	uint32_t ad4170_buff_available_size;
	uint32_t timeout;
	uint32_t remaining_bytes = nb_of_bytes;
	int32_t ret;

	ret = ad4170_start_data_capture();
	if (ret) {
		return ret;
	}

#if (INTERFACE_MODE == TDM_MODE)
	do {
		if (remaining_bytes > DATA_BUFFER_SIZE) {
			nb_of_bytes = DATA_BUFFER_SIZE;
			remaining_bytes -= nb_of_bytes;
		} else {
			nb_of_bytes = remaining_bytes;
			remaining_bytes = 0;
		}

		ad4170_iio_dev_data = iio_dev_data;
		/* Retrieve the address of data buffer from where DMA data write needs to start */
		ret = no_os_cb_prepare_async_write(iio_dev_data->buffer->buf,
						   nb_of_bytes,
						   &dma_buff, &ad4170_buff_available_size);
		if (ret) {
			return ret;
		}

		/* Trigger TDM-DMA read to capture data into buffer in the background */
		ret = no_os_tdm_read(ad4170_tdm_desc, dma_buff, num_samples_ignore);
		if (ret) {
			return ret;
		}

		/* Wait until DMA buffer is full */
		timeout = BUF_READ_TIMEOUT;
		while ((!dma_buffer_full) && (timeout > 0))  {
			timeout--;
		}

		if (timeout == 0) {
			return -ETIMEDOUT;
		}

		tdm_read_started = false;
		/* Update the data buffer pointer to a new index post DMA write operation */
		ret = no_os_cb_end_async_write(iio_dev_data->buffer->buf);
		if (ret) {
			return ret;
		}

		ret = no_os_tdm_stop(ad4170_tdm_desc);
		if (ret) {
			return ret;
		}

		dma_buffer_full = false;
	} while (remaining_bytes > 0);
#endif

	ret = ad4170_stop_data_capture();
	if (ret) {
		return ret;
	}

	return 0;
}

/**
 * @brief Read data in burst mode via SPI
 * @param nb_of_samples[in] - Number of samples requested by IIO
 * @param iio_dev_data[in] - IIO Device data instance
 * @return 0 in case of success or negative value otherwise
 */
static int32_t ad4170_read_burst_data_spi(uint32_t nb_of_samples,
		struct iio_device_data *iio_dev_data)
{
	uint32_t sample_index = 0;
	uint32_t adc_raw;
	int32_t ret;

	ret = ad4170_start_data_capture();
	if (ret) {
		return ret;
	}

	while (sample_index < nb_of_samples) {
		/* This function monitors RDY line to read ADC result */
		ret = ad4170_read24(p_ad4170_dev_inst, &adc_raw, 1);
		if (ret) {
			return ret;
		}

		ret = no_os_cb_write(iio_dev_data->buffer->buf, &adc_raw, BYTES_PER_SAMPLE);
		if (ret) {
			return ret;
		}

		sample_index++;
	}

	ret = ad4170_stop_data_capture();
	if (ret) {
		return ret;
	}

	return 0;
}

/**
 * @brief Read data in burst mode via SPI DMA
 * @param nb_of_samples[in] - Number of samples requested by IIO
 * @param iio_dev_data[in] - IIO Device data instance
 * @return 0 in case of success or negative value otherwise
 */
static int32_t ad4170_read_burst_data_spi_dma(uint32_t nb_of_samples,
		struct iio_device_data* iio_dev_data)
{
	nb_of_samples *= BYTES_PER_SAMPLE;
	int ret;
	uint32_t local_tx_data = 0x000000;
	uint32_t timeout = BUF_READ_TIMEOUT;
	uint32_t spirxdma_ndtr;

#if (INTERFACE_MODE == SPI_DMA_MODE)
	ad4170_dma_buff_full = false;
	/* STM32 SPI Descriptor */
	struct stm32_spi_desc* sdesc = p_ad4170_dev_inst->spi_desc->extra;

	nb_of_samples_g = nb_of_samples;
	iio_dev_data_g = iio_dev_data;

#if (DATA_CAPTURE_MODE == BURST_DATA_CAPTURE)
	ret = no_os_cb_prepare_async_write(iio_dev_data->buffer->buf,
					   nb_of_samples, &buff_start_addr, &data_read);
	if (ret) {
		return ret;
	}

	if (!dma_config_updated) {
		/* Set SYNC Low */
		ret = no_os_gpio_set_value(p_ad4170_dev_inst->gpio_sync_inb, NO_OS_GPIO_LOW);
		if (ret) {
			return ret;
		}

		/* Cap SPI RX DMA NDTR to MAX_DMA_NDTR. */
		spirxdma_ndtr = no_os_min(MAX_DMA_NDTR, nb_of_samples);
		rxdma_ndtr = spirxdma_ndtr;

		/* Register half complete callback, for ping-pong buffers implementation. */
		HAL_DMA_RegisterCallback(&hdma_spi1_rx,
					 HAL_DMA_XFER_HALFCPLT_CB_ID,
					 ad4170_spi_dma_rx_half_cplt_callback);

		struct no_os_spi_msg  ad4170_spi_msg = {
			.tx_buff = (uint32_t*)local_tx_data,
			.rx_buff = (uint32_t*)local_buf,
			.bytes_number = spirxdma_ndtr
		};

		ret = no_os_spi_transfer_dma_async(p_ad4170_dev_inst->spi_desc, &ad4170_spi_msg,
						   1, NULL, NULL);
		if (ret) {
			return ret;
		}
		dma_config_updated = true;

		/* Configure Tx Trigger timer parameters */
		tim8_config();
	}

	dma_cycle_count = ((nb_of_samples) / rxdma_ndtr) + 1;
	update_buff(local_buf, buff_start_addr);

	TIM8->CNT = 0;

	/* Set CS Low to stream data continuously on SDO */
	ret = no_os_gpio_set_value(csb_gpio_desc, NO_OS_GPIO_LOW);
	if (ret) {
		return ret;
	}

	/* Set SYNC High to Initiate conversion */
	ret = no_os_gpio_set_value(p_ad4170_dev_inst->gpio_sync_inb, NO_OS_GPIO_HIGH);
	if (ret) {
		return ret;
	}

	while (ad4170_dma_buff_full != true && timeout > 0) {
		timeout--;
	}

	if (!timeout) {
		return -EIO;
	}

	ret = no_os_cb_end_async_write(iio_dev_data->buffer->buf);
	if (ret) {
		return ret;
	}

	/* Set CS back high to enable Reg access mode */
	ret = no_os_gpio_set_value(csb_gpio_desc, NO_OS_GPIO_HIGH);
	if (ret) {
		return ret;
	}
#else
	if (!dma_config_updated) {
		/* SPI Message */
		struct no_os_spi_msg ad4170_spi_msg = {
			.tx_buff = (uint32_t*)local_tx_data,
			.bytes_number = nb_of_samples * (BYTES_PER_SAMPLE)
		};

		/* Set SYNC Low */
		ret = no_os_gpio_set_value(p_ad4170_dev_inst->gpio_sync_inb, NO_OS_GPIO_LOW);
		if (ret) {
			return ret;
		}

		ret = no_os_cb_prepare_async_write(iio_dev_data_g->buffer->buf,
						   nb_of_samples * (BYTES_PER_SAMPLE), &buff_start_addr, &data_read);
		if (ret) {
			return ret;
		}
		ad4170_spi_msg.rx_buff = (uint32_t*)buff_start_addr;

		ret = no_os_spi_transfer_dma_async(p_ad4170_dev_inst->spi_desc, &ad4170_spi_msg,
						   1, NULL, NULL);
		if (ret) {
			return ret;
		}

		dma_config_updated = true;

		/* Configure Tx trigger timer parameters */
		tim8_config();

		TIM8->CNT = 0;

		/* Set CS Low to stream data continuously on SDO */
		ret = no_os_gpio_set_value(csb_gpio_desc, NO_OS_GPIO_LOW);
		if (ret) {
			return ret;
		}

		/* Set SYNC High to Initiate conversion */
		ret = no_os_gpio_set_value(p_ad4170_dev_inst->gpio_sync_inb, NO_OS_GPIO_HIGH);
		if (ret) {
			return ret;
		}
	}
#endif // DATA_CAPTURE_MODE
#endif // INTERFACE_MODE

	return 0;
}

/**
 * @brief	Read buffer data corresponding to AD4170 ADC IIO device
 * @param	iio_dev_data[in] - IIO device data instance
 * @return 0 in case of success or negative value otherwise
 */
static int32_t iio_ad4170_submit_buffer(struct iio_device_data *iio_dev_data)
{
	uint32_t ret;
	uint32_t nb_of_samples;
	nb_of_samples = iio_dev_data->buffer->size / BYTES_PER_SAMPLE;

#if (INTERFACE_MODE  != TDM_MODE)
	if (!buf_size_updated) {
		/* Update total buffer size according to requested samples
		 * IIO from  for proper alignment of multi-channel IIO buffer data */
		iio_dev_data->buffer->buf->size = iio_dev_data->buffer->size;
		buf_size_updated = true;
	}
#endif

#if (DATA_CAPTURE_MODE == BURST_DATA_CAPTURE)
#if (INTERFACE_MODE == SPI_INTERRUPT_MODE)
	ret = ad4170_read_burst_data_spi(nb_of_samples, iio_dev_data);
	if (ret) {
		return ret;
	}
#elif (INTERFACE_MODE == TDM_MODE)
	ret = ad4170_read_burst_data_tdm(iio_dev_data->buffer->size, iio_dev_data);
	if (ret) {
		return ret;
	}
#elif (INTERFACE_MODE == SPI_DMA_MODE)
	ret = ad4170_read_burst_data_spi_dma(nb_of_samples, iio_dev_data);
	if (ret) {
		return ret;
	}
#endif
#else // CONTINUOUS_DATA_CAPTURE
#if(INTERFACE_MODE == SPI_DMA_MODE)
	ret = ad4170_read_burst_data_spi_dma(nb_of_samples, iio_dev_data);
	if (ret) {
		return ret;
	}
#endif
#endif

	return 0;
}

/**
 * @brief Cache register values modified by attributes
 * @return 0 in case of success, negative error code otherwise
 */
int ad4170_cache_register_values(void)
{
	int ret;
	uint8_t chn_setup_index = 0;
	uint32_t debug_addr = AD4170_REG_ADC_CTRL;
	uint32_t debug_val = 0;

	read_reg_id = 0;

	/* Cache ADC control register */
	reg_values[read_reg_id].addr = AD4170_REG_ADC_CTRL;
	ret = ad4170_spi_reg_read(p_ad4170_dev_inst, reg_values[read_reg_id].addr,
				  &reg_values[read_reg_id].value);
	if (ret) {
		return ret;
	}

	read_reg_id++;

	/* Cache setup register */
	for (chn_setup_index = 0; chn_setup_index < AD4170_NUM_SETUPS;
	     chn_setup_index++) {
		reg_values[read_reg_id].addr = AD4170_REG_ADC_CHANNEL_SETUP(chn_setup_index);
		ret = ad4170_spi_reg_read(p_ad4170_dev_inst,
					  reg_values[read_reg_id].addr, &reg_values[read_reg_id].value);
		if (ret) {
			return ret;
		}
		read_reg_id++;
	}

	/* Cache AFE register */
	for (chn_setup_index = 0; chn_setup_index < AD4170_NUM_SETUPS;
	     chn_setup_index++) {
		reg_values[read_reg_id].addr = AD4170_REG_ADC_SETUPS_AFE(chn_setup_index);
		ret = ad4170_spi_reg_read(p_ad4170_dev_inst,
					  reg_values[read_reg_id].addr, &reg_values[read_reg_id].value);
		if (ret) {
			return ret;
		}
		read_reg_id++;
	}

	/* Cache clock control register */
	reg_values[read_reg_id].addr = AD4170_REG_CLOCK_CTRL;
	ret = ad4170_spi_reg_read(p_ad4170_dev_inst,
				  reg_values[read_reg_id].addr, &reg_values[read_reg_id].value);
	if (ret) {
		return ret;
	}
	read_reg_id++;

	/* Cache Filter Fs register */
	for (chn_setup_index = 0; chn_setup_index < AD4170_NUM_SETUPS;
	     chn_setup_index++) {
		reg_values[read_reg_id].addr = AD4170_REG_ADC_SETUPS_FILTER_FS(chn_setup_index);
		ret = ad4170_spi_reg_read(p_ad4170_dev_inst,
					  reg_values[read_reg_id].addr, &reg_values[read_reg_id].value);
		if (ret) {
			return ret;
		}
		read_reg_id++;
	}

	return 0;
}

/**
 * @brief Restore cached register values
 * @return 0 in case of success, negative error code otherwise
 */
int ad4170_restore_cache(void)
{
	int ret;
	uint8_t write_reg_id;

	for (write_reg_id = 0; write_reg_id < read_reg_id; write_reg_id++) {
		ret = ad4170_spi_reg_write(p_ad4170_dev_inst,
					   reg_values[write_reg_id].addr, reg_values[write_reg_id].value);
		if (ret) {
			return ret;
		}
	}

	return 0;
}

/**
 * @brief	Prepare for ADC data capture (transfer from device to memory)
 * @param	dev_instance[in] - IIO device instance
 * @param	chn_mask[in] - Channels select mask
 * @return	0 in case of success, negative error code otherwise
 */
static int32_t iio_ad4170_prepare_transfer(void *dev_instance,
		uint32_t chn_mask)
{
	int32_t ret;
	uint8_t mask = 0x1;
	uint8_t chn;
	uint8_t setup;
	uint8_t index = 0;
	uint32_t timeout = BUF_READ_TIMEOUT;

	num_of_active_channels = 0;
	buf_size_updated = false;
	data_capture_operation = true;

	/* Store the previous active channels */
	prev_active_channels = p_ad4170_dev_inst->config.channel_en;

	/* Cache register values */
	ret = ad4170_cache_register_values();
	if (ret) {
		return ret;
	}

	/* Enable/Disable channels based on channel mask set in the IIO client */
	for (chn = 0; chn < AD4170_NUM_CHANNELS; chn++) {
		if (chn_mask & mask) {
			num_of_active_channels++;
			active_channels[index++] = chn;

			setup = p_ad4170_dev_inst->config.setup[chn].setup_n;
			bipolar[chn] = p_ad4170_dev_inst->config.setups[setup].afe.bipolar;

			/* Enable the selected channel */
			ret = ad4170_enable_input_chn(chn);
			if (ret) {
				return ret;
			}
		} else {
			/* Disable the selected channel */
			ret = ad4170_disable_input_chn(chn);
			if (ret) {
				return ret;
			}
		}

		mask <<= 1;
	}

	adc_data_capture_started = true;

	/* Ignore num_of_active_channels-1 samples if more than
	 * one channel is enabled */
	if (num_of_active_channels > 2) {
		num_samples_ignore = num_of_active_channels - 2;
	} else {
		num_samples_ignore = 2;
	}

#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE) && (INTERFACE_MODE != SPI_DMA_MODE)
	ret = iio_trig_enable(ad4170_hw_trig_desc);
	if (ret) {
		return ret;
	}

	/* Trigger continuous data capture */
	ret = ad4170_start_data_capture();
	if (ret) {
		return ret;
	}

#if (INTERFACE_MODE == TDM_MODE)
	/* Ensure that the start_tdm_dma_to_cb_transfer() is executed
	 * only after the ad4170_iio_dev_data is populated */
	while ((!is_triggered) && (timeout > 0)) {
		timeout--;
	}

	if (timeout == 0) {
		return -ETIMEDOUT;
	}

	ret = start_tdm_dma_to_cb_transfer(ad4170_tdm_desc, ad4170_iio_dev_data,
					   TDM_DMA_READ_SIZE, BYTES_PER_SAMPLE, num_samples_ignore);
	if (ret) {
		return ret;
	}
#endif // INTERFACE_MODE
#endif // DATA_CAPTURE_MODE

#if (INTERFACE_MODE == SPI_DMA_MODE)
	ret = ad4170_start_data_capture();
	if (ret) {
		return ret;
	}

	spi_init_param = ad4170_init_params.spi_init.extra;
	spi_init_param->dma_init = &ad4170_dma_init_param;

	spi_init_param->irq_num = Rx_DMA_IRQ_ID;
	spi_init_param->rxdma_ch = &rxdma_channel;
	spi_init_param->txdma_ch = &txdma_channel;

	/* Init SPI interface in DMA Mode */
	ret = no_os_spi_init(&p_ad4170_dev_inst->spi_desc,
			     &ad4170_init_params.spi_init);
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
static int32_t iio_ad4170_end_transfer(void *dev)
{
	int32_t ret;

	adc_data_capture_started = false;
	is_triggered = false;
	data_capture_started = false;
	tdm_read_started = false;
	data_capture_operation = false;

#if (INTERFACE_MODE != SPI_DMA_MODE)
#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
	ret = iio_trig_disable(ad4170_hw_trig_desc);
	if (ret) {
		return ret;
	}

	/* Stop data capture */
	ret = ad4170_stop_data_capture();
	if (ret) {
		return ret;
	}

	/* Restore (re-enable) the previous active channels */
	ret = ad4170_set_channel_en(p_ad4170_dev_inst, prev_active_channels);
	if (ret) {
		return ret;
	}
#endif
#else // SPI_DMA
	stm32_timer_stop();

	stm32_abort_dma_transfer();

	spi_init_param = ad4170_init_params.spi_init.extra;
	spi_init_param->dma_init = NULL;

	/* Init SPI Interface in normal mode (Non DMA) */
	ret = no_os_spi_init(&p_ad4170_dev_inst->spi_desc,
			     &ad4170_init_params.spi_init);
	if (ret) {
		return ret;
	}

	/* Set SYNC High to Initiate conversion */
	ret = no_os_gpio_set_value(p_ad4170_dev_inst->gpio_sync_inb, NO_OS_GPIO_HIGH);
	if (ret) {
		return ret;
	}

	/* Stop data capture */
	ret = ad4170_stop_data_capture();
	if (ret) {
		return ret;
	}

	/* Restore (re-enable) the previous active channels */
	ret = ad4170_set_channel_en(p_ad4170_dev_inst, prev_active_channels);
	if (ret) {
		return ret;
	}
	dma_config_updated = false;
#endif

	data_capture_operation = false;

	return 0;
}

/**
 * @brief Push data into IIO buffer when trigger handler IRQ is invoked
 * @param iio_dev_data[in] - IIO device data instance
 * @return 0 in case of success or negative value otherwise
 */
int32_t iio_ad4170_trigger_handler(struct iio_device_data *iio_dev_data)
{
	int32_t ret;
	uint32_t adc_raw;

	if (data_capture_started) {
#if (INTERFACE_MODE == TDM_MODE)
		/* Disable IIO trigger after first occurrence to the trigger handler.
		 * The handler is enabled only once to point the private iio_dev_data to a
		 * global ad4170_iio_dev_data structure variable for future IIO CB operations */
		ret = iio_trig_disable(ad4170_hw_trig_desc);
		if (ret) {
			return ret;
		}

		ad4170_iio_dev_data = iio_dev_data;
		is_triggered = true;
#else
		if (!buf_size_updated) {
			/* Update total buffer size according to bytes per scan for proper
			 * alignment of multi-channel IIO buffer data */
			iio_dev_data->buffer->buf->size = ((uint32_t)(DATA_BUFFER_SIZE /
							   iio_dev_data->buffer->bytes_per_scan)) * iio_dev_data->buffer->bytes_per_scan;
			buf_size_updated = true;
		}

		/* Read adc conversion data for previously enabled channel in the sequencer */
		ret = ad4170_read_converted_sample(&adc_raw);
		if (ret) {
			return ret;
		}

		ret = no_os_cb_write(iio_dev_data->buffer->buf, &adc_raw, BYTES_PER_SAMPLE);
		if (ret) {
			return ret;
		}
#endif
	}

	return 0;
}

/*********************************************************
 *               IIO Attributes and Structures
 ********************************************************/

/* IIOD channels attributes list */
struct iio_attribute channel_input_attributes[] = {
	{
		.name = "raw",
		.show = get_adc_raw,
		.store = set_adc_raw,
		.priv = IIO_RAW_ATTR_ID
	},
	{
		.name = "scale",
		.show = get_adc_raw,
		.store = set_adc_raw,
		.priv = IIO_SCALE_ATTR_ID
	},
	{
		.name = "offset",
		.show = get_adc_raw,
		.store = set_adc_raw,
		.priv = IIO_OFFSET_ATTR_ID
	},
	{
		.name = "internal_calibration",
		.show = get_calibration_status,
		.store = set_calibration_routine,
		.priv = INTERNAL_CALIB_ID
	},
	{
		.name = "system_calibration",
		.show = get_calibration_status,
		.store = set_calibration_routine,
		.priv = SYSTEM_CALIB_ID
	},
#if (ACTIVE_DEMO_MODE_CONFIG == LOADCELL_CONFIG)
	{
		.name = "loadcell_offset_calibration",
		.show = get_loadcell_calibration_status,
		.store = set_loadcell_calibration_status,
		.priv = LOADCELL_OFFSET_CALIB_ID
	},
	{
		.name = "loadcell_gain_calibration",
		.show = get_loadcell_calibration_status,
		.store = set_loadcell_calibration_status,
		.priv = LOADCELL_GAIN_CALIB_ID
	},
#endif
	{
		.name = "filter",
		.show = get_filter,
		.store = set_filter,
		.priv = FILTER_ATTR_ID
	},
	{
		.name = "filter_available",
		.show = get_filter_available,
		.store = set_filter_available,
		.priv = FILTER_ATTR_ID
	},

	{
		.name = "ref_select",
		.show = get_reference,
		.store = set_reference,
		.priv = REF_SELECT_ATTR_ID
	},
	{
		.name = "ref_select_available",
		.show = get_reference_available,
		.store = set_reference_available,
		.priv = REF_SELECT_ATTR_ID
	},
	{
		.name = "fs",
		.show = get_fs,
		.store = set_fs
	},
	END_ATTRIBUTES_ARRAY
};

/* IIOD device (global) attributes list */
static struct iio_attribute global_attributes[] = {
	{
		.name = "demo_config",
		.show = get_demo_config,
		.store = set_demo_config
	},
	{
		.name = "sampling_frequency",
		.show = get_sampling_frequency,
		.store = set_sampling_frequency,
	},
	{
		.name = "diagnostic_error_status",
		.show = get_diag_error,
		.store = set_diag_error
	},
	{
		.name = "adc_mode_available",
		.show = get_adc_mode_available,
		.store = set_adc_mode_available
	},
	{
		.name = "adc_mode",
		.show = get_adc_mode,
		.store = set_adc_mode
	},
	{
		.name = "filter_available",
		.show = get_filter_available,
		.store = set_filter_available
	},
	{
		.name = "clock_ctrl",
		.show = get_clock,
		.store = set_clock
	},
	{
		.name = "clock_ctrl_available",
		.show = get_clock_available,
		.store = set_clock_available
	},

	END_ATTRIBUTES_ARRAY
};

/* IIO voltage type channel structure */
#define AD4170_IIO_VOLT_CH(nm, chn) {\
	.name = nm,\
	.ch_type = IIO_VOLTAGE,\
	.ch_out = false,\
	.indexed = true,\
	.channel = chn,\
	.scan_index = chn,\
	.scan_type = &chn_scan[chn],\
	.attributes = channel_input_attributes\
}

/* IIO temperature type channel structure */
#define AD4170_IIO_TEMP_CH(nm, chn) {\
	.name = nm,\
	.ch_type = IIO_TEMP,\
	.ch_out = false,\
	.indexed = true,\
	.channel = chn,\
	.scan_index = chn,\
	.scan_type = &chn_scan[chn],\
	.attributes = channel_input_attributes\
}

static struct iio_channel iio_ad4170_channels[] = {
#if (ACTIVE_DEMO_MODE_CONFIG == USER_DEFAULT_CONFIG)
	AD4170_IIO_VOLT_CH("Chn0", 0),
	AD4170_IIO_VOLT_CH("Chn1", 1),
	AD4170_IIO_VOLT_CH("Chn2", 2),
#if (TOTAL_CHANNELS > 3)
	AD4170_IIO_VOLT_CH("Chn3", 3),
#endif
#if (TOTAL_CHANNELS > 4)
	AD4170_IIO_VOLT_CH("Chn4", 4),
	AD4170_IIO_VOLT_CH("Chn5", 5),
#endif
#if (TOTAL_CHANNELS > 6)
	AD4170_IIO_VOLT_CH("Chn6", 6),
	AD4170_IIO_VOLT_CH("Chn7", 7),
#endif
#if (TOTAL_CHANNELS > 8)
	AD4170_IIO_VOLT_CH("Chn8", 8),
	AD4170_IIO_VOLT_CH("Chn9", 9),
	AD4170_IIO_VOLT_CH("Chn10", 10),
	AD4170_IIO_VOLT_CH("Chn11", 11),
	AD4170_IIO_VOLT_CH("Chn12", 12),
	AD4170_IIO_VOLT_CH("Chn13", 13),
	AD4170_IIO_VOLT_CH("Chn14", 14),
	AD4170_IIO_VOLT_CH("Chn15", 15),
#endif
#elif (ACTIVE_DEMO_MODE_CONFIG == ACCELEROMETER_CONFIG)
	/* Note" Channel type is considered as voltage as IIO oscilloscope doesn't
	 * support accelerometer unit format of G */
	AD4170_IIO_VOLT_CH("Sensor1", SENSOR_CHANNEL0),
#elif (ACTIVE_DEMO_MODE_CONFIG == LOADCELL_CONFIG)
	/* Note" Channel type is considered as voltage as IIO oscilloscope doesn't
	 * support loadcell unit fomat of gram */
	AD4170_IIO_VOLT_CH("Sensor1", SENSOR_CHANNEL0),
	AD4170_IIO_VOLT_CH("Sensor2", SENSOR_CHANNEL1),
#if defined(FOUR_WIRE_LOAD_CELL)
	AD4170_IIO_VOLT_CH("Sensor3", SENSOR_CHANNEL2),
	AD4170_IIO_VOLT_CH("Sensor4", SENSOR_CHANNEL3),
#endif
#elif (ACTIVE_DEMO_MODE_CONFIG == THERMISTOR_CONFIG)
	AD4170_IIO_TEMP_CH("Sensor1", SENSOR_CHANNEL0),
	AD4170_IIO_TEMP_CH("Sensor2", SENSOR_CHANNEL1),
	AD4170_IIO_TEMP_CH("Sensor3", SENSOR_CHANNEL2),
	AD4170_IIO_TEMP_CH("Sensor4", SENSOR_CHANNEL3),
#elif (ACTIVE_DEMO_MODE_CONFIG == RTD_3WIRE_CONFIG)
	AD4170_IIO_TEMP_CH("Sensor1", SENSOR_CHANNEL0),
	AD4170_IIO_TEMP_CH("Sensor2", SENSOR_CHANNEL1),
#elif (ACTIVE_DEMO_MODE_CONFIG == RTD_2WIRE_CONFIG || ACTIVE_DEMO_MODE_CONFIG == RTD_4WIRE_CONFIG)
	AD4170_IIO_TEMP_CH("Sensor1", SENSOR_CHANNEL0),
	AD4170_IIO_TEMP_CH("Sensor2", SENSOR_CHANNEL1),
	AD4170_IIO_TEMP_CH("Sensor3", SENSOR_CHANNEL2),
#elif (ACTIVE_DEMO_MODE_CONFIG == THERMOCOUPLE_CONFIG)
	AD4170_IIO_TEMP_CH("Sensor1", SENSOR_CHANNEL0),
	AD4170_IIO_TEMP_CH("Sensor2", SENSOR_CHANNEL1),
	AD4170_IIO_TEMP_CH("CJC", SENSOR_CHANNEL2),
#endif
};

/**
 * @brief	Read the IIO local backend event data
 * @param	conn[in] - connection descriptor
 * @param	buf[in] - local backend data handling buffer
 * @param	len[in] - Number of bytes to read
 * @return	0 in case of success, negative error code otherwise
 */
static int iio_ad4170_local_backend_event_read(void *conn, uint8_t *buf,
		uint32_t len)
{
#if (ACTIVE_IIO_CLIENT == IIO_CLIENT_LOCAL)
	return pl_gui_event_read(buf, len);
#endif
}

/**
 * @brief	Write the IIO local backend event data
 * @param	conn[in] - connection descriptor
 * @param	buf[in] - local backend data handling buffer
 * @param	len[in] - Number of bytes to read
 * @return	0 in case of success, negative error code otherwise
 */
static int iio_ad4170_local_backend_event_write(void *conn, uint8_t *buf,
		uint32_t len)
{
#if (ACTIVE_IIO_CLIENT == IIO_CLIENT_LOCAL)
	return pl_gui_event_write(buf, len);
#endif
}

/**
 * @brief Initialization of IIO hardware trigger specific parameters
 * @param desc[in,out] - IIO hardware trigger descriptor
 * @return 0 in case of success, negative error code otherwise
 */
static int32_t ad4170_iio_trigger_param_init(struct iio_hw_trig **desc)
{
	struct iio_hw_trig_init_param ad4170_hw_trig_init_params;
	struct iio_hw_trig *hw_trig_desc;
	int32_t ret;

	hw_trig_desc = calloc(1, sizeof(struct iio_hw_trig));
	if (!hw_trig_desc) {
		return -ENOMEM;
	}

	ad4170_hw_trig_init_params.irq_id = TRIGGER_INT_ID;
	ad4170_hw_trig_init_params.name = IIO_TRIGGER_NAME;
	ad4170_hw_trig_init_params.irq_trig_lvl = NO_OS_IRQ_EDGE_FALLING;
	ad4170_hw_trig_init_params.irq_ctrl = trigger_irq_desc;
	ad4170_hw_trig_init_params.cb_info.event = NO_OS_EVT_GPIO;
	ad4170_hw_trig_init_params.cb_info.peripheral = NO_OS_GPIO_IRQ;
	ad4170_hw_trig_init_params.cb_info.handle = trigger_gpio_handle;
	ad4170_hw_trig_init_params.iio_desc = p_ad4170_iio_desc;

	/* Initialize hardware trigger */
	ret = iio_hw_trig_init(&hw_trig_desc, &ad4170_hw_trig_init_params);
	if (ret) {
		return ret;
	}

	*desc = hw_trig_desc;

	return 0;
}

/**
 * @brief	Init for reading/writing and parameterization of a
 * 			ad4170 IIO device
 * @param 	desc[in,out] - IIO device descriptor
 * @return	0 in case of success or negative value otherwise
 */
static int32_t ad4170_iio_init(struct iio_device **desc)
{
	struct iio_device *iio_ad4170_inst;
	uint8_t chn;
	uint8_t setup;
	bool bipolar;

	iio_ad4170_inst = calloc(1, sizeof(struct iio_device));
	if (!iio_ad4170_inst) {
		return -EINVAL;
	}

	/* Update IIO device init parameters */
	for (chn = 0; chn < AD4170_NUM_CHANNELS; chn++) {
		update_vltg_conv_scale_factor(chn);

		setup = p_ad4170_dev_inst->config.setup[chn].setup_n;
		bipolar = p_ad4170_dev_inst->config.setups[setup].afe.bipolar;

		if (bipolar) {
			chn_scan[chn].sign = 's';
		} else {
			chn_scan[chn].sign = 'u';
		}

		chn_scan[chn].realbits = CHN_REAL_BITS;
		chn_scan[chn].storagebits = CHN_STORAGE_BITS;
#if (INTERFACE_MODE == SPI_DMA_MODE)
		chn_scan[chn].shift = CHN_STORAGE_BITS - CHN_REAL_BITS;
		chn_scan[chn].is_big_endian = true;
#else
		chn_scan[chn].shift = 0;
		chn_scan[chn].is_big_endian = false;
#endif
	}

	iio_ad4170_inst->num_ch = NO_OS_ARRAY_SIZE(iio_ad4170_channels);
	iio_ad4170_inst->channels = iio_ad4170_channels;
	iio_ad4170_inst->attributes = global_attributes;

	iio_ad4170_inst->submit = iio_ad4170_submit_buffer;
	iio_ad4170_inst->pre_enable = iio_ad4170_prepare_transfer;
	iio_ad4170_inst->post_disable = iio_ad4170_end_transfer;
#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE) && (INTERFACE_MODE != SPI_DMA_MODE)
	iio_ad4170_inst->trigger_handler = iio_ad4170_trigger_handler;
#endif

	iio_ad4170_inst->debug_reg_read = debug_reg_read;
	iio_ad4170_inst->debug_reg_write = debug_reg_write;

	num_of_channels = iio_ad4170_inst->num_ch;
	*desc = iio_ad4170_inst;

	return 0;
}

/**
 * @brief Release resources allocated for IIO device
 * @param desc[in] - IIO device descriptor
 * @return 0 in case of success or negative value otherwise
 */
static int32_t ad4170_iio_remove(struct iio_desc *desc)
{
	int32_t status;

	if (!desc) {
		return -EINVAL;
	}

	status = iio_remove(desc);
	if (status) {
		return status;
	}

	return 0;
}

/*!
 * @brief	This is an ISR (Interrupt Service Routine) for Ticker object
 * @param	ctx[in] - Callback context (unused)
 * @return	none
 * @details	This function is periodically called based on the time period
 *			configured during Ticker instance creation/initialization.
 */
void ticker_callback(void *ctx)
{
	static uint32_t tick_cntr;
	static bool led_on = false;

	tick_cntr++;
	if (tick_cntr >= LED_TOGGLE_TICK_CNTR) {
		tick_cntr = 0;

		if (diag_err_status) {
			if (led_on) {
				/* Turn off LED */
				no_os_gpio_set_value(led_gpio_desc, NO_OS_GPIO_HIGH);
				led_on = false;
			} else {
				/* Turn on LED */
				no_os_gpio_set_value(led_gpio_desc, NO_OS_GPIO_LOW);
				led_on = true;
			}
		} else {
			/* Turn off LED */
			no_os_gpio_set_value(led_gpio_desc, NO_OS_GPIO_HIGH);
		}
	}
}

/**
 * @brief Configure filter parameters according to active device chosen
 * @param None
 * @return none
 */
void ad4170_configure_filter_params(void)
{
	uint16_t filter_fs;
	uint8_t setup_id;
	enum ad4170_filter_type  filter_type;

	if (ad4170_init_params.id == ID_AD4170) {
#if (INTERFACE_MODE == SPI_INTERRUPT_MODE)
		filter_fs = FS_SINC5_AVG_24_KSPS;
		filter_type = AD4170_FILT_SINC5_AVG;
#else // TDM_MODE and SPI_DMA_MODE
		filter_fs = FS_SINC5_512_KSPS;
		filter_type = AD4170_FILT_SINC5;
#endif
	} else if (ad4170_init_params.id == ID_AD4190) {
#if (INTERFACE_MODE == SPI_INTERRUPT_MODE)
		filter_fs = FS_SINC5_AVG_24_KSPS;
		filter_fs = AD4170_FILT_SINC5_AVG;
#else // TDM_MODE and SPI_DMA Mode
		filter_fs = FS_SINC3_62P5_KSPS;
		filter_type = AD4170_FILT_SINC3;
#endif
	}

	/* Update the setup register with the filter and FS value */
	for (setup_id = 0; setup_id < AD4170_NUM_SETUPS; setup_id++) {
		ad4170_init_params.config.setups[setup_id].filter.filter_type = filter_type;
		ad4170_init_params.config.setups[setup_id].filter_fs = filter_fs;
	}

	/* Calculate the effective sampling rate of the device */
	sampling_rate = (AD4170_INTERNAL_CLOCK / (FILTER_SCALE * filter_fs));
}

/**
 * @brief	Initialize the IIO interface for AD4170 IIO device
 * @return	none
 * @return	0 in case of success, negative error code otherwise
 */
int32_t ad4170_iio_initialize(void)
{
	int32_t init_status;
	uint8_t read_id;

	/* Init the system peripherals */
	init_status = init_system();
	if (init_status) {
		return init_status;
	}

	/* Read context attributes */
	for (read_id = 0; read_id < NO_OS_ARRAY_SIZE(mezzanine_names); read_id++) {
		init_status = get_iio_context_attributes(&iio_init_params.ctx_attrs,
				&iio_init_params.nb_ctx_attr,
				eeprom_desc,
				mezzanine_names[read_id],
				STR(HW_CARRIER_NAME),
				&hw_mezzanine_is_valid);
		if (init_status) {
			return init_status;
		}

		if (hw_mezzanine_is_valid) {
			switch (read_id) {
			case 0:
			case 1:
				/* AD4170 Device */
				ad4170_init_params.id = ID_AD4170;
				iio_device_init_params[0].name = active_dev[0];
				break;

			case 2:
				/* AD4190 Device */
				ad4170_init_params.id = ID_AD4190;
				iio_device_init_params[0].name = active_dev[1];
				break;

			default:
				return -EINVAL;
			}

			break;
		}
	}

	/* Re-assign the parameters according to the active device */
	ad4170_configure_filter_params();

	if (hw_mezzanine_is_valid) {
		/* Initialize AD4170 device and peripheral interface */
		init_status = ad4170_init(&p_ad4170_dev_inst, &ad4170_init_params);
		if (init_status) {
			return init_status;
		}

		/* Initialize the device if HW mezzanine status is valid */
		init_status = ad4170_iio_init(&p_iio_ad4170_dev);
		if (init_status) {
			return init_status;
		}

		iio_device_init_params[0].name = active_dev[p_ad4170_dev_inst->id];
		iio_device_init_params[0].raw_buf = adc_data_buffer;
		iio_device_init_params[0].raw_buf_len = DATA_BUFFER_SIZE;

		iio_device_init_params[0].dev = p_ad4170_dev_inst;
		iio_device_init_params[0].dev_descriptor = p_iio_ad4170_dev;

		iio_init_params.nb_devs++;

#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE) && (INTERFACE_MODE != SPI_DMA_MODE)
		iio_init_params.nb_trigs++;
#endif
	}

	/* Initialize the IIO interface */
	iio_init_params.uart_desc = uart_desc;
	iio_init_params.devs = iio_device_init_params;
	init_status = iio_init(&p_ad4170_iio_desc, &iio_init_params);
	if (init_status) {
		ad4170_iio_remove(p_ad4170_iio_desc);
		return init_status;
	}

#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE) && (INTERFACE_MODE != SPI_DMA_MODE)
	/* Initialize the IIO trigger specific parameters */
	init_status = ad4170_iio_trigger_param_init(&ad4170_hw_trig_desc);
	if (init_status) {
		return init_status;
	}
#endif

#if (ACTIVE_IIO_CLIENT == IIO_CLIENT_LOCAL)
	pocket_lab_gui_init_params.extra = &iio_init_params;
	init_status = pl_gui_init(&pocket_lab_gui_desc, &pocket_lab_gui_init_params);
	if (init_status) {
		return init_status;
	}
#endif

	return init_status;
}

/**
 * @brief 	Run the AD4170 IIO event handler
 * @return	none
 * @details	This function monitors the new IIO client event
 */
void ad4170_iio_event_handler(void)
{
	(void)iio_step(p_ad4170_iio_desc);
#if (ACTIVE_IIO_CLIENT == IIO_CLIENT_LOCAL)
	pl_gui_event_handle(LVGL_TICK_TIME_MS);
#endif
}
