/*!
 *****************************************************************************
  @file:  main.c

  @brief: main module for ADT74xx application interface

  @details: main module for ADT74xx application interface
 -----------------------------------------------------------------------------
 Copyright (c) 2019, 2021-2022, Analog Devices, Inc.
 All rights reserved.

 This software is proprietary to Analog Devices, Inc. and its licensors.
 By using this software you agree to the terms of the associated
 Analog Devices Software License Agreement.
/*****************************************************************************/

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include <math.h>
#include <stdlib.h>
#include "app_config.h"
#include "no_os_spi.h"
#include "no_os_i2c.h"
#include "no_os_error.h"
#include "mbed_i2c.h"
#include "mbed_spi.h"
#include "no_os_delay.h"
#include "mbed_platform_support.h"
#include "adt7420.h"
#include "adt7xxx_support.h"

/******************************************************************************/
/********************** Macros and Constants Definitions **********************/
/******************************************************************************/

#define constrain(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))

#define TEMP_MIN                -40
#define TEMP_MAX                150
#define MAX_HYST_TEMP           15
#define MIN_HYST_TEMP           0

#define RESET_DELAY             500
#define WAIT_MENU_TIME          1000
#define NOT_USED                0

/******************************************************************************/
/************************** Functions Declarations ****************************/
/******************************************************************************/

static void print_title(void);
static void print_prompt(void);
static uint8_t select_device();
static void print_active_device(int ext_int_value);
static int get_menu_select(uint8_t *menu_select);
static uint8_t read_temperature();
static int32_t set_resolution();
static int32_t set_op_mode();
static int32_t bunch_of_temps();
static int32_t readback_reg();
static int32_t reset_interface();
static int32_t write_to_setpoint_reg();
static int32_t set_fault_queue();
static int32_t set_ct_int_config();
static void microcontroller_reset();
static int input_check(int input_val,
		       int lowest_accepted_val,
		       int highest_accepted_val,
		       int invalid_check);

/******************************************************************************/
/********************** Variables and User Defined Data Types *****************/
/******************************************************************************/

// Initialize the extra parameters for I2C initialization
struct mbed_i2c_init_param i2c_init_extra_params = {
	.i2c_sda_pin = I2C_SDA,
	.i2c_scl_pin = I2C_SCL
};

struct no_os_i2c_init_param i2c_params = {
	.device_id = 0,
	.max_speed_hz = 100000, 		 // i2c max speed (hz)
	.slave_address  = INT_I2C_ADDRESS, // i2c slave address
	.extra = &i2c_init_extra_params,	// I2C extra init parameters
	.platform_ops = &i2c_platform_ops
};

// Initialize the extra parameters for SPI initialization
struct mbed_spi_init_param spi_init_extra_params = {
	.spi_miso_pin = SPI_HOST_SDI,
	.spi_mosi_pin = SPI_HOST_SDO,
	.spi_clk_pin = SPI_SCK
};

struct no_os_spi_init_param spi_params = {
	.max_speed_hz  = 1000000,		 //SPI frequency (Hz)
	.chip_select  = SPI_CSB,	 		 //Chip Select
	.mode = NO_OS_SPI_MODE_3,		 //CPOL/CPHA settings for your device
	.extra = &spi_init_extra_params,	// SPI extra init parameters
	.platform_ops = &spi_platform_ops
};

static struct adt7420_init_param init_params = {
	.resolution_setting  = NOT_USED,		//Resolution setting
	.active_device =  ACTIVE_DEVICE   //Set this in app_config.h
};

struct adt7420_dev *device;

registers_e registers;

int32_t connected = -1;
uint8_t device_id = 0;

/******************************************************************************/
/************************ Functions Definitions *******************************/
/******************************************************************************/

/*******************************************************************************
 * @brief - Main function
 * @param None
 * @return 0 in case of success, negative error code otherwise
 *******************************************************************************/
int main()
{
	uint8_t menu_select = 0;
	int32_t ret = 0;

	print_title();

	device_id = select_device();
	connected = adt7420_init(&device, init_params);

	if (connected != 0) {
		printf(EOL EOL "  Connection to device failed :(" EOL);
		printf("  ...Restarting application...  " EOL);
		no_os_mdelay(WAIT_MENU_TIME);
		microcontroller_reset();
	} else {
		printf(EOL EOL "  Connection to device succeeded!" EOL);
	}

	while (connected == 0) {
		menu_select = 0;
		print_active_device(device_id);
		print_prompt();

		ret = get_menu_select(&menu_select);
		if (ret)
			printf(EOL "*****   Returning to main menu   *****" EOL);
		else {
			switch (menu_select) {
			case 1:
				ret = read_temperature();
				break;
			case 2:
				ret = set_resolution();
				break;
			case 3:
				ret = set_op_mode();
				break;
			case 4:
				ret = bunch_of_temps();
				break;
			case 5:
				ret = readback_reg();
				break;
			case 6:
				ret = reset_interface();
				break;
			case 7:
				ret = write_to_setpoint_reg();
				break;
			case 8:
				ret = set_fault_queue();
				break;
			case 9:
				ret = set_ct_int_config();
				break;
			case 10:
				/*Restore device registers to default values before restarting microcontroller*/
				ret = reset_interface();
				microcontroller_reset();
				break;
			default:
				printf("Invalid option" EOL);
				break;
			}
		}
		no_os_mdelay(WAIT_MENU_TIME); //wait 1 second
	}

	return ret;
}


/*******************************************************************************
 * @brief	Prints the title block.
 *******************************************************************************/
void print_title()
{
	printf("*****************************************************************" EOL);
	printf("* EVAL-TempeSense-ARDZ Demonstration Program                    *" EOL);
	printf("*                                                               *" EOL);
	printf("* This program demonstrates communication with the ADT7xx       *" EOL);
	printf("* High-Accuracy digital temperature sensor family               *" EOL);
	printf("* It works with both SPI & I2C versions                         *" EOL);
	printf("*                                                               *" EOL);
	printf("*****************************************************************" EOL);
}

/*******************************************************************************
 * @brief	Prints the "main menu" prompt to the console.
 *******************************************************************************/
void print_prompt()
{
	printf(EOL EOL "Command Summary:" EOL);
	printf("  1  -Read temperature" EOL);
	printf("  2  -Set resolution" EOL);
	printf("  3  -Set operation mode" EOL);
	printf("  4  -Poll temperature" EOL);
	printf("  5  -Read a register" EOL);
	printf("  6  -Reset the interface" EOL);
	printf("  7  -Write to a setpoint register" EOL);
	printf("  8  -Set Fault Queue configuration" EOL);
	printf("  9  -Set CT/INT polarity and mode" EOL);
	printf("  10 -Full System Reset" EOL);
	printf(EOL);
}

/*******************************************************************************
 * @brief 	- Get the selected menu
 * @param 	menu_select Selected menu option
 * @return 0 in case of success, negative error code otherwise
 *******************************************************************************/
static int get_menu_select(uint8_t *menu_select)
{
	int invalid_check = scanf("%d",(int *) menu_select);
	return input_check(*menu_select, 1, 10, invalid_check);
}

/*******************************************************************************
 * @brief 	- Select the serial interface (SPI/I2C) and device
 *				based on the part family.
 *		  	- Only one device and interface can be active.
 *						Example: ADT7320 - SPI (Internal or Remote device)
 *								 ADT7420 - I2C (Internal or Remote device)
 *
 * @param 	None
 *
 * @return 	new_dev - Return device selected
 *				 	  Example: 1  - Internal(Main PCB)
 *							   2  - Remote (External PCB)
 *******************************************************************************/
uint8_t select_device()
{
	int32_t ret;

	printf("Please select interface by choosing a device:" EOL);
	printf("    1- ADT7320 (SPI)" EOL);
	printf("    2- ADT7420 (I2C)" EOL);
	printf("  Select an option: ");

	int invalid_check, new_interface = 0;
	invalid_check = scanf("%d", &new_interface);

	//Prompts for user input while correct interface is not selected
	ret = input_check(new_interface, 1, 2, invalid_check);
	while (ret) {
		printf("Please select interface by choosing a device:" EOL);
		printf("    1- ADT7320 (SPI)" EOL);
		printf("    2- ADT7420 (I2C)" EOL);
		printf("  Select an option: ");
		invalid_check = scanf("%d", &new_interface);
	}
	printf("%d", new_interface);

	switch (new_interface) {
	case 1:
		printf("  ADT7320 sensor selected!" EOL EOL);
		init_params.active_device = ID_ADT7320;
		init_params.interface_init.spi_init = spi_params;
		break;
	case 2:
		printf(" ADT7420 sensor selected!" EOL EOL);
		init_params.active_device = ID_ADT7420;
		init_params.interface_init.i2c_init = i2c_params;
		break;
	}

	printf("Available devices:" EOL);
	printf("    1- Internal (Main PCB)" EOL);
	printf("    2- Remote   (External PCB)" EOL);
	printf("  Select an option: ");

	int new_dev = 0;
	invalid_check = scanf("%d", &new_dev);

	//Prompts for user input while correct device is not selected
	ret = input_check(new_dev, 1, 2, invalid_check);
	while (ret) {
		printf("Device select:" EOL);
		printf("    1- Internal (Main PCB)" EOL);
		printf("    2- Remote   (External PCB)" EOL);
		printf("  Select an option: ");
		invalid_check = scanf("%d", &new_dev);
	}

	printf("%d", new_dev);

	switch (new_dev) {
	case 1:
		printf("  Internal sensor selected!");

		if (init_params.active_device == ID_ADT7420) {
			init_params.interface_init.i2c_init.slave_address = INT_I2C_ADDRESS;
		} else {
			init_params.interface_init.spi_init.chip_select = SPI_CSB;
		}
		break;
	case 2:
		printf("  External sensor selected!");
		if (init_params.active_device == ID_ADT7420) {
			init_params.interface_init.i2c_init.slave_address = EXT_I2C_ADDRESS;
		} else {
			init_params.interface_init.spi_init.chip_select = SPI_CSE;
		}
		break;
	}

	return new_dev;
}

/*******************************************************************************
 * @brief	- Reads and prints the temperature in Celsius Degree
 *
 * @param None
 *
 * @return	- 0 - currently unused.
 *******************************************************************************/
static uint8_t read_temperature()
{
	float temp = adt7420_get_temperature(device);

	printf("Current temperature:%.3f C", temp);
	return 0;
}

/*******************************************************************************
 * @brief	-Set the device resolution for 13 or 16 bits
 *
 * @param None
 *
 * @return	- 0 in case of success, negative error code otherwise
 *******************************************************************************/
static int32_t set_resolution()
{
	int32_t ret;

	printf("  Available resolutions:" EOL);
	printf("    1- 13-bit" EOL);
	printf("    2- 16-bit" EOL);
	printf("  Select an option: ");

	int new_res, invalid_check = 0;
	invalid_check = scanf("%d", &new_res);

	ret = input_check(new_res, 1, 2, invalid_check);
	if (ret) {
		return ret;
	} else {
		printf("%d" EOL, new_res);
		new_res = (new_res == 1) ? 0 : 1;
		ret = adt7420_set_resolution(device, new_res);
		if (ret) {
			return ret;
		}
		printf("Set resolution to %d-bit", (13 + 3 * new_res));
		return ret;
	}
}

/*******************************************************************************
 * @brief	- Set the device operation mode
 *			  (Continuous conversion, One-shot, SPS, Shutdown).
 *			- Consult datasheet for more information.
 *
 * @param None
 *
 * @return - 0 in case of success, negative error code otherwise
 *******************************************************************************/
static int32_t set_op_mode()
{
	int32_t ret;

	printf("  Available operation modes:" EOL);
	printf("    1- Continuous conversion mode (default)" EOL);
	printf("    2- One-shot mode" EOL);
	printf("    3- 1 SPS mode" EOL);
	printf("    4- Shutdown" EOL);
	printf("  Select a mode: ");

	int new_mode, invalid_check = 0;
	invalid_check = scanf("%d", &new_mode);
	ret = input_check(new_mode, 1, 4, invalid_check);
	if (ret) {
		return ret;
	} else {
		printf("%d" EOL, new_mode);
		switch (new_mode) {
		case 1:
			ret = adt7420_set_operation_mode(device, ADT7420_OP_MODE_CONT_CONV);
			break;
		case 2:
			/*When One shot mode is set completes one conversion and immediately goes to shutdown mode*/
			ret = adt7420_set_operation_mode(device, ADT7420_OP_MODE_ONE_SHOT);
			printf( EOL"       One Shot mode enabled, device will enter shutdown mode once a conversion is complete."
				EOL);
			printf("         See page 10 in datasheet for details." EOL);
			break;
		case 3:
			ret = adt7420_set_operation_mode(device, ADT7420_OP_MODE_1_SPS);
			break;
		case 4:
			ret = adt7420_set_operation_mode(device, ADT7420_OP_MODE_SHUTDOWN);
			break;
		default:
			printf("Invalid option" EOL);
			break;
		}
		return ret;
	}
}

/*******************************************************************************
 * @brief	- Prints poll of temperature based on the frequency of readings and
 *			  number of samples.
 *
 *
 * @param None
 *
 * @return	- 0 in case of success, negative error code otherwise
 *******************************************************************************/
static int32_t bunch_of_temps()
{
	int32_t ret;

	printf("  Enter number of desired samples: ");
	int num_samples, invalid_check = 1;

	invalid_check = scanf("%d", &num_samples);
	ret = input_check(num_samples, 1, 2000000, invalid_check);
	if (ret) {
		return ret;
	}
	printf("%d" EOL, num_samples);

	printf("  Enter a desired frequency in samples/sec (max 10): ");
	int sample_freq = 1;
	invalid_check = scanf("%d", &sample_freq);
	ret = input_check(sample_freq, 1, 10, invalid_check);
	if (ret) {
		return ret;
	}
	sample_freq = constrain(sample_freq, 1, 10);
	printf("%d", sample_freq);

	uint32_t delay_sec = 1000000 / sample_freq;

	printf("  Gathering %d seconds of samples" EOL, num_samples/sample_freq);
	printf("Press enter to continue and then press again to quit" EOL);
	getchar();

	for (int i = 0; i < num_samples; i++) {
		if(getchar_noblock()) {
			return 0;
		} else {
			printf("  Sample:%d: Temperature:", i + 1);
			float temp = adt7420_get_temperature(device);
			printf("%.4f" EOL, temp);
			no_os_udelay(delay_sec);
		}
	}
	return ret;
}

/*******************************************************************************
 * @brief	- Reads back data store in device registers
 *
 * @param None
 *
 * @return	- 0 in case of success, negative error code otherwise
 *******************************************************************************/

static int32_t readback_reg()
{
	printf("  Available registers:" EOL);
	printf("    1- Status" EOL);
	printf("    2- Configuration" EOL);
	printf("    3- Temperature" EOL);
	printf("    4- ID" EOL);
	printf("    5- Critical Temperature setpoint" EOL);
	printf("    6- Hysteresis Temperature setpoint" EOL);
	printf("    7- Temperature high setpoint" EOL);
	printf("    8- Temperature low setpoint" EOL);
	printf("  Select a mode: ");

	uint16_t read_value = 0;
	int new_mode, invalid_check = 0;
	int32_t ret;

	invalid_check = scanf("%d", &new_mode);
	ret = input_check(new_mode, 1, 8, invalid_check);
	if (ret) {
		return ret;
	}
	printf("%d" EOL, new_mode);

	switch (new_mode) {
	case 1:
		ret = adt7420_get_register_address_and_value(device, REG_STATUS, &read_value);
		break;
	case 2:
		ret = adt7420_get_register_address_and_value(device, REG_CONFIG, &read_value);
		break;
	case 3:
		ret = adt7420_get_register_address_and_value(device, REG_TEMP, &read_value);
		break;
	case 4:
		ret = adt7420_get_register_address_and_value(device, REG_ID, &read_value);
		break;
	case 5:
		ret = adt7420_get_register_address_and_value(device, REG_T_CRIT, &read_value);
		break;
	case 6:
		ret = adt7420_get_register_address_and_value(device, REG_HIST, &read_value);
		break;
	case 7:
		ret = adt7420_get_register_address_and_value(device, REG_T_HIGH, &read_value);
		break;
	case 8:
		ret = adt7420_get_register_address_and_value(device, REG_T_LOW, &read_value);
		break;
	default:
		break;
	}
	if (ret) {
		return ret;
	}
	printf("Read value: 0x%x" EOL, read_value);

	return ret;
}

/*******************************************************************************
 * @brief	- Resets device interface (SPI/I2C) (power-on reset)
 *
 * @param None
 *
 * @return	- 0 in case of success, negative error code otherwise
 *******************************************************************************/
static int32_t reset_interface()
{
	int32_t ret;

	printf("  Resetting interface..." EOL);
	ret = adt7420_reset(device);
	if (ret) {
		return ret;
	}
	no_os_udelay(RESET_DELAY);

	return ret;
}

/*******************************************************************************
 * @brief	- Write to setpoint registers THIGH, TLOW, TCRIT and THYST.
			- Values entered in Celsius and rounded to a near integer value.
 *
 * @param None
 *
 * @return	- 0 in case of success, negative error code otherwise
 *******************************************************************************/
static int32_t write_to_setpoint_reg()
{
	printf("  Available registers:" EOL);
	printf("    1- Critical setpoint" EOL);
	printf("    2- Hystersis setpoint" EOL);
	printf("    3- Temperature high setpoint" EOL);
	printf("    4- Temperature low setpoint" EOL);
	printf("  Select a mode: ");

	int new_mode, invalid_check = 0;
	invalid_check = scanf("%d", &new_mode);
	int32_t ret;

	ret = input_check(new_mode, 1, 4, invalid_check);
	if (ret) {
		return ret;
	}
	printf("%d" EOL, new_mode);

	float temp_c;

	if(new_mode == 2) {
		printf("Enter value to write (0 to 15) Celsius:");
		invalid_check = scanf("%f", &temp_c);
		ret = input_check(temp_c, MIN_HYST_TEMP, MAX_HYST_TEMP, invalid_check);
		if(ret) {
			return ret;
		}
	} else {
		printf("Enter value to write (in Celsius):");
		invalid_check = scanf("%f", &temp_c);
		ret = input_check(temp_c, TEMP_MIN, TEMP_MAX, invalid_check);
		if(ret) {
			return ret;
		}
	}

	printf(" %.2f", temp_c);

	int16_t write_value;

	if(new_mode == 2)
		write_value = round(temp_c);
	else
		write_value = round(128 * temp_c);

	switch (new_mode) {
	case 1:
		ret = adt7420_wr_setpoint_reg(device, REG_T_CRIT, write_value);
		if (ret == 0)
			printf(EOL "0x%x successfuly written" EOL, write_value);
		else
			printf(EOL "0x%x NOT successfuly written" EOL, write_value);
		break;
	case 2:
		ret = adt7420_wr_setpoint_reg(device, REG_HIST, write_value);
		if (ret == 0)
			printf(EOL "0x%x successfuly written (bits 7:4 are fixed at 0)" EOL,
			       write_value);
		else
			printf(EOL "0x%x NOT successfuly written" EOL, write_value);
		break;
	case 3:
		ret = adt7420_wr_setpoint_reg(device, REG_T_HIGH, write_value);
		if (ret == 0)
			printf(EOL "0x%x successfuly written" EOL, write_value);
		else
			printf(EOL "0x%x NOT successfuly written" EOL, write_value);
		break;
	case 4:
		ret = adt7420_wr_setpoint_reg(device, REG_T_LOW, write_value);
		if (ret == 0)
			printf(EOL "0x%x successfuly written" EOL, write_value);
		else
			printf(EOL "0x%x NOT successfuly written" EOL, write_value);
		break;
	default:
		printf("Invalid selection - try again." EOL);
		no_os_mdelay(2000);
		break;
	}
	return ret;
}

/*******************************************************************************
 * @brief	- Set the number of undertemperature/overtemperature faults
 *			that can occur before setting the INT and CT output pins
 *
 * @param None
 *
 * @return	- 0 in case of success, negative error code otherwise
 *******************************************************************************/
static int32_t set_fault_queue()
{
	printf("  Available fault queue options:" EOL);
	printf("    1- 1 fault (default) " EOL);
	printf("    2- 2 faults" EOL);
	printf("    3- 3 faults" EOL);
	printf("    4- 4 faults" EOL);
	printf("  Select a mode: ");

	int new_fault, invalid_check = 0;
	int32_t ret;

	invalid_check = scanf("%d", &new_fault);
	ret = input_check(new_fault, 1, 4, invalid_check);
	if (ret) {
		return ret;
	} else {
		printf("%d" EOL, new_fault);

		switch (new_fault) {
		case 1:
			ret = adt7420_set_fault_queue(device, ADT7420_FAULT_QUEUE_1_FAULT);
			break;
		case 2:
			ret = adt7420_set_fault_queue(device, ADT7420_FAULT_QUEUE_2_FAULTS);
			break;
		case 3:
			ret = adt7420_set_fault_queue(device, ADT7420_FAULT_QUEUE_3_FAULTS);
			break;
		case 4:
			ret = adt7420_set_fault_queue(device, ADT7420_FAULT_QUEUE_4_FAULTS);
			break;
		default:
			printf("Invalid option" EOL);
			break;
		}

		return ret;
	}
}

/*******************************************************************************
 * @brief	- Set INT/CT Outputs pins to Comparator or Interrupt mode
 *
 * @param None
 *
 * @return	- 0 in case of success, negative error code otherwise
 *******************************************************************************/
static int32_t set_ct_int_config()
{
	int new_polarity = 0;
	int new_mode, invalid_check = 0;
	int32_t ret;

	printf("  Choose INT/CT mode:" EOL);
	printf("    1- Interrupt (default) " EOL);
	printf("    2- Comparator " EOL);
	printf("  Select a mode: ");
	invalid_check = scanf("%d", &new_mode);
	ret = input_check(new_mode, 1, 2, invalid_check);
	if (ret) {
		return ret;
	} else {
		printf("%d" EOL, new_mode);
		new_mode = (new_mode == 1) ? 0 : 1;
		ret = adt7420_set_ct_int_mode(device, new_mode);
		if (ret) {
			return ret;
		}
	}

	printf(EOL " Set output polarity for Critical and Over/Under Temperature pin:"
	       EOL);
	printf("   (Feature available only for internal sensors)." EOL);

	if(init_params.interface_init.i2c_init.slave_address == INT_I2C_ADDRESS ||
	    init_params.interface_init.spi_init.chip_select == SPI_CSB) {

		printf("    1- Active Low (default) " EOL);
		printf("    2- Active High" EOL);
		printf("  Select a mode: ");

		invalid_check = scanf("%d", &new_polarity);
		ret = input_check(new_polarity, 1, 2, invalid_check);
		if (ret) {
			return ret;
		} else {
			printf("%d" EOL, new_polarity);
			new_polarity = (new_polarity == 1) ? 0 : 1;
			ret = adt7420_set_ct_int_polarity(device, new_polarity);
			if (ret) {
				return ret;
			}
		}
	}
	return ret;
}

/*******************************************************************************
 * @brief Reset microcontroller
 *
 * @param None
 *
 * @return None.
 *******************************************************************************/
static void microcontroller_reset()
{
	NVIC_SystemReset();
}

/*******************************************************************************
 * @brief Prints the active device every time the main menu is redrawn
 *
 * @param external_internal_selection - External or Internal Chip Selected
 *
 * @return None.
 *******************************************************************************/
static void print_active_device(int external_internal_selection)
{
	const char* devices[7] = {EOL EOL "   Active Device: ADT7410 I2C",
				  EOL EOL "   Active Device: ADT7420 I2C",
				  EOL EOL "   Active Device: ADT7422 I2C",
				  EOL EOL "   Active Device: ADT7310 SPI",
				  EOL EOL "   Active Device: ADT7311 SPI",
				  EOL EOL "   Active Device: ADT7312 SPI",
				  EOL EOL "   Active Device: ADT7320 SPI"
				 };

	const char* external_internal_print[] = {" - Internal Chip " EOL,
						 " - External Chip " EOL
						};

	printf("%s %s", devices[init_params.active_device],
	       external_internal_print[external_internal_selection - 1]);
}

/*******************************************************************************
 * @brief Checks is an input is a digit and within valid range
 *
 * @param input_val - Value inputed by user
 * @param lowest_accepted_val - Lowest acceptable value
 * @param highest_accepted_val - Highest acceptable value
 * @param invalid_check - Checks if unexpected type of data was entered in scanf
 *
 * @return 0 in case of success, negative error code otherwise
 *******************************************************************************/
static int input_check(int input_val,
		       int lowest_accepted_val,
		       int highest_accepted_val,
		       int invalid_check)
{
	if(invalid_check == 0 || input_val < lowest_accepted_val
	    || input_val > highest_accepted_val) {
		printf(EOL EOL "*****   Invalid entry: No changes made *****" EOL );
		no_os_mdelay(WAIT_MENU_TIME);
		return -EINVAL;
	}
	return 0;
}
