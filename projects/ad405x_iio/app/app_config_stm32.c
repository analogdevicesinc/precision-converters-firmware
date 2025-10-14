/***************************************************************************//**
 * @file    app_config_stm32.c
 * @brief   Source file for STM32 platform configurations
********************************************************************************
* Copyright (c) 2023-2025 Analog Devices, Inc.
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
#include <string.h>
#include "app_config.h"
#include "app_config_stm32.h"
#include "ad405x_iio.h"
#include "ad405x.h"
#include "iio.h"

#ifdef STM32F469xx
#include "usb_device.h"
#endif
#ifdef STM32H563xx
#include "app_usbx_device.h"
#endif

/******************************************************************************/
/************************ Macros/Constants ************************************/
/******************************************************************************/

/******************************************************************************/
/******************** Variables and User Defined Data Types *******************/
/******************************************************************************/
#ifdef STM32H563xx
I2C_HandleTypeDef hi2c1;

/* I2C extra init parameters */
struct stm32_i2c_init_param stm32_i2c_extra_init_params = {
	.i2c_timing = I2C_TIMING
};
#endif

#ifdef SPI_SUPPORT_AVAILABLE
/* STM32 Tx DMA channel extra init params */
struct stm32_dma_channel spi_dma_txdma_channel = {
	.hdma = &AD405x_TxDMA_HANDLE,
	.ch_num = AD405x_TxDMA_CHANNEL_NUM,
	.mem_increment = false,
	.per_increment = false,
	.mem_data_alignment = DATA_ALIGN_BYTE,
	.per_data_alignment = DATA_ALIGN_BYTE,
	.dma_mode = DMA_CIRCULAR_MODE
};

/* STM32 Rx DMA channel extra init params */
struct stm32_dma_channel spi_dma_rxdma_channel = {
	.hdma = &AD405x_RxDMA_HANDLE,
	.ch_num = AD405x_RxDMA_CHANNEL_NUM,
	.mem_increment = true,
	.mem_data_alignment = DATA_ALIGN_HALF_WORD,
	.per_data_alignment = DATA_ALIGN_HALF_WORD,
	.dma_mode = DMA_CIRCULAR_MODE,
};
#endif

#ifdef I3C_SUPPORT_AVAILABLE
/* STM32 trigger parameter for TX DMA Channel */
struct stm32_dma_trigger i3c_dma_txdma_trig = {
	.id = GPDMA1_TRIGGER_LPTIM1_CH1,
	.mode = STM32_DMA_SINGLE_BURST_MODE,
	.polarity = STM32_DMA_TRIG_RISING
};

/* STM32 Tx DMA channel extra init params */
struct stm32_dma_channel i3c_dma_txdma_channel = {
	.hdma = &AD405x_TxDMA_HANDLE,
	.ch_num = AD405x_TxDMA_CHANNEL_NUM,
	.mem_increment = false,
	.per_increment = false,
	.mem_data_alignment = DATA_ALIGN_WORD,
	.per_data_alignment = DATA_ALIGN_WORD,
	.dma_mode = DMA_CIRCULAR_MODE,
	.trig = &i3c_dma_txdma_trig
};

/* STM32 Rx DMA channel extra init params */
struct stm32_dma_channel i3c_dma_rxdma_channel = {
	.hdma = &AD405x_RxDMA_HANDLE,
	.ch_num = AD405x_RxDMA_CHANNEL_NUM,
	.mem_increment = true,
	.mem_data_alignment = DATA_ALIGN_BYTE,
	.per_data_alignment = DATA_ALIGN_BYTE,
	.dma_mode = DMA_CIRCULAR_MODE,
};
#endif

/* STM32 UART specific parameters */
struct stm32_uart_init_param stm32_uart_extra_init_params = {
	.huart = &UART_HANDLE,
};

#ifdef STM32F469xx
/* STM32 VCOM init parameters */
struct stm32_usb_uart_init_param stm32_vcom_extra_init_params = {
	.husbdevice = &hUsbDeviceHS,
};
#endif

#ifdef STM32H563xx
/* STM32 VCOM init parameters */
struct stm32_usb_uart_init_param stm32_vcom_extra_init_params = {
	.hpcd = &hpcd_USB_DRD_FS,
};
#endif

#ifdef SPI_SUPPORT_AVAILABLE
/* STM32 SPI specific parameters */
struct stm32_spi_init_param stm32_spi_extra_init_params = {
	.chip_select_port = SPI_CS_PORT_NUM,
	.get_input_clock = HAL_RCC_GetPCLK2Freq,
	.alternate = GPIO_AF1_TIM2
};

/* STM32 GPIO specific parameters */
struct stm32_gpio_init_param stm32_gpio_cnv_extra_init_params = {
	.mode = GPIO_MODE_OUTPUT_PP,
	.speed = GPIO_SPEED_FREQ_HIGH,
};
#endif

#ifdef I3C_SUPPORT_AVAILABLE
struct stm32_i3c_dma_desc i3c_dma_desc;
/* STM32 I3C specific parameters */
struct stm32_i3c_init_param stm32_i3c_extra_init_params = {
	.hi3c = &I3C_HANDLE,
};
#endif

/* STM32 GPIO specific parameters */
struct stm32_gpio_init_param stm32_gpio_gp0_extra_init_params = {
	.mode = GPIO_MODE_INPUT,
	.speed = GPIO_SPEED_FREQ_HIGH,
};

/* STM32 GPIO specific parameters */
struct stm32_gpio_init_param stm32_gpio_gp1_extra_init_params = {
	.mode = GPIO_MODE_INPUT,
	.speed = GPIO_SPEED_FREQ_HIGH,
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
	.speed = GPIO_SPEED_FREQ_HIGH,
	.alternate = GPIO_AF1_TIM2
};

/* STM32 CS GPIO Extra init params in GPIO Mode */
struct stm32_gpio_init_param stm32_cs_gpio_extra_init_params = {
	.mode = GPIO_MODE_OUTPUT_PP,
	.speed = GPIO_SPEED_FREQ_HIGH,
};

/* STM32 PWM GPIO specific parameters */
struct stm32_gpio_init_param stm32_pwm_gpio_extra_init_params = {
	.mode = GPIO_MODE_AF_PP,
	.speed = GPIO_SPEED_FREQ_HIGH,
	.alternate = GPIO_AF1_TIM1
};

/* STM32 PWM for specific parameters for generating conversion pulses
 * in PWM 1 mode as well for triggering SPI DMA transaction of
 * higher byte of 16-bit data.
 * */
struct stm32_pwm_init_param stm32_pwm_cnv_extra_init_params = {
	.htimer = CNV_TIMER_HANDLE,
	.pwm_timer = CNV_TIMER_TYPE,
	.prescaler = CNV_TIMER_PRESCALER,
	.timer_autoreload = true,
	.mode = TIM_OC_PWM1,
	.timer_chn = CNV_TIMER_CHANNEL,
	.get_timer_clock = HAL_RCC_GetPCLK2Freq,
	.clock_divider = CNV_TIMER_CLK_DIVIDER,
	.slave_mode = STM32_PWM_SM_DISABLE,
	.trigger_output = PWM_TRGO_UPDATE
};

#ifdef SPI_SUPPORT_AVAILABLE
/* STM32 PWM for specific parameters for generating the
 * the chip select signals in PWM Mode 2.
 * */
struct stm32_pwm_init_param stm32_cs_extra_init_params = {
	.htimer = CS_TIMER_HANDLE,
	.pwm_timer = CS_TIMER_TYPE,
	.prescaler = CS_TIMER_PRESCALER,
	.timer_autoreload = false,
	.mode = TIM_OC_PWM2,
	.timer_chn = CS_TIMER_CHANNEL,
	.get_timer_clock = HAL_RCC_GetPCLK1Freq,
	.clock_divider = CS_TIMER_CLK_DIVIDER
};

/* STM32 PWM specific init params */
struct stm32_pwm_init_param stm32_tx_trigger_extra_init_params = {
	.htimer = TX_TRIGGER_TIMER_HANDLE,
	.pwm_timer = TX_TRIGGER_TIMER_TYPE,
	.prescaler = TX_TRIGGER_TIMER_PRESCALER,
	.timer_autoreload = true,
	.mode = TIM_OC_TOGGLE,
	.timer_chn = TX_TRIGGER_TIMER_CHANNEL,
	.complementary_channel = false,
	.get_timer_clock = HAL_RCC_GetPCLK1Freq,
	.clock_divider = TX_TRIGGER_TIMER_CLK_DIVIDER,
	.slave_mode = STM32_PWM_SM_TRIGGER,
	.trigger_source = PWM_TS_ITR0,
	.repetitions = 0,
	.onepulse_enable = true,
	.dma_enable = true,
};
#endif

/* STM32 SPI Descriptor*/
volatile struct stm32_spi_desc* sdesc;

/* Number of times the DMA complete callback needs to be invoked for
 * capturing the desired number of samples*/
volatile int dma_cycle_count = 0;

/* The number of transactions requested for the RX DMA stream */
uint32_t rxdma_ndtr;

/* Pointer to start of the IIO buffer */
volatile uint8_t *iio_buf_start_idx;

/* Pointer to start of the local SRAM buffer
 * used by RXDMA to put data directly in. */
volatile uint8_t *dma_buf_start_idx;

/* Pointer to the current location being written to, in the IIO buffer */
volatile uint8_t *iio_buf_current_idx;

/* Pointer to the current location being written to, by the DMA */
volatile uint8_t *dma_buf_current_idx;

/******************************************************************************/
/************************** Functions Declaration *****************************/
/******************************************************************************/
void SystemClock_Config(void);
void receivecomplete_callback(DMA_HandleTypeDef * hdma);
/******************************************************************************/
/************************** Functions Definition ******************************/
/******************************************************************************/

#ifdef STM32H563xx
/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
void MX_I2C1_Init(void)
{

	/* USER CODE BEGIN I2C1_Init 0 */

	/* USER CODE END I2C1_Init 0 */

	/* USER CODE BEGIN I2C1_Init 1 */

	/* USER CODE END I2C1_Init 1 */
	hi2c1.Instance = I2C1;
	hi2c1.Init.Timing = 0x00000E14;
	hi2c1.Init.OwnAddress1 = 0;
	hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
	hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
	hi2c1.Init.OwnAddress2 = 0;
	hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
	hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
	hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
	if (HAL_I2C_Init(&hi2c1) != HAL_OK) {
		Error_Handler();
	}

	/** Configure Analogue filter
	*/
	if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK) {
		Error_Handler();
	}

	/** Configure Digital filter
	*/
	if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN I2C1_Init 2 */

	/* USER CODE END I2C1_Init 2 */

}


/**
* @brief I2C MSP Initialization
* This function configures the hardware resources used in this example
* @param hi2c: I2C handle pointer
* @retval None
*/
void HAL_I2C_MspInit(I2C_HandleTypeDef* hi2c)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
	if (hi2c->Instance == I2C1) {
		/* USER CODE BEGIN I2C1_MspInit 0 */

		/* USER CODE END I2C1_MspInit 0 */

		/** Initializes the peripherals clock
		*/
		PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_I2C1;
		PeriphClkInitStruct.I2c1ClockSelection = RCC_I2C1CLKSOURCE_CSI;
		if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK) {
			Error_Handler();
		}

		__HAL_RCC_GPIOB_CLK_ENABLE();
		/**I2C1 GPIO Configuration
		PB8     ------> I2C1_SCL
		PB9     ------> I2C1_SDA
		*/
		GPIO_InitStruct.Pin = GPIO_PIN_8 | GPIO_PIN_9;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
		GPIO_InitStruct.Pull = GPIO_PULLUP;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
		GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
		HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

		/* Peripheral clock enable */
		__HAL_RCC_I2C1_CLK_ENABLE();
		/* USER CODE BEGIN I2C1_MspInit 1 */

		/* USER CODE END I2C1_MspInit 1 */
	}

}

/**
* @brief I2C MSP De-Initialization
* This function freeze the hardware resources used in this example
* @param hi2c: I2C handle pointer
* @retval None
*/
void HAL_I2C_MspDeInit(I2C_HandleTypeDef* hi2c)
{
	if (hi2c->Instance == I2C1) {
		/* USER CODE BEGIN I2C1_MspDeInit 0 */

		/* USER CODE END I2C1_MspDeInit 0 */
		/* Peripheral clock disable */
		__HAL_RCC_I2C1_CLK_DISABLE();

		/**I2C1 GPIO Configuration
		PB8     ------> I2C1_SCL
		PB9     ------> I2C1_SDA
		*/
		HAL_GPIO_DeInit(GPIOB, GPIO_PIN_8);

		HAL_GPIO_DeInit(GPIOB, GPIO_PIN_9);

		/* USER CODE BEGIN I2C1_MspDeInit 1 */

		/* USER CODE END I2C1_MspDeInit 1 */
	}

}

/**
  * @brief This function handles I2C1 Event interrupt.
  */
void I2C1_EV_IRQHandler(void)
{
	/* USER CODE BEGIN I2C1_EV_IRQn 0 */

	/* USER CODE END I2C1_EV_IRQn 0 */
	HAL_I2C_EV_IRQHandler(&hi2c1);
	/* USER CODE BEGIN I2C1_EV_IRQn 1 */

	/* USER CODE END I2C1_EV_IRQn 1 */
}

/**
  * @brief This function handles I2C1 Error interrupt.
  */
void I2C1_ER_IRQHandler(void)
{
	/* USER CODE BEGIN I2C1_ER_IRQn 0 */

	/* USER CODE END I2C1_ER_IRQn 0 */
	HAL_I2C_ER_IRQHandler(&hi2c1);
	/* USER CODE BEGIN I2C1_ER_IRQn 1 */

	/* USER CODE END I2C1_ER_IRQn 1 */
}
#endif

/**
 * @brief 	Initialize the STM32 system peripherals
 * @return	None
 */
void stm32_system_init(void)
{
	HAL_Init();
	SystemClock_Config();
	MX_GPIO_Init();
	MX_I2C1_Init();
#ifdef STM32F469xx
	MX_DMA_Init();
	MX_TIM2_Init();
	MX_UART5_Init();
	MX_TIM1_Init();
	MX_TIM8_Init();
	MX_SPI1_Init();
	MX_USB_DEVICE_Init();
	HAL_NVIC_DisableIRQ(STM32_GP1_IRQ);
#endif
#ifdef STM32H563xx

	MX_GPIO_Init();
	MX_USART3_UART_Init();
	MX_LPTIM1_Init();
	MX_GPDMA1_Init();
#ifdef USE_VIRTUAL_COM_PORT
	MX_USB_PCD_Init();
	MX_USBX_Device_Init();
#endif

#ifdef Tx_DMA_IRQ_ID
	HAL_NVIC_DisableIRQ(Tx_DMA_IRQ_ID);
#endif

#endif
}

/**
 * @brief 	Initialize the STM32 system peripherals after the device
 * 			has been verified.
 * @return	None
 */
void stm32_system_init_post_verification(void)
{
#ifdef I3C_SUPPORT_AVAILABLE
	MX_I3C1_Init();

	if (ad405x_interface_mode == I3C_DMA) {

		*(uint32_t *)(i3c_cr_dma_xfer.src) = 0x90 << 24 |
						     ((ad405x_i3c_dyn_addr << 1) | 0x01) << 16 |
						     bytes_per_sample;
		i3c_cr_dma_xfer.dst = (uint8_t *)&I3C_CR_REG;

		i3c_dma_desc.dma_desc = ad405x_dma_desc;
		i3c_dma_desc.crdma_ch = &ad405x_dma_desc->channels[0];
		i3c_dma_desc.rxdma_ch = &ad405x_dma_desc->channels[1];
		i3c_dma_desc.txdma_ch = NULL;
		i3c_dma_desc.srdma_ch = NULL;

		stm32_i3c_extra_init_params.irq_id = I3C1_EV_IRQn;
		stm32_i3c_extra_init_params.i3c_dma_desc = &i3c_dma_desc;
	}
#endif

#if (APP_CAPTURE_MODE == WINDOWED_DATA_CAPTURE)
	/* Register half complete callback, for ping-pong buffers implementation. */
	HAL_DMA_RegisterCallback(&AD405x_RxDMA_HANDLE,
				 HAL_DMA_XFER_HALFCPLT_CB_ID,
				 receivecomplete_callback);
#endif
}

/**
 * @brief Update buffer index
 * @param local_buf[out] - Local Buffer
 * @param buf_start_addr[out] - Buffer start addr
 * @return	None
 */
void update_buff(uint8_t *local_buf, uint8_t *buf_start_addr)
{
	iio_buf_start_idx = buf_start_addr;
	dma_buf_start_idx = local_buf;

	iio_buf_current_idx = iio_buf_start_idx;
	dma_buf_current_idx = dma_buf_start_idx;
}

#ifdef SPI_SUPPORT_AVAILABLE

/**
 * @brief   Callback function to flag the capture of number
 *          of requested samples.
 * @param hdma - DMA handler (Unused)
 * @return	None
 */
void receivecomplete_callback(DMA_HandleTypeDef * hdma)
{
#if (APP_CAPTURE_MODE == WINDOWED_DATA_CAPTURE)
	uint32_t half_cmplt_size = rxdma_ndtr;

	if (dma_cycle_count) {

		dma_cycle_count--;

		/* Update samples captured so far */
		if (dma_cycle_count) {
			memcpy((void *)iio_buf_current_idx,
			       (void *)dma_buf_current_idx,
			       half_cmplt_size);

			/* Update the current IIO buffer pointer */
			iio_buf_current_idx += half_cmplt_size;
		} else {

			/* Stop timers at the last entry to the callback */
			TIM8->DIER &= ~TIM_DIER_CC1DE;

			/* Timer is already stopped in GPDMA1_Channel1_IRQHandler */
			memcpy((void *)iio_buf_current_idx,
			       (void *)dma_buf_current_idx,
			       nb_of_bytes_remaining_g);

			/* Update the current IIO buffer pointer */
			iio_buf_current_idx = iio_buf_start_idx;
			data_ready = true;
		}
	}

	/* Update the current DMA and IIO buffer pointers */
	if (dma_buf_current_idx != dma_buf_start_idx) {
		dma_buf_current_idx = dma_buf_start_idx;
	} else {
		dma_buf_current_idx += half_cmplt_size;
	}

#elif (APP_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
	no_os_cb_end_async_write(iio_dev_data_g->buffer->buf);
	no_os_cb_prepare_async_write(iio_dev_data_g->buffer->buf,
				     nb_of_bytes_g,
				     (void **)&buff_start_addr,
				     &data_read);
#endif

	return;
}

/**
  * @brief 	IRQ handler for RX DMA channel
  * @return None
  */
void DMA2_Stream0_IRQHandler(void)
{
#if (APP_CAPTURE_MODE == WINDOWED_DATA_CAPTURE)
	/* Stop timers at the last entry to the callback */
	if (dma_cycle_count == 1) {

		TIM8->DIER &= ~TIM_DIER_CC1DE;
	}
#endif

	HAL_DMA_IRQHandler(&hdma_spi1_rx);
}

/**
 * @brief 	Starts the timer signal generation for
 *          PWM and OC channels all at once.
 * @return	None
 */
void stm32_timer_enable(void)
{
	/* Reset the Count values of timers to keep in sync */
	(CNV_TIMER_HANDLE)->Instance->CNT = 0;
	(CS_TIMER_HANDLE)->Instance->CNT = 0;

	/* Enable timers 1 and 2 */
	(CNV_TIMER_HANDLE)->Instance->CCER |= TIM_CCER_CC3E;
	(CS_TIMER_HANDLE)->Instance->CCER |= TIM_CCER_CC1E;

	(CNV_TIMER_HANDLE)->Instance->BDTR |= TIM_BDTR_MOE;
	(CS_TIMER_HANDLE)->Instance->BDTR |= TIM_BDTR_MOE;

	/* Start CS PWM before CNV PWM */
	(CS_TIMER_HANDLE)->Instance->CR1 |= TIM_CR1_CEN;
	(CNV_TIMER_HANDLE)->Instance->CR1 |= TIM_CR1_CEN;
}

/**
 * @brief 	Stops generating timer signals.
 * @return	None
 */
void stm32_timer_stop(void)
{
	TIM8->DIER &= ~TIM_DIER_CC1DE;

	no_os_pwm_disable(pwm_desc);
	no_os_pwm_disable(cs_pwm_desc);
}

/**
 * @brief 	Configures the chip select pin as output mode.
 * @param   is_gpio[in] - Mode of the Pin
 * @return	None
 */
void stm32_cs_output_gpio_config(bool is_gpio)
{
	if (cs_gpio_desc) {
		no_os_gpio_remove(cs_gpio_desc);
	}

	if (is_gpio) {
		cs_pwm_gpio_params.extra = &stm32_cs_gpio_extra_init_params;
	} else {
		cs_pwm_gpio_params.extra = &stm32_cs_pwm_gpio_extra_init_params;
	}

	no_os_gpio_get(&cs_gpio_desc, &cs_pwm_gpio_params);
}

/**
 * @brief 	Abort ongoing SPI RX DMA transfer.
 * @return	None
 */
int stm32_abort_dma_transfer(void)
{
	int ret;

	sdesc = (struct stm32_spi_desc *)p_ad405x_dev->com_desc.spi_desc->extra;

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

/**
 * @brief   Configures the SPI data frame format pin to 8/16 bit.
 * @param   is_16_bit[in] - Boolean indicatin the data frame format.
 * @return  None
 */
void stm32_config_spi_data_frame_format(bool is_16_bit)
{
	SPI1->CR1 &= ~SPI_CR1_SPE;
	if (is_16_bit) {
		SPI1->CR1 |= SPI_CR1_DFF;
	} else {
		SPI1->CR1 &= ~SPI_CR1_DFF;
	}
	SPI1->CR1 |= SPI_CR1_SPE;
}
#endif

/**
 * @brief   Configures the prescalar according to the operating mode.
 * @return  None
 */
void stm32_config_cnv_prescalar(void)
{
	if (ad405x_operating_mode == AD405X_BURST_AVERAGING_MODE_OP) {
		stm32_pwm_cnv_extra_init_params.prescaler = CNV_TIMER_BURST_AVG_PRESCALER;
	} else {
		stm32_pwm_cnv_extra_init_params.prescaler = CNV_TIMER_PRESCALER;
	}
}


#ifdef STM32H563xx
/**
  * @brief 	IRQ handler for RX DMA channel
  * @return None
  */
void GPDMA1_Channel1_IRQHandler(void)
{
#if (APP_CAPTURE_MODE == WINDOWED_DATA_CAPTURE)
	/* Stop timers at the last entry to the callback */
	if (dma_cycle_count <= 1) {
		(CNV_TIMER_HANDLE)->Instance->CR &= ~0x01;
		(void) no_os_pwm_disable(pwm_desc);
	}
#endif
	HAL_DMA_IRQHandler(&handle_GPDMA1_Channel1);
}
#endif

#ifdef I3C_SUPPORT_AVAILABLE

/**
 * @brief   Callback function to flag the capture of number
 *          of requested samples.
 * @param hdma - DMA handler (Unused)
 * @return	None
 */
void receivecomplete_callback(DMA_HandleTypeDef *hdma)
{
#if (APP_CAPTURE_MODE == WINDOWED_DATA_CAPTURE)
	/* rxdma_ndtr is always 2B or 4B aligned */
	uint32_t half_cmplt_size = rxdma_ndtr >> 1;

	if (dma_cycle_count) {

		dma_cycle_count--;

		/* Update samples captured so far */
		if (dma_cycle_count) {
			memcpy((void *)iio_buf_current_idx,
			       (void *)dma_buf_current_idx,
			       half_cmplt_size);

			/* Update the current IIO buffer pointer */
			iio_buf_current_idx += half_cmplt_size;
		} else {
			/* Timer is already stopped in GPDMA1_Channel1_IRQHandler */
			memcpy((void *)iio_buf_current_idx,
			       (void *)dma_buf_current_idx,
			       nb_of_bytes_remaining_g);
			/*
			 * There is no timer stop here for windowed capture since the timer is already
			 * in GPDMA1_Channel1_IRQHandler.
			 */

			/* Update the current IIO buffer pointer */
			iio_buf_current_idx = iio_buf_start_idx;
			data_ready = true;
		}
	}

	/* Update the current DMA and IIO buffer pointers */
	if (dma_buf_current_idx != dma_buf_start_idx) {
		dma_buf_current_idx = dma_buf_start_idx;
	} else {
		dma_buf_current_idx += half_cmplt_size;
	}

#elif (APP_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
	no_os_cb_end_async_write(iio_dev_data_g->buffer->buf);
	no_os_cb_prepare_async_write(iio_dev_data_g->buffer->buf,
				     nb_of_bytes_g,
				     (void **)&buff_start_addr,
				     &data_read);
#endif

	return;
}

/**
 * @brief 	Abort ongoing SPI RX DMA transfer.
 * @return	None
 */
int stm32_abort_dma_transfer(void)
{
	return no_os_i3c_transfer_abort(p_ad405x_dev->com_desc.i3c_desc);
}
#endif


/**
 * @brief   Dummy function for USBx middleware used in STM32H563.
 * @return	0
 */
__weak unsigned int ux_device_stack_tasks_run(void)
{
	return 0;
}
