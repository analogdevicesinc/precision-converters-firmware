/***************************************************************************//**
 *   @file    ad590_console_app.c
 *
 *   @brief   ad590 console application interfaces
 *
 *   @details This file is specific to ad590 console menu application handle.
 *            The functions defined in this file performs the action
 *            based on user selected console menu.
 *
********************************************************************************
 * Copyright (c) 2021-22 Analog Devices, Inc.
 *
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
#include <stdbool.h>
#include <ctype.h>

#include "ltc2488_user_config.h"

#include "mbed_platform_support.h"
#include "no_os_delay.h"

#include "ltc2488.h"
#include "ad590_console_app.h"

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

#define MIN_SAMPLES           1
#define MAX_SAMPLES           1000
#define MIN_SAMPLE_INTERVAL   0.2
#define MAX_SAMPLE_INTERVAL   60.0
#define MAX_RETRY_ATTEMPTS    3
#define RESISTOR_DIVIDER_FACTOR    2
#define TRANSFER_FUNCTION_CONSTANT 100
#define TEMP_CONVERSION_CONSTANT 273.15

/* Comment out this line to print new sample on each line of screen */
#define PRINT_IN_SINGLE_LINE  1

/* Pointer to the structure representing the LTC2488 device */
static struct ltc2488_dev *p_ltc2488_dev = NULL;
/* ADC command word for channel selection */
static uint8_t adc_cmd;
/* ADC output data buffer */
static uint32_t adc_data;
/* Flag to discard first ADC sample after power-on reset */
static bool first_sample = false;

/******************************************************************************/
/***************************** Function Declarations **************************/
/******************************************************************************/
static void print_title(void);
static void print_conv_stop(void);
static void print_invalid_input(void);
static bool was_escape_key_pressed(void);
static void handle_read_write_print(uint8_t move_cursor_up);
static int32_t continuous_measure(uint32_t menu_id);
static int32_t interval_measure(uint32_t menu_id);
static bool float_in_range(float input_val,
			   float lowest_accepted_val,
			   float highest_accepted_val);
static int32_t menu_read_write(uint32_t menu_id);
static float voltage_to_temp_conversion(float channel_voltage);

/******************************************************************************/
/***************************** Function Definitions ***************************/
/******************************************************************************/
/**
 * @brief 	Initialize the LTC2488 device and user configurations
 * @return	LTC2488 device initialization status
 */
int32_t ltc2488_app_initialize(void)
{
	int32_t init_status;

	/* Initialize LTC2488 device and peripheral interface */
	init_status = ltc2488_init(&p_ltc2488_dev, &ltc2488_init_str);
	if (init_status) {
		printf(EOL "SPI initialization Error");
	}

	return init_status;
}

/**
 * @brief 	Prints the title block.
 * @return	none
 */
static void print_title(void)
{
	printf("*****************************************************************" EOL);
	printf("*             EVAL-AD590-ARDZ Demonstration Program             *" EOL);
	printf("*                                                               *" EOL);
	printf("*  This program demonstrates High-Accuracy temperature sensing  *" EOL);
	printf("*  capabilities of the AD590 using the on-board ADC             *" EOL);
	printf("*  LTC2488 communicating with SDP-K1 over SPI.                  *" EOL);
	printf("*                                                               *" EOL);
	printf("*****************************************************************" EOL);
}

/**
 * @brief 	Prints the instructions to stop ADC conversion
 * @return	none
 */
static void print_conv_stop(void)
{
	printf(EOL "" EOL);
	printf("*****************************************************************" EOL);
	printf("                  Press [ESC] to Stop Conversion                 " EOL);
	printf("*****************************************************************" EOL);
}

/**
 * @brief 	Prints the warning message for invalid input
 * @return	none
 */
static void print_invalid_input(void)
{
	printf(EOL "Please enter a valid selection" EOL);
	no_os_mdelay(2000);
	/* Moves the cursor 2 lines up and clears the entire line*/
	printf(VT100_MOVE_UP_N_LINES VT100_CLEAR_CURRENT_LINE,(uint8_t)2);
}

/*!
 * @brief	determines if the Escape key was pressed
 * @return	bool - key press status
 */
static bool was_escape_key_pressed(void)
{
	char rxChar;
	bool wasPressed = false;

	/* Check for Escape key pressed */
	if ((rxChar = getchar_noblock()) > 0) {
		if (rxChar == ESCAPE_KEY_CODE) {
			wasPressed = true;
		}
	}
	return (wasPressed);
}

/**
 * @brief   Handles read/write operations to/from ADC and prints out the desired
 *		    output to the serial console.
 * @param 	move_cursor_up - no of lines to shift the cursor
 * @return  none
 */
static void handle_read_write_print(uint8_t move_cursor_up)
{
	int32_t init_status, sign_extend;
	enum input_status input_range_state;
	float channel_voltage;

	/* The channel configuration on the LTC2488 are programmed on the fly by
	 * sending a channel command word with each SPI read_write transaction.
	 * The first sample after power on reset results in a stale value as device
	 * doesn't have information on the channel configuration and by default the
	 * LTC2488 would sample data with following input channel configurations:
	 * IN+ = CH0, IN– = CH1. Hence we receive a stale value in the order of
	 * millivolts.
	 * For this specific application we need the IN- pin is to be latched
	 * to COM pin by the multiplexer. Since we cannot configure the channels
	 * for the first sample, so we simply discard the first sample after POR
	 * of the SDP-K1.
	 * Note: It is hereby assumed that the POR events of SDP-K1 and LTC2488
	 * are mutually inclusive and cannot happen independently.
	 * */
	if (first_sample == false) {
		init_status = ltc2488_read_write(p_ltc2488_dev->spi_desc, adc_cmd, &adc_data);
		no_os_mdelay(LTC2488_CHANNEL_CONV_TIME);
		first_sample = true;
	}

	/* SPI Read Write Operation */
	init_status = ltc2488_read_write(p_ltc2488_dev->spi_desc, adc_cmd, &adc_data);
	if (init_status < 0)
		printf(EOL "SPI read_write error");

	/* Processing the data received from ADC */
	input_range_state = ltc2488_data_process(&adc_data, &sign_extend);

	if (LTC2488_EOC_DETECT(adc_data)) {
		switch (input_range_state) {
		case OVER_RANGE :
			printf(" OVER Voltage Detected" EOL);
			break ;

		case UNDER_RANGE :
			printf(" Under Voltage Detected" EOL);
			break ;

		default :  /* ADC value between -0.5*Vref and +0.5*Vref */
			channel_voltage = ltc2488_code_to_voltage(&sign_extend);
			printf("Temperature: %0.2f C" EOL, voltage_to_temp_conversion(channel_voltage));
		}
	} else
		printf(" Conversion still in process" EOL);

	/* Moves cursor 2 lines up, and clear clears the entire line */
#ifdef PRINT_IN_SINGLE_LINE
	printf(VT100_MOVE_UP_N_LINES VT100_CLEAR_CURRENT_LINE, move_cursor_up);
#endif
}

/*!
 * @brief	Continuous measurement of Voltage from the select device connected
 *			to a specific input channel
 * @param	menu_id - Optional menu ID parameter
 * @return	Menu status constant
 */
static int32_t continuous_measure(uint32_t menu_id)
{
	print_conv_stop();
	while (!was_escape_key_pressed()) {
		/* wait for the conversion to complete */
		no_os_mdelay(LTC2488_CHANNEL_CONV_TIME);
		handle_read_write_print(1);
	}

	return MENU_CONTINUE;
}

/*!
 * @brief	Measurement of Voltage from the select device connected to
 *	        a specific input channel periodically for a specified time
 *	        interval
 * @param	menu_id - Optional menu ID parameter
 * @return	Menu status constant
 */
static int32_t interval_measure(uint32_t menu_id)
{
	uint16_t samples;
	float interval;
	uint8_t max_try = 0;

	printf(EOL);
	do {
		printf("Enter the number of samples (%d-%d): ", (uint8_t)MIN_SAMPLES,
		       (uint16_t)MAX_SAMPLES);
		/* Restricting the input length to 4 characters */
		samples = adi_get_decimal_int(4);
		if (!float_in_range(samples, MIN_SAMPLES, MAX_SAMPLES)) {
			if (max_try == MAX_RETRY_ATTEMPTS)
				goto retry_exceed;
			print_invalid_input();
		} else {
			max_try = 0;
			break;
		}
	} while (++max_try <= MAX_RETRY_ATTEMPTS);

	printf(EOL);
	do {
		printf("Enter the interval between each sample in seconds (%0.1f-%2.1f): ",
		       (float)MIN_SAMPLE_INTERVAL,(float)MAX_SAMPLE_INTERVAL);
		/* Restricting the input length to 4 characters */
		interval = adi_get_decimal_float(4);
		if (!float_in_range(interval, MIN_SAMPLE_INTERVAL, MAX_SAMPLE_INTERVAL)) {
			if (max_try == MAX_RETRY_ATTEMPTS)
				goto retry_exceed;
			print_invalid_input();
		} else
			break;
	} while (++max_try <= MAX_RETRY_ATTEMPTS);

	print_conv_stop();
	for(uint16_t i = 0 ; i < samples ; i++) {
		if (was_escape_key_pressed())
			break;

		printf(EOL " Sample :%d  ",i+1);
		handle_read_write_print(2);
		no_os_mdelay(interval * 1000);
	}
	return MENU_CONTINUE;

retry_exceed:
	printf(EOL "Maximum try limit exceeded" EOL);
	adi_press_any_key_to_continue();
}

/*!
 * @brief	Checks if an input is within a valid range
 * @param	input_val - Value inputed by user
 * @param   lowest_accepted_val - Lowest acceptable value
 * @param   highest_accepted_val - Highest acceptable value
 * @return	true if within range otherwise false
 */
static bool float_in_range(float input_val,
			   float lowest_accepted_val,
			   float highest_accepted_val)
{
	if ((input_val >= lowest_accepted_val) && (input_val <= highest_accepted_val))
		return true;
	else
		return false;
}

/*
 * Definition of the Main Menu Items and menu itself
 */
static console_menu_item cmd_menu_items[] = {
	{ "Read Temperature ( Continuous Mode )", '1', continuous_measure},
	{ "" },
	{ "Read Temperature ( Interval Mode )", '2', interval_measure},
	{ "" },
};

console_menu ad590_cmd_menu = {
	.title = EOL "Set Operation Type" EOL,
	.items = cmd_menu_items,
	.itemCount = ARRAY_SIZE(cmd_menu_items),
	.headerItem = NULL,
	.footerItem = NULL,
	.enableEscapeKey = true
};

/*!
 * @brief	Display and handle console menu for interfacing with internal
 *			and external ad590 temperature sensor.
 * @param   menu_id - Optional menu ID parameter
 * @return	Menu status constant
 */
static int32_t menu_read_write(uint32_t menu_id)
{
	adc_cmd = LTC2488_CHANNEL_CONF_ENABLE | menu_id;
	printf(EOL "Selection Made: %ld" EOL, menu_id);
	return (adi_do_console_menu(&ad590_cmd_menu));
}

/**
 * @brief   Converts the input channel voltage to temperature in Celsius
 * @param   channel_voltage - ADC Channel Voltage value
 * @return  temperature equivalent
 */
static float voltage_to_temp_conversion(float channel_voltage)
{
	float temp = (channel_voltage * TRANSFER_FUNCTION_CONSTANT *
		      RESISTOR_DIVIDER_FACTOR) - TEMP_CONVERSION_CONSTANT;
	return temp;
}

/*
 * Definition of the Main Menu Items and menu itself
 * */
static console_menu_item main_menu_items[] = {
	{ " AD590      ( On-board Sensor ) ", 'A', menu_read_write, NULL, LTC2488_SINGLE_CH3},
	{ "" },
	{ " AD590      ( External Remote Sensor ) ", 'B', menu_read_write, NULL, LTC2488_SINGLE_CH2},
};

console_menu ad590_main_menu = {
	.title = EOL "Please select a device:" EOL,
	.items = main_menu_items,
	.itemCount = ARRAY_SIZE(main_menu_items),
	.headerItem = print_title,
	.footerItem = NULL,
	.enableEscapeKey = false
};
