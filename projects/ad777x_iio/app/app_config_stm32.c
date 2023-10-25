/***************************************************************************//**
 * @file    app_config_stm32.c
 * @brief   Source file for STM32 platform configurations
********************************************************************************
* Copyright (c) 2022-2023 Analog Devices, Inc.
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
#include "stm32_gpio.h"
#include "no_os_error.h"
#include "stm32_tdm_support.h"
#include "app_config.h"

/******************************************************************************/
/********************* Macros and Constants Definition ************************/
/******************************************************************************/

/******************************************************************************/
/******************** Variables and User Defined Data Types *******************/
/******************************************************************************/

/******************************************************************************/
/************************ Functions Definitions *******************************/
/******************************************************************************/

/**
 * @brief 	Return the peripheral frequency
 * @return	Peripheral frequency in Hz
 */
uint32_t HAL_RCC_GetSysClockFreq_app()
{
	return HAL_RCC_GetPCLK2Freq();
}

/* STM32 UART specific parameters */
struct stm32_uart_init_param stm32_uart_extra_init_params = {
	.huart = APP_UART_HANDLE
};

/* STM32 SPI specific parameters */
struct stm32_spi_init_param stm32_spi_extra_init_params = {
	.chip_select_port = STM32_SPI_CS_PORT,
	.get_input_clock = HAL_RCC_GetSysClockFreq_app
};

/* STM32 GPIO specific parameters */
struct stm32_gpio_init_param stm32_gpio_reset_extra_init_params = {
	.mode = GPIO_MODE_OUTPUT_PP
};

/* STM32 GPIO specific parameters */
struct stm32_gpio_init_param stm32_gpio_mode0_extra_init_params = {
	.mode = GPIO_MODE_OUTPUT_OD
};

/* STM32 GPIO specific parameters */
struct stm32_gpio_init_param stm32_gpio_mode1_extra_init_params = {
	.mode = GPIO_MODE_OUTPUT_PP
};

/* STM32 GPIO specific parameters */
struct stm32_gpio_init_param stm32_gpio_mode2_extra_init_params = {
	.mode = GPIO_MODE_OUTPUT_PP
};

/* STM32 GPIO specific parameters */
struct stm32_gpio_init_param stm32_gpio_mode3_extra_init_params = {
	.mode = GPIO_MODE_OUTPUT_PP
};

/* STM32 GPIO specific parameters */
struct stm32_gpio_init_param stm32_gpio_dclk0_extra_init_params = {
	.mode = GPIO_MODE_OUTPUT_PP
};

/* STM32 GPIO specific parameters */
struct stm32_gpio_init_param stm32_gpio_dclk1_extra_init_params = {
	.mode = GPIO_MODE_OUTPUT_PP
};

/* STM32 GPIO specific parameters */
struct stm32_gpio_init_param stm32_gpio_dclk2_extra_init_params = {
	.mode = GPIO_MODE_OUTPUT_PP
};

/* STM32 GPIO specific parameters */
struct stm32_gpio_init_param stm32_gpio_sync_in_extra_init_params = {
	.mode = GPIO_MODE_OUTPUT_PP
};

/* STM32 GPIO specific parameters */
struct stm32_gpio_init_param stm32_gpio_convst_sar_extra_init_params = {
	.mode = GPIO_MODE_OUTPUT_PP
};

/* STM32 GPIO specific parameters */
struct stm32_gpio_init_param stm32_gpio_drdy_extra_init_params = {
	.mode = GPIO_MODE_INPUT
};

/* STM32 GPIO specific parameters */
struct stm32_gpio_init_param stm32_gpio_error_extra_init_params = {
	.mode = GPIO_MODE_OUTPUT_OD
};

/* STM32 GPIO IRQ specific parameters */
struct stm32_gpio_irq_init_param stm32_trigger_gpio_irq_init_params = {
	.port_nb = GPIO_TRIGGER_INT_PORT
};

/* STM32 TDM specific parameters */
struct stm32_tdm_init_param stm32_tdm_extra_init_params = {
	.base = SAI_BASE
};

/* STM32 PWM specific parameters */
struct stm32_pwm_init_param stm32_pwm_extra_init_params = {
	.prescaler = MCLK_PWM_PRESCALER,
	.timer_autoreload = true,
	.mode = TIM_OC_PWM1,
	.timer_chn = MCLK_PWM_CHANNEL,
	.get_timer_clock = HAL_RCC_GetPCLK2Freq,
	.clock_divider = MCLK_PWM_CLK_DIVIDER
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
	MX_LPUART1_UART_Init();
	MX_SPI1_Init();
	MX_GPIO_Init();
	MX_SAI1_Init();
	MX_DMA_Init();
	MX_ICACHE_Init();
	MX_TIM1_Init();
}

/*!
 * @brief Prioritizes the LPUART1 interrupt over the other peripheral interrupts.
 * @return None
 */
void ad777x_configure_intr_priority(void)
{
	uint8_t priority = 0;
	uint8_t priGroup = 0;

	NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_3);

	priGroup = NVIC_GetPriorityGrouping();
	priority = NVIC_EncodePriority(priGroup, PERIPH_INTR_PRE_EMPT_PRIORITY,
				       PERIPH_INTR_SUB_PRI_PRIORITY);

	for (int currIRQ = WWDG_IRQn; currIRQ < ICACHE_IRQn; currIRQ++ ) {
		NVIC_SetPriority(currIRQ, priority);
	}

	priority = NVIC_EncodePriority(priGroup, UART_PRE_EMPT_PRIORITY,
				       UART_SUB_PRI_PRIORITY);
	NVIC_SetPriority(LPUART1_IRQn, priority);

}

/*!
 * @brief SAI DMA Receive Half Complete Callback function
 * @param hsai - pointer to a SAI_HandleTypeDef structure
 * @return None
 */
void ad777x_dma_rx_half_cplt(SAI_HandleTypeDef *hsai)
{
#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
	if (data_capture_operation) {
		end_tdm_dma_to_cb_transfer(ad777x_tdm_desc, ad777x_iio_dev_data,
					   TDM_DMA_READ_SIZE, BYTES_PER_SAMPLE);
	}
#endif
}

/*!
 * @brief SAI DMA Receive Complete Callback function
 * @param hsai - pointer to a SAI_HandleTypeDef structure
 * @return None
 */
void ad777x_dma_rx_cplt(SAI_HandleTypeDef *hsai)
{
	update_dma_buffer_overflow();

#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
	if (data_capture_operation) {
		end_tdm_dma_to_cb_transfer(ad777x_tdm_desc, ad777x_iio_dev_data,
					   TDM_DMA_READ_SIZE, BYTES_PER_SAMPLE);

		/* Start TDM DMA read as the peripheral is disabled in Normal(Linear)
		 * Buffer Mode upon buffer completion */
		no_os_tdm_read(ad777x_tdm_desc, dma_buff, TDM_DMA_READ_SIZE << 1);
	}
#endif
}
