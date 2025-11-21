/***************************************************************************
 *   @file    dpot_info_table.c
 *   @brief   Digipots information table which gives the details about the parts.
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

#include "dpot_support.h"
#include "dpot_user_config.h"
#include "dpot_iio.h"
#include "dpot.h"

/******************************************************************************/
/********************* Macros and Constants Definition ************************/
/******************************************************************************/

/******************************************************************************/
/******************** Variables and User Defined Data Types *******************/
/******************************************************************************/

/******************************************************************************/
/************************ Functions Definitions *******************************/
/******************************************************************************/

/*!
 * @brief	Set RDAC write protect of digital potentiometer.
   @param dpot_rdac_wp_indx - Flag to indicate whetehr to enable/Disable  RDAC Write Protection.
 * @return	0 in case of success, negative error code otherwise.
 */
int dpot_set_rdac_wp(uint8_t dpot_rdac_wp_indx)
{
	struct dpot_command cmd;
	uint8_t control_mode;
	int ret;

	/* Readback control register */
	cmd.control = 0x3;
	cmd.address = 0x0;
	cmd.data = 0x2;
	cmd.is_readback = true;

	ret = dpot_send_cmd(dpot_dev_desc, &cmd);
	if (ret) {
		return ret;
	}

	control_mode = cmd.response;

	/* Set RDAC WP status */
	cmd.control = 0xD;
	cmd.address = 0x0;
	if (dpot_rdac_wp_indx == 0) {
		/* Disable RDAC WP */
		cmd.data = control_mode | (1 << 0);
	} else {
		/* Enable RDAC WP */
		cmd.data = control_mode & ~(1 << 0);
	}
	cmd.is_readback = false;

	ret = dpot_send_cmd(dpot_dev_desc, &cmd);
	if (ret) {
		return ret;
	}

	return 0;
}

/*!
 * @brief	Set NVM programming of digital potentiometer.
   @param dpot_nvm_programming_indx - Flag to indicate whetehr to enable/Disable  NVM programming.
 * @return	0 in case of success, negative error code otherwise.
 */
int dpot_set_nvm_programming(uint8_t dpot_nvm_programming_indx)
{
	struct dpot_command cmd;
	uint8_t control_mode;
	int ret;

	/* Readback control register */
	cmd.control = 0x3;
	cmd.address = 0x0;
	cmd.data = 0x2;
	cmd.is_readback = true;

	ret = dpot_send_cmd(dpot_dev_desc, &cmd);
	if (ret) {
		return ret;
	}

	control_mode = cmd.response;

	/* Set NVM programming status */
	cmd.control = 0xD;
	cmd.address = 0x0;
	if (dpot_nvm_programming_indx == 0) {
		/* Enable NVM programming */
		cmd.data = control_mode | (1 << 1);
	} else {
		/* Disable NVM programming */
		cmd.data = control_mode & ~(1 << 1);
	}
	cmd.is_readback = false;

	ret = dpot_send_cmd(dpot_dev_desc, &cmd);
	if (ret) {
		return ret;
	}

	return 0;
}
