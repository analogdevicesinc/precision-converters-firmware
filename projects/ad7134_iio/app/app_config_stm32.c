/***************************************************************************//**
 * @file    app_config_stm32.c
 * @brief   STM32 Specific configuration files for AD7134 IIO Application
 * @details This module contains the STM32 platform specific configurations
********************************************************************************
* Copyright (c) 2021,2023-24 Analog Devices, Inc.
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

/******************************************************************************/
/********************* Macros and Constants Definition ************************/
/******************************************************************************/

/******************************************************************************/
/******************** Variables and User Defined Data Types *******************/
/******************************************************************************/

/**
 * @brief 	Return the peripheral frequency
 * @return	Peripheral frequency in Hz
 */
uint32_t HAL_RCC_GetSysClockFreq_app()
{
	return HAL_RCC_GetPCLK2Freq();
}

/* UART STM32 Platform Specific Init Parameters */
struct stm32_uart_init_param stm32_uart_extra_init_params = {
	.huart = APP_UART_HANDLE
};

/* SPI STM32 Platform Specific Init Parameters */
struct stm32_spi_init_param stm32_spi_extra_init_params = {
	.chip_select_port = STM32_SPI_CS_PORT,
	.get_input_clock = HAL_RCC_GetSysClockFreq_app
};

/* SAI-TDM STM32 PLatform Specific Init Parameters */
struct stm32_tdm_init_param stm32_tdm_extra_init_params = {
	.base = STM32_SAI_BASE,
};

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
	MX_USART3_UART_Init();
	MX_SPI1_Init();
	MX_GPIO_Init();
	MX_SAI1_Init();
	MX_GPDMA1_Init();
	MX_ICACHE_Init();
}

/*!
 * @brief SAI DMA Receive Half Complete Callback function
 * @param hsai - pointer to a SAI_HandleTypeDef structure
 * @return None
 */
void ad7134_dma_rx_half_cplt(SAI_HandleTypeDef *hsai)
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
 * @param hsai - pointer to a SAI_HandleTypeDef structure
 * @return None
 */
void ad7134_dma_rx_cplt(SAI_HandleTypeDef *hsai)
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
