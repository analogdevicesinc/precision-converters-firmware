/*******************************************************************************
 *   @file   cn0540_user_config.c
 *   @brief  User configuration module for CN0540
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

#include <stdbool.h>

#include "app_config.h"
#include "cn0540_user_config.h"

/******************************************************************************/
/********************** Variables and User Defined Data Types *****************/
/******************************************************************************/

// SPI bus init parameters
const struct no_os_spi_init_param spi_params = {
	.device_id = SPI_DEVICE_ID,
	.max_speed_hz = MAX_SPI_SCLK,
	.mode = NO_OS_SPI_MODE_3,
	.chip_select = SPI_CS_PIN,
	.bit_order = NO_OS_SPI_BIT_ORDER_MSB_FIRST,
	.platform_ops = &spi_ops,
	.extra = &spi_init_extra_params
};

// Initial parameters for the ADC AD7768-1
struct ad77681_init_param init_params = {
	.spi_eng_dev_init = spi_params, // SPI parameters
	.power_mode = AD77681_ECO, // power_mode
	.mclk_div = AD77681_MCLK_DIV_16, // mclk_div
	.conv_mode = AD77681_CONV_CONTINUOUS, // conv_mode
	.diag_mux_sel = AD77681_AIN_SHORT, // diag_mux_sel
	.conv_diag_sel = false, // conv_diag_sel
	.conv_len = AD77681_CONV_24BIT, // conv_len
	.crc_sel = AD77681_NO_CRC, // crc_sel
	.status_bit = 0, // status bit
	.VCM_out = AD77681_VCM_HALF_VCC, // VCM setup
	.AINn = AD77681_AINn_ENABLED, // AIN- precharge buffer
	.AINp = AD77681_AINp_ENABLED, // AIN+ precharge buffer
	.REFn = AD77681_BUFn_ENABLED, // REF- buffer
	.REFp = AD77681_BUFp_ENABLED, // REF+ buffer
	.filter = AD77681_FIR, // FIR Filter
	.decimate = AD77681_SINC5_FIR_DECx32, // Decimate by 32
	.sinc3_osr = 0, // OS ratio of SINC3
	.vref = ADC_REF_VOLTAGE, // Reference voltage
	.mclk = MCLK_KHZ,
	.sample_rate = DEFAULT_SAMPLE_RATE
};

// Initial parameters for the DAC LTC2606's I2C bus
const struct no_os_i2c_init_param i2c_params_dac = {
	.device_id = I2C_DEVICE_ID,
	.platform_ops = &i2c_ops,
	.max_speed_hz = 100000,
	.slave_address = LTC2606_I2C_ADDRESS,
};

// Initial parameters for the DAC LTC2606
struct ltc26x6_init_param init_params_dac = {
	.i2c_init = i2c_params_dac,
	.resolution = LTC2606_RES,
	.vref = LTC2606_REF_VOLTAGE,
	.typical_offset = LTC2606_TYP_OFFSET
};
