/***************************************************************************//**
 *   @file    app_config_stm32.c
 *   @brief   Application configurations module for STM32 platform
********************************************************************************
 * Copyright (c) 2023-24 Analog Devices, Inc.
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
#include "app_config.h"
#if (INTERFACE_MODE == TDM_MODE)
#include "stm32_tdm_support.h"
#endif
#include "ad4170_iio.h"
#include "no_os_pwm.h"

/******************************************************************************/
/************************ Macros/Constants ************************************/
/******************************************************************************/

/******************************************************************************/
/******************** Variables and User Defined Data Types *******************/
/******************************************************************************/

/* Count to track the number of entries into the callback functions */
volatile uint32_t callback_count = 0;

/**
 * @brief 	Return the peripheral frequency
 * @return	Peripheral frequency in Hz
 */
uint32_t HAL_RCC_GetSysClockFreq_app()
{
	return HAL_RCC_GetPCLK2Freq();
}

/* SPI STM32 Platform Specific Init Parameters */
struct stm32_spi_init_param stm32_spi_extra_init_params = {
	.chip_select_port = STM32_SPI_CS_PORT,
	.get_input_clock = HAL_RCC_GetSysClockFreq_app
};

/* UART STM32 Platform Specific Init Parameters */
struct stm32_uart_init_param stm32_uart_extra_init_params = {
	.huart = APP_UART_HANDLE
};

/* STM32 GPIO specific parameters */
struct stm32_gpio_init_param stm32_trigger_gpio_extra_init_params = {
	.mode = GPIO_MODE_INPUT,
	.speed = GPIO_SPEED_FREQ_VERY_HIGH,
};

/* STM32 GPIO specific parameters */
struct stm32_gpio_init_param stm32_dig_aux1_gpio_extra_init_params = {
	.mode = GPIO_MODE_INPUT,
	.speed = GPIO_SPEED_FREQ_VERY_HIGH,
};

/* STM32 GPIO specific parameters */
struct stm32_gpio_init_param stm32_dig_aux2_gpio_extra_init_params = {
	.mode = GPIO_MODE_INPUT,
	.speed = GPIO_SPEED_FREQ_VERY_HIGH,
};

/* STM32 GPIO specific parameters */
struct stm32_gpio_init_param stm32_sync_inb_gpio_extra_init_params = {
	.mode = GPIO_MODE_OUTPUT_PP,
	.speed = GPIO_SPEED_FREQ_VERY_HIGH,
};

#if (INTERFACE_MODE != SPI_DMA_MODE)
/* STM32 GPIO IRQ specific parameters */
struct stm32_gpio_irq_init_param stm32_trigger_gpio_irq_init_params = {
	.port_nb = GPIO_TRIGGER_INT_PORT
};
#endif

#if (INTERFACE_MODE == TDM_MODE)
/* SAI-TDM STM32 PLatform Specific Init Parameters */
struct stm32_tdm_init_param stm32_tdm_extra_init_params = {
	.base = STM32_SAI_BASE,
};
#endif

/* STM32 GPIO specific parameters */
struct stm32_gpio_init_param stm32_csb_gpio_extra_init_params = {
	.mode = GPIO_MODE_OUTPUT_PP,
	.speed = GPIO_SPEED_FREQ_VERY_HIGH,
};

/* STM32 I2C specific parameters */
struct stm32_i2c_init_param stm32_i2c_extra_init_params = {
	.i2c_timing = I2C_TIMING
};

#if defined (TARGET_SDP_K1)
/* STM32 VCOM init parameters */
struct stm32_usb_uart_init_param stm32_vcom_extra_init_params = {
	.husbdevice = &hUsbDeviceHS,
};
#endif

#if (INTERFACE_MODE == SPI_DMA_MODE)
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

/* STM32 PWM specific init params */
struct stm32_pwm_init_param stm32_tx_trigger_extra_init_params = {
	.prescaler = TIMER_8_PRESCALER,
	.timer_autoreload = true,
	.mode = TIM_OC_PWM1,
	.timer_chn = TIMER_CHANNEL_1,
	.complementary_channel = false,
	.get_timer_clock = HAL_RCC_GetPCLK1Freq,
	.clock_divider = TIMER_8_CLK_DIVIDER,
	.trigger_output = PWM_TRGO_UPDATE,
	.dma_enable = true,
	.repetitions = BYTES_PER_SAMPLE - 1,
	.onepulse_enable = true
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
	MX_SPI1_Init();
	MX_GPIO_Init();
	MX_SAI1_Init();
#if !defined (TARGET_SDP_K1)
	MX_USART3_UART_Init();
	MX_GPDMA1_Init();
	MX_ICACHE_Init();
#else
	MX_UART5_Init();
	MX_DMA_Init();
	MX_TIM8_Init();
#ifdef USE_VIRTUAL_COM_PORT
	MX_USB_DEVICE_Init();
#endif
#endif
}

/*!
 * @brief SAI DMA Receive Half Complete Callback function
 * @param hsai - pointer to a SAI_HandleTypeDef structure
 * @return None
 */
void ad4170_dma_rx_half_cplt(SAI_HandleTypeDef *hsai)
{
#if (INTERFACE_MODE == TDM_MODE)
#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
	if ((tdm_read_started) && (data_capture_operation)) {
		end_tdm_dma_to_cb_transfer(ad4170_tdm_desc, ad4170_iio_dev_data,
					   TDM_DMA_READ_SIZE, BYTES_PER_SAMPLE);
	}
#endif
#endif // INTERFACE_MODE
}

/*!
 * @brief SAI DMA Receive Complete Callback function
 * @param hsai - pointer to a SAI_HandleTypeDef structure
 * @return None
 */
void ad4170_dma_rx_cplt(SAI_HandleTypeDef *hsai)
{
#if (INTERFACE_MODE == TDM_MODE)
	if (data_capture_operation) {
#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
		/* TDM read is not invoked in-time to read the first channel in sequencer
		 * due to higher execution time of MCU, which results into missing of
		 * first sample. So ignoring the first (num_of_active_channels-1) samples
		 * before filling up the buffer */
		if (!tdm_read_started) {
			/* Start TDM DMA read as the peripheral is disabled in Normal(Linear)
			 * Buffer Mode upon buffer completion */
			no_os_tdm_read(ad4170_tdm_desc, dma_buff, TDM_DMA_READ_SIZE << 1);
			tdm_read_started = true;
		} else {
			end_tdm_dma_to_cb_transfer(ad4170_tdm_desc, ad4170_iio_dev_data,
						   TDM_DMA_READ_SIZE, BYTES_PER_SAMPLE);
			/* Start TDM DMA read as the peripheral is disabled in Normal(Linear)
			 * Buffer Mode upon buffer completion */
			no_os_tdm_read(ad4170_tdm_desc, dma_buff, TDM_DMA_READ_SIZE << 1);
		}
#else
		if (!tdm_read_started) {
			no_os_tdm_read(ad4170_tdm_desc, dma_buff,
				       ad4170_iio_dev_data->buffer->samples * num_of_active_channels);
			tdm_read_started = true;
		} else {
			update_dma_buffer_overflow();
		}
#endif
	} else {
		update_dma_buffer_overflow();
	}
#endif // INTERFACE_MODE
}

/**
 * @brief   Callback function to flag the capture of number
 *          of requested samples.
 * @param hdma - DMA handler (Unused)
 * @return None
 */
void ad4170_spi_dma_rx_cplt_callback(DMA_HandleTypeDef* hdma)
{
	callback_count--;

#if (INTERFACE_MODE == SPI_DMA_MODE)
#if (DATA_CAPTURE_MODE == BURST_DATA_CAPTURE)
	/* Update samples captured so far */
	dma_cycle_count -= 1;

	if (!dma_cycle_count) {
		memcpy((void*)iio_buf_current_idx, dma_buf_current_idx, rxdma_ndtr / 2);

		ad4170_dma_buff_full = true;
		iio_buf_current_idx = iio_buf_start_idx;
		dma_buf_current_idx = dma_buf_start_idx;
	} else {
		memcpy((void*)iio_buf_current_idx, dma_buf_current_idx, rxdma_ndtr / 2);

		dma_buf_current_idx = dma_buf_start_idx;
		iio_buf_current_idx += rxdma_ndtr / 2;
	}

#else // CONTUNUOUS_DATA_CAPTURE
	no_os_cb_end_async_write(iio_dev_data_g->buffer->buf);
	no_os_cb_prepare_async_write(iio_dev_data_g->buffer->buf,
				     nb_of_samples_g * (BYTES_PER_SAMPLE), &buff_start_addr, &data_read);
#endif // DATA_CAPTURE_MODE
#endif // INTERFACE_MODE
}

/**
 * @brief   Callback function to flag the capture of Half the number
 *          of requested samples.
 * @param hdma - DMA Handler (Unused)
 * @return None
 */
void ad4170_spi_dma_rx_half_cplt_callback(DMA_HandleTypeDef* hdma)
{
#if (INTERFACE_MODE == SPI_DMA_MODE)
	/* Copy first half of the data to the IIO buffer */
	memcpy((void*)iio_buf_current_idx, dma_buf_current_idx, rxdma_ndtr / 2);

	dma_buf_current_idx += rxdma_ndtr / 2;
	iio_buf_current_idx += rxdma_ndtr / 2;
#endif

	callback_count--;
}

/**
 * @brief Update buffer index
 * @param local_buf[out] - Local Buffer
 * @param buf_start_addr[out] - Buffer start addr
 * @return None
 */
void update_buff(uint32_t* local_buf, uint32_t* buf_start_addr)
{
#if (INTERFACE_MODE == SPI_DMA_MODE)
	iio_buf_start_idx = (uint8_t*)buf_start_addr;
	dma_buf_start_idx = (uint8_t*)local_buf;

	iio_buf_current_idx = iio_buf_start_idx;
	dma_buf_current_idx = dma_buf_start_idx;
#endif
}

/**
 * @brief Configure Tx Trigger timer
 * @return None
 */
void tim8_config(void)
{
#if (INTERFACE_MODE == SPI_DMA_MODE)
	TIM8->DIER |= TIM_DIER_CC1DE; // Enable CC1 DMA
#endif
}

/**
 * @brief Disable Timer signals
 * @return None
 */
void stm32_timer_stop(void)
{
#if (INTERFACE_MODE == SPI_DMA_MODE)
	int ret;
	sdesc = p_ad4170_dev_inst->spi_desc->extra;

	/* Disable Tx Trigger DMA */
	TIM8->DIER &= ~TIM_DIER_CC1DE;

	/* Reset Timer count */
	TIM8->CNT = 0;

	/* Set SYNC Low to Stop conversion */
	ret = no_os_gpio_set_value(p_ad4170_dev_inst->gpio_sync_inb, NO_OS_GPIO_LOW);
	if (ret) {
		return ret;
	}

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
#if (INTERFACE_MODE == SPI_DMA_MODE)
	int ret;

	sdesc = p_ad4170_dev_inst->spi_desc->extra;

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
 * @brief  Initialize Tx trigger advanced PWM parameters
 * @param pwm_desc[in, out]
 * @return None
 */
void tim8_init(struct no_os_pwm_desc *pwm_desc)
{
#if (INTERFACE_MODE == SPI_DMA_MODE)
	if (!pwm_desc) {
		return -EINVAL;
	}

	struct stm32_pwm_desc *spwm_desc = pwm_desc->extra;

	TIM8->SMCR = TIM_SMCR_ETP | TIM_MASTERSLAVEMODE_ENABLE | TIM_SLAVEMODE_TRIGGER |
		     TIM_TS_ETRF;
#endif
}

void DMA2_Stream0_IRQHandler(void)
{
#if (DATA_CAPTURE_MODE == BURST_DATA_CAPTURE)
	if (callback_count == 1) {
		HAL_GPIO_WritePin(SYNC_INB_PORT_ID, 1 << SYNC_INB, GPIO_PIN_RESET);
	}
#endif

	HAL_DMA_IRQHandler(&hdma_spi1_rx);
}
