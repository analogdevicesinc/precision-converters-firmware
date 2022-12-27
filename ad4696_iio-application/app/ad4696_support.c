/*************************************************************************//**
 *   @file   ad4696_support.c
 *   @brief  AD469x device No-OS driver supports
******************************************************************************
* Copyright (c) 2021-22 Analog Devices, Inc.
*
* All rights reserved.
*
* This software is proprietary to Analog Devices, Inc. and its licensors.
* By using this software you agree to the terms of the associated
* Analog Devices Software License Agreement.
*****************************************************************************/

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include <stdint.h>

#include "app_config.h"
#include "ad4696_support.h"
#include "ad4696_user_config.h"
#include "no_os_error.h"
#include "no_os_gpio.h"
#include "no_os_spi.h"

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/
/* Timeout count to avoid stuck into potential infinite loop while checking
 * for data ready signal.
 * Note: This timeout factor is dependent upon the MCU clock frequency. Below timeout
 * is tested for SDP-K1 platform @180Mhz default core clock */
#define DATA_READY_TIMEOUT	0xffffffff

/******************************************************************************/
/********************** Variables and User Defined Data Types *****************/
/******************************************************************************/

/******************************************************************************/
/************************ Functions Definitions *******************************/
/******************************************************************************/
/*!
 * @brief	Select between polarity modes
 * @param	device[in] - device instance
 * @param   polarity_sel[in] - polarity selection.
 * @return	0 in case of success, negative error code otherwise.
 */
int32_t ad469x_polarity_mode_select(struct ad469x_dev *device,
				    enum ad469x_polarity_select polarity_sel)
{
	int32_t ret;
	uint8_t reg_data = 0;
	uint8_t chn_count;
	uint8_t write_data;

	if (polarity_sel == AD469x_PSEUDO_BIPOLAR_MODE) {
		write_data = (AD469x_REG_CONFIG_IN_PAIR(polarity_sel)
			      | AD469x_REG_CONFIG_IN_MODE(AD469x_INx_COM));
	} else {
		write_data = (AD469x_REG_CONFIG_IN_PAIR(polarity_sel)
			      | AD469x_REG_CONFIG_IN_MODE(AD469x_INx_REF_GND));
	}

	for (chn_count = 0; chn_count < AD469x_CHANNEL_NO; chn_count++) {
		ret = ad469x_spi_reg_read(device,
					  AD469x_REG_CONFIG_IN(chn_count),
					  &reg_data);
		if (ret < 0) {
			return ret;
		}

		reg_data &= ~AD469x_REG_CONFIG_IN_MODE_MASK;
		reg_data |= write_data;

		ret = ad469x_spi_reg_write(device,
					   AD469x_REG_CONFIG_IN(chn_count),
					   reg_data);
		if (ret < 0) {
			return ret;
		}
	}

	return 0;
}

/*!
 * @brief	Configures the reference voltage setting.
 * @param	device[in] - device instance
 * @return	0 in case of success, negative error code otherwise.
 */
int32_t ad469x_reference_config(struct ad469x_dev *device)
{
	int32_t ret;
	uint8_t write_data = AD469x_REG_REF_VREF_SET(false) |
			     AD469x_REG_REF_VREF_REFHIZ(true) |
			     AD469x_REG_REF_VREF_REFBUF(true);

	ret = ad469x_spi_reg_write(device,
				   AD469x_REG_REF_CTRL,
				   write_data);
	if (ret < 0) {
		return ret;
	}

	return 0;
}

/*!
 * @brief	Toggles conversion pin to trigger a new conversion.
 * @param	device[in] - device instance.
 * @return	0 in case of success, negative error code otherwise.
 */
int32_t ad469x_trigger_conversion(struct ad469x_dev *device)
{
	int32_t ret;

#if (ACTIVE_PLATFORM == MBED_PLATFORM)
	/* By default Mbed configures the I/O direction of a gpio
	 * (when used for PWM) in analog mode, after disabling or
	 * removing the PWM object.
	 * In this applications, the conversion trigger pin is being shared with
	 * ad4696 drivers as gpio output pin and is configured in output mode
	 * only when it is initialized.
	 * Hence we need to reinitialize the gpio so that ad4696 driver
	 * can gain access to pin configured in output mode.
	 * */
	ret = no_os_gpio_remove(device->gpio_convst);
	if (ret) {
		return ret;
	}

	ret = no_os_gpio_get(&device->gpio_convst, ad4696_init_str.gpio_convst);
	if (ret) {
		return ret;
	}

	ret = no_os_gpio_direction_output(device->gpio_convst, NO_OS_GPIO_HIGH);
	if (ret) {
		return ret;
	}
#endif

	ret = no_os_gpio_set_value(device->gpio_convst, NO_OS_GPIO_HIGH);
	if (ret) {
		return ret;
	}

	ret = no_os_gpio_set_value(device->gpio_convst, NO_OS_GPIO_LOW);
	if (ret) {
		return ret;
	}

	return 0;
}

/*!
 * @brief	Read single sample from the ADC.
 * @param	device[in] - device instance.
 * @param	chn_num[in] - channel number to be selected.
 * @param	data[out] - pointer to the adc data variable.
 * @return	0 in case of success, negative error code otherwise.
 */
int32_t ad469x_read_single_sample(struct ad469x_dev *device,
				  uint8_t chn_num,
				  uint32_t *data)
{
	int32_t ret;
	uint32_t timeout = DATA_READY_TIMEOUT;
	volatile uint8_t gpio_val = NO_OS_GPIO_HIGH;

	/* First array element represents command word for channel selection */
	uint8_t buf[3] = { AD469x_CMD_CONFIG_CH_SEL(chn_num), 0x00, 0x00 };

	/* Set the device into single cycle mode */
	ret = ad469x_set_channel_sequence(device, AD469x_single_cycle);
	if (ret) {
		return ret;
	}

	ret = ad469x_enter_conversion_mode(device);
	if (ret) {
		return ret;
	}

	ret = ad469x_trigger_conversion(device);
	if (ret) {
		return ret;
	}

	/* Monitoring the end of conversion for writing 
	 * channel number into the sequencer. */
	while ((gpio_val != NO_OS_GPIO_LOW)  && (timeout > 0)) {
		no_os_gpio_get_value(device->gpio_busy, &gpio_val);
		timeout--;
	}

	if (timeout == 0) {
		return -ETIME;
	}

	/* Write the selected channel into the sequencer */
	ret = no_os_spi_write_and_read(device->spi_desc, buf, sizeof(buf));
	if (ret) {
		return ret;
	}

	ret = ad469x_trigger_conversion(device);
	if (ret) {
		return ret;
	}

	/* Monitoring the end of conversion for reading
	 * conversion data sample */
	gpio_val = NO_OS_GPIO_HIGH;
	while ((gpio_val != NO_OS_GPIO_LOW)  && (timeout > 0)) {
		no_os_gpio_get_value(device->gpio_busy, &gpio_val);
		timeout--;
	}

	if (timeout == 0) {
		return -ETIME;
	}

	/* Exiting the conversion mode and read the sample
	 * corresponding to the selected channel  */
	buf[0] = AD469x_CMD_REG_CONFIG_MODE;
	ret = no_os_spi_write_and_read(device->spi_desc, buf, sizeof(buf));
	if (ret) {
		return ret;
	}

	*data = (uint32_t)(buf[0] << 8) | buf[1];

	ret = ad469x_set_channel_sequence(device, AD469x_standard_seq);
	if (ret) {
		return ret;
	}

	return 0;
}
