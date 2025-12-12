/***************************************************************************//**
 * @file    app_config_stm32.c
 * @brief   STM32 Specific configuration files for AD4134 IIO Application
 * @details This module contains the STM32 platform specific configurations
********************************************************************************
* Copyright (c) 2021,2023-25 Analog Devices, Inc.
* All rights reserved.
*
* This software is proprietary to Analog Devices, Inc. and its licensors.
* By using this software you agree to the terms of the associated
* Analog Devices Software License Agreement.
*******************************************************************************/

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/
#include "app_config.h"
#include "no_os_error.h"
#include "app_config_stm32.h"
#include "stm32_tdm_support.h"
#include "ad4134_support.h"

/******************************************************************************/
/********************* Macros and Constants Definition ************************/
/******************************************************************************/

/******************************************************************************/
/******************** Variables and User Defined Data Types *******************/
/******************************************************************************/
/* UART STM32 Platform Specific Init Parameters */
struct stm32_uart_init_param stm32_uart_extra_init_params = {
	.huart = &APP_UART_HANDLE
};

#ifdef STM32F469xx
/* VCOM Init Parameter */
struct stm32_usb_uart_init_param stm32_vcom_extra_init_params = {
	.husbdevice = &APP_UART_USB_HANDLE
};
#endif

/* SPI STM32 Platform Specific Init Parameters */
struct stm32_spi_init_param stm32_spi_extra_init_params = {
	.chip_select_port = STM32_SPI_CS_PORT,
	.get_input_clock = HAL_RCC_GetPCLK2Freq,
	.dma_init = NULL,
	.rxdma_ch = NULL,
	.txdma_ch = NULL,
	.irq_num = 0,
	.alternate = 0,
};

#if(INTERFACE_MODE == TDM_MODE)
/* SAI-TDM STM32 PLatform Specific Init Parameters */
struct stm32_tdm_init_param stm32_tdm_extra_init_params = {
	.base = STM32_SAI_BASE,
};
#endif

/* STM32 GPIO IRQ specific parameters */
struct stm32_gpio_irq_init_param stm32_trigger_gpio_irq_init_params = {
	.port_nb = GPIO_TRIGGER_INT_PORT
};

/* STM32 PDN GPIO specific parameters */
struct stm32_gpio_init_param stm32_pdn_extra_init_params = {
	.mode = GPIO_MODE_OUTPUT_PP
};

/* STM32 I2C specific parameters */
struct stm32_i2c_init_param stm32_i2c_extra_init_params = {
	.i2c_timing = I2C_TIMING
};

#if(INTERFACE_MODE == BIT_BANGING_MODE)
/* STM32 LDAC PWM specific parameters */
struct stm32_pwm_init_param stm32_pwm_extra_init_params = {
	.htimer = &PWM_HANDLE,
	.prescaler = PWM_PRESCALER,
	.timer_autoreload = true,
	.mode = TIM_OC_PWM1,
	.timer_chn = PWM_CHANNEL,
	.get_timer_clock = HAL_RCC_GetPCLK1Freq,
	.clock_divider = PWM_CLK_DIVIDER
};

/* LDAC pin STM32 GPIO in PWM alternate function mode specific parameters */
struct stm32_gpio_init_param stm32_pwm_gpio_init_params = {
	.mode = GPIO_MODE_AF_PP,
	.speed = GPIO_SPEED_FREQ_VERY_HIGH,
	.alternate = GPIO_AF2_TIM4
};

struct stm32_gpio_init_param stm32_input_extra_init_params = {
	.mode = GPIO_MODE_INPUT,
	.speed = GPIO_SPEED_FREQ_HIGH,
	.alternate = 0
};

struct stm32_gpio_init_param stm32_output_extra_init_params = {
	.mode = GPIO_MODE_OUTPUT_PP,
	.speed = GPIO_SPEED_FREQ_HIGH,
	.alternate = 0
};

extern uint16_t adc_data_continuous_mode[AD7134_NUM_CHANNELS];
#endif

/******************************************************************************/
/************************** Functions Declaration *****************************/
/******************************************************************************/
extern void SystemClock_Config(void);

/******************************************************************************/
/************************** Functions Definition ******************************/
/******************************************************************************/
#if(INTERFACE_MODE == BIT_BANGING_MODE)
/**
  * @brief This function handles EXTI line[15:10] interrupts.
  */
void EXTI15_10_IRQHandler(void)
{
	int32_t ret;

	/* Read all channels using GPIO Bit banging method for
	 * detecting a level change in DCLK signal */
	ret = ad7134_read_all_channels_bit_banging(adc_data_continuous_mode, false);
	if (ret) {
		return;
	}

	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_12);
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
	/* Provide some delay to initialize LL */
	HAL_Delay(2000);
	MX_SPI1_Init();
	MX_GPIO_Init();
#if(INTERFACE_MODE != TDM_MODE)
	MX_UART5_Init();
	MX_TIM4_Init();
	MX_I2C1_Init();
	MX_USB_DEVICE_Init();
	HAL_NVIC_SetPriority(APP_UART_USB_IRQ, 1, 0);
#else
	MX_SAI1_Init();
	MX_USART3_UART_Init();
	MX_GPDMA1_Init();
	MX_ICACHE_Init();
#endif
}

#if (INTERFACE_MODE == TDM_MODE)
/*!
 * @brief SAI DMA Receive Half Complete Callback function
 * @param hsai - pointer to a SAI Handle
 * @return None
 */
void ad7134_dma_rx_half_cplt(void *hsai)
{
#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
	if (data_capture_operation) {
		end_tdm_dma_to_cb_transfer(ad7134_tdm_desc, ad7134_iio_dev_data,
					   TDM_DMA_READ_SIZE, BYTES_PER_SAMPLE);
	}
#endif
}

/*!
 * @brief SAI DMA Receive Complete Callback function
 * @param hsai - pointer to a SAI Handle structure
 * @return None
 */
void ad7134_dma_rx_cplt(void *hsai)
{
	update_dma_buffer_overflow();

#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
	if (data_capture_operation) {
		end_tdm_dma_to_cb_transfer(ad7134_tdm_desc, ad7134_iio_dev_data,
					   TDM_DMA_READ_SIZE, BYTES_PER_SAMPLE);

		/* Start TDM DMA read as the peripheral is disabled in Normal(Linear)
		 * Buffer Mode upon buffer completion */
		no_os_tdm_read(ad7134_tdm_desc, dma_buff, TDM_DMA_READ_SIZE << 1);
	}
#endif
}
#endif
