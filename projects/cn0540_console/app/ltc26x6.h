/***************************************************************************//**
 *   @file   ltc26x6.c
 *   @brief  Header file of LTC2606,LTC2616,LTC2626 Driver.
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
#ifndef _LTC26X6_H_
#define _LTC26X6_H_

#include <stdint.h>
#include "no_os_i2c.h"

/******************************************************************************/
/**************         MACROS AND CONSTANT DEFINITIONS          **************/
/******************************************************************************/

// LTC2606 I2C Address                 //  AD2       AD1       AD0
// #define LTC2606_I2C_ADDRESS 0x10    //  LOW       LOW       LOW
// #define LTC2606_I2C_ADDRESS 0x11    //  LOW       LOW       Float
// #define LTC2606_I2C_ADDRESS 0x12    //  LOW       LOW       HIGH
// #define LTC2606_I2C_ADDRESS 0x13    //  LOW       Float     LOW
// #define LTC2606_I2C_ADDRESS 0x20    //  LOW       Float     Float
// #define LTC2606_I2C_ADDRESS 0x21    //  LOW       Float     High
// #define LTC2606_I2C_ADDRESS 0x22    //  LOW       HIGH      LOW
// #define LTC2606_I2C_ADDRESS 0x23    //  LOW       HIGH      Float
// #define LTC2606_I2C_ADDRESS 0x30    //  LOW       High      HIGH
// #define LTC2606_I2C_ADDRESS 0x31    //  Float     LOW       LOW
// #define LTC2606_I2C_ADDRESS 0x32    //  Float     LOW       Float
// #define LTC2606_I2C_ADDRESS 0x33    //  Float     LOW       HIGH
// #define LTC2606_I2C_ADDRESS 0x40    //  Float     Float     LOW
// #define LTC2606_I2C_ADDRESS 0x41    //  Float     Float     Float
// #define LTC2606_I2C_ADDRESS 0x42    //  Float     Float     HIGH
// #define LTC2606_I2C_ADDRESS 0x43    //  Float     High      LOW
// #define LTC2606_I2C_ADDRESS 0x50    //  Float     High      Float
// #define LTC2606_I2C_ADDRESS 0x51    //  Float     High      HIGH
// #define LTC2606_I2C_ADDRESS 0x52    //  High      LOW       LOW
// #define LTC2606_I2C_ADDRESS 0x53    //  High      LOW       Float
// #define LTC2606_I2C_ADDRESS 0x60    //  High      LOW       High
// #define LTC2606_I2C_ADDRESS 0x61    //  High      Float     LOW
// #define LTC2606_I2C_ADDRESS 0x62    //  High      Float     Float
// #define LTC2606_I2C_ADDRESS 0x63    //  High      Float     High
// #define LTC2606_I2C_ADDRESS 0x70    //  High      High      LOW
// #define LTC2606_I2C_ADDRESS 0x71    //  High      High      Float
// #define LTC2606_I2C_ADDRESS 0x72    //  High      High      High


#define LTC26X6_I2C_GLOBAL_ADDRESS      0x73            //  Global Address

#define LTC26X6_UPDATE_COMMAND          0x10            // Command to update (and power up) LTC2606. Output voltage will be set to the value stored in the internal register by previous write command.
#define LTC26X6_POWER_DOWN_COMMAND      0x40            // Command to power down the LTC2606.

#define LTC26X6_WRITE_ADDRESS(x)        (x << 1)        // I2C addres shift, to make R/W bit LOW by default

// Return error values
#define LTC26X6_CODE_OVERFLOW           -2
#define LTC26X6_CODE_UNDERFLOW          -3

/*****************************************************************************/
/*************************** Types Declarations *******************************/
/******************************************************************************/
enum ltc26x6_write_command {       // SDP-K1 GPIO number assignment
	write_command = 0x00,          // Command to update (and power up) LTC26X6. Output voltage will be set to the value stored in the internal register by previous write command.
	write_update_command = 0x30    // Command to write and update (and power up) the LTC26X6. The output voltage will immediate change to the value being written to the internal register.
};

struct ltc26x6_dev {
	/* I2C */
	struct no_os_i2c_desc   *i2c_desc;
	/* Device Settings */
	uint8_t     resolution;     //LTC2606-16bits, LTC2616-14bit, LTC2626-12bits
	float       vref;           //DAC reference voltage
	float       typical_offset; //DAC typical offset
};

struct ltc26x6_init_param {
	/* I2C */
	struct no_os_i2c_init_param     i2c_init;
	/* Device Settings */
	uint8_t     resolution;     //LTC2606-16bits, LTC2616-14bit, LTC2626-12bits
	float       vref;           //DAC reference voltage
	float       typical_offset; //DAC typical offset
};

/******************************************************************************/
/************************ Functions Declarations ******************************/
/******************************************************************************/

int32_t ltc26x6_init(struct ltc26x6_dev **device,
		     struct ltc26x6_init_param init_param);
int16_t ltc26x6_voltage_to_code(struct ltc26x6_dev *device, float dac_voltage,
				uint16_t *code);
int32_t ltc26x6_write_code(struct ltc26x6_dev *dev,
			   enum ltc26x6_write_command write_command, uint16_t dac_code);
int32_t ltc26x6_power_down(struct ltc26x6_dev *dev);
int32_t ltc26x6_power_up(struct ltc26x6_dev *dev);

#endif // !_LTC26X6_H_


