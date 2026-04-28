/***************************************************************************//**
 *   @file   ad552xr_regs.c
 *   @brief  Source file for the ad552xr registers map
********************************************************************************
 * Copyright 2026(c) Analog Devices, Inc.
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

#include "ad552xr_regs.h"

/* AD552XR registers */
const uint32_t ad552xr_regs[AD552XR_NUM_REGS] = {
	/* AD552XR Core Registers */
	AD552XR_REG_INTERFACE_CONFIG_A,
	AD552XR_REG_INTERFACE_CONFIG_B,
	AD552XR_REG_DEVICE_CONFIG,
	AD552XR_REG_CHIP_TYPE,
	AD552XR_REG_PRODUCT_ID_L,
	AD552XR_REG_PRODUCT_ID_H,
	AD552XR_REG_CHIP_GRADE,
	AD552XR_REG_SCRATCH_PAD,
	AD552XR_REG_SPI_REVISION,
	AD552XR_REG_VENDOR_L,
	AD552XR_REG_VENDOR_H,
	AD552XR_REG_STREAM_MODE,
	AD552XR_REG_TRANSFER_CONFIG,
	AD552XR_REG_INTERFACE_CONFIG_C,
	AD552XR_REG_INTERFACE_STATUS_A,

	/* AD552XR DAC Configuration Registers */
	AD552XR_REG_MULTI_INPUT_SEL,
	AD552XR_REG_LDAC_SYNC_ASYNC,
	AD552XR_REG_LDAC_HW_SW,
	AD552XR_RChn | AD552XR_REG_LDAC_HW_SRC_CH(0),
	AD552XR_REG_OUT_EN,
	AD552XR_RChn | AD552XR_REG_OUT_RANGE_CH(0),
	AD552XR_RChn | AD552XR_REG_CAL_GAIN_CH(0),
	AD552XR_RChn | AD552XR_REG_CAL_OFFSET_CH(0),
	AD552XR_REG_FUNC_EN,
	AD552XR_RChn | AD552XR_REG_FUNC_MODE_SEL_CH(0),
	AD552XR_RChn | AD552XR_REG_FUNC_DAC_INPUT_B_CH(0),
	AD552XR_RChn | AD552XR_REG_FUNC_DITHER_PERIOD_CH(0),
	AD552XR_RChn | AD552XR_REG_FUNC_DITHER_PHASE_CH(0),
	AD552XR_RChn | AD552XR_REG_FUNC_RAMP_STEP_CH(0),
	AD552XR_RChn | AD552XR_REG_FUNC_INT_EN(0),
	AD552XR_REG_MUX_OUT_SEL,
	AD552XR_REG_MULTI_SW_LDAC,
	AD552XR_REG_MULTI_INPUT,
	AD552XR_REG_SW_LDAC,
	AD552XR_RChn | AD552XR_REG_DAC_INPUT_A_CH(0),
	AD552XR_REG_FUNC_INT_STAT,
	AD552XR_RChn | AD552XR_REG_DAC_DATA_READBACK_CH(0),

	/* AD552XR DAC Product Specific Registers */
	AD552XR_REG_TSENS_EN,
	AD552XR_REG_TSENS_ALERT_FLAG,
	AD552XR_REG_TSENS_SHTD_FLAG,
	AD552XR_REG_TSENS_ALERT_STAT,
	AD552XR_REG_TSENS_SHTD_STAT,
	AD552XR_REG_ALARMB_TSENS_EN,
	AD552XR_REG_ALARMB_TSENS_SEL,
	AD552XR_REG_TSENS_SHTD_EN_CH,
	AD552XR_REG_DAC_DIS_DEGLITCH_CH,
	AD552XR_REG_DAC_INT_EN,
	AD552XR_REG_ALL_FUNC_INT_STAT,
	AD552XR_REG_FUNC_BUSY,
	AD552XR_REG_REF_SEL,
	AD552XR_REG_INIT_CRC_ERR_STAT,
};

