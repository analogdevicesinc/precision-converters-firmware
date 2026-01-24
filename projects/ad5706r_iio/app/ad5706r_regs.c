/***************************************************************************//**
 *   @file   ad5706r_regs.c
 *   @brief  Source file for the AD5706R registers map
********************************************************************************
 * Copyright 2024-2026(c) Analog Devices, Inc.
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

#include "ad5706r_regs.h"

/* AD5706R registers */
const uint32_t ad5706r_regs[AD5706R_NUM_REGS] = {
	AD5706R_REG_INTERFACE_CONFIG_A,
	AD5706R_REG_INTERFACE_CONFIG_B,
	AD5706R_REG_DEVICE_CONFIG,
	AD5706R_REG_CHIP_TYPE,
	AD5706R_REG_PRODUCT_ID_L,
	AD5706R_REG_PRODUCT_ID_H,
	AD5706R_REG_CHIP_GRADE,
	AD5706R_REG_DEVICE_INDEX,
	AD5706R_REG_SCRATCH_PAD,
	AD5706R_REG_SPI_REVISION,
	AD5706R_REG_VENDOR_L,
	AD5706R_REG_VENDOR_H,
	AD5706R_REG_STREAM_MODE,
	AD5706R_REG_TRANSFER_CONFIG,
	AD5706R_REG_INTERFACE_CONFIG_C,
	AD5706R_REG_INTERFACE_STATUS_A,
	AD5706R_REG_MULTI_DAC_CH_SEL,
	AD5706R_REG_LDAC_SYNC_ASYNC,
	AD5706R_REG_LDAC_HW_SW,
	AD5706R_REG_LDAC_EDGE_SEL_CH(0),
	AD5706R_REG_LDAC_EDGE_SEL_CH(1),
	AD5706R_REG_LDAC_EDGE_SEL_CH(2),
	AD5706R_REG_LDAC_EDGE_SEL_CH(3),
	AD5706R_REG_OUT_OPERATING_MODE,
	AD5706R_REG_OUT_SWITCH_EN,
	AD5706R_REG_HW_SHUTDOWN_EN,
	AD5706R_REG_OUT_RANGE_CH(0),
	AD5706R_REG_OUT_RANGE_CH(1),
	AD5706R_REG_OUT_RANGE_CH(2),
	AD5706R_REG_OUT_RANGE_CH(3),
	AD5706R_REG_FUNC_EN,
	AD5706R_REG_FUNC_MODE_SEL_CH(0),
	AD5706R_REG_FUNC_MODE_SEL_CH(1),
	AD5706R_REG_FUNC_MODE_SEL_CH(2),
	AD5706R_REG_FUNC_MODE_SEL_CH(3),
	AD5706R_REG_FUNC_DAC_INPUT_B_CH(0),
	AD5706R_REG_FUNC_DAC_INPUT_B_CH(1),
	AD5706R_REG_FUNC_DAC_INPUT_B_CH(2),
	AD5706R_REG_FUNC_DAC_INPUT_B_CH(3),
	AD5706R_REG_FUNC_DITHER_PERIOD_CH(0),
	AD5706R_REG_FUNC_DITHER_PERIOD_CH(1),
	AD5706R_REG_FUNC_DITHER_PERIOD_CH(2),
	AD5706R_REG_FUNC_DITHER_PERIOD_CH(3),
	AD5706R_REG_FUNC_DITHER_PHASE_CH(0),
	AD5706R_REG_FUNC_DITHER_PHASE_CH(1),
	AD5706R_REG_FUNC_DITHER_PHASE_CH(2),
	AD5706R_REG_FUNC_DITHER_PHASE_CH(3),
	AD5706R_REG_MUX_OUT_SEL,
	AD5706R_REG_MUX_OUT_CONTROL,
	AD5706R_REG_TEMP_WARN_INT_EN,
	AD5706R_REG_MULTI_DAC_SW_LDAC,
	AD5706R_REG_MULTI_DAC_INPUT_A,
	AD5706R_REG_DAC_SW_LDAC,
	AD5706R_REG_DAC_INPUT_A_CH(0),
	AD5706R_REG_DAC_INPUT_A_CH(1),
	AD5706R_REG_DAC_INPUT_A_CH(2),
	AD5706R_REG_DAC_INPUT_A_CH(3),
	AD5706R_REG_DAC_DATA_READBACK_CH(0),
	AD5706R_REG_DAC_DATA_READBACK_CH(1),
	AD5706R_REG_DAC_DATA_READBACK_CH(2),
	AD5706R_REG_DAC_DATA_READBACK_CH(3),
	AD5706R_REG_TEMP_WARN_STAT,
	AD5706R_REG_DIGITAL_STATUS,
	AD5706R_REG_BANDGAP_CONTROL,
	AD5706R_REG_USER_SPARE_0,
	AD5706R_REG_USER_SPARE_1,
	AD5706R_REG_USER_SPARE_2,
	AD5706R_REG_USER_SPARE_3
};
