/*!
 *****************************************************************************
  @file:  nanodac_console_app.c

  @brief: Implementation for the menu functions that handles the nanodac
          functionality

  @details: This file is specific to nanodac console menu application handle.
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

#include "app_config.h"

#include "ad5686.h"
#include "nanodac_console_app.h"
#include "no_os_error.h"
#include "no_os_delay.h"
#include "no_os_i2c.h"
#include "no_os_spi.h"
#include "mbed_platform_support.h"
#include "mbed_gpio.h"
#include "mbed_i2c.h"
#include "mbed_spi.h"

/******************************************************************************/
/********************** Macros and Constants Definitions **********************/
/******************************************************************************/

#if !defined(I2C_SLAVE_ADDRESS)
#define I2C_SLAVE_ADDRESS 0		// For non I2C devices
#endif

#define INTERNAL_VREF_VOLTAGE	2.5
#define	INTERNAL_VREF_SOURCE	0U
#define	EXTERNAL_VREF_SOURCE	1U

#define GAIN_LOW				1
#define GAIN_HIGH				2

/******************************************************************************/
/********************** Variables and User Defined Data Types *****************/
/******************************************************************************/

// Pointer to a nanodac device structure
static struct ad5686_dev *nanodac_dev = NULL;

// Current DAC channel (default value is channel 0)
static enum ad5686_dac_channels current_dac_channel = AD5686_CH_0;

// Define the Vref source and voltage
#if !defined(EXT_VREF_SOURCE_ONLY)
// Default Vref voltage is 2.5v for internal reference source
static uint32_t vref_source = INTERNAL_VREF_SOURCE;
static float vref_voltage = INTERNAL_VREF_VOLTAGE;
#else
// Vref voltage for external reference source is user selectable
static uint32_t vref_source = EXTERNAL_VREF_SOURCE;
static float vref_voltage = 0;
#endif

// Gain value (default is 1)
static uint32_t gain = GAIN_LOW;

// LDAC pin state (default is High/Vlogic)
static uint32_t ldac_pin_state = NO_OS_GPIO_HIGH;

// LDAC mask status (default is False/Disable)
static bool ldac_mask_status = false;


// Vref sources string
static const char *vref_source_str[] = {
	"Internal",
	"External"
};

// Operating mode string
static const char *operating_mode_str[] = {
	"Normal Power-Up",
	"1K to GND",
	"100K to GND",
	"Three State"
};

// Menu pre-declarations
extern console_menu dac_channel_select_menu;
extern console_menu vref_select_menu;
extern console_menu gain_select_menu;
extern console_menu dac_readback_select_menu;
extern console_menu ldac_mask_select_menu;
extern console_menu operating_mode_select_menu;
extern console_menu ldac_pin_select_menu;

/******************************************************************************/
/************************ Functions Declarations ******************************/
/******************************************************************************/

static int32_t gpio_power_up_configuration(void);

/******************************************************************************/
/************************ Functions Definitions *******************************/
/******************************************************************************/

/* @brief     Initialize the nanodac device
 * @param     none
 * @return    0 in case of success, negative error code otherwise
 **/
int32_t nanodac_app_initialize(void)
{
	int32_t device_init_status;	// Init status of device

	// Initialize the extra parameters for I2C initialization
	struct mbed_i2c_init_param i2c_init_extra_params = {
		.i2c_sda_pin = I2C_SDA,
		.i2c_scl_pin = I2C_SCL
	};

	// Initialize the extra parameters for SPI initialization
	struct mbed_spi_init_param spi_init_extra_params = {
		.spi_clk_pin = SPI_SCK,
		.spi_miso_pin = SPI_HOST_SDI,
		.spi_mosi_pin = SPI_HOST_SDO,
		.use_sw_csb = false
	};

	struct mbed_gpio_init_param reset_gpio_extra_init_params = {
		.pin_mode = 0	// NA
	};

	struct mbed_gpio_init_param ldac_gpio_extra_init_params = {
		.pin_mode = 0	// NA
	};

	struct mbed_gpio_init_param gain_gpio_extra_init_params = {
		.pin_mode = 0	// NA
	};

	// Initialize the device structure
	struct ad5686_init_param nanodac_init_params = {
		// i2c_init_param
		{
			.max_speed_hz = 100000,				// I2C max speed (Hz)
			.slave_address = I2C_SLAVE_ADDRESS,	// I2C slave address
			.extra = &i2c_init_extra_params,	// I2C extra init parameters
			.platform_ops = &mbed_i2c_ops
		},

		// spi_init_param
		.spi_init = {
			.max_speed_hz = 2000000, 	    // SPI max speed (Hz)
			.chip_select = SPI_CSB, 		// Chip select
			.mode = NO_OS_SPI_MODE_2,		// SPI Mode
			.extra = &spi_init_extra_params,	// SPI extra init parameters
			.platform_ops = &mbed_spi_ops
		},

		// gpio_init_param
		{
			.number = RESET_PIN,            // Reset GPIO Pin
			.platform_ops = &mbed_gpio_ops,
			.extra =  &reset_gpio_extra_init_params
		},
		{
			.number = LDAC_PIN,				// LDAC GPIO Pin
			.platform_ops = &mbed_gpio_ops,
			.extra =  &ldac_gpio_extra_init_params
		},
		{
			.number = GAIN_PIN,             // Gain GPIO Pin
			.platform_ops = &mbed_gpio_ops,
			.extra =  &gain_gpio_extra_init_params
		},

		.act_device  = ACTIVE_DEVICE	   // Active device
	};

	do {
		// Initialize the device
		if((device_init_status = ad5686_init(&nanodac_dev,
						     nanodac_init_params)) != 0)
			break;

		// Configure the GPIOs specific to application upon power-up
		if((device_init_status = gpio_power_up_configuration()) != 0)
			break;

		return device_init_status;
	} while (0);

	ad5686_remove(nanodac_dev);
	return device_init_status;
}


/*!
 * @brief      Set the power-up GPIO configurations
 * @return     0 in case of success, negative error code otherwise
 */
static int32_t gpio_power_up_configuration(void)
{
	int32_t gpio_power_up_status = 0;

	// Set the GPIO values
	gpio_power_up_status |= no_os_gpio_set_value(nanodac_dev->gpio_reset,
				NO_OS_GPIO_HIGH);
	gpio_power_up_status |= no_os_gpio_set_value(nanodac_dev->gpio_gain,
				NO_OS_GPIO_LOW);
	gpio_power_up_status |= no_os_gpio_set_value(nanodac_dev->gpio_ldac,
				NO_OS_GPIO_HIGH);

	return gpio_power_up_status;
}


/*!
 * @brief      Display the header info for menu
 * @return     None
 */
static void display_menu_header(void)
{
	printf("\t%s (nanodac) | ", ACTIVE_DEVICE_NAME);
	printf("Vref:%s (%.1fV) | ", vref_source_str[vref_source], vref_voltage);
	printf("Gain:%ld"EOL, gain);
}


/*!
 * @brief      Display the footer info for menu
 * @return     None
 */
static void display_menu_footer(void)
{
	// Display the device name
	printf("\tActive Channel: %d | ", current_dac_channel);
	printf("LDAC Pin: %ld | ", ldac_pin_state);
	printf("LDAC Mask: %d"EOL, ldac_mask_status);
}


/*!
 * @brief      Handle the DAC channel selection menu
 * @param      uint32_t channel_id- Selected DAC channel number
 * @return     MENU_CONTINUE
 */
static int32_t menu_select_dac_channel(uint32_t channel_id)
{
	// Store the dac channel for future read/write operations
	current_dac_channel = (enum ad5686_dac_channels)channel_id;

	printf(EOL EOL"\tDAC Channel %ld is selected..."EOL, channel_id);

	adi_press_any_key_to_continue();
	return MENU_CONTINUE;
}


/*!
 * @brief      Handle the menu to display DAC channel selection
 * @param      uint32_t menu_id- (Optional parameter)
 * @return     MENU_CONTINUE
 */
static int32_t menu_dac_channels(uint32_t menu_id)
{
	// Display the dac channel selection menu
	adi_do_console_menu(&dac_channel_select_menu);

	return MENU_CONTINUE;
}


/*!
 * @brief      Handle the menu to write DAC input register
 * @param      uint32_t menu_id- (Optional parameter)
 * @return     MENU_CONTINUE
 */
static int32_t menu_write_to_input_register(uint32_t menu_id)
{
	uint16_t dac_data_input;	// Data to be written to DAC

	printf(EOL"\tEnter the Data/Code (in decimal): ");
	dac_data_input = (uint16_t)adi_get_decimal_int(5);

	// Write DAC input register for current selected channel
	ad5686_write_register(nanodac_dev, current_dac_channel, dac_data_input);

	printf(EOL EOL"\tData %d written to DAC input register..."EOL, dac_data_input);

	adi_press_any_key_to_continue();
	return MENU_CONTINUE;
}


/*!
 * @brief      Handle the menu to update DAC register with value from input register
 * @param      uint32_t menu_id- (Optional parameter)
 * @return     MENU_CONTINUE
 */
static int32_t menu_update_dac_from_input(uint32_t menu_id)
{
	// Update the DAC with input register data for current selected channel
	ad5686_update_register(nanodac_dev, current_dac_channel);

	printf(EOL EOL"\tUpdated DAC register with contents of input register..."EOL);

	adi_press_any_key_to_continue();
	return MENU_CONTINUE;
}


/*!
 * @brief      Handle the menu to update DAC register by asserting LDAC pin
 * @param      uint32_t menu_id- (Optional parameter)
 * @return     MENU_CONTINUE
 */
static int32_t menu_update_dac_by_ldac_assert(uint32_t menu_id)
{
	// Update DAC registers by asserting LDAC pin High to Low
	no_os_gpio_set_value(nanodac_dev->gpio_ldac, NO_OS_GPIO_HIGH);
	no_os_mdelay(1);
	no_os_gpio_set_value(nanodac_dev->gpio_ldac, NO_OS_GPIO_LOW);
	no_os_mdelay(1);

	// Restore the previous state of LDAC pin
	if (ldac_pin_state == NO_OS_GPIO_HIGH) {
		no_os_gpio_set_value(nanodac_dev->gpio_ldac, NO_OS_GPIO_HIGH);
	}

	printf(EOL EOL"\tUpdated DAC register with contents of input register..."EOL);

	adi_press_any_key_to_continue();
	return MENU_CONTINUE;
}


/*!
 * @brief      Handle the menu to write and update DAC register directly
 * @param      uint32_t menu_id- (Optional parameter)
 * @return     MENU_CONTINUE
 */
static int32_t menu_write_and_update_dac(uint32_t menu_id)
{
	uint16_t dac_data_input;	// Data to be written to DAC

	printf(EOL"\tEnter the Data/Code (in decimal): ");
	dac_data_input = (uint16_t)adi_get_decimal_int(5);

	// Update DAC data register for current selected channel
	ad5686_write_update_register(nanodac_dev, current_dac_channel, dac_data_input);

	printf(EOL EOL"\tDAC updated with Data %d"EOL, dac_data_input);

	adi_press_any_key_to_continue();
	return MENU_CONTINUE;
}


/*!
 * @brief      Handle the menu to select operating mode of DAC
 * @param      uint32_t operating_mode_input- User input operating mode
 * @return     MENU_CONTINUE
 */
static int32_t menu_select_operating_mode(uint32_t operating_mode_input)
{
	// Select the operating mode of DAC
	ad5686_power_mode(nanodac_dev, current_dac_channel, operating_mode_input);

	printf(EOL EOL"\tSelected operating mode as %s"EOL,
	       operating_mode_str[operating_mode_input]);

	adi_press_any_key_to_continue();
	return MENU_CONTINUE;
}


/*!
 * @brief      Handle the menu to display operating modes of DAC
 * @param      uint32_t menu_id- (Optional parameter)
 * @return     MENU_CONTINUE
 */
static int32_t menu_dac_operating_modes(uint32_t menu_id)
{
	// Display the operating mode select menu
	adi_do_console_menu(&operating_mode_select_menu);

	return MENU_CONTINUE;
}


/*!
 * @brief      Handle the menu to select the reference source for DAC
 * @param      uint32_t ref_source_input- user selected reference source
 * @return     MENU_CONTINUE
 */
static int32_t menu_select_vref_source(uint32_t ref_source_input)
{
	float user_vref_value = INTERNAL_VREF_VOLTAGE; 	// user input reference
	// voltage value

	if (ref_source_input == INTERNAL_VREF_SOURCE) {
		// Enable the internal reference source
		ad5686_internal_reference(nanodac_dev, AD5686_INTREF_EN);
		vref_source = INTERNAL_VREF_SOURCE;
		vref_voltage = INTERNAL_VREF_VOLTAGE;

		// Display the vref selections on console window
		printf(EOL"\tVref Source: %s"EOL, vref_source_str[vref_source]);
		printf("\tVref Voltage: %f"EOL, vref_voltage);
	} else if (ref_source_input == EXTERNAL_VREF_SOURCE) {
		printf(EOL EOL"\tEnter the external reference voltage"EOL);
		user_vref_value = adi_get_decimal_float(5);

		// Disable the internal reference source
		ad5686_internal_reference(nanodac_dev, AD5686_INTREF_DIS);
		vref_source = EXTERNAL_VREF_SOURCE;
		vref_voltage = user_vref_value;

		// Display the vref selections on console window
		printf(EOL"\tVref Source: %s"EOL, vref_source_str[vref_source]);
		printf("\tVref Voltage: %f"EOL, vref_voltage);
	} else {
		printf(EOL EOL"\tInvalid Vref Source selection"EOL);
	}

	adi_press_any_key_to_continue();
	return MENU_CONTINUE;
}


/*!
 * @brief      Handle the menu to display the reference source for DAC
 * @param      uint32_t menu_id- (Optional parameter)
 * @return     MENU_CONTINUE
 */
static int32_t menu_vref_sources(uint32_t menu_id)
{
	// Display the Vref selection menu
	adi_do_console_menu(&vref_select_menu);

	return MENU_CONTINUE;
}


/*!
 * @brief      Handle the menu to readback DAC register for selected channel
 * @param      uint32_t menu_id- (Optional parameter)
 * @return     MENU_CONTINUE
 */
static int32_t menu_dac_readback(uint32_t menu_id)
{
	uint16_t dac_data; 		// Readback DAC data
	float output_voltage;	// Output voltage of DAC channel

	// Readback data for current selected DAC channel
	dac_data = ad5686_read_back_register(nanodac_dev, current_dac_channel);

	// Calculate equivalent output voltage
	output_voltage = (vref_voltage * gain * ((float)dac_data / TOTAL_OUTPUT_CODES));

	printf(EOL EOL"\tDAC Channel %d Data: %d"EOL, current_dac_channel, dac_data);
	printf(EOL"\tVoltage: %.3f V"EOL EOL, output_voltage);

	adi_press_any_key_to_continue();
	return MENU_CONTINUE;
}


/*!
 * @brief      Handle the menu to select LDACx mask (x:DAC channel)
 * @param      uint32_t mask_status- user input LDAC mask
 * @return     MENU_CONTINUE
 */
static int32_t menu_set_ldac_mask(uint32_t mask_status)
{
	// Set the LDAC mask (0/1)
	ad5686_ldac_mask(nanodac_dev, current_dac_channel, mask_status);

	ldac_mask_status = mask_status;
	printf(EOL EOL"\tLDAC Mask for Channel %d: %ld"EOL, current_dac_channel,
	       mask_status);

	adi_press_any_key_to_continue();
	return MENU_CONTINUE;
}


/*!
 * @brief      Handle the menu to display LDACx mask menu selections
 * @param      uint32_t menu_id- (Optional parameter)
 * @return     MENU_CONTINUE
 */
static int32_t menu_ldac_masks(uint32_t menu_id)
{
	// Display the LDAC mask selection menu
	adi_do_console_menu(&ldac_mask_select_menu);

	return MENU_CONTINUE;
}


/*!
 * @brief      Handle the menu to set LDAC pin state
 * @param      uint32_t pin_state- user input pin state (HIGH/LOW)
 * @return     MENU_CONTINUE
 */
static int32_t menu_set_ldac_pin(uint32_t pin_state)
{
	if (pin_state == NO_OS_GPIO_HIGH) {
		no_os_gpio_set_value(nanodac_dev->gpio_ldac, NO_OS_GPIO_HIGH);
	} else {
		no_os_gpio_set_value(nanodac_dev->gpio_ldac, NO_OS_GPIO_LOW);
	}

	ldac_pin_state = pin_state;
	printf(EOL EOL"\tLDAC pin set to %ld"EOL, ldac_pin_state);

	adi_press_any_key_to_continue();
	return MENU_CONTINUE;
}


/*!
 * @brief      Handle the menu to assert LDAC pin to update DAC
 * @param      uint32_t menu_id- (Optional parameter)
 * @return     MENU_CONTINUE
 */
static int32_t menu_select_ldac_pin_state(uint32_t menu_id)
{
	// Display the LDAC pin selection menu
	adi_do_console_menu(&ldac_pin_select_menu);

	return MENU_CONTINUE;
}


/*!
 * @brief      Handle the menu to set the DAC gain
 * @param      uint32_t gain_input- Gain input value
 * @return     MENU_CONTINUE
 */
static int32_t menu_set_gain(uint32_t gain_input)
{
	// Set the device gain
	gain = gain_input;

#if defined (SOFTWARE_CONTROLLED_GAIN)
	ad5686_gain_mode(nanodac_dev, gain);
#else
	if (gain == GAIN_LOW) {
		no_os_gpio_set_value(nanodac_dev->gpio_gain, NO_OS_GPIO_LOW);
	} else {
		no_os_gpio_set_value(nanodac_dev->gpio_gain, NO_OS_GPIO_HIGH);
	}
#endif

	printf(EOL EOL"\tGain set to %ld"EOL,gain);

	adi_press_any_key_to_continue();
	return MENU_CONTINUE;
}


/*!
 * @brief      Handle the menu to display DAC gain selections
 * @param      uint32_t menu_id- (Optional parameter)
 * @return     MENU_CONTINUE
 */
static int32_t menu_gain_selection(uint32_t menu_id)
{
	// Display the DAC gain selection menu
	adi_do_console_menu(&gain_select_menu);

	return MENU_CONTINUE;
}


/*!
 * @brief      Handle the menu to perform DAC software reset
 * @param      uint32_t menu_id- (Optional parameter)
 * @return     MENU_CONTINUE
 */
static int32_t menu_assert_software_reset(uint32_t menu_id)
{
	// Do device software reset
	ad5686_software_reset(nanodac_dev);

	printf(EOL"\tSoftware Reset Complete..."EOL);

	// Device reset disables the LDAC mask through hardware.
	// This needs to be synched with software.
	ldac_mask_status = false;

	adi_press_any_key_to_continue();
	return MENU_CONTINUE;
}


/*!
 * @brief      Handle the menu to perform DAC hardware reset
 * @param      uint32_t menu_id- (Optional parameter)
 * @return     MENU_CONTINUE
 */
static int32_t menu_assert_hardware_reset(uint32_t menu_id)
{
	// Do device hardware reset
	no_os_gpio_set_value(nanodac_dev->gpio_reset, NO_OS_GPIO_LOW);
	no_os_mdelay(1);
	no_os_gpio_set_value(nanodac_dev->gpio_reset, NO_OS_GPIO_HIGH);

	printf(EOL"\tHardware Reset Complete..."EOL);

	// Device reset disables the LDAC mask through hardware.
	// This needs to be synched with software.
	ldac_mask_status = false;

	adi_press_any_key_to_continue();
	return MENU_CONTINUE;
}


// Operating mode menu for DAC
static console_menu_item operating_mode_select_items[] = {
	{ "Normal Power-Up", 'A', menu_select_operating_mode, NULL, AD5686_PWRM_NORMAL },

#if defined(_1K_TO_GND_POWER_DOWN)
	{ "1K to GND (Power-Down)",   'B', menu_select_operating_mode, NULL, AD5686_PWRM_1K },
#endif
#if defined(_100K_TO_GND_POWER_DOWN)
	{ "100K to GND (Power-Down)", 'C', menu_select_operating_mode, NULL, AD5686_PWRM_100K },
#endif
#if defined(THREE_STATE_POWER_DOWN)
	{ "Three-State (Power-Down)", 'D', menu_select_operating_mode, NULL, AD5686_PWRM_THREESTATE },
#endif
};

console_menu operating_mode_select_menu = {
	.title = "Select Operating Mode",
	.items = operating_mode_select_items,
	.itemCount = ARRAY_SIZE(operating_mode_select_items),
	.headerItem = display_menu_header,
	.footerItem = display_menu_footer,
	.enableEscapeKey = true
};


// LDAC pin set menu for DAC
static console_menu_item ldac_pin_select_items[] = {
	{ "High (VLogic)", 'H', menu_set_ldac_pin, NULL, NO_OS_GPIO_HIGH },
	{ "Low  (GND)", 'L', menu_set_ldac_pin, NULL, NO_OS_GPIO_LOW }
};

console_menu ldac_pin_select_menu = {
	.title = "Select LDAC Mask",
	.items = ldac_pin_select_items,
	.itemCount = ARRAY_SIZE(ldac_pin_select_items),
	.headerItem = display_menu_header,
	.footerItem = display_menu_footer,
	.enableEscapeKey = true
};


// LDAC mask menu for DAC
static console_menu_item ldac_mask_select_items[] = {
	{ "Disable (LDAC Pin Controlled)", 'D', menu_set_ldac_mask, NULL, 0 },
	{ "Enable  (LDAC Pin Ignored)", 'E', menu_set_ldac_mask, NULL, 1 }
};

console_menu ldac_mask_select_menu = {
	.title = "Select LDAC Mask",
	.items = ldac_mask_select_items,
	.itemCount = ARRAY_SIZE(ldac_mask_select_items),
	.headerItem = display_menu_header,
	.footerItem = display_menu_footer,
	.enableEscapeKey = true
};


// Gain select menu for DAC
static console_menu_item gain_select_items[] = {
	{ "Gain= 1 (Vout: 0-Vref)", '1', menu_set_gain, NULL, GAIN_LOW },
	{ "Gain= 2 (Vout: 0-2*Vref)", '2', menu_set_gain, NULL, GAIN_HIGH }
};

console_menu gain_select_menu = {
	.title = "Select Gain",
	.items = gain_select_items,
	.itemCount = ARRAY_SIZE(gain_select_items),
	.headerItem = display_menu_header,
	.footerItem = display_menu_footer,
	.enableEscapeKey = true
};


// Vref select menu for DAC
static console_menu_item vref_select_items[] = {
#if !defined(EXT_VREF_SOURCE_ONLY)
	{ "Internal Vref", 'I', menu_select_vref_source, NULL, INTERNAL_VREF_SOURCE },
#endif
	{ "External Vref", 'E', menu_select_vref_source, NULL, EXTERNAL_VREF_SOURCE }
};

console_menu vref_select_menu = {
	.title = "Select Vref Source",
	.items = vref_select_items,
	.itemCount = ARRAY_SIZE(vref_select_items),
	.headerItem = display_menu_header,
	.footerItem = display_menu_footer,
	.enableEscapeKey = true
};


// Channel selection menu for DAC
static console_menu_item dac_channel_select_items[] = {
	{ "Channel 0", 'A', menu_select_dac_channel, NULL, (uint32_t)AD5686_CH_0 },
	{ "Channel 1", 'B', menu_select_dac_channel, NULL, (uint32_t)AD5686_CH_1 },
#if DAC_CHANNEL_COUNT > 2
	{ "Channel 2", 'C', menu_select_dac_channel, NULL, (uint32_t)AD5686_CH_2 },
	{ "Channel 3", 'D', menu_select_dac_channel, NULL, (uint32_t)AD5686_CH_3 },
#endif
#if DAC_CHANNEL_COUNT > 4
	{ "Channel 4", 'E', menu_select_dac_channel, NULL, (uint32_t)AD5686_CH_4 },
	{ "Channel 5", 'F', menu_select_dac_channel, NULL, (uint32_t)AD5686_CH_5 },
	{ "Channel 6", 'G', menu_select_dac_channel, NULL, (uint32_t)AD5686_CH_6 },
	{ "Channel 7", 'H', menu_select_dac_channel, NULL, (uint32_t)AD5686_CH_7 },
#endif
#if DAC_CHANNEL_COUNT > 8
	{ "Channel 8", 'I', menu_select_dac_channel, NULL, (uint32_t)AD5686_CH_8 },
	{ "Channel 9", 'J', menu_select_dac_channel, NULL, (uint32_t)AD5686_CH_9 },
	{ "Channel 10", 'K', menu_select_dac_channel, NULL, (uint32_t)AD5686_CH_10 },
	{ "Channel 11", 'L', menu_select_dac_channel, NULL, (uint32_t)AD5686_CH_11 },
	{ "Channel 12", 'M', menu_select_dac_channel, NULL, (uint32_t)AD5686_CH_12 },
	{ "Channel 13", 'N', menu_select_dac_channel, NULL, (uint32_t)AD5686_CH_13 },
	{ "Channel 14", 'O', menu_select_dac_channel, NULL, (uint32_t)AD5686_CH_14 },
	{ "Channel 15", 'P', menu_select_dac_channel, NULL, (uint32_t)AD5686_CH_15 },
#endif
};

console_menu dac_channel_select_menu = {
	.title = "Select DAC Channel",
	.items = dac_channel_select_items,
	.itemCount = ARRAY_SIZE(dac_channel_select_items),
	.headerItem = display_menu_header,
	.footerItem = display_menu_footer,
	.enableEscapeKey = true
};


/*
 * Definition of the Main Menu Items and menu itself
 */
static console_menu_item main_menu_items[] = {
#ifdef DISPLAY_DAC_CHANNEL_SELECT_MENU
	{	"Select DAC Channel",					'A',	menu_dac_channels },
	{""},
#endif
	{	"Write to Input Register (LDAC Dependent)",	'B',menu_write_to_input_register },
	{""},
	{	"Update DAC from Input Register",		'C',	menu_update_dac_from_input },
	{""},
	{	"Update DAC by LDAC Assert (H->L)",		'D',	menu_update_dac_by_ldac_assert },
	{""},
	{	"Write and Update DAC (Direct Update)",	'E',	menu_write_and_update_dac },
	{""},
	{	"Read Back DAC Channel",				'F',	menu_dac_readback },
	{""},
#ifdef DISPLAY_LDAC_MASK_SELECT_MENU
	{	"Set LDAC# Mask",						'G',	menu_ldac_masks },
	{""},
#endif
	{	"Select LDAC Pin State",				'H',	menu_select_ldac_pin_state },
	{""},
	{	"Select Operating Mode",				'I',	menu_dac_operating_modes },
	{""},
	{	"Select Reference Source (Vref)",		'J',	menu_vref_sources },
	{""},
	{	"Set Gain",								'K',	menu_gain_selection },
	{""},
	{	"Assert Software Reset",				'L',	menu_assert_software_reset },
	{""},
	{	"Assert Hardware Reset",				'M',	menu_assert_hardware_reset },
	{""},
};

console_menu nanodac_main_menu = {
	.title = "Main Menu",
	.items = main_menu_items,
	.itemCount = ARRAY_SIZE(main_menu_items),
	.headerItem = display_menu_header,
	.footerItem = display_menu_footer,
	.enableEscapeKey = false
};
