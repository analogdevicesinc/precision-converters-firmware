/*******************************************************************************
 *   @file   cn0540_console_app.c
 *   @brief  CN0540 console application interfaces
********************************************************************************
Copyright 2025(c) Analog Devices, Inc.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of Analog Devices, Inc. nor the names of its
   contributors may be used to endorse or promote products derived from this
   software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY ANALOG DEVICES, INC. “AS IS” AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
EVENT SHALL ANALOG DEVICES, INC. BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*******************************************************************************/

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include <assert.h>
#include "app_config.h"
#include "cn0540_console_app.h"
#include "cn0540_user_config.h"
#include "ad77681.h"
#include "ltc26x6.h"
#include "adi_fft.h"
#include "no_os_gpio.h"
#include "no_os_delay.h"

/**
 * @brief	Convert ADC data to voltage without Vref
 * @param	data[in] - ADC data in straight binary format (signed)
 * @param	chn[in] - ADC channel (unused)
 * @return	voltage
 */
static float cn0540_data_to_voltage_without_vref(int32_t data, uint8_t chn)
{
	return ((float)data / AD7768_HALF_SCALE);
}

/**
 * @brief	Convert ADC data to voltage with respect to Vref
 * @param	data[in] - ADC data in straight binary format (signed)
 * @param	chn[in] - ADC channel (unused)
 * @return	voltage
 */
static float cn0540_data_to_voltage_wrt_vref(int32_t data, uint8_t chn)
{
	return ((float)data * (((float)ADC_REF_VOLTAGE / 1000) / AD7768_HALF_SCALE));
}

/**
 * @brief	Convert ADC code to straight binary data
 * @param	code[in] - ADC code (unsigned)
 * @param	chn[in] - ADC channel (unused)
 * @return	ADC straight binary data (signed)
 */
static int32_t cn0540_code_to_straight_binary(uint32_t code, uint8_t chn)
{
	int32_t adc_data;

	/* Data output format is offset binary for bipolar input */
	adc_data = code - AD7768_HALF_SCALE;

	return adc_data;
}

/* FFT init parameters specific to device */
struct adi_fft_init_params fft_init_params = {
	.vref = ADC_REF_VOLTAGE / 1000, // Vref in Volts
	.sample_rate = DEFAULT_SAMPLE_RATE,
	.samples_count = ADI_FFT_MAX_SAMPLES,
	.input_data_zero_scale = AD7768_HALF_SCALE,
	.input_data_full_scale = AD7768_FULL_SCALE,
	.convert_data_to_volt_without_vref = cn0540_data_to_voltage_without_vref,
	.convert_data_to_volt_wrt_vref = cn0540_data_to_voltage_wrt_vref,
	.convert_code_to_straight_binary = cn0540_code_to_straight_binary
};

/* FFT processing parameters */
struct adi_fft_processing fft_proc_params;

/* FFT meausurement parameters */
struct adi_fft_measurements fft_meas_params;

/* UART Descriptor */
struct no_os_uart_desc *uart_desc;

// Descriptor of the main device - the ADC AD7768-1
struct ad77681_dev *device_adc;

// Structure carying measured data, sampled by the ADC
struct adc_data measured_data;

// AD7768-1's status register structure, carying all the error flags
struct ad77681_status_registers *current_status;

// Initialize the interrupt event variable
volatile bool int_event = false;

// Descriptor of the DAC LTC2606
struct ltc26x6_dev *device_dac;

/**
 * @brief	Interrupt callback function
 * @return	None
 */
void drdy_interrupt()
{
	int_event = true;

	if (measured_data.count == measured_data.samples) {
		/* Desired numer of samples have been taken, set everything back */
		no_os_irq_disable(trigger_irq_desc, 0);
		measured_data.finish = true; // Desired number of samples have been taken
		measured_data.count = 0; // Set measured data counter to 0
	}
}

/**
 * @brief	Read the ADC data in continuous mode
 * @return	None
 */
static void cont_sampling()
{
	uint8_t buf[6];

	/* Enable continuous read */
	ad77681_set_continuos_read(device_adc, AD77681_CONTINUOUS_READ_ENABLE);

	/* Enable the interrupt */
	no_os_irq_enable(trigger_irq_desc, 0);

	while (!measured_data.finish) {
		// While loop. Waiting for the measurements to be completed
		if (int_event == true) {
			// Checks if interrupt has occurred
			ad77681_spi_read_adc_data(device_adc, buf,
						  AD77681_CONTINUOUS_DATA_READ); // Read the continuous read data
			if (device_adc->conv_len ==
			    AD77681_CONV_24BIT) {
				// 24bit format
				measured_data.raw_data[measured_data.count] = (buf[0] << 16 | buf[1] << 8 |
						buf[2] << 0); // Combining the SPI buffers
			} else {
				// 16bit format
				measured_data.raw_data[measured_data.count] = (buf[0] << 8 | buf[1] <<
						0); // Combining the SPI buffers
			}
			measured_data.count++; // Increment Measured Data Counter
			int_event = false; // Set int event flag after reading the Data
		}
	}

	/* Disable continuous read */
	ad77681_set_continuos_read(device_adc,
				   AD77681_CONTINUOUS_READ_DISABLE); // Disable continuous ADC read
}

/**
 * @brief	Read user input from uart
 * @param	UserInput[out] - user input
 * @return	0 in case of success, negative error code otherwise
 */
static int32_t getUserInput(uint32_t *UserInput)
{
	long uart_val;
	int32_t ret;

	ret = scanf("%ld", &uart_val); // Return 1 = OK, Return 0 = Fail

	if ((ret == 0) || (uart_val < 0)) {
		// Failure if uart_val is negative, or non-digit
		*UserInput = 0;
		return -1;
	}

	*UserInput = (uint32_t)(uart_val);

	return 0;
}

/**
 * @brief	Prints out an array in binary form
 * @param	number[in] - number to be converted
 * @param	binary_number[out] - number in binary format
 * @return	None
 */
static void print_binary(uint8_t number, char *binary_number)
{
	for (int8_t i = 7; i >= 0; i--) {
		if (number & 1) {
			binary_number[i] = '1';
		} else {
			binary_number[i] = '0';
		}
		number >>= 1;
	}

	binary_number[8] = '\0';
}

/**
 * @brief	Get mean from sampled data
 * @param	measured_data[in] - ADC data structure
 * @param	mean_voltage[out] - Mean Voltage
 * @return	None
 */
static void get_mean_voltage(struct adc_data *measured_data,
			     double *mean_voltage)
{
	double sum = 0, voltage = 0;
	uint16_t i;

	for (i = 0; i < measured_data->samples; i++) {
		ad77681_data_to_voltage(device_adc, &measured_data->raw_data[i], &voltage);
		sum += voltage;
	}
	*mean_voltage = (double)(sum / (double)(measured_data->samples));
}

/**
 * @brief	Set SINC3 filter
 * @return	None
 */
static void set_adc_SINC3_filter(void)
{
	uint32_t new_sinc3 = 0;
	int32_t ret;

	printf("\r\n SINC3 filter Oversampling ratios: \r\n");
	printf(" OSR is calculated as (x + 1)*32\r\n");
	printf(" x is SINC3 OSR register value\r\n");
	printf(" Please input a value from 0 to 8192 = 2^13 :");

	ret = getUserInput(&new_sinc3);

	if ((new_sinc3 <= 8192) && (!ret)) {
		printf("%ld\r\n", new_sinc3);
		ad77681_set_filter_type(device_adc, AD77681_SINC5_FIR_DECx32, AD77681_SINC3,
					new_sinc3);
		printf(" SINC3 OSR is set to %ld\r\n", (new_sinc3 + 1) * 32);
	} else {
		printf("%ld\r\n", new_sinc3);
		printf(" Invalid option - too large number\r\n");
	}
}

/**
 * @brief	Set SINC5 filter
 * @return	None
 */
static void set_adc_SINC5_filter(void)
{
	uint32_t new_sinc5;

	printf("\r\n SINC5 filter Oversampling ratios: \r\n");
	printf("  1 - Oversampled by 8\r\n");
	printf("  2 - Oversampled by 16\r\n");
	printf("  3 - Oversampled by 32\r\n");
	printf("  4 - Oversampled by 64\r\n");
	printf("  5 - Oversampled by 128\r\n");
	printf("  6 - Oversampled by 256\r\n");
	printf("  7 - Oversampled by 512\r\n");
	printf("  8 - Oversampled by 1024\r\n");
	printf(" Select an option: \r\n");

	getUserInput(&new_sinc5);

	switch (new_sinc5) {
	case 1:
		ad77681_set_filter_type(device_adc, AD77681_SINC5_FIR_DECx32,
					AD77681_SINC5_DECx8, 0);
		printf(" SINC5 with OSRx8 set\r\n");
		break;
	case 2:
		ad77681_set_filter_type(device_adc, AD77681_SINC5_FIR_DECx32,
					AD77681_SINC5_DECx16, 0);
		printf(" SINC5 with OSRx16 set\r\n");
		break;
	case 3:
		ad77681_set_filter_type(device_adc, AD77681_SINC5_FIR_DECx32, AD77681_SINC5, 0);
		printf(" SINC5 with OSRx32 set\r\n");
		break;
	case 4:
		ad77681_set_filter_type(device_adc, AD77681_SINC5_FIR_DECx64, AD77681_SINC5, 0);
		printf(" SINC5 with OSRx64 set\r\n");
		break;
	case 5:
		ad77681_set_filter_type(device_adc, AD77681_SINC5_FIR_DECx128, AD77681_SINC5,
					0);
		printf(" SINC5 with OSRx128 set\r\n");
		break;
	case 6:
		ad77681_set_filter_type(device_adc, AD77681_SINC5_FIR_DECx256, AD77681_SINC5,
					0);
		printf(" SINC5 with OSRx256 set\r\n");
		break;
	case 7:
		ad77681_set_filter_type(device_adc, AD77681_SINC5_FIR_DECx512, AD77681_SINC5,
					0);
		printf(" SINC5 with OSRx512 set\r\n");
		break;
	case 8:
		ad77681_set_filter_type(device_adc, AD77681_SINC5_FIR_DECx1024, AD77681_SINC5,
					0);
		printf(" SINC5 with OSRx1024 set\r\n");
		break;
	default:
		printf(" Invalid option\r\n");
		break;
	}
}

/**
 * @brief	Set FIR filter
 * @return	None
 */
static void set_adc_FIR_filter(void)
{
	uint32_t new_fir;

	printf("\r\n FIR filter Oversampling ratios: \r\n");
	printf("  1 - Oversampled by 32\r\n");
	printf("  2 - Oversampled by 64\r\n");
	printf("  3 - Oversampled by 128\r\n");
	printf("  4 - Oversampled by 256\r\n");
	printf("  5 - Oversampled by 512\r\n");
	printf("  6 - Oversampled by 1024\r\n");
	printf(" Select an option: \r\n");

	getUserInput(&new_fir);

	switch (new_fir) {
	case 1:
		ad77681_set_filter_type(device_adc, AD77681_SINC5_FIR_DECx32, AD77681_FIR, 0);
		printf(" FIR with OSRx32 set\r\n");
		break;
	case 2:
		ad77681_set_filter_type(device_adc, AD77681_SINC5_FIR_DECx64, AD77681_FIR, 0);
		printf(" FIR with OSRx64 set\r\n");
		break;
	case 3:
		ad77681_set_filter_type(device_adc, AD77681_SINC5_FIR_DECx128, AD77681_FIR, 0);
		printf(" FIR with OSRx128 set\r\n");
		break;
	case 4:
		ad77681_set_filter_type(device_adc, AD77681_SINC5_FIR_DECx256, AD77681_FIR, 0);
		printf(" FIR with OSRx256 set\r\n");
		break;
	case 5:
		ad77681_set_filter_type(device_adc, AD77681_SINC5_FIR_DECx512, AD77681_FIR, 0);
		printf(" FIR with OSRx512 set\r\n");
		break;
	case 6:
		ad77681_set_filter_type(device_adc, AD77681_SINC5_FIR_DECx1024, AD77681_FIR, 0);
		printf(" FIR with OSRx1024 set\r\n");
		break;
	default:
		printf(" Invalid option\r\n");
		break;
	}
}

/**
 * @brief	Set 50HZ rejection bit when SINC3 is being used
 * @return	None
 */
static void set_adc_50HZ_rej(void)
{
	uint32_t new_50Hz;

	printf("\r\n SINC3 50/60Hz rejection: \r\n");
	printf("  1 - 50/60Hz rejection enable \r\n");
	printf("  2 - 50/60Hz rejection disable \r\n");
	printf(" Select an option: \r\n");

	getUserInput(&new_50Hz);

	switch (new_50Hz) {
	case 1:
		ad77681_set_50HZ_rejection(device_adc, ENABLE);
		printf(" SINC3 50/60Hz rejection enabled\r\n");
		break;
	case 2:
		ad77681_set_50HZ_rejection(device_adc, DISABLE);
		printf(" SINC3 50/60Hz rejection disabled\r\n");
		break;
	default:
		printf(" Invalid option\r\n");
		break;
	}
}

/**
 * @brief	Insert user-defined FIR filter coeffs
 * @return	None
 */
static void set_adc_user_defined_FIR(void)
{
	const uint8_t coeff_reg_length =
		56; // Maximum allowed number of coefficients in the coeff register

	printf(" User Defined FIR filter\r\n");

	if ((ARRAY_SIZE(programmable_FIR) <= coeff_reg_length)
	    && (count_of_active_coeffs <= coeff_reg_length)) {
		printf("  Aplying user-defined FIR filter coefficients \r\n");
		ad77681_programmable_filter(device_adc,
					    programmable_FIR,
					    count_of_active_coeffs);
		printf(" Coeffs inserted successfully\r\n");
	} else {
		printf("  Incorrect count of coefficients in 'FIR_user_coeffs.h'\r\n");
	}
}

/**
 * @brief	Write to GPIOs, part of the ADC_GPIO function
 * @return	None
 */
static void adc_GPIO_write(void)
{
	uint32_t new_gpio_write = 0, new_value = 0;
	int32_t ret;

	printf("\r\n Write to GPIO: \r\n");
	printf("  1 - Write to all GPIOs\r\n");
	printf("  2 - Write to GPIO0\r\n");
	printf("  3 - Write to GPIO1\r\n");
	printf("  4 - Write to GPIO2\r\n");
	printf("  5 - Write to GPIO3\r\n");
	printf(" Select an option: \r\n");

	getUserInput(&new_gpio_write);

	switch (new_gpio_write) {
	case 1:
		printf("Insert value to be writen into all GPIOs, same value for all GPIOs: ");
		ret = getUserInput(&new_value);

		if (((new_value == NO_OS_GPIO_HIGH) || (new_value == NO_OS_GPIO_LOW))
		    && (!ret)) {
			new_value *= 0xF;
			ad77681_gpio_write(device_adc, new_value, AD77681_ALL_GPIOS);
			printf("\r\n Value %ld successully written to all GPOIs\r\n", new_value);
		} else {
			printf("\r\nInvalid value\r\n");
		}
		break;
	case 2:
		printf("Insert value to be written into GPIO0: ");
		ret = getUserInput(&new_value);

		if (((new_value == NO_OS_GPIO_HIGH) || (new_value == NO_OS_GPIO_LOW))
		    && (!ret)) {
			ad77681_gpio_write(device_adc, new_value, AD77681_GPIO0);
			printf("\r\n Value %ld successully written to GPIO0\r\n", new_value);
		} else {
			printf("\r\nInvalid value\r\n");
		}
		break;
	case 3:
		printf("Insert value to be written into GPIO1: ");
		ret = getUserInput(&new_value);

		if (((new_value == NO_OS_GPIO_HIGH) || (new_value == NO_OS_GPIO_LOW))
		    && (!ret)) {
			ad77681_gpio_write(device_adc, new_value, AD77681_GPIO1);
			printf("\r\n Value %ld successully written to GPIO1\r\n", new_value);
		} else {
			printf("\r\nInvalid value\r\n");
		}
		break;
	case 4:
		printf("Insert value to be written into GPIO2: ");
		ret = getUserInput(&new_value);

		if (((new_value == NO_OS_GPIO_HIGH) || (new_value == NO_OS_GPIO_LOW))
		    && (!ret)) {
			ad77681_gpio_write(device_adc, new_value, AD77681_GPIO2);
			printf("\r\n Value %ld successully written to GPIO2\r\n", new_value);
		} else {
			printf("\r\nInvalid value\r\n");
		}
		break;
	case 5:
		printf("Insert value to be written into GPIO3: ");
		ret = getUserInput(&new_value);

		if (((new_value == NO_OS_GPIO_HIGH) || (new_value == NO_OS_GPIO_LOW))
		    && (!ret)) {
			ad77681_gpio_write(device_adc, new_value, AD77681_GPIO3);
			printf("\r\n Value %ld successully written to GPIO3\r\n", new_value);
		} else {
			printf("\r\nInvalid value\r\n");
		}
		break;
	default:
		printf(" Invalid option\r\n");
		break;
	}
}

/**
 * @brief	GPIO direction, part of the ADC_GPIO function
 * @return	None
 */
static void adc_GPIO_inout(void)
{
	uint32_t new_gpio_inout = 0, new_gpio_inout_set = 0;

	printf("\r\n Set GPIOs as input or output: \r\n");
	printf("  1 - Set all GPIOs\r\n");
	printf("  2 - Set GPIO0\r\n");
	printf("  3 - Set GPIO1\r\n");
	printf("  4 - Set GIPO2\r\n");
	printf("  5 - Set GPIO3\r\n");
	printf(" Select an option: \r\n");

	getUserInput(&new_gpio_inout);

	switch (new_gpio_inout) {
	case 1:
		printf("   1 - Set all GPIOS as inputs\r\n");
		printf("   2 - Set all GPIOS as outputs\r\n");

		getUserInput(&new_gpio_inout_set);

		if ((new_gpio_inout_set == 1) || (new_gpio_inout_set == 2)) {
			new_gpio_inout_set -= 1;
			new_gpio_inout_set *= 0xF;
			ad77681_gpio_inout(device_adc, new_gpio_inout_set, AD77681_ALL_GPIOS);
			printf("All GPIOs successfully set");
		} else {
			printf("\r\nInvalid value\r\n");
		}
		break;
	case 2:
		printf("   1 - Set GPIO0 as input\r\n");
		printf("   2 - Set GPIO0 as output\r\n");

		getUserInput(&new_gpio_inout_set);

		if ((new_gpio_inout_set == 1) || (new_gpio_inout_set == 2)) {
			new_gpio_inout_set -= 1;
			ad77681_gpio_inout(device_adc, new_gpio_inout_set, AD77681_GPIO0);
			printf("GPIO0 successfully set");
		} else {
			printf("\r\nInvalid value\r\n");
		}
		break;
	case 3:
		printf("   1 - Set GPIO1 as input\r\n");
		printf("   2 - Set GPIO1 as output\r\n");

		getUserInput(&new_gpio_inout_set);

		if ((new_gpio_inout_set == 1) || (new_gpio_inout_set == 2)) {
			new_gpio_inout_set -= 1;
			ad77681_gpio_inout(device_adc, new_gpio_inout_set, AD77681_GPIO1);
			printf("GPIO1 successfully set");
		} else {
			printf("\r\nInvalid value\r\n");
		}
		break;
	case 4:
		printf("   1 - Set GPIO2 as input\r\n");
		printf("   2 - Set GPIO2 as output\r\n");

		getUserInput(&new_gpio_inout_set);

		if ((new_gpio_inout_set == 1) || (new_gpio_inout_set == 2)) {
			new_gpio_inout_set -= 1;
			ad77681_gpio_inout(device_adc, new_gpio_inout_set, AD77681_GPIO2);
			printf("GPIO2 successfully set");
		} else {
			printf("\r\nInvalid value\r\n");
		}
		break;
	case 5:
		printf("   1 - Set GPIO3 as input\r\n");
		printf("   2 - Set GPIO3 as output\r\n");

		getUserInput(&new_gpio_inout_set);

		if ((new_gpio_inout_set == 1) || (new_gpio_inout_set == 2)) {
			new_gpio_inout_set -= 1;
			ad77681_gpio_inout(device_adc, new_gpio_inout_set, AD77681_GPIO3);
			printf("GPIO3 successfully set");
		} else {
			printf("\r\nInvalid value\r\n");
		}
		break;
	default:
		printf(" Invalid option\r\n");
		break;
	}
}

/**
 * @brief	Additional GPIO settings, part of the ADC_GPIO function
 * @return	None
 */
static void adc_GPIO_settings(void)
{
	uint32_t new_gpio_settings = 0;

	printf("\r\n GPIO Settings: \r\n");
	printf("  1 - Enable  all GPIOs (Global enble)\r\n");
	printf("  2 - Disable all GPIOs (Global disable)\r\n");
	printf("  3 - Set GPIO0 - GPIO2 as open drain\r\n");
	printf("  4 - Set GPIO0 - GPIO2 as strong driver\r\n");
	printf(" Select an option: \r\n");

	getUserInput(&new_gpio_settings);

	switch (new_gpio_settings) {
	case 1:
		ad77681_global_gpio(device_adc, AD77681_GLOBAL_GPIO_ENABLE);
		printf(" Global GPIO enalbe bit enabled");
		break;
	case 2:
		ad77681_global_gpio(device_adc, AD77681_GLOBAL_GPIO_DISABLE);
		printf(" Global GPIO enalbe bit disabled");
		break;
	default:
		printf(" Invalid option\r\n");
		break;
	}
}

/**
 * @brief	Error warning, in case of unsuccessful SPI connection
 * @return	None
 */
static void go_to_error()
{
	int32_t connected;

	while (1) {
		printf("\r\nERROR: NOT CONNECTED\r\nCHECK YOUR PHYSICAL CONNECTION\r\n"); // When not connected, keep showing error message
		no_os_mdelay(5000);
		connected = ad77681_setup(&device_adc, init_params,
					  &current_status); // Keep trying to connect
		if (connected == 0) {
			printf("\r\nSUCCESSFULLY RECONNECTED\r\n"); // If successfull reading from scratchpad, init the ADC and go back
			break;
		}
	}
}

/**
 * @brief	Print title
 * @return	None
 */
static void print_title()
{
	printf("\r\n");
	printf("****************************************************************\r\n");
	printf("*            EVAL-CN0540-PMDZ Demonstration Program            *\r\n");
	printf("*                                                              *\r\n");
	printf("*   This program demonstrates IEPE / ICP piezo accelerometer   *\r\n");
	printf("*        interfacing and FFT measurements using AD7768-1       *\r\n");
	printf("*           Precision 24-bit sigma-delta AD converter          *\r\n");
	printf("*                                                              *\r\n");
	printf("* Set the baud rate to 230400 select the newline terminator.   *\r\n");
	printf("****************************************************************\r\n");
}

/**
 * @brief	Set power mode
 * @return	Menu status constant
 */
static int32_t menu_1_set_adc_powermode(uint32_t id)
{
	uint32_t new_pwr_mode;

	printf("\r\n Avaliable power modes: \r\n");
	printf("  1 - Low power mode \r\n");
	printf("  2 - Median power mode\r\n");
	printf("  3 - Fast power mode\r\n");
	printf(" Select an option: \r\n");

	getUserInput(&new_pwr_mode);

	switch (new_pwr_mode) {
	case 1:
		ad77681_set_power_mode(device_adc, AD77681_ECO);
		printf(" Low power mode selected\r\n");
		break;
	case 2:
		ad77681_set_power_mode(device_adc, AD77681_MEDIAN);
		printf(" Median power mode selected\r\n");
		break;
	case 3:
		ad77681_set_power_mode(device_adc, AD77681_FAST);
		printf(" Fast power mode selected\r\n");
		break;
	default:
		printf(" Invalid option\r\n");
		break;
	}

	return (MENU_CONTINUE);
}

/**
 * @brief	Set clock divider
 * @return	Menu status constant
 */
static int32_t menu_2_set_adc_clock_divider(uint32_t id)
{
	uint32_t new_mclk_div;

	printf("\r\n Avaliable MCLK divide options: \r\n");
	printf("  1 - MCLK/16\r\n");
	printf("  2 - MCLK/8\r\n");
	printf("  3 - MCLK/4\r\n");
	printf("  4 - MCLK/2\r\n");
	printf(" Select an option: \r\n");

	getUserInput(&new_mclk_div);

	switch (new_mclk_div) {
	case 1:
		ad77681_set_mclk_div(device_adc, AD77681_MCLK_DIV_16);
		printf(" MCLK/16 selected\r\n");
		break;
	case 2:
		ad77681_set_mclk_div(device_adc, AD77681_MCLK_DIV_8);
		printf(" MCLK/8 selected\r\n");
		break;
	case 3:
		ad77681_set_mclk_div(device_adc, AD77681_MCLK_DIV_4);
		printf(" MCLK/4 selected\r\n");
		break;
	case 4:
		ad77681_set_mclk_div(device_adc, AD77681_MCLK_DIV_2);
		printf(" MCLK/2 selected\r\n");
		break;
	default:
		printf(" Invalid option\r\n");
		break;
	}

	// Update the sample rate after changing the MCLK divider
	ad77681_update_sample_rate(device_adc);

	return (MENU_CONTINUE);
}

/**
 * @brief	Set filter type
 * @return	Menu status constant
 */
static int32_t menu_3_set_adc_filter_type(uint32_t id)
{
	uint32_t new_filter = 0;

	printf("\r\n Avaliable clock divide options: \r\n");
	printf(" 1 - SINC3 Fileter\r\n");
	printf(" 2 - SINC5 Filter\r\n");
	printf(" 3 - Low ripple FIR Filter\r\n");
	printf(" 4 - SINC3 50/60Hz rejection\r\n");
	printf(" 5 - User-defined FIR filter\r\n");
	printf(" Select an option: \r\n");

	getUserInput(&new_filter);

	switch (new_filter) {
	case 1:
		set_adc_SINC3_filter();
		break;
	case 2:
		set_adc_SINC5_filter();
		break;
	case 3:
		set_adc_FIR_filter();
		break;
	case 4:
		set_adc_50HZ_rej();
		break;
	case 5:
		set_adc_user_defined_FIR();
		break;
	default:
		printf(" Invalid option\r\n");
		break;
	}

	// Update the sample rate after changing the Filter type
	ad77681_update_sample_rate(device_adc);

	return (MENU_CONTINUE);
}

/**
 * @brief	AIN and REF buffers control
 * @return	Menu status constant
 */
static int32_t menu_4_adc_buffers_control(uint32_t id)
{
	uint32_t new_AIN_buffer = 0, new_REF_buffer = 0, new_buffer = 0;

	printf("\r\n Buffers settings: \r\n");
	printf("  1 - Set AIN precharge buffers\r\n");
	printf("  2 - Set REF buffers\r\n");
	printf(" Select an option: \r\n");

	getUserInput(&new_buffer);

	switch (new_buffer) {
	case 1:
		printf(" Analog IN precharge buffers settings: \r\n");
		printf("  1 - Turn ON  both precharge buffers\r\n");
		printf("  2 - Turn OFF both precharge buffers\r\n");
		printf("  3 - Turn ON  AIN- precharge buffer\r\n");
		printf("  4 - Turn OFF AIN- precharge buffer\r\n");
		printf("  5 - Turn ON  AIN+ precharge buffer\r\n");
		printf("  6 - Turn OFF AIN+ precharge buffer\r\n");
		printf(" Select an option: \r\n");

		getUserInput(&new_AIN_buffer);

		switch (new_AIN_buffer) {
		case 1:
			ad77681_set_AINn_buffer(device_adc, AD77681_AINn_ENABLED);
			ad77681_set_AINp_buffer(device_adc, AD77681_AINp_ENABLED);
			printf(" AIN+ and AIN- enabled\r\n");
			break;
		case 2:
			ad77681_set_AINn_buffer(device_adc, AD77681_AINn_DISABLED);
			ad77681_set_AINp_buffer(device_adc, AD77681_AINp_DISABLED);
			printf(" AIN+ and AIN- disabled\r\n");
			break;
		case 3:
			ad77681_set_AINn_buffer(device_adc, AD77681_AINn_ENABLED);
			printf(" AIN- Enabled\r\n");
			break;
		case 4:
			ad77681_set_AINn_buffer(device_adc, AD77681_AINn_DISABLED);
			printf(" AIN- Disabled\r\n");
			break;
		case 5:
			ad77681_set_AINp_buffer(device_adc, AD77681_AINp_ENABLED);
			printf(" AIN+ Enabled\r\n");
			break;
		case 6:
			ad77681_set_AINp_buffer(device_adc, AD77681_AINp_DISABLED);
			printf(" AIN+ Disabled\r\n");
			break;
		default:
			printf(" Invalid option\r\n");
			break;
		}
		break;
	case 2:
		printf(" REF buffers settings: \r\n");
		printf("  1 - Full REF- reference buffer\r\n");
		printf("  2 - Full REF+ reference buffer\r\n");
		printf("  3 - Unbuffered REF- reference buffer\r\n");
		printf("  4 - Unbuffered REF+ reference buffer\r\n");
		printf("  5 - Precharge  REF- reference buffer\r\n");
		printf("  6 - Precharge  REF+ reference buffer\r\n");
		printf(" Select an option: \r\n");

		getUserInput(&new_REF_buffer);

		switch (new_REF_buffer) {
		case 1:
			ad77681_set_REFn_buffer(device_adc, AD77681_BUFn_FULL_BUFFER_ON);
			printf(" Fully buffered REF-\r\n");
			break;
		case 2:
			ad77681_set_REFp_buffer(device_adc, AD77681_BUFp_FULL_BUFFER_ON);
			printf(" Fully buffered REF+\r\n");
			break;
		case 3:
			ad77681_set_REFn_buffer(device_adc, AD77681_BUFn_DISABLED);
			printf(" Unbuffered REF-\r\n");
			break;
		case 4:
			ad77681_set_REFp_buffer(device_adc, AD77681_BUFp_DISABLED);
			printf(" Unbuffered REF+\r\n");
			break;
		case 5:
			ad77681_set_REFn_buffer(device_adc, AD77681_BUFn_ENABLED);
			printf(" Precharge buffer on REF-\r\n");
			break;
		case 6:
			ad77681_set_REFp_buffer(device_adc, AD77681_BUFp_ENABLED);
			printf(" Precharge buffer on REF+\r\n");
			break;
		default:
			printf(" Invalid option\r\n");
			break;
		}
		break;
	default:
		printf(" Invalid option\r\n");
		break;
	}

	return (MENU_CONTINUE);
}

/**
 * @brief	Default ADC Settings
 * @return	Menu status constant
 */
static int32_t menu_5_set_default_settings(uint32_t id)
{
	int32_t ret;

	ret = ad77681_setup(&device_adc, init_params, &current_status);

	if (!ret) {
		printf("\r\n Default ADC settings successfull \r\n");
	} else {
		printf("\r\n Error in settings, please reset the ADC \r\n");
	}

	return (MENU_CONTINUE);
}

/**
 * @brief	VCM output control
 * @return	Menu status constant
 */
static int32_t menu_6_set_adc_vcm(uint32_t id)
{
	uint32_t new_vcm = 0;

	printf("\r\n Avaliable VCM output voltage levels: \r\n");
	printf("  1 - VCM = (AVDD1-AVSS)/2\r\n");
	printf("  2 - VCM = 2.5V\r\n");
	printf("  3 - VCM = 2.05V\r\n");
	printf("  4 - VCM = 1.9V\r\n");
	printf("  5 - VCM = 1.65V\r\n");
	printf("  6 - VCM = 1.1V\r\n");
	printf("  7 - VCM = 0.9V\r\n");
	printf("  8 - VCM off\r\n");
	printf(" Select an option: \r\n");

	getUserInput(&new_vcm);

	switch (new_vcm) {

	case 1:
		ad77681_set_VCM_output(device_adc, AD77681_VCM_HALF_VCC);
		printf(" VCM set to half of the Vcc\r\n");
		break;
	case 2:
		ad77681_set_VCM_output(device_adc, AD77681_VCM_2_5V);
		printf(" VCM set to 2.5V\r\n");
		break;
	case 3:
		ad77681_set_VCM_output(device_adc, AD77681_VCM_2_05V);
		printf(" VCM set to 2.05V\r\n");
		break;
	case 4:
		ad77681_set_VCM_output(device_adc, AD77681_VCM_1_9V);
		printf(" VCM set to 1.9V\r\n");
		break;
	case 5:
		ad77681_set_VCM_output(device_adc, AD77681_VCM_1_65V);
		printf(" VCM set to 1.65V\r\n");
		break;
	case 6:
		ad77681_set_VCM_output(device_adc, AD77681_VCM_1_1V);
		printf(" VCM set to 1.1V\r\n");
		break;
	case 7:
		ad77681_set_VCM_output(device_adc, AD77681_VCM_0_9V);
		printf(" VCM set to 0.9V\r\n");
		break;
	case 8:
		ad77681_set_VCM_output(device_adc, AD77681_VCM_OFF);
		printf(" VCM OFF\r\n");
		break;
	default:
		printf(" Invalid option\r\n");
		break;
	}

	return (MENU_CONTINUE);
}

/**
 * @brief	Register read
 * @return	Menu status constant
 */
static int32_t menu_7_adc_read_register(uint32_t id)
{
	uint32_t new_reg_to_read = 0;
	uint8_t reg_read_buf[3], read_adc_data[6];
	uint8_t HI = 0, MID = 0, LO = 0;
	char binary_number[9];

	printf("\r\n Read desired register: \r\n");
	printf("  1 - 0x03        - Chip type \r\n");
	printf("  2 - 0x14        - Interface format \r\n");
	printf("  3 - 0x15        - Power clock \r\n");
	printf("  4 - 0x16        - Analog \r\n");
	printf("  5 - 0x17        - Analog2 \r\n");
	printf("  6 - 0x18        - Conversion \r\n");
	printf("  7 - 0x19        - Digital filter \r\n");
	printf("  8 - 0x1A        - SINC3 Dec. rate MSB \r\n");
	printf("  9 - 0x1B        - SINC3 Dec. rate LSB \r\n");
	printf(" 10 - 0x1C        - Duty cycle ratio \r\n");
	printf(" 11 - 0x1D        - Sync, Reset \r\n");
	printf(" 12 - 0x1E        - GPIO Control \r\n");
	printf(" 13 - 0x1F        - GPIO Write \r\n");
	printf(" 14 - 0x20        - GPIO Read \r\n");
	printf(" 15 - 0x21 - 0x23 - Offset register \r\n");
	printf(" 16 - 0x24 - 0x26 - Gain register \r\n");
	printf(" 17 - 0x2C        - ADC Data \r\n");
	printf(" Select an option: \r\n");
	getUserInput(&new_reg_to_read);

	switch (new_reg_to_read) {
	case 1:
		ad77681_spi_reg_read(device_adc, AD77681_REG_CHIP_TYPE, reg_read_buf);
		print_binary(reg_read_buf[1], binary_number);
		printf(" Value of 0x03 - Chip type register is: 0x%x  0b%s \r\n",
		       reg_read_buf[1], binary_number);
		break;
	case 2:
		ad77681_spi_reg_read(device_adc, AD77681_REG_INTERFACE_FORMAT, reg_read_buf);
		print_binary(reg_read_buf[1], binary_number);
		printf(" Value of 0x14 - Interface format register is: 0x%x  0b%s \r\n",
		       reg_read_buf[1], binary_number);
		break;
	case 3:
		ad77681_spi_reg_read(device_adc, AD77681_REG_POWER_CLOCK, reg_read_buf);
		print_binary(reg_read_buf[1], binary_number);
		printf(" Value of 0x15 - Power clock register is: 0x%x  0b%s \r\n",
		       reg_read_buf[1], binary_number);
		break;
	case 4:
		ad77681_spi_reg_read(device_adc, AD77681_REG_ANALOG, reg_read_buf);
		print_binary(reg_read_buf[1], binary_number);
		printf(" Value of 0x16 - Anlaog register is: 0x%x  0b%s \r\n", reg_read_buf[1],
		       binary_number);
		break;
	case 5:
		ad77681_spi_reg_read(device_adc, AD77681_REG_ANALOG2, reg_read_buf);
		print_binary(reg_read_buf[1], binary_number);
		printf(" Value of 0x17 - Analog2 regster is: 0x%x  0b%s \r\n", reg_read_buf[1],
		       binary_number);
		break;
	case 6:
		ad77681_spi_reg_read(device_adc, AD77681_REG_CONVERSION, reg_read_buf);
		print_binary(reg_read_buf[1], binary_number);
		printf(" Value of 0x18 - Conversion register is: 0x%x  0b%s \r\n",
		       reg_read_buf[1], binary_number);
		break;
	case 7:
		ad77681_spi_reg_read(device_adc, AD77681_REG_DIGITAL_FILTER, reg_read_buf);
		print_binary(reg_read_buf[1], binary_number);
		printf(" Value of 0x19 - Digital filter register is: 0x%x  0b%s \r\n",
		       reg_read_buf[1], binary_number);
		break;
	case 8:
		ad77681_spi_reg_read(device_adc, AD77681_REG_SINC3_DEC_RATE_MSB, reg_read_buf);
		print_binary(reg_read_buf[1], binary_number);
		printf(" Value of 0x1A - SINC3 Dec. rate MSB is: 0x%x  0b%s \r\n",
		       reg_read_buf[1], binary_number);
		break;
	case 9:
		ad77681_spi_reg_read(device_adc, AD77681_REG_SINC3_DEC_RATE_LSB, reg_read_buf);
		print_binary(reg_read_buf[1], binary_number);
		printf(" Value of 0x1B - SINC3 Dec. rate LSB is: 0x%x  0b%s \r\n",
		       reg_read_buf[1], binary_number);
		break;
	case 10:
		ad77681_spi_reg_read(device_adc, AD77681_REG_DUTY_CYCLE_RATIO, reg_read_buf);
		print_binary(reg_read_buf[1], binary_number);
		printf(" Value of 0x1C - Duty cycle ratio 0x%x  0b%s \r\n", reg_read_buf[1],
		       binary_number);
		break;
	case 11:
		ad77681_spi_reg_read(device_adc, AD77681_REG_SYNC_RESET, reg_read_buf);
		print_binary(reg_read_buf[1], binary_number);
		printf(" Value of 0x1D - Sync, Reset 0x%x  0b%s \r\n", reg_read_buf[1],
		       binary_number);
		break;
	case 12:
		ad77681_spi_reg_read(device_adc, AD77681_REG_GPIO_CONTROL, reg_read_buf);
		print_binary(reg_read_buf[1], binary_number);
		printf(" Value of 0x1E - GPIO Controll is: 0x%x  0b%s \r\n", reg_read_buf[1],
		       binary_number);
		break;
	case 13:
		ad77681_spi_reg_read(device_adc, AD77681_REG_GPIO_WRITE, reg_read_buf);
		print_binary(reg_read_buf[1], binary_number);
		printf(" Value of 0x1F - GPIO Write is: 0x%x  0b%s \r\n", reg_read_buf[1],
		       binary_number);
		break;
	case 14:
		ad77681_spi_reg_read(device_adc, AD77681_REG_GPIO_READ, reg_read_buf);
		print_binary(reg_read_buf[1], binary_number);
		printf(" Value of 0x20 - GPIO Read is: 0x%x  0b%s \r\n", reg_read_buf[1],
		       binary_number);
		break;
	case 15:
		ad77681_spi_reg_read(device_adc, AD77681_REG_OFFSET_HI, reg_read_buf);
		HI = reg_read_buf[1];

		ad77681_spi_reg_read(device_adc, AD77681_REG_OFFSET_MID, reg_read_buf);
		MID = reg_read_buf[1];

		ad77681_spi_reg_read(device_adc, AD77681_REG_OFFSET_LO, reg_read_buf);
		LO = reg_read_buf[1];

		printf(" Value of 0x21 - 0x23 - Offset register is: 0x%x %x %x \r\n", HI, MID,
		       LO);
		break;

	case 16:
		ad77681_spi_reg_read(device_adc, AD77681_REG_GAIN_HI, reg_read_buf);
		HI = reg_read_buf[1];

		ad77681_spi_reg_read(device_adc, AD77681_REG_GAIN_MID, reg_read_buf);
		MID = reg_read_buf[1];

		ad77681_spi_reg_read(device_adc, AD77681_REG_GAIN_LO, reg_read_buf);
		LO = reg_read_buf[1];

		printf(" Value of 0x24 - 0x26 - Gain register is: 0x%x %x %x \r\n", HI, MID,
		       LO);
		break;
	case 17:
		ad77681_spi_read_adc_data(device_adc, read_adc_data,
					  AD77681_REGISTER_DATA_READ);
		printf(" Value of 0x2C - ADC data is: 0x%x 0x%x 0x%x \r\n", read_adc_data[1],
		       read_adc_data[2], read_adc_data[3]);
		break;

	default :
		printf(" Invalid option \r\n");
		break;
	}

	return (MENU_CONTINUE);
}

/**
 * @brief	Read ADC data
 * @return	Menu status constant
 */
static int32_t menu_8_adc_cont_read_data(uint32_t id)
{
	uint32_t new_sample_count = 0;
	int32_t ret;

	printf("\r\n Read Continuous ADC Data \r\n");
	printf("\r\n  Input number of samples (1 to 2048): \r\n");

	ret = getUserInput(&new_sample_count); // Get user input

	if ((new_sample_count <= 2048) && (!ret)) {
		printf("\r\n%ld of samples\r\n",
		       new_sample_count); // Print Desired Measurement Count
		measured_data.samples = (uint16_t)(new_sample_count);
		measured_data.finish = false;
		measured_data.count = 0;
		printf("Sampling....\r\n");
		cont_sampling();
		printf("Done Sampling....\r\n");
	} else {
		printf("\r\n Invalid option \r\n");
	}

	return (MENU_CONTINUE);
}

/**
 * @brief	Reset ADC
 * @return	Menu status constant
 */
static int32_t menu_9_reset_ADC(uint32_t id)
{
	uint32_t new_reset_option = 0;

	printf("\r\n ADC reset opportunities: \r\n");
	printf("  1 - Soft reset - over SPI\r\n");
	printf("  2 - Hard reset - uing RESET pin\r\n");
	printf(" Select an option: \r\n");

	getUserInput(&new_reset_option);

	switch (new_reset_option) {
	case 1:
		ad77681_soft_reset(device_adc); // Perform soft reset thru SPI write
		printf(" ADC after soft reset\r\n");
		break;
	case 2:
		adc_hard_reset();
		printf(" ADC after hard reset\r\n");
		break;
	default:
		printf(" Invalid option\r\n");
		break;
	}

	return (MENU_CONTINUE);
}

/**
 * @brief	Sleep mode / Wake up ADC
 * @return	Menu status constant
 */
static int32_t menu_10_power_down(uint32_t id)
{
	uint32_t new_sleep = 0;

	printf("\r\n Controll sleep mode of the ADC: \r\n");
	printf("  1 - Put ADC to sleep mode\r\n");
	printf("  2 - Wake up ADC\r\n");
	printf(" Select an option: \r\n");

	getUserInput(&new_sleep);

	switch (new_sleep) {
	case 1:
		ad77681_power_down(device_adc, AD77681_SLEEP);
		printf(" ADC put to sleep mode\r\n");
		break;
	case 2:
		ad77681_power_down(device_adc, AD77681_WAKE);
		printf(" ADC powered\r\n");
		break;
	default:
		printf("Invalid option\r\n");
		break;
	}

	return (MENU_CONTINUE);
}

/**
 * @brief	ADC's GPIOs control
 * @return	Menu status constant
 */
static int32_t menu_11_ADC_GPIO(uint32_t id)
{
	uint8_t GPIO_state;
	uint32_t new_gpio_sel = 0;
	char binary_number[9];

	printf("\r\n ADC GPIO Controll: \r\n");
	printf("  1 - Read from GPIO\r\n");
	printf("  2 - Write to  GPIO\r\n");
	printf("  3 - Set GPIO as input / output\r\n");
	printf("  4 - Change GPIO settings\r\n");
	printf(" Select an option: \r\n");

	getUserInput(&new_gpio_sel);

	switch (new_gpio_sel) {
	case 1:
		ad77681_gpio_read(device_adc, &GPIO_state, AD77681_ALL_GPIOS);
		print_binary(GPIO_state, binary_number);
		printf(" Current GPIO Values:\r\n GPIO0: %c\r\n GPIO1: %c\r\n GPIO2: %c\r\n GPIO3: %c\r\n",
		       binary_number[7], binary_number[6], binary_number[5], binary_number[4]);
		break;
	case 2:
		adc_GPIO_write();
		break;
	case 3:
		adc_GPIO_inout();
		break;
	case 4:
		adc_GPIO_settings();
		break;
	default:
		printf(" Invalid option\r\n");
		break;
	}

	return (MENU_CONTINUE);
}

/**
 * @brief	Read ADC status from status registers
 * @return	Menu status constant
 */
static int32_t menu_12_read_master_status(uint32_t id)
{
	char ok[3] = { 'O', 'K' }, fault[6] = { 'F', 'A', 'U', 'L', 'T' };

	ad77681_status(device_adc, current_status);

	printf("== MASTER STATUS REGISER\r\n");
	printf("Master error:          %s\r\n",
	       ((current_status->master_error == 0) ? (ok) : (fault)));
	printf("ADC error:             %s\r\n",
	       ((current_status->adc_error == 0) ? (ok) : (fault)));
	printf("Dig error:             %s\r\n",
	       ((current_status->dig_error == 0) ? (ok) : (fault)));
	printf("Ext. clock:            %s\r\n",
	       ((current_status->adc_err_ext_clk_qual == 0) ? (ok) : (fault)));
	printf("Filter saturated:      %s\r\n",
	       ((current_status->adc_filt_saturated == 0) ? (ok) : (fault)));
	printf("Filter not settled:    %s\r\n",
	       ((current_status->adc_filt_not_settled == 0) ? (ok) : (fault)));
	printf("SPI error:             %s\r\n",
	       ((current_status->spi_error == 0) ? (ok) : (fault)));
	printf("POR Flag:              %s\r\n",
	       ((current_status->por_flag == 0) ? (ok) : (fault)));

	if (current_status->spi_error == 1) {
		printf("\r\n== SPI DIAG STATUS REGISER\r\n");
		printf("SPI ignore error:      %s\r\n",
		       ((current_status->spi_ignore == 0) ? (ok) : (fault)));
		printf("SPI clock count error: %s\r\n",
		       ((current_status->spi_clock_count == 0) ? (ok) : (fault)));
		printf("SPI read error:        %s\r\n",
		       ((current_status->spi_read_error == 0) ? (ok) : (fault)));
		printf("SPI write error:       %s\r\n",
		       ((current_status->spi_write_error == 0) ? (ok) : (fault)));
		printf("SPI CRC error:         %s\r\n",
		       ((current_status->spi_crc_error == 0) ? (ok) : (fault)));
	}
	if (current_status->adc_error == 1) {
		printf("\r\n== ADC DIAG STATUS REGISER\r\n");
		printf("DLDO PSM error:        %s\r\n",
		       ((current_status->dldo_psm_error == 0) ? (ok) : (fault)));
		printf("ALDO PSM error:        %s\r\n",
		       ((current_status->aldo_psm_error == 0) ? (ok) : (fault)));
		printf("REF DET error:         %s\r\n",
		       ((current_status->ref_det_error == 0) ? (ok) : (fault)));
		printf("FILT SAT error:        %s\r\n",
		       ((current_status->filt_sat_error == 0) ? (ok) : (fault)));
		printf("FILT NOT SET error:    %s\r\n",
		       ((current_status->filt_not_set_error == 0) ? (ok) : (fault)));
		printf("EXT CLK QUAL error:    %s\r\n",
		       ((current_status->ext_clk_qual_error == 0) ? (ok) : (fault)));
	}
	if (current_status->dig_error == 1) {
		printf("\r\n== DIGITAL DIAG STATUS REGISER\r\n");
		printf("Memory map CRC error:  %s\r\n",
		       ((current_status->memoy_map_crc_error == 0) ? (ok) : (fault)));
		printf("RAM CRC error:         %s\r\n",
		       ((current_status->ram_crc_error == 0) ? (ok) : (fault)));
		printf("FUSE CRC error:        %s\r\n",
		       ((current_status->fuse_crc_error == 0) ? (ok) : (fault)));
	}

	return (MENU_CONTINUE);
}

/**
 * @brief	Set Vref anc MCLK as "external" values, depending on your setup
 * @return	Menu status constant
 */
static int32_t menu_13_mclk_vref(uint32_t id)
{
	uint32_t input = 0, new_settings = 0;
	int32_t ret;

	printf("\r\n Set Vref and Mclk: \r\n");
	printf("  1 - Change Vref\r\n");
	printf("  2 - Change MCLK\r\n");
	printf(" Select an option: \r\n");

	getUserInput(&new_settings);

	switch (new_settings) {
	case 1:
		printf(" Change Vref from %d mV to [mV]: ", device_adc->vref); // Vref change
		ret = getUserInput(&input);

		if ((input >= 1000) && (input <= 5000) && (!ret)) {
			printf("\r\n New Vref value is %ld mV", input);
			device_adc->vref = input;
		} else {
			printf(" Invalid option\r\n");
		}
		break;
	case 2:
		printf(" Change MCLK from %d kHz to [kHz]: ", device_adc->mclk); // MCLK change
		ret = getUserInput(&input);

		if ((input >= 10000) && (input <= 50000) && (!ret)) {
			printf("\r\n New MCLK value is %ld kHz\r\n", input);
			device_adc->mclk = input;
			ad77681_update_sample_rate(
				device_adc); // Update the sample rate after changing the MCLK
		} else {
			printf(" Invalid option\r\n");
		}
		break;
	default:
		printf(" Invalid option\r\n");
		break;
	}

	return (MENU_CONTINUE);
}

/**
 * @brief	Print measured data
 * @return	Menu status constant
 */
static int32_t menu_14_print_measured_data(uint32_t id)
{
	double voltage;
	int32_t shifted_data;
	uint16_t i;

	if (measured_data.finish) {
		// Printing Voltage
		printf("\r\n\nVoltage\r\n");
		for (i = 0; i < measured_data.samples; i++) {
			ad77681_data_to_voltage(device_adc, &measured_data.raw_data[i], &voltage);
			printf("%.9f \r\n", voltage);
		}
		// Printing Codes
		printf("\r\n\nCodes\r\n");
		for (i = 0; i < measured_data.samples; i++) {
			if (measured_data.raw_data[i] & 0x800000) {
				shifted_data = (int32_t)((0xFF << 24) | measured_data.raw_data[i]);
			} else {
				shifted_data = (int32_t)((0x00 << 24) | measured_data.raw_data[i]);
			}
			printf("%ld\r\n", shifted_data + AD7768_HALF_SCALE);
		}
		// Printing Raw Date
		printf("\r\n\nRaw data\r\n");
		for (i = 0; i < measured_data.samples; i++)
			printf("%ld\r\n", measured_data.raw_data[i]);
		// Set  measured_data.finish to false after Printing
		measured_data.finish = false;
	} else {
		printf("Data not prepared\r\n");
	}

	return (MENU_CONTINUE);
}

/**
 * @brief	Set data output mode
 * @return	Menu status constant
 */
static int32_t menu_15_set_adc_data_output_mode(uint32_t id)
{
	uint32_t new_data_mode = 0, new_length = 0, new_status = 0, new_crc = 0;

	printf("\r\n ADC data output modes: \r\n");
	printf("  1 - Continuous: waiting for DRDY\r\n");
	printf("  2 - Continuous one shot: waiting for SYNC_IN\r\n");
	printf("  3 - Single-conversion standby\r\n");
	printf("  4 - Periodic standby\r\n");
	printf("  5 - Standby mode\r\n");
	printf("  6 - 16bit or 24bit data format\r\n");
	printf("  7 - Status bit output\r\n");
	printf("  8 - Switch from diag mode to measure\r\n");
	printf("  9 - Switch from measure to diag mode\r\n");
	printf(" 10 - Set CRC type\r\n");
	printf(" Select an option: \r\n");

	getUserInput(&new_data_mode);

	switch (new_data_mode) {
	case 1:
		ad77681_set_conv_mode(device_adc, AD77681_CONV_CONTINUOUS,
				      device_adc->diag_mux_sel, device_adc->conv_diag_sel); // DIAG MUX NOT SELECTED
		printf(" Continuous mode set\r\n");
		break;
	case 2:
		ad77681_set_conv_mode(device_adc, AD77681_CONV_ONE_SHOT,
				      device_adc->diag_mux_sel, device_adc->conv_diag_sel);
		printf(" Continuous one shot conversion set\r\n");
		break;
	case 3:
		ad77681_set_conv_mode(device_adc, AD77681_CONV_SINGLE, device_adc->diag_mux_sel,
				      device_adc->conv_diag_sel);
		printf(" Single conversion standby mode set\r\n");
		break;
	case 4:
		ad77681_set_conv_mode(device_adc, AD77681_CONV_PERIODIC,
				      device_adc->diag_mux_sel, device_adc->conv_diag_sel);
		printf(" Periodiec standby mode set\r\n");
		break;
	case 5:
		ad77681_set_conv_mode(device_adc, AD77681_CONV_STANDBY,
				      device_adc->diag_mux_sel, device_adc->conv_diag_sel);
		printf(" Standby mode set\r\n");
		break;
	case 6:
		printf(" Conversion length select: \r\n");
		printf("  1 - 24bit length\r\n");
		printf("  2 - 16bit length\r\n");

		getUserInput(&new_length);

		switch (new_length) {
		case 1:
			ad77681_set_convlen(device_adc, AD77681_CONV_24BIT);
			printf(" 24bit data output format selected\r\n");
			break;
		case 2:
			ad77681_set_convlen(device_adc, AD77681_CONV_16BIT);
			printf(" 16bit data output format selected\r\n");
			break;
		default:
			printf(" Invalid option\r\n");
			break;
		}
		break;
	case 7:
		printf(" Status bit output: \r\n");
		printf("  1 - Enable status bit after each ADC conversion\r\n");
		printf("  2 - Disable status bit after each ADC conversion\r\n");

		getUserInput(&new_status);

		switch (new_status) {
		case 1:
			ad77681_set_status_bit(device_adc, true);
			printf(" Status bit enabled\r\n");
			break;
		case 2:
			ad77681_set_status_bit(device_adc, false);
			printf(" Status bit disabled\r\n");
			break;
		default:
			printf(" Invalid option\r\n");
			break;
		}
		break;
	case 8:
		ad77681_set_conv_mode(device_adc, device_adc->conv_mode,
				      device_adc->diag_mux_sel, false); // DIAG MUX NOT SELECTED
		printf(" Measure mode selected\n");
		break;
	case 9:
		ad77681_set_conv_mode(device_adc, device_adc->conv_mode,
				      device_adc->diag_mux_sel, true); // DIAG MUX SELECTED
		printf(" Diagnostic mode selected\n");
		break;
	case 10:
		printf(" CRC settings \r\n");
		printf("  1 - Disable CRC\r\n");
		printf("  2 - 8-bit polynomial CRC\r\n");
		printf("  3 - XOR based CRC\r\n");

		getUserInput(&new_crc);

		switch (new_crc) {
		case 1:
			ad77681_set_crc_sel(device_adc, AD77681_NO_CRC);
			printf(" CRC disabled\r\n");
			break;
		case 2:
			ad77681_set_crc_sel(device_adc, AD77681_CRC);
			printf("  8-bit polynomial CRC method selected\r\n");
			break;
		case 3:
			ad77681_set_crc_sel(device_adc, AD77681_XOR);
			printf("  XOR based CRC method selected\r\n");
			break;
		default:
			printf(" Invalid option\r\n");
			break;
		}
		break;
	default:
		printf(" Invalid option\r\n");
		break;
	}

	return (MENU_CONTINUE);
}

/**
 * @brief	Set diagnostic mode
 * @return	Menu status constant
 */
static int32_t menu_16_set_adc_diagnostic_mode(uint32_t id)
{
	uint32_t new_diag_mode = 0;

	printf("\r\n ADC diagnostic modes: \r\n");
	printf("  1 - Internal temperature sensor\r\n");
	printf("  2 - AIN shorted\r\n");
	printf("  3 - Positive full-scale\r\n");
	printf("  4 - Negative full-scale\r\n");
	printf(" Select an option: \r\n");

	getUserInput(&new_diag_mode);

	switch (new_diag_mode) {
	case 1:
		ad77681_set_conv_mode(device_adc, device_adc->conv_mode, AD77681_TEMP_SENSOR,
				      true);
		printf(" Diagnostic mode: Internal temperature sensor selected\r\n");
		break;
	case 2:
		ad77681_set_conv_mode(device_adc, device_adc->conv_mode, AD77681_AIN_SHORT,
				      true);
		printf(" Diagnostic mode: AIN shorted selected\r\n");
		break;
	case 3:
		ad77681_set_conv_mode(device_adc, device_adc->conv_mode, AD77681_POSITIVE_FS,
				      true);
		printf(" Diagnostic mode: Positive full-scale selected\r\n");
		break;
	case 4:
		ad77681_set_conv_mode(device_adc, device_adc->conv_mode, AD77681_NEGATIVE_FS,
				      true);
		printf(" Diagnostic mode: Negative full-scale selected\r\n");
		break;
	default:
		printf(" Invalid option\r\n");
		break;
	}

	return (MENU_CONTINUE);
}

/**
 * @brief	Perform FFT
 * @return	Menu status constant
 */
static int32_t menu_17_do_the_fft(uint32_t id)
{
	printf("\r\n FFT in progress...\r\n");
	measured_data.samples = fft_proc_params.fft_length;
	measured_data.finish = false;
	measured_data.count = 0;

	fft_proc_params.sample_rate = device_adc->sample_rate;
	fft_proc_params.vref = device_adc->vref / 1000;

	printf("Sampling....\r\n");
	cont_sampling();

	for (int i = 0; i < ADI_FFT_MAX_SAMPLES; i++) {
		fft_proc_params.input_data[i] = measured_data.raw_data[i];
	}

	adi_fft_perform(&fft_proc_params, &fft_meas_params);
	printf(" FFT Done!\r\n");
	measured_data.finish = false;

	printf("\r\n THD:\t\t%.3f dB", fft_meas_params.THD);
	printf("\r\n SNR:\t\t%.3f dB", fft_meas_params.SNR);
	printf("\r\n DR:\t\t%.3f dB", fft_meas_params.DR);
	printf("\r\n Fundamental:\t%.3f dBFS", fft_meas_params.harmonics_mag_dbfs[0]);
	printf("\r\n Fundamental:\t%.3f Hz",
	       fft_meas_params.harmonics_freq[0]*fft_proc_params.bin_width);
	printf("\r\n RMS noise:\t%.6f uV", fft_meas_params.RMS_noise * 1000000.0);
	printf("\r\n LSB noise:\t%.3f", fft_meas_params.transition_noise_LSB);

	return (MENU_CONTINUE);
}

/**
 * @brief	Set FFT module
 * @return	Menu status constant
 */
static int32_t menu_18_fft_settings(uint32_t id)
{
	uint32_t new_menu_select, new_window, new_sample_count;

	printf(" FFT settings: \r\n");
	printf("  1 - Set window type\r\n");
	printf("  2 - Set sample count\r\n");
	printf("  3 - Print FFT plot\r\n");
	printf(" Select an option: \r\n\n");

	getUserInput(&new_menu_select);

	switch (new_menu_select) {
	case 1:
		printf(" Choose window type:\r\n");
		printf("  1 - 7-term Blackman-Harris\r\n");
		printf("  2 - Rectangular\r\n");

		getUserInput(&new_window);

		switch (new_window) {
		case 1:
			printf("  7-term Blackman-Harris window selected\r\n");
			fft_proc_params.window = BLACKMAN_HARRIS_7TERM;
			break;
		case 2:
			printf("  Rectalngular window selected\r\n");
			fft_proc_params.window = RECTANGULAR;
			break;
		default:
			printf(" Invalid option\r\n");
			break;
		}
		break;
	case 2:
		printf(" Set sample count:\r\n");
		printf("  1 - 2048 samples\r\n");
		printf("  2 - 1024 samples\r\n");
		printf("  3 - 256  samples\r\n");
		printf("  4 - 64   samples\r\n");
		printf("  5 - 16   samples\r\n");

		getUserInput(&new_sample_count);

		switch (new_sample_count) {
		case 1:
			printf(" 2048 samples selected\r\n");
			fft_proc_params.fft_length = 2048;
			break;
		case 2:
			printf(" 1024 samples selected\r\n");
			fft_proc_params.fft_length = 1024;
			break;
		case 3:
			printf(" 256 samples selected\r\n");
			fft_proc_params.fft_length = 256;
			break;
		case 4:
			printf(" 64 samples selected\r\n");
			fft_proc_params.fft_length = 64;
			break;
		case 5:
			printf(" 16 samples selected\r\n");
			fft_proc_params.fft_length = 16;
			break;
		default:
			printf(" Invalid option\r\n");
			break;
		}
		break;
	case 3:
		if (fft_proc_params.fft_done == true) {
			printf(" Printing FFT plot in dB:\r\n");

			for (uint16_t i = 0; i < fft_proc_params.fft_length; i++)
				printf("%.4f\r\n", fft_proc_params.fft_dB[i]);
		} else {
			printf(" Data not prepared\r\n");
		}
		break;
	default:
		printf(" Invalid option\r\n");
		break;
	}

	return (MENU_CONTINUE);
}

/**
 * @brief	Set gain and offset
 * @return	Menu status constant
 */
static int32_t menu_19_gains_offsets(uint32_t id)
{
	uint32_t gain_offset, new_menu_select;
	int32_t ret;

	printf("\r\n Gains and Offsets settings: \r\n");
	printf("  1 - Set gain\r\n");
	printf("  2 - Set offset\r\n");
	printf(" Select an option: \r\n");

	getUserInput(&new_menu_select);

	switch (new_menu_select) {
	case 1:
		printf(" Insert new Gain value in decimal form\r\n");
		ret = getUserInput(&gain_offset);

		if ((gain_offset <= 0xFFFFFF) && (!ret)) {
			ad77681_apply_gain(device_adc, gain_offset);
			printf(" Value %ld has been successfully inserted to the Gain register\r\n",
			       gain_offset);
		} else {
			printf(" Invalid value\r\n");
		}
		break;
	case 2:
		printf(" Insert new Offset value in decimal form\r\n");
		ret = getUserInput(&gain_offset);
		if ((gain_offset <= 0xFFFFFF) && (!ret)) {
			ad77681_apply_offset(device_adc, gain_offset);
			printf(" Value %ld has been successfully inserted to the Offset register\r\n",
			       gain_offset);
		} else {
			printf(" Invalid value\r\n");
		}
		break;
	default:
		printf(" Invalid option\r\n");
		break;
	}

	return (MENU_CONTINUE);
}

/**
 * @brief	Check read and write functionality using scratchpad register
 * @return	Menu status constant
 */
static int32_t menu_20_check_scratchpad(uint32_t id)
{
	int32_t ret;
	uint32_t new_menu_select;
	uint8_t check_sequence;

	printf("\r\n Scratchpad check \r\n");
	printf("  Insert 8-bit number for scratchpad check: \r\n");

	ret = getUserInput(&new_menu_select);

	if ((new_menu_select <= 0xFF) && (!ret)) {
		check_sequence = (uint8_t)(new_menu_select);
		ret = ad77681_scratchpad(device_adc, &check_sequence);
		printf("  Inserted sequence:  %ld\r\n  Returned sequence: %d\r\n",
		       new_menu_select,
		       check_sequence);
		if (!ret) {
			printf("  SUCCESS!\r\n");
		} else {
			printf("  FAILURE!\r\n");
		}
	} else {
		printf("  Invalid value\r\n");
	}

	return (MENU_CONTINUE);
}

/**
 * @brief	Piezo accelerometer offset compensation
 * @details	GPIO channels that are selected, with the selection being stored in
 *			The offset compenzation process uses a successive approximation model.
 *			Due to the significant noise from the piezo accelerometer,
 *			extensive averaging is required, which will take some time
 * @return	Menu status constant
 */
static int32_t menu_21_piezo_offset(uint32_t id)
{
	uint8_t ltc2606_res = 16;
	uint32_t dac_code = 0;
	uint32_t dac_code_arr[16];
	double mean_voltage = 0.0, min_voltage;
	double mean_voltage_arr[16];
	int8_t sar_loop, min_find, min_index = 0;
	uint16_t SINC3_odr;

	// Low power mode and MCLK/16
	ad77681_set_power_mode(device_adc, AD77681_ECO);
	ad77681_set_mclk_div(device_adc, AD77681_MCLK_DIV_16);

	// 4SPS = 7999 SINC3, 10SPS = 3199 SINC3, 50SPS = 639 SINC3
	ad77681_SINC3_ODR(device_adc, &SINC3_odr, 4);
	// Set the oversamplig ratio to high value, to extract DC
	ad77681_set_filter_type(device_adc, AD77681_SINC5_FIR_DECx32, AD77681_SINC3,
				SINC3_odr);

	// successive approximation algorithm
	printf("\r\nInitialize SAR loop (DAC MSB set to high)\r\n");

	// Set DAC code to half scale
	dac_code = (1 << (ltc2606_res - 1));

	// Update output of the DAC
	ltc26x6_write_code(device_dac, write_update_command, dac_code);

	// Wait for DAC output to settle
	no_os_mdelay(500);

	// Set desired number of samples for every iteration
	measured_data.samples = 100;
	measured_data.finish = false;
	measured_data.count = 0;

	// Take X number of samples
	cont_sampling();

	// Get the mean voltage of taken samples stroed in the measured_data strucutre
	get_mean_voltage(&measured_data, &mean_voltage);

	// Print the mean ADC read voltage for a given DAC code
	printf("DAC code:%x\t\tMean Voltage: %.6f\r\n", (unsigned int)dac_code,
	       mean_voltage);

	// Store the initial DAC code in the array
	dac_code_arr[ltc2606_res - 1] = dac_code;

	// Store the initial mean voltage in the array
	mean_voltage_arr[ltc2606_res - 1] = mean_voltage;

	for (sar_loop = ltc2606_res - 1; sar_loop > 0; sar_loop--) {
		// Check if the mean voltage is positive or negative
		if (mean_voltage > 0) {
			dac_code = dac_code + (1 << (sar_loop - 1));
			printf("UP\r\n\n");
		} else {
			dac_code = dac_code - (1 << (sar_loop)) + (1 << (sar_loop - 1));
			printf("DOWN\r\n\n");
		}
		// Print loop coard
		printf("SAR loop #: %d\r\n", sar_loop);

		// Update output of the DAC
		ltc26x6_write_code(device_dac, write_update_command, dac_code);

		// Wait for DAC output to settle
		no_os_mdelay(500);

		// Clear data finish flag
		measured_data.finish = false;
		measured_data.count = 0;

		// Take X number of samples
		cont_sampling();

		// Get the mean voltage of taken samples stroed in the measured_data strucutre
		get_mean_voltage(&measured_data, &mean_voltage);
		printf("DAC code:%x\t\tMean Voltage: %.6f\r\n", (unsigned int)dac_code,
		       mean_voltage);
		dac_code_arr[sar_loop - 1] = dac_code;
		mean_voltage_arr[sar_loop - 1] = mean_voltage;
	}
	min_voltage = fabs(mean_voltage_arr[0]);
	for (min_find = 0; min_find < 16; min_find++) {
		if (min_voltage > fabs(mean_voltage_arr[min_find])) {
			min_voltage = fabs(mean_voltage_arr[min_find]);
			min_index = min_find;
		}
	}

	ltc26x6_write_code(device_dac, write_update_command, dac_code_arr[min_index]);

	// Wait for DAC output to settle
	no_os_mdelay(500);

	// Print the final DAC code
	printf("\r\nFinal DAC code set to:0x%x\t\tFinal Mean Voltage: %.6f\r\n",
	       (unsigned int)dac_code_arr[min_index], mean_voltage_arr[min_index]);

	// Set to original filter
	ad77681_set_filter_type(device_adc, AD77681_SINC5_FIR_DECx32, AD77681_FIR, 0);

	ad77681_update_sample_rate(device_adc);
	printf("\r\nOffset compenzation done!\r\n");

	return (MENU_CONTINUE);
}

/**
 * @brief	Set output of the on-board DAC in codes or in voltage
 * @return	Menu status constant
 */
static int32_t menu_22_set_DAC_output(uint32_t id)
{
	int16_t dac_status;
	uint16_t  code;
	uint32_t new_menu_select, new_dac;
	float dac_voltage;
	// Gain factor of the on-board DAC buffer, to have full 5V range(ADA4807-1ARJZ)
	// Non-inverting op-amp resistor ratio => 1 + (2.7 k ohm / 2.7 k ohm)
	float buffer_gain =  2;

	printf("\r\n Set DAC output: \r\n");
	printf("  1 - Voltage\r\n");
	printf("  2 - Codes\r\n");
	printf(" Select an option: \r\n");

	getUserInput(&new_menu_select);

	switch (new_menu_select) {
	case 1:
		printf(" Set DAC output in mV: ");
		getUserInput(&new_dac);

		dac_voltage = ((float)(new_dac) / 1000.0) / buffer_gain;
		dac_status = ltc26x6_voltage_to_code(device_dac, dac_voltage, &code);
		ltc26x6_write_code(device_dac, write_update_command, code);
		if (!dac_status) {
			printf("%.3f V at Shift output\r\n\n", dac_voltage * buffer_gain);
		} else if (dac_status == LTC26X6_CODE_OVERFLOW) {
			printf("%.3f V at Shift output, OVERFLOW!\r\n\n", dac_voltage * buffer_gain);
		} else if (dac_status == LTC26X6_CODE_UNDERFLOW) {
			printf("%.3f V at Shift output, UNDERFLOW!\r\n\n", dac_voltage * buffer_gain);
		}
		break;
	case 2:
		printf(" Set DAC codes in decimal form: ");
		getUserInput(&new_dac);
		ltc26x6_write_code(device_dac, write_update_command, new_dac);
		printf("0x%x at DAC output\r\n\n", (unsigned int)new_dac);
		break;
	default:
		printf(" Invalid option\r\n");
		break;
	}

	return (MENU_CONTINUE);
}

/*
 * Definition of the Main Menu Items and menu itself
 */
console_menu_item main_menu_items[] = {
	{ "Set ADC power mode", 'A', menu_1_set_adc_powermode, NULL },
	{ "Set ADC MCLK divider", 'B', menu_2_set_adc_clock_divider, NULL },
	{ "Set ADC filter type", 'C', menu_3_set_adc_filter_type, NULL },
	{ "Set ADC AIN and REF buffers", 'D', menu_4_adc_buffers_control, NULL },
	{ "Set ADC to default config", 'E', menu_5_set_default_settings, NULL },
	{ "Set ADC VCM output", 'F', menu_6_set_adc_vcm, NULL },
	{ "Read desired ADC register", 'G', menu_7_adc_read_register, NULL },
	{ "Read continuous ADC data", 'H', menu_8_adc_cont_read_data, NULL },
	{ "Reset ADC", 'I', menu_9_reset_ADC, NULL },
	{ "ADC Power-down", 'J', menu_10_power_down, NULL },
	{ "Set ADC GPIOs", 'K', menu_11_ADC_GPIO, NULL },
	{ "Read ADC master status", 'L', menu_12_read_master_status, NULL },
	{ "Set ADC Vref and MCLK", 'M', menu_13_mclk_vref, NULL },
	{ "Print ADC measured data", 'N', menu_14_print_measured_data, NULL },
	{ "Set ADC data output mode", 'O', menu_15_set_adc_data_output_mode, NULL },
	{ "Set ADC diagnostic mode", 'P', menu_16_set_adc_diagnostic_mode, NULL },
	{ "Do the FFT", 'Q', menu_17_do_the_fft, NULL },
	{ "FFT settings", 'R', menu_18_fft_settings, NULL },
	{ "Set ADC Gains, Offsets", 'S', menu_19_gains_offsets, NULL },
	{ "ADC Scratchpad Check", 'T', menu_20_check_scratchpad, NULL },
	{ "Compensate Piezo sensor offset", 'U', menu_21_piezo_offset, NULL },
	{ "Set DAC output", 'V', menu_22_set_DAC_output, NULL },
};

console_menu cn0540_main_menu = {
	.title = "CN0540 Main Menu",
	.items = main_menu_items,
	.itemCount = ARRAY_SIZE(main_menu_items),
	.headerItem = NULL,
	.footerItem = NULL,
	.enableEscapeKey = false
};

/**
 * @brief	Initialize CN0540
 * @return	0 in case of success, negative error code otherwise.
 */
int32_t cn0540_app_initalization(void)
{
	/* Initialize the AD7124 application before the main loop */
	int32_t ret;

	ret = no_os_uart_init(&uart_desc, &uart_init_param);
	if (ret) {
		return ret;
	}

	/* Set up the UART for standard I/O operations */
	no_os_uart_stdio(uart_desc);

	ret = sdpk1_gpio_setup(); // Setup SDP-K1 GPIOs
	if (ret) {
		return ret;
	}

	ret = adc_hard_reset(); // Perform ADC hard reset
	if (ret) {
		return ret;
	}

	ret = gpio_trigger_init();
	if (ret) {
		return ret;
	}

	ret = ad77681_setup(&device_adc, init_params,
			    &current_status); // SETUP and check connection
	if (ret) {
		go_to_error();
	}

	ret = ltc26x6_init(&device_dac, init_params_dac); // Initialize DAC
	if (ret) {
		return ret;
	}

	ret = adi_fft_init(&fft_init_params, &fft_proc_params, &fft_meas_params);
	if (ret) {
		return ret;
	}

	print_title();	// Print the title of the application

	return 0;
}