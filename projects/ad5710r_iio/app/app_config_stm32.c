/***************************************************************************//**
 *   @file    app_config_stm32.c
 *   @brief   Application configurations module for STM32 platform
********************************************************************************
 * Copyright (c) 2024-25 Analog Devices, Inc.
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
#include "ad5710r.h"
#include "ad5710r_iio.h"
#include "usb_device.h"

/******************************************************************************/
/************************ Macros/Constants ************************************/
/******************************************************************************/

/******************************************************************************/
/******************** Variables and User Defined Data Types *******************/
/******************************************************************************/

/* UART STM32 Platform Specific Init Parameters */
struct stm32_uart_init_param stm32_uart_init_params = {
	.huart = APP_UART_HANDLE
};

/* VCOM STM32 Platform Specific Init Parameters */
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

/* LDAC pin STM32 GPIO specific parameters */
struct stm32_gpio_init_param stm32_gpio_ldac_init_params = {
	.mode = GPIO_MODE_OUTPUT_PP,
	.speed = GPIO_SPEED_FREQ_VERY_HIGH
};

/* STM32 LDAC PWM GPIO specific parameters */
struct stm32_gpio_init_param stm32_pwm_ldac_gpio_init_params = {
	.mode = GPIO_MODE_AF_PP,
	.speed = GPIO_SPEED_FREQ_VERY_HIGH,
	.alternate = GPIO_AF1_TIM1
};

/* Reset pin STM32 GPIO specific parameters */
struct stm32_gpio_init_param stm32_gpio_reset_init_params = {
	.mode = GPIO_MODE_OUTPUT_PP
};

/* STM32 LDAC PWM specific parameters */
struct stm32_pwm_init_param stm32_ldac_pwm_init_params = {
	.htimer = &LDAC_PWM_HANDLE,
	.prescaler = LDAC_PWM_PRESCALER,
	.timer_autoreload = true,
	.mode = TIM_OC_PWM1,
	.timer_chn = LDAC_PWM_CHANNEL,
	.get_timer_clock = HAL_RCC_GetPCLK1Freq,
	.clock_divider = LDAC_PWM_CLK_DIVIDER,
	.slave_mode = STM32_PWM_SM_DISABLE,
	.trigger_output = PWM_TRGO_UPDATE
};

#if (INTERFACE_MODE == SPI_DMA)
DMA_HandleTypeDef hdma_spi1_tx;

/* STM32 CSB GPIO specific parameters */
struct stm32_gpio_init_param stm32_csb_gpio_init_params = {
	.mode = GPIO_MODE_OUTPUT_PP,
	.speed = GPIO_SPEED_FREQ_VERY_HIGH
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
	.clock_divider = TIMER_8_CLK_DIVIDER,
	.slave_mode = STM32_PWM_SM_TRIGGER,
	.trigger_source = PWM_TS_ITR0,
	.dma_enable = true,
	.repetitions = NUM_PULSE_REPETITIONS,
	.onepulse_enable = true
};

/* STM32 Tx DMA channel extra init params for single instruction mode */
struct stm32_dma_channel txdma_channel_single_instr_mode = {
	.hdma = &hdma_tim8_ch1,
	.ch_num = TxDMA_CHANNEL_NUM,
	.mem_increment = true,
	.mem_data_alignment = DATA_ALIGN_BYTE,
	.per_data_alignment = DATA_ALIGN_BYTE,
	.dma_mode = DMA_CIRCULAR_MODE
};

/* STM32 Tx DMA channel extra init params for streaming mode */
struct stm32_dma_channel txdma_channel_stream_mode = {
	.hdma = &hdma_spi1_tx,
	.ch_num = DMA_CHANNEL_3,
	.mem_increment = true,
	.mem_data_alignment = DATA_ALIGN_BYTE,
	.per_data_alignment = DATA_ALIGN_BYTE,
	.dma_mode = DMA_NORMAL_MODE
};

/* STM32 Rx DMA channel extra init params */
struct stm32_dma_channel rxdma_channel = {
	.hdma = &hdma_spi1_rx,
	.ch_num = RxDMA_CHANNEL_NUM,
	.mem_increment = false,
	.mem_data_alignment = DATA_ALIGN_BYTE,
	.per_data_alignment = DATA_ALIGN_BYTE,
	.dma_mode = DMA_CIRCULAR_MODE,
};

/* STM32 SPI Descriptor*/
volatile struct stm32_spi_desc* sdesc;

/* Flag to monitor the transfer stop command */
static bool transfer_stop_flag;

/* Flag to monitor if complete transfer callback has been entered */
static bool entered_cb;

/* Timeout count to avoid stuck into potential infinite loop while checking
 * for full transfer complete after the stop command is receieved.
 * Note: This timeout factor is dependent upon the MCU clock frequency. Below timeout
 * is tested for SDP-K1 platform @180Mhz default core clock */
#define TRANSFER_COMPLETE_TIMEOUT	0xffffff

#endif

/* Flag to monitor if stm32_init function has been entered */
static bool entered_init;

/******************************************************************************/
/************************** Functions Declarations ****************************/

void SystemClock_Config(void);

/******************************************************************************/
/************************** Functions Definitions *****************************/
/******************************************************************************/
/**
 * @brief 	Initialize the STM32 system peripherals
 * @return	None
 */
void stm32_system_init(void)
{
	if (entered_init ==  false) {
		HAL_Init();
		SystemClock_Config();
		MX_GPIO_Init();
		MX_UART5_Init();
		MX_I2C1_Init();
		MX_SPI1_Init();
#if (INTERFACE_MODE == SPI_DMA)
		MX_DMA_Init();
#endif
		MX_TIM1_Init();
		MX_USB_DEVICE_Init();
		entered_init = true;
	}
#if (INTERFACE_MODE == SPI_DMA)
	if (streaming_option == SINGLE_INSTRUCTION_MODE) {
		MX_TIM8_Init();
		hdma_tim8_ch1.Instance = DMA2_Stream2;
	} else {
		hdma_spi1_tx.Instance = DMA2_Stream3;
	}
#endif
}

/**
  * @brief This function handles the LDAC GPIO interrupt event, to which
  *	       EXTI line 10 interrupt is dedicated
  * @return None
  */
void EXTI15_10_IRQHandler(void)
{
	HAL_GPIO_EXTI_IRQHandler(1 << PWM_GPIO_PIN);
}

/**
 * @brief	Reconfigure the STM32 specific system parameters
 * 			with change in data streaming mode
 * @return	0 in case of success, negative error code otherwise
 */
int reconfig_stm32_params(void)
{
	int ret;

	HAL_NVIC_DisableIRQ(DMA2_Stream0_IRQn);

	ret = no_os_pwm_remove(pwm_desc);
	if (ret) {
		return ret;
	}

	/* Reinit stm32 system parameters */
	stm32_system_init();

	ret = init_pwm();
	if (ret) {
		return ret;
	}

	return 0;
}

#if (INTERFACE_MODE == SPI_DMA)
/**
 * @brief Configure Tx Trigger timer for slave mode operation, one-pulse mode
 *		  and to generate DMA requests
 * @return None
 */
void tim8_config(void)
{
	TIM8->EGR = TIM_EGR_UG; // Generate update event

	TIM8->DIER |= TIM_DIER_CC1DE; // Generate DMA request after overflow
}

/**
 * @brief 	Starts the timer signal generation for
 *          PWM.
 * @return	0 in case of success, negative error code otherwise
 */
int stm32_timer_enable(void)
{
	int ret;

	/* Enable tx trigger timer */
	ret = no_os_pwm_enable(tx_trigger_desc);
	if (ret) {
		return ret;
	}

	/* Enable LDAC */
	ret = no_os_pwm_enable(pwm_desc);
	if (ret) {
		return ret;
	}

	return 0;
}

/**
 * @brief 	Stop generating timer signals.
 * @return	0 in case of success, negative error code otherwise
 */
int stm32_timer_stop(void)
{
	int ret;
	sdesc = ad5710r_dev_desc->spi->extra;
	uint32_t timeout = TRANSFER_COMPLETE_TIMEOUT;
	transfer_stop_flag = true;

	while (transfer_stop_flag && (timeout-- > 0));
	if (timeout == 0) {
		return -ETIMEDOUT;
	}

	if (streaming_option == SINGLE_INSTRUCTION_MODE) {
		ret = no_os_pwm_disable(pwm_desc);
		if (ret) {
			return ret;
		}

		ret = no_os_pwm_disable(tx_trigger_desc);
		if (ret) {
			return ret;
		}
	}

	/* Disable RX DMA */
	CLEAR_BIT(sdesc->hspi.Instance->CR2, SPI_CR2_RXDMAEN);

	return 0;
}

/**
 * @brief  Abort DMA Transfers
 * @return None
 */
int stm32_abort_dma_transfer(void)
{
	int ret;
	sdesc = ad5710r_dev_desc->spi->extra;
	entered_cb = false;

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
 * @brief Callback function to flag the transfer of number
 *        of requested samples.
 * @param hdma[in] - DMA Handler (Unused)
 * @return	None
 */
void receivecomplete_callback(DMA_HandleTypeDef* hdma)
{
	if (streaming_option == SINGLE_INSTRUCTION_MODE) {
		if (transfer_stop_flag) {
			TIM8->DIER &= ~TIM_DIER_CC1DE;
			transfer_stop_flag = false;
		}
	} else {
		/* Streaming mode */
		if (!entered_cb) {
			/* Enable circular dma transfer mode and update memory address, ndtr to exclude channel address
			*  after the first linear/normal dma transfer is completed */
			DMA2_Stream3->CR &= ~DMA_SxCR_EN; // Disable DMA
			DMA2_Stream3->CR |= DMA_SxCR_CIRC; //circular mode enable
			DMA2_Stream3->M0AR = (uint32_t)(global_iio_buff + 2);	// update memory address
			DMA2_Stream3->NDTR = num_of_samples * 2; // update NDTR

			DMA2_Stream3->CR |= DMA_SxCR_EN; // Enable DMA
			entered_cb = true;
		} else if (transfer_stop_flag) {
			/* Disable TX DMA */
			CLEAR_BIT(sdesc->hspi.Instance->CR2, SPI_CR2_TXDMAEN);
			transfer_stop_flag = false;
		}
	}
}

#endif
