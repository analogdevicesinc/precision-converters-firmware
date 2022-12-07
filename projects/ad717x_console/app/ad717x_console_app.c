/*!
 *****************************************************************************
  @file:  ad717x_console_app.c

  @brief: Implementation of the menu functions which handles the
          functionality of AD717x and AD411x family of devices.

  @details: This file is specific to AD717x/AD411x console menu application handle.
            The functions defined in this file performs the action
            based on user selected console menu.
 -----------------------------------------------------------------------------
 Copyright (c) 2020-2022 Analog Devices, Inc.
 All rights reserved.

 This software is proprietary to Analog Devices, Inc. and its licensors.
 By using this software you agree to the terms of the associated
 Analog Devices Software License Agreement.
*****************************************************************************/

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#include "app_config.h"

#include "ad717x.h"
#include "mbed_platform_support.h"
#include "no_os_spi.h"
#include "mbed_spi.h"

#include "ad717x_console_app.h"
#include "ad717x_menu_defines.h"
#include "ad717x_support.h"

/******************************************************************************/
/********************** Macros and Constants Definitions **********************/
/******************************************************************************/

// Include the device register address map headers and device register map based
// on the user selected device (default is AD4111)
#if defined(DEV_AD4111) || defined(DEV_AD4112) || defined(DEV_AD4114) || \
	defined(DEV_AD4115)
#include <ad411x_regs.h>
static ad717x_st_reg *ad717x_device_map = ad4111_regs;
static uint8_t ad717x_reg_count = sizeof(ad4111_regs) / sizeof(ad4111_regs[0]);
#elif defined(DEV_AD7172_2)
#include <ad7172_2_regs.h>
static ad717x_st_reg *ad717x_device_map = ad7172_2_regs;
static uint8_t ad717x_reg_count = sizeof(ad7172_2_regs) / sizeof(
		ad7172_2_regs[0]);
#elif defined(DEV_AD7172_4)
#include <ad7172_4_regs.h>
static ad717x_st_reg *ad717x_device_map = ad7172_4_regs;
static uint8_t ad717x_reg_count = sizeof(ad7172_4_regs) / sizeof(
		ad7172_4_regs[0]);
#elif defined(DEV_AD7173_8)
#include <ad7173_8_regs.h>
static ad717x_st_reg *ad717x_device_map = ad7173_8_regs;
static uint8_t ad717x_reg_count = sizeof(ad7173_8_regs) / sizeof(
		ad7173_8_regs[0]);
#elif defined(DEV_AD7175_2)
#include <ad7175_2_regs.h>
static ad717x_st_reg *ad717x_device_map = ad7175_2_regs;
static uint8_t ad717x_reg_count = sizeof(ad7175_2_regs) / sizeof(
		ad7175_2_regs[0]);
#elif defined(DEV_AD7175_8)
#include <ad7175_8_regs.h>
static ad717x_st_reg *ad717x_device_map = ad7175_8_regs;
static uint8_t ad717x_reg_count = sizeof(ad7175_8_regs) / sizeof(
		ad7175_8_regs[0]);
#elif defined(DEV_AD7176_2)
#include <ad7176_2_regs.h>
static ad717x_st_reg *ad717x_device_map = ad7176_2_regs;
static uint8_t ad717x_reg_count = sizeof(ad7176_2_regs) / sizeof(
		ad7176_2_regs[0]);
#else
#include <ad411x_regs.h>
static ad717x_st_reg *ad717x_device_map = ad4111_regs;
static uint8_t ad717x_reg_count = sizeof(ad4111_regs) / sizeof(ad4111_regs[0]);
#endif


#define SHOW_ALL_CHANNELS      false
#define SHOW_ENABLED_CHANNELS  true

#define DISPLAY_DATA_TABULAR    0
#define DISPLAY_DATA_STREAM     1

// Open wire detect ADC count threshold (eqv of 300mv for bipolar mode)
#define	OPEN_WIRE_DETECT_THRESHOLD	100000

/******************************************************************************/
/********************** Variables and User Defined Data Types *****************/
/******************************************************************************/

// Pointer to the struct representing the AD717x device
static ad717x_dev *pad717x_dev = NULL;

// Device setup
static ad717x_setup_config device_setup;

// User selected input (pair/positive/negative)
static uint8_t input_to_select;

// Last Sampled values for All ADC channels
static uint32_t channel_samples[NUMBER_OF_CHANNELS] = { 0 };

// How many times a given channel is sampled in total for one sample run
static uint32_t channel_samples_count[NUMBER_OF_CHANNELS] = { 0 };

// Variables used for open wire detection functionality
static uint32_t analog_input_type;	// Analog input type
static uint32_t channel_pair;		// Channel pair for open wire detection
static int32_t open_wire_detect_sample_data[2]; // Sampled data for channel pair

/******************************************************************************/
/************************ Functions Declarations ******************************/
/******************************************************************************/

static bool was_escape_key_pressed(void);

/******************************************************************************/
/************************ Functions Definitions *******************************/
/******************************************************************************/

/*!
 * @brief      Initialize the AD717x device and associated low level peripherals
 * @return     0 in case of success, negative error code otherwise
 */
int32_t ad717x_app_initialize(void)
{
	// Init SPI extra parameters structure
	struct mbed_spi_init_param spi_init_extra_params = {
		.spi_clk_pin = SPI_SCK,
		.spi_miso_pin = SPI_HOST_SDI,
		.spi_mosi_pin = SPI_HOST_SDO
	};

	// Used to create the ad717x device
	ad717x_init_param ad717x_init = {
		// spi_init_param type
		{
			.max_speed_hz = 2500000, 			// Max SPI Speed
			.chip_select = SPI_CSB,				// Chip Select pin
			.mode = NO_OS_SPI_MODE_3,			// CPOL = 1, CPHA =1
			.extra = &spi_init_extra_params, 	// SPI extra configurations
			.platform_ops = &mbed_spi_ops
		},
		ad717x_device_map,		// pointer to device register map
		ad717x_reg_count,		// number of device registers
	};

	// Initialze the device
	return (AD717X_Init(&pad717x_dev, ad717x_init));
}


/*!
 * @brief      determines if the Escape key was pressed
 * @return     key press status
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


/* @brief  Perform the channel selection
 * @return Selected channel
 **/
static uint8_t get_channel_selection(void)
{
	uint32_t current_channel = 0;
	bool current_selection_done = false;

	do {
		printf(EOL "\tEnter Channel Value <0-%d>: ", NUMBER_OF_CHANNELS-1);
		current_channel = adi_get_decimal_int(sizeof(current_channel));

		if (current_channel < NUMBER_OF_CHANNELS) {
			current_selection_done = true;
		} else {
			printf(EOL "\tInvalid channel selection!!" EOL);
		}
	} while (current_selection_done == false);

	return current_channel;
}


/* @brief  Perform the setup selection
 * @return Selected setup
 **/
static uint8_t get_setup_selection(void)
{
	uint32_t current_setup = 0;
	bool current_selection_done = false;

	do {
		printf(EOL "\tEnter Setup Selection <0-%d>: ", NUMBER_OF_SETUPS-1);
		current_setup = adi_get_decimal_int(sizeof(current_setup));

		if (current_setup < NUMBER_OF_SETUPS) {
			current_selection_done = true;
		} else {
			printf(EOL "\tInvalid setup selection!!" EOL);
		}
	} while (current_selection_done == false);

	return current_setup;
}


/* @brief  Assign setup to adc channel
 * @param  setup to be assigned
 **/
static void assign_setup_to_channel(uint8_t setup)
{
	uint8_t current_channel;       // channel to be assigned with setup
	ad717x_st_reg *device_chnmap_reg;	// pointer to channel map register

	adi_clear_console();

	// Get the channel selection
	current_channel = get_channel_selection();

	// Get the pointer to channel map register structure
	device_chnmap_reg = AD717X_GetReg(pad717x_dev,
					  AD717X_CHMAP0_REG + current_channel);

	// Load the setup value
	device_chnmap_reg->value =
		((device_chnmap_reg->value & ~AD717X_CHMAP_REG_SETUP_SEL_MSK) |
		 AD717X_CHMAP_REG_SETUP_SEL(setup));

	if (AD717X_WriteRegister(pad717x_dev,
				 AD717X_CHMAP0_REG + current_channel) != 0) {
		printf(EOL "\tError in setup assignment!!" EOL);
	} else {
		printf(EOL "\tSetup %d is assigned to channel %d successfully..." EOL,
		       setup,
		       current_channel);
	}

	adi_press_any_key_to_continue();
}


/* @brief  Select adc channel to be assigned to setup
 * @return  none
 **/
static char select_chn_assignment(void)
{
	bool current_selection_done = false;
	char rx_char;

	do {
		printf(EOL EOL "\tDo you want to assign setup to a channel (y/n)?: ");
		rx_char = toupper(getchar());

		if (rx_char == 'Y') {
			assign_setup_to_channel(device_setup.setup);
			current_selection_done = true;
		} else if (rx_char == 'N') {
			current_selection_done = true;
		} else {
			printf(EOL "\tInvalid entry!!");
		}
	} while (current_selection_done == false);

	return rx_char;
}


/*!
 * @brief      Display the header info for main menu
 * @return     None
 */
void display_main_menu_header(void)
{
	// Display the device name
	printf(EOL "\tDevice: %s" EOL, ACTIVE_DEVICE_NAME);
}


/*!
 * @brief      Handle the menu to read device ID
 * @param      menu_id- (Optional parameter)
 * @return     MENU_CONTINUE
 */
int32_t menu_read_id(uint32_t menu_id)
{
	ad717x_st_reg *device_id_reg;	// Pointer to register

	device_id_reg = AD717X_GetReg(pad717x_dev, AD717X_ID_REG);
	if (!device_id_reg) {
		printf(EOL EOL "\tError reading device ID!!" EOL);
	} else {
		if (AD717X_ReadRegister(pad717x_dev, AD717X_ID_REG) != 0) {
			printf(EOL EOL "\tError reading device ID!!" EOL);
		} else {
			printf(EOL EOL "\tDevice ID: 0x%lx" EOL, device_id_reg->value);
		}
	}

	adi_press_any_key_to_continue();
	return MENU_CONTINUE;
}


/*!
 * @brief      Handle the menu to read device status register
 * @param      menu_id- (Optional parameter)
 * @return     MENU_CONTINUE
 */
int32_t menu_read_status(uint32_t menu_id)
{
	ad717x_st_reg *device_status_reg; 	// Pointer to register

	device_status_reg = AD717X_GetReg(pad717x_dev, AD717X_STATUS_REG);
	if (!device_status_reg) {
		printf(EOL EOL "\tError reading status register!!" EOL);
	} else {
		if (AD717X_ReadRegister(pad717x_dev, AD717X_STATUS_REG) != 0) {
			printf(EOL EOL "\tError reading status register!!" EOL);
		} else {
			printf(EOL EOL "\tStatus Register: 0x%lx" EOL, device_status_reg->value);
		}
	}

	adi_press_any_key_to_continue();
	return MENU_CONTINUE;
}


/*
 * @brief helper function get the bipolar setting for an ADC channel
 *
 * @param dev The device structure.
 *
 * @param channel ADC channel to get bipolar mode for.
 *
 * @return value of bipolar field in the setup for an ADC channel.
 */
static bool ad717x_get_channel_bipolar(ad717x_dev *dev, uint8_t channel)
{
	ad717x_st_reg *device_setup_reg;    // Pointer to device setup register
	ad717x_st_reg *device_chnmap_reg;   // Pointer to device channelmap register
	uint8_t polarity;					// Polarity status flag
	uint8_t setup;						// Current setup

	device_chnmap_reg = AD717X_GetReg(pad717x_dev, AD717X_CHMAP0_REG + channel);
	(void)AD717X_ReadRegister(pad717x_dev, AD717X_CHMAP0_REG + channel);

	// Read the setup value for the current channel
	setup = AD717X_CHMAP_REG_SETUP_SEL_RD(device_chnmap_reg->value);

	device_setup_reg = AD717X_GetReg(pad717x_dev, AD717X_SETUPCON0_REG + setup);
	(void)AD717X_ReadRegister(pad717x_dev, AD717X_SETUPCON0_REG + setup);

	// Get the polarity bit for current setup
	polarity = AD717X_SETUP_CONF_REG_BI_UNIPOLAR_RD(device_setup_reg->value);

	if (polarity == BIPOLAR) {
		return true;
	} else {
		return false;
	}
}


/*
 * @brief converts ADC sample value to voltage based on gain setting
 *
 * @param dev The device structure.
 *
 * @param channel ADC channel to get Setup for.
 *
 * @param sample Raw ADC sample
 *
 * @return Sample ADC value converted to voltage.
 *
 * @note The conversion equation is implemented for simplicity,
 *       not for accuracy or performance
 *
 */
static float ad717x_convert_sample_to_voltage(ad717x_dev *dev,
		uint8_t channel,
		uint32_t sample)
{
	float converted_value;
	bool is_bipolar = ad717x_get_channel_bipolar(dev, channel);

	if (is_bipolar) {
		converted_value = (((float)sample / (1 << (ADC_RESOLUTION - 1))) - 1) *
				  ADC_REF_VOLTAGE;
	} else {
		converted_value = (((float)sample * ADC_REF_VOLTAGE) / (1 << ADC_RESOLUTION));
	}

	return (converted_value);
}


/*!
 * @brief      displays the current sample value for a ADC channels
 *
 * @param showOnlyEnabledChannels  only channels that are enabled are displayed
 *
 */
static void dislay_channel_samples(bool showOnlyEnabledChannels,
				   uint8_t console_mode)
{
	ad717x_st_reg *device_chnmap_reg;  	// Pointer to channel map register
	bool channel_printed = false;		// Channel print status flag

	switch (console_mode) {
	case DISPLAY_DATA_TABULAR:
		printf("\tCh\tValue\t\tCount\t\tVoltage" EOL);
		for (uint8_t chn = 0; chn < NUMBER_OF_CHANNELS; chn++) {
			// Get the pointer to channel register
			device_chnmap_reg = AD717X_GetReg(pad717x_dev, AD717X_CHMAP0_REG + chn);

			// if showing all channels, or channel is enabled
			if ((showOnlyEnabledChannels == false)
			    || (device_chnmap_reg->value & AD717X_CHMAP_REG_CH_EN)) {
				printf("\t%-2d\t%-10ld\t%ld\t\t% .6f" EOL,
				       chn,
				       channel_samples[chn],
				       channel_samples_count[chn],
				       ad717x_convert_sample_to_voltage(pad717x_dev, chn, channel_samples[chn]));
			}
		}
		break;

	case DISPLAY_DATA_STREAM:
		// Output a CSV list of the sampled channels as voltages on a single line
		for (uint8_t chn = 0 ; chn < NUMBER_OF_CHANNELS; chn++) {
			// Get the pointer to channel register
			device_chnmap_reg = AD717X_GetReg(pad717x_dev, AD717X_CHMAP0_REG + chn);

			// if showing all channels, or channel is enabled
			if ((showOnlyEnabledChannels == false) ||
			    (device_chnmap_reg->value & AD717X_CHMAP_REG_CH_EN)) {
				/*
					*  add the comma before we output the next channel but
					*  only if at least one channel has been printed
					*/
				if (channel_printed) {
					printf(", ");
				}

				printf("%.6f", ad717x_convert_sample_to_voltage(pad717x_dev, chn,
						channel_samples[chn]));

				channel_printed = true;
			}
		}
		printf(EOL);
		break;

	default:
		break;
	}
}


/*!
 * @brief      resets the channelSampleCounts to zero
 *
 * @details
 */
static void clear_channel_samples(void)
{
	for (uint8_t i = 0; i < NUMBER_OF_CHANNELS; i++) {
		channel_samples[i] = 0;
		channel_samples_count[i] = 0;
	}
}


/*!
 * @brief      Continuously acquires samples in Continuous Conversion mode
 *
 * @details   The ADC is run in continuous mode, and all samples are acquired
 *            and assigned to the channel they come from. Escape key an be used
 *            to exit the loop
 */
static int32_t do_continuous_conversion(uint8_t display_mode)
{
	int32_t error_code;
	int32_t sample_data;
	ad717x_st_reg *device_mode_reg;
	ad717x_st_reg *device_chnmap_reg;
	ad717x_st_reg *device_status_reg;

	// Get the pointer to mode register
	device_mode_reg = AD717X_GetReg(pad717x_dev, AD717X_ADCMODE_REG);

	// Clear the ADC CTRL MODE bits, has the effect of selecting continuous mode
	device_mode_reg->value &= ~(AD717X_ADCMODE_REG_MODE(0xf));

	if ((error_code = AD717X_WriteRegister(pad717x_dev,
					       AD717X_ADCMODE_REG)) != 0) {
		printf("Error (%ld) setting AD717x Continuous conversion mode." EOL,
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

		for (uint8_t chn = 0; chn < NUMBER_OF_CHANNELS; chn++) {
			// Get the pointer to channel register
			device_chnmap_reg = AD717X_GetReg(pad717x_dev, AD717X_CHMAP0_REG + chn);

			// if showing all channels, or channel is enabled
			if (device_chnmap_reg->value & AD717X_CHMAP_REG_CH_EN) {
				/*
				 *  add the comma before we output the next channel but
				 *  only if at least one channel has been printed
				 */
				if (channel_printed) {
					printf(", ");
				}
				printf("%d", chn);
			}
			channel_printed = true;
		}
		printf(EOL);
	}

	// Continuously read the channels, and store sample values
	while (was_escape_key_pressed() != true) {
		if (display_mode == DISPLAY_DATA_TABULAR) {
			adi_clear_console();
			printf("Running continuous conversion mode...\r\nPress Escape to stop" EOL EOL);
		}

		/*
		 *  this polls the status register READY/ bit to determine when conversion is done
		 *  this also ensures the STATUS register value is up to date and contains the
		 *  channel that was sampled as well.
		 *  Generally, no need to read STATUS separately, but for faster sampling
		 *  enabling the DATA_STATUS bit means that status is appended to ADC data read
		 *  so the channel being sampled is read back (and updated) as part of the same frame
		 */
		if ((error_code = AD717X_WaitForReady(pad717x_dev, 10000)) != 0) {
			printf("Error/Timeout waiting for conversion ready %ld" EOL EOL, error_code);
			continue;
		}

		if ((error_code = AD717X_ReadData(pad717x_dev, &sample_data)) != 0) {
			printf("Error reading ADC Data (%ld)." EOL, error_code);
			continue;
		}

		/*
		 * No error, need to process the sample, what channel has been read? update that channelSample
		 */
		device_status_reg = AD717X_GetReg(pad717x_dev, AD717X_STATUS_REG);
		uint8_t channelRead = device_status_reg->value & 0x0000000F;

		if (channelRead < NUMBER_OF_CHANNELS) {
			channel_samples[channelRead] = sample_data;
			channel_samples_count[channelRead]++;
		} else {
			printf("Channel Read was %d, which is not < %d" EOL,
			       channelRead,
			       NUMBER_OF_CHANNELS);
		}

		dislay_channel_samples(SHOW_ENABLED_CHANNELS, display_mode);
	}

	// All done, ADC put into standby mode
	device_mode_reg->value =
		((device_mode_reg->value & ~AD717X_ADCMODE_REG_MODE_MSK) |
		 AD717X_ADCMODE_REG_MODE(STANDBY_MODE));

	if ((error_code = AD717X_WriteRegister(pad717x_dev,
					       AD717X_ADCMODE_REG)) != 0) {
		printf("Error (%ld) setting ADC into standby mode." EOL, error_code);
		adi_press_any_key_to_continue();
	}

	return (MENU_CONTINUE);
}


/*!
 * @brief      Samples all enabled channels and displays in tabular form
 *
 * @details
 */
int32_t menu_continuous_conversion_tabular(uint32_t channel_id)
{
	do_continuous_conversion(DISPLAY_DATA_TABULAR);

	adi_clear_console();
	printf("Continuous Conversion completed..." EOL EOL);
	dislay_channel_samples(SHOW_ALL_CHANNELS, DISPLAY_DATA_TABULAR);
	adi_press_any_key_to_continue();

	return (MENU_CONTINUE);
}


/*!
 * @brief      Samples all enabled channels and displays on the console
 *
 * @details
 */
int32_t menu_continuous_conversion_stream(uint32_t channel_id)
{
	do_continuous_conversion(DISPLAY_DATA_STREAM);
	printf("Continuous Conversion completed..." EOL EOL);

	adi_press_any_key_to_continue();
	return (MENU_CONTINUE);
}


/*!
 * @brief      Samples all enabled channels once in Single Conversion mode
 *
 * @details    This stores all channels that are enabled in a bitmask, and then
 *             runs the ADC in single conversion mode, which acquires one channel
 *             of data at a time. After capture, that channel is disabled, and
 *             single conversion run again, until no channels are enabled.
 *             The original enable state of each channel is then restored.
 */
int32_t menu_single_conversion(uint32_t channel_id)
{
	int32_t    error_code;
	uint16_t   channel_enable_mask = 0;
	uint8_t    channel_count = 0;
	int32_t    sample_data;
	ad717x_st_reg *device_chnmap_reg;
	ad717x_st_reg *device_mode_reg;
	ad717x_st_reg *device_status_reg;

	// Need to store which channels are enabled in this config so it can be restored
	for (uint8_t chn = 0 ; chn < NUMBER_OF_CHANNELS; chn++) {
		// Get the pointer to channel register
		device_chnmap_reg = AD717X_GetReg(pad717x_dev, AD717X_CHMAP0_REG + chn);

		if (device_chnmap_reg->value & AD717X_CHMAP_REG_CH_EN) {
			channel_enable_mask |= (1 << chn);
			channel_count++;
		}
	}

	clear_channel_samples();

	adi_clear_console();
	printf("Running Single conversion mode...\r\nPress Escape to stop" EOL EOL);

	// Get the pointer to mode register
	device_mode_reg = AD717X_GetReg(pad717x_dev, AD717X_ADCMODE_REG);

	// Clear the ADC CTRL MODE bits, selecting continuous mode
	device_mode_reg->value =
		((device_mode_reg->value & ~AD717X_ADCMODE_REG_MODE_MSK) |
		 AD717X_ADCMODE_REG_MODE(CONTINUOUS_CONVERSION));

	// read the channels, and store sample values
	for(uint8_t loopCount = 0 ; ((was_escape_key_pressed() != true)
				     && (loopCount < channel_count)) ; loopCount++) {

		device_mode_reg->value =
			((device_mode_reg->value & ~AD717X_ADCMODE_REG_MODE_MSK) |
			 AD717X_ADCMODE_REG_MODE(SINGLE_CONVERISION));

		if ((error_code = AD717X_WriteRegister(pad717x_dev,
						       AD717X_ADCMODE_REG)) != 0) {
			printf("Error (%ld) setting AD717x Single conversion mode." EOL, error_code);
			adi_press_any_key_to_continue();
			continue;
		}

		/*
		 *  this polls the status register READY/ bit to determine when conversion is done
		 *  this also ensures the STATUS register value is up to date and contains the
		 *  channel that was sampled as well. No need to read STATUS separately
		 */
		if ((error_code = AD717X_WaitForReady(pad717x_dev, 10000)) != 0) {
			printf("Error/Timeout waiting for conversion ready %ld" EOL, error_code);
			continue;
		}

		if ((error_code = AD717X_ReadData(pad717x_dev, &sample_data)) != 0) {
			printf("Error reading ADC Data (%ld)." EOL, error_code);
			continue;
		}

		/*
		 * No error, need to process the sample, what channel has been read? update that channelSample
		 */
		device_status_reg = AD717X_GetReg(pad717x_dev, AD717X_STATUS_REG);
		uint8_t channelRead = device_status_reg->value & 0x0000000F;

		if (channelRead < NUMBER_OF_CHANNELS) {
			channel_samples[channelRead] = sample_data;
			channel_samples_count[channelRead]++;

			// Get the pointer to channel register
			device_chnmap_reg = AD717X_GetReg(pad717x_dev, AD717X_CHMAP0_REG + channelRead);

			/* also need to clear the channel enable bit so the next single conversion cycle will sample the next channel */
			device_chnmap_reg->value &= (~AD717X_CHMAP_REG_CH_EN);
			if ((error_code = AD717X_WriteRegister(pad717x_dev,
							       AD717X_CHMAP0_REG + channelRead)) != 0) {
				printf("Error (%ld) Clearing channel %d Enable bit." EOL,
				       error_code,
				       channelRead);

				adi_press_any_key_to_continue();
				continue;
			}
		} else {
			printf("Channel Read was %d, which is not < AD717x_CHANNEL_COUNT" EOL,
			       channelRead);
		}
	}

	// All done, ADC put into standby mode
	device_mode_reg->value =
		((device_mode_reg->value & ~AD717X_ADCMODE_REG_MODE_MSK) |
		 AD717X_ADCMODE_REG_MODE(STANDBY_MODE));

	// Need to restore the channels that were disabled during acquisition
	for(uint8_t chn = 0 ; chn < NUMBER_OF_CHANNELS ; chn++) {
		if (channel_enable_mask & (1 << chn)) {
			// Get the pointer to channel register
			device_chnmap_reg = AD717X_GetReg(pad717x_dev, AD717X_CHMAP0_REG + chn);

			device_chnmap_reg->value |= AD717X_CHMAP_REG_CH_EN;

			if ((error_code = AD717X_WriteRegister(pad717x_dev,
							       AD717X_CHMAP0_REG + chn)) != 0) {
				printf("Error (%ld) Setting channel %d Enable bit" EOL EOL, error_code, chn);

				adi_press_any_key_to_continue();
				return (MENU_CONTINUE);
			}
		}
	}

	printf("Single Conversion completed..." EOL EOL);
	dislay_channel_samples(SHOW_ENABLED_CHANNELS, DISPLAY_DATA_TABULAR);

	adi_press_any_key_to_continue();
	return (MENU_CONTINUE);
}


/*!
 * @brief      Handle the menu to sample the channels
 * @param      menu_id- (Optional parameter)
 * @return     MENU_CONTINUE
 */
int32_t menu_sample_channels(uint32_t menu_id)
{
	return adi_do_console_menu(&acquisition_menu);
}


/* @brief  Enable or disable adc channels
 * @param  channel ENABLE/DISABLE action
 * @return MENU_CONTINUE
 **/
int32_t menu_channels_enable_disable(uint32_t action)
{
	char rx_char;                // received character from the serial port
	uint8_t current_channel;     // channel to be enabled
	ad717x_st_reg *device_chnmap_reg; 	// Pointer to channel map register

	do {
		// Get the channel selection
		current_channel = get_channel_selection();

		// Get the pointer to channel register
		device_chnmap_reg = AD717X_GetReg(pad717x_dev,
						  AD717X_CHMAP0_REG + current_channel);

		if (action == SELECT_ENABLE) {
			// Enable the selected channel
			device_chnmap_reg->value |= AD717X_CHMAP_REG_CH_EN;
			printf("\tChannel %d is Enabled ", current_channel);
		} else {
			// Disable the selected channel
			device_chnmap_reg->value &= (~AD717X_CHMAP_REG_CH_EN);
			printf("\tChannel %d is Disabled ", current_channel);
		}

		// Write to ADC channel register
		if (AD717X_WriteRegister(pad717x_dev,
					 AD717X_CHMAP0_REG + current_channel) != 0) {
			printf("\tError in channel Enable/Disable!!" EOL);
			break;
		}

		printf(EOL EOL "\tDo you want to continue (y/n)?: ");
		rx_char = toupper(getchar());

		if ((rx_char != 'N') && (rx_char != 'Y')) {
			printf("Invalid entry!!" EOL);
		} else {
			// Print the entered character back on console window (serial port)
			printf("%c" EOL, rx_char);
		}
	} while (rx_char != 'N');

	return MENU_CONTINUE;
}


/*!
 * @brief      Display the menu to enable/disable channel selection
 * @param      menu_id- (Optional parameter)
 * @return     MENU_CONTINUE
 */
int32_t menu_chn_enable_disable_display(uint32_t menu_id)
{
	return adi_do_console_menu(&chn_enable_disable_menu);
}


/*!
 * @brief      Handle the menu to connect input to channel
 * @param      analog input to be connected
 * @return     MENU_CONTINUE
 */
int32_t menu_analog_input_connect(uint32_t user_analog_input)
{
	uint8_t current_channel;			// current channel
	ad717x_st_reg *device_chnmap_reg; 	// Pointer to channel map register

	adi_clear_console();

	// Get the channel selection
	current_channel = get_channel_selection();

	if (input_to_select == POS_ANALOG_INP_SELECT) {
		printf(EOL "\tSelect Positive Analog Input" EOL);
		device_setup.pos_analog_input = user_analog_input;
	} else if (input_to_select == NEG_ANALOG_INP_SELECT) {
		printf(EOL "\tSelect Negative Analog Input" EOL);
		device_setup.neg_analog_input = user_analog_input;
	} else {
		device_setup.pos_analog_input = AD717X_CHMAP_REG_AINPOS_RD(user_analog_input);
		device_setup.neg_analog_input = AD717X_CHMAP_REG_AINNEG_RD(user_analog_input);
	}

	// Get the pointer to channel map register structure
	device_chnmap_reg = AD717X_GetReg(pad717x_dev,
					  AD717X_CHMAP0_REG + current_channel);

#if defined(DEV_AD4111) || defined(DEV_AD4112) || defined(DEV_AD4114) || \
	defined(DEV_AD4115)
	// Select analog input pair
	device_chnmap_reg->value =
		((device_chnmap_reg->value & ~AD4111_CHMAP_REG_INPUT_MSK) |
		 AD4111_CHMAP_REG_INPUT(user_analog_input));
#else
	// Select positive analog input
	device_chnmap_reg->value =
		((device_chnmap_reg->value & ~AD717X_CHMAP_REG_AINPOS_MSK) |
		 AD717X_CHMAP_REG_AINPOS(device_setup.pos_analog_input));

	// Select negative analog input
	device_chnmap_reg->value =
		((device_chnmap_reg->value & ~AD717X_CHMAP_REG_AINNEG_MSK) |
		 AD717X_CHMAP_REG_AINNEG(device_setup.neg_analog_input));
#endif

	// Write to ADC channel register
	if (AD717X_WriteRegister(pad717x_dev,
				 AD717X_CHMAP0_REG + current_channel) != 0) {
		printf(EOL "\tError in analog input connection!!" EOL);
	} else {
		printf(EOL "\t%s is connected to INP+ and %s is connectd to INP- for channel %d"
		       EOL
		       EOL,
		       input_pin_map[device_setup.pos_analog_input],
		       input_pin_map[device_setup.neg_analog_input],
		       current_channel);
	}

	adi_press_any_key_to_continue();
	return MENU_CONTINUE;
}


/*!
 * @brief      Display the menu selections to connect analog input pins to a channel
 * @param      menu_id- (Optional parameter)
 * @return     MENU_CONTINUE
 */
int32_t menu_input_chn_connect_display(uint32_t menu_id)
{
#if defined(DEV_AD4111) || defined(DEV_AD4112) || defined(DEV_AD4114) || \
	defined(DEV_AD4115)
	input_to_select = ANALOG_INP_PAIR_SELECT;
	adi_do_console_menu(&analog_input_connect_menu);
#else
	input_to_select = POS_ANALOG_INP_SELECT;
	adi_do_console_menu(&analog_input_connect_menu);

	input_to_select = NEG_ANALOG_INP_SELECT;
	adi_do_console_menu(&analog_input_connect_menu);
#endif

	return MENU_CONTINUE;
}


/*!
 * @brief      Handle the menu to select the filter type
 * @param      user selected filter type
 * @return     MENU_DONE
 */
int32_t menu_filter_select(uint32_t user_input_filter_type)
{
	ad717x_st_reg *device_filter_config_reg;	// Pointer to filter config register

	// Get the pointer to filter config register structure
	device_filter_config_reg = AD717X_GetReg(pad717x_dev,
				   AD717X_FILTCON0_REG + device_setup.setup);

	device_setup.filter = user_input_filter_type;

	device_filter_config_reg->value =
		((device_filter_config_reg->value & ~AD717X_FILT_CONF_REG_ORDER_MSK) |
		 AD717X_FILT_CONF_REG_ORDER(device_setup.filter));

	if (device_setup.filter == SINC3_FILTER) {
		device_filter_config_reg->value |= AD717X_FILT_CONF_REG_SINC3_MAP;
	} else {
		device_filter_config_reg->value &= (~AD717X_FILT_CONF_REG_SINC3_MAP);
	}

	if (AD717X_WriteRegister(pad717x_dev,
				 AD717X_FILTCON0_REG + device_setup.setup) != 0) {
		printf(EOL "\tError in Filter Selection!!" EOL);
		adi_press_any_key_to_continue();
	}

	return MENU_DONE;
}


/*!
 * @brief      Handle the menu to enable/disable the post filter
 * @param      user selected action
 * @return     MENU_DONE
 */
int32_t menu_postfiler_enable_disable(uint32_t user_action)
{
	ad717x_st_reg *device_filter_config_reg;	// Pointer to filter config register

	// Get the pointer to filter config register structure
	device_filter_config_reg = AD717X_GetReg(pad717x_dev,
				   AD717X_FILTCON0_REG + device_setup.setup);

	if (user_action == SELECT_ENABLE) {
		device_setup.post_filter_enabled = SELECT_ENABLE;
		device_filter_config_reg->value |= AD717X_FILT_CONF_REG_ENHFILTEN;
	} else {
		device_setup.post_filter_enabled = SELECT_DISBLE;
		device_filter_config_reg->value &= (~AD717X_FILT_CONF_REG_ENHFILTEN);
	}

	if (AD717X_WriteRegister(pad717x_dev,
				 AD717X_FILTCON0_REG + device_setup.setup) != 0) {
		printf(EOL "\tError in Enabling/Disabling Postfilter!!" EOL);
		adi_press_any_key_to_continue();
	}

	return MENU_DONE;
}


/*!
 * @brief      Handle the menu to select the post filter
 * @param      user selected post filter type
 * @return     MENU_DONE
 */
int32_t menu_postfiler_select(uint32_t user_input_post_filter_type)
{
	ad717x_st_reg *device_filter_config_reg;	// Pointer to filter config register

	// Get the pointer to filter config register structure
	device_filter_config_reg = AD717X_GetReg(pad717x_dev,
				   AD717X_FILTCON0_REG + device_setup.setup);

	device_setup.postfilter = user_input_post_filter_type;

	device_filter_config_reg->value =
		((device_filter_config_reg->value & ~AD717X_FILT_CONF_REG_ENHFILT_MSK) |
		 AD717X_FILT_CONF_REG_ENHFILT(device_setup.postfilter));

	if (AD717X_WriteRegister(pad717x_dev,
				 AD717X_FILTCON0_REG + device_setup.setup) != 0) {
		printf(EOL "\tError in Post-Filter Selection!!" EOL);
		adi_press_any_key_to_continue();
	}

	return MENU_DONE;
}


/*!
 * @brief      Handle the menu to select the ODR value
 * @param      user selected ODR
 * @return     MENU_DONE
 */
int32_t menu_odr_select(uint32_t user_input_odr_val)
{
	ad717x_st_reg *device_filter_config_reg; 	// Pointer to filter config register

	// Get the pointer to filter config register structure
	device_filter_config_reg = AD717X_GetReg(pad717x_dev,
				   AD717X_FILTCON0_REG + device_setup.setup);

	device_setup.odr_bits = user_input_odr_val;

	device_filter_config_reg->value =
		((device_filter_config_reg->value & ~AD717X_FILT_CONF_REG_ODR_MSK) |
		 AD717X_FILT_CONF_REG_ODR(device_setup.odr_bits));

	if (AD717X_WriteRegister(pad717x_dev,
				 AD717X_FILTCON0_REG + device_setup.setup) != 0) {
		printf(EOL "\tError in ODR Selection!!" EOL);
		adi_press_any_key_to_continue();
	}

	return MENU_DONE;
}


/*!
 * @brief      Handle the menu to select the polarity
 * @param      user selected polarity
 * @return     MENU_DONE
 */
int32_t menu_polarity_select(uint32_t user_input_polarity)
{
	ad717x_st_reg *device_setup_control_reg;	// Pointer to setup control register

	// Get the pointer to setup control register structure
	device_setup_control_reg = AD717X_GetReg(pad717x_dev,
				   AD717X_SETUPCON0_REG + device_setup.setup);

	if (user_input_polarity == BIPOLAR) {
		device_setup.polarity = BIPOLAR;
		device_setup_control_reg->value |= AD717X_SETUP_CONF_REG_BI_UNIPOLAR;
	} else {
		device_setup.polarity = UNIPOLAR;
		device_setup_control_reg->value &= (~AD717X_SETUP_CONF_REG_BI_UNIPOLAR);
	}

	if (AD717X_WriteRegister(pad717x_dev,
				 AD717X_SETUPCON0_REG + device_setup.setup) != 0) {
		printf(EOL "\tError in Polarity Selection!!" EOL);
		adi_press_any_key_to_continue();
	}

	return MENU_DONE;
}


/*!
 * @brief      Handle the menu to select the reference source
 * @param      user selected reference source
 * @return     MENU_DONE
 */
int32_t menu_reference_source_select(uint32_t user_input_reference)
{
	ad717x_st_reg *device_setup_control_reg; 	// Pointer to setup control register
	ad717x_st_reg *device_mode_reg;				// Pointer to adc mode register

	device_setup.reference = user_input_reference;

	// Get the pointer to device mode register structure
	device_mode_reg = AD717X_GetReg(pad717x_dev, AD717X_ADCMODE_REG);

	if (device_setup.reference == INTERNAL) {
		device_mode_reg->value |= AD717X_ADCMODE_REG_REF_EN;
	} else {
		device_mode_reg->value &= (~AD717X_ADCMODE_REG_REF_EN);
	}

	if (AD717X_WriteRegister(pad717x_dev, AD717X_ADCMODE_REG) != 0) {
		printf(EOL "\tError in Polarity Selection!!" EOL);
		adi_press_any_key_to_continue();
		return MENU_CONTINUE;
	}

	// Get the pointer to setup control register structure
	device_setup_control_reg = AD717X_GetReg(pad717x_dev,
				   AD717X_SETUPCON0_REG + device_setup.setup);

	device_setup_control_reg->value =
		((device_setup_control_reg->value & ~AD717X_SETUP_CONF_REG_REF_SEL_MSK) |
		 AD717X_SETUP_CONF_REG_REF_SEL(device_setup.reference));

	if (AD717X_WriteRegister(pad717x_dev,
				 AD717X_SETUPCON0_REG + device_setup.setup) != 0) {
		printf(EOL "\tError in Polarity Selection!!" EOL);
		adi_press_any_key_to_continue();
	}

	return MENU_DONE;
}


/*!
 * @brief      Handle the menu to enable/disable the reference buffers
 * @param      user selected action
 * @return     MENU_DONE
 */
int32_t  menu_ref_buffer_enable_disable(uint32_t user_action)
{
	ad717x_st_reg *device_setup_control_reg; 	// Pointer to setup control register

	// Get the pointer to setup control register structure
	device_setup_control_reg = AD717X_GetReg(pad717x_dev,
				   AD717X_SETUPCON0_REG + device_setup.setup);

	device_setup.reference_buffers = user_action;

	if (user_action == SELECT_ENABLE) {
		// Enable ref buffers (+ve/-ve)
		device_setup_control_reg->value |=
			(AD717X_SETUP_CONF_REG_REFBUF_P |
			 AD717X_SETUP_CONF_REG_REFBUF_N);
	} else {
		// Disable ref buffers (+ve/-ve)
		device_setup_control_reg->value &=
			(~(AD717X_SETUP_CONF_REG_REFBUF_P |
			   AD717X_SETUP_CONF_REG_REFBUF_N));
	}

	if (AD717X_WriteRegister(pad717x_dev,
				 AD717X_SETUPCON0_REG + device_setup.setup) != 0) {
		printf(EOL "\tError in Reference Buffer Selection!!" EOL);
		adi_press_any_key_to_continue();
	}

	return MENU_DONE;
}


/*!
 * @brief      Handle the menu to enable/disable the input buffers
 * @param      user selected action
 * @return     MENU_DONE
 */
int32_t menu_input_buffer_enable_disable(uint32_t user_action)
{
	ad717x_st_reg *device_setup_control_reg;  	// Pointer to setup control register

	// Get the pointer to setup control register structure
	device_setup_control_reg = AD717X_GetReg(pad717x_dev,
				   AD717X_SETUPCON0_REG + device_setup.setup);

	device_setup.input_buffers = user_action;

	if (user_action == SELECT_ENABLE) {
		// Enable ref buffers (+ve/-ve)
		device_setup_control_reg->value |=
			(AD717X_SETUP_CONF_REG_AINBUF_P |
			 AD717X_SETUP_CONF_REG_AINBUF_N);
	} else {
		// Disable input buffers (+ve/-ve)
		device_setup_control_reg->value &=
			(~(AD717X_SETUP_CONF_REG_AINBUF_P |
			   AD717X_SETUP_CONF_REG_AINBUF_N));
	}

	if (AD717X_WriteRegister(pad717x_dev,
				 AD717X_SETUPCON0_REG + device_setup.setup) != 0) {
		printf(EOL "\tError in Reference Buffer Selection!!" EOL);
		adi_press_any_key_to_continue();
	}

	return MENU_DONE;
}


/*!
 * @brief      Handle the menu to configure and assign the device setup
 * @param      menu_id- (Optional parameter)
 * @return     MENU_CONTINUE
 */
int32_t menu_config_and_assign_setup(uint32_t menu_id)
{
	float filter_odr;	// filter ODR value
	char rx_char;

	adi_clear_console();

	// Get the current setup selection
	device_setup.setup = get_setup_selection();

	// Select the filter type
	adi_do_console_menu(&filter_select_menu);

	if (device_setup.filter == SINC5_SINC1_FILTER) {
		// Select the post filter parameters
		adi_do_console_menu(&postfilter_enable_disable_menu);

		if (device_setup.post_filter_enabled == SELECT_ENABLE) {
			// Select the post filter type
			adi_do_console_menu(&postfilter_select_menu);
		}

		// Select the SINC+SINC1 filter ODR
		adi_do_console_menu(&sinc5_1_data_rate_select_menu);
		filter_odr = sinc5_sinc1_odr_map[device_setup.odr_bits];
	} else {
		// Select the SINC3 filter ODR
		adi_do_console_menu(&sinc3_data_rate_select_menu);
		filter_odr = sinc3_odr_map[device_setup.odr_bits];
	}

	// Select the polarity
	adi_do_console_menu(&polarity_select_menu);

	// Select the reference source
	adi_do_console_menu(&reference_select_menu);

	// Select the reference buffer
	adi_do_console_menu(&ref_buffer_enable_disable_menu);

	// Select the input buffer
	adi_do_console_menu(&input_buffer_enable_disable_menu);

	// Print selections
	printf(EOL EOL "\tSetup %ld is configured successfully =>" EOL,
	       device_setup.setup);
	printf(EOL "\tFilter Type: %s", filter_name[device_setup.filter]);

	if (device_setup.filter == SINC5_SINC1_FILTER
	    && device_setup.post_filter_enabled) {
		printf("\r\n\tPost Filter Type: %s", postfilter_name[device_setup.postfilter]);
	}

	printf(EOL "\tData Rate: %f", filter_odr);
	printf(EOL "\tPolarity: %s", polarity_status[device_setup.polarity]);
	printf(EOL "\tReference: %s", reference_name[device_setup.reference]);
	printf(EOL "\tReference Buffers: %s",
	       enable_disable_status[device_setup.reference_buffers]);
	printf(EOL "\tInput Buffers: %s",
	       enable_disable_status[device_setup.input_buffers]);
	printf(EOL);

	/* Allow user to assign setup to multiple channels*/
	do {
		// Select and assign the channel
		rx_char = select_chn_assignment();
	} while (rx_char != 'N');

	return MENU_CONTINUE;
}


/* @brief  Get the data rate based on data rate FS value and vice a versa
 * @param  Filter Type
 * @param  Filter data rate register value/bits
 * @return Actual Data Rate
 **/
static float get_data_rate(uint32_t filter, uint32_t odr_reg_val)
{
	float data_rate;   // filter data rate

	if (filter == SINC5_SINC1_FILTER) {
		data_rate = sinc5_sinc1_odr_map[odr_reg_val];
	} else {
		data_rate = sinc3_odr_map[odr_reg_val];
	}

	return data_rate;
}


/*!
 * @brief      Handle the menu to display device setup
 * @param      menu_id- (Optional parameter)
 * @return     MENU_CONTINUE
 */
int32_t menu_display_setup(uint32_t menu_id)
{
	float filter_data_rate;				// Filter data rate in SPS
	uint8_t setup_cnt;					// setup counter
	uint8_t chn_cnt;					// channel counter
	ad717x_st_reg *device_chnmap_reg;	// Pointer to channel map register
	ad717x_st_reg *device_setupcon_reg; // Pointer to setup control register
	ad717x_st_reg *device_filtercon_reg; // Pointer to filter control register

	printf(EOL);
	printf("\t---------------------------------------" EOL);
	printf("\tChannel# | Status | Setup | INP0 | INP1" EOL);
	printf("\t---------------------------------------" EOL);

	for (chn_cnt = 0; chn_cnt < NUMBER_OF_CHANNELS; chn_cnt++) {
		// Get the pointer to channel register
		device_chnmap_reg = AD717X_GetReg(pad717x_dev, AD717X_CHMAP0_REG + chn_cnt);
		if (AD717X_ReadRegister(pad717x_dev, AD717X_CHMAP0_REG + chn_cnt) != 0) {
			printf(EOL "Error reading setup!!" EOL);
			break;
		}

		// Extract the channel parameters from device register read value
		device_setup.channel_enabled = AD717X_CHMAP_REG_CH_EN_RD(
						       device_chnmap_reg->value);
		device_setup.setup_assigned = AD717X_CHMAP_REG_SETUP_SEL_RD(
						      device_chnmap_reg->value);
		device_setup.pos_analog_input = AD717X_CHMAP_REG_AINPOS_RD(
							device_chnmap_reg->value);
		device_setup.neg_analog_input = AD717X_CHMAP_REG_AINNEG_RD(
							device_chnmap_reg->value);

		//      Channel# | Status | Setup | INP0 | INP1
		printf("\t%4d %13s %4ld %8s %6s" EOL,
		       chn_cnt,
		       enable_disable_status[device_setup.channel_enabled],
		       device_setup.setup_assigned,
		       input_pin_map[device_setup.pos_analog_input],
		       input_pin_map[device_setup.neg_analog_input]);
	}

	printf(EOL);
	printf("\t-------------------------------------------------------------------------------------------------------------"
	       EOL);
	printf("\tSetup# | Filter |   Post Filter   | Data Rate | INPBUF+ | INPBUF- | REFBUF+ | REFBUF- | Polarity | Ref Source"
	       EOL);
	printf("\t-------------------------------------------------------------------------------------------------------------"
	       EOL);

	for (setup_cnt = 0; setup_cnt < NUMBER_OF_SETUPS; setup_cnt++) {
		// Get the pointer to filter control register
		device_filtercon_reg = AD717X_GetReg(pad717x_dev,
						     AD717X_FILTCON0_REG + setup_cnt);
		if (AD717X_ReadRegister(pad717x_dev,
					AD717X_FILTCON0_REG + setup_cnt) != 0) {
			printf("\r\nError reading setup!!\r\n");
			break;
		}

		/* Extract the setup parameters from device register read value */
		device_setup.filter = AD717X_FILT_CONF_REG_ORDER_RD(
					      device_filtercon_reg->value);
		device_setup.odr_bits = AD717X_FILT_CONF_REG_ODR_RD(
						device_filtercon_reg->value);
		device_setup.post_filter_enabled = AD717X_FILT_CONF_REG_ENHFILTEN_RD(
				device_filtercon_reg->value);
		device_setup.postfilter = AD717X_FILT_CONF_REG_ENHFILT_RD(
						  device_filtercon_reg->value);

		if (device_setup.filter == SINC3_FILTER) {
			// Post filter unavailable for SINC3 type filter
			device_setup.post_filter_enabled = SELECT_DISBLE;
			device_setup.postfilter = POST_FILTER_NA;
		}

		// Get the pointer to setup control register
		device_setupcon_reg = AD717X_GetReg(pad717x_dev,
						    AD717X_SETUPCON0_REG + setup_cnt);
		if (AD717X_ReadRegister(pad717x_dev,
					AD717X_SETUPCON0_REG + setup_cnt) != 0) {
			printf("\r\nError reading setup!!\r\n");
			break;
		}

#if defined(DEV_AD4111) || defined(DEV_AD4112) || defined(DEV_AD4114) || \
	defined(DEV_AD4115)
		device_setup.input_buffers =
			AD4111_SETUP_CONF_REG_AIN_BUF_RD(device_setupcon_reg->value);
		device_setup.reference_buffers =
			(AD4111_SETUP_CONF_REG_REFPOS_BUF_RD(device_setupcon_reg->value) << 1 |
			 AD4111_SETUP_CONF_REG_REFNEG_BUF_RD(device_setupcon_reg->value));
#elif defined(DEV_AD7172_2) || defined(DEV_AD7172_4) || defined(DEV_AD7175_8)
		device_setup.input_buffers =
			(AD717X_SETUP_CONF_REG_AINBUF_P_RD(device_setupcon_reg->value) << 1) |
			(AD717X_SETUP_CONF_REG_AINBUF_N_RD(device_setupcon_reg->value));

		device_setup.reference_buffers =
			(AD717X_SETUP_CONF_REG_REFBUF_P_RD(device_setupcon_reg->value) << 1) |
			(AD717X_SETUP_CONF_REG_REFBUF_N_RD(device_setupcon_reg->value));
#elif defined(DEV_AD7173_8)
		device_setup.input_buffers =
			AD717X_SETUP_CONF_REG_AIN_BUF_RD(device_setupcon_reg->value);
		device_setup.reference_buffers =
			AD717X_SETUP_CONF_REG_REF_BUF_RD(device_setupcon_reg->value);
#endif

		device_setup.polarity = AD717X_SETUP_CONF_REG_BI_UNIPOLAR_RD(
						device_setupcon_reg->value);
		device_setup.reference = AD717X_SETUP_CONF_REG_REF_SEL_RD(
						 device_setupcon_reg->value);

		// Get the actual data rate based on the filter selection
		filter_data_rate = get_data_rate(device_setup.filter,
						 device_setup.odr_bits);

		//      Setup# | Filter | Post Filter | Data Rate | INPBUF+ | INPBUF- | REFBUF+ | REFBUF- | Polarity | Ref Source
		printf("\t%4d %11s %8s(%6s) %10.2f %9s %9s %9s %9s %10s %10s" EOL,
		       setup_cnt,
		       filter_name[device_setup.filter],
		       postfilter_name[device_setup.postfilter],
		       enable_disable_status[device_setup.post_filter_enabled],
		       filter_data_rate,
		       enable_disable_status[(device_setup.input_buffers >> 1) & 0x01],
		       enable_disable_status[(device_setup.input_buffers & 0x01)],
		       enable_disable_status[(device_setup.reference_buffers >> 1) & 0x01],
		       enable_disable_status[device_setup.reference_buffers & 0x01],
		       polarity_status[device_setup.polarity],
		       reference_name[device_setup.reference]);
	}

	adi_press_any_key_to_continue();
	return MENU_CONTINUE;
}


/*!
 * @brief      Handle the menu to read die temperature of device
 * @param      menu_id- (Optional parameter)
 * @return     MENU_CONTINUE
 */
int32_t menu_read_temperature(uint32_t menu_id)
{
	uint8_t chn_cnt;				// current channel
	uint16_t chn_mask = 0;			// channels enable mask
	ad717x_st_reg *device_mode_reg; // Pointer to device mode register
	ad717x_st_reg *device_chnmap_reg; 	// Pointer to channel map register
	ad717x_st_reg *device_setup_control_reg;	// Pointer to setup control register
	int32_t sample_data;			// ADC sample result
	float temperature = 0.00;		// Temperature in Celcius
	float conversion_result; 		// raw sample data to voltage conversion result
	int32_t prev_adc_reg_values[3];	// Holds the previous register values
	bool temperature_read_error;	// Temperature read error status

	temperature_read_error = false;

	// Disable the other enabled channels to read temperature from only 0th channel
	for (chn_cnt = 1; chn_cnt < NUMBER_OF_CHANNELS; chn_cnt++) {
		// Get the pointer to channel register
		device_chnmap_reg = AD717X_GetReg(pad717x_dev, AD717X_CHMAP0_REG + chn_cnt);
		if (AD717X_ReadRegister(pad717x_dev, AD717X_CHMAP0_REG + chn_cnt) != 0) {
			temperature_read_error = true;
			break;
		}

		if (device_chnmap_reg->value & AD717X_CHMAP_REG_CH_EN) {
			// Save enabled channel
			chn_mask |= (1 << chn_cnt);

			// Disable the curent channel
			device_chnmap_reg->value &= (~AD717X_CHMAP_REG_CH_EN);

			// Write to ADC channel register
			if (AD717X_WriteRegister(pad717x_dev, AD717X_CHMAP0_REG + chn_cnt) != 0) {
				temperature_read_error = true;
				break;
			}
		}
	}

	if (temperature_read_error == false) {
		// Get the pointer to device registers
		device_mode_reg = AD717X_GetReg(pad717x_dev, AD717X_ADCMODE_REG);
		device_chnmap_reg = AD717X_GetReg(pad717x_dev, AD717X_CHMAP0_REG);
		device_setup_control_reg = AD717X_GetReg(pad717x_dev, AD717X_SETUPCON0_REG);

		// Save the previous values of below registers in order to not disturb
		// the user configured setup.
		// *Note: This step is not required for someone who intended to just
		//        read temperature. It is an application specific functionality.
		prev_adc_reg_values[0] = device_mode_reg->value;
		prev_adc_reg_values[1] = device_chnmap_reg->value;
		prev_adc_reg_values[2] = device_setup_control_reg->value;

		// Configure the channel map 0 register
		// AINP = Temp + , AINM = Temp - , Setup = 0, CHN Enabled = True
		device_chnmap_reg->value =
			(AD717X_CHMAP_REG_AINPOS(TEMP_SENSOR_POS_INP_BITS) |
			 AD717X_CHMAP_REG_AINNEG(TEMP_SENSOR_NEG_INP_BITS) |
			 AD717X_CHMAP_REG_SETUP_SEL(0) |
			 AD717X_CHMAP_REG_CH_EN);

		if (AD717X_WriteRegister(pad717x_dev, AD717X_CHMAP0_REG) != 0) {
			printf(EOL EOL "\tError Reading Temperature!!");
			adi_press_any_key_to_continue();
			return MENU_CONTINUE;
		}

		// Configure the setup control 0 register
		// Polarity: Bipolar, Input Buffers: Enable, Ref: Internal
		device_setup_control_reg->value =
			(AD717X_SETUP_CONF_REG_BI_UNIPOLAR |
			 AD717X_SETUP_CONF_REG_AINBUF_P |
			 AD717X_SETUP_CONF_REG_AINBUF_N |
			 AD717X_SETUP_CONF_REG_REF_SEL(INTERNAL));

		if (AD717X_WriteRegister(pad717x_dev, AD717X_SETUPCON0_REG) != 0) {
			printf(EOL EOL "\tError Reading Temperature!!");
			adi_press_any_key_to_continue();
			return MENU_CONTINUE;
		}

		// Configure the device mode register
		// Internal Ref: Enable, Mode: Single Conversion
		device_mode_reg->value |= AD717X_ADCMODE_REG_REF_EN;
		device_mode_reg->value =
			((device_mode_reg->value & ~AD717X_ADCMODE_REG_MODE_MSK) |
			 AD717X_ADCMODE_REG_MODE(SINGLE_CONVERISION));

		if (AD717X_WriteRegister(pad717x_dev, AD717X_ADCMODE_REG) != 0) {
			printf(EOL EOL "\tError Reading Temperature!!");
			adi_press_any_key_to_continue();
			return MENU_CONTINUE;
		}

		for (uint8_t samples = 0; samples < 2; samples++) {
			// Wait for conversion to complete, then obtain sample
			AD717X_WaitForReady(pad717x_dev, 10000);

			if (AD717X_ReadData(pad717x_dev, &sample_data) != 0) {
				temperature_read_error = true;
				break;
			}
		}

		if (temperature_read_error == false) {
			conversion_result = (((float)sample_data / (1 << (ADC_RESOLUTION - 1))) - 1) *
					    ADC_REF_VOLTAGE;

			// Calculate the temparture in degree celcius (sensitivity: 477uV/K)
			// *Below equation is referred from the device datasheet
			temperature = ((float)conversion_result / 0.000477) - 273.15;

			// All done, restore previous state of device registers
			// *Note: This step is not required for someone who intended to just
			//        read temperature. It is an application specific functionality.
			device_mode_reg->value = prev_adc_reg_values[0];
			(void)AD717X_WriteRegister(pad717x_dev, AD717X_ADCMODE_REG);

			device_chnmap_reg->value = prev_adc_reg_values[1];
			(void)AD717X_WriteRegister(pad717x_dev, AD717X_CHMAP0_REG);

			device_setup_control_reg->value = prev_adc_reg_values[2];
			(void)AD717X_WriteRegister(pad717x_dev, AD717X_SETUPCON0_REG);

			// Need to restore the channels those were disabled during temperaure read
			for (uint8_t i = 0 ; i < NUMBER_OF_CHANNELS ; i++) {
				if (chn_mask & (1 << i)) {
					// Get the pointer to channel register
					device_chnmap_reg = AD717X_GetReg(pad717x_dev, AD717X_CHMAP0_REG + i);
					device_chnmap_reg->value |= AD717X_CHMAP_REG_CH_EN;

					if (AD717X_WriteRegister(pad717x_dev, AD717X_CHMAP0_REG + i) != 0) {
						// continue with other channels
					}
				}
			}
		}
	}

	if (temperature_read_error == false) {
		printf(EOL EOL "\tTemperature: %.2f Celcius", temperature);
	} else {
		printf(EOL EOL "\tError Reading Temperature!!");
	}

	adi_press_any_key_to_continue();
	return MENU_CONTINUE;
}


/*!
 * @brief      Handle the menu to calibrate the device
 * @param      menu_id- (Optional parameter)
 * @return     MENU_CONTINUE
 */
int32_t menu_calibrate_adc(uint32_t menu_id)
{
	uint8_t chn_cnt;					// Current channel
	uint16_t chn_mask = 0; 				// Channel enable mask
	ad717x_st_reg *device_chnmap_reg;  	// Pointer to channel map register
	ad717x_st_reg *device_mode_reg;		// Pointer to device mode register
	bool calibration_error;				// Calibration error flag

	calibration_error = false;

	for (chn_cnt = 0; chn_cnt < NUMBER_OF_CHANNELS; chn_cnt++) {
		// Get the pointer to channel register
		device_chnmap_reg = AD717X_GetReg(pad717x_dev, AD717X_CHMAP0_REG + chn_cnt);
		if (AD717X_ReadRegister(pad717x_dev, AD717X_CHMAP0_REG + chn_cnt) != 0) {
			calibration_error = true;
			break;
		}

		if (device_chnmap_reg->value & AD717X_CHMAP_REG_CH_EN) {
			// Save enabled channel
			chn_mask |= (1 << chn_cnt);

			// Disable the curent channel
			device_chnmap_reg->value &= (~AD717X_CHMAP_REG_CH_EN);

			if(AD717X_WriteRegister(pad717x_dev, AD717X_CHMAP0_REG + chn_cnt) != 0) {
				calibration_error = true;
				break;
			}
		}
	}

	if (calibration_error == false) {
		// Calibrate all the channels
		for (chn_cnt = 0 ; chn_cnt < NUMBER_OF_CHANNELS ; chn_cnt++) {
			printf(EOL "\tCalibrating Channel %d => " EOL, chn_cnt);

			// Enable current channel
			device_chnmap_reg = AD717X_GetReg(pad717x_dev, AD717X_CHMAP0_REG + chn_cnt);
			device_chnmap_reg->value |= AD717X_CHMAP_REG_CH_EN;

			if (AD717X_WriteRegister(pad717x_dev, AD717X_CHMAP0_REG + chn_cnt) != 0) {
				calibration_error = true;
				break;
			}

			device_mode_reg = AD717X_GetReg(pad717x_dev, AD717X_ADCMODE_REG);

			// Start full scale internal (gain) calibration
			printf("\tRunning full-scale internal calibration..." EOL);
			device_mode_reg->value =
				((device_mode_reg->value & ~AD717X_ADCMODE_REG_MODE_MSK) |
				 AD717X_ADCMODE_REG_MODE(INTERNAL_FULL_SCALE_CAL_MODE));

			if (AD717X_WriteRegister(pad717x_dev, AD717X_ADCMODE_REG) != 0) {
				calibration_error = true;
				break;
			}

			// Wait for calibration to over
			if (AD717X_WaitForReady(pad717x_dev, 10000) != 0) {
				calibration_error = true;
				break;
			} else {
				// Start zero scale internal (offset) calibration
				printf("\tRunning zero-scale internal calibration..." EOL);
				device_mode_reg->value =
					((device_mode_reg->value & ~AD717X_ADCMODE_REG_MODE_MSK) |
					 AD717X_ADCMODE_REG_MODE(INTERNAL_OFFSET_CAL_MODE));

				if (AD717X_WriteRegister(pad717x_dev, AD717X_ADCMODE_REG) != 0) {
					calibration_error = true;
					break;
				}

				// Wait for calibration to over
				if (AD717X_WaitForReady(pad717x_dev, 10000) != 0) {
					printf("\tError in channel calibration..." EOL);
				} else {
					printf("\tCalibration Successful..." EOL);
				}
			}

			// Disable current channel
			device_chnmap_reg->value &= (~AD717X_CHMAP_REG_CH_EN);

			if (AD717X_WriteRegister(pad717x_dev, AD717X_CHMAP0_REG + chn_cnt) != 0) {
				calibration_error = true;
				break;
			}
		}
	}

	// Need to restore the channels that were disabled during calibration
	for (chn_cnt = 0 ; chn_cnt < NUMBER_OF_CHANNELS ; chn_cnt++) {
		if (chn_mask & (1 << chn_cnt)) {
			// Get the pointer to channel register
			device_chnmap_reg = AD717X_GetReg(pad717x_dev, AD717X_CHMAP0_REG + chn_cnt);
			device_chnmap_reg->value |= AD717X_CHMAP_REG_CH_EN;

			if (AD717X_WriteRegister(pad717x_dev, AD717X_CHMAP0_REG + chn_cnt) != 0) {
				// continue with other channels
			}
		}
	}

	adi_press_any_key_to_continue();
	return MENU_CONTINUE;
}


/*!
 * @brief      Handle the menu to select input type for open wire detection
 * @param      User selected analog input type
 * @return     MENU_DONE
 */
int32_t menu_input_type_selection(uint32_t user_input_type)
{
	// Save the analog input type, for channel and input pair selections
	analog_input_type = user_input_type;

	return MENU_DONE;
}


/*!
 * @brief      Handle the menu to select channel pair for open wire detection
 * @param      User selected channel pair
 * @return     MENU_DONE
 */
int32_t menu_select_chn_pair(uint32_t user_channel_pair)
{
	// Save the channel pair for configuring channel map register
	channel_pair = user_channel_pair;

	return MENU_DONE;
}


/*!
 * @brief      Handle the menu to select input pair for open wire detection
 * @param      User selected analog input
 * @return     MENU_DONE
 */
int32_t menu_select_input_pair(uint32_t user_analog_input)
{
	uint8_t current_channel; 			// current channel
	ad717x_st_reg *device_chnmap_reg;  	// pointer to channel map register
	ad717x_st_reg *device_mode_reg; 	// pointer to device mode register
	bool input_pair_select_error = false;	// Error in input pair selection

	if (input_pair_select_error == false) {
		// Set the configurations for channel pair
		for (uint8_t chn_cnt = 0; chn_cnt < 2; chn_cnt++) {
			if (chn_cnt == 0) {
				// Get the first channel from the pair
				current_channel = (channel_pair >> CHN_PAIR_OFFSET);
			} else {
				// Get the second channel from the pair
				current_channel = (channel_pair & CHN_PAIR_MASK);
			}

			// Get the pointer to channel map register structure
			device_chnmap_reg = AD717X_GetReg(pad717x_dev,
							  AD717X_CHMAP0_REG + current_channel);

			// Load the setup 0 value
			device_chnmap_reg->value =
				((device_chnmap_reg->value & ~AD717X_CHMAP_REG_SETUP_SEL_MSK) |
				 AD717X_CHMAP_REG_SETUP_SEL(0));

			// Select the analog input
			device_chnmap_reg->value =
				((device_chnmap_reg->value & ~AD4111_CHMAP_REG_INPUT_MSK) |
				 AD4111_CHMAP_REG_INPUT(user_analog_input));

			// Enable the channel
			device_chnmap_reg->value |= AD717X_CHMAP_REG_CH_EN;

			device_mode_reg = AD717X_GetReg(pad717x_dev, AD717X_ADCMODE_REG);
			device_mode_reg->value =
				((device_mode_reg->value & ~AD717X_ADCMODE_REG_MODE_MSK) |
				 AD717X_ADCMODE_REG_MODE(SINGLE_CONVERISION));

			if (AD717X_WriteRegister(pad717x_dev, AD717X_ADCMODE_REG) != 0) {
				input_pair_select_error = true;
			}

			if (AD717X_WriteRegister(pad717x_dev,
						 AD717X_CHMAP0_REG + current_channel) != 0) {
				input_pair_select_error = true;
				break;
			}

			if (AD717X_WaitForReady(pad717x_dev, 10000) != 0) {
				input_pair_select_error = true;
				break;
			}

			if (AD717X_ReadData(pad717x_dev,
					    &open_wire_detect_sample_data[chn_cnt]) != 0) {
				input_pair_select_error = true;
				break;
			}

			// Disable the current channel
			device_chnmap_reg->value &= (~AD717X_CHMAP_REG_CH_EN);

			if (AD717X_WriteRegister(pad717x_dev,
						 AD717X_CHMAP0_REG + current_channel) != 0) {
				input_pair_select_error = true;
				break;
			}
		}
	}

	if (input_pair_select_error == true) {
		printf("\tError in analog input selection!!" EOL);
		adi_press_any_key_to_continue();
	}

	return MENU_DONE;
}


/*!
 * @brief      Handle the menu to perform open wire detection
 * @param      menu_id- (Optional parameter)
 * @return     MENU_CONTINUE
 */
int32_t menu_open_wire_detection(uint32_t menu_id)
{
	ad717x_st_reg *device_chnmap_reg;	// pointer to channel map register
	ad717x_st_reg *device_gpiocon_reg;	// pointer to gpio control register
	ad717x_st_reg *device_setupcon_reg; // pointer to setup control register
	uint8_t chn_cnt;					// Current channel count
	uint8_t chn_mask = 0;				// Channel enable/disable mask
	bool open_wire_detection_error = false; // Open wire detection error flag

	// Disable all the enabled channels before starting open wire detection
	for (chn_cnt = 0; chn_cnt < NUMBER_OF_CHANNELS; chn_cnt++) {
		device_chnmap_reg = AD717X_GetReg(pad717x_dev, AD717X_CHMAP0_REG + chn_cnt);

		if (AD717X_ReadRegister(pad717x_dev, AD717X_CHMAP0_REG + chn_cnt) != 0) {
			open_wire_detection_error = true;
			break;
		}

		if (device_chnmap_reg->value & AD717X_CHMAP_REG_CH_EN) {
			// Save enabled channel
			chn_mask |= (1 << chn_cnt);

			// Disable the curent channel
			device_chnmap_reg->value &= (~AD717X_CHMAP_REG_CH_EN);

			// Write to ADC channel register
			if (AD717X_WriteRegister(pad717x_dev, AD717X_CHMAP0_REG + chn_cnt) != 0) {
				open_wire_detection_error = true;
				break;
			}
		}
	}

	if (open_wire_detection_error == false) {
		do {
			// Enable the open wire detection on voltage channels
			device_gpiocon_reg = AD717X_GetReg(pad717x_dev, AD717X_GPIOCON_REG);

			if (AD717X_ReadRegister(pad717x_dev, AD717X_GPIOCON_REG) == 0) {
				device_gpiocon_reg->value |= (AD4111_GPIOCON_REG_OP_EN0_1 |
							      AD4111_GPIOCON_REG_OW_EN);

				if (AD717X_WriteRegister(pad717x_dev, AD717X_GPIOCON_REG) != 0) {
					open_wire_detection_error = true;
					break;
				}
			} else {
				break;
			}

			// Configure the setup control 0 register
			device_setupcon_reg = AD717X_GetReg(pad717x_dev, AD717X_SETUPCON0_REG);

			if (AD717X_ReadRegister(pad717x_dev, AD717X_SETUPCON0_REG) == 0) {
				device_setupcon_reg->value |= (AD717X_SETUP_CONF_REG_AINBUF_P |
							       AD717X_SETUP_CONF_REG_AINBUF_N |
							       AD717X_SETUP_CONF_REG_BI_UNIPOLAR);

				device_setupcon_reg->value = ((device_setupcon_reg->value &
							       ~AD717X_SETUP_CONF_REG_REF_SEL_MSK) |
							      AD717X_SETUP_CONF_REG_REF_SEL(EXTERNAL));

				if (AD717X_WriteRegister(pad717x_dev, AD717X_SETUPCON0_REG) != 0) {
					open_wire_detection_error = true;
					break;
				}
			} else {
				break;
			}
		} while (0);


		if (open_wire_detection_error == false) {
			// Select the analog input type
			adi_do_console_menu(&open_wire_detect_input_type_menu);

			if (analog_input_type == SINGLE_ENDED_INPUT) {
				// Select the single ended input channel pair
				adi_do_console_menu(&open_wire_detect_se_channel_menu);

				// Select the single ended analog input pair
				adi_do_console_menu(&open_wire_detect_se_analog_input_menu);
			} else {
				// Select the differential ended input channel pair
				adi_do_console_menu(&open_wire_detect_de_channel_menu);

				// Select the differential ended analog input pair
				adi_do_console_menu(&open_wire_detect_de_analog_input_menu);
			}

			printf(EOL "\tChannel %d = %d" EOL, (channel_pair >> CHN_PAIR_OFFSET),
			       open_wire_detect_sample_data[0]);

			printf(EOL "\tChannel %d = %d" EOL, (channel_pair & CHN_PAIR_MASK),
			       open_wire_detect_sample_data[1]);

			// Check for open wire detection by measuring offset of channel pair sampled data
			if (abs(open_wire_detect_sample_data[0] - open_wire_detect_sample_data[1]) >
			    OPEN_WIRE_DETECT_THRESHOLD) {
				printf(EOL "\tOpen Wire Detected on Selected Input Pair!!" EOL);
			} else {
				printf(EOL "\tNo Open Wire Detected on Selected Input Pair..." EOL);
			}
		} else {
			printf(EOL "\tError in Open Wire Detection!!" EOL);
		}
	} else {
		printf(EOL "\tError in Open Wire Detection!!" EOL);
	}

	// Need to restore the channels that were disabled during open wire detection
	for (chn_cnt = 0 ; chn_cnt < NUMBER_OF_CHANNELS ; chn_cnt++) {
		if (chn_mask & (1 << chn_cnt)) {
			// Get the pointer to channel register
			device_chnmap_reg = AD717X_GetReg(pad717x_dev, AD717X_CHMAP0_REG + chn_cnt);

			device_chnmap_reg->value |= AD717X_CHMAP_REG_CH_EN;

			if (AD717X_WriteRegister(pad717x_dev, AD717X_CHMAP0_REG + chn_cnt) != 0) {
				continue;
			}
		}
	}

	adi_press_any_key_to_continue();
	return MENU_CONTINUE;
}


/* @brief  Console menu to read/write the adc register
 * @param  Read/Write ID/Operation
 * @return MENU_CONTINUE
 **/
int32_t menu_rw_ad717x_register(uint32_t rw_id)
{
	uint32_t reg_address;
	uint32_t reg_data;
	ad717x_st_reg *device_data_reg;  // Pointer to data register

	printf(EOL "\tEnter the register address (in hex): ");
	reg_address = adi_get_hex_integer(sizeof(reg_address));

	// Get the pointer to data register
	device_data_reg = AD717X_GetReg(pad717x_dev, reg_address);

	if ((uint32_t)DEVICE_REG_READ_ID == rw_id) {
		// Read from ADC channel register
		if (AD717X_ReadRegister(pad717x_dev, reg_address) != 0) {
			printf(EOL "Error reading setup!!" EOL);
		} else {
			reg_data = device_data_reg->value;
			printf(EOL "\tRead Value: 0x%lx", reg_data);
		}
	} else {
		printf(EOL "\tEnter the register data (in hex): ");
		reg_data = adi_get_hex_integer(sizeof(reg_data));

		device_data_reg->value = reg_data;

		if (AD717X_WriteRegister(pad717x_dev, reg_address) != 0) {
			printf(EOL "\tError in writing adc register!!" EOL);
		} else {
			printf(EOL "\tWrite Successful..." EOL);
		}
	}

	adi_press_any_key_to_continue();
	return MENU_CONTINUE;
}


/*!
 * @brief      Handle the menu to read/write device registers
 * @param      menu_id- (Optional parameter)
 * @return     MENU_CONTINUE
 */
int32_t menu_read_write_device_regs(uint32_t menu_id)
{
	return (adi_do_console_menu(&reg_read_write_menu));
}
