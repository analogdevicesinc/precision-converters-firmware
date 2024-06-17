/***************************************************************************//*
 * @file    ad4170_support.c
 * @brief   AD4170 No-OS driver support file
******************************************************************************
 * Copyright (c) 2021-24 Analog Devices, Inc. All Rights Reserved.
 *
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 * By using this software you agree to the terms of the associated
 * Analog Devices Software License Agreement.
******************************************************************************/

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include "ad4170_support.h"
#include "app_config.h"
#include "ad4170_iio.h"
#include "no_os_error.h"

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
 * @brief	Read the single ADC sample (raw data) for input channel
 * @param	input_chn[in] - Input channel to be sampled and read data for
 * @param	raw_data[in, out]- ADC raw data
 * @return	0 in case of success, negative error code otherwise
 */
int32_t ad4170_read_single_sample(uint8_t input_chn, uint32_t *raw_data)
{
	int32_t ret;
	struct ad4170_adc_ctrl adc_ctrl;
	uint32_t prev_active_channels;
	uint8_t adc_data[BYTES_PER_SAMPLE];

	if (!raw_data) {
		return -EINVAL;
	}

	/* Save the previous active channels */
	prev_active_channels = p_ad4170_dev_inst->config.channel_en;

	/* Disable all active channels */
	ret = ad4170_set_channel_en(p_ad4170_dev_inst, 0);
	if (ret) {
		return ret;
	}

	/* Enable single user input channel */
	ret = ad4170_enable_input_chn(input_chn);
	if (ret) {
		return ret;
	}

	/* Apply excitation on the input (demo config specific) */
	ret = ad4170_apply_excitation(input_chn);
	if (ret) {
		return ret;
	}

#if (INTERFACE_MODE == SPI_INTERRUPT_MODE) || (INTERFACE_MODE == SPI_DMA_MODE)
	/* Enable single conversion mode */
	adc_ctrl = p_ad4170_dev_inst->config.adc_ctrl;
	adc_ctrl.mode = AD4170_MODE_SINGLE;
	ret = ad4170_set_adc_ctrl(p_ad4170_dev_inst, adc_ctrl);
	if (ret) {
		return ret;
	}

	/* This function monitors RDY line to read ADC result */
	ret = ad4170_read24(p_ad4170_dev_inst, raw_data, 1);
	if (ret) {
		return ret;
	}
#elif (INTERFACE_MODE == TDM_MODE)
	/* Set to continuous transmit mode as TDM works only in this mode */
	adc_ctrl.mode =
		AD4170_CONT_CONV_MODE_CONFIG; /* ADC in continuous conversion mode */
	adc_ctrl.cont_read =
		AD4170_CONT_TRANSMIT_ON; /* Turn ON Continuous transmit Mode for TDM */
	adc_ctrl.cont_read_status_en = false;
	ret = ad4170_set_adc_ctrl(p_ad4170_dev_inst, adc_ctrl);
	if (ret) {
		return ret;
	}

	/* Pull the SPI CS line low to enable the data on SDO.
	 * This also ensures that the DIG_AUX1 and DIG_AUX2 signals are sent out
	 * via the pins on the Zio connector during continuous transmit mode */
	ret = no_os_gpio_set_value(csb_gpio_desc, NO_OS_GPIO_LOW);
	if (ret) {
		return ret;
	}

	ret = no_os_tdm_read(ad4170_tdm_desc, adc_data, 1);
	if (ret) {
		return ret;
	}

	/* Pull the SPI CS line high to stop streaming data on SDO */
	ret = no_os_gpio_set_value(csb_gpio_desc, NO_OS_GPIO_HIGH);
	if (ret) {
		return ret;
	}

	/* Exit continuous transmit mode */
	ret = ad4170_continuous_transmit_exit(p_ad4170_dev_inst);
	if (ret) {
		return ret;
	}

	*raw_data = no_os_get_unaligned_le32(&adc_data[0]);
#endif

	/* Remove excitation on the input (demo config specific) */
	ret = ad4170_remove_excitation(input_chn);
	if (ret) {
		return ret;
	}

	/* Restore (re-enable) the previous active channels */
	ret = ad4170_set_channel_en(p_ad4170_dev_inst, prev_active_channels);
	if (ret) {
		return ret;
	}

	return 0;
}

/*!
 * @brief	Read ADC raw data for recently sampled channel
 * @param	adc_data[in, out] - Pointer to adc data read variable
 * @return	0 in case of success, negative error code otherwise
 */
int32_t ad4170_read_converted_sample(uint32_t *adc_data)
{
	uint8_t buf[AD4170_TRANSF_LEN(AD4170_REG_DATA_24b)];

	if (!adc_data) {
		return -EINVAL;
	}

	/* Read data over spi interface (in continuous read mode) */
	memset(buf, 0, sizeof(buf));
	if (no_os_spi_write_and_read(p_ad4170_dev_inst->spi_desc,
				     buf,
				     sizeof(buf)) != 0) {
		return -EIO;
	}

	/* Extract data:       MSB                       LSB */
	*adc_data = (buf[0] << 16) | (buf[1] << 8) | buf[2];

	return 0;
}

/*!
 * @brief	Perform the sign conversion for handling negative voltages in
 *			bipolar mode
 * @param	adc_raw_data[in] - ADC raw value
 * @param	chn[in] - ADC Channel
 * @return	ADC data after signed conversion
 */
int32_t perform_sign_conversion(uint32_t adc_raw_data, uint8_t chn)
{
	int32_t adc_data;
	uint8_t setup = p_ad4170_dev_inst->config.setup[chn].setup_n;
	bool bipolar = p_ad4170_dev_inst->config.setups[setup].afe.bipolar;

	/* Bipolar ADC Range:  (-FS) <-> 0 <-> (+FS) : 2^(ADC_RES-1) <-> 0 <-> 2^(ADC_RES-1)-1
	   Unipolar ADC Range: 0 <-> (+FS) : 0 <-> 2^ADC_RES
	 **/
	if (bipolar) {
		/* Data output format is 2's complement for bipolar mode */
		if (adc_raw_data >= ADC_MAX_COUNT_BIPOLAR) {
			/* Remove the offset from result to convert into negative reading */
			adc_data = ADC_MAX_COUNT_UNIPOLAR - adc_raw_data;
			adc_data = -adc_data;
		} else {
			adc_data = adc_raw_data;
		}
	} else {
		/* Data output format is straight binary for unipolar mode */
		adc_data = adc_raw_data;
	}

	return adc_data;
}

/*!
 * @brief	Get the actual ADC gain decimal value
 * @param	chn[in] - ADC channel
 * @return	ADC programmable gain value
 */
float ad4170_get_gain_value(uint8_t chn)
{
	float gain;
	uint8_t setup = p_ad4170_dev_inst->config.setup[chn].setup_n;
	enum ad4170_pga_gain pga = p_ad4170_dev_inst->config.setups[setup].afe.pga_gain;

	if (pga == AD4170_PGA_GAIN_1_PRECHARGE) {
		gain = 1.0;
	} else if (pga == AD4170_PGA_GAIN_0P5) {
		gain = 0.5;
	} else {
		gain = AD4170_PGA_GAIN(pga);
	}

	return gain;
}

/*!
 * @brief	Get the reference voltage based on the reference source
 * @param	chn[in] - ADC channel
 * @return	Reference voltage
 */
float ad4170_get_reference_voltage(uint8_t chn)
{
	float ref_voltage;
	uint8_t setup = p_ad4170_dev_inst->config.setup[chn].setup_n;
	enum ad4170_ref_select ref =
		p_ad4170_dev_inst->config.setups[setup].afe.ref_select;

	if (ref == AD4170_REFIN_REFIN1) {
		ref_voltage = AD4170_REFIN_REFIN1_VOLTAGE;
	} else if (ref == AD4170_REFIN_REFIN2) {
		ref_voltage = AD4170_REFIN_REFIN2_VOLTAGE;
	} else if (ref == AD4170_REFIN_AVDD) {
		ref_voltage = AD4170_REFIN_AVDD_VOLTAGE;
	} else {
		ref_voltage = AD4170_REFIN_REFOUT_VOLTAGE;
	}

	return ref_voltage;
}

/*!
 * @brief	Convert the ADC raw value into equivalent voltage
 * @param	adc_raw[in]- ADC raw data
 * @param	chn[in] - ADC channel
 * @return	ADC voltage value
 */
float convert_adc_sample_into_voltage(uint32_t adc_raw, uint8_t chn)
{
	float gain;
	float vref;
	int32_t adc_data;
	uint8_t setup = p_ad4170_dev_inst->config.setup[chn].setup_n;
	bool bipolar = p_ad4170_dev_inst->config.setups[setup].afe.bipolar;

	vref = ad4170_get_reference_voltage(chn);
	gain = ad4170_get_gain_value(chn);
	adc_data = perform_sign_conversion(adc_raw, chn);

	if (bipolar) {
		return (adc_data * (vref / (ADC_MAX_COUNT_BIPOLAR * gain)));
	} else {
		return (adc_data * (vref / (ADC_MAX_COUNT_UNIPOLAR * gain)));
	}
}

/**
 * @brief	Convert ADC data to voltage without Vref
 * @param	data[in] - ADC data in straight binary format (signed)
 * @param	chn[in] - ADC channel
 * @return	voltage
 */
float convert_adc_data_to_voltage_without_vref(int32_t data, uint8_t chn)
{
	float gain;
	uint8_t setup = p_ad4170_dev_inst->config.setup[chn].setup_n;
	bool bipolar = p_ad4170_dev_inst->config.setups[setup].afe.bipolar;

	gain = ad4170_get_gain_value(chn);

	if (bipolar) {
		return (data / (ADC_MAX_COUNT_BIPOLAR * gain));
	} else {
		return (data / (ADC_MAX_COUNT_UNIPOLAR * gain));
	}
}

/**
 * @brief	Convert ADC data to voltage w.r.t. Vref
 * @param	data[in] - ADC data in straight binary format (signed)
 * @param	chn[in] - ADC channel
 * @return	voltage
 */
float convert_adc_data_to_voltage_wrt_vref(int32_t data, uint8_t chn)
{
	float gain;
	float vref;
	uint8_t setup = p_ad4170_dev_inst->config.setup[chn].setup_n;
	bool bipolar = p_ad4170_dev_inst->config.setups[setup].afe.bipolar;

	vref = ad4170_get_reference_voltage(chn);
	gain = ad4170_get_gain_value(chn);

	if (bipolar) {
		return (data * (vref / (ADC_MAX_COUNT_BIPOLAR * gain)));
	} else {
		return (data * (vref / (ADC_MAX_COUNT_UNIPOLAR * gain)));
	}
}

/*!
 * @brief	Convert the ADC raw value into equivalent RTD resistance
 * @param	adc_raw[in] - ADC raw sample
 * @param	rtd_ref[in] - RTD reference resistance in ohms
 * @param	chn[in] - ADC channel
 * @return	RTD resistance value
 * @note	RTD is biased with constant excitation current. Below formula
 *			is based on ratiometric measurement, where fixed value of RTD RREF
 *			(reference resistor) and gain is taken into account
 */
float convert_adc_raw_into_rtd_resistance(uint32_t adc_raw, float rtd_ref,
		uint8_t chn)
{
	float gain;
	int32_t adc_data;
	uint8_t setup = p_ad4170_dev_inst->config.setup[chn].setup_n;
	bool bipolar = p_ad4170_dev_inst->config.setups[setup].afe.bipolar;

	gain = ad4170_get_gain_value(chn);
	adc_data = perform_sign_conversion(adc_raw, chn);

	if (bipolar) {
		return (((float)adc_data * rtd_ref) / (ADC_MAX_COUNT_BIPOLAR * gain));
	} else {
		return (((float)adc_data * rtd_ref) / (ADC_MAX_COUNT_UNIPOLAR * gain));
	}
}

/*!
 * @brief	Disable ADC conversion
 * @return	0 in case of success, negative error code otherwise
 */
int32_t ad4170_disable_conversion(void)
{
	int32_t ret;
	struct ad4170_adc_ctrl adc_ctrl = p_ad4170_dev_inst->config.adc_ctrl;

	/* Exit from continuous read mode */
	if (adc_ctrl.cont_read == AD4170_CONT_READ_ON) {
		ret = ad4170_continuous_read_exit(p_ad4170_dev_inst);
		if (ret) {
			return ret;
		}

		adc_ctrl.cont_read = AD4170_CONT_READ_OFF;
	}

	adc_ctrl.mode = AD4170_MODE_STANDBY;
	return ad4170_set_adc_ctrl(p_ad4170_dev_inst, adc_ctrl);
}

/*!
 * @brief	Enable input channel
 * @param	input_chn[in] - Channel to be enabled
 * @return	0 in case of success, negative error otherwise
 */
int32_t ad4170_enable_input_chn(uint8_t input_chn)
{
	uint16_t chn_enable_status;

	chn_enable_status = (p_ad4170_dev_inst->config.channel_en | AD4170_CHANNEL(
				     input_chn));
	return ad4170_set_channel_en(p_ad4170_dev_inst, chn_enable_status);
}

/*!
 * @brief	Disable input channel
 * @param	input_chn[in] - Channel to be disabled
 * @return	0 in case of success, negative error otherwise
 */
int32_t ad4170_disable_input_chn(uint8_t input_chn)
{
	uint16_t chn_enable_status = 0;

	chn_enable_status = (p_ad4170_dev_inst->config.channel_en & ~AD4170_CHANNEL(
				     input_chn));
	return ad4170_set_channel_en(p_ad4170_dev_inst, chn_enable_status);
}

/*!
 * @brief	Set the excitation sources based on sensor demo config
 * @param	input_chn[in] - Input channel
 * @param	exc_enable[in] - Excitation enable/disable flag
 * @return	0 in case of success, negative error error code otherwise
 */
static int32_t ad4170_set_excitation_sources(uint8_t input_chn,
		bool exc_enable)
{
	int32_t ret;
	struct ad4170_current_source current_source;
	uint8_t exc_source1;
	uint8_t exc_source2;
	enum ad4170_i_out_val exc_val;

	/* Load the excitation current value (same for all configs) */
	if (exc_enable) {
		exc_val = AD4170_I_OUT_500UA;
	} else {
		exc_val = AD4170_I_OUT_0UA;
	}

#if (ACTIVE_DEMO_MODE_CONFIG == RTD_3WIRE_CONFIG)
	switch (input_chn) {
	case SENSOR_CHANNEL0:
		/* Use excitation sources 0 and 1 for Sensor 0 */
		exc_source1 = 0;
		exc_source2 = 1;
		break;

	case SENSOR_CHANNEL1:
		/* Use excitation sources 2 and 3 for Sensor 1 */
		exc_source1 = 2;
		exc_source2 = 3;
		break;

	default:
		return -EINVAL;
	}

	/* Apply/Remove excitation */
	current_source = p_ad4170_dev_inst->config.current_source[exc_source1];
	current_source.i_out_val = exc_val;
	ret = ad4170_set_current_source(p_ad4170_dev_inst,
					exc_source1,
					current_source);
	if (ret) {
		return ret;
	}

	/* Apply/Remove excitation */
	current_source = p_ad4170_dev_inst->config.current_source[exc_source2];
	current_source.i_out_val = exc_val;
	return ad4170_set_current_source(p_ad4170_dev_inst, exc_source2,
					 current_source);
#elif ((ACTIVE_DEMO_MODE_CONFIG == RTD_2WIRE_CONFIG) || (ACTIVE_DEMO_MODE_CONFIG == RTD_4WIRE_CONFIG))
	switch (input_chn) {
	case SENSOR_CHANNEL0:
		/* Use excitation source 0 for Sensor 0 */
		exc_source1 = 0;
		break;

	case SENSOR_CHANNEL1:
		/* Use excitation source 1 for Sensor 1 */
		exc_source1 = 1;
		break;

	case SENSOR_CHANNEL2:
		/* Use excitation source 2 for Sensor 2 */
		exc_source1 = 2;
		break;

	default:
		return -EINVAL;
	}

	/* Apply/Remove excitation */
	current_source = p_ad4170_dev_inst->config.current_source[exc_source1];
	current_source.i_out_val = exc_val;
	return ad4170_set_current_source(p_ad4170_dev_inst, exc_source1,
					 current_source);
#elif ((ACTIVE_DEMO_MODE_CONFIG == THERMOCOUPLE_CONFIG) && defined(USE_CJC_AS_RTD))
	/* Apply/Remove excitation */
	exc_source1 = 0;
	current_source = p_ad4170_dev_inst->config.current_source[exc_source1];
	current_source.i_out_val = exc_val;
	return ad4170_set_current_source(p_ad4170_dev_inst, exc_source1,
					 current_source);
#endif

	return 0;
}

/*!
 * @brief	Apply the excitation sources
 * @param	input_chn[in] - Input channel
 * @return	0 in case of success, negative error code otherwise
 */
int32_t ad4170_apply_excitation(uint8_t input_chn)
{
	return ad4170_set_excitation_sources(input_chn, true);
}

/*!
 * @brief	Remove the excitation sources
 * @param	input_chn[in] - Input channel
 * @return	0 in case of success, negative error code otherwise
 */
int32_t ad4170_remove_excitation(uint8_t input_chn)
{
	return ad4170_set_excitation_sources(input_chn, false);
}

/*!
 * @brief Set filter type
 * @param dev[in, out] - AD4170 device descriptor
 * @param chn[in] - Channel ID
 * @param filt_type[in] - Filter Type
 * @return	0 in case of success, negative error code otherwise
 */
int32_t ad4170_set_filter(struct ad4170_dev *dev, uint8_t chn,
			  enum ad4170_filter_type filt_type)
{
	uint8_t reg;
	int32_t ret;
	uint8_t setup = dev->config.setup[chn].setup_n;

	if (!dev) {
		return -EINVAL;
	}

	reg = no_os_field_prep(AD4170_ADC_SETUPS_FILTER_TYPE_MSK,
			       filt_type);
	ret = ad4170_spi_reg_write(dev, AD4170_REG_ADC_SETUPS_FILTER(setup), reg);
	if (ret) {
		return ret;
	}

	dev->config.setups[chn].filter.filter_type = filt_type;

	return 0;
}

/*!
 * @brief Set Reference
 * @param dev[in, out] - AD4170 device descriptor
 * @param chn[in] - Channel ID
 * @param ref[in] - Reference Type
 * @return	0 in case of success, negative error code otherwise
 */
int32_t ad4170_set_reference(struct ad4170_dev *dev, uint8_t chn,
			     enum ad4170_ref_select ref)
{
	uint8_t reg;
	int32_t ret;
	uint8_t setup = p_ad4170_dev_inst->config.setup[chn].setup_n;

	if (!dev) {
		return -EINVAL;
	}

	reg = no_os_field_prep(AD4170_ADC_SETUPS_AFE_REF_SELECT_MSK,
			       ref);
	ret = ad4170_spi_reg_write(dev, AD4170_REG_ADC_SETUPS_AFE(setup), reg);
	if (ret) {
		return ret;
	}

	p_ad4170_dev_inst->config.setups[p_ad4170_dev_inst->config.setup[chn].setup_n].afe.ref_select
		= ref;

	return 0;
}

/*!
 * @brief Set Reference
 * @param dev[in, out] - AD4170 device descriptor
 * @param setup[in] - Setup ID
 * @param chn[in] - Channel ID
 * @param ref[in] - Reference Type
 * @return	0 in case of success, negative error code otherwise
 */
int32_t ad4170_set_fs(struct ad4170_dev *dev, uint8_t setup, uint8_t chn,
		      uint16_t fs_val)
{
	int32_t ret;
	uint8_t reg;

	if (!dev) {
		return -EINVAL;
	}

	ret = ad4170_spi_reg_write(dev, AD4170_REG_ADC_SETUPS_FILTER_FS(setup), fs_val);
	if (ret) {
		return ret;
	}

	dev->config.setups[dev->config.setup[chn].setup_n].filter_fs = fs_val;

	return 0;
}
