﻿/*************************************************************************//**
 *   @file   main.c
 *   @brief  Main application code for AD5933 firmware example program
******************************************************************************
* Copyright (c) 2019-2022, 2025 Analog Devices, Inc.
*
* All rights reserved.
*
* This software is proprietary to Analog Devices, Inc. and its licensors.
* By using this software you agree to the terms of the associated
* Analog Devices Software License Agreement.
*****************************************************************************/

#include <ctype.h>
#include <limits.h>
#include "no_os_i2c.h"
#include "no_os_uart.h"
#include "no_os_error.h"
#include "no_os_delay.h"
#include "app_config.h"

//Lower this value if storage becomes a problem
#define MAX_FREQ_INCREMENTS 511
#define TEMP_LIMIT_MIN -40
#define TEMP_LIMIT_MAX 125
#define MAX_SETTLING_CYCLES  511
#define EOL "\r\n"

static void print_title(void);
static void getMenuSelect(uint8_t *menuSelect);
static void print_prompt();
static uint8_t read_temperature();
static uint8_t set_system_clock();
static uint8_t set_vrange_and_pga_gain();
static int32_t configure_system();
static uint8_t calculate_gain_factor();
static uint8_t guide();
static uint8_t impedance_sweep();

typedef struct ad5933_config_data {
	float 	start_freq;
	uint8_t pga_gain;
	float 	output_voltage_range;
	int32_t start_frequency;
	int32_t frequency_increment;
	int32_t number_increments;
	int32_t number_settling_cycles;
} ad5933_config_data;

ad5933_config_data config_data;

static double gain_factor = 0;
static double temperature;


/******************************************************************************/
/************************** Variables Declarations ****************************/
/******************************************************************************/

/* UART Descriptor */
struct no_os_uart_desc *uart_desc;

struct no_os_uart_init_param uart_init_params = {
	.device_id = 0,
	.baud_rate = 230400,
	.size = NO_OS_UART_CS_8,
	.parity = NO_OS_UART_PAR_NO,
	.stop = NO_OS_UART_STOP_1_BIT,
	.irq_id = UART_IRQ_ID,
	.asynchronous_rx = false,
	.platform_ops = &uart_ops,
	.extra = &uart_extra_init_params
};


struct ad5933_init_param init_params = {
	.i2c_init = {
		.device_id = I2C_DEVICE_ID,
		.max_speed_hz = 100000, 			// i2c max speed (hz)
		.slave_address = AD5933_ADDRESS, 	// i2c slave address //A0 tied high
		.platform_ops = &i2c_ops,
		.extra = &i2c_init_extra_params		// i2c extra initialization parameters
	},
	.current_sys_clk = AD5933_INTERNAL_SYS_CLK,			//current_sys_clk frequency (16MHz)
	.current_clock_source = AD5933_CONTROL_INT_SYSCLK,	//current_clock_source
	.current_gain = AD5933_RANGE_1000mVpp,				//current_gain
	.current_range = AD5933_GAIN_X1,					//current_range
};

struct ad5933_dev *device;
int32_t connected = -EINVAL;

int main()
{
	int ret;
#if (ACTIVE_PLATFORM == STM32_PLATFORM)
	stm32_system_init();
#endif
	ret = no_os_uart_init(&uart_desc, &uart_init_params);
	if (ret) {
		return ret;
	}

	no_os_uart_stdio(uart_desc);

	uint8_t menu_select = 0;
	print_title();
	connected = ad5933_init(&device, init_params);

	//Do a quick check to ensure basic connectivity is ok
	temperature  = ad5933_get_temperature(device);
	if (temperature >= TEMP_LIMIT_MIN && temperature <= TEMP_LIMIT_MAX) {
		printf("\nTemperature: %f, AD5933 initialization successful!" EOL,
		       temperature);
	} else {
		printf(EOL"AD5933 initialization reported a bad temperature - recommend debug :"
		       EOL);
	}

	while (connected == 0) {
		print_prompt();
		getMenuSelect(&menu_select);
		config_data.start_freq = 10000;

		if (menu_select > 12)
			print_prompt();
		else switch (menu_select) {
			case 0:
				guide();
				no_os_mdelay(2000);
				break;

			case 1:
				read_temperature();
				break;
			case 2:
				configure_system();
				break;
			case 3:
				calculate_gain_factor();
				break;
			case 4:
				impedance_sweep();
				break;

			default:
				printf("Invalid option: Ignored.");
				break;
			}

		no_os_mdelay(100);
	}

	return 0;
}

//! Prints the title block
void print_title()
{
	printf(EOL "*****************************************************************"
	       EOL);
	printf("* AD5933 Demonstration Program                                  *"EOL);
	printf("*                                                               *"EOL);
	printf("* This program demonstrates communication with the AD5933       *"EOL);
	printf("*                                                               *"EOL);
	printf("* 1 MSPS, 12-Bit Impedance Converter, Network analyser          *"EOL);
	printf("*                                                               *"EOL);
	printf("* Set the baud rate to 115200 select the newline terminator.    *"EOL);
	printf("*****************************************************************"EOL);
}

void print_prompt()
{
	printf("\n\n\rCommand Summary:\n"EOL);
	printf("  0  -Software Guide" EOL);
	printf("  1  -Read temperature" EOL);
	printf("  2  -Configure voltage-range, PGA-Gain and sweep parameters" EOL);
	printf("  3  -Calculate Gain-Factor" EOL);
	printf("  4  -Do an impedance sweep" EOL);
	printf("\n\rMake a selection..." EOL);

}

static void getMenuSelect(uint8_t *menuSelect)
{
	int32_t ret;

	ret = scanf("%d", (int *)menuSelect);
	if (ret < 0) {
		printf("Invalid Input\r\n");
	}

}

static uint8_t read_temperature()
{
	temperature = ad5933_get_temperature(device);

	printf("Current temperature:%.3f C", temperature);
	return 0;
}

static uint8_t set_system_clock()
{
	printf("  Select Internal (1) or external clock (2): ");

	int input = 0;
	int invalid_check;
	int ret;

	invalid_check = scanf("%d", &input);
	ret = input_check(input, 1, 2, invalid_check);
	if (ret) {
		return -EINVAL;
	}

	if (isdigit(input) == 0 && (input == 1 || input == 2)) {
		input == 1 ? printf("\n  You selected Internal clock source" EOL) :
		printf("  You selected external Source clock source" EOL);
	} else {
		printf("Invalid entry" EOL);
		no_os_mdelay(2000);
		return -EINVAL;
	}

	if (input == 2) {

		printf("  Enter external clock frequency in Hz ");
		invalid_check = scanf("%d", &input);
		ret = input_check(input, 0, INT_MAX, invalid_check);
		if (ret) {
			return -EINVAL;
		}
		if (isdigit(input) == 0  && input > 0 && input < 20000000) {
			printf("  External clk-source frequency set to %d " EOL, input);
		} else {
			printf("Invalid entry" EOL);
			no_os_mdelay(2000);
			return -EINVAL;
		}
	}

	ad5933_set_system_clk(device,
			      input == 1 ? AD5933_CONTROL_INT_SYSCLK :
			      AD5933_CONTROL_EXT_SYSCLK,
			      input);

	return 0;
}

static uint8_t set_vrange_and_pga_gain()
{
	int input;
	uint8_t v_range = AD5933_RANGE_1000mVpp;
	int invalid_check;

	printf("  Select output voltage range" EOL);
	printf("    1: 200mVpp typical:" EOL);
	printf("    2: 400mVpp typical:" EOL);
	printf("    3: 1Vpp typical:" EOL);


	invalid_check = scanf("%d", &input);
	if (invalid_check == 0) {
		return -EINVAL;
	}
	if (input >= 0 && input < 4) {
		switch (input) {
		case AD5933_RANGE_2000mVpp: {
			printf("  Selected 2V pp typical." EOL);
			break;
		}
		case AD5933_RANGE_200mVpp: {
			printf("  Selected 200mV pp typical." EOL);
			break;
		}
		case AD5933_RANGE_400mVpp: {
			printf("  Selected 400mV pp typical." EOL);
			break;
		}
		case AD5933_RANGE_1000mVpp: {
			printf("  Selected 1V pp typical." EOL);
			break;
		}
		}
		v_range = input;
	} else {
		printf("Invalid entry" EOL);
		no_os_mdelay(2000);
		return -EINVAL;
	}

	printf("\n  Select PGA Gain (0=X5, 1=X1)" EOL);
	invalid_check = scanf("%d", &input);
	if (invalid_check == 0) {
		return -EINVAL;
	}
	if (input >= 0 && input < 2) {
		config_data.pga_gain = input;
		config_data.output_voltage_range = v_range;

		printf("PGA gain set to : ");
		input == AD5933_GAIN_X5 ? printf("X5\n\n"EOL) : printf("X1\n\n");
		ad5933_set_range_and_gain(device,
					  config_data.output_voltage_range,
					  config_data.pga_gain);
	} else {
		printf(EOL "Invalid entry: write aborted" EOL);
		no_os_mdelay(2000);
		return -EINVAL;
	}


	return 0;
}

static int32_t configure_system()
{

	printf("Configure the impedance meter\n" EOL);
	set_vrange_and_pga_gain();
	set_system_clock();

	int start_freq;
	int freq_inc;
	int num_increments;
	int num_settling_cycles;
	int multiplier = AD5933_SETTLING_X1;
	int invalid_check;
	int ret;

	printf("\n  Enter start-frequency as a decimal number: ");
	if (scanf("%d", &start_freq) == 1) {
		if (start_freq <= 0) {
			printf("  Invalid entry, write aborted: " EOL);
			return -EINVAL;
		}
	}

	printf("\n  Enter frequency-increment as a decimal number: ");
	invalid_check = scanf("%d", &freq_inc);
	ret = input_check(freq_inc, 0, INT_MAX, invalid_check);
	if (ret) {
		return -EINVAL;
	}
	if (isdigit(freq_inc) != 0  || freq_inc <= 0) {
		printf("  Invalid entry, write aborted: \n");
		return -EINVAL;
	}

	printf("\n  Enter the number of increments as a decimal number: ");
	printf(EOL " Number of increments must be less than %d" EOL,
	       MAX_FREQ_INCREMENTS);
	invalid_check = scanf("%d", &num_increments);
	ret = input_check(num_increments, 0, INT_MAX, invalid_check);
	if (ret) {
		return -EINVAL;
	}
	if (isdigit(num_increments) != 0  || num_increments > MAX_FREQ_INCREMENTS) {
		printf("  Invalid entry, write aborted: " EOL);
		return -EINVAL;
	}

	printf("Enter the number of settling-time cycles before ADC is triggered." EOL);
	invalid_check = scanf("%d", &num_settling_cycles);
	if (invalid_check == 0) {
		return -EINVAL;
	}
	if (num_settling_cycles > MAX_SETTLING_CYCLES) {
		printf("  Invalid entry, write aborted: " EOL);
		return -EINVAL;
	}

	printf("Set the settling time multiplier (X1=0, X2=1, X4=2)." EOL);
	invalid_check = scanf("%d", &multiplier);
	if (invalid_check == 0) {
		return -EINVAL;
	}
	if (multiplier > 2) {
		printf("  Invalid entry, write aborted: " EOL);
		return -EINVAL;
	} else {
		//adjust X4 option to match memory map
		if (multiplier == 2)
			multiplier = AD5933_SETTLING_X4;
	}

	printf("\n    Setting start frequency to %d\n\r", (unsigned int)start_freq);
	printf("    Setting frequency increment to %d\n\r", (unsigned int)freq_inc);
	printf("    Setting the number of increments to %d\n\r",
	       (unsigned int)num_increments);
	printf("    Setting the number of settling-cycles to %d\n\r",
	       (unsigned int)num_settling_cycles);
	printf("    The multiplier for the settling-cycles %d\n\r",
	       (unsigned int)multiplier + 1);

	//update device state
	config_data.start_freq = start_freq;
	config_data.frequency_increment = freq_inc;
	config_data.number_increments = num_increments;
	config_data.number_settling_cycles = num_settling_cycles;

	ad5933_set_settling_time(device, multiplier, num_settling_cycles);
	ad5933_set_range_and_gain(device, device->current_range, device->current_gain);
	ad5933_config_sweep(device, start_freq, freq_inc, num_increments);

	return 0;

}

static uint8_t calculate_gain_factor()
{
	double calibration_impedance;

	printf("\n\nCalculate the gain-factor (see data-sheet for information)" EOL);
	printf("Calculated gain-factor will be stored for impedance measurements and"EOL);
	printf("displayed on the terminal screen." EOL);
	printf("Ensure that the system has been configured before" EOL);
	printf("calculating the gain factor" EOL);

	ad5933_config_sweep(device,
			    config_data.start_freq,
			    config_data.frequency_increment,
			    config_data.number_increments);

	// Do standby, init-start freq, start the sweep, and wait for valid data
	ad5933_start_sweep(device);

	printf("\nEnter calibration resistance in Ohms: ");
	int invalid_check = scanf("%le", &calibration_impedance);
	if (invalid_check == 0) {
		return -EINVAL;
	}


	printf("Calculating gain factor\n\r");

	gain_factor = ad5933_calculate_gain_factor(device,
			calibration_impedance,
			AD5933_FUNCTION_REPEAT_FREQ);
	printf("\n\r    Calculated gain factor %e\n\r", gain_factor);

	return 0;
}

static uint8_t guide()
{
	printf("\n\rAD5933-Demo quick-start guide: " EOL);
	printf("This program can be used both as a demo of the AD5933 impedance " EOL);
	printf("measurement system and as a starting point for developing a " EOL);
	printf("more advanced program for prototyping. This program is not " EOL);
	printf("provided as production-quality code, but as a helpful starting point."
	       EOL);

	printf("As a quick start, the following steps can be implemented to ensure"EOL);
	printf("firmware is communicating with the board and measurements taking place."EOL);

	printf("Firstly - use menu option 1 to read the on-chip temperature." EOL);
	printf("If a realistic temperature comes back - you are good to go :)" EOL);

	printf("Step 1\tConnect a 200k Resistor across the SMA terminals of the PMOD 1A"EOL);
	printf("Step 2\tSelect the 100k feedback resistor by pulling the SEL pin high"EOL);
	printf("Step 2\tConfigure the impedance system with Menu Option 2" EOL);
	printf("Step 3\tCalculate the gain factor with menu-item 3" EOL);
	printf("Step 3\tReplace the 200k impedance across the SMA terminals with a "EOL);
	printf("different 'unknown' impedance (300K perhaps)" EOL);
	printf("Step 4\tRun the impedance measurement with menu-item 4" EOL);
	printf("\tresults are displayed on the terminal" EOL);

	return 0;
}

static uint8_t impedance_sweep()
{
	printf("\nPerform a sweep to calculate an unknown impedance (see data-sheet for information)"EOL);
	printf("System should have been previously configured (Menu Option 2)" EOL);
	printf("Impedance will be calculated and results shown.\n\r");

	int32_t status = -EINVAL;
	double impedance;
	float frequency = config_data.start_freq;


	ad5933_config_sweep(device,
			    config_data.start_freq,
			    config_data.frequency_increment,
			    config_data.number_increments);

	/*
		> program frequency sweep parameters into relevant registerS
		> place the ad5933 into standby mode.
		> start frequency register
		> number of increments register
	*/
	ad5933_start_sweep(device);
	printf(EOL" FREQUENCY MAGNITUDE   PHASE	IMPEDANCE" EOL);

	do {
		//Fill up the results struct with data
		impedance = ad5933_calculate_impedance(device,
						       gain_factor,
						       AD5933_FUNCTION_INC_FREQ);

		printf("  %.2f,", frequency);
		printf("  %.2f\n", impedance);

		frequency += config_data.frequency_increment;

		//poll the status register to check if frequency sweep is complete.
		status = ad5933_get_register_value(device, AD5933_REG_STATUS, 1);

	} while ((status & AD5933_STAT_SWEEP_DONE) == 0);

	return status;
}

/*******************************************************************************
 * @brief Check if is an input is a digit and within valid range
 *
 * @param input_val - Value inputed by user
 * @param lowest_accepted_val - Lowest acceptable value
 * @param highest_accepted_val - Highest acceptable value
 * @param invalid_check - Checks if unexpected type of data was entered in scanf
 *
 * @return 0 in case of success, negative error code otherwise
 *******************************************************************************/
int input_check(int input_val,
		int lowest_accepted_val,
		int highest_accepted_val,
		int invalid_check)
{
	if (invalid_check == 0 || input_val < lowest_accepted_val
	    || input_val > highest_accepted_val) {
		printf(EOL EOL "*****   Invalid entry: No changes made *****" EOL);
		no_os_mdelay(1000);
		return -EINVAL;
	}

	return 0;
}
