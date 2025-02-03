/***************************************************************************//**
 * @file    app_config_stm32.c
 * @brief   Source file for STM32 platform configurations
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
#include <stdbool.h>
#include "app_config.h"
#include "app_config_stm32.h"
#include "ad405x_iio.h"
#include "ad405x.h"
#include "iio.h"

/******************************************************************************/
/************************ Macros/Constants ************************************/
/******************************************************************************/

/******************************************************************************/
/******************** Variables and User Defined Data Types *******************/
/******************************************************************************/
/* STM32 Tx DMA channel extra init params */
struct stm32_dma_channel txdma_channel = {
	.hdma = &hdma_tim8_ch1,
	.ch_num = AD405x_TxDMA_CHANNEL_NUM,
	.mem_increment = false,
	.mem_data_alignment = DATA_ALIGN_BYTE,
	.per_data_alignment = DATA_ALIGN_BYTE,
	.dma_mode = DMA_CIRCULAR_MODE
};

/* STM32 Rx DMA channel extra init params */
struct stm32_dma_channel rxdma_channel = {
	.hdma = &hdma_spi1_rx,
	.ch_num = AD405x_RxDMA_CHANNEL_NUM,
	.mem_increment = true,
	.mem_data_alignment = DATA_ALIGN_HALF_WORD,//DATA_ALIGN_BYTE,
	.per_data_alignment = DATA_ALIGN_HALF_WORD,//DATA_ALIGN_BYTE,
	.dma_mode = DMA_CIRCULAR_MODE,
};

/* STM32 UART specific parameters */
struct stm32_uart_init_param stm32_uart_extra_init_params = {
	.huart = &huart5,
};

/* STM32 VCOM init parameters */
struct stm32_usb_uart_init_param stm32_vcom_extra_init_params = {
	.husbdevice = &hUsbDeviceHS,
};

/* STM32 SPI specific parameters */
struct stm32_spi_init_param stm32_spi_extra_init_params = {
	.chip_select_port = STM32_SPI_CS_PORT_NUM,
	.get_input_clock = HAL_RCC_GetPCLK2Freq,
	.alternate = GPIO_AF1_TIM2
};

/* STM32 GPIO specific parameters */
struct stm32_gpio_init_param stm32_gpio_cnv_extra_init_params = {
	.mode = GPIO_MODE_OUTPUT_PP,
	.speed = GPIO_SPEED_FREQ_VERY_HIGH,
};

/* STM32 GPIO specific parameters */
struct stm32_gpio_init_param stm32_gpio_gp0_extra_init_params = {
	.mode = GPIO_MODE_INPUT,
	.speed = GPIO_SPEED_FREQ_VERY_HIGH,
};

/* STM32 GPIO specific parameters */
struct stm32_gpio_init_param stm32_gpio_gp1_extra_init_params = {
	.mode = GPIO_MODE_INPUT,
	.speed = GPIO_SPEED_FREQ_VERY_HIGH,
};

/* STM32 GPIO IRQ specific parameters */
struct stm32_gpio_irq_init_param stm32_gpio_irq_extra_init_params = {
	.port_nb = GP1_PORT_NUM, /* Port G */
};

/************************** Note **************************/
/* Most of the configurations specific to SPI_DMA implementation
 * such as clock configuration, timer master or slave mode,
 * has been done through the auto generated initialization code.
 * */

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

/* STM32 PWM GPIO specific parameters */
struct stm32_gpio_init_param stm32_pwm_gpio_extra_init_params = {
	.mode = GPIO_MODE_AF_PP,
	.speed = GPIO_SPEED_FREQ_VERY_HIGH,
	.alternate = GPIO_AF1_TIM1
};

/* STM32 PWM for specific parameters for generating conversion pulses
 * in PWM 1 mode as well for triggering SPI DMA transaction of
 * higher byte of 16-bit data.
 * */
struct stm32_pwm_init_param stm32_pwm_cnv_extra_init_params = {
	.prescaler = TIMER_1_PRESCALER,
	.timer_autoreload = true,
	.mode = TIM_OC_PWM1,
	.timer_chn = TIMER_CHANNEL_3,
	.get_timer_clock = HAL_RCC_GetPCLK2Freq,
	.clock_divider = TIMER_1_CLK_DIVIDER,
	.trigger_enable = false,
	.trigger_output = PWM_TRGO_UPDATE
};

#if (INTERFACE_MODE == SPI_DMA)
/* STM32 PWM for specific parameters for generating the
 * the chip select signals in PWM Mode 2.
 * */
struct stm32_pwm_init_param stm32_cs_extra_init_params = {
	.prescaler = TIMER_2_PRESCALER,
	.timer_autoreload = false,
	.mode = TIM_OC_PWM2,
	.timer_chn = TIMER_CHANNEL_1,
	.get_timer_clock = HAL_RCC_GetPCLK1Freq,
	.clock_divider = TIMER_2_CLK_DIVIDER
};

/* STM32 PWM specific init params */
struct stm32_pwm_init_param stm32_tx_trigger_extra_init_params = {
	.prescaler = TIMER_8_PRESCALER,
	.timer_autoreload = true,
	.mode = TIM_OC_TOGGLE,
	.timer_chn = TIMER_CHANNEL_1,
	.complementary_channel = false,
	.get_timer_clock = HAL_RCC_GetPCLK1Freq,
	.clock_divider = TIMER_8_CLK_DIVIDER,
	.trigger_enable = true,
	.trigger_source = PWM_TS_ITR0,
	.repetitions = 0,
	.onepulse_enable = true,
	.dma_enable = true,
};
#endif

/* STM32 SPI Descriptor*/
volatile struct stm32_spi_desc* sdesc;

/* Flag to indicate whether conversion and acquisition of requested
 * samples is over. */
volatile bool ad405x_conversion_flag = false;

#if (INTERFACE_MODE == SPI_DMA)
/* Number of times the DMA complete callback needs to be invoked for
 * capturing the desired number of samples*/
int dma_cycle_count = 0;

/* Global variable for callback count */
uint32_t callback_count;

/* The number of transactions requested for the RX DMA stream */
uint32_t rxdma_ndtr;

/* Pointer to start of the IIO buffer */
uint8_t *iio_buf_start_idx;

/* Pointer to start of the local SRAM buffer
 * used by RXDMA to put data directly in. */
uint8_t *dma_buf_start_idx;

/* Pointer to the current location being written to, in the IIO buffer */
uint8_t *iio_buf_current_idx;

/* Pointer to the current location being written to, by the DMA */
uint8_t *dma_buf_current_idx;
#endif

/******************************************************************************/
/************************** Functions Declaration *****************************/
/******************************************************************************/
void receivecomplete_callback(DMA_HandleTypeDef * hdma);
/******************************************************************************/
/************************** Functions Definition ******************************/
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
	/* If SPI_DMA mode is enabled, we don't need DMA initialization
	 * and Timer 2 initialization which is used for generating the
	 * chip select signals.
	 */
	MX_DMA_Init();
	MX_TIM2_Init();
#endif
	MX_GPIO_Init();
	MX_UART5_Init();
	MX_I2C1_Init();
	MX_TIM1_Init();
	MX_TIM8_Init();
	MX_SPI1_Init();
	MX_USB_DEVICE_Init();
	HAL_NVIC_DisableIRQ(STM32_DMA_CONT_TRIGGER);
#if (INTERFACE_MODE == SPI_INTERRUPT)
	HAL_NVIC_DisableIRQ(STM32_DMA_SPI_RX_TRIGGER);
	HAL_NVIC_EnableIRQ(STM32_GP1_IRQ);
	/* The channel 1 of timer 1 is configured in output compare mode
	 * to trigger the chip select signal generation in SPI_DMA mode.
	 * However, this should be de-initialized in SPI_INTERRUPT mode
	 * to avoid influence of output compare on the chip select generation.
	 */
	HAL_TIM_OC_DeInit(&htim1);
#else
#if (APP_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
	HAL_NVIC_DisableIRQ(STM32_DMA_SPI_RX_TRIGGER);
#else
	HAL_DMA_RegisterCallback(&hdma_spi1_rx, HAL_DMA_XFER_CPLT_CB_ID,
				 receivecomplete_callback);
#endif
	HAL_NVIC_DisableIRQ(STM32_GP1_IRQ);
#endif
}

#if (INTERFACE_MODE == SPI_DMA)
/**
  * @brief 	IRQ handler for RX DMA channel
  * @return None
  */
void DMA2_Stream0_IRQHandler(void)
{
#if (APP_CAPTURE_MODE == WINDOWED_DATA_CAPTURE)
	/* Stop timers at the last entry to the callback */
	if (callback_count == 1) {
		sdesc = p_ad405x_dev->spi_desc->extra;

		TIM8->DIER &= ~TIM_DIER_CC1DE;
		no_os_pwm_disable(pwm_desc);
		no_os_pwm_disable(sdesc->pwm_desc);
	}
#endif
	HAL_DMA_IRQHandler(&hdma_spi1_rx);
}

/**
 * @brief   Callback function to flag the capture of half the number
 *          of requested samples.
 * @param hdma - DMA Handler (Unused)
 * @return	None
 */
void halfcmplt_callback(DMA_HandleTypeDef * hdma)
{
	if (!dma_cycle_count) {
		return;
	}

	/* Copy first half of the data to the IIO buffer */
	memcpy((void *)iio_buf_current_idx, dma_buf_current_idx, rxdma_ndtr);

	dma_buf_current_idx += rxdma_ndtr;
	iio_buf_current_idx += rxdma_ndtr;

	callback_count--;
}

/**
 * @brief Update buffer index
 * @param local_buf[out] - Local Buffer
 * @param buf_start_addr[out] - Buffer start addr
 * @return	None
 */
void update_buff(uint32_t* local_buf, uint32_t* buf_start_addr)
{
	iio_buf_start_idx = (uint8_t*)buf_start_addr;
	dma_buf_start_idx = (uint8_t*)local_buf;

	iio_buf_current_idx = iio_buf_start_idx;
	dma_buf_current_idx = dma_buf_start_idx;
}

/**
 * @brief 	Starts the timer signal generation for
 *          PWM and OC channels all at once.
 * @return	None
 */
void stm32_timer_enable(void)
{
	/* Enable TIM DMA request */
	no_os_pwm_enable(tx_trigger_desc);

	/* Enable timers 1 and 2 */
	TIM1->CCER |= TIM_CCER_CC3E;
	TIM2->CCER |= TIM_CCER_CC1E;

	TIM1->BDTR |= TIM_BDTR_MOE;
	TIM2->BDTR |= TIM_BDTR_MOE;

	/* Start CS PWM before CNV PWM */
	TIM2->CR1 |= TIM_CR1_CEN;
	TIM1->CR1 |= TIM_CR1_CEN;
}

/**
 * @brief 	Stops generating timer signals.
 * @return	None
 */
void stm32_timer_stop(void)
{
	int ret;
	sdesc = p_ad405x_dev->spi_desc->extra;

	TIM1->CR1 &= ~TIM_CR1_CEN;
	TIM2->CR1 &= ~TIM_CR1_CEN;

	TIM8->DIER &= ~TIM_DIER_CC1DE;

	/* Disable RX DMA */
	CLEAR_BIT(sdesc->hspi.Instance->CR2, SPI_CR2_RXDMAEN);
}

/**
 * @brief   Callback function to flag the capture of number
 *          of requested samples.
 * @param hdma - DMA handler (Unused)
 * @return	None
 */

void receivecomplete_callback(DMA_HandleTypeDef * hdma)
{
#if (APP_CAPTURE_MODE == WINDOWED_DATA_CAPTURE)
	if (!dma_cycle_count) {
		return;
	}

	/* Copy second half of the data to the IIO buffer */
	memcpy((void *)iio_buf_current_idx, dma_buf_current_idx, rxdma_ndtr);

	dma_buf_current_idx = dma_buf_start_idx;
	iio_buf_current_idx += rxdma_ndtr;

	/* Update samples captured so far */
	dma_cycle_count--;
	callback_count--;

	/* If required cycles are done, stop timers and reset counters. */
	if (!dma_cycle_count) {
		TIM2->CR1 &= ~1;
		TIM1->CR1 &= ~1;
		TIM1->CNT = 0;
		TIM2->CNT = 0;
		TIM8->CNT = 0;

		data_ready = true;
		ad405x_conversion_flag = true;

		iio_buf_current_idx = iio_buf_start_idx;
		dma_buf_current_idx = dma_buf_start_idx;
	}

	return;
#else
	no_os_cb_end_async_write(iio_dev_data_g->buffer->buf);
	no_os_cb_prepare_async_write(iio_dev_data_g->buffer->buf,
				     nb_of_bytes_g,
				     &buff_start_addr,
				     &data_read);
#endif
}

/**
 * @brief 	Configures the chip select pin as output mode.
 * @param   is_gpio[in] - Mode of the Pin
 * @return	None
 */
void stm32_cs_output_gpio_config(bool is_gpio)
{
	struct no_os_gpio_desc *cs_gpio_desc;

	if (is_gpio) {
		cs_pwm_gpio_params.extra = &stm32_cs_gpio_extra_init_params;
	} else {
		cs_pwm_gpio_params.extra = &stm32_cs_pwm_gpio_extra_init_params;
	}

	no_os_gpio_get(&cs_gpio_desc, &cs_pwm_gpio_params);

}

/**
 * @brief 	Configures the conversion pin as output mode.
 * @param   is_gpio[in] Mode of the Pin
 * @return	None
 */
void stm32_cnv_output_gpio_config(bool is_gpio)
{
	struct no_os_gpio_desc* cnv_gpio_desc;

	if (is_gpio) {
		pwm_gpio_params.extra = &stm32_gpio_cnv_extra_init_params;

	} else {
		pwm_gpio_params.extra = &stm32_pwm_gpio_extra_init_params;
	}
	no_os_gpio_get(&cnv_gpio_desc, &pwm_gpio_params);
}

/**
 * @brief 	Abort ongoing SPI RX DMA transfer.
 * @return	None
 */
int stm32_abort_dma_transfer(void)
{
	int ret;

	sdesc = p_ad405x_dev->spi_desc->extra;

	ret = no_os_dma_xfer_abort(sdesc->dma_desc, sdesc->rxdma_ch);
	if (ret) {
		return ret;
	}

	ret = no_os_dma_xfer_abort(sdesc->dma_desc, sdesc->txdma_ch);
	if (ret) {
		return ret;
	}

	return 0;
}
#endif