/***************************************************************************//**
 *   @file    app_config_stm32.c
 *   @brief   Application configurations module for STM32 platform
********************************************************************************
 * Copyright (c) 2024 Analog Devices, Inc.
 * All rights reserved.
 *
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 * By using this software you agree to the terms of the associated
 * Analog Devices Software License Agreement.
*******************************************************************************/

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include "app_config_stm32.h"
#include "no_os_error.h"
#include "no_os_util.h"
#include "no_os_pwm.h"
#include "ad7091r_iio.h"

/******************************************************************************/
/************************ Macros/Constants ************************************/
/******************************************************************************/

/* Timer channel output */
#define TIM_CCMR_CCS_OUTPUT 0

/* Compare pulse as trigger event */
#define TIM_CR2_MMS_COMPARE_PULSE 3

/* Trigger mode as slave mode */
#define TIM_SMCR_SMS_TRIGGER 6

/* TIM1 as ITR source */
#define TIM_ITR_SOURCE 0

/******************************************************************************/
/******************** Variables and User Defined Data Types *******************/
/******************************************************************************/

/* UART STM32 Platform Specific Init Parameters */
struct stm32_uart_init_param stm32_uart_init_params = {
	.huart = APP_UART_HANDLE
};

/* VCOM Init Parameters */
struct stm32_usb_uart_init_param stm32_vcom_extra_init_params = {
	.husbdevice = APP_UART_USB_HANDLE
};

/* STM32 GPIO IRQ specific parameters */
struct stm32_gpio_irq_init_param stm32_trigger_gpio_irq_init_params = {
	.port_nb = GPIO_TRIGGER_INT_PORT
};

/* SPI STM32 Platform Specific Init Parameters */
struct stm32_spi_init_param stm32_spi_init_params = {
	.chip_select_port = STM32_SPI_CS_PORT,
	.get_input_clock = HAL_RCC_GetPCLK2Freq
};

/* I2C STM32 Platform Specific Init Parameters */
struct stm32_i2c_init_param stm32_i2c_init_params = {
	.i2c_timing = I2C_TIMING
};

/* CNV pin STM32 GPIO specific parameters */
struct stm32_gpio_init_param stm32_gpio_cnv_init_params = {
	.mode = GPIO_MODE_OUTPUT_PP,
	.speed = GPIO_SPEED_FREQ_VERY_HIGH
};

/* STM32 CNV PWM GPIO specific parameters */
struct stm32_gpio_init_param stm32_pwm_cnv_gpio_init_params = {
	.mode = GPIO_MODE_AF_PP,
	.speed = GPIO_SPEED_FREQ_VERY_HIGH,
	.alternate = GPIO_AF1_TIM1
};

/* Reset pin STM32 GPIO specific parameters */
struct stm32_gpio_init_param stm32_gpio_reset_init_params = {
	.mode = GPIO_MODE_OUTPUT_PP
};

/* STM32 GP0 GPIO specific parameters */
struct stm32_gpio_init_param stm32_gpio_gp0_extra_init_params = {
	.mode = GPIO_MODE_INPUT,
	.speed = GPIO_SPEED_FREQ_VERY_HIGH,
};

/* STM32 CNV PWM specific parameters */
struct stm32_pwm_init_param stm32_cnv_pwm_init_params = {
	.htimer = &CNV_PWM_HANDLE,
	.prescaler = CNV_PWM_PRESCALER,
	.timer_autoreload = true,
	.mode = TIM_OC_PWM2,
	.timer_chn = CNV_PWM_CHANNEL,
	.get_timer_clock = HAL_RCC_GetPCLK2Freq,
	.clock_divider = CNV_PWM_CLK_DIVIDER,
	.complementary_channel = false
};

#if (INTERFACE_MODE == SPI_DMA)
/* STM32 PWM for specific parameters */
struct stm32_pwm_init_param stm32_cs_extra_init_params = {
	.htimer = &CS_TIMER_HANDLE,
	.prescaler = CS_TIMER_PRESCALER,
	.timer_autoreload = false,
	.mode = TIM_OC_PWM2,
	.timer_chn = CS_TIMER_CHANNEL,
	.complementary_channel = false,
	.get_timer_clock = HAL_RCC_GetPCLK1Freq,
	.clock_divider = TIMER_2_CLK_DIVIDER
};

/* STM32 PWM specific init params */
struct stm32_pwm_init_param stm32_tx_trigger_extra_init_params = {
	.htimer = &TIMER8_HANDLE,
	.prescaler = TIMER_8_PRESCALER,
	.timer_autoreload = true,
	.mode = TIM_OC_TOGGLE,
	.timer_chn = TIMER_CHANNEL_1,
	.complementary_channel = false,
	.get_timer_clock = HAL_RCC_GetPCLK1Freq,
	.clock_divider = TIMER_8_CLK_DIVIDER
};

/* STM32 Tx DMA channel extra init params */
struct stm32_dma_channel txdma_channel = {
	.hdma = &hdma_tim8_ch1,
	.ch_num = TxDMA_CHANNEL_NUM,
	.mem_increment = false,
	.mem_data_alignment = DATA_ALIGN_BYTE,
	.per_data_alignment = DATA_ALIGN_BYTE,
	.dma_mode = DMA_CIRCULAR_MODE
};

/* STM32 Rx DMA channel extra init params */
struct stm32_dma_channel rxdma_channel = {
	.hdma = &hdma_spi1_rx,
	.ch_num = RxDMA_CHANNEL_NUM,
	.mem_increment = true,
	.mem_data_alignment = DATA_ALIGN_BYTE,
	.per_data_alignment = DATA_ALIGN_BYTE,
	.dma_mode = DMA_CIRCULAR_MODE,
};

/* STM32 CS GPIO Extra init params in PWM Mode */
struct stm32_gpio_init_param stm32_cs_pwm_gpio_extra_init_params = {
	.mode = GPIO_MODE_AF_PP,
	.speed = GPIO_SPEED_FREQ_VERY_HIGH,
	.alternate = GPIO_AF1_TIM2
};

/* STM32 CS GPIO Extra init params in GPIO Mode */
struct stm32_gpio_init_param stm32_cs_gpio_extra_init_params = {
	.mode = GPIO_MODE_OUTPUT_PP,
	.speed = GPIO_SPEED_FREQ_VERY_HIGH,
};

/* STM32 SPI Descriptor*/
volatile struct stm32_spi_desc* sdesc;

/* Value of RXDMA NDTR Reg */
uint32_t rxdma_ndtr;

/* Number of times the DMA complete callback needs to be invoked for
 * capturing the desired number of samples*/
uint32_t dma_cycle_count = 0;

/* IIO Buffer start index */
uint8_t* iio_buf_start_idx;

/* DMA Buffer Start index */
uint8_t* dma_buf_start_idx;

/* IIO Buffer present index */
uint8_t* iio_buf_current_idx;

/* DMA Buffer present index */
uint8_t* dma_buf_current_idx;
#endif

/******************************************************************************/
/************************** Functions Declarations ****************************/
/******************************************************************************/

/******************************************************************************/
/************************** Functions Definitions *****************************/
/******************************************************************************/

/**
 * @brief 	Initialize the STM32 system peripherals
 * @return	None
 */
void stm32_system_init(void)
{
	HAL_Init();
	SystemClock_Config();
#if (INTERFACE_MODE == SPI_DMA)
	MX_DMA_Init();
	MX_TIM2_Init();
	MX_TIM8_Init();
#endif
	MX_GPIO_Init();
	MX_SPI1_Init();
	MX_UART5_Init();
	MX_TIM1_Init();
	MX_I2C1_Init();
	MX_USB_DEVICE_Init();
}

/**
 * @brief Configure CS timer
 * @return None
 */
void tim2_config(void)
{
	TIM2->CCMR1 &= ~TIM_CCMR1_CC1S_Msk; // Output compare

	TIM2->SMCR |= (TIM_TS_ITR1 |
		       TIM_SLAVEMODE_TRIGGER); // Set in trigger slave mode with ITR1 as trigger source (TX trigger timer)

	TIM2->CCER |= TIM_CCER_CC1E; // CC1 Output Enable
}

/**
 * @brief Configure Tx Trigger timer
 * @return None
 * @note Tx timer acts as a slave for External Trigger (ETR1)
 *       in one-pulse mode to generate DMA requests
 */
void tim8_config(void)
{
	TIM8->RCR = BYTES_PER_SAMPLE - 1; // RCR value in one-pulse mode

	TIM8->EGR = TIM_EGR_UG; // Generate update event

	TIM8->DIER |=
		TIM_DIER_CC1DE; // Generate DMA request after capture/compare event

	TIM8->SMCR |=
		TIM_TRIGGERPOLARITY_INVERTED; // Inverted Polarity for ETR trigger source (Busy falling edge)
}

/**
 * @brief 	Starts the timer signal generation for
 *          PWM and OC channels all at once.
 * @return	None
 */
void stm32_timer_enable(void)
{
#if (INTERFACE_MODE == SPI_DMA)
	TIM1->CNT = 0;
	TIM2->CNT = 0;
	TIM8->CNT = 0;

	TIM1->CCER |= TIM_CCER_CC3E; // Enable Capture/Compare 3 output

	TIM1->BDTR |= TIM_BDTR_MOE;

	/* Start CNV PWM */
	TIM1->CR1 |= TIM_CR1_CEN;
#endif
}

/**
 * @brief 	Stop generating timer signals.
 * @return	None
 */
void stm32_timer_stop(void)
{
#if (INTERFACE_MODE == SPI_DMA)
	sdesc = ad7091r_dev_desc->spi_desc->extra;
	TIM1->CR1 &= ~TIM_CR1_CEN;
	TIM2->CR1 &= ~TIM_CR1_CEN;

	TIM8->DIER &= ~TIM_DIER_CC1DE; // Disable Trigger timer CC DMA request

	/* Disable RX DMA */
	CLEAR_BIT(sdesc->hspi.Instance->CR2, SPI_CR2_RXDMAEN);
#endif
}

/**
 * @brief  Abort DMA Transfers
 * @return None
 */
void stm32_abort_dma_transfer(void)
{
#if (INTERFACE_MODE == SPI_DMA)
	int ret;
	sdesc = ad7091r_dev_desc->spi_desc->extra;

	ret = no_os_dma_xfer_abort(sdesc->dma_desc, sdesc->rxdma_ch);
	if (ret) {
		return ret;
	}

	ret = no_os_dma_xfer_abort(sdesc->dma_desc, sdesc->txdma_ch);
	if (ret) {
		return ret;
	}
#endif
}

/**
 * @brief 	Configures the chip select pin as output mode.
 * @param   is_gpio[in] Mode of the Pin
 * @return	None
 */
void stm32_cs_output_gpio_config(bool is_gpio)
{
#if (INTERFACE_MODE == SPI_DMA)
	struct no_os_gpio_desc* cs_gpio_desc;

	if (is_gpio) {
		cs_pwm_gpio_params.extra = &stm32_cs_gpio_extra_init_params;
	} else {
		cs_pwm_gpio_params.extra = &stm32_cs_pwm_gpio_extra_init_params;
	}

	no_os_gpio_get(&cs_gpio_desc, &cs_pwm_gpio_params);
#endif
}

#if (INTERFACE_MODE == SPI_DMA)
/**
 * @brief Callback function to flag the capture of number
 *        of requested samples.
 * @param hdma - DMA Handler (Unused)
 * @return	None
 */
void receivecomplete_callback(DMA_HandleTypeDef* hdma)
{
#if (DATA_CAPTURE_MODE == BURST_DATA_CAPTURE)
	if (!dma_cycle_count) {
		return;
	}

	/* Copy second half of the data to the IIO buffer */
	memcpy((void*)iio_buf_current_idx, dma_buf_current_idx, rxdma_ndtr / 2);

	dma_buf_current_idx = dma_buf_start_idx;
	iio_buf_current_idx += rxdma_ndtr / 2;

	/* Update samples captured so far */
	dma_cycle_count -= 1;

	/* If required cycles are done, stop timers and reset counters. */
	if (!dma_cycle_count) {
		TIM1->CR1 &= ~TIM_CR1_CEN;
		TIM2->CR1 &= ~TIM_CR1_CEN;
		TIM1->CNT = 0;
		TIM8->CNT = 0;

		ad7091r_conversion_flag = true;

		iio_buf_current_idx = iio_buf_start_idx;
		dma_buf_current_idx = dma_buf_start_idx;
	}

#else //CONTINUOUS_MODE
	no_os_cb_end_async_write(global_iio_dev_data->buffer->buf);
	no_os_cb_prepare_async_write(global_iio_dev_data->buffer->buf,
				     global_nb_of_samples * (BYTES_PER_SAMPLE), &buff_start_addr, &data_read);
#endif
}
#endif

/**
 * @brief Callback function to flag the capture of Half the number
 *        of requested samples.
 * @param hdma - DMA Handler (Unused)
 * @return	None
 */
void halfcmplt_callback(DMA_HandleTypeDef* hdma)
{
#if (INTERFACE_MODE == SPI_DMA)
#if (DATA_CAPTURE_MODE == BURST_DATA_CAPTURE)
	if (!dma_cycle_count) {
		return;
	}
#endif

	/* Copy first half of the data to the IIO buffer */
	memcpy((void*)iio_buf_current_idx, dma_buf_current_idx, rxdma_ndtr / 2);

	dma_buf_current_idx += rxdma_ndtr / 2;
	iio_buf_current_idx += rxdma_ndtr / 2;
#endif
}

/**
 * @brief Update buffer index
 * @param local_buf[out] - Local Buffer
 * @param buf_start_addr[out] - Buffer start addr
 * @return	None
 */
void update_buff(uint32_t* local_buf, uint32_t* buf_start_addr)
{
#if (INTERFACE_MODE == SPI_DMA)
	iio_buf_start_idx = (uint8_t*)buf_start_addr;
	dma_buf_start_idx = (uint8_t*)local_buf;

	iio_buf_current_idx = iio_buf_start_idx;
	dma_buf_current_idx = dma_buf_start_idx;
#endif
}

/**
 * @brief Pull the CONVST line down then up.
 * @return none.
 */
void ad7091r8_pulse_convst_stm(void)
{
	HAL_GPIO_WritePin(GPIOA, NO_OS_BIT(CNV_PIN), GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, NO_OS_BIT(CNV_PIN), GPIO_PIN_SET);
}

/**
 * @brief Read one sample.
 * @param channel[in] - Channel.
 * @param read_val[out] - Value.
 * @return 0 in case of success, negative error code otherwise.
 */
int ad7091r8_read_one_stm(uint8_t channel,
			  uint16_t* read_val)
{
	int ret;
	uint8_t adc_sample[2] = { 0 };

	/* Set channel to sequencer */
	ad7091r8_pulse_convst_stm();

	ret = ad7091r8_spi_reg_write(ad7091r_dev_desc, AD7091R8_REG_CHANNEL,
				     NO_OS_BIT(channel));

	ad7091r8_pulse_convst_stm();

	/* Perform single dummy spi read */
	ret = no_os_spi_write_and_read(ad7091r_dev_desc->spi_desc,
				       adc_sample,
				       BYTES_PER_SAMPLE);
	if (ret) {
		return -EIO;
	}

	ad7091r8_pulse_convst_stm();

	/* Read actual sample data over spi */
	ret = no_os_spi_write_and_read(ad7091r_dev_desc->spi_desc,
				       adc_sample,
				       BYTES_PER_SAMPLE);
	if (ret) {
		return -EIO;
	}

	*read_val = no_os_get_unaligned_be16(adc_sample);

	return 0;
}

/*!
 * @brief Prioritizes the UART1 interrupt over the other peripheral interrupts.
 * @return None
 */
void configure_intr_priority(void)
{
	for (int currIRQ = WWDG_IRQn; currIRQ <= DSI_IRQn; currIRQ++) {
		NVIC_SetPriority(currIRQ, 1);
	}

	NVIC_SetPriority(UART5_IRQn, 0);
}

