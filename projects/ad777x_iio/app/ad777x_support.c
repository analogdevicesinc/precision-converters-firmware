/***************************************************************************//*
 * @file    ad777x_support.c
 * @brief   AD777x No-OS driver support file
******************************************************************************
 * Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
 *
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 * By using this software you agree to the terms of the associated
 * Analog Devices Software License Agreement.
******************************************************************************/

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include "ad777x_support.h"
#include "no_os_error.h"
#include "ad777x_iio.h"
#include "app_config.h"
#include "no_os_util.h"
#if (INTERFACE_MODE == TDM_MODE)
#include "stm32_tdm_support.h"
#endif

/******************************************************************************/
/********************* Macros and Constants Definitions ***********************/
/******************************************************************************/

/* Value of DOUT_FORMAT bits to enable single line DOUT for all 8 channels */
#define AD777x_DOUT_FORMAT_SELECT	0x2

/* Number of data bytes */
#define AD777x_DATA_BYTES	3

/* Conversion Delay for SAR ADC */
#define SAR_CONV_DELAY_USEC	1

/* Acquisition Delay for SAR ADC */
#define SAR_ACQ_DELAY_USEC	1

/******************************************************************************/
/******************** Variables and User Defined Data Types *******************/
/******************************************************************************/

/******************************************************************************/
/************************** Functions Definitions *****************************/
/******************************************************************************/

/*!
 * @brief Read the SD ADC Code
 * @param dev[in]- Pointer to IIO device instance
 * @param ch_num[in] - Channel Id (Number)
 * @param sd_adc_code[out]- ADC Raw data
 * @return 0 in case of success, negative error code otherwise
 */
int32_t ad777x_raw_data_read(ad7779_dev *dev, uint8_t ch_num,
			     uint32_t *sd_adc_code)
{
	uint8_t byte_index = ch_num * BYTES_PER_SAMPLE;

	uint8_t buff[32] = { 0 };
	uint8_t drdy_value = NO_OS_GPIO_HIGH;	// Value of the DRDY pin
	uint32_t timeout = AD777x_CONV_TIMEOUT;
	uint32_t read_buffer[AD777x_NUM_CHANNELS];
	int32_t ret;

	if (!dev || !sd_adc_code) {
		return -EINVAL;
	}

#if (INTERFACE_MODE == TDM_MODE)
	ret = no_os_tdm_read(ad777x_tdm_desc, buff, TDM_SLOTS_PER_FRAME);
	if (ret) {
		return ret;
	}

	/* Check for DMA buffer full */
	while (!dma_buffer_full) {
		timeout--;
	}
	if (timeout == 0) {
		return -ETIMEDOUT;
	}

	dma_buffer_full = false;

#else
	/* Configure SPI to read the SD ADC conversion data */
	ret = ad7779_set_spi_op_mode(dev, AD7779_SD_CONV);
	if (ret) {
		return ret;
	}

	/* Wait for DRDY to go low */
	do {
		ret = no_os_gpio_get_value(gpio_drdy_desc, &drdy_value);
		if (ret) {
			return ret;
		}
		timeout--;
	} while ((drdy_value != NO_OS_GPIO_LOW) && (timeout > 0));

	if (timeout == 0) {
		return -ETIMEDOUT;
	}

	/* Read the SPI data */
	ret = no_os_spi_write_and_read(dev->spi_desc,
				       buff,
				       sizeof(buff));
	if (ret) {
		return ret;
	}

	ret = ad7779_set_spi_op_mode(dev, AD7779_INT_REG);
	if (ret) {
		return ret;
	}
#endif
	/* Decode the Bytes into ADC Raw data */
	*sd_adc_code = no_os_get_unaligned_le24(&buff[byte_index]);

	return 0;
}


/*!
 * @brief Read the SD ADC Code of all channels
 * @param dev[in]- Pointer to IIO device instance
 * @param sd_adc_code[out]- ADC Raw data
 * @return 0 in case of success, negative error code otherwise
 */
int32_t ad777x_read_all_channels(ad7779_dev *dev, uint32_t *sd_adc_code)
{
	uint8_t buff[32] = { 0 };
	uint8_t byte_index;
	uint8_t ch_id;
	int32_t ret;

	if (!dev || !sd_adc_code) {
		return -EINVAL;
	}

	/* Read the SPI data */
	ret = no_os_spi_write_and_read(dev->spi_desc,
				       buff,
				       sizeof(buff));
	if (ret) {
		return ret;
	}

	/* Decode the bytes into ADC Raw data */
	for (ch_id = 0; ch_id < AD777x_NUM_CHANNELS; ch_id++) {
		byte_index = (AD777x_DATA_BYTES * ch_id) + (ch_id + 1);
		sd_adc_code[ch_id] = (buff[byte_index] << 16) | (buff[byte_index + 1] << 8) |
				     (buff[byte_index + 2]);
	}

	return 0;
}


/*!
 * @brief Enable single DOUT for all 8 channels
 * @param dev[in]- Pointer to IIO device instance
 * @return 0 in case of success, negative error code otherwise
 */
int32_t ad777x_enable_single_dout(ad7779_dev *dev)
{
	int32_t ret;

	/* Enable DOUT0 as data output line for all 8 channels */
	ret = ad7779_spi_int_reg_write_mask(dev, AD7779_REG_DOUT_FORMAT,
					    AD7779_DOUT_FORMAT(0x3), AD7779_DOUT_FORMAT(AD777x_DOUT_FORMAT_SELECT));
	if (ret) {
		return ret;
	}

	return 0;
}

/*!
 * @brief Read SAR ADC data
 * @param dev[in]- Pointer to IIO device instance
 * @param mux [in] - SAR mux configuration
 * @param sar_code [out] - SAR ADC code
 * @return 0 in case of success, negative error code otherwise
 */
int32_t ad7779_sar_data_read(ad7779_dev *dev,
			     ad7779_sar_mux mux,
			     uint16_t *sar_code)
{
	int32_t ret;

	/* Toggle the CONVST_SAR Pin */
	ret = no_os_gpio_set_value(dev->gpio_convst_sar, NO_OS_GPIO_LOW);
	if (ret) {
		return ret;
	}
	no_os_udelay(SAR_ACQ_DELAY_USEC);

	ret = no_os_gpio_set_value(dev->gpio_convst_sar, NO_OS_GPIO_HIGH);
	if (ret) {
		return ret;
	}
	no_os_udelay(SAR_CONV_DELAY_USEC);

	/* Read the SAR Code */
	ret = ad7779_spi_sar_read_code(dev, mux, sar_code);
	if (ret) {
		return ret;
	}

	return ret;
}
