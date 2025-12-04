/***************************************************************************//**
 *   @file    ad4134_support.c
 *   @brief   Source file for for AD4134 IIO Application
********************************************************************************
 * Copyright (c) 2023,2025 Analog Devices, Inc.
 * All rights reserved.
 *
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 * By using this software you agree to the terms of the associated
 * Analog Devices Software License Agreement.
*******************************************************************************/

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/
#include "ad4134_support.h"
#include "no_os_error.h"
#include "no_os_delay.h"
#include "app_config.h"
#if (INTERFACE_MODE == TDM_MODE)
#include "stm32_tdm_support.h"
#endif

/******************************************************************************/
/********************** Macros and Constants Definitions **********************/
/******************************************************************************/
/* Min count for input pin debouncing. The count is dependent on the MCU
 * clock frequency and compiler used. The debounce count added below ensures
 * pins are debounced for this minimum count in a while loop */
#define GPIO_MIN_DBNCE_CNT		(2U)

/* Max wait count for ODR to trigger during conversion wait and read function
 * in a while loop */
#define ODR_TRIGGER_WAIT_DBNCE_CNT	(20000U)

/* Channel offset for dual data read mode */
#define DUAL_CHN_MODE_OFFSET	(2)

/* LT6373 amplifier gain configuration. Gain = 1
 * GPIO 0, 1 and 2 sets the gain for AIN0+/- and AIN1+/-
 * GPIO 5, 6 and 7 sets the gain for AIN2+/- and AIN3+/-
 **/
#define LT6373_GPIO_DIR_CTRL_VAL	0xFF	// All pins are output
#define LT6373_GPIO_DATA_VAL		0x84	// GPIO 0,1,2,5,6,7 are set high

/* Finding minimum required DCLK frequency for ASRC controller mode =>
 * DCLK(min) = ODR * chn per DOUT * (frame size + 6) =>
 * DCLK = 16KSPS * 2 * (16+6) => DCLK Value = 704KHz
 **/
#if (INTERFACE_MODE == BIT_BANGING_MODE)
#define DCLK_FREQ_SELECT	5	// Using 1.5MHz DCLK
#elif (INTERFACE_MODE == TDM_MODE)
#define DCLK_FREQ_SELECT	3	// Using 6MHz DCLK
#endif

/* Configure ODR (data rate) for ASRC controller mode */
#define MCLK_FREQ		(48000000)
#define MCLK_DIVISOR	(2)
#define	ODR_INT_VAL		(uint32_t)(MCLK_FREQ / MCLK_DIVISOR / SAMPLING_RATE)

/* Set integral part */
#define ODR_VAL_INT_LSB		(uint8_t)(ODR_INT_VAL)
#define ODR_VAL_INT_MID		(uint8_t)(ODR_INT_VAL >> 8)
#define ODR_VAL_INT_MSB		(uint8_t)(ODR_INT_VAL >> 16)

/* Set fractional part */
#define ODR_VAL_FLT_LSB		0x00
#define ODR_VAL_FLT_MID0	0x00
#define ODR_VAL_FLT_MID1	0x00
#define ODR_VAL_FLT_MSB		0x00

/******************************************************************************/
/********************** Variables and User Defined Data Types *****************/
/******************************************************************************/
/* Interface mode */
static enum ad4134_interface_modes ad4134_iio_interface_mode =
#if (INTERFACE_MODE == TDM_MODE)
	INTERFACE_MODE_TDM;
#elif (INTERFACE_MODE == BIT_BANGING_MODE)
	INTERFACE_MODE_BIT_BANGING;
#elif (INTERFACE_MODE == MINIMAL_IO_MODE)
	INTERFACE_MODE_MINIMAL_IO;
#endif

/* Data capture mode */
static enum ad4134_data_capture_modes ad4134_iio_data_capture_mode =
#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
	DATA_CAPTURE_MODE_CONTINUOUS;
#elif (DATA_CAPTURE_MODE == BURST_DATA_CAPTURE)
	DATA_CAPTURE_MODE_BURST;
#endif

/* ASRC mode */
static enum ad4134_asrc_modes ad4134_iio_asrc_mode =
#if (AD7134_ASRC_MODE == CONTROLLER_MODE)
	ASRC_MODE_CONTROLLER;
#elif (AD7134_ASRC_MODE == TARGET_MODE)
	ASRC_MODE_TARGET;
#endif

#if (INTERFACE_MODE == BIT_BANGING_MODE)
/* Variables holding the GPIO Input Data Register (IDR) values */
static uint32_t dout0_idr_vals[ADC_RESOLUTION * DUAL_CHN_MODE_OFFSET];
static uint32_t dout1_idr_vals[ADC_RESOLUTION * DUAL_CHN_MODE_OFFSET];
#endif

/******************************************************************************/
/************************ Functions Definitions *******************************/
/******************************************************************************/
/**
 * @brief 	Get the interface mode.
 * @return 	Interface mode
 */
inline enum ad4134_interface_modes ad4134_get_interface_mode(void)
{
	return ad4134_iio_interface_mode;
}

/**
 * @brief 	Get the data capture mode.
 * @return 	Data capture mode
 */
inline enum ad4134_data_capture_modes ad4134_get_data_capture_mode(void)
{
	return ad4134_iio_data_capture_mode;
}

/**
 * @brief 	Get the ASRC mode.
 * @return 	ASRC mode
 */
inline enum ad4134_asrc_modes ad4134_get_asrc_mode(void)
{
	return ad4134_iio_asrc_mode;
}

/*!
 * @brief   Perform the data capture initialization
 * @param 	dev[in] - AD7134 Device descriptor.
 * @return  0 in case of success, negative error code otherwise
 * @details This function configures the AD7134 registers to capture the data
 */
int32_t ad7134_data_capture_init(struct ad713x_dev *dev)
{
	do {
		/* Select High performance power mode */
		if (ad713x_set_power_mode(dev, HIGH_POWER) != 0) {
			break;
		}

		if (ad4134_get_interface_mode() != INTERFACE_MODE_TDM) {
			/* Select CH0 filter as wideband for required ODR */
			if (ad713x_dig_filter_sel_ch(dev, FIR, 0) != 0) {
				break;
			}
		} else {
			/* Select SINC3 filter to enable ODR higher than 374ksps */
			if (ad713x_dig_filter_sel_ch(dev, SINC3, 0) != 0) {
				break;
			}
			if (ad713x_dig_filter_sel_ch(dev, SINC3, 1) != 0) {
				break;
			}
			if (ad713x_dig_filter_sel_ch(dev, SINC3, 2) != 0) {
				break;
			}
			if (ad713x_dig_filter_sel_ch(dev, SINC3, 3) != 0) {
				break;
			}
		}

		/* Set GPIO direction and value for gain selection of LT6373 */
		if (ad713x_spi_reg_write(dev,
					 AD713X_REG_GPIO_DIR_CTRL,
					 LT6373_GPIO_DIR_CTRL_VAL) != 0) {
			break;
		}

		if (ad713x_spi_reg_write(dev,
					 AD713X_REG_GPIO_DATA,
					 LT6373_GPIO_DATA_VAL) != 0) {
			break;
		}

#if (AD7134_ASRC_MODE == CONTROLLER_MODE)
		/* Set the DCLK frequency */
		if (ad713x_spi_write_mask(dev,
					  AD713X_REG_DATA_PACKET_CONFIG,
					  AD713X_DATA_PACKET_CONFIG_DCLK_FREQ_MSK,
					  AD713X_DATA_PACKET_CONFIG_DCLK_FREQ_MODE(DCLK_FREQ_SELECT)) != 0) {
			break;
		}

		/* Load the ODR value integer and floating registers (controller) */
		if (ad713x_spi_reg_write(dev,
					 AD713X_REG_ODR_VAL_INT_LSB,
					 ODR_VAL_INT_LSB) != 0) {
			break;
		}

		if (ad713x_spi_reg_write(dev,
					 AD713X_REG_ODR_VAL_INT_MID,
					 ODR_VAL_INT_MID) != 0) {
			break;
		}

		if (ad713x_spi_reg_write(dev,
					 AD713X_REG_ODR_VAL_INT_MSB,
					 ODR_VAL_INT_MSB) != 0) {
			break;
		}

		if (ad713x_spi_reg_write(dev,
					 AD713X_REG_ODR_VAL_FLT_LSB,
					 ODR_VAL_FLT_LSB) != 0) {
			break;
		}

		if (ad713x_spi_reg_write(dev,
					 AD713X_REG_ODR_VAL_FLT_MID0,
					 ODR_VAL_FLT_MID0) != 0) {
			break;
		}

		if (ad713x_spi_reg_write(dev,
					 AD713X_REG_ODR_VAL_FLT_MID1,
					 ODR_VAL_FLT_MID1) != 0) {
			break;
		}

		if (ad713x_spi_reg_write(dev,
					 AD713X_REG_ODR_VAL_FLT_MSB,
					 ODR_VAL_FLT_MSB) != 0) {
			break;
		}

		/* Transfer controller registers data to target */
		if (ad713x_spi_reg_write(dev,
					 AD713X_REG_TRANSFER_REGISTER,
					 AD713X_TRANSFER_MASTER_SLAVE_TX_BIT_MSK) != 0) {
			break;
		}

		/* Make sure data ODR is updated into target */
		no_os_mdelay(500);
#endif

		return 0;
	} while (0);

	return -EINVAL;
}

#if (INTERFACE_MODE == BIT_BANGING_MODE)
/*!
 * @brief	Generate the ODR low to DCLK high delay in AD7134 target mode
 * @return	none
 * @note	The delay is derived based on the NOP instruction and tested for
 *			STM32F469NI MCU on SDP-K1 controller board with GCC and ARMC compilers.
 *			Delay time may vary from MCU to MCU and compiler optimization level
 */
static void odr_low_to_dclk_high_delay(void)
{
	/* Delay b/w ODR falling edge to DCLK rising edge in target mode is
	 * min 8nsec as per device specifications */

	/* Delay = 2 (#nop) * 4 (instruction cycles) * 5.5nsec (1/Fclk=1/180Mhz)
	 *       = 2 * 4 * 5.5 = ~44nsec */
	asm("nop");
	asm("nop");
}

/*!
 * @brief	Generate the DCLK high/low delay in AD7134 target mode
 * @return	none
 * @note	The delay is derived based on the NOP instruction and tested for
 *			STM32F469NI MCU on SDP-K1 controller board with GCC and ARMC compilers.
 *			Delay time may vary from MCU to MCU and compiler optimization level
 */
static void dclk_high_low_delay(void)
{
	/* DCLK high/low period is min tdclk/2-1 and max 1/24Mhz = ~42nsec as per
	 * device specifications */

	/* Delay = 5 (#nop's) * 4 (instruction cycles) * 5.5nsec (1/Fclk=1/180Mhz)
	 *       = 5 * 4 * 5.5 = ~110nsec
	 * Actual DCLK high/low time = 66nsec + time to sample the data over DOUTx pin */
	asm("nop");
	asm("nop");
	asm("nop");
	asm("nop");
	asm("nop");
}

/*!
 * @brief	Wait for ODR GPIO to change to new state for data read operation
 * @param	new_gpio_state[out] - New expected GPIO state
 * @param	timeout[in] - Timeout in ticks
 * @return	0 in case of success, negative error code otherwise
 */
static int32_t wait_for_odr_gpio_state_change(bool new_gpio_state,
		uint16_t timeout)
{
	uint16_t odr_dbnc = 0;

	if (ad4134_get_interface_mode() == INTERFACE_MODE_BIT_BANGING) {
		do {
			if (new_gpio_state == ((uint32_t)(ODR_IDR & ODR_PIN_MASK) >> ODR_PIN)) {
				/* Increment debounce counter if new state is detected */
				odr_dbnc++;
			} else {
				/* Reset debounce counter and increment timeout counter if new
				 * state is not detected */
				odr_dbnc = 0;
				timeout--;
			}
		} while ((odr_dbnc < GPIO_MIN_DBNCE_CNT) && (timeout));

		if (!timeout) {
			return -ETIMEDOUT;
		}
	}

	return 0;
}
#endif

/*!
 * @brief	Read ADC data over SAI TDM Peripheral
 * @param	adc_data[out] - Pointer to adc data read variable
 * @param	curr_chn[in] - Channel for which data is to read
 * @return	0 in case of success, negative error code otherwise
 */
static int32_t ad7134_read_tdm_data(uint16_t *adc_data, uint8_t curr_chn)
{
#if (INTERFACE_MODE == TDM_MODE)
	int32_t ret;
	uint8_t channel_data[8];
	uint32_t timeout = AD7134_CONV_TIMEOUT;

	ret = no_os_tdm_read(ad7134_tdm_desc, channel_data, TDM_SLOTS_PER_FRAME);
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
	*adc_data = no_os_get_unaligned_le16(&channel_data[curr_chn *
						      BYTES_PER_SAMPLE]);

	return 0;
#endif

	return -EINVAL;
}

/*!
 * @brief	Read ADC data over DOUT0 and DOUT1 pins using bit-banging method for
 * 			all channels
 * @param 	chn_data[in, out] - Pointer to array for adc data for all channels
 * @param 	check_odr_state[in] - Whether to check for ODR state change
 * @return 0 in case of success, negative error code otherwise
 */
int32_t ad7134_read_all_channels_bit_banging(uint16_t *chn_data,
		bool check_odr_state)
{
#if (INTERFACE_MODE == BIT_BANGING_MODE)
	uint8_t bit_cnt;
	uint8_t frame_cntr;
	uint8_t cntr;

	if (check_odr_state) {
		/* Debounce the ODR for HIGH (rising edge to ready for data read) */
		if (wait_for_odr_gpio_state_change(NO_OS_GPIO_HIGH,
						   ODR_TRIGGER_WAIT_DBNCE_CNT) != 0) {
			return -EINVAL;
		}

		/* Debounce the ODR for LOW to start data read */
		if (wait_for_odr_gpio_state_change(NO_OS_GPIO_LOW,
						   ODR_TRIGGER_WAIT_DBNCE_CNT) != 0) {
			return -EINVAL;
		}
	}

	/* Read the ADC data for all channels using Dual channel data mode
	 * Chn0 and 1 are output on DOUT0 pin
	 * Chn2 and 3 are output on DOUT1 pin
	 */
	if (ad4134_get_asrc_mode() == ASRC_MODE_TARGET) {
		odr_low_to_dclk_high_delay();

		for (bit_cnt = 0; bit_cnt < (ADC_RESOLUTION * DUAL_CHN_MODE_OFFSET);
		     bit_cnt++) {
			/* Set DCLK pin to high to sample next DOUT bit. High time is ~0.2usec
			 * based on the non-loop delay */
			DCLK_ODR |= (1 << DCLK_PIN);
			dclk_high_low_delay();

			/* Store the PORTG IDR register value which corresponds to DOUT0 pin */
			dout0_idr_vals[bit_cnt] = DOUT0_IDR;

			/* Store the PORTA IDR register value which corresponds to DOUT1 pin */
			dout1_idr_vals[bit_cnt] = DOUT1_IDR;

			/* Set DCLK pin to low to sample next DOUT bit. Low time is ~0.2usec
			 * based on the non-loop delay */
			DCLK_ODR &= (uint32_t)(~(1 << DCLK_PIN));
			dclk_high_low_delay();
		}
	} else {
		for (bit_cnt = 0; bit_cnt < (ADC_RESOLUTION * DUAL_CHN_MODE_OFFSET);
		     bit_cnt++) {
			/* Wait for DCLK pin to go high to sample DOUTx bit */
			while (!(DCLK_IDR & DCLK_PIN_MASK));

			/* Store the PORTG IDR register value which corresponds to DOUT0 pin */
			dout0_idr_vals[bit_cnt] = DOUT0_IDR;

			/* Store the PORTA IDR register value which corresponds to DOUT1 pin */
			dout1_idr_vals[bit_cnt] = DOUT1_IDR;

			/* Wait for DCLK pin to go low to sample next DOUTx bit */
			while ((DCLK_IDR & DCLK_PIN_MASK));
		}
	}

	/* Clear channel data variables */
	chn_data[0] = 0;
	chn_data[1] = 0;
	chn_data[2] = 0;
	chn_data[3] = 0;

	/* Extract DOUTx data bits from IDR register corresponding to each channel */
	for (frame_cntr = 0, cntr = ADC_RESOLUTION - 1;
	     frame_cntr < ADC_RESOLUTION; frame_cntr++) {
		if (dout0_idr_vals[frame_cntr] & DOUT0_PIN_MASK) {
			chn_data[0] |= (1 << cntr);
		}

		if (dout0_idr_vals[frame_cntr + ADC_RESOLUTION] & DOUT0_PIN_MASK) {
			chn_data[1] |= (1 << cntr);
		}

		if (dout1_idr_vals[frame_cntr] & DOUT1_PIN_MASK) {
			chn_data[2] |= (1 << cntr);
		}

		if (dout1_idr_vals[frame_cntr + ADC_RESOLUTION] & DOUT1_PIN_MASK) {
			chn_data[3] |= (1 << cntr);
		}

		cntr--;
	}

	return 0;
#endif

	return -EINVAL;
}

/*!
 * @brief	Read ADC data over DOUT0 and DOUT1 pins using bit-banging method for
 * 			a single channel
 * @param	adc_data[in, out] - Pointer to adc data read variable
 * @param	ch[in] - Channel for which data is to read
 * @param 	check_odr_state[in] - Whether to check for ODR GPIO state
 * @return	0 in case of success, negative error code otherwise
 */
static int32_t ad7134_read_data_bit_banging(uint16_t *adc_data, uint8_t ch,
		bool check_odr_state)
{
#if (INTERFACE_MODE == BIT_BANGING_MODE)
	int32_t ret;
	uint16_t chn_data[AD7134_NUM_CHANNELS] = {0};

	/* Get all channels data */
	ret = ad7134_read_all_channels_bit_banging(chn_data, check_odr_state);
	if (ret) {
		return ret;
	}

	*adc_data = chn_data[ch];

	return 0;
#endif

	return -EINVAL;
}

/*!
 * @brief	Read ADC single sample data
 * @param	adc_data[in, out] - Pointer to adc data read variable
 * @param	ch[in] - Channel for which data is to read
 * @return	0 in case of success, negative error code otherwise
 */
int32_t ad7134_perform_conv_and_read_sample(uint16_t *adc_data, uint8_t ch)
{
	switch (ad4134_get_interface_mode()) {
	case INTERFACE_MODE_TDM:
		return ad7134_read_tdm_data(adc_data, ch);
	case INTERFACE_MODE_BIT_BANGING:
		return ad7134_read_data_bit_banging(adc_data, ch, true);

	default:
		return -EINVAL;
	}

	return 0;
}
