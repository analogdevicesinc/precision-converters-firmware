/***************************************************************************//**
 * @file    app_config_stm32.c
 * @brief   Source file for STM32 platform configurations
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

#include <stdbool.h>
#include "app_config_stm32.h"
#include "ad4692.h"
#include "app_config.h"
#include "no_os_error.h"
#include "ad4692_user_config.h"

/******************************************************************************/
/************************ Macros/Constants ************************************/
/******************************************************************************/

/******************************************************************************/
/******************** Variables and User Defined Data Types *******************/
/******************************************************************************/

/* STM32 SPI specific parameters */
struct stm32_spi_init_param stm32_spi_extra_init_params = {
	.chip_select_port = SPI_CS_PORT_NUM,
	.get_input_clock = HAL_RCC_GetPCLK2Freq,
	.dma_init = &ad4692_dma_init_param,
	.irq_num = Rx_DMA_IRQ_ID,
	.rxdma_ch = &rxdma_channel,
	.txdma_ch = &txdma_channel
};

/* STM32 GPIO Output parameters */
struct stm32_gpio_init_param stm32_gpio_output_extra_init_params = {
	.mode = GPIO_MODE_OUTPUT_PP,
	.speed = GPIO_SPEED_FREQ_VERY_HIGH,
};

/* STM32 GPIO Input parameters */
struct stm32_gpio_init_param stm32_gpio_input_extra_init_params = {
	.mode = GPIO_MODE_INPUT,
	.speed = GPIO_SPEED_FREQ_VERY_HIGH,
};

/* STM32 GPIO IRQ specific parameters */
struct stm32_gpio_irq_init_param stm32_gpio_irq_extra_init_params = {
	.port_nb = BSY_PORT_NUM,
};

/* STM32 PWM SPI Burst specific parameters */
struct stm32_pwm_init_param stm32_pwm_spi_burst_extra_init_params = {
	.htimer = &SPI_BURST_TIMER_HANDLE,
	.prescaler = TIMER_4_PRESCALER,
	.timer_autoreload = true,
	.mode = TIM_OC_PWM1,
	.timer_chn = TIMER_CHANNEL_1,
	.complementary_channel = false,
	.get_timer_clock = HAL_RCC_GetPCLK2Freq,
	.clock_divider = TIMER_4_CLK_DIVIDER
};

/* STM32 CNV PWM GPIO specific parameters */
struct stm32_gpio_init_param stm32_spi_burst_pwm_gpio_extra_init_params = {
	.mode = GPIO_MODE_AF_PP,
	.speed = GPIO_SPEED_FREQ_VERY_HIGH,
	.alternate = GPIO_AF2_TIM4
};

/* STM32 PWM CNV specific parameters */
struct stm32_pwm_init_param stm32_pwm_cnv_extra_init_params = {
	.htimer = &CNV_TIMER_HANDLE,
	.prescaler = TIMER_1_PRESCALER,
	.timer_autoreload = true,
	.mode = TIM_OC_PWM1,
	.timer_chn = TIMER_CHANNEL_3,
	.complementary_channel = false,
	.get_timer_clock = HAL_RCC_GetPCLK2Freq,
	.clock_divider = TIMER_1_CLK_DIVIDER
};

/* STM32 CNV PWM GPIO specific parameters */
struct stm32_gpio_init_param stm32_cnv_pwm_gpio_extra_init_params = {
	.mode = GPIO_MODE_AF_PP,
	.speed = GPIO_SPEED_FREQ_VERY_HIGH,
	.alternate = GPIO_AF1_TIM1
};

/* STM32 I2C specific parameters */
struct stm32_i2c_init_param stm32_i2c_extra_init_params = {
	.i2c_timing = I2C_TIMING
};

/* STM32 Tx DMA channel extra init params */
struct stm32_dma_channel txdma_channel = {
	.hdma = &hdma_tim8_ch1,
	.ch_num = AD4692_TxDMA_CHANNEL_NUM,
	.mem_increment = true,
	.mem_data_alignment = DATA_ALIGN_BYTE,
	.per_data_alignment = DATA_ALIGN_BYTE,
	.dma_mode = DMA_CIRCULAR_MODE
};

/* STM32 Rx DMA channel extra init params */
struct stm32_dma_channel rxdma_channel = {
	.hdma = &hdma_spi1_rx,
	.ch_num = AD4692_RxDMA_CHANNEL_NUM,
	.mem_increment = true,
	.mem_data_alignment = DATA_ALIGN_BYTE,
	.per_data_alignment = DATA_ALIGN_BYTE,
	.dma_mode = DMA_CIRCULAR_MODE,
};

/* STM32 PWM specific init params */
struct stm32_pwm_init_param stm32_tx_trigger_extra_init_params = {
	.htimer = &TX_TRIGGER_TIMER_HANDLE,
	.prescaler = TX_TRIGGER_PRESCALER,
	.timer_autoreload = true,
	.mode = TIM_OC_PWM1,
	.timer_chn = TIMER_CHANNEL_1,
	.complementary_channel = false,
	.get_timer_clock = HAL_RCC_GetPCLK1Freq,
	.clock_divider = TX_TRIGGER_CLK_DIVIDER,
	.slave_mode = STM32_PWM_SM_TRIGGER,
	.trigger_source = PWM_TS_ETR,
	.trigger_polarity = PWM_TRIG_POL_FALLING,
	.trigger_output = PWM_TRGO_UPDATE,
	.dma_enable = true,
	.repetitions = BYTES_PER_SAMPLE - 1,
	.onepulse_enable = true
};

/* STM32 VCOM init parameters */
struct stm32_usb_uart_init_param stm32_vcom_extra_init_params = {
	.husbdevice = &hUsbDeviceHS,
};
/* STM32 UART specific parameters */
struct stm32_uart_init_param stm32_uart_extra_init_params = {
	.huart = &huart5,
};

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

/* STM32 SPI desc */
struct stm32_spi_desc* sdesc;

/******************************************************************************/
/************************** Functions Declaration *****************************/
/******************************************************************************/

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

	MX_GPIO_Init();
	MX_UART5_Init();
	MX_I2C1_Init();
	MX_SPI1_Init();
	MX_DMA_Init();

	MX_TIM1_Init();
	MX_TIM4_Init();
	MX_TIM8_Init();

#ifdef USE_VIRTUAL_COM_PORT
	MX_USB_DEVICE_Init();
#endif
}
/**
 * @brief Configure and start PWM
 * @param desc[in] - Device descriptor
 * @return 0 in case of success, negative error code otherwise.
 */
int ad4692_config_and_start_pwm(struct ad4692_desc *desc)
{
	int ret;

	if (!desc) {
		return -EINVAL;
	}

	/* Enable Tx Trigger DMA */
	TIM8->DIER |= TIM_DIER_CC1DE;

	/* Enable CNV PWM for any mode other than SPI Burst */
	if (ad4692_init_params.mode != AD4692_SPI_BURST) {
		ret = no_os_pwm_enable(ad4692_dev->conv_desc);
		if (ret) {
			return ret;
		}
	} else {
		ret = no_os_pwm_enable(spi_burst_pwm_desc);
		if (ret) {
			return ret;
		}
	}

	return 0;
}

/**
 * @brief Stop timers and DMA transfers
 * @return None
 */
void ad4692_stop_timer(void)
{
	if (ad4692_interface_mode == SPI_DMA) {
		/* Disable Tx Trigger DMA */
		TIM8->DIER &= ~TIM_DIER_CC1DE;
	}

	/* Disable CNV PWM for any mode other than SPI Burst */
	if (ad4692_init_params.mode != AD4692_SPI_BURST) {
		no_os_pwm_disable(ad4692_dev->conv_desc);
	} else {
		no_os_pwm_disable(spi_burst_pwm_desc);
	}

	return;
}

/**
 * @brief  Abort DMA Transfers
 * @return None
 */
void stm32_abort_dma_transfer(void)
{
	sdesc = ad4692_dev->comm_desc->extra;

	no_os_dma_xfer_abort(sdesc->dma_desc, sdesc->rxdma_ch);
	no_os_dma_xfer_abort(sdesc->dma_desc, sdesc->txdma_ch);

	return;
}

/**
 * @brief Update buffer index
 * @param local_buf[in] - Local Buffer
 * @param buf_start_addr[in] - Buffer start addr
 * @return None
 */
void update_buff(uint8_t *local_buf, uint8_t *buf_start_addr)
{
	iio_buf_start_idx = (uint8_t *)buf_start_addr - (N_CYCLE_OFFSET *
			    BYTES_PER_SAMPLE);
	dma_buf_start_idx = (uint8_t *)local_buf;

	iio_buf_current_idx = iio_buf_start_idx ;
	dma_buf_current_idx = dma_buf_start_idx;
}

/**
 * @brief   Callback function to flag the capture of number
 *          of requested samples.
 * @param hdma - DMA handler (Unused)
 * @return None
 */
void ad4692_spi_dma_rx_half_cplt_callback(DMA_HandleTypeDef* hdma)
{
	/* Copy first half of the data to the IIO buffer */
	memcpy((void*)iio_buf_current_idx,
	       dma_buf_current_idx, rxdma_ndtr / 2);

	dma_buf_current_idx += (rxdma_ndtr / 2);
	iio_buf_current_idx += rxdma_ndtr / 2;

	callback_count--;
}

/**
 * @brief   Callback function to flag the capture of number
 *          of requested samples.
 * @param hdma - DMA handler (Unused)
 * @return None
 */
void ad4692_spi_dma_rx_cplt_callback(DMA_HandleTypeDef* hdma)
{
	if (ad4692_data_capture_mode == BURST) {
		/* Update samples captured so far */
		dma_cycle_count -= 1;

		if (!dma_cycle_count) {
			ad4692_dma_buff_full = true;
			memcpy((void*)iio_buf_current_idx, dma_buf_current_idx, rxdma_ndtr / 2);

			iio_buf_current_idx = iio_buf_start_idx;
			dma_buf_current_idx = dma_buf_start_idx;
		} else {
			memcpy((void*)iio_buf_current_idx, dma_buf_current_idx, rxdma_ndtr / 2);

			dma_buf_current_idx = dma_buf_start_idx;
			iio_buf_current_idx += rxdma_ndtr / 2;
		}

		/* Update the callback count */
		callback_count--;
	} else { // CONTINUOUS_DATA_CAPTURE
		no_os_cb_end_async_write(iio_dev_data_g->buffer->buf);
		no_os_cb_prepare_async_write(iio_dev_data_g->buffer->buf,
					     nb_of_samples_g,
					     (void **) &buff_start_addr,
					     &data_read);
	}
}

/**
 * @brief  DMA2 Stream0 IRQ Handler
 * @param None
 * @return None
 */
void DMA2_Stream0_IRQHandler(void)
{
	if (ad4692_data_capture_mode == BURST) {
		/* Stop Tx trigger DMA and CNV timer at the last entry
		to the callback */
		if (callback_count == 1) {
			TIM8->DIER &= ~TIM_DIER_CC1DE;
		}
	} else { // CONTINUOUS
		memcpy(iio_buf_current_idx,
		       dma_buf_current_idx, rxdma_ndtr);
	}

	HAL_DMA_IRQHandler(&hdma_spi1_rx);
}

/**
 * @brief  GPIO IRQ handler for the interrupt trigger
 * @return None
 */
void EXTI0_IRQHandler(void)
{
	HAL_GPIO_EXTI_IRQHandler(1 << BSY_PIN_NUM);
}

/**
 * @brief  GPIO15-10 IRQ handler for the interrupt trigger
 * @return None
 */
void EXTI15_10_IRQHandler(void)
{
	HAL_GPIO_EXTI_IRQHandler(1 << SPI_BURST_PWM_ID);
}

/**
 * @brief Initialize Timer 4
 * @return None
 */
void stm32_tim4_init(void)
{
	MX_TIM4_Init();
}
