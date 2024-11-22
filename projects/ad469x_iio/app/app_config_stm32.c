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
#include "ad469x_iio.h"
#include "ad469x.h"
#include "no_os_pwm.h"

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

extern volatile bool ad469x_conversion_flag;

/******************************************************************************/
/******************** Variables and User Defined Data Types *******************/
/******************************************************************************/

/************************** Note **************************/
/* Most of the configurations specific to SPI_DMA implementation
 * such as clock configuration, timer master or slave mode,
 * has been done through the auto generated initialization code.
 * */

/* STM32 UART specific parameters */
struct stm32_uart_init_param stm32_uart_extra_init_params = {
	.huart = &huart5,
};

/* STM32 SPI specific parameters */
struct stm32_spi_init_param stm32_spi_extra_init_params = {
	.chip_select_port = SPI_CS_PORT_NUM,
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

/* STM32 GPIO specific parameters */
struct stm32_gpio_init_param stm32_gpio_reset_extra_init_params = {
	.mode = GPIO_MODE_OUTPUT_PP,
	.speed = GPIO_SPEED_FREQ_VERY_HIGH,
};

/* STM32 GPIO IRQ specific parameters */
struct stm32_gpio_irq_init_param stm32_gpio_irq_extra_init_params = {
	.port_nb = GP0_PORT_NUM, /* Port B */
};

/* STM32 PWM GPIO specific parameters */
struct stm32_gpio_init_param stm32_pwm_gpio_extra_init_params = {
	.mode = GPIO_MODE_AF_PP,
	.speed = GPIO_SPEED_FREQ_VERY_HIGH,
	.alternate = GPIO_AF1_TIM1
};

/* STM32 PWM for specific parameters */
struct stm32_pwm_init_param stm32_pwm_cnv_extra_init_params = {
	.prescaler = TIMER_1_PRESCALER,
	.timer_autoreload = true,
	.mode = TIM_OC_PWM1,
	.timer_chn = TIMER_CHANNEL_3,
	.complementary_channel = false,
	.get_timer_clock = HAL_RCC_GetPCLK2Freq,
	.clock_divider = TIMER_1_CLK_DIVIDER,
	.trigger_enable = false,
	.trigger_output = PWM_TRGO_OC1
};

#if (INTERFACE_MODE == SPI_DMA)
/* STM32 PWM specific parameters */
struct stm32_pwm_init_param stm32_cs_extra_init_params = {
	.prescaler = TIMER_2_PRESCALER,
	.timer_autoreload = false,
	.mode = TIM_OC_PWM1,
	.timer_chn = TIMER_CHANNEL_1,
	.complementary_channel = false,
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
	.repetitions = 1,
	.onepulse_enable = true,
	.dma_enable = true,
	.trigger_output = PWM_TRGO_RESET
};

/* STM32 Tx DMA channel extra init params */
struct stm32_dma_channel txdma_channel = {
	.hdma = &hdma_tim8_ch1,
	.ch_num = AD469x_TxDMA_CHANNEL_NUM,
	.mem_increment = false,
	.mem_data_alignment = DATA_ALIGN_BYTE,
	.per_data_alignment = DATA_ALIGN_BYTE,
	.dma_mode = DMA_CIRCULAR_MODE
};

/* STM32 Rx DMA channel extra init params */
struct stm32_dma_channel rxdma_channel = {
	.hdma = &hdma_spi1_rx,
	.ch_num = AD469x_RxDMA_CHANNEL_NUM,
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

/* STM32 VCOM init parameters */
struct stm32_usb_uart_init_param stm32_vcom_extra_init_params = {
	.husbdevice = &hUsbDeviceHS,
};

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
	MX_TIM8_Init();
#endif
	MX_GPIO_Init();
	MX_UART5_Init();
	MX_I2C1_Init();
	MX_TIM1_Init();
	MX_SPI1_Init();
#if (INTERFACE_MODE == SPI_INTERRUPT)
	HAL_NVIC_DisableIRQ(DMA2_Stream0_IRQn);
#else
	HAL_NVIC_DisableIRQ(EXTI9_5_IRQn);
#endif
#ifdef USE_VIRTUAL_COM_PORT
	MX_USB_DEVICE_Init();
#endif
}

/**
 * @brief 	Starts the timer signal generation for
 *          PWM and OC channels all at once.
 * @return	None
 */
void stm32_timer_enable(void)
{
#if (INTERFACE_MODE == SPI_DMA)
	sdesc = p_ad469x_dev->spi_desc->extra;

	TIM8->DIER |= TIM_DIER_CC1DE;
	no_os_pwm_enable(sdesc->pwm_desc); // CS PWM
	no_os_pwm_enable(pwm_desc); // CNV PWM
#endif
}

/**
 * @brief Disable PWM signals
 * @return None
 */
void stm32_timer_stop(void)
{
#if (INTERFACE_MODE == SPI_DMA)
	int ret;
	sdesc = p_ad469x_dev->spi_desc->extra;

	no_os_pwm_disable(pwm_desc); // CNV PWM
	no_os_pwm_disable(sdesc->pwm_desc); // CS PWM
	TIM8->DIER &= ~TIM_DIER_CC1DE;

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

	sdesc = p_ad469x_dev->spi_desc->extra;

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
 * @brief  Configures the chip select pin as output mode.
 * @param  is_gpio[in] Mode of the Pin
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


/**
 * @brief 	Configures the conversion pin as output mode.
 * @param is_gpio[in] Mode of the Pin
 * @return	None
 */
void stm32_cnv_output_gpio_config(bool is_gpio)
{
#if (INTERFACE_MODE == SPI_DMA)
	struct no_os_gpio_desc* cnv_gpio_desc;

	if (is_gpio) {
		pwm_gpio_params.extra = &stm32_gpio_cnv_extra_init_params;

	} else {
		pwm_gpio_params.extra = &stm32_pwm_gpio_extra_init_params;
	}
	no_os_gpio_get(&cnv_gpio_desc, &pwm_gpio_params);
#endif
}

/**
 * @brief   Callback function to flag the capture of number
 *          of requested samples.
 * @param hdma - DMA handler (Unused)
 * @return	None
 */

void receivecomplete_callback(DMA_HandleTypeDef* hdma)
{
#if (INTERFACE_MODE == SPI_DMA)
#if (DATA_CAPTURE_MODE == BURST_DATA_CAPTURE)
	sdesc = p_ad469x_dev->spi_desc->extra;

	/* Update samples captured so far */
	dma_cycle_count -= 1;

	if (!dma_cycle_count) {
		ad469x_conversion_flag = true;
		memcpy((void*)iio_buf_current_idx, dma_buf_current_idx, rxdma_ndtr / 2);

		iio_buf_current_idx = iio_buf_start_idx;
		dma_buf_current_idx = dma_buf_start_idx;
	} else {
		memcpy((void*)iio_buf_current_idx, dma_buf_current_idx, rxdma_ndtr / 2);

		dma_buf_current_idx = dma_buf_start_idx;
		iio_buf_current_idx += rxdma_ndtr / 2;

	}
	callback_count--;
#else
	no_os_cb_end_async_write(global_iio_dev_data->buffer->buf);
	no_os_cb_prepare_async_write(global_iio_dev_data->buffer->buf,
				     global_nb_of_samples  * (BYTES_PER_SAMPLE), &buff_start_addr, &data_read);
#endif // DATA_CAPTURE_MODE
#endif // INTERFACE_MODE
}

/**
 * @brief   Callback function to flag the capture of Half the number
 *          of requested samples.
 * @param hdma - DMA Handler (Unused)
 * @return	None
 */
void halfcmplt_callback(DMA_HandleTypeDef* hdma)
{
#if (INTERFACE_MODE == SPI_DMA)
	if (!dma_cycle_count) {
		return;
	}

	/* Copy first half of the data to the IIO buffer */
	memcpy((void*)iio_buf_current_idx, dma_buf_current_idx, rxdma_ndtr / 2);

	dma_buf_current_idx += rxdma_ndtr / 2;
	iio_buf_current_idx += rxdma_ndtr / 2;

	callback_count--;
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
 * @brief DMA2 Stream0 IRQ Handler
 * @param None
 * @return None
 */
void DMA2_Stream0_IRQHandler(void)
{
	/* Stop Tx trigger DMA and CNV timer at the last entry
	to the callback */
#if (DATA_CAPTURE_MODE == BURST_DATA_CAPTURE)
	if (callback_count == 1) {
		TIM8->DIER &= ~TIM_DIER_CC1DE;
		no_os_pwm_disable(pwm_desc);
		no_os_pwm_disable(sdesc->pwm_desc);
	}
#endif
	HAL_DMA_IRQHandler(&hdma_spi1_rx);
}