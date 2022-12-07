/*************************************************************************//**
 *   @file   ad4130_support.c
 *   @brief  AD4130 device No-OS driver supports
******************************************************************************
* Copyright (c) 2020, 2022 Analog Devices, Inc.
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
#include "ad4130_support.h"
#include "ad4130_user_config.h"
#include "no_os_error.h"

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

/* AD4130 FIFO size and readback command size in bytes */
#define AD4130_FIFO_MAX_SIZE		(256)
#define AD4130_FIFO_READ_CMD_BYTES	(2)

#define BYTE_SIZE					(8)

/* Timeout to monitor CON monitor GPIO. The timeout count is dependent upon the
 * MCU clock frequency. This timeout is tested for SDP-K1 Mbed controller platform */
#define CONV_MON_GPIO_TIMEOUT	(10000)

/* Select between GPIO Or STATUS register to monitor the end
 * of conversion in single conversion mode */
//#define CONV_MON_USING_RDY_STATUS		// Uncomment to use STATUS reg

/* FIFO busy time as per specifications (in usec)
 * Note : This time is stringent in FIFO readback.The minimum time period
 * as per specifications is 20usec
 */
#define	FIFO_BUSY_TIME		(20)

/******************************************************************************/
/********************** Variables and User Defined Data Types *****************/
/******************************************************************************/

/* AD4130 FIFO readback buffer.
 * Size for 24-bit ADC = (256 * 3) + 2 = 770 bytes
 * Size for 16-bit ADC = (256 * 2) + 2 = 514 bytes
 * */
static uint8_t fifo_buf[(AD4130_FIFO_MAX_SIZE * (ADC_RESOLUTION / BYTE_SIZE)) +
									      AD4130_FIFO_READ_CMD_BYTES];

/******************************************************************************/
/************************ Functions Definitions *******************************/
/******************************************************************************/

/*!
 * @brief	Get reference voltage based on the reference source
 * @param	dev[in] - Device instance
 * @param	chn[in] - ADC channel
 * @return	Reference voltage
 */
float ad4130_get_reference_voltage(struct ad413x_dev *dev, uint8_t chn)
{
	float ref_voltage;
	uint8_t preset = dev->ch[chn].preset;
	enum ad413x_ref_sel ref = dev->preset[preset].ref_sel;
	enum ad413x_int_ref int_ref = dev->int_ref;

	switch (ref) {
	case AD413X_REFIN1:
		ref_voltage = AD4130_REFIN1_VOLTAGE;
		break;

	case AD413X_REFIN2:
		ref_voltage = AD4130_REFIN2_VOLTAGE;
		break;

	case AD413X_AVDD_AVSS:
		ref_voltage = AD4130_AVDD_VOLTAGE;
		break;

	case AD413X_REFOUT_AVSS:
		if (int_ref == AD413X_INTREF_1_25V) {
			ref_voltage = AD4170_1_25V_INT_REF_VOLTAGE;
		} else {
			ref_voltage = AD4170_2_5V_INT_REF_VOLTAGE;
		}
		break;

	default:
		ref_voltage = AD4170_2_5V_INT_REF_VOLTAGE;
		break;
	}

	return ref_voltage;
}

/*!
 * @brief	Perform the sign conversion for handling negative voltages in
 *			bipolar mode
 * @param	dev[in] - Device instance
 * @param	adc_raw_data[in] - ADC raw value
 * @param	chn[in] - ADC Channel
 * @return	ADC data after signed conversion
 */
int32_t perform_sign_conversion(struct ad413x_dev *dev, uint32_t adc_raw_data,
				uint8_t chn)
{
	int32_t adc_data;
	bool bipolar = dev->bipolar;

	/* Bipolar ADC Range:  (-FS) <-> 0 <-> (+FS) : 0 <-> 2^(ADC_RES-1)-1 <-> 2^(ADC_RES-1)
	   Unipolar ADC Range: 0 <-> (+FS) : 0 <-> 2^ADC_RES
	 **/
	if (bipolar) {
		/* Data output format is offset binary for bipolar mode */
		adc_data = adc_raw_data - ADC_MAX_COUNT_BIPOLAR;
	} else {
		/* Data output format is straight binary for unipolar mode */
		adc_data = adc_raw_data;
	}

	return adc_data;
}

/*!
 * @brief	Convert the ADC raw value into equivalent voltage
 * @param	dev[in] - Device instance
 * @param	adc_raw[in]- ADC raw data
 * @param	chn[in] - ADC channel
 * @return	ADC voltage value
 */
float convert_adc_sample_into_voltage(void *dev, uint32_t adc_raw,
				      uint8_t chn)
{
	enum ad413x_gain pga;
	float vref;
	int32_t adc_data;
	uint8_t preset = ((struct ad413x_dev *)dev)->ch[chn].preset;
	bool bipolar = ((struct ad413x_dev *)dev)->bipolar;

	pga = ((struct ad413x_dev *)dev)->preset[preset].gain;
	vref = ad4130_get_reference_voltage(dev, chn);
	adc_data = perform_sign_conversion(dev, adc_raw, chn);

	if (bipolar) {
		return (adc_data * (vref / (ADC_MAX_COUNT_BIPOLAR * (1 << pga))));
	} else {
		return (adc_data * (vref / (ADC_MAX_COUNT_UNIPOLAR * (1 << pga))));
	}
}

/*!
 * @brief	Convert the ADC raw value into equivalent RTD resistance
 * @param	dev[in] - Device instance
 * @param	adc_raw[in] - ADC raw sample
 * @param	rtd_ref[in] - RTD reference resistance in ohms
 * @param	chn[in] - ADC channel
 * @return	RTD resistance value
 * @note	RTD is biased with constant excitation current. Below formula
 *			is based on ratiometric measurement, where fixed value of RTD RREF
 *			(reference resistor) and gain is taken into account
 */
float convert_adc_raw_into_rtd_resistance(void *dev, uint32_t adc_raw,
		float rtd_ref, uint8_t chn)
{
	enum ad413x_gain pga;
	int32_t adc_data;
	uint8_t preset = ((struct ad413x_dev *)dev)->ch[chn].preset;
	bool bipolar = ((struct ad413x_dev *)dev)->bipolar;

	pga = ((struct ad413x_dev *)dev)->preset[preset].gain;
	adc_data = perform_sign_conversion(dev, adc_raw, chn);

	if (bipolar) {
		return (((float)adc_data * rtd_ref) / (ADC_MAX_COUNT_BIPOLAR * (1 << pga)));
	} else {
		return (((float)adc_data * rtd_ref) / (ADC_MAX_COUNT_UNIPOLAR * (1 << pga)));
	}
}

/*!
 * @brief	Function to read the single ADC sample (raw data) for input channel
 * @param	dev[in] - Device instance
 * @param	input_chn[in] - Input channel to be sampled and read data for
 * @param	adc_raw[in, out]- ADC raw data
 * @return	0 in case of success, negative error code otherwise
 * @note	The single conversion mode is used to read a single sample
 */
int32_t ad413x_read_single_sample(struct ad413x_dev *dev, uint8_t input_chn,
				  uint32_t *adc_raw)
{
	uint32_t chn_mask = 0;
	uint8_t chn;
	int32_t ret;

	if (!adc_raw) {
		return -EINVAL;
	}

	/* Disable all active channels */
	for (chn = 0; chn < ADC_USER_CHANNELS; chn++) {
		if (dev->ch[chn].enable) {
			chn_mask |= (1 << chn);

			/* Disable the current channel */
			ret = ad413x_ch_en(dev, chn, 0);
			if (ret) {
				return ret;
			}
		}
	}

	/* Enable user input channel */
	if (!dev->ch[input_chn].enable) {
		ret = ad413x_ch_en(dev, input_chn, 1);
		if (ret) {
			return ret;
		}
	}

	/* Put device into single conversion mode */
	ret = ad413x_set_adc_mode(dev, AD413X_SINGLE_CONV_MODE);
	if (ret) {
		return ret;
	}

	/* Monitor conversion and read the result */
	ret = ad413x_mon_conv_and_read_data(dev, adc_raw);
	if (ret) {
		return ret;
	}

	/* Disable user input channel */
	ret = ad413x_ch_en(dev, input_chn, 0);
	if (ret) {
		return ret;
	}

	return 0;
}

/*!
 * @brief	Function to monitor end of conversion and read
 *			conversion result
 * @param	dev[in] - Device instance
 * @param	raw_data[in, out]- ADC raw data
 * @return	0 in case of success, negative error code otherwise
 */
int32_t ad413x_mon_conv_and_read_data(struct ad413x_dev *dev,
				      uint32_t *raw_data)
{
	int32_t ret;
	uint8_t conv_mon = 0;
	uint32_t timeout = CONV_MON_GPIO_TIMEOUT;

	if (!dev || !raw_data) {
		return -EINVAL;
	}

	/* Wait for conversion */
#if defined(CONV_MON_USING_RDY_STATUS)
	while (!conv_mon && timeout--) {
		/* Read the value of the Status Register */
		ret = ad413x_reg_read(dev, AD413X_REG_STATUS, raw_data);
		if (ret) {
			return ret;
		}

		/* Check the RDY bit in the Status Register */
		conv_mon = (*raw_data & AD413X_ADC_DATA_STATUS);
	}

	if (!timeout) {
		return -EIO;
	}

	/* Read the conversion result */
	ret = ad413x_reg_read(dev, AD413X_REG_DATA, raw_data);
	if (ret) {
		return ret;
	}
#else
	conv_mon = NO_OS_GPIO_HIGH;
	while (conv_mon == NO_OS_GPIO_HIGH && timeout--) {
		ret = no_os_gpio_get_value(trigger_gpio_desc, &conv_mon);
		if (ret) {
			return ret;
		}
	}

	if (!timeout) {
		return -EIO;
	}

	/* Read the conversion result */
	ret = ad413x_reg_read(dev, AD413X_REG_DATA, raw_data);
	if (ret) {
		return ret;
	}

	conv_mon = NO_OS_GPIO_LOW;
	timeout = CONV_MON_GPIO_TIMEOUT;
	while (conv_mon == NO_OS_GPIO_LOW && timeout--) {
		ret = no_os_gpio_get_value(trigger_gpio_desc, &conv_mon);
		if (ret) {
			return ret;
		}
	}

	if (!timeout) {
		return -EIO;
	}
#endif

	return 0;
}

/*!
 * @brief	Read the data from FIFO
 * @param	dev[in] - device instance
 * @param	data[in] - Buffer to store FIFO data
 * @param	adc_samples[in] - Number of ADC samples to read
 * @return	0 in case of success, negative error code otherwise
 * @note	This function doesn't consider the FIFO status and header information
 *			during data readback. It is assumed data user is intending to read
 *			only the data from FIFO.
 */
int32_t ad4130_read_fifo(struct ad413x_dev *dev, uint32_t *data,
			 uint32_t adc_samples)
{
	int32_t ret;
	uint32_t loop_cntr;
	uint32_t buf_indx = 0;
	uint32_t bytes;

	if (!dev || !data) {
		return -EINVAL;
	}

	/* Watermark count of 0 implies full FIFO readback */
	if ((adc_samples == 0) || (adc_samples > AD4130_FIFO_MAX_SIZE)) {
		adc_samples = AD4130_FIFO_MAX_SIZE;
	}

	/* Delay b/w interrupt trigger and FIFO readback start */
	no_os_udelay(FIFO_BUSY_TIME);

	/* MOSI pin outputs 0x00 during FIFO data readback */
	memset(fifo_buf, 0, sizeof(fifo_buf));

	/* Enter into FIFO read mode by issuing dummy read command. Command consists of first byte as
	 * address of FIFO data register and 2nd byte as number of samples to read from FIFO */
	fifo_buf[0] = AD413X_COMM_REG_RD | AD413X_ADDR(AD413X_REG_FIFO_DATA);
	fifo_buf[1] = adc_samples;

	/* Bytes to read = (samples * data size) + fifo data reg address + sample_cnt */
	bytes = (adc_samples * (ADC_RESOLUTION / BYTE_SIZE)) +
		AD4130_FIFO_READ_CMD_BYTES;

	/* Read all bytes over SPI */
	ret = no_os_spi_write_and_read(dev->spi_dev, fifo_buf, bytes);
	if (ret) {
		return ret;
	}

	/* Extract the data from buffer (data doesn't contain header/status info) */
	for (loop_cntr = AD4130_FIFO_READ_CMD_BYTES; loop_cntr < bytes;
	     loop_cntr += (ADC_RESOLUTION / BYTE_SIZE)) {
#if (ADC_RESOLUTION == 24)
		data[buf_indx++] = ((int32_t)fifo_buf[loop_cntr] << 16) |
				   ((int32_t)fifo_buf[loop_cntr + 1] << 8) |
				   (int32_t)fifo_buf[loop_cntr + 2];
#else
		/* For 16-bit resolution */
		data[buf_indx++] = ((int32_t)fifo_buf[loop_cntr] << 8) |
				   (int32_t)fifo_buf[loop_cntr + 1];
#endif
	}

	return 0;
}

/*!
 * @brief	Set interrupt conversion source (GPIO)
 * @param	dev[in] - Device instance
 * @param	conv_int_source[in]- Interrupt source
 * @return	0 in case of success, negative error code otherwise
 */
int32_t ad413x_set_int_source(struct ad413x_dev *dev,
			      adc_conv_int_source_e conv_int_source)
{
	int32_t ret;

	if (!dev) {
		return -EINVAL;
	}

	ret = ad413x_reg_write_msk(dev,
				   AD413X_REG_IO_CTRL,
				   AD413X_INT_PIN_SEL(conv_int_source),
				   AD4130_INT_SRC_SEL_MSK);
	if (ret) {
		return ret;
	}

	return 0;
}

/*!
 * @brief	Set filter FS value
 * @param	dev[in] - Device instance
 * @param	fs[in]- FS value
 * @param	preset[in] - Channel setup
 * @return	0 in case of success, negative error code otherwise
 */
int32_t ad413x_set_filter_fs(struct ad413x_dev *dev, uint32_t fs,
			     uint8_t preset)
{
	int32_t ret;

	if (!dev) {
		return -EINVAL;
	}

	ret = ad413x_reg_write_msk(dev,
				   AD413X_REG_FILTER(preset),
				   AD413X_FS_N(fs),
				   AD4130_FILTER_FS_MSK);
	if (ret) {
		return ret;
	}

	return 0;
}
