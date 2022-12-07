/*!
 *****************************************************************************
  @file:  ad7124_console_app.c

  @brief: Implementation for the menu functions that handle the AD7124

  @details: This file is specific to ad7124 console menu application handle.
            The functions defined in this file performs the action
            based on user selected console menu.
 -----------------------------------------------------------------------------
 Copyright (c) 2019-2022 Analog Devices, Inc.
 All rights reserved.

 This software is proprietary to Analog Devices, Inc. and its licensors.
 By using this software you agree to the terms of the associated
 Analog Devices Software License Agreement.
*****************************************************************************/

/* includes */
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#include "app_config.h"

#include "mbed_platform_support.h"
#include "no_os_error.h"
#include "no_os_gpio.h"
#include "no_os_spi.h"
#include "mbed_spi.h"
#include "mbed_gpio.h"

#include "ad7124.h"
#include "ad7124_regs.h"
#include "ad7124_support.h"
#include "ad7124_regs_configs.h"

#include "ad7124_console_app.h"


/*  defines */
#define AD7124_CHANNEL_COUNT 16

#define SHOW_ALL_CHANNELS      false
#define SHOW_ENABLED_CHANNELS  true

#define DISPLAY_DATA_TABULAR    0
#define DISPLAY_DATA_STREAM     1

#define AD7124_MAX_SETUPS       8
#define AD7124_MAX_CHANNELS     16
#define NUM_OF_FILTERS          5
#define MAX_FILTER_DATA_RATE_FS 2047
#define MIN_FILTER_DATA_RATE    1
#define MAX_GAIN_BITS_VALUE     7
#define MIN_PROGRAMMABLE_GAIN   1
#define MAX_PROGRAMMABLE_GAIN   128
#define MAX_ANALOG_INPUTS       32


/* Private Variables */
/*
 * This is the 'live' AD7124 register map that is used by the driver
 * the other 'default' configs are used to populate this at init time
 */
static struct ad7124_st_reg ad7124_register_map[AD7124_REG_NO];

// Pointer to the struct representing the AD7124 device
static struct ad7124_dev * pAd7124_dev = NULL;

// GPIO descriptor and init parameters for the activity LED pin
static struct no_os_gpio_desc *activity_led_desc = NULL;
static struct no_os_gpio_init_param activity_led_init_param = {
	.number =  LED_GREEN,
	.platform_ops = &mbed_gpio_ops,
	.extra = NULL
};

// Last Sampled values for All ADC channels
static uint32_t channel_samples[AD7124_CHANNEL_COUNT] = {0};
// How many times a given channel is sampled in total for one sample run
static uint32_t channel_samples_count[AD7124_CHANNEL_COUNT] = {0};

static ad7124_setup_config setup[AD7124_MAX_SETUPS];  // AD7124 setup

static float filter_data_rate_raw;     // filter data rate value
static uint32_t gain_raw;              // Gain value
static uint8_t power_mode;             // power mode of ADC

typedef enum {
	CHN_DISABLE,
	CHN_ENABLE
} Chn_EnbleDisable_action;

/* Filter names */
static char *filter_name[] = {
	"SINC4",
	"Reserved",
	"SINC3",
	"Reserved",
	"FS SINC4",
	"FS SINC3",
	"Reserved",
	"POST SINC3"
};

static char *reference_name[] = {
	"REFIN1",
	"REFIN2",
	"INTERNAL",
	"AVDD"
};

static char *enable_disable_status[] = {
	"DISABLED",
	"ENABLED"
};

static char *polarity_status[] = {
	"UNIPOLAR",
	"BIPOLAR"
};

static char *power_modes_str[] = {
	"Low Power",
	"Medium Power",
	"Full Power",
	"Full Power"
};

/* Programmable gain values */
static uint8_t p_gain[] = { 1, 2, 4, 8, 16, 32, 64, 128 };

/* Temperature sensor data structure */
typedef struct {
	int16_t temp;              // Temperature
	uint32_t adc_sample;       // ADC conversion sample
} temp_loopup;


/* Function protoypes */
static int32_t menu_continuous_conversion_tabular(uint32_t);
static int32_t menu_continuous_conversion_stream(uint32_t);
static int32_t menu_single_conversion(uint32_t id);
static int32_t menu_read_status(uint32_t id);
static int32_t menu_read_id(uint32_t id);
static int32_t menu_reset(uint32_t id);
static int32_t menu_reset_to_configuration(uint32_t config_type);
static int32_t menu_read_temperature(uint32_t id);
static int32_t menu_select_power_mode(uint32_t id);
static int32_t menu_power_modes_selection(uint32_t power_mode);
static int32_t menu_rw_ad7124_register(uint32_t rw_id);
static int32_t menu_sample_channels(uint32_t id);
static int32_t menu_enable_disable_channels(uint32_t id);
static int32_t menu_config_and_assign_setup(uint32_t id);
static int32_t menu_connect_input_to_channel(uint32_t id);
static int32_t menu_calibrate_adc(uint32_t id);

static bool was_escape_key_pressed(void);
static void toggle_activity_led(void);
static void read_status_register(void);
static void dislay_channel_samples(bool showOnlyEnabledChannels,
				   uint8_t console_mode);
static void clear_channel_samples(void);
static int32_t do_continuous_conversion(uint8_t display_mode);
static int16_t scan_temperature(uint32_t value);
static void init_with_configuration(uint8_t configID);
static uint8_t get_setup_selection(void);
static uint8_t get_channel_selection(void);
static void config_analog_inputs(ad7124_setup_config *psetup);
static float calculate_data_rate(filter_type filter, uint8_t  power_mode,
				 float data_rate);
static void config_filter_parameters(ad7124_setup_config *psetup);
static void assign_setup_to_channel(uint8_t setup);


/* The temperature lookup table is formed using below formula:
 * temp = ((adc_conv_reading - 8388608) / 13584) - 272.5;
 *
 * Note: Refer datasheet page 71 (TEMPERATURE SENSOR) for using internal
 * temperature sensor of AD7124, which monitors the die temperature.
 * This method of temperature sensing is not accurate and might have deviations
 * of +/-1C due to non-floating calculations. In order to get more precise result,
 * use floating point calculations (using above formula).
 **/

/* Temperature Range: -20C to +50C */
static temp_loopup temperature_lookup[] = {
	{ -20, 11818568 },
	{ -19, 11832152 },
	{ -18, 11845736 },
	{ -17, 11859320 },
	{ -16, 11872904 },
	{ -15, 11886488 },
	{ -14, 11900072 },
	{ -13, 11913656 },
	{ -12, 11927240 },
	{ -11, 11940824 },
	{ -10, 11954408 },
	{ -9, 11967992 },
	{ -8, 11981576 },
	{ -7, 11995160 },
	{ -6, 12008744 },
	{ -5, 12022328 },
	{ -4, 12035912 },
	{ -3, 12049496 },
	{ -2, 12063080 },
	{ -1, 12076664 },
	{ 0, 12090248 },
	{ 1, 12103832 },
	{ 2, 12117416 },
	{ 3, 12131000 },
	{ 4, 12144584 },
	{ 5, 12158168 },
	{ 6, 12171752 },
	{ 7, 12185336 },
	{ 8, 12198920 },
	{ 9, 12212504 },
	{ 10, 12226088 },
	{ 11, 12239672 },
	{ 12, 12253256 },
	{ 13, 12266840 },
	{ 14, 12280424 },
	{ 15, 12294008 },
	{ 16, 12307592 },
	{ 17, 12321176 },
	{ 18, 12334760 },
	{ 19, 12348344 },
	{ 20, 12361928 },
	{ 21, 12375512 },
	{ 22, 12389096 },
	{ 23, 12402680 },
	{ 24, 12416264 },
	{ 25, 12429848 },
	{ 26, 12443432 },
	{ 27, 12457016 },
	{ 28, 12470600 },
	{ 29, 12484184 },
	{ 30, 12497768 },
	{ 31, 12511352 },
	{ 32, 12524936 },
	{ 33, 12538520 },
	{ 34, 12552104 },
	{ 35, 12565688 },
	{ 36, 12579272 },
	{ 37, 12592856 },
	{ 38, 12606440 },
	{ 39, 12620024 },
	{ 40, 12633608 },
	{ 41, 12647192 },
	{ 42, 12660776 },
	{ 43, 12674360 },
	{ 44, 12687944 },
	{ 45, 12701528 },
	{ 46, 12715112 },
	{ 47, 12728696 },
	{ 48, 12742280 },
	{ 49, 12755864 },
	{ 50, 12769448 }
};


// Public Functions

/*!
 * @brief      Initialize the AD7124 device and the SPI port as required
 * @param      configID- Configuration ID
 * @return     0 in case of success, negative error code otherwise
 * @details    This resets and then writes the default register map value to
 *             the device.  A call to init the SPI port is made, but may not
 *             actually do very much, depending on the platform
 */
int32_t ad7124_app_initialize(uint8_t configID)
{
	// Init SPI extra parameters structure
	struct mbed_spi_init_param spi_init_extra_params = {
		.spi_clk_pin =	SPI_SCK,
		.spi_miso_pin = SPI_HOST_SDI,
		.spi_mosi_pin = SPI_HOST_SDO
	};

	// Create a new descriptor for activity led
	if (no_os_gpio_get(&activity_led_desc, &activity_led_init_param) != 0) {
		return -EINVAL;
	}

	// Set the direction of activity LED
	if ((no_os_gpio_direction_output(activity_led_desc,
					 NO_OS_GPIO_HIGH)) != 0) {
		return -EINVAL;
	}

	/*
	 * Copy one of the default/user configs to the live register memory map
	 * Requirement, not checked here, is that all the configs are the same size
	 */
	switch (configID) {
	case AD7124_CONFIG_A: {
		memcpy(ad7124_register_map, ad7124_regs_config_a, sizeof(ad7124_register_map));
		break;
	}
	case AD7124_CONFIG_B: {
		memcpy(ad7124_register_map, ad7124_regs_config_b, sizeof(ad7124_register_map));
		break;
	}
	default:
		// Not a defined configID
		return -EINVAL;
	}

	// Designated SPI Initialization Structure
	struct no_os_spi_init_param	ad7124_spi_init = {
		.max_speed_hz = 2500000, // Max SPI Speed
		.chip_select =  SPI_CSB, // Chip Select pin
		.mode = NO_OS_SPI_MODE_3, // CPOL = 1, CPHA =1
		.extra = &spi_init_extra_params,  // SPI extra configurations
		.platform_ops = &mbed_spi_ops
	};

	// Used to create the ad7124 device
	struct  ad7124_init_param sAd7124_init = {
		&ad7124_spi_init,  // spi_init_param type
		ad7124_register_map,
		10000               // Retry count for polling
	};

	return (ad7124_setup(&pAd7124_dev, &sAd7124_init));
}

// Private Functions

/*!
 * @brief      determines if the Escape key was pressed
 *
 * @return     bool- key press status
 */
static bool was_escape_key_pressed(void)
{
	char rxChar;
	bool wasPressed = false;

	// Check for Escape key pressed
	if ((rxChar = getchar_noblock()) > 0) {
		if (rxChar == ESCAPE_KEY_CODE) {
			wasPressed = true;
		}
	}
	return (wasPressed);
}


/**
  * @brief  toggles an LED to show something has happened
  * @param  None
  * @return None
  */
static void toggle_activity_led(void)
{
	static int led_state = NO_OS_GPIO_HIGH;

	/* Toggle the LED state */
	if (led_state == NO_OS_GPIO_LOW) {
		led_state = NO_OS_GPIO_HIGH;
	} else {
		led_state = NO_OS_GPIO_LOW;
	}

	no_os_gpio_set_value(activity_led_desc, led_state);
}


/*!
 * @brief      reads and displays the status register on the AD7124
 *
 * @details
 */
static void read_status_register(void)
{
	if (ad7124_read_register(pAd7124_dev,
				 &ad7124_register_map[AD7124_Status]) < 0) {
		printf("\r\nError Encountered reading Status register\r\n");
	} else {
		uint32_t status_value = (uint32_t)ad7124_register_map[AD7124_Status].value;
		printf("\r\nRead Status Register = 0x%02lx\r\n", status_value);
	}
}


/*!
 * @brief      displays the current sample value for a ADC channels
 * @param showOnlyEnabledChannels  only channels that are enabled are displayed
 * @param console_mode  Mode of console display
 * @details
 *
 */
static void dislay_channel_samples(bool showOnlyEnabledChannels,
				   uint8_t console_mode)
{
	switch (console_mode) {
	case DISPLAY_DATA_TABULAR: {
		printf("\tCh\tValue\t\tCount\t\tVoltage\r\n");
		for (uint8_t i = 0; i < AD7124_CHANNEL_COUNT; i++) {
			// if showing all channels, or channel is enabled
			if ((showOnlyEnabledChannels == false)
			    || (ad7124_register_map[AD7124_Channel_0 + i].value &
				AD7124_CH_MAP_REG_CH_ENABLE)) {
				printf("\t%-2d\t%-10ld\t%ld\t\t% .6f\r\n", \
				       i,
				       channel_samples[i],
				       channel_samples_count[i],
				       ad7124_convert_sample_to_voltage(pAd7124_dev, i, channel_samples[i]));
			}
		}
		break;
	}
	case DISPLAY_DATA_STREAM: {
		// Output a CSV list of the sampled channels as voltages on a single line
		bool channel_printed = false;

		for (uint8_t i = 0; i < AD7124_CHANNEL_COUNT; i++) {
			// if showing all channels, or channel is enabled
			if ((showOnlyEnabledChannels == false)
			    || (ad7124_register_map[AD7124_Channel_0 + i].value &
				AD7124_CH_MAP_REG_CH_ENABLE)) {
				/*
				 *  add the comma before we output the next channel but
				 *  only if at least one channel has been printed
				 */
				if (channel_printed) {
					printf(", ");
				}
				printf("%.6f", \
				       ad7124_convert_sample_to_voltage(pAd7124_dev, i, channel_samples[i]));
				channel_printed = true;
			}
		}
		printf("\r\n");
		break;
	}
	default: {
		// ASSERT(false);
	}
	}
}


/*!
 * @brief      resets the channelSampleCounts to zero
 *
 * @details
 */
static void clear_channel_samples(void)
{
	for (uint8_t i = 0; i < AD7124_CHANNEL_COUNT; i++) {
		channel_samples[i] = 0;
		channel_samples_count[i] = 0;
	}
}


/*!
 * @brief      Continuously acquires samples in Continuous Conversion mode
 * @param      display_mode  Display Mode
 * @return     menu status constant
 * @details   The ADC is run in continuous mode, and all samples are acquired
 *            and assigned to the channel they come from. Escape key an be used
 *            to exit the loop
 */
static int32_t do_continuous_conversion(uint8_t display_mode)
{
	int32_t error_code;
	int32_t sample_data;

	// Clear the ADC CTRL MODE bits, has the effect of selecting continuous mode
	ad7124_register_map[AD7124_ADC_Control].value &= ~(AD7124_ADC_CTRL_REG_MODE(
				0xf));
	if ((error_code = ad7124_write_register(pAd7124_dev,
						ad7124_register_map[AD7124_ADC_Control])) < 0) {
		printf("Error (%ld) setting AD7124 Continuous conversion mode.\r\n",
		       error_code);
		adi_press_any_key_to_continue();
		return (MENU_CONTINUE);
	}

	clear_channel_samples();

	/*
	 *  If displaying data in stream form, want to output a channel header
	 */
	if (display_mode == DISPLAY_DATA_STREAM) {
		bool channel_printed = false;

		for (uint8_t i = 0; i < AD7124_CHANNEL_COUNT; i++) {
			// if showing all channels, or channel is enabled
			if(ad7124_register_map[AD7124_Channel_0 + i].value &
			    AD7124_CH_MAP_REG_CH_ENABLE) {
				/*
				 *  add the comma before we output the next channel but
				 *  only if at least one channel has been printed
				 */
				if (channel_printed) {
					printf(", ");
				}
				printf("%d", i);
			}
			channel_printed = true;
		}
		printf("\r\n");
	}

	// Continuously read the channels, and store sample values
	while(was_escape_key_pressed() != true) {
		toggle_activity_led();

		if (display_mode == DISPLAY_DATA_TABULAR) {
			adi_clear_console();
			printf("Running continuous conversion mode...\r\nPress Escape to stop\r\n\r\n");
		}

		/*
		 *  this polls the status register READY/ bit to determine when conversion is done
		 *  this also ensures the STATUS register value is up to date and contains the
		 *  channel that was sampled as well.
		 *  Generally, no need to read STATUS separately, but for faster sampling
		 *  enabling the DATA_STATUS bit means that status is appended to ADC data read
		 *  so the channel being sampled is read back (and updated) as part of the same frame
		 */

		if ((error_code = ad7124_wait_for_conv_ready(pAd7124_dev, 10000)) < 0) {
			printf("Error/Timeout waiting for conversion ready %ld\r\n", error_code);
			continue;
		}

		if ((error_code = ad7124_read_data(pAd7124_dev, &sample_data)) < 0) {
			printf("Error reading ADC Data (%ld).\r\n", error_code);
			continue;
		}

		/*
		 * No error, need to process the sample, what channel has been read? update that channelSample
		 */
		uint8_t channel_read = ad7124_register_map[AD7124_Status].value & 0x0000000F;

		if (channel_read < AD7124_CHANNEL_COUNT) {
			channel_samples[channel_read] = sample_data;
			channel_samples_count[channel_read]++;
		} else {
			printf("Channel Read was %d, which is not < AD7124_CHANNEL_COUNT\r\n",
			       channel_read);
		}

		dislay_channel_samples(SHOW_ENABLED_CHANNELS, display_mode);
	}

	// All done, ADC put into standby mode
	ad7124_register_map[AD7124_ADC_Control].value &= ~(AD7124_ADC_CTRL_REG_MODE(
				0xf));
	// 2 = sleep/standby mode
	ad7124_register_map[AD7124_ADC_Control].value |= AD7124_ADC_CTRL_REG_MODE(2);

	if ((error_code = ad7124_write_register(pAd7124_dev,
						ad7124_register_map[AD7124_ADC_Control])) < 0) {
		printf("Error (%ld) setting AD7124 ADC into standby mode.\r\n", error_code);
		adi_press_any_key_to_continue();
	}

	return (MENU_CONTINUE);
}


/*!
 * @brief      Samples all enabled channels and displays in tabular form
 * @param      id (Unused)
 * @return     menu status constant
 * @details
 */
static int32_t menu_continuous_conversion_tabular(uint32_t id)
{
	do_continuous_conversion(DISPLAY_DATA_TABULAR);

	adi_clear_console();
	printf("Continuous Conversion completed...\r\n\r\n");
	dislay_channel_samples(SHOW_ALL_CHANNELS, DISPLAY_DATA_TABULAR);
	adi_press_any_key_to_continue();

	return (MENU_CONTINUE);
}


/*!
 * @brief      Samples all enabled channels and displays on the console
 * @param      id (Unused)
 * @return     menu status constant
 * @details
 */
static int32_t menu_continuous_conversion_stream(uint32_t id)
{
	do_continuous_conversion(DISPLAY_DATA_STREAM);
	printf("Continuous Conversion completed...\r\n\r\n");
	adi_press_any_key_to_continue();
	return (MENU_CONTINUE);
}


/*!
 * @brief      Samples all enabled channels once in Single Conversion mode
 * @param      id (Unused)
 * @return     menu status constant
 * @details    This stores all channels that are enabled in a bitmask, and then
 *             runs the ADC in single conversion mode, which acquires one channel
 *             of data at a time. After capture, that channel is disabled, and
 *             single conversion run again, until no channels are enabled.
 *             The original enable state of each channel is then restored.
 */
static int32_t menu_single_conversion(uint32_t id)
{
	int32_t    error_code;
	uint16_t   channel_enable_mask = 0;
	uint8_t    channel_count = 0;
	int32_t    sample_data;

	// Need to store which channels are enabled in this config so it can be restored
	for (uint8_t i = 0 ; i < AD7124_CHANNEL_COUNT ; i++) {
		if (ad7124_register_map[AD7124_Channel_0 + i].value &
		    AD7124_CH_MAP_REG_CH_ENABLE) {
			channel_enable_mask |= (1 << i);
			channel_count++;
		}
	}

	clear_channel_samples();
	adi_clear_console();
	printf("Running Single conversion mode...\r\nPress Escape to stop\r\n\r\n");

	// Clear the ADC CTRL MODE bits, selecting continuous mode
	ad7124_register_map[AD7124_ADC_Control].value &= ~(AD7124_ADC_CTRL_REG_MODE(
				0xf));

	// read the channels, and store sample values
	for (uint8_t loopCount = 0 ; ((was_escape_key_pressed() != true)
				      && (loopCount < channel_count)) ; loopCount++) {
		toggle_activity_led();

		// 1 = single conversion mode
		ad7124_register_map[AD7124_ADC_Control].value |= AD7124_ADC_CTRL_REG_MODE(1);

		if ((error_code = ad7124_write_register(pAd7124_dev,
							ad7124_register_map[AD7124_ADC_Control])) < 0) {
			printf("Error (%ld) setting AD7124 Single conversion mode.\r\n", error_code);
			adi_press_any_key_to_continue();
			continue;
		}

		/*
		 *  this polls the status register READY/ bit to determine when conversion is done
		 *  this also ensures the STATUS register value is up to date and contains the
		 *  channel that was sampled as well. No need to read STATUS separately
		 */
		if ((error_code = ad7124_wait_for_conv_ready(pAd7124_dev, 10000)) < 0) {
			printf("Error/Timeout waiting for conversion ready %ld\r\n", error_code);
			continue;
		}

		if ((error_code = ad7124_read_data(pAd7124_dev, &sample_data)) < 0) {
			printf("Error reading ADC Data (%ld).\r\n", error_code);
			continue;
		}
		/*
		 * No error, need to process the sample, what channel has been read? update that channelSample
		 */
		uint8_t channelRead = ad7124_register_map[AD7124_Status].value & 0x0000000F;

		if (channelRead < AD7124_CHANNEL_COUNT) {
			channel_samples[channelRead] = sample_data;
			channel_samples_count[channelRead]++;

			/* also need to clear the channel enable bit so the next single conversion cycle will sample the next channel */
			ad7124_register_map[AD7124_Channel_0 + channelRead].value &=
				~AD7124_CH_MAP_REG_CH_ENABLE;
			if ((error_code = ad7124_write_register(pAd7124_dev,
								ad7124_register_map[AD7124_Channel_0 + channelRead])) < 0) {
				printf("Error (%ld) Clearing channel %d Enable bit.\r\n", error_code,
				       channelRead);
				adi_press_any_key_to_continue();
				continue;
			}
		} else {
			printf("Channel Read was %d, which is not < AD7124_CHANNEL_COUNT\r\n",
			       channelRead);
		}
	}

	// All done, ADC put into standby mode
	ad7124_register_map[AD7124_ADC_Control].value &= ~(AD7124_ADC_CTRL_REG_MODE(
				0xf));
	// 2 = sleep/standby mode
	ad7124_register_map[AD7124_ADC_Control].value |= AD7124_ADC_CTRL_REG_MODE(2);

	// Need to restore the channels that were disabled during acquisition
	for (uint8_t i = 0 ; i < AD7124_CHANNEL_COUNT ; i++) {
		if (channel_enable_mask & (1 << i)) {
			ad7124_register_map[AD7124_Channel_0 + i].value |= AD7124_CH_MAP_REG_CH_ENABLE;
			if ((error_code = ad7124_write_register(pAd7124_dev,
								ad7124_register_map[AD7124_Channel_0 + i])) < 0) {
				printf("Error (%ld) Setting channel %d Enable bit.\r\r\n", error_code, i);
				adi_press_any_key_to_continue();
				return (MENU_CONTINUE);
			}
		}
	}

	printf("Single Conversion completed...\r\n\r\n");
	dislay_channel_samples(SHOW_ENABLED_CHANNELS, DISPLAY_DATA_TABULAR);

	adi_press_any_key_to_continue();
	return (MENU_CONTINUE);
}


/*!
 * @brief      menu item that reads the status register the AD7124
 * @param      id (Unused)
 * @return     menu status constant
 * @details
 */
static int32_t menu_read_status(uint32_t id)
{
	read_status_register();
	adi_press_any_key_to_continue();
	return (MENU_CONTINUE);
}


/*!
 * @brief      reads the ID register on the AD7124
 * @param      id (Unused)
 * @return     menu status constant
 * @details
 */
static int32_t menu_read_id(uint32_t id)
{
	if (ad7124_read_register(pAd7124_dev, &ad7124_register_map[AD7124_ID]) < 0) {
		printf("\r\nError Encountered reading ID register\r\n");
	} else {
		printf("\r\nRead ID Register = 0x%02lx\r\n",
		       (uint32_t)ad7124_register_map[AD7124_ID].value);
	}
	adi_press_any_key_to_continue();
	return (MENU_CONTINUE);
}


/*!
 * @brief      Initialize the part with a specific configuration
 * @param      configID Configuration ID
 * @details
 */
static void init_with_configuration(uint8_t configID)
{
	int32_t status = 0;

	// Free the device resources
	(void)no_os_gpio_remove(activity_led_desc);
	(void)ad7124_remove(pAd7124_dev);

	status = ad7124_app_initialize(configID);
	if (status < 0) {
		printf("\r\n\r\n Error setting Configuration %c \r\n\r\n",
		       (char)(configID + 'A'));
	} else {
		printf("\r\n\r\n Configuration %c Set\r\n\r\n", (char)(configID + 'A'));
	}
	adi_press_any_key_to_continue();
}


/*
 * @brief      Sends a reset command on the SPI to reset the AD7124
 * @param      id (Unused)
 * @return     menu status constant
 * @details
 */
static int32_t menu_reset(uint32_t id)
{
	if (ad7124_reset(pAd7124_dev)  < 0) {
		printf("\r\n\r\n Error performing Reset\r\n\r\n");
	} else {
		// Need to set the live register map to defaults as well
		memcpy(ad7124_register_map, ad7124_regs, sizeof(ad7124_register_map));
		printf("\r\n\r\n Reset Complete\r\n\r\n");
	}
	adi_press_any_key_to_continue();
	return (MENU_CONTINUE);
}


/*!
 * @brief      Reset and set the ad7124 with configuration A or B
 * @param      config_type Configuration Type
 * @return     menu status constant
 * @details
 */
static int32_t menu_reset_to_configuration(uint32_t config_type)
{
	if (AD7124_CONFIG_A == config_type) {
		init_with_configuration(AD7124_CONFIG_A);
	} else {
		init_with_configuration(AD7124_CONFIG_B);
	}

	return (MENU_CONTINUE);
}


/* @brief  Scan the temparture value from the lookup table using binry search
 *
 * @param  ADC conversion sample
 * @return Temperature
 **/
static int16_t scan_temperature(uint32_t value)
{
	uint16_t key=0, start, end;
	bool found = false;

	start = 0;
	end = sizeof(temperature_lookup) / sizeof(temperature_lookup[0]) - 1;

	while ((start < end) && !found) {
		key = (start + end) >> 1;

		if (temperature_lookup[key].adc_sample == value) {
			found = true;
		} else if (value > temperature_lookup[key].adc_sample) {
			start = key + 1;
		} else if (value < temperature_lookup[key].adc_sample) {
			end = key - 1;
		} else {
			break;
		}
	}

	/* Return the scanned temperature value */
	return temperature_lookup[key].temp;
}


/* @brief  Console menu to read and display device temperature
 * @param  id (Unused)
 * @return int32_t- menu status constant
 **/
static int32_t menu_read_temperature(uint32_t id)
{
	int32_t error_code;         // adc read/write error
	uint8_t samples;            // sample count
	int32_t temp_readings;      // temperature equivalent adc count
	int16_t temperature = 0;    // temperature read
	uint8_t chn_cnt;            // channel counter
	uint16_t chn_mask = 0;      // channels enable mask

	/* Save the previous values of below registers in order to not disturb setup
	 * configured by the user (Note: Channel 0 is used for the temperature sensing) */
	int32_t prev_adc_reg_values[] = {
		ad7124_register_map[AD7124_Channel_0].value,    // Channel_0 Register previous value
		ad7124_register_map[AD7124_Config_0].value,     // Config_0 Register previous value
		ad7124_register_map[AD7124_ADC_Control].value   // Control Register previous value
	};

	/* Disable the other enabled channels, to read temperature from only 0th channel */
	for (chn_cnt = 1; chn_cnt < AD7124_MAX_CHANNELS; chn_cnt++) {
		if ((error_code = ad7124_read_register(pAd7124_dev,
						       &ad7124_register_map[AD7124_Channel_0 + chn_cnt]) < 0)) {
			printf("\r\n\tError reading temperature!!\r\n");
			adi_press_any_key_to_continue();
			return MENU_CONTINUE;
		}

		if (ad7124_register_map[AD7124_Channel_0 + chn_cnt].value &
		    AD7124_CH_MAP_REG_CH_ENABLE) {
			// Save enabled channel
			chn_mask |= (1 << chn_cnt);

			/* Disable the curent channel */
			ad7124_register_map[AD7124_Channel_0 + chn_cnt].value &=
				(~AD7124_CH_MAP_REG_CH_ENABLE);

			/* Write to ADC channel register */
			if ((error_code = ad7124_write_register(pAd7124_dev,
								ad7124_register_map[AD7124_Channel_0 + chn_cnt]) < 0)) {
				printf("\r\n\tError reading temperature!!\r\n");
				adi_press_any_key_to_continue();
				return MENU_CONTINUE;
			}
		}
	}

	/* Channel 0 Selections: AINP= Temp (16), AINM= AVSS (17), Setup= 0, CHN Enabled= True */
	ad7124_register_map[AD7124_Channel_0].value = (AD7124_CH_MAP_REG_AINP(
				16) | AD7124_CH_MAP_REG_AINM(17) |
			AD7124_CH_MAP_REG_SETUP(0) | AD7124_CH_MAP_REG_CH_ENABLE);

	/* Write to ADC channel 0 register */
	if ((error_code = ad7124_write_register(pAd7124_dev,
						ad7124_register_map[AD7124_Channel_0]) < 0)) {
		printf("\r\n\tError reading temperature!!\r\n");
		adi_press_any_key_to_continue();
		return MENU_CONTINUE;
	}

	/* Setup 0 selections: Bipolar= 1, AINP/M Buffer= Enabled, Ref= EXT1(2.5v), Gain= 1 */
	ad7124_register_map[AD7124_Config_0].value = (AD7124_CFG_REG_BIPOLAR |
			AD7124_CFG_REG_AIN_BUFP |
			AD7124_CFG_REG_AINN_BUFM | AD7124_CFG_REG_REF_SEL(0) |
			AD7124_CFG_REG_PGA(0));

	/* Write to ADC config 0 register */
	if ((error_code = ad7124_write_register(pAd7124_dev,
						ad7124_register_map[AD7124_Config_0]) < 0)) {
		printf("\r\n\tError reading temperature!!\r\n");
		adi_press_any_key_to_continue();
		return MENU_CONTINUE;
	}

	/* ADC operating mode: Single Conversion (masking off bits 5:2) */
	ad7124_register_map[AD7124_ADC_Control].value = ((
				ad7124_register_map[AD7124_ADC_Control].value &
				~AD7124_ADC_CTRL_REG_MSK) |
			AD7124_ADC_CTRL_REG_MODE(1));

	/* Write to ADC control register */
	if ((error_code = ad7124_write_register(pAd7124_dev,
						ad7124_register_map[AD7124_ADC_Control]) < 0)) {
		printf("\r\n\tError reading temperature!!\r\n");
		adi_press_any_key_to_continue();
		return MENU_CONTINUE;
	}

	printf("\r\n\r\n\tReading temperature...\r\n");

	for (samples = 0; samples < 2; samples++) {
		/* Wait for conversion to complete, then obtain sample */
		ad7124_wait_for_conv_ready(pAd7124_dev, pAd7124_dev->spi_rdy_poll_cnt);

		if ((error_code = ad7124_read_data(pAd7124_dev, &temp_readings) < 0)) {
			printf("\r\n\tError reading temperature!!\r\n");
			adi_press_any_key_to_continue();
			return MENU_CONTINUE;
		}

		//temp += ((temp_readings - 8388608) / 13584) - 272.5; // Use this for more precision
		temperature += scan_temperature(temp_readings);
	}

	/* Get the averaged temperature value */
	temperature >>= 1;

	/* Validate temperaure range as specified in look-up table */
	if (temperature >= -20 || temperature <= 50) {
		printf("\r\n\tTemperature: %d Celcius\r\n", temperature);
	} else {
		printf("\r\n\tError reading temperature!!\r\n");
	}

	/* Restore the ADC registers with previous values (i.e before modifying them for temperature sensing)
	 * Note: This needs to be done to not disturb the setup configured by user through console menus */
	ad7124_register_map[AD7124_Channel_0].value = prev_adc_reg_values[0];
	ad7124_write_register(pAd7124_dev, ad7124_register_map[AD7124_Channel_0]);

	ad7124_register_map[AD7124_Config_0].value = prev_adc_reg_values[1];
	ad7124_write_register(pAd7124_dev, ad7124_register_map[AD7124_Config_0]);

	ad7124_register_map[AD7124_ADC_Control].value = prev_adc_reg_values[2];
	ad7124_write_register(pAd7124_dev, ad7124_register_map[AD7124_ADC_Control]);

	/* Enable the previously disabled channels */
	for (chn_cnt = 1; chn_cnt < AD7124_MAX_CHANNELS; chn_cnt++) {
		if ((chn_mask >> chn_cnt) & 0x01) {
			ad7124_register_map[AD7124_Channel_0 + chn_cnt].value |=
				AD7124_CH_MAP_REG_CH_ENABLE;

			/* Write to ADC channel regiter */
			if ((error_code = ad7124_write_register(pAd7124_dev,
								ad7124_register_map[AD7124_Channel_0 + chn_cnt]) < 0)) {
				printf("\r\n\tError reading temperature!!\r\n");
				adi_press_any_key_to_continue();
				return MENU_CONTINUE;
			}
		}
	}

	adi_press_any_key_to_continue();
	adi_clear_console();

	return (MENU_CONTINUE);
}


/* @brief  Console menu to select the power modes of adc
 * @param  mode Power Mode selected
 * @return int32_t- menu status constant
 **/
static int32_t menu_power_modes_selection(uint32_t mode)
{
	int32_t error_code;

	ad7124_register_map[AD7124_ADC_Control].value =
		((ad7124_register_map[AD7124_ADC_Control].value &
		  ~AD7124_ADC_CTRL_REG_POWER_MODE_MSK) |
		 AD7124_ADC_CTRL_REG_POWER_MODE(mode));

	/* Write to ADC channel regiter */
	if ((error_code = ad7124_write_register(pAd7124_dev,
						ad7124_register_map[AD7124_ADC_Control])) < 0) {
		printf("\r\n\tError setting %s mode!!\r\n", power_modes_str[mode]);
	} else {
		power_mode = mode;
		printf("\r\n\t%s mode selected...\r\n", power_modes_str[mode]);
	}

	adi_press_any_key_to_continue();
	adi_clear_console();

	return MENU_CONTINUE;
}


/* @brief  Console menu to read the adc register
 * @param  rw_id Register ID
 * @return int32_t- menu status constant
 **/
static int32_t menu_rw_ad7124_register(uint32_t rw_id)
{
	uint32_t reg_address;
	uint32_t reg_data;
	int32_t error_code;

	printf("\r\n\tEnter the register address (in hex): ");
	reg_address = adi_get_hex_integer(sizeof(reg_address));

	if ((uint32_t)DEVICE_REG_READ_ID == rw_id) {
		/* Read from ADC channel register */
		if ((reg_address >= AD7124_REG_NO) ||
		    ((error_code = ad7124_read_register(pAd7124_dev,
							&ad7124_register_map[reg_address])) < 0)) {
			printf("\r\n\tError in reading adc register!!\r\n");
		} else {
			reg_data = ad7124_register_map[reg_address].value;
			printf("\r\n\tRead Value: 0x%x", reg_data);
		}
	} else {
		printf("\r\n\tEnter the register data (in hex): ");
		reg_data = adi_get_hex_integer(sizeof(reg_data));

		ad7124_register_map[reg_address].value = reg_data;

		/* Write to ADC channel register */
		if ((reg_address >= AD7124_REG_NO)   ||
		    ((error_code = ad7124_write_register(pAd7124_dev,
				    ad7124_register_map[reg_address])) < 0)) {
			printf("\r\n\tError in writing adc register!!\r\n");
		} else {
			printf("\r\n\tWrite Successful...\r\n");
		}
	}

	adi_press_any_key_to_continue();
	adi_clear_console();

	return MENU_CONTINUE;
}


/* @brief  Enable or disable adc channels
 *
 * @param  uint32_t action- channel ENABLE/DISABLE action
 * @return menu status constant
 **/
static int32_t menu_channels_enable_disable(uint32_t action)
{
	char rx_char;               // received character from the serial port
	int32_t error_code;         // data read/write error code
	uint8_t current_channel;    // channel to be enabled

	do {
		/* Get the channel selection */
		current_channel = get_channel_selection();

		if (CHN_ENABLE == action) {
			/* Enable the selected channel */
			ad7124_register_map[AD7124_Channel_0 + current_channel].value |=
				AD7124_CH_MAP_REG_CH_ENABLE;
			printf("\tChannel %d is Enabled ", current_channel);
		} else {
			/* Disable the selected channel */
			ad7124_register_map[AD7124_Channel_0 + current_channel].value &=
				(~AD7124_CH_MAP_REG_CH_ENABLE);
			printf("\tChannel %d is Disabled ", current_channel);
		}

		/* Write to ADC channel register */
		if ((error_code = ad7124_write_register(pAd7124_dev,
							ad7124_register_map[AD7124_Channel_0 + current_channel]) < 0)) {
			printf("\tError in channel Enable/Disable!!\r\n");
			break;
		}

		printf("\r\n\r\n\tDo you want to continue (y/n)?: ");
		rx_char = toupper(getchar());

		if (rx_char != 'N' && rx_char != 'Y') {
			printf("Invalid entry!!\r\n");
		} else {
			/* Print the entered character back on console window (serial port) */
			printf("%c\r\n", rx_char);
		}

	} while (rx_char != 'N');
}


/* @brief  Assign setup to adc channel
 *
 * @param  uint8_t setup- setup to be assigned
 **/
static void assign_setup_to_channel(uint8_t setup)
{
	uint8_t current_channel;     // channel to be assigned with setup
	int32_t error_code;          // data read/write error code

	adi_clear_console();

	/* Get the channel selection */
	current_channel = get_channel_selection();

	/* Load the setup value */
	ad7124_register_map[AD7124_Channel_0 + current_channel].value =
		((ad7124_register_map[AD7124_Channel_0 + current_channel].value &
		  ~AD7124_CH_MAP_REG_SETUP_MSK) |
		 AD7124_CH_MAP_REG_SETUP(setup));

	if ((error_code = ad7124_write_register(pAd7124_dev,
						ad7124_register_map[AD7124_Channel_0 + current_channel])) < 0) {
		printf("\r\n\tError in setup assignment!!\r\n");
	} else {
		printf("\r\n\tSetup %d is assigned to channel %d successfully...\r\n", setup,
		       current_channel);
	}

	adi_press_any_key_to_continue();
}


/* @brief  Select adc channel to be assigned to setup
 *
 * @param  uint8_t current_setup- setup
 **/
static void select_chn_assignment(uint8_t current_setup)
{
	bool current_selection_done = false;
	char rx_char;

	do {
		printf("\r\n\r\n\tDo you want to assign setup to a channel (y/n)?: ");
		rx_char = toupper(getchar());

		if (rx_char == 'Y') {
			assign_setup_to_channel(current_setup);
			current_selection_done = true;
		} else if (rx_char == 'N') {
			current_selection_done = true;
		} else {
			printf("\r\n\tInvalid entry!!");
		}
	} while (current_selection_done == false);
}


/* @brief  Configure the setup and check if want to assign to channel
 * @param  id (Unused)
 * @return  int32_t- menu status constant
 **/
static int32_t menu_config_and_assign_setup(uint32_t id)
{
	uint8_t current_setup;    // current setup to be configured
	int32_t error_code;       // data read/write error

	adi_clear_console();

	/* Get the current setup selection */
	current_setup = get_setup_selection();

	/* Select the filter parameters */
	config_filter_parameters(&setup[current_setup]);

	/* Select the analog input parameters */
	config_analog_inputs(&setup[current_setup]);

	/* Select device gain */
	ad7124_register_map[AD7124_Config_0 + current_setup].value =
		((ad7124_register_map[AD7124_Config_0 + current_setup].value &
		  ~AD7124_CFG_REG_PGA_MSK) |
		 AD7124_CFG_REG_PGA(setup[current_setup].programmable_gain_bits));

	/* Select the polarity (bit 11) */
	if (setup[current_setup].polarity) {
		// Bipolar (1)
		ad7124_register_map[AD7124_Config_0 + current_setup].value |=
			AD7124_CFG_REG_BIPOLAR;
	} else {
		// Unipolar (0)
		ad7124_register_map[AD7124_Config_0 + current_setup].value &=
			(~AD7124_CFG_REG_BIPOLAR);
	}

	/* Enable/Disable analog inputs AINP & AINM buffers */
	if (setup[current_setup].input_buffers) {
		// Buffers enabled (1)
		ad7124_register_map[AD7124_Config_0 + current_setup].value |=
			(AD7124_CFG_REG_AIN_BUFP |
			 AD7124_CFG_REG_AINN_BUFM);
	} else {
		// Buffers disabled (0)
		ad7124_register_map[AD7124_Config_0 + current_setup].value &=
			(~AD7124_CFG_REG_AIN_BUFP &
			 ~AD7124_CFG_REG_AINN_BUFM);
	}

	/* Enable/Disable reference buffer */
	if (setup[current_setup].reference_buffers) {
		// Buffers enabled (1)
		ad7124_register_map[AD7124_Config_0 + current_setup].value |=
			(AD7124_CFG_REG_REF_BUFP |
			 AD7124_CFG_REG_REF_BUFM);
	} else {
		// Buffers disabled (0)
		ad7124_register_map[AD7124_Config_0 + current_setup].value &=
			(~AD7124_CFG_REG_REF_BUFP &
			 ~AD7124_CFG_REG_REF_BUFM);
	}

	/* Select the reference source */
	ad7124_register_map[AD7124_Config_0 + current_setup].value =
		((ad7124_register_map[AD7124_Config_0 + current_setup].value &
		  ~AD7124_CFG_REG_REF_SEL_MSK) |
		 AD7124_CFG_REG_REF_SEL(setup[current_setup].reference));


	/* Write to ADC config register */
	if ((error_code = ad7124_write_register(pAd7124_dev,
						ad7124_register_map[AD7124_Config_0 + current_setup])) < 0) {
		printf("\r\n\tError in configuring device setup!!\r\n");
		adi_press_any_key_to_continue();
		return MENU_CONTINUE;
	}

	/* Select filter type */
	ad7124_register_map[AD7124_Filter_0 + current_setup].value =
		((ad7124_register_map[AD7124_Filter_0 + current_setup].value &
		  ~AD7124_FILT_REG_FILTER_MSK) |
		 AD7124_FILT_REG_FILTER(setup[current_setup].filter));

	/* Set the data rate FS value */
	ad7124_register_map[AD7124_Filter_0 + current_setup].value =
		((ad7124_register_map[AD7124_Filter_0 + current_setup].value &
		  ~AD7124_FILT_REG_FS_MSK) |
		 AD7124_FILT_REG_FS(setup[current_setup].data_rate_fs_val));

	/* Write to ADC filter register */
	if ((error_code = ad7124_write_register(pAd7124_dev,
						ad7124_register_map[AD7124_Filter_0 + current_setup])) < 0) {
		printf("\r\n\tError in configuring device setup!!\r\n");
		adi_press_any_key_to_continue();
		return MENU_CONTINUE;
	}

	/* Print selections */
	printf("\r\n\r\n\tSetup %d is configured successfully =>\r\n", current_setup);
	printf("\r\n\tFilter Type: %s", filter_name[setup[current_setup].filter]);
	printf("\r\n\tData Rate: %f", filter_data_rate_raw);
	printf("\r\n\tGain: %u", gain_raw);
	printf("\r\n\tReference: %s", reference_name[setup[current_setup].reference]);
	printf("\r\n");

	select_chn_assignment(current_setup);

	return MENU_CONTINUE;
}


/* @brief  Connect analog inputs (AINP & AINM) to a channel
 * @param  id (Unused)
 * @return  int32_t- menu status constant
 **/
static int32_t menu_connect_input_to_channel(uint32_t id)
{
	uint32_t pos_analog_input;   // positive analog input
	uint32_t neg_analog_input;   // negative analog input
	uint32_t analog_input;       // user entered analog input
	uint8_t current_channel;     // current channel
	bool current_selection_done =
		false;  // flag checking if current menu selection is done
	int32_t error_code;          // data read/write error

	adi_clear_console();

	/* Get the channel selection */
	current_channel = get_channel_selection();

	/* Select analog inputs (positive and negative) */
	for (uint8_t index = 0; index < 2; index++) {
		current_selection_done = false;

		do {
			if (index == 0) {
				printf("\r\n\tEnter positive analog input- AINP <0-31>: ");
			} else {
				printf("\r\n\tEnter negative analog input- AINM <0-31>: ");
			}

			analog_input = adi_get_decimal_int(sizeof(analog_input));

			/* Validate channel selection */
			if (analog_input < MAX_ANALOG_INPUTS) {
				/* Break the loop by setting below flag */
				current_selection_done = true;

				if (index == 0) {
					pos_analog_input = analog_input;
				} else {
					neg_analog_input = analog_input;
				}
			} else {
				printf("\r\n\tInvalid analog input!!\r\n");
			}

		} while (current_selection_done == false);
	}

	/* Select positive analog input */
	ad7124_register_map[AD7124_Channel_0 + current_channel].value =
		((ad7124_register_map[AD7124_Channel_0 + current_channel].value &
		  ~AD7124_CH_MAP_REG_AINP_MSK) |
		 AD7124_CH_MAP_REG_AINP(pos_analog_input));

	/* Select negative analog input */
	ad7124_register_map[AD7124_Channel_0 + current_channel].value =
		((ad7124_register_map[AD7124_Channel_0 + current_channel].value &
		  ~AD7124_CH_MAP_REG_AINM_MSK) |
		 AD7124_CH_MAP_REG_AINM(neg_analog_input));

	/* Write to ADC channel register */
	if ((error_code = ad7124_write_register(pAd7124_dev,
						ad7124_register_map[AD7124_Channel_0 + current_channel])) < 0) {
		printf("\r\n\tError in analog input connection!!\r\n");
	} else {
		printf("\r\n\tAIN%d is connected to AINP and AIN%d is connectd to AINM for channel %d\r\n\r\n",
		       pos_analog_input,
		       neg_analog_input,
		       current_channel);
	}

	adi_press_any_key_to_continue();

	return MENU_CONTINUE;
}


/* @brief  display and handle console menu for calibrating adc
 * @param  id (Unused)
 * @return  int32_t- menu status constant
 **/
static int32_t menu_calibrate_adc(uint32_t id)
{
	int32_t adc_control_reg_val;
	int32_t cal_error;
	int32_t error_code;
	uint8_t chn_cnt;
	uint8_t channel_enable_mask = 0;

	adi_clear_console();

	// Save the ADC control register
	ad7124_read_register(pAd7124_dev, &ad7124_register_map[AD7124_ADC_Control]);
	adc_control_reg_val = ad7124_register_map[AD7124_ADC_Control].value;

	// Enable calibration error monitoring
	ad7124_register_map[AD7124_Error_En].value |= AD7124_ERREN_REG_ADC_CAL_ERR_EN;
	ad7124_write_register(pAd7124_dev, ad7124_register_map[AD7124_Error_En]);

	// Need to store which channels are enabled in this config so it can be restored
	for(chn_cnt = 0 ; chn_cnt < AD7124_MAX_CHANNELS ; chn_cnt++) {
		ad7124_read_register(pAd7124_dev,
				     &ad7124_register_map[AD7124_Channel_0 + chn_cnt]);
		if (ad7124_register_map[AD7124_Channel_0 + chn_cnt].value &
		    AD7124_CH_MAP_REG_CH_ENABLE) {
			channel_enable_mask |= (1 << chn_cnt);

			/* Disable the curent channel */
			ad7124_register_map[AD7124_Channel_0 + chn_cnt].value &=
				(~AD7124_CH_MAP_REG_CH_ENABLE);
			ad7124_write_register(pAd7124_dev,
					      ad7124_register_map[AD7124_Channel_0 + chn_cnt]);
		}
	}

	// Calibrate all the channels
	for (chn_cnt = 0 ; chn_cnt < AD7124_MAX_CHANNELS ; chn_cnt++) {
		printf("\r\n\tCalibrating Channel %d => \r\n", chn_cnt);

		// Enable current channel
		ad7124_register_map[AD7124_Channel_0 + chn_cnt].value |=
			AD7124_CH_MAP_REG_CH_ENABLE;
		ad7124_write_register(pAd7124_dev,
				      ad7124_register_map[AD7124_Channel_0 + chn_cnt]);

		// Write 0x800000 to offset register before full-scale calibration
		ad7124_register_map[AD7124_Offset_0 + chn_cnt].value = 0x800000;
		ad7124_write_register(pAd7124_dev,
				      ad7124_register_map[AD7124_Offset_0 + chn_cnt]);

		// Start full scale internal calibration (mode: 6)
		printf("\tRunning full-scale internal calibration...\r\n");
		ad7124_register_map[AD7124_ADC_Control].value =
			((ad7124_register_map[AD7124_ADC_Control].value & ~AD7124_ADC_CTRL_REG_MSK) | \
			 AD7124_ADC_CTRL_REG_MODE(6));
		ad7124_write_register(pAd7124_dev, ad7124_register_map[AD7124_ADC_Control]);

		/* Wait for calibration to over */
		if ((error_code = ad7124_wait_for_conv_ready(pAd7124_dev,
				  pAd7124_dev->spi_rdy_poll_cnt)) < 0) {
			printf("\tError in calibration...\r\n");
		} else {
			// Start zero scale internal calibration (mode: 5)
			printf("\tRunning zero-scale internal calibration...\r\n");
			ad7124_register_map[AD7124_ADC_Control].value =
				((ad7124_register_map[AD7124_ADC_Control].value & ~AD7124_ADC_CTRL_REG_MSK) | \
				 AD7124_ADC_CTRL_REG_MODE(5));
			ad7124_write_register(pAd7124_dev, ad7124_register_map[AD7124_ADC_Control]);

			// Wait for calibration to over
			if((error_code = ad7124_wait_for_conv_ready(pAd7124_dev,
					 pAd7124_dev->spi_rdy_poll_cnt)) < 0) {
				printf("\tError in calibration...\r\n");
			} else {
				// Check for any calibration error (bit 18 of AD7124_ERROR register)
				ad7124_read_register(pAd7124_dev, &ad7124_register_map[AD7124_Error]);
				cal_error = ((ad7124_register_map[AD7124_Error].value >> 18) & 0x01);

				if (!cal_error) {
					printf("\tCalibration Successful...\r\n");
				} else {
					printf("\tError in calibration...\r\n");
				}
			}
		}

		// Disable current channel
		ad7124_register_map[AD7124_Channel_0 + chn_cnt].value &=
			(~AD7124_CH_MAP_REG_CH_ENABLE);
		ad7124_write_register(pAd7124_dev,
				      ad7124_register_map[AD7124_Channel_0 + chn_cnt]);
	}

	// Need to restore the channels that were disabled during calibration
	for (chn_cnt = 0 ; chn_cnt < AD7124_MAX_CHANNELS ; chn_cnt++) {
		if (channel_enable_mask & (1 << chn_cnt)) {
			ad7124_register_map[AD7124_Channel_0 + chn_cnt].value |=
				AD7124_CH_MAP_REG_CH_ENABLE;
			ad7124_write_register(pAd7124_dev,
					      ad7124_register_map[AD7124_Channel_0 + chn_cnt]);
		}
	}

	// Write back previous value of control register
	ad7124_register_map[AD7124_ADC_Control].value = adc_control_reg_val;
	ad7124_write_register(pAd7124_dev, ad7124_register_map[AD7124_ADC_Control]);

	// Disable calibration error monitoring
	ad7124_register_map[AD7124_Error_En].value &=
		(~AD7124_ERREN_REG_ADC_CAL_ERR_EN);
	ad7124_write_register(pAd7124_dev, ad7124_register_map[AD7124_Error_En]);

	adi_press_any_key_to_continue();
	adi_clear_console();

	return MENU_CONTINUE;
}


/* @brief  Display the setup
 * @param  id (Unused)
 * @return int32_t- menu status constant
 **/
static int32_t menu_display_setup(uint32_t id)
{
	ad7124_setup_config device_setup;    // setup to be displayed
	float filter_data_rate;              // Filter data rate in SPS
	uint8_t setup_cnt;                   // setup counter
	uint8_t chn_cnt;                     // channel counter
	int32_t error_code;                  // data read/write error
	uint8_t power_mode_index;            // Index pointing to power mode string

	printf("\r\n\t---------------------------------------\r\n");
	printf("\r\n");

	/* Extract and print the power mode */
	(void)ad7124_read_register(pAd7124_dev,
				   &ad7124_register_map[AD7124_ADC_Control]);
	power_mode_index =
		AD7124_ADC_CTRL_REG_POWER_MODE_RD(
			ad7124_register_map[AD7124_ADC_Control].value);
	printf("\tPower Mode: %s\r\n", power_modes_str[power_mode_index]);

	printf("\r\n");
	printf("\t---------------------------------------\r\n");
	printf("\tChannel# | Status | Setup | AINP | AINM\r\n");
	printf("\t---------------------------------------\r\n");

	for (chn_cnt = 0; chn_cnt < AD7124_MAX_CHANNELS; chn_cnt++) {
		/* Read the channel register */
		if ((error_code = ad7124_read_register(pAd7124_dev,
						       &ad7124_register_map[AD7124_Channel_0 + chn_cnt])) < 0) {
			printf("\r\nError reading setup!!\r\n");
			break;
		}

		/* Extract the channel parameters from device register read value */

		device_setup.channel_enabled =
			AD7124_CH_MAP_REG_CH_ENABLE_RD(ad7124_register_map[AD7124_Channel_0 +
									chn_cnt].value);

		device_setup.setup_assigned =
			AD7124_CH_MAP_REG_SETUP_RD(ad7124_register_map[AD7124_Channel_0 +
								    chn_cnt].value);

		device_setup.pos_analog_input =
			AD7124_CH_MAP_REG_AINP_RD(ad7124_register_map[AD7124_Channel_0 +
								   chn_cnt].value);

		device_setup.neg_analog_input =
			AD7124_CH_MAP_REG_AINM_RD(ad7124_register_map[AD7124_Channel_0 +
								   chn_cnt].value);

		//      Channel# | Status | Setup | AINP | AINM
		printf("\t%4d %13s %4d %7d %6d\r\n",
		       chn_cnt,
		       enable_disable_status[device_setup.channel_enabled],
		       device_setup.setup_assigned,
		       device_setup.pos_analog_input,
		       device_setup.neg_analog_input);
	}

	printf("\r\n");
	printf("\t-----------------------------------------------------------------------------------------------------------\r\n");
	printf("\tSetup# | Filter Type | Data Rate | AIN_BUFP | AIN_BUFM | REF_BUFP | REF_BUFM | Polarity | Gain | REF SOURCE\r\n");
	printf("\t-----------------------------------------------------------------------------------------------------------\r\n");

	for (setup_cnt = 0; setup_cnt < AD7124_MAX_SETUPS; setup_cnt++) {
		/* Read the filter register */
		if ((error_code = ad7124_read_register(pAd7124_dev,
						       &ad7124_register_map[AD7124_Filter_0 + setup_cnt])) < 0) {
			printf("\r\nError reading setup!!\r\n");
			break;
		}

		/* Read the config register */
		if ((error_code = ad7124_read_register(pAd7124_dev,
						       &ad7124_register_map[AD7124_Config_0 + setup_cnt])) < 0) {
			printf("\r\nError reading setup!!\r\n");
			break;
		}

		/* Extract the setup parameters from device register read value */

		device_setup.filter =
			AD7124_FILT_REG_FILTER_RD(ad7124_register_map[AD7124_Filter_0 +
								  setup_cnt].value);

		device_setup.data_rate_fs_val =
			AD7124_FILT_REG_FS_RD(ad7124_register_map[AD7124_Filter_0 + setup_cnt].value);

		device_setup.input_buffers =
			(AD7124_CFG_REG_AIN_BUFP_RD(ad7124_register_map[AD7124_Config_0 +
								    setup_cnt].value) << 1) |
			(AD7124_CFG_REG_AINM_BUFP_RD(ad7124_register_map[AD7124_Config_0 +
								     setup_cnt].value));

		device_setup.reference_buffers =
			(AD7124_CFG_REG_REF_BUFP_RD(ad7124_register_map[AD7124_Config_0 +
								    setup_cnt].value) << 1) |
			(AD7124_CFG_REG_REF_BUFM_RD(ad7124_register_map[AD7124_Config_0 +
								    setup_cnt].value));

		device_setup.polarity =
			AD7124_CFG_REG_BIPOLAR_RD(ad7124_register_map[AD7124_Config_0 +
								  setup_cnt].value);

		device_setup.programmable_gain_bits =
			AD7124_CFG_REG_PGA_RD(ad7124_register_map[AD7124_Config_0 + setup_cnt].value);

		device_setup.reference =
			AD7124_CFG_REG_REF_SEL_RD(ad7124_register_map[AD7124_Config_0 +
								  setup_cnt].value);

		filter_data_rate = calculate_data_rate(device_setup.filter, power_mode,
						       (float)device_setup.data_rate_fs_val);

		//      Setup# | Filter Type | Data Rate | AIN_BUFP | AIN_BUFM | REF_BUFP | REF_BUFM | Polarity | Gain | REF
		printf("\t%4d %15s %10.2f %12s %10s %10s %10s %10s %5d %12s\r\n",
		       setup_cnt,
		       filter_name[device_setup.filter],
		       filter_data_rate,
		       enable_disable_status[(device_setup.input_buffers >> 1) & 0x01],
		       enable_disable_status[(device_setup.input_buffers & 0x01)],
		       enable_disable_status[(device_setup.reference_buffers >> 1) & 0x01],
		       enable_disable_status[device_setup.reference_buffers & 0x01],
		       polarity_status[device_setup.polarity],
		       p_gain[device_setup.programmable_gain_bits],
		       reference_name[device_setup.reference]);
	}

	adi_press_any_key_to_continue();

	return MENU_CONTINUE;
}


/* @brief  Configure the analog inputs for polarity and input buffers
 *
 * @param  ad7124_setup_config *psetup- pointer to setup to be assigned
 **/
static void config_analog_inputs(ad7124_setup_config *psetup)
{
	bool current_selection_done =
		false;   // flag checking if current menu selection is done
	uint32_t polarity;        // Polarity of ADC channel
	uint32_t input_buffers;   // Analog input buffers enable/disable status
	uint32_t ref_buffers;     // Reference buffers enable/disable status
	uint32_t reference_sel;   // Reference source selection

	/* Polarity selection */
	do {
		printf("\r\n\tSelect the polarity <0: Unipolar, 1: Bipolar>: ");
		polarity = adi_get_decimal_int(sizeof(polarity));

		/* Validate the range for polarity values */
		if (polarity <= 1) {
			psetup->polarity = (uint8_t)polarity;
			current_selection_done = true;
		} else {
			printf("\r\n\tPolarity out of range!!\r\n");
		}

	} while (current_selection_done == false) ;


	/* Input buffer enable/disable selection */
	current_selection_done = false;

	do {
		printf("\r\n\tEnable the AINP/M Buffers <0: Disable, 1: Enable>: ");
		input_buffers = adi_get_decimal_int(sizeof(input_buffers));

		/* Validate the range for input buffer values */
		if (input_buffers <= 1) {
			psetup->input_buffers = (uint8_t)input_buffers;
			current_selection_done = true;
		} else {
			printf("\r\n\tInvalid selection !!\r\n");
		}

	} while (current_selection_done == false);


	/* Reference buffer enable/disable selection */
	current_selection_done = false;

	do {
		printf("\r\n\tEnable the Reference Buffers <0: Disable, 1: Enable>: ");
		ref_buffers = adi_get_decimal_int(sizeof(ref_buffers));

		/* Validate the range for reference buffer values */
		if (ref_buffers <= 1) {
			psetup->reference_buffers = (uint8_t)ref_buffers;
			current_selection_done = true;
		} else {
			printf("\r\n\tInvalid selection !!\r\n");
		}

	} while (current_selection_done == false);


	/* Input buffer enable/disable selection */
	current_selection_done = false;

	do {
		printf("\r\n\tEnter the reference source: ");
		printf("\r\n\t[0] %s \r\n\t[1] %s \r\n\t[2] %s \r\n\t[3] %s \r\n\t",
		       reference_name[0],
		       reference_name[1],
		       reference_name[2],
		       reference_name[3]);

		reference_sel = adi_get_decimal_int(sizeof(reference_sel));

		/* Validate the range for reference sources (0:3) */
		if (reference_sel <= 3) {
			psetup->reference = (uint8_t)reference_sel;
			current_selection_done = true;
		} else {
			printf("\r\n\tInvalid selection !!\r\n");
		}

	} while (current_selection_done == false);
}


/* @brief  Calculate the data rate based on data rate FS value and vice a versa
 *
 * @param  filter_type filter- Filter Type
 * @param  uint8_t  power_mode- Power mode of device
 * @param  float data_rate- Actual Data Rate
 * @return Calculated data rate
 **/
static float calculate_data_rate(filter_type filter, uint8_t  power_mode,
				 float data_rate)
{
	float calc_data_rate = 120.0;  // default data rate

	// The data rate selection depends upon the power mode and device frequency.
	// fadc = Output data rate, fclk = master clock frequency
	// fclk = 614.4Khz (full power), 153.6Khz (mid power), 76.8Khz (low power)
	// fadc = 9.38SPS to 19200SPS (full power), 2.35SPS to 4800SPS (mid power),
	//        1.17SPS to 2400SPS for low power

	// Calculate FS value for SINC4 and SINC3 filter
	// FS[10:0] = fclk / (32 * fadc) OR fadc = fclk / (32 * FS[10:0])
	if (filter == SINC4_FILTER || filter == SINC3_FILTER) {
		if (power_mode == FULL_POWER_MODE) {
			calc_data_rate = (FUL_POWER_MODE_FREQUENCY / (data_rate * 32));
		} else if (power_mode == MED_POWER_MODE) {
			calc_data_rate = (MED_POWER_MODE_FREQUENCY / (data_rate * 32));
		} else {
			// Low power mode (default)
			calc_data_rate = (LOW_POWER_MODE_FREQUENCY / (data_rate * 32));
		}
	}

	// Calculate FS value for fast settling SINC4 filter
	// FS[10:0] = fclk / ((4+Avg-1) * 32 * fadc) Or fadc = fclk / ((4+Avg-1) * 32 * FS[10:0])
	// Avg = 16 for Full and Med power mode, 8 for low power mode
	if (filter == FAST_SETTLING_SINC4_FILTER) {
		if (power_mode == FULL_POWER_MODE) {
			calc_data_rate = (FUL_POWER_MODE_FREQUENCY / (19 * (data_rate * 32)));
		} else if (power_mode == MED_POWER_MODE) {
			calc_data_rate = (MED_POWER_MODE_FREQUENCY / (19 * (data_rate * 32)));
		} else {
			// Low power mode (default)
			calc_data_rate = (LOW_POWER_MODE_FREQUENCY / (11 * (data_rate * 32)));
		}
	}

	// Calculate FS value for fast settling SINC3 filter
	// FS[10:0] = fclk / ((3+Avg-1) * 32 * fadc) Or fadc = fclk / ((3+Avg-1) * 32 * FS[10:0])
	// Avg = 16 for Full and Med power mode, 8 for low power mode
	if (filter == FAST_SETTLING_SINC3_FILTER) {
		if (power_mode == FULL_POWER_MODE) {
			calc_data_rate = (FUL_POWER_MODE_FREQUENCY / (18 * (data_rate * 32)));
		} else if (power_mode == MED_POWER_MODE) {
			calc_data_rate = (MED_POWER_MODE_FREQUENCY / (18 * (data_rate * 32)));
		} else {
			// Low power mode (default)
			calc_data_rate = (LOW_POWER_MODE_FREQUENCY / (10 * (data_rate * 32)));
		}
	}

	return calc_data_rate;
}


/* @brief  Configure the filter parameters
 *
 * @param  ad7124_setup_config *psetup- pointer to setup to be assigned
 **/
static void config_filter_parameters(ad7124_setup_config *psetup)
{
	bool current_selection_done =
		false;    // flag checking if current menu selection is done
	uint32_t filter_type = NUM_OF_FILTERS;   // filter type selection
	uint16_t data_rate_fs = MAX_FILTER_DATA_RATE_FS +
				1;   // filter data rate FS value
	uint8_t gain_bits_value = 0;            // gain bits value

	do {
		printf("\r\n\tEnter the filter type selection: ");
		printf("\r\n\t[0] %s \r\n\t[1] %s \r\n\t[2] %s \r\n\t[3] %s \r\n\t",
		       filter_name[SINC4_FILTER],
		       filter_name[SINC3_FILTER],
		       filter_name[FAST_SETTLING_SINC4_FILTER],
		       filter_name[FAST_SETTLING_SINC3_FILTER]);

		filter_type = adi_get_decimal_int(sizeof(filter_type));

		/* Check for valid menu item selection (menu keys 0:3) */
		if (filter_type <= 3) {
			switch (filter_type) {
			case 0:
				psetup->filter = SINC4_FILTER;
				break;

			case 1:
				psetup->filter = SINC3_FILTER;
				break;

			case 2:
				psetup->filter = FAST_SETTLING_SINC4_FILTER;
				break;

			case 3:
				psetup->filter = FAST_SETTLING_SINC3_FILTER;
				break;

			default:
				psetup->filter = SINC4_FILTER;
				break;
			}

			current_selection_done = true;
		} else {
			printf("\r\n\tInvalid filter type selection!!\r\n");
		}

	} while (current_selection_done == false) ;


	/* Data rate selection */
	current_selection_done = false;

	/* Get the data rate for the selected filter except SINC3 Post filter, which
	 * has fixed data rates selectable from bits 19:17 of filter register */
	while (current_selection_done == false) {
		printf("\r\n\tEnter the filter Data Rate (in SPS): ");
		filter_data_rate_raw = adi_get_decimal_float(sizeof(filter_data_rate_raw) * 2);

		// Get the value and round off to nearest integer
		data_rate_fs = (uint16_t)(calculate_data_rate(psetup->filter, power_mode,
					  filter_data_rate_raw) + 0.5);

		/* Validate entered filter data range */
		if (data_rate_fs >= MIN_FILTER_DATA_RATE
		    && data_rate_fs <= MAX_FILTER_DATA_RATE_FS) {
			psetup->data_rate_fs_val = data_rate_fs;
			current_selection_done = true;
		} else {
			printf("\r\n\tData rate out of range!!\r\n");
		}
	}


	/* Select the gain factor for the filter */
	current_selection_done = false;

	do {
		printf("\r\n\tSelect the programmable gain <1-128>: ");
		gain_raw = adi_get_decimal_int(sizeof(gain_raw));

		// Get the gain bits value
		for(gain_bits_value = 0 ; gain_bits_value <= MAX_GAIN_BITS_VALUE ;
		    gain_bits_value++) {
			if (gain_raw == p_gain[gain_bits_value]) {
				psetup->programmable_gain_bits = gain_bits_value;
				current_selection_done = true;
				break;
			}
		}

		/* Validate the range for gain values */
		if (!current_selection_done) {
			printf("\r\n\tGain out of range!!\r\n");
		}

	} while (current_selection_done == false);
}


/* @brief  Get the channel selection
 *
 * @return  uint8- Selected channel
 **/
static uint8_t get_channel_selection(void)
{
	uint32_t current_channel = AD7124_MAX_CHANNELS;
	bool current_selection_done = false;

	do {
		printf("\r\n\tEnter Channel Value <0-15>: ");
		current_channel = adi_get_decimal_int(sizeof(current_channel));

		/* Validate channel selection */
		if (current_channel < AD7124_MAX_CHANNELS) {
			/* Break the loop by setting below flag */
			current_selection_done = true;
		} else {
			printf("\r\n\tInvalid channel selection!!\r\n");
		}

	} while (current_selection_done == false);

	return current_channel;
}


/* @brief  Get the setup selection
 *
 * @return  uint8- Selected setup
 **/
static uint8_t get_setup_selection(void)
{
	uint32_t current_setup = AD7124_MAX_SETUPS;
	bool current_selection_done = false;

	/* Setup selection */
	do {
		printf("\r\n\tEnter Setup Selection <0-7>: ");
		current_setup = adi_get_decimal_int(sizeof(current_setup));

		if (current_setup < AD7124_MAX_SETUPS) {
			current_selection_done = true;
		} else {
			printf("\r\n\tInvalid setup selection!!\r\n");
		}

	} while (current_selection_done == false);

	return current_setup;
}


/*
 * Definition of the power mode Menu Items and menu itself
 */
static console_menu_item power_mode_menu_items[] = {
	{ "Low Power Mode",  'L',  menu_power_modes_selection, NULL, (uint32_t)LOW_POWER_MODE  },
	{ "Med Power Mode",  'M',  menu_power_modes_selection, NULL, (uint32_t)MED_POWER_MODE  },
	{ "Full Power Mode", 'F',  menu_power_modes_selection, NULL, (uint32_t)FULL_POWER_MODE },
};

static console_menu power_mode_menu = {
	.title = "Power Mode Selection Menu",
	.items = power_mode_menu_items,
	.itemCount = ARRAY_SIZE(power_mode_menu_items),
	.headerItem = NULL,
	.footerItem = NULL,
	.enableEscapeKey = true
};

/*
 * Definition of the Sampling Menu Items and menu itself
 */
static console_menu_item acquisition_menu_items[] = {
	{ "Single Conversion Mode",                   'S',  menu_single_conversion },
	{ "Continuous Conversion Mode - Table View",  'T',  menu_continuous_conversion_tabular },
	{ "Continuous Conversion Mode - Stream Data", 'C',  menu_continuous_conversion_stream },
};

static console_menu acquisition_menu = {
	.title = "Data Acquisition Menu",
	.items = acquisition_menu_items,
	.itemCount = ARRAY_SIZE(acquisition_menu_items),
	.headerItem = NULL,
	.footerItem = NULL,
	.enableEscapeKey = true
};

/*
 * Definition of the channel enable/disable Menu Items and menu itself
 */
static console_menu_item chn_enable_disable_items[] = {
	{ "Enable Channels",  'E',  menu_channels_enable_disable, NULL, (uint32_t)CHN_ENABLE  },
	{ "Disable Channels", 'D',  menu_channels_enable_disable, NULL, (uint32_t)CHN_DISABLE },
};

static console_menu chn_enable_disable_menu = {
	.title = "Channel Enable/Disable Menu",
	.items = chn_enable_disable_items,
	.itemCount = ARRAY_SIZE(chn_enable_disable_items),
	.headerItem = NULL,
	.footerItem = NULL,
	.enableEscapeKey = true
};

/*
 * Definition of the adc register read/write Menu Items and menu itself
 */
static console_menu_item reg_read_write_items[] = {
	{ "Read Device Register",  'R',  menu_rw_ad7124_register, NULL, (uint32_t)DEVICE_REG_READ_ID  },
	{ "Write Device Register", 'W',  menu_rw_ad7124_register, NULL, (uint32_t)DEVICE_REG_WRITE_ID },
};

static console_menu reg_read_write_items_menu = {
	.title = "Register Read/Write Menu",
	.items = reg_read_write_items,
	.itemCount = ARRAY_SIZE(reg_read_write_items),
	.headerItem = NULL,
	.footerItem = NULL,
	.enableEscapeKey = true
};


/*!
 * @brief   displays and handles the Sample Channel menu
 * @param	id (Unused)
 * @return  int32_t- menu status constant
 */
static int32_t menu_sample_channels(uint32_t id)
{
	return (adi_do_console_menu(&acquisition_menu));
}


/* @brief  display and handle console menu for enabling/disabling adc channels
 * @param  id (Unused)
 * @return  int32_t- menu status constant
 **/
static int32_t menu_enable_disable_channels(uint32_t id)
{
	return (adi_do_console_menu(&chn_enable_disable_menu));
}


/* @brief  display and handle console menu for reading/writing adc registers
 * @param  id (Unused)
 * @return  int32_t- menu status constant
 **/
static int32_t menu_read_write_device_regs(uint32_t id)
{
	return (adi_do_console_menu(&reg_read_write_items_menu));
}

/*!
 * @brief   displays and handles the power mode select menu
 * @param   id (Unused)
 * @return  int32_t- menu status constant
 */
static int32_t menu_select_power_mode(uint32_t id)
{
	return (adi_do_console_menu(&power_mode_menu));
}


/*
 * Definition of the Main Menu Items and menu itself
 */
static console_menu_item main_menu_items[] = {
	{ "Reset to Default Configuration", 'A',   menu_reset },
	{ "Reset to Configuration A",       'B',   menu_reset_to_configuration, NULL,  (uint32_t)AD7124_CONFIG_A },
	{ "Reset to Configuration B",       'C',   menu_reset_to_configuration, NULL, (uint32_t)AD7124_CONFIG_B },
	{ "",                               '\00', NULL },
	{ "Read ID Register",               'D',   menu_read_id },
	{ "Read Status Register",           'E',   menu_read_status },
	{ "",                               '\00', NULL },
	{ "Sample Channels",                'F',   menu_sample_channels },
	{ "",                               '\00', NULL },
	{ "Select Power Mode",              'G',   menu_select_power_mode },
	{ "",                               '\00', NULL },
	{ "Enable/Disable Channels",        'H',   menu_enable_disable_channels },
	{ "",                               '\00', NULL },
	{ "Connect Analog Inputs to Channel", 'I', menu_connect_input_to_channel },
	{ "",                               '\00', NULL },
	{ "Configure and Assign Setup",     'J',   menu_config_and_assign_setup },
	{ "",                               '\00', NULL },
	{ "Display setup",                  'K',   menu_display_setup },
	{ "",                               '\00', NULL },
	{ "Read Temperature",               'L',   menu_read_temperature },
	{ "",                               '\00', NULL },
	{ "Calibrate ADC (Internal)",       'M',   menu_calibrate_adc },
	{ "",                               '\00', NULL },
	{ "Read/Write Device Registers",    'N',   menu_read_write_device_regs },
};

console_menu ad7124_main_menu = {
	.title = "AD7124 Main Menu",
	.items = main_menu_items,
	.itemCount = ARRAY_SIZE(main_menu_items),
	.headerItem = NULL,
	.footerItem = NULL,
	.enableEscapeKey = false
};
