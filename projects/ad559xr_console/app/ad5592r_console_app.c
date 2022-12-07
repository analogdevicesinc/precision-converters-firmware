/*!
 *****************************************************************************
 *   @file    ad5592r_console_app.c
 *   @brief   AD5592R console application interfaces
 *   @details This file is specific to ad5592r and ad5593r console menu application handle.
 *            The functions defined in this file performs the action
 *            based on user selected console menu.
 *
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
#include "ad5592r_configs.h"

#include "mbed_platform_support.h"
#include "no_os_error.h"
#include "no_os_gpio.h"
#include "no_os_i2c.h"
#include "no_os_spi.h"
#include "mbed_spi.h"
#include "mbed_i2c.h"

#include "ad5592r-base.h"
#include "ad5592r.h"
#include "ad5593r.h"

#include "ad5592r_console_app.h"

/******************************************************************************/
/************************* Macros & Constant Definitions **********************/
/******************************************************************************/
// vref_voltage can be defined as EXTERNAL_VREF_VOLTAGE or INTERNAL_VREF_VOLTAGE
// Change EXTERNAL_VREF_VOLTAGE if using supply other than 2.5V
#define EXTERNAL_VREF_VOLTAGE			2.5
float vref_voltage = EXTERNAL_VREF_VOLTAGE;

#define	AD5592R_CHANNEL(N)			    (N)
#define AD5592R_REG_ADC_SEQ_INCL(x)		NO_OS_BIT(x)
#define AD5592R_REG_PD_CHANNEL(x)		NO_OS_BIT(x)
#define AD5592R_GPIO(x)				    NO_OS_BIT(x)

#define TEMP_SAMPLE_SIZE			    5
#define CLEAR_CHANNEL_SELECTION			1000
#define MDELAY_TO_DISPLAY_INSTRUCTION   1000
#define TEMPERATURE_READBACK_CHANNEL	8

#define MAX_ADC_CODE				    4095.0
#define ADC_GAIN_LOW_CONVERSION_VALUE	2.654
#define ADC_GAIN_HIGH_CONVERSION_VALUE	1.327

/* Private Variables */
static struct ad5592r_dev sAd5592r_dev;

static const char *mode_names[] = {
	"Unused",
	"ADC\t",
	"DAC\t",
	"ADC+DAC",
	"GPI\t",
	"GPO\t",
};
static const char *offstate_names[] = {
	"Pulldown",
	"Low\t",
	"High\t",
	"Tristate"
};
static bool active_channel_selections[NUM_CHANNELS] = {
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false
};

static uint16_t adc_channels_in_seq = AD5592R_REG_ADC_SEQ_TEMP_READBACK;

/******************************************************************************/
/***************************** Function Declarations **************************/
/******************************************************************************/
static int32_t do_software_reset(uint32_t id);
static int32_t do_read_die_temp(uint32_t id);
static float die_temp_calculation(uint16_t adc_temp_code, bool adc_gain);
static void do_set_channel_modes(void);
static int32_t do_toggle_channel_selection(uint32_t channel);
static int32_t do_mode_selection(uint32_t mode);
static int32_t do_reset_channel_modes(uint32_t id);
static int32_t do_offstate_selection(uint32_t mode);
static int32_t do_channel_7_adc_indicator(uint32_t id);
static int32_t do_general_settings_toggle(uint32_t reg_bit_id);
static int32_t do_write_dac_value(uint32_t id);
static int32_t do_dac_input_reg_to_output(uint32_t id);
static int32_t do_toggle_ldac_mode(uint32_t id);
static int32_t do_toggle_dac_powerdown(uint32_t id);
static int32_t do_toggle_incl_in_seq(uint32_t id);
static int32_t do_read_adc_sequence(uint32_t id);

extern console_menu power_down_pin_select_menu;
extern console_menu config_channels_menu;
extern console_menu general_settings_menu;
extern console_menu dac_menu;
extern console_menu gpio_menu;
extern console_menu adc_menu;

/******************************************************************************/
/***************************** Function Definitions ***************************/
/******************************************************************************/

/*!
 * @brief	Initialize AD5592/3R. ACTIVE_DEVICE defined in app_config.h
 *@details	The device initialization varies depending on what ACTIVE_DEVICE is defined.
 *			Device is reset and default register map values written.
 *			SPI or I2C initialization occurs.
 * @return	0 in case of success, negative error code otherwise.
 */
int32_t ad5592r_app_initalization(void)
{
	int32_t status;

	memcpy(&sAd5592r_dev, &ad5592r_dev_user, sizeof(ad5592r_dev_user));

#if (ACTIVE_DEVICE == DEV_AD5593R)
	status = i2c_init(&sAd5592r_dev.i2c, &i2c_user_params);
	if (status != 0) {
		return (status);
	}
	status = ad5593r_init(&sAd5592r_dev, &ad5592r_user_param);
#else // Default to AD5592R device
	status = no_os_spi_init(&sAd5592r_dev.spi, &spi_user_params);
	if (status != 0) {
		return (status);
	}

	status = ad5592r_init(&sAd5592r_dev, &ad5592r_user_param);
#endif
	return status;
}

/*!
 * @brief   Performs software reset
 * @details	Writes to the reset register. Resets Ad5592r_dev configuration using
 *			configuration from ad5592r_reset_config.c SPI, I2C and ops are stored
 *			and restored after the reset.
 * @return	Menu status constant.
 */
static int32_t do_software_reset(uint32_t id)
{
	int32_t status;

	if ((status = ad5592r_software_reset(&sAd5592r_dev)) == 0) {
		// Save spi_desc field, i2c_desc and device ops settings as it is not reset
		struct no_os_spi_desc *spi_interface = sAd5592r_dev.spi;
		struct no_os_i2c_desc *i2c_interface = sAd5592r_dev.i2c;
		const struct ad5592r_rw_ops *dev_ops = sAd5592r_dev.ops;
		// Copy over the reset state of the device
		memcpy(&sAd5592r_dev, &ad5592r_dev_reset, sizeof(ad5592r_dev_reset));

		// Restore device ops
		sAd5592r_dev.ops = dev_ops;
		if (ACTIVE_DEVICE == DEV_AD5592R) {
			// Restore the spi_desc pointer field
			sAd5592r_dev.spi = spi_interface;
			printf(EOL " --- AD5592R Software Reset Successful---" EOL);
		} else {
			// Restore the i2c_desc pointer field
			sAd5592r_dev.i2c = i2c_interface;
			printf(EOL " --- AD5593R Reset Request Successful---" EOL);
		}
	} else {
		printf(EOL " *** Software Reset Failure: %d ***" EOL, status);
		adi_press_any_key_to_continue();
	}

	adi_press_any_key_to_continue();
	return (MENU_CONTINUE);
}

/*!
 * @brief	Prints the temperature of the die
 * @details	Sets the devices to perform a temperature readback.
 *			Performs a number of samples based on TEMP_SAMPLE_SIZE
 * @return	Menu status constant
 */
static int32_t do_read_die_temp(uint32_t id)
{
	uint16_t readback_reg[1] = { 0 };
	int32_t status = 0, ch_state = 0;
	float result = 0;

	ch_state = sAd5592r_dev.channel_modes[7];
	sAd5592r_dev.channel_modes[7] = CH_MODE_ADC;
	do_set_channel_modes();

	do {
		for (int8_t i = 0; i < TEMP_SAMPLE_SIZE; i++) {
			do {
				status = sAd5592r_dev.ops->read_adc(&sAd5592r_dev,
								    AD5592R_CHANNEL(8),
								    readback_reg);
			} while (0);
			if (status != 0) {
				// Break out of for loop if not successful
				break;
			}
			result += die_temp_calculation(readback_reg[0],
						       (AD5592R_REG_CTRL_ADC_RANGE & sAd5592r_dev.cached_gp_ctrl));
		}

		result /= TEMP_SAMPLE_SIZE;

		if (status == 0) {
			// Print average of samples
			printf(EOL " --- Temperature: %.1f*C --- " EOL, result);
		} else {
			printf(EOL " *** Error reading die temperature: %d **" EOL, status);
			break;
		}
	} while (0);

	sAd5592r_dev.channel_modes[7] = ch_state;
	do_set_channel_modes();

	adi_press_any_key_to_continue();
	return (MENU_CONTINUE);
}

/*!
 * @brief	Calculates the die temperature
 * @details	Based on conversion equation, die temperature is estimated
 * @param	adc_temp_code - data read from ADC readback frame
 *			adc_gain - status of adc_gain
 * @return	result
 */
static float die_temp_calculation(uint16_t adc_temp_code, bool adc_gain)
{
	float result = 0;

	// use different equation depending on gain
	if(adc_gain) {
		result = 25 + ((AD5592R_REG_ADC_SEQ_CODE_MSK(adc_temp_code) -
				((0.5 / (2 * vref_voltage)) * MAX_ADC_CODE)) /
			       (ADC_GAIN_HIGH_CONVERSION_VALUE * (2.5 / vref_voltage)));
	} else {
		result = 25 + ((AD5592R_REG_ADC_SEQ_CODE_MSK(adc_temp_code) -
				((0.5 / vref_voltage) * MAX_ADC_CODE)) /
			       (ADC_GAIN_LOW_CONVERSION_VALUE * (2.5 / vref_voltage)));
	}

	return result;
}

/*!
 * @brief	Set channel modes
 *
 * @details	The channels modes are set by passing the altered device
 *			struct into the ad5592r_set_channel_modes function. There the channels are
 *			set to desired modes.
 */
static void do_set_channel_modes(void)
{
	int32_t status;
	if ((status =  ad5592r_set_channel_modes(&sAd5592r_dev)) != 0) {
		printf(EOL "Error configuring Channels (%d)" EOL, status);
		adi_press_any_key_to_continue();
	}
}

/*!
 * @brief	Toggle channels currently selected
 * @details	The channels the user has currently selected are set here.
 *			These are the channels that will be altered by mode or offstate selection
 * @param	channel - A channel that the user wants to add to the currently selected channels
 * @return	Menu status constant
 */
static int32_t do_toggle_channel_selection(uint32_t channel)
{
	if (channel == CLEAR_CHANNEL_SELECTION) {
		for (uint8_t i = 0; i < sAd5592r_dev.num_channels; i++) {

			active_channel_selections[i] = false;
		}
	} else {
		active_channel_selections[channel] = !active_channel_selections[channel];
	}

	return (MENU_CONTINUE);
}

/*!
 * @brief	Mode selection
 * @details	The mode the users wishes to apply to the currently selected channels
 *			are selected here. do_set_channel_modes is called which sets the channels
 *			on the device.
 * @param	mode -The mode that the user wishes to apply to the selected channels
 * @return	Menu status constant
 */
static int32_t do_mode_selection(uint32_t mode)
{
	for (uint8_t i = 0; i < sAd5592r_dev.num_channels; i++) {
		if (active_channel_selections[i] == true) {
			sAd5592r_dev.channel_modes[i]	= mode;
		}
	}
	do_set_channel_modes();
	do_toggle_channel_selection(CLEAR_CHANNEL_SELECTION);
	return (MENU_CONTINUE);
}

/*!
 * @brief	Offstate selection
 * @details	The offstate the users wishes to apply to the currently selected channels
 *		    are selected here. do_set_channel_modes is called which sets the channels
 *		    on the device.
 * @param	The offstate that the user wishes to apply to the selected channels
 * @return	Menu status constant
 */
static int32_t do_offstate_selection(uint32_t mode)
{
	for (uint8_t i = 0; i < sAd5592r_dev.num_channels; i++) {
		if (active_channel_selections[i] == true) {
			sAd5592r_dev.channel_offstate[i]	= mode;
		}
	}
	do_set_channel_modes();
	do_toggle_channel_selection(CLEAR_CHANNEL_SELECTION);
	return (MENU_CONTINUE);
}

/*!
 * @brief	Reset Channel Modes
 * @details	This resets all channel modes to unused.
 * @return	Menu status constant
 */
static int32_t do_reset_channel_modes(uint32_t id)
{
	ad5592r_reset_channel_modes(&sAd5592r_dev);
	do_toggle_channel_selection(CLEAR_CHANNEL_SELECTION);
	return (MENU_CONTINUE);
}

/*!
 * @brief	Sets Channel 7 as ADC conversion indicator
 * @details	Channel 7 is set as a GPIO and the NOT BUSY bit is set in the GPIO
 *			write configuration register enabling channel 7 to be used as an indicator
 *			when ADC conversion are occurring. Channel 7 will go LOW when a conversion
 *			is occurring.
 *			***NOTE*** After selecting this Channel 7 will appear as GPO.
 *			***NOTE*** Ensure this is the last channel to be configured in order to
 *					     ensure preference will no be overwritten
 * @return	Menu status constant
 */
static int32_t do_channel_7_adc_indicator(uint32_t id)
{
	sAd5592r_dev.channel_modes[AD5592R_CHANNEL(7)] =
		((sAd5592r_dev.channel_modes[AD5592R_CHANNEL(7)] == CH_MODE_UNUSED)
		 ? CH_MODE_GPO : CH_MODE_UNUSED);
	do_set_channel_modes();
	do_general_settings_toggle(((AD5592R_REG_GPIO_OUT_EN << 12)
				    | AD5592R_REG_GPIO_OUT_EN_ADC_NOT_BUSY));
	do_toggle_channel_selection(CLEAR_CHANNEL_SELECTION);
	return (MENU_CONTINUE);
}

/*!
 * @brief	Toggle general setting
 * @details	Setting (reg_bit) in register (reg) is toggled
 * @param	reg_bit_id - Combined value containing register address and bit to toggle
 * @return	Menu status constant
 */
static int32_t do_general_settings_toggle(uint32_t reg_bit_id)
{
	uint8_t reg = (reg_bit_id >> 12);
	uint16_t reg_bit = (reg_bit_id & 0xFFF), readback_reg;
	int32_t status;

	if ((status = ad5592r_base_reg_read(&sAd5592r_dev, reg,
					    &readback_reg)) != 0) {
		printf(" *** Error Reading Setting Status (%x) *** " EOL, reg);
		adi_press_any_key_to_continue();
	} else if ((status = ad5592r_base_reg_write(&sAd5592r_dev, reg,
			     (reg_bit ^ readback_reg))) != 0) {
		printf(" *** Error  Toggling Setting (%x) *** " EOL, reg);
		adi_press_any_key_to_continue();
	}

	if (reg == AD5592R_REG_PD && reg_bit == AD5592R_REG_PD_EN_REF) {
		if ((AD5592R_REG_PD_EN_REF & (reg_bit ^ readback_reg))) {
			vref_voltage = INTERNAL_VREF_VOLTAGE;
		} else {
			vref_voltage = EXTERNAL_VREF_VOLTAGE;
		}
	}
	return (MENU_CONTINUE);
}

/*!
 * @brief      displays the general settings header
 */
static void display_general_setting_header(void)
{

	int32_t status = 0;
	uint16_t ctrl_reg_data = 0, pd_reg_data = 0;

	do {
		if ((status = ad5592r_base_reg_read(&sAd5592r_dev, AD5592R_REG_CTRL,
						    &ctrl_reg_data)) == 0) {
			sAd5592r_dev.cached_gp_ctrl = ctrl_reg_data;
		} else {
			printf(" *** Error reading register (%x) *** " EOL, AD5592R_REG_CTRL);
			adi_press_any_key_to_continue();
			break;
		}

		if ((status = ad5592r_base_reg_read(&sAd5592r_dev, AD5592R_REG_PD,
						    &pd_reg_data)) == 0) {
		} else {
			printf(" *** Error reading register (%x) *** " EOL, AD5592R_REG_PD);
			adi_press_any_key_to_continue();
			break;
		}
	} while(0);

	printf("\tSetting \tEnabled\t\tSetting \tEnabled"EOL);
	printf("\tEn Ref\t\t%s\t\tADC Gain\t%s"EOL,
	       (AD5592R_REG_PD_EN_REF & pd_reg_data)?"X":"\00",
	       (AD5592R_REG_CTRL_ADC_RANGE & ctrl_reg_data)?"X":"\00");
	printf("\tPC Buff\t\t%s\t\tPD All\t\t%s"EOL,
	       (AD5592R_REG_CTRL_ADC_PC_BUFF & ctrl_reg_data)?"X":"\00",
	       (AD5592R_REG_PD_PD_ALL & pd_reg_data) ? "X" : "\00");
	printf("\tBuff\t\t%s\t\tDAC Gain\t%s"EOL,
	       (AD5592R_REG_CTRL_ADC_BUFF_EN & ctrl_reg_data)?"X":"\00",
	       (AD5592R_REG_CTRL_DAC_RANGE & ctrl_reg_data)?"X":"\00");
	printf("\tLock Config\t%s\t\tWr All\t\t%s"EOL,
	       (AD5592R_REG_CTRL_CONFIG_LOCK & ctrl_reg_data)?"X":"\00",
	       (AD5592R_REG_CTRL_W_ALL_DACS & ctrl_reg_data)?"X":"\00");

}

/*!
 * @brief	DAC input register to DAC output
 * @details	Writes the data from the DAC input register to the DAC output.
 *			The LDAC mode is returned to write data to the input register only.
 * @return	Menu status constant
 */
static int32_t do_dac_input_reg_to_output(uint32_t id)
{
	int32_t status;
	if ((status = ad5592r_base_reg_write(&sAd5592r_dev, AD5592R_REG_LDAC,
					     AD5592R_REG_LDAC_INPUT_REG_OUT)) != 0) {
		printf("*** Error setting LDAC to write to output (%d) *** ", status);
		adi_press_any_key_to_continue();
	}
	sAd5592r_dev.ldac_mode = AD5592R_REG_LDAC_INPUT_REG_ONLY;
	return (MENU_CONTINUE);
}

/*!
 * @brief	User dac code
 * @details	Generate dac code that can be written to device from voltage provided by user
 * @param 	user_voltage - float value provided by user for voltage value to be set
 * @return	dac code value
 */
static uint16_t user_dac_code(float user_voltage)
{
	return (uint16_t) (((user_voltage) * MAX_ADC_CODE) / vref_voltage);
}

/*!
 * @brief	Code values to voltage
 * @details	Generate voltage based on code value
 * @param 	code - integer value used to generate voltage value
 * @return	float voltage value
 */
static float code_to_volts(int16_t code)
{
	return ((code / MAX_ADC_CODE) * vref_voltage);
}

/*!
 * @brief	Write DAC Values
 * @details	Write value specified by user to Channels selected by user in the DAC menu
 * @return	(MENU_CONTINUE)
 */
static int32_t do_write_dac_value(uint32_t id)
{
	int32_t status;
	uint16_t user_code = 0;
	float user_voltage = 2.5;

	printf(EOL "\tEnter voltage to write to selected DACs (0 - Vref) : " EOL);
	user_voltage = adi_get_decimal_float(5);
	user_voltage = user_voltage < 0 || user_voltage > 2.5 ?  0 : user_voltage;
	user_code = user_dac_code(user_voltage);

	for (uint8_t i = 0; i < NUM_CHANNELS; i++) {
		if (active_channel_selections[i]) {
			if ((status = sAd5592r_dev.ops->write_dac(&sAd5592r_dev, i,
					user_code)) != 0) {
				printf("*** Error writing DAC value to channel %d (%d) ***" EOL, i, status);
				adi_press_any_key_to_continue();
			}
			sAd5592r_dev.cached_dac[i] = user_code;
		}
	}
	return (MENU_CONTINUE);
}

/*!
 * @brief	Toggle LDAC Modes
 * @details	Toggles the LDAC mode variable between Immediate write to output
 *			and write values to input register
 * @return	Menu status constant
 */
static int32_t do_toggle_ldac_mode(uint32_t id)
{
	if (sAd5592r_dev.ldac_mode == AD5592R_REG_LDAC_INPUT_REG_ONLY) {
		sAd5592r_dev.ldac_mode = AD5592R_REG_LDAC_IMMEDIATE_OUT;
	} else {
		sAd5592r_dev.ldac_mode = AD5592R_REG_LDAC_INPUT_REG_ONLY;
	}
	return (MENU_CONTINUE);
}

/*!
 * @brief	Toggle DAC channels to power-down
 * @details	Toggles DAC channels that are powered down based on user selection
 * @return	Menu status constant
 */
static int32_t do_toggle_dac_powerdown(uint32_t id)
{
	int32_t status;
	uint16_t powerdown = 0;

	if ((status = ad5592r_base_reg_read(&sAd5592r_dev, AD5592R_REG_PD,
					    &powerdown)) != 0) {
		printf("*** Error Reading Power Down Config (%d)***" EOL, status);
		adi_press_any_key_to_continue();
	}

	for (uint8_t i = 0; i < NUM_CHANNELS; i++) {
		if (active_channel_selections[i]) {
			powerdown ^= AD5592R_REG_PD_CHANNEL(i);
		}
	}

	if ((status = ad5592r_base_reg_write(&sAd5592r_dev, AD5592R_REG_PD,
					     powerdown)) != 0) {
		printf("*** Error writing Power Down Config (%d)***" EOL, status);
		adi_press_any_key_to_continue();
	}

	return (MENU_CONTINUE);
}

/*!
 * @brief	Toggle Channels to include in ADC Sequence
 * @details	Toggles channels that are included in the ADC conversion sequence
 * @return	Menu status constant
 */
static int32_t do_toggle_incl_in_seq(uint32_t id)
{
	for (uint8_t i = 0; i < NUM_CHANNELS; i++) {
		if (active_channel_selections[i]) {
			adc_channels_in_seq ^= AD5592R_REG_ADC_SEQ_INCL(i);
		}
	}
	return (MENU_CONTINUE);
}

/*!
 * @brief	Read ADC Sequence
 * @details	The channels that are included in an ADC conversion sequence are read.
 *			For the number of channels in the sequence, the data is parsed, converted
 *			and printed.
 * @return	0 in case of success, negative error code otherwise.
 */
int32_t do_read_adc_sequence(uint32_t id)
{
	int32_t status;
	uint16_t adc_seq_data[9] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	uint16_t adc_code;
	uint8_t chan;
	float temperature, voltage;
	size_t samples;

	samples = no_os_hweight8(adc_channels_in_seq);

	if ((status = sAd5592r_dev.ops->multi_read_adc(&sAd5592r_dev,
			adc_channels_in_seq, adc_seq_data)) != 0) {
		printf("*** Error reading adc_sequencer (%d)***" EOL, status);
		adi_press_any_key_to_continue();
		return -EINVAL;
	}

	printf("\tCh \tCode \tVoltage \tdegC" EOL);

	for (uint8_t i = 0; i < samples; i++) {
		adc_code = AD5592R_REG_ADC_SEQ_CODE_MSK(adc_seq_data[i]);
		chan = ((adc_seq_data[i] & 0xF000) >> 12);

		if (chan == TEMPERATURE_READBACK_CHANNEL) {
			temperature = die_temp_calculation(adc_code,
							   (AD5592R_REG_CTRL_ADC_RANGE &
							    sAd5592r_dev.cached_gp_ctrl));
			printf("\tTemp \t%x \t   \t\t%.1f" EOL,
			       adc_code,
			       temperature);
		} else {
			voltage = code_to_volts(adc_code);
			printf("\t%d \t%x \t%.2f" EOL,
			       chan,
			       adc_code,
			       voltage
			      );
		}
	}
	adi_press_any_key_to_continue();
	return 0;
}

/*!
 * @brief	Set GPI
 * @details	GPIO channels that are selected, with the selection being stored in
 *			active_channel_selections array are set to GPIO Inputs. The selection is then cleared.
 * @return	Menu status constant
 */
static int32_t do_set_gpio_input(uint32_t id)
{
	int32_t status;

	for (uint8_t i = 0; i < NUM_CHANNELS; i++) {
		if (active_channel_selections[i] == true) {
			sAd5592r_dev.channel_modes[i] = CH_MODE_GPI;
			if ((status = ad5592r_gpio_direction_input
				      (&sAd5592r_dev, AD5592R_CHANNEL(i) )) != 0) {
				printf(" *** Error Setting GPIO Input on Channel %d (%d) ***" EOL, i, status);
				adi_press_any_key_to_continue();
			}
		}
	}
	do_toggle_channel_selection(CLEAR_CHANNEL_SELECTION);
	return (MENU_CONTINUE);
}

/*!
 * @brief	Set GPO
 * @details	GPIO channels that are selected, with the selection being stored in
 *			active_channel_selections array are set to GPIO Outputs with their
 *			output set LOW. The selection is then cleared.
 * @return	Menu status constant
 */
static int32_t do_set_gpio_output(uint32_t value)
{
	int32_t status;

	for (uint8_t i = 0; i < NUM_CHANNELS; i++) {
		if (active_channel_selections[i] == true) {
			sAd5592r_dev.channel_modes[i] = CH_MODE_GPO;
			if ((status = ad5592r_gpio_direction_output
				      (&sAd5592r_dev, AD5592R_CHANNEL(i), NO_OS_GPIO_LOW)) != 0) {
				printf(" *** Error Setting GPIO Output on channel %d (%d) ***" EOL, i,status);
				adi_press_any_key_to_continue();
			}
		}
	}
	do_toggle_channel_selection(CLEAR_CHANNEL_SELECTION);
	return (MENU_CONTINUE);
}

/*!
 * @brief	Toggle GPO
 * @details	GPIO channels that are selected, with the selection being stored in
 *			active_channel_selections array are set to GPIO Outputs with their
 *			output set toggled HIGH and LOW. The selection is then cleared.
 * @return	Menu status constant
 */
static int32_t do_toggle_gpio_output(uint32_t id)
{
	int32_t status;

	for (uint8_t i = 0; i < NUM_CHANNELS; i++) {
		if (active_channel_selections[i] == true) {
			if ((status = ad5592r_gpio_set(&sAd5592r_dev,
						       AD5592R_CHANNEL(i),
						       !ad5592r_gpio_get(&sAd5592r_dev, AD5592R_CHANNEL(i))) != 0)) {
				printf(" *** Error Toggling GPIO Output on Channel %d (%d) ***", i, status);
				adi_press_any_key_to_continue();
			}
		}
	}
	do_toggle_channel_selection(CLEAR_CHANNEL_SELECTION);
	return (MENU_CONTINUE);
}

/*!
 * @brief      calls the general configuration menu
 */
static int32_t menu_general_settings(uint32_t id)
{
	return adi_do_console_menu(&general_settings_menu);
}

/*!
 * @brief      calls the DAC configuration menu
 */
static int32_t menu_dac(uint32_t id)
{
	return adi_do_console_menu(&dac_menu);
}

/*!
 * @brief      calls the channel configuration menu
 */
static int32_t menu_config_channels(uint32_t id)
{
	return adi_do_console_menu(&config_channels_menu);
}

/*!
 * @brief      calls the ADC configuration menu
 */
static int32_t menu_adc(uint32_t id)
{
	return adi_do_console_menu(&adc_menu);
}

/*!
 * @brief      calls the menu to select GPIO pins to toggle
 */
static int32_t menu_gpio(uint32_t id)
{
	return adi_do_console_menu(&gpio_menu);
}

/*!
 * @brief      displays the channel configuration header
 */
static void display_channel_selection_header(void)
{
	printf(" Configuration Lock: %s" EOL,
	       (AD5592R_REG_CTRL_CONFIG_LOCK & sAd5592r_dev.cached_gp_ctrl)
	       ?"Enabled":"Disabled");

	printf("\tCh\tMode\t\tOffstate\tSelected" EOL);
	for (uint8_t i = 0; i < sAd5592r_dev.num_channels; i++) {
		printf("\t%d \t%s  \t%s \t\t%s" EOL,
		       i,
		       mode_names[sAd5592r_dev.channel_modes[i]],
		       offstate_names[sAd5592r_dev.channel_offstate[i]],
		       active_channel_selections[i]?"X":"\00" );
	}
}

/*!
 * @brief      displays the gpio menu header
 */
static void display_gpio_menu_header(void)
{
	printf("\tCh\tDir \tValue\tSelected" EOL);

	for (uint8_t i = 0; i < sAd5592r_dev.num_channels; i++) {

		printf("\t%d \t%s%s \t%s \t%s" EOL,
		       i,
		       (sAd5592r_dev.gpio_in &  AD5592R_GPIO(i)) ? "In " : "",
		       (sAd5592r_dev.gpio_out & AD5592R_GPIO(i)) ? "Out " : "",
		       ad5592r_gpio_get(&sAd5592r_dev, AD5592R_CHANNEL(i)) ? "High" : "Low",
		       active_channel_selections[i] ? "X" : "\00"
		      );
	}
}

/*!
 * @brief      displays the DAC menu header
 */
static void display_dac_menu_header(void)
{
	int32_t status;
	float voltage;
	uint16_t powerdown_read;
	char *dac_channel_state = "";

	printf("\tLDAC mode: %s" EOL EOL,
	       sAd5592r_dev.ldac_mode ? "Write to Input Register": "Immediate Output");

	printf("\tCH \tConfig \tCode \tVoltage \tSelected" EOL);

	if ((status = ad5592r_base_reg_read(&sAd5592r_dev,
					    AD5592R_REG_PD,
					    &powerdown_read))  != 0) {
		printf("*** Error checking Power Down status (%d) ***" EOL, status);
		adi_press_any_key_to_continue();
	}

	for (uint8_t i = 0; i < NUM_CHANNELS; i++) {
		voltage = 0;
		switch (sAd5592r_dev.channel_modes[i]) {
		case CH_MODE_DAC:
		case CH_MODE_DAC_AND_ADC:
			if (powerdown_read &  AD5592R_REG_PD_CHANNEL(i)) {
				dac_channel_state = "PD";
			} else {
				dac_channel_state = "DAC";
				voltage = code_to_volts(sAd5592r_dev.cached_dac[i]);
			}
			break;
		default:
			dac_channel_state = "-";
			// Channel no longer set as DAC - Clear cached value
			sAd5592r_dev.cached_dac[i] = 0;
			break;
		}

		printf("\t%d \t%s \t%x  \t%.2fV \t\t%s" EOL,
		       i,
		       dac_channel_state,
		       sAd5592r_dev.cached_dac[i],
		       voltage,
		       active_channel_selections[i]?"X":"\00");
	}
}

/*!
 * @brief      displays the Main menu header
 */
static void display_main_menu_header(void)
{
	printf("\tCurrent Channel Configuration:" EOL);
	printf("\tCH \tMode " EOL);
	for (uint8_t i = 0; i < NUM_CHANNELS; i++) {
		printf("\t%d \t%s" EOL,
		       i,
		       mode_names[sAd5592r_dev.channel_modes[i]]);
	}
}

/*!
 * @brief      displays the ADC menu header
 */
static void display_adc_menu_header(void)
{
	char *adc_channel_state = "";

	printf("\tCh \tMode \tIncl \tSelected" EOL);

	for (uint8_t i = 0; i < NUM_CHANNELS; i++) {
		switch (sAd5592r_dev.channel_modes[i]) {
		case CH_MODE_ADC:
		case CH_MODE_DAC_AND_ADC:
			adc_channel_state = "ADC";

			break;
		default:
			adc_channel_state = "-";
			break;
		}

		printf("\t%d \t%s \t%s \t%s" EOL,
		       i,
		       adc_channel_state,
		       (adc_channels_in_seq & AD5592R_REG_ADC_SEQ_INCL(i)) ?"X":"",
		       active_channel_selections[i]?"X":""
		      );
	}
}

/*
 * Definition of the menu of pins to include in adc sequence and menu itself
 */
console_menu_item gpio_menu_items[] = {
	{ "Select Channel", '\00', NULL, NULL },
	{ "Channel 0", 'A', do_toggle_channel_selection, NULL, AD5592R_CHANNEL(0) },
	{ "Channel 1", 'S', do_toggle_channel_selection, NULL, AD5592R_CHANNEL(1) },
	{ "Channel 2", 'D', do_toggle_channel_selection, NULL, AD5592R_CHANNEL(2) },
	{ "Channel 3", 'F', do_toggle_channel_selection, NULL, AD5592R_CHANNEL(3) },
	{ "Channel 4", 'G', do_toggle_channel_selection, NULL, AD5592R_CHANNEL(4) },
	{ "Channel 5", 'H', do_toggle_channel_selection, NULL, AD5592R_CHANNEL(5) },
	{ "Channel 6", 'J', do_toggle_channel_selection, NULL, AD5592R_CHANNEL(6) },
	{ "Channel 7", 'K', do_toggle_channel_selection, NULL, AD5592R_CHANNEL(7) },
	{ "", '\00', NULL, NULL },
	{ "Set as GPIO Input", 'Z', do_set_gpio_input, NULL, 0},
	{ "Set as GPIO Output", 'X', do_set_gpio_output, NULL, 0},
	{ "Toggle Output Value", 'C', do_toggle_gpio_output, NULL, 0},
};

console_menu gpio_menu = {
	.title = "GPIO Menu" EOL,
	.items = gpio_menu_items,
	.itemCount = ARRAY_SIZE(gpio_menu_items),
	.headerItem = display_gpio_menu_header,
	.footerItem = NULL,
	.enableEscapeKey = true
};

/*
 * Definition of the ADC config menu and menu itself
 */
console_menu_item adc_menu_items[] = {
	{ "Select channels:" },
	{ "Channel 0", 'A', do_toggle_channel_selection, NULL, AD5592R_CHANNEL(0) },
	{ "Channel 1", 'S', do_toggle_channel_selection, NULL, AD5592R_CHANNEL(1) },
	{ "Channel 2", 'D', do_toggle_channel_selection, NULL, AD5592R_CHANNEL(2) },
	{ "Channel 3", 'F', do_toggle_channel_selection, NULL, AD5592R_CHANNEL(3) },
	{ "Channel 4", 'G', do_toggle_channel_selection, NULL, AD5592R_CHANNEL(4) },
	{ "Channel 5", 'H', do_toggle_channel_selection, NULL, AD5592R_CHANNEL(5) },
	{ "Channel 6", 'J', do_toggle_channel_selection, NULL, AD5592R_CHANNEL(6) },
	{ "Channel 7", 'K', do_toggle_channel_selection, NULL, AD5592R_CHANNEL(7) },
	{ "", '\00', NULL, NULL},
	{ "Toggle Channels in Sequence", 'Q', do_toggle_incl_in_seq },
	{ "Read ADC Sequence", 'W', do_read_adc_sequence},
};

console_menu adc_menu = {
	.title = "ADC Configuration Settings",
	.items = adc_menu_items,
	.itemCount = ARRAY_SIZE(adc_menu_items),
	.headerItem = display_adc_menu_header,
	.footerItem = NULL,
	.enableEscapeKey = true
};

/*
 * Definition of the DAC menu and menu itself
 */
console_menu_item dac_menu_items[] = {
	{ "Select Channels:"},
	{ "Channel 0", 'A', do_toggle_channel_selection, NULL, AD5592R_CHANNEL(0) },
	{ "Channel 1", 'S', do_toggle_channel_selection, NULL, AD5592R_CHANNEL(1) },
	{ "Channel 2", 'D', do_toggle_channel_selection, NULL, AD5592R_CHANNEL(2) },
	{ "Channel 3", 'F', do_toggle_channel_selection, NULL, AD5592R_CHANNEL(3) },
	{ "Channel 4", 'G', do_toggle_channel_selection, NULL, AD5592R_CHANNEL(4) },
	{ "Channel 5", 'H', do_toggle_channel_selection, NULL, AD5592R_CHANNEL(5) },
	{ "Channel 6", 'J', do_toggle_channel_selection, NULL, AD5592R_CHANNEL(6) },
	{ "Channel 7", 'K', do_toggle_channel_selection, NULL, AD5592R_CHANNEL(7) },
	{ "", '\00', NULL },
	{ "Write voltage to selected DAC channels", 'Q', do_write_dac_value, NULL, 0},
	{ "Toggle Power Down selected DAC channels", 'W', do_toggle_dac_powerdown, NULL, 0},
	{ "Write Input Reg to DAC output", 'E', do_dac_input_reg_to_output, NULL, 0},
	{ "Toggle LDAC mode", 'R', do_toggle_ldac_mode, NULL, 0},
};

console_menu dac_menu = {
	.title = "DAC Menu",
	.items = dac_menu_items,
	.itemCount = ARRAY_SIZE(dac_menu_items),
	.headerItem = display_dac_menu_header,
	.footerItem = NULL,
	.enableEscapeKey = true
};

/*
 * Definition of the General Settings menu and menu itself
 */
console_menu_item general_settings_menu_items[] = {
	{
		"Toggle Internal Voltage Ref (En Ref)",     'A', do_general_settings_toggle,
		NULL, ((AD5592R_REG_PD << 12) | AD5592R_REG_PD_EN_REF)
	},
	{
		"Toggle ADC PreCharge Buffer  (PC Buff)",   'S', do_general_settings_toggle,
		NULL, ((AD5592R_REG_CTRL << 12) | AD5592R_REG_CTRL_ADC_PC_BUFF)
	},
	{
		"Toggle ADC Buffer (Buff)",                 'D', do_general_settings_toggle,
		NULL, ((AD5592R_REG_CTRL << 12) | AD5592R_REG_CTRL_ADC_BUFF_EN)
	},
	{
		"Toggle Lock Channel Config (Lock Config)", 'F', do_general_settings_toggle,
		NULL, ((AD5592R_REG_CTRL << 12) | AD5592R_REG_CTRL_CONFIG_LOCK)
	},
	{
		"Toggle PD All DACs and Internal Ref",      'G', do_general_settings_toggle,
		NULL, ((AD5592R_REG_PD << 12) | AD5592R_REG_PD_PD_ALL)
	},
	{
		"Toggle ADC Gain Range (ADC Gain)",         'H', do_general_settings_toggle,
		NULL, ((AD5592R_REG_CTRL << 12) | AD5592R_REG_CTRL_ADC_RANGE)
	},
	{
		"Toggle DAC Gain Range (DAC Gain)",         'J', do_general_settings_toggle,
		NULL, ((AD5592R_REG_CTRL << 12) | AD5592R_REG_CTRL_DAC_RANGE)
	},
	{
		"Toggle Write All DACS (Wr All)",           'K', do_general_settings_toggle,
		NULL, ((AD5592R_REG_CTRL << 12) | AD5592R_REG_CTRL_W_ALL_DACS)
	},
};

console_menu general_settings_menu = {
	.title = "General Configuration Settings",
	.items = general_settings_menu_items,
	.itemCount = ARRAY_SIZE(general_settings_menu_items),
	.headerItem = display_general_setting_header,
	.footerItem = NULL,
	.enableEscapeKey = true
};

/*
 * Definition of the Channel  mode selection menu and menu itself
 */
console_menu_item config_channels_menu_items[] = {
	{ "Select Channels:"},
	{ "Channel 0", 'A', do_toggle_channel_selection, NULL, AD5592R_CHANNEL(0) },
	{ "Channel 1", 'S', do_toggle_channel_selection, NULL, AD5592R_CHANNEL(1) },
	{ "Channel 2", 'D', do_toggle_channel_selection, NULL, AD5592R_CHANNEL(2) },
	{ "Channel 3", 'F', do_toggle_channel_selection, NULL, AD5592R_CHANNEL(3) },
	{ "Channel 4", 'G', do_toggle_channel_selection, NULL, AD5592R_CHANNEL(4) },
	{ "Channel 5", 'H', do_toggle_channel_selection, NULL, AD5592R_CHANNEL(5) },
	{ "Channel 6", 'J', do_toggle_channel_selection, NULL, AD5592R_CHANNEL(6) },
	{ "Channel 7", 'K', do_toggle_channel_selection, NULL, AD5592R_CHANNEL(7) },
	{ "", '\00', NULL },
	{ "DAC", 'Q', do_mode_selection, NULL, CH_MODE_DAC },
	{ "ADC", 'W', do_mode_selection, NULL, CH_MODE_ADC },
	{ "ADC + DAC", 'E', do_mode_selection, NULL, CH_MODE_DAC_AND_ADC },
	{ "GPI", 'R', do_mode_selection, NULL, CH_MODE_GPI },
	{ "GPO", 'T', do_mode_selection, NULL, CH_MODE_GPO },
	{ "Unused", 'Y', do_mode_selection, NULL, CH_MODE_UNUSED },
	{ "Restore Default Modes", 'U', do_reset_channel_modes, NULL },
	{ "", '\00', NULL },
	{ "Pulldown", 'Z', do_offstate_selection, NULL, CH_OFFSTATE_PULLDOWN },
	{ "Output Low", 'X', do_offstate_selection, NULL, CH_OFFSTATE_OUT_LOW },
	{ "Output High", 'C', do_offstate_selection, NULL, CH_OFFSTATE_OUT_HIGH },
	{ "Tristate", 'V', do_offstate_selection, NULL, CH_OFFSTATE_OUT_TRISTATE },
	{ "", '\00', NULL },
	{ "Channel 7 as ADC conversion indicator (AD5592R)", 'M', do_channel_7_adc_indicator, NULL },
};

console_menu config_channels_menu = {
	.title = "Configure IO Channels",
	.items = config_channels_menu_items,
	.itemCount = ARRAY_SIZE(config_channels_menu_items),
	.headerItem = display_channel_selection_header,
	.footerItem = NULL,
	.enableEscapeKey = true
};

/*
 * Definition of the Main Menu Items and menu itself
 */
console_menu_item main_menu_items[] = {
	{ "Software Reset", 'Q', do_software_reset, NULL },
	{ "Read ADC die temp", 'W', do_read_die_temp, NULL },
	{ "", '\00', NULL },
	{ "Configure Channels", 'A', menu_config_channels, NULL },
	{ "General Settings", 'S', menu_general_settings, NULL },
	{ "DAC Menu", 'D', menu_dac, NULL },
	{ "ADC Menu", 'F', menu_adc, NULL },
	{ "GPIO Menu", 'G', menu_gpio, NULL },
};

console_menu ad5592r_main_menu = {
#if ACTIVE_DEVICE == DEV_AD5593R
	.title = "AD5593R Main Menu",
#else // Default to AD5592R device
	.title = "AD5592R Main Menu",
#endif
	.items = main_menu_items,
	.itemCount = ARRAY_SIZE(main_menu_items),
	.headerItem = display_main_menu_header,
	.footerItem = NULL,
	.enableEscapeKey = NULL
};
