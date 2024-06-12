/***************************************************************************//**
 *   @file    ad355xr_support.c
 *   @brief   AD3552r No-OS driver support file
********************************************************************************
 * Copyright (c) 2023-2024 Analog Devices, Inc.
 * All rights reserved.
 *
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 * By using this software you agree to the terms of the associated
 * Analog Devices Software License Agreement.
*******************************************************************************/

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include "app_config.h"
#include "ad355xr_support.h"
#include "no_os_util.h"

/******************************************************************************/
/********************* Macros and Constants Definitions ***********************/
/******************************************************************************/

/******************************************************************************/
/******************** Variables and User Defined Data Types *******************/
/******************************************************************************/

/******************************************************************************/
/************************** Functions Definitions *****************************/
/******************************************************************************/

/*!
 * @brief	Writes one sample to all the dac channels
 * @param	desc[in]- Pointer to ad3552r device descriptor
 * @param	data[in]- data to be written
 * @return	0 in case of success, negative error code otherwise
 */
int32_t ad355xr_write_one_sample_all_ch(struct ad3552r_desc *desc,
					uint16_t *data)
{
	uint8_t buffer[7] = { 0 };
	uint8_t len = 0;
	uint8_t addr;

	if (!desc || !data) {
		return -EINVAL;
	}

	addr = ad3552r_get_code_reg_addr(1, 0, desc->ch_data[0].fast_en);

	buffer[0] = addr;
	len++;

	memcpy(buffer + len, &data[0], 2);
	len += 2;
	if (!desc->ch_data[0].fast_en) {
		len++;
	}

	memcpy(buffer + len, &data[1], 2);
	len += 2;
	if (!desc->ch_data[0].fast_en) {
		len++;
	}

	return no_os_spi_write_and_read(desc->spi, buffer, len);
}

/*!
 * @brief	Writes one sample to a dac channel
 * @param	desc[in]- Pointer to ad3552r device descriptor
 * @param	data[in]- data to be written
 * @param	ch_num[in]-  channel number
 * @return	0 in case of success, negative error code otherwise
 */
int32_t ad355xr_write_one_sample_one_ch(struct ad3552r_desc *desc,
					uint16_t *data, uint8_t ch_num)
{
	uint8_t buffer[4] = { 0 };
	uint8_t len = 0;
	uint8_t addr;

	if (!desc || !data) {
		return -EINVAL;
	}

	addr = ad3552r_get_code_reg_addr(ch_num, 0, desc->ch_data[0].fast_en);

	buffer[0] = addr;
	len++;

	memcpy(buffer + len, &data[0], 2);

	len += 2;
	if (!desc->ch_data[0].fast_en) {
		len++;
	}

	return no_os_spi_write_and_read(desc->spi, buffer, len);
}
