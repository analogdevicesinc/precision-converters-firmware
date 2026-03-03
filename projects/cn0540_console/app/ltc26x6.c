/***************************************************************************//**
 *   @file   ltc26x6.c
 *   @brief  Implementation of LTC2606,LTC2616,LTC2626 Driver.
********************************************************************************
 * Copyright 2021(c) Analog Devices, Inc.
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *  - Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  - Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *  - Neither the name of Analog Devices, Inc. nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *  - The use of this software may or may not infringe the patent rights
 *    of one or more patent holders.  This license does not release you
 *    from the requirement that you obtain separate licenses from these
 *    patent holders to use this software.
 *  - Use of the software either in source or binary form, must be run
 *    on or directly connected to an Analog Devices Inc. component.
 *
 * THIS SOFTWARE IS PROVIDED BY ANALOG DEVICES "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, NON-INFRINGEMENT,
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL ANALOG DEVICES BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, INTELLECTUAL PROPERTY RIGHTS, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*******************************************************************************/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>

#include "ltc26x6.h"

/**
 * Initialize the device.
 * @param device - The device structure.
 * @param init_param - The structure that contains the device initial
 *                     parameters.
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t ltc26x6_init(struct ltc26x6_dev **device,
		     struct ltc26x6_init_param init_param)
{
	struct ltc26x6_dev *dev;
	int32_t ret;

	dev = (struct ltc26x6_dev *)malloc(sizeof(*dev));
	if (!dev)
		return -1;

	ret = no_os_i2c_init(&dev->i2c_desc, &init_param.i2c_init);
	dev->resolution = init_param.resolution;
	dev->vref = init_param.vref;
	dev->typical_offset = init_param.typical_offset;
	*device = dev;
	return ret;
}

/**
 * Calculates an LTC26X6 DAC code for the desired output voltage.
 * Based on the desired output voltage, the offset, and lsb parameters, return the corresponding DAC code that should be written to the LTC26X6.
 * @param dac_voltage       Desired output voltage
 * @param *code             Returned DAC code
 * @return 0 in case of success, negative error code otherwise.
 */
int16_t ltc26x6_voltage_to_code(struct ltc26x6_dev *device, float dac_voltage,
				uint16_t *code)
{
	uint32_t ltc26x6_full_scale = (2 << device->resolution) - 1;
	// The LTC26X6 least significant bit value with given voltage reference
	float   ltc26x6_lsb = (float)(device->vref / ltc26x6_full_scale);
	int32_t dac_code, ret;
	float float_code;

	// 1) Calculate the DAC code: (DAC voltage/LSB)- typical offset volatge
	float_code = (dac_voltage / ltc26x6_lsb) - device->typical_offset;
	// 2) Round
	float_code = (float_code > (floor(float_code) + 0.5)) ? ceil(
			     float_code) : floor(float_code);
	// 3) Convert to unsigned integer
	dac_code = (int32_t)(float_code);

	if (dac_code >  ltc26x6_full_scale) {
		// Requesetd voltage is bigger than reference voltage
		// Return fullscale code
		dac_code =  ltc26x6_full_scale;
		// Return Overflow error
		ret = LTC26X6_CODE_OVERFLOW;
	} else if (dac_code < 0) {
		// Requested voltage is lower than offset voltage
		dac_code = 0;
		// Return Underflow error
		ret = LTC26X6_CODE_UNDERFLOW;
	}
	//else // Requestedd volage is in supported range
	//ret = SUCCESS;

	*code = ((uint16_t)(dac_code));
	return 0;
}

/**
 * Write code to LTC26X6
 * @param dev               The device structure.
 * @param dac_command       Command for the DAC
 *                          Accepted values:    write_command
 *                                              write_update_command
 *
 * @param dac_code          16-bit DAC output voltage code
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t ltc26x6_write_code(struct ltc26x6_dev *dev,
			   enum ltc26x6_write_command write_command, uint16_t dac_code)
{
	uint8_t data[3];

	data[0] = write_command;      // Write command
	data[1] = dac_code >> 8;      // MSB code
	data[2] = dac_code & 0x00FF;  // LSB code

	return (no_os_i2c_write(dev->i2c_desc, data, 3, 1));
}

/**
 * Power down the device
 * @param device - The device structure.
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t ltc26x6_power_down(struct ltc26x6_dev *dev)
{
	uint8_t command = LTC26X6_POWER_DOWN_COMMAND;
	return (no_os_i2c_write(dev->i2c_desc, &command, 1, 1));
}

/**
 * Power up the device
 * @param device - The device structure.
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t ltc26x6_power_up(struct ltc26x6_dev *dev)
{
	uint8_t command = LTC26X6_UPDATE_COMMAND;
	return (no_os_i2c_write(dev->i2c_desc, &command, 1, 1));
}

