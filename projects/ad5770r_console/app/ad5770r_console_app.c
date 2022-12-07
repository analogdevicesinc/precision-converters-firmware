/*!
 *****************************************************************************
  @file:  ad5770r_console_app.c

  @brief: Implementation for the menu functions that handle the AD5770R

  @details:
 -----------------------------------------------------------------------------
 *
Copyright (c) 2020-2022 Analog Devices, Inc. All Rights Reserved.

This software is proprietary to Analog Devices, Inc. and its licensors.
By using this software you agree to the terms of the associated
Analog Devices Software License Agreement.
 ******************************************************************************/

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>

#include "app_config.h"

#include "no_os_delay.h"
#include "no_os_error.h"
#include "no_os_gpio.h"
#include "no_os_spi.h"
#include "mbed_platform_support.h"
#include "mbed_gpio.h"
#include "ad5770r.h"

#include "ad5770r_console_app.h"
#include "ad5770r_user_config.h"
#include "ad5770r_reset_config.h"

/******************************************************************************/
/********************** Macros and Constants Definitions **********************/
/******************************************************************************/

#define TOGGLE_MUX_BUFFER       1000
#define TOGGLE_DIODE_EXT_BIAS   1001
#define MENU_CHANNEL_OFFSET     100

/******************************************************************************/
/********************** Variables and User Defined Data Types *****************/
/******************************************************************************/

static struct ad5770r_dev * pAd5770r_dev = NULL;

static struct ad5770r_channel_switches sw_ldac_shadow;

// GPIO descriptor and init parameters for the HW LDACB pin
static struct no_os_gpio_desc * hw_ldacb_desc = NULL;
struct mbed_gpio_init_param hw_ldacb_extra_init_params = {
	.pin_mode = 0	// NA
};
static struct no_os_gpio_init_param hw_ldacb_init_param = {
	.number = HW_LDACB,
	.platform_ops = &mbed_gpio_ops,
	.extra = &hw_ldacb_extra_init_params
};

// Forward Declaration
static console_menu general_configuration_menu;
static console_menu monitor_setup_menu;
static console_menu dac_channel_configuration_menu;
static console_menu dac_operations_menu;

/******************************************************************************/
/************************ Functions Definitions *******************************/
/******************************************************************************/

/*!
 * @brief      Initialize the AD5770R device and the SPI port as required
 * @return     0 in case of success, negative error code otherwise
 * @details    This resets and then writes the default register map value to
 *  		   the device.  A call to init the SPI port is made, but may not
 *  		   actually do very much, depending on the platform
 */
int32_t ad5770r_app_initialize(void)
{
	// Create a new descriptor for HW LDACB
	if(no_os_gpio_get(&hw_ldacb_desc, &hw_ldacb_init_param) != 0) {
		return -EINVAL;
	}

	// Set the direction of HW LDACB
	if((no_os_gpio_direction_output(hw_ldacb_desc, NO_OS_GPIO_HIGH)) != 0) {
		return -EINVAL;
	}

	// Set the default output state  of HW LDACB
	if((no_os_gpio_set_value(hw_ldacb_desc, NO_OS_GPIO_HIGH)) != 0) {
		return -EINVAL;
	}

	return(ad5770r_init(&pAd5770r_dev, &ad5770r_user_param));
}

/**
 * @brief Performs a software reset.
 * @param dev[in] - The device structure.
 * @return 0 in case of success, negative error code otherwise
 */
int32_t ad5770r_software_reset(struct ad5770r_dev *dev)
{
	int32_t ret;

	if (!dev)
		return -EINVAL;

	ret = ad5770r_spi_reg_write(dev,
				    AD5770R_INTERFACE_CONFIG_A,
				    AD5770R_INTERFACE_CONFIG_A_SW_RESET_MSK |
				    AD5770R_INTERFACE_CONFIG_A_ADDR_ASCENSION_MSB(
					    dev->dev_spi_settings.addr_ascension));

	if (ret)
		return ret;

	// Save the spi_desc pointer field
	struct no_os_spi_desc * spi_interface = dev->spi_desc;

	// Copy over the reset state of the device
	memcpy(dev, &ad5770r_dev_reset, sizeof(ad5770r_dev_reset));

	// Restore the spi_desc pointer field
	dev->spi_desc = spi_interface;

	return ret;
}

/*
 * @brief      Sends a reset command on the SPI to reset the device
 * @param      id[in]- Menu ID
 * @return     MENU_CONTINUE
 */
static int32_t do_software_reset(uint32_t id)
{
	int32_t ret;

	if ((ret = ad5770r_software_reset(pAd5770r_dev)) == 0) {
		printf(EOL " --- Software Reset Succeeded ---" EOL);
	} else {
		printf(EOL " *** Software Reset Failure: %d ***" EOL, ret);
	}
	adi_press_any_key_to_continue();
	return(MENU_CONTINUE);
}

/*
 * @brief      Creates and initializes a device with user configuration
 * @param      id[in]- Menu ID
 * @return     MENU_CONTINUE
 */
static int32_t do_device_init(uint32_t id)
{
	if (ad5770r_init(&pAd5770r_dev, &ad5770r_user_param) == 0) {
	} else {
		printf("\n\r *** Error device init ***\n\r");
	}

	adi_press_any_key_to_continue();
	return(MENU_CONTINUE);
}

/*
 * @brief      Removes the device from memory
 * @param      id[in]- Menu ID
 * @return     MENU_CONTINUE
 */
static int32_t do_device_remove(uint32_t id)
{
	if (ad5770r_remove(pAd5770r_dev) != 0) {
		printf("\n\r *** Error doing device remove ***\n\r");
	}

	pAd5770r_dev = NULL;

	adi_press_any_key_to_continue();
	return(MENU_CONTINUE);
}

/*!
 * @brief      toggles the int/ext ref resistor option
 * @param      id[in]- Menu ID
 * @return     MENU_CONTINUE
 */
static int32_t do_toggle_ref_resistor(uint32_t id)
{
	int32_t status;

	if ((status = ad5770r_set_reference(pAd5770r_dev,
					    !pAd5770r_dev->external_reference,
					    pAd5770r_dev->reference_selector)) != 0) {
		printf(EOL " *** Error toggling ref resistor setting: %d" EOL, status);
		adi_press_any_key_to_continue();
	}

	return(MENU_CONTINUE);
}

/*!
 * @brief      sets the int/ext refer configuration options
 * @param      ref_option[in]- Reference source option
 * @return     MENU_CONTINUE
 */
static int32_t do_set_reference(uint32_t ref_option)
{
	int32_t status;

	if ((status = ad5770r_set_reference(pAd5770r_dev,
					    pAd5770r_dev->external_reference,
					    (enum ad5770r_reference_voltage)ref_option)) != 0) {
		printf(EOL " *** Error toggling ref resistor setting: %d" EOL, status);
		adi_press_any_key_to_continue();
	}

	return(MENU_CONTINUE);
}

/*!
 * @brief      Sets the Alarm Menu option bits
 * @param      alarm_id[in]- Alarm ID
 * @return     MENU_CONTINUE
 */
static int32_t do_set_alarm(uint32_t alarm_id)
{
	int32_t status;
	struct ad5770r_alarm_cfg alarm_config;

	alarm_config = pAd5770r_dev->alarm_config;

	switch(alarm_id) {
	case AD5770R_ALARM_CONFIG_OPEN_DRAIN_EN(1):
		alarm_config.open_drain_en = !alarm_config.open_drain_en;
		break;
	case AD5770R_ALARM_CONFIG_THERMAL_SHUTDOWN_EN(1):
		alarm_config.thermal_shutdown_en = !alarm_config.thermal_shutdown_en;
		break;
	case AD5770R_ALARM_CONFIG_BACKGROUND_CRC_EN(1):
		alarm_config.background_crc_en = !alarm_config.background_crc_en;
		break;
	case AD5770R_ALARM_CONFIG_TEMP_WARNING_ALARM_MASK(1):
		alarm_config.temp_warning_msk = !alarm_config.temp_warning_msk;
		break;
	case AD5770R_ALARM_CONFIG_OVER_TEMP_ALARM_MASK(1):
		alarm_config.over_temp_msk = !alarm_config.over_temp_msk;
		break;
	case AD5770R_ALARM_CONFIG_NEGATIVE_CHANNEL0_ALARM_MASK(1):
		alarm_config.neg_ch0_msk = !alarm_config.neg_ch0_msk;
		break;
	case AD5770R_ALARM_CONFIG_IREF_FAULT_ALARM_MASK(1):
		alarm_config.iref_fault_msk = !alarm_config.iref_fault_msk;
		break;
	case AD5770R_ALARM_CONFIG_BACKGROUND_CRC_ALARM_MASK(1):
		alarm_config.background_crc_msk = !alarm_config.background_crc_msk;
		break;
	default:
		// not a supported menu option
		assert(false);
	}

	if ((status = ad5770r_set_alarm(pAd5770r_dev,
					&alarm_config)) != 0) {
		printf(EOL " *** Error setting alarm config: %d" EOL, status);
		adi_press_any_key_to_continue();
	}

	return(MENU_CONTINUE);
}

/*!
 * @brief      Sets the Channel Configuration option bits
 * @param      ch_switches[out] - Channel switches
 * @param      channel_id[in]- Channel ID
 * @return     None
 */
static void ch_switches_toggle(struct ad5770r_channel_switches *ch_switches,
			       uint32_t channel_id)
{
	switch(channel_id) {
	case AD5770R_CHANNEL_CONFIG_CH0_SHUTDOWN_B(1):
		ch_switches->en0 = !ch_switches->en0;
		break;
	case AD5770R_CHANNEL_CONFIG_CH1_SHUTDOWN_B(1):
		ch_switches->en1 = !ch_switches->en1;
		break;
	case AD5770R_CHANNEL_CONFIG_CH2_SHUTDOWN_B(1):
		ch_switches->en2 = !ch_switches->en2;
		break;
	case AD5770R_CHANNEL_CONFIG_CH3_SHUTDOWN_B(1):
		ch_switches->en3 = !ch_switches->en3;
		break;
	case AD5770R_CHANNEL_CONFIG_CH4_SHUTDOWN_B(1):
		ch_switches->en4 = !ch_switches->en4;
		break;
	case AD5770R_CHANNEL_CONFIG_CH5_SHUTDOWN_B(1):
		ch_switches->en5 = !ch_switches->en5;
		break;
	case AD5770R_CHANNEL_CONFIG_CH0_SINK_EN(1):
		ch_switches->sink0 = !ch_switches->sink0;
		break;
	default:
		// not a supported menu option
		assert(false);
	}
}

/*!
 * @brief      Sets the Channel Configuration option bits
 * @param      channel_id[in]- Channel ID
 * @return     MENU_CONTINUE
 */
static int32_t do_channel_config(uint32_t channel_id)
{
	int32_t status;
	struct ad5770r_channel_switches channel_config;

	channel_config = pAd5770r_dev->channel_config;

	ch_switches_toggle(&channel_config, channel_id);

	if ((status = ad5770r_channel_config(pAd5770r_dev,
					     &channel_config)) != 0) {
		printf(EOL " *** Error setting channel config: %d" EOL, status);
		adi_press_any_key_to_continue();
	}

	return(MENU_CONTINUE);
}

/*!
 * @brief      prompts user for value to write to input register on channel
 * @param      channel_id[in]- Channel ID
 * @return     MENU_CONTINUE
 */
static int32_t do_input_value(uint32_t channel_id)
{
	int32_t status;
	uint16_t value;

	printf(EOL "Enter Input register value (hex) for channel %d: " EOL,
	       channel_id);
	value = adi_get_hex_integer(4);

	/* Channels are 14-bits, mask off top 2 bits*/
	value &= 0x3FFF;

	if ((status =
		     ad5770r_set_dac_input(pAd5770r_dev, value,
					   (enum ad5770r_channels) channel_id)) != 0) {
		printf(EOL " *** Error writing DAC Input register: %d" EOL, status);
		adi_press_any_key_to_continue();
	}

	return(MENU_CONTINUE);
}

/*!
 * @brief      prompts user for value to write to input register on channel
 * @param      channel_id[in]- Channel ID
 * @return     MENU_CONTINUE
 */
static int32_t do_dac_value(uint32_t channel_id)
{
	int32_t status;
	uint16_t value;

	printf(EOL "Enter DAC register value (hex) for channel %d: " EOL, channel_id);
	value = adi_get_hex_integer(4);

	/* Channels are 14-bits, mask off top 2 bits*/
	value &= 0x3FFF;

	if ((status =
		     ad5770r_set_dac_value(pAd5770r_dev, value,
					   (enum ad5770r_channels) channel_id)) != 0) {
		printf(EOL " *** Error writing DAC value register: %d" EOL, status);
		adi_press_any_key_to_continue();
	}

	return(MENU_CONTINUE);
}

/*!
 * @brief      updating shadow SW LDAC, by toggling the channel bit
 * @param      channel_id[in]- Channel ID
 * @return     MENU_CONTINUE
 */
static int32_t do_sw_ldac(uint32_t channel_id)
{
	ch_switches_toggle(&sw_ldac_shadow, channel_id);

	return(MENU_CONTINUE);
}

/*!
 * @brief      Writing SW LDAC to device
 * @param      id[in]- Menu ID
 * @return     MENU_CONTINUE
 */
static int32_t do_sw_ldac_write(uint32_t id)
{
	int32_t status;

	if ((status = ad5770r_set_sw_ldac(pAd5770r_dev,
					  &sw_ldac_shadow)) != 0) {
		printf(EOL " *** Error writing SW LDAC: %d" EOL, status);
		adi_press_any_key_to_continue();
	}

	return(MENU_CONTINUE);
}

/*!
 * @brief      Toggles HW LDAC
 * @param      id[in]- Menu ID
 * @return     MENU_CONTINUE
 * @details    This toggles the LDAC pin on the device, but is independent to the driver
 *              Therefore this does not update dac_values from input values.
 */
static int32_t do_hw_ldac_toggle(uint32_t id)
{
	int32_t status;

	do {
		if ((status = no_os_gpio_set_value(hw_ldacb_desc, NO_OS_GPIO_LOW)) != 0) {
			break;
		}

		no_os_mdelay(1);

		if ((status = no_os_gpio_set_value(hw_ldacb_desc, NO_OS_GPIO_HIGH)) != 0) {
			break;
		}
	} while (0);

	if (status == 0) {
		printf(EOL " --- HW LDAC toggled ---" EOL);
	} else {
		printf(EOL " *** Error toggling HW LDACB ***" EOL);
	}

	adi_press_any_key_to_continue();

	return(MENU_CONTINUE);
}

/*!
 * @brief      displays general device configuration state
 * @return     None
 */
static void display_gen_config(void)
{
	printf("\tRef Resistor: %s\t\tRef Voltage: %d" EOL,
	       pAd5770r_dev->external_reference == true ? "External" : "Internal",
	       pAd5770r_dev->reference_selector);

	printf("\tAlarms\tBgCRC Msk: %d\tIRef: %d\tneg: %d\tOT: %d " EOL \
	       "\t\tT Warn: %d\tBgCRC En: %d\tT Shdn: %d\tOD: %d" EOL,
	       pAd5770r_dev->alarm_config.background_crc_msk,
	       pAd5770r_dev->alarm_config.iref_fault_msk,
	       pAd5770r_dev->alarm_config.neg_ch0_msk,
	       pAd5770r_dev->alarm_config.over_temp_msk,
	       pAd5770r_dev->alarm_config.temp_warning_msk,
	       pAd5770r_dev->alarm_config.background_crc_en,
	       pAd5770r_dev->alarm_config.thermal_shutdown_en,
	       pAd5770r_dev->alarm_config.open_drain_en);
}

/*!
 * @brief      displays general device configuration state
 * @param      ch_switches[in] - Channel switches
 * @param      prefix[in] - prefix for printing output
 * @param      include_sink[in] - Sink include status
 * @return     None
 */
static void print_channel_switches(struct ad5770r_channel_switches *ch_switches,
				   char * prefix, bool include_sink)
{
	if (include_sink) {
		printf("\t%s - en0: %d sink0: %d  en1: %d  en2: %d  " \
		       "en3: %d  en4: %d  en5: %d" EOL, prefix,
		       ch_switches->en0, ch_switches->sink0, ch_switches->en1,
		       ch_switches->en2, ch_switches->en3, ch_switches->en4,
		       ch_switches->en5);
	} else {
		printf("\t%s - ch0: %d  ch1: %d  ch2: %d  " \
		       "ch3: %d  ch4: %d  ch5: %d" EOL, prefix,
		       ch_switches->en0, ch_switches->en1, ch_switches->en2,
		       ch_switches->en3, ch_switches->en4, ch_switches->en5);
	}
}

/*!
 * @brief      displays general device configuration state
 * @return     None
 */
static void display_dac_channel_configuration_header(void)
{
	print_channel_switches(&pAd5770r_dev->channel_config, "Ch Configs", true);
}

/*!
 * @brief      displays the SW LDAC shadown and other channel output values
 * @return     None
 */
static void display_dac_operations_header(void)
{
	for (uint8_t i = 0; i < 6 ; i++) {
		printf("\tCh %i - Input: 0x%04X \t\tDAC: 0x%04X" EOL, i,
		       pAd5770r_dev->input_value[i], pAd5770r_dev->dac_value[i]);
	}

	printf(EOL);
	print_channel_switches(&sw_ldac_shadow, "SW LDAC shadow", false);
}

/*!
 * @brief      prints the provided monitor config to the terminal
 * @param      mon_setup[in] - struct for the monitor config to be displayed
 * @return     None
 */
static void print_monitor_setup(const struct ad5770r_monitor_setup * mon_setup)
{
	printf("\tMonitor: ");
	switch (mon_setup->monitor_function) {
	case AD5770R_DISABLE: {
		printf("Disabled ");
		break;
	}
	case AD5770R_VOLTAGE_MONITORING: {
		printf("Voltage Ch %d", mon_setup->monitor_channel);
		break;
	}
	case AD5770R_CURRENT_MONITORING: {
		printf("Current Ch %d", mon_setup->monitor_channel);
		break;
	}
	case AD5770R_TEMPERATURE_MONITORING: {
		printf("Temperature");
		break;
	}
	}

	printf("\tBuffer: ");
	if (mon_setup->mux_buffer == true) {
		printf("On");
	} else {
		printf("Off");
	}

	printf("\tIB_Ext: ");
	if (mon_setup->ib_ext_en == true) {
		printf("On");
	} else {
		printf("Off");
	}
	printf(EOL);
}

/*!
 * @brief      configure the Mux Monitor setup
 * @return     None
 */
static void display_monitor_setup_header(void)
{
	print_monitor_setup(&pAd5770r_dev->mon_setup);
}

/*!
 * @brief      configure the Mux Monitor setup
 * @param      id[in]- Menu ID
 * @return     MENU_CONTINUE
 */
static int32_t do_monitor_setup(uint32_t id)
{
	int32_t status;
	struct ad5770r_monitor_setup monitor_setup;

	monitor_setup = pAd5770r_dev->mon_setup;

	switch(id) {
	case AD5770R_DISABLE:
		monitor_setup.monitor_function = AD5770R_DISABLE;
		break;
	case AD5770R_VOLTAGE_MONITORING:
		monitor_setup.monitor_function = AD5770R_VOLTAGE_MONITORING;
		break;
	case AD5770R_CURRENT_MONITORING:
		monitor_setup.monitor_function = AD5770R_CURRENT_MONITORING;
		break;
	case AD5770R_TEMPERATURE_MONITORING:
		monitor_setup.monitor_function = AD5770R_TEMPERATURE_MONITORING;
		break;
	case TOGGLE_MUX_BUFFER:
		monitor_setup.mux_buffer = !monitor_setup.mux_buffer;
		break;
	case TOGGLE_DIODE_EXT_BIAS:
		monitor_setup.ib_ext_en = !monitor_setup.ib_ext_en;
		break;
	default:
		// ensure the id is valid.
		assert(( id >= AD5770R_CH0 + MENU_CHANNEL_OFFSET )
		       && (id <= AD5770R_CH5 + MENU_CHANNEL_OFFSET));
		monitor_setup.monitor_channel = (enum ad5770r_channels)(
							id - MENU_CHANNEL_OFFSET);
	}

	if ((status = ad5770r_set_monitor_setup(pAd5770r_dev,
						&monitor_setup)) != 0) {
		printf(EOL " *** Error setting monitor setup: %d" EOL, status);
		adi_press_any_key_to_continue();
	}

	return(MENU_CONTINUE);
}

/*!
 * @brief      displays several pieces of status information above main menu
 * @return     None
 */
static void display_main_menu_header(void)
{
	int32_t ret;
	uint8_t device_status = 0, interface_status = 0, scratchpad = 0;

	if (pAd5770r_dev == NULL) {
		printf(EOL " *** Device Not Initialized ***" EOL);
		return;
	}

	do {
		if ((ret = ad5770r_get_status(pAd5770r_dev, &device_status)) != 0) {
			break;
		}
		if ((ret = ad5770r_get_interface_status(pAd5770r_dev,
							&interface_status)) != 0) {
			break;
		}
		if ((ret = ad5770r_spi_reg_read(pAd5770r_dev, AD5770R_SCRATCH_PAD,
						&scratchpad)) != 0) {
			break;
		}

	} while(0);

	if (ret != 0) {
		printf(EOL " *** Error in display state: %d **" EOL, ret);
	}

	printf(EOL "\tInterface Status = 0x%02X\t\tDevice Status = 0x%02X"
	       EOL "\tScratchpad = 0x%02X" EOL,
	       interface_status, device_status, scratchpad);
	print_monitor_setup(&pAd5770r_dev->mon_setup);

	// Increment the scratchpad by 1 to show a +1 delta in footer
	if((ret = ad5770r_spi_reg_write(pAd5770r_dev, AD5770R_SCRATCH_PAD,
					scratchpad + 1)) != 0) {
		printf(EOL " *** Error writing scratchpad + 1 : %d **" EOL, ret);
	}
}

/*!
 * @brief      displays several pieces of status information below main menu
 * @return     None
 */
static void display_main_menu_footer(void)
{
	int32_t ret;
	uint8_t scratchpad;

	if (pAd5770r_dev == NULL) {
		printf(EOL " *** Device Not Initialized ***" EOL);
		return;
	}

	if ((ret = ad5770r_spi_reg_read(pAd5770r_dev, AD5770R_SCRATCH_PAD,
					&scratchpad)) != 0) {
		printf(EOL " *** Error reading scratchpad: %d **" EOL, ret);
	}

	printf(EOL "\tScratchpad = 0x%02X" EOL, scratchpad);
}

/*!
 * @brief      calls the general configuration menu
 * @param      id[in]- Menu ID
 * @return     MENU_CONTINUE
 */
static int32_t do_general_configuration_menu(uint32_t id)
{
	return adi_do_console_menu(&general_configuration_menu);
}

/*!
 * @brief      calls the monitor setup menu
 * @param      id[in]- Menu ID
 * @return     MENU_CONTINUE
 */
static int32_t do_monitor_setup_menu(uint32_t id)
{
	return adi_do_console_menu(&monitor_setup_menu);
}

/*!
 * @brief      calls the DAC channel confguration menu
 * @param      id[in]- Menu ID
 * @return     MENU_CONTINUE
 */
static int32_t do_dac_channel_configuration_menu(uint32_t id)
{
	return adi_do_console_menu(&dac_channel_configuration_menu);
}

/*!
 * @brief      calls the DAC operations menu
 * @param      id[in]- Menu ID
 * @return     MENU_CONTINUE
 */
static int32_t do_dac_operations_menu(uint32_t id)
{
	return adi_do_console_menu(&dac_operations_menu);
}

/*
 * DAC Operations Menu
 */
static  console_menu_item dac_operations_menu_items[] = {
	{"\tSet Input Channel 0", 'Q', do_input_value, NULL, 0},
	{"\tSet Input Channel 1", 'W', do_input_value, NULL, 1},
	{"\tSet Input Channel 2", 'E', do_input_value, NULL, 2},
	{"\tSet Input Channel 3", 'R', do_input_value, NULL, 3},
	{"\tSet Input Channel 4", 'T', do_input_value, NULL, 4},
	{"\tSet Input Channel 5", 'Y', do_input_value, NULL, 5},
	{""},
	{"\tToggle Channel 0 SW LDAC Shadow", '0', do_sw_ldac, NULL, AD5770R_HW_LDAC_MASK_CH(1, 0)},
	{"\tToggle Channel 1 SW LDAC Shadow", '1', do_sw_ldac, NULL, AD5770R_HW_LDAC_MASK_CH(1, 1)},
	{"\tToggle Channel 2 SW LDAC Shadow", '2', do_sw_ldac, NULL, AD5770R_HW_LDAC_MASK_CH(1, 2)},
	{"\tToggle Channel 3 SW LDAC Shadow", '3', do_sw_ldac, NULL, AD5770R_HW_LDAC_MASK_CH(1, 3)},
	{"\tToggle Channel 4 SW LDAC Shadow", '4', do_sw_ldac, NULL, AD5770R_HW_LDAC_MASK_CH(1, 4)},
	{"\tToggle Channel 5 SW LDAC Shadow", '5', do_sw_ldac, NULL, AD5770R_HW_LDAC_MASK_CH(1, 5)},
	{"\tWrite SW LDAC Shadow ",                    'U', do_sw_ldac_write},
	{""},
	{"\tToggle HW LDAC digital input",             'J', do_hw_ldac_toggle},
	{""},
	{"\tSet DAC Channel 0", 'A', do_dac_value, NULL, 0},
	{"\tSet DAC Channel 1", 'S', do_dac_value, NULL, 1},
	{"\tSet DAC Channel 2", 'D', do_dac_value, NULL, 2},
	{"\tSet DAC Channel 3", 'F', do_dac_value, NULL, 3},
	{"\tSet DAC Channel 4", 'G', do_dac_value, NULL, 4},
	{"\tSet DAC Channel 5", 'H', do_dac_value, NULL, 5},
};

static console_menu dac_operations_menu = {
	.title = "DAC Operations",
	.items = dac_operations_menu_items,
	.itemCount = ARRAY_SIZE(dac_operations_menu_items),
	.headerItem = display_dac_operations_header,
	.footerItem = NULL,
	.enableEscapeKey = true
};

/*
 * Monitor Setup Menu
 */
static console_menu_item dac_channel_configuration_menu_items[] = {
	{"\tToggle Channel 0 Enable", '0', do_channel_config, NULL, AD5770R_CHANNEL_CONFIG_CH0_SHUTDOWN_B(1)},
	{"\tToggle Channel 0 Sink Enable", 'S', do_channel_config, NULL, AD5770R_CHANNEL_CONFIG_CH0_SINK_EN(1)},
	{"\tToggle Channel 1 Enable", '1', do_channel_config, NULL, AD5770R_CHANNEL_CONFIG_CH1_SHUTDOWN_B(1)},
	{"\tToggle Channel 2 Enable", '2', do_channel_config, NULL, AD5770R_CHANNEL_CONFIG_CH2_SHUTDOWN_B(1)},
	{"\tToggle Channel 3 Enable", '3', do_channel_config, NULL, AD5770R_CHANNEL_CONFIG_CH3_SHUTDOWN_B(1)},
	{"\tToggle Channel 4 Enable", '4', do_channel_config, NULL, AD5770R_CHANNEL_CONFIG_CH4_SHUTDOWN_B(1)},
	{"\tToggle Channel 5 Enable", '5', do_channel_config, NULL, AD5770R_CHANNEL_CONFIG_CH5_SHUTDOWN_B(1)}
};

static console_menu dac_channel_configuration_menu = {
	.title = "DAC Channel Configuration",
	.items = dac_channel_configuration_menu_items,
	.itemCount = ARRAY_SIZE(dac_channel_configuration_menu_items),
	.headerItem = display_dac_channel_configuration_header,
	.footerItem = NULL,
	.enableEscapeKey = true
};

/*
 * Monitor Setup Menu
 */
static console_menu_item monitor_setup_menu_items[] = {
	{"Disable Monitoring", 'Q', do_monitor_setup, NULL, AD5770R_DISABLE},
	{"Enable Voltage Monitoring", 'W', do_monitor_setup, NULL, AD5770R_VOLTAGE_MONITORING},
	{"Enable Current Monitoring", 'E', do_monitor_setup, NULL, AD5770R_CURRENT_MONITORING},
	{"Enable Temperature Monitoring", 'R', do_monitor_setup, NULL, AD5770R_TEMPERATURE_MONITORING},
	{"", 								        '\00', NULL},
	{"Toggle Mux Buffer", 'M', do_monitor_setup, NULL, TOGGLE_MUX_BUFFER},
	{"Toggle Diode External Bias", 'X', do_monitor_setup, NULL, TOGGLE_DIODE_EXT_BIAS},
	{"", 								        '\00', NULL},
	{"\tSelect Channel 0", '0', do_monitor_setup, NULL, AD5770R_CH0 + MENU_CHANNEL_OFFSET},
	{"\tSelect Channel 1", '1', do_monitor_setup, NULL, AD5770R_CH1 + MENU_CHANNEL_OFFSET},
	{"\tSelect Channel 2", '2', do_monitor_setup, NULL, AD5770R_CH2 + MENU_CHANNEL_OFFSET},
	{"\tSelect Channel 3", '3', do_monitor_setup, NULL, AD5770R_CH3 + MENU_CHANNEL_OFFSET},
	{"\tSelect Channel 4", '4', do_monitor_setup, NULL, AD5770R_CH4 + MENU_CHANNEL_OFFSET},
	{"\tSelect Channel 5", '5', do_monitor_setup, NULL, AD5770R_CH5 + MENU_CHANNEL_OFFSET},
};

static console_menu monitor_setup_menu = {
	.title = "Monitor Setup",
	.items = monitor_setup_menu_items,
	.itemCount = ARRAY_SIZE(monitor_setup_menu_items),
	.headerItem = display_monitor_setup_header,
	.footerItem = NULL,
	.enableEscapeKey = true
};

/*
 * General Configuration Menu
 */
static console_menu_item general_configuration_menu_items[] = {
	{"Select Int/External Reference Resistor",  'R', do_toggle_ref_resistor},
	{""},
	{"Set Ext 2.50V Reference", 'A', do_set_reference, NULL, AD5770R_EXT_REF_2_5_V},
	{"Set Int 1.25V Reference, Vout: ON", 'S', do_set_reference, NULL, AD5770R_INT_REF_1_25_V_OUT_ON},
	{"Set Ext 1.25V Reference", 'D', do_set_reference, NULL, AD5770R_EXT_REF_1_25_V},
	{"Set Int 1.25V Reference, Vout: OFF", 'F', do_set_reference, NULL, AD5770R_INT_REF_1_25_V_OUT_OFF},
	{""},
	{" --Toggle Alarm Configuration bits --"},
	{"\tOpen Drain Enable", '0', do_set_alarm, NULL, AD5770R_ALARM_CONFIG_OPEN_DRAIN_EN(1)},
	{"\tThermal Shutdown Enable", '1', do_set_alarm, NULL, AD5770R_ALARM_CONFIG_THERMAL_SHUTDOWN_EN(1)},
	{"\tBackground CRC Enable", '2', do_set_alarm, NULL, AD5770R_ALARM_CONFIG_BACKGROUND_CRC_EN(1)},
	{"\tTemperature Warning Alarm Mask", '3', do_set_alarm, NULL, AD5770R_ALARM_CONFIG_TEMP_WARNING_ALARM_MASK(1)},
	{"\tOver Temperature Alarm Mask", '4', do_set_alarm, NULL, AD5770R_ALARM_CONFIG_OVER_TEMP_ALARM_MASK(1)},
	{"\tNegative Channel 0  Mask", '5', do_set_alarm, NULL, AD5770R_ALARM_CONFIG_NEGATIVE_CHANNEL0_ALARM_MASK(1)},
	{"\tIREF Fault Alarm Mask", '6', do_set_alarm, NULL, AD5770R_ALARM_CONFIG_IREF_FAULT_ALARM_MASK(1)},
	{"\tBackground CRC Alarm Mask", '7', do_set_alarm, NULL, AD5770R_ALARM_CONFIG_BACKGROUND_CRC_ALARM_MASK(1)},
};

static console_menu general_configuration_menu = {
	.title = "General Configuration",
	.items = general_configuration_menu_items,
	.itemCount = ARRAY_SIZE(general_configuration_menu_items),
	.headerItem = display_gen_config,
	.footerItem = NULL,
	.enableEscapeKey = true
};

/*
 * Definition of the Main Menu Items and menu itself
 */
static console_menu_item main_menu_items[] = {
	{"Initialize Device to User Configuration", 'I', do_device_init},
	{"Remove Device",                           'X', do_device_remove},
	{""},
	{"Do Software Reset", 'R', do_software_reset},
	{""},
	{"General Configuration...",                'G', do_general_configuration_menu},
	{"Monitor Setup...",                        'M', do_monitor_setup_menu},
	{""},
	{"DAC Channel Configuration...",            'C', do_dac_channel_configuration_menu},
	{"DAC Operations...",                       'D', do_dac_operations_menu},
};

console_menu ad5770r_main_menu = {
#if ACTIVE_DEVICE==GENERIC_AD5770R
	.title = "AD5770R Console App",
#elif ACTIVE_DEVICE==GENERIC_AD5772R
	.title = "AD5772R Terminal App",
#else
#error "Unsupported device"
#endif
	.items = main_menu_items,
	.itemCount = ARRAY_SIZE(main_menu_items),
	.headerItem = display_main_menu_header,
	.footerItem = display_main_menu_footer,
	.enableEscapeKey = false
};