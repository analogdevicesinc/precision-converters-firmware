/***************************************************************************
 *   @file    ltc2488.c
 *   @brief   Implementation of LTC2488 Driver
 *
********************************************************************************
 * Copyright (c) 2021-22 Analog Devices, Inc.
 *
 * All rights reserved.
 *
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 * By using this software you agree to the terms of the associated
 * Analog Devices Software License Agreement.
 *
*****************************************************************************/

/***************************************************************************//**
* SPI DATA FORMAT (MSB First):
*             Byte #1                            Byte #2
*
* Data Out :  !EOC DMY SIG MSB D15 D14 D13 D12   D11 D10 D9  D8  D7  D6  D5  D4
* Data In  :   1   0   EN  SGL OS  A2  A1  A0    X   X   X   X   X   X   X   X
*
*			  Byte #3
* Data Out :  D3  D2  D1  D0  -   -   -   -
* Data In  :  X   X   X   X   X   X   X   X
*
* !EOC : End of Conversion Bit (Active Low)
* DMY  : Dummy Bit (Always 0)
* SIG  : Sign Bit (1-data positive, 0-data negative)
* MSB  : Most Significant Bit (Provides under range and over range indication)
* Dx   : Data Bits
* EN   : Enable Bit (0-keep previous mode, 1-change mode)
* SGL  : Enable Single-Ended Bit (0-differential, 1-single-ended)
* OS   : ODD/Sign Bit
* Sx   : Address Select Bit
*
* Command Byte
* 1    0    EN   SGL  OS   A2   A1   A0   Comments
* 1    0    0    X    X    X    X    X    Keep Previous Mode
* 1    0    1    0    X    X    X    X    Differential Mode
* 1    0    1    1    X    X    X    X    Single-Ended Mode
*
*******************************************************************************/

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include "ltc2488.h"
#include "no_os_error.h"
#include "no_os_delay.h"

/******************************************************************************/
/*************************** Function Definitions *****************************/

/***************************************************************************//**
 * @brief  Extracts the actual 17-Bit ADC value from the ADC code, returns the
 *         32-bit sign extended value along with over/underrange status.
 *         The ADC value to be returned has been to limited to +1 the max 17-bit
 *         positive value (for over-range) or -1 the minimum 17-bit value
 *         (for under-range) to avoid the ADC value being misinterpreted when
 *         not in desired range.
 *
 * @param adc_code - Pointer to Read-Only 24bit ADC output word in 32bit format
 *
 * @param adc_value - Pointer to buffer to store the 32 bit sign extended
 *                   adc value.
 *                   Under-Range : 0xFFFDFFFF , Over-Range : 0x00020000
 *
 * @return Input range
*******************************************************************************/
enum input_status ltc2488_data_process(const uint32_t *adc_code,
				       int32_t *adc_value)
{
	//Checks for the input range.
	enum input_status range_check = LTC2488_INPUT_RANGE(*adc_code);

	switch(range_check) {

	case OVER_RANGE : // Limits the ADC value to +1 the max 17-bit positive value
		*adc_value = 0x00020000;
		break;

	case UNDER_RANGE : // Limits the ADC value to -1 the min 17-bit negative value
		*adc_value = 0xFFFDFFFF;
		break;

	default: // ADC value is processed if it lies between -0.5*Vref and +0.5*Vref .
		*adc_value = LTC2488_SIGN_EXTEND_ADC_DATA(LTC2488_GET_ADC_DATA(*adc_code));
	}

	return range_check;
}

/***************************************************************************//**
 * @brief  Calculates the voltage corresponding to an adc code, given the
 *         reference voltage (in volts). Currently this function only supports
 *         the single-ended configuration.
 *
 * @param adc_code - Pointer to Read-Only 24bit ADC output word in 32bit format
 *
 * @return Corresponding Voltage Output
*******************************************************************************/
float ltc2488_code_to_voltage(const int32_t *adc_data)
{
	// This calculates the input as a fraction of the reference voltage
	// as the ADC is of 16 bit - 1LSB i.e ( 2^16 - 1) = 65535
	// Multiplying fraction by FS (Full Sacle ) to get the actual voltage
	// (in volts)
	float voltage = (float)((*adc_data) / 65535.0) * (LTC2488_FS_VOLTAGE);

	// LTC2488 accepts negative input voltage upto -0.3V below ground as
	// per the data sheet/fig 28.

	return voltage;
}

/***************************************************************************//**
 * @brief Initialize the ltc2488 device structure.
 *
 * Performs memory allocation of the device structure.
 *
 * @param device     - Pointer to location of device structure to write.
 * @param init_param - Pointer to configuration of the driver.
 *
 * @return ret - return code.
 *         Example: FAILURE- Errors Encountered.
 *                  SUCCESS - No errors encountered.
*******************************************************************************/
int32_t ltc2488_init(struct ltc2488_dev **device,
		     struct ltc2488_dev_init *init_param)
{
	struct ltc2488_dev *dev;
	int32_t ret;

	dev = (struct ltc2488_dev *)calloc(1, sizeof(*dev));
	if (!dev) {
		ret = -ENOMEM;
		goto error;
	}

	/* wait DEVICE_SETUP time */
	no_os_udelay(253);

	ret = no_os_spi_init(&dev->spi_desc, &init_param->spi_init);
	if (ret < 0)
		goto error;

	*device = dev;

	printf("ltc2488 successfully initialized\n");
	return ret;

error:
	printf("ltc2488 initialization failed\n");
	ltc2488_remove(dev);
	return ret;
}

/***************************************************************************//**
 * @brief Free any resource used by the driver.
 *
 * @param dev - The device structure.
 *
 * @return ret - return code.
 *         Example: -EIO - SPI communication error.
 *                  SUCCESS - No errors encountered.
*******************************************************************************/
int32_t ltc2488_remove(struct ltc2488_dev *dev)
{
	int32_t ret;

	ret = no_os_spi_remove(dev->spi_desc);

	free(dev);

	return ret;
}

/***************************************************************************//**
 * @brief  Reads/writes data from/to LTC2488 ADC that accepts a 8 bit configuration
 *         and returns a 24 bit result.
 *
 * @param device - Pointer to the lcoation of SPI device descriptor.
 * @param adc_cmd - 8 bit adc command word.
 * @param adc_buff - The buffer to receive 24 bit output data word.
 *
 * @return Status of SPI transaction
*******************************************************************************/
int32_t ltc2488_read_write(struct no_os_spi_desc *desc,
			   uint8_t adc_cmd,
			   uint32_t *adc_buff)
{
	int32_t ret;
	uint8_t buff[3] = { 0 };

	// First byte to be sent should contain ADC configuration
	// and rest two bytes are dummy to read out the remaining
	// data from ADC
	buff[0] = adc_cmd;
	ret = no_os_spi_write_and_read(desc, buff, 3);

	//Packing 24 bit output data in 32 bit buffer.
	*adc_buff = (buff[0] << 16) | (buff[1] << 8) | buff[0];

	return ret;
}
