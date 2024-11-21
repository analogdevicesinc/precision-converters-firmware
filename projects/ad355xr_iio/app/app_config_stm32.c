/***************************************************************************//**
 *   @file    app_config_stm32.c
 *   @brief   Application configurations module for STM32 platform
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

#include "app_config_stm32.h"
#include "no_os_error.h"
#include "no_os_util.h"

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

/* STM32 GPIO IRQ specific parameters */
struct stm32_gpio_irq_init_param stm32_trigger_gpio_irq_init_params = {
	.port_nb = GPIO_TRIGGER_INT_PORT
};

/* SPI STM32 Platform Specific Init Parameters */
struct stm32_spi_init_param stm32_spi_init_params = {
	.chip_select_port = STM32_SPI_CS_PORT,
	.get_input_clock = HAL_RCC_GetPCLK2Freq
};

/* SPI STM32 Platform Specific Init Parameters */
struct stm32_spi_init_param stm32_spi_init_params_without_sw_csb = {
	.chip_select_port = STM32_SPI_CS_PORT,
	.get_input_clock = HAL_RCC_GetPCLK2Freq
};

/* LDAC pin STM32 GPIO specific parameters */
struct stm32_gpio_init_param stm32_gpio_ldac_init_params = {
	.mode = GPIO_MODE_AF_PP,
	.speed = GPIO_SPEED_FREQ_VERY_HIGH,
	.alternate = GPIO_AF9_TIM12
};

/* GPIO pin for timer output which will stop spi dma transfer */
struct stm32_gpio_init_param stm32_spi_dma_tx_stop_pwm_gpio_init_params
	= {
	.mode = GPIO_MODE_AF_PP,
	.speed = GPIO_SPEED_FREQ_VERY_HIGH,
	.alternate = GPIO_AF2_TIM4
};

/* Reset pin STM32 GPIO specific parameters */
struct stm32_gpio_init_param stm32_gpio_reset_init_params = {
	.mode = GPIO_MODE_OUTPUT_PP
};

/* STM32 LDAC PWM specific parameters */
struct stm32_pwm_init_param stm32_ldac_pwm_init_params = {
	.prescaler = LDAC_PWM_PRESCALER,
	.timer_autoreload = true,
	.mode = TIM_OC_PWM1,
	.timer_chn = LDAC_PWM_CHANNEL,
	.get_timer_clock = HAL_RCC_GetPCLK1Freq,
	.clock_divider = LDAC_PWM_CLK_DIVIDER
};

/* STM32 PWM specific parameters used to stop spi dma transfer */
struct stm32_pwm_init_param stm32_spi_dma_tx_stop_pwm_init_params = {
	.prescaler = SPI_DMA_TX_STOP_PWM_PRESCALER,
	.timer_autoreload = true,
	.mode = TIM_OC_PWM1,
	.timer_chn = SPI_DMA_TX_STOP_PWM_CHANNEL,
	.get_timer_clock = HAL_RCC_GetPCLK1Freq,
	.clock_divider = SPI_DMA_TX_STOP_PWM_CLK_DIVIDER
};

/* Dummy receive buffer for spi dma */
static uint8_t spi_dma_rx_buf[6];

/* look up table for SPI DMA transfer stop pwm frequency
   note: this is mcu specific and number of bytes to be transfered
   per ldac cycle */
uint32_t spi_dma_tx_stop_pwm_frequency[NUMBER_OF_CHANNELS] = { 3214285, 319182 };

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
	MX_GPIO_Init();
	MX_SPI1_Init();
	MX_UART5_Init();
	MX_TIM12_Init();
#if (INTERFACE_MODE == SPI_DMA)
	MX_TIM4_Init();
	MX_DMA_Init();
#endif
}

/**
  * @brief This function handles the LDAC GPIO interrupt event
  * @return None
  */
void EXTI15_10_IRQHandler(void)
{
#if (INTERFACE_MODE == SPI_DMA)
	if (__HAL_GPIO_EXTI_GET_IT(1 << LDAC_PIN) != 0) {
		__HAL_GPIO_EXTI_CLEAR_IT(1 << LDAC_PIN);

		/* start the dma transfer */
		hspi1.Instance->CR2 |= SPI_CR2_TXDMAEN | SPI_CR2_RXDMAEN;

		/* start timer*/
		htim4.Instance->CNT = 0;
		htim4.Instance->CR1 |= TIM_CR1_CEN;
	}
#else
	HAL_GPIO_EXTI_IRQHandler(1 << LDAC_PIN);
#endif
}

/**
  * @brief This function handles the timer interrupt event
           which is used to stop the spi dma transfer
  * @return None
  */
void TIM4_IRQHandler(void)
{
	/* Disable the spi dma Tx requests */
	hspi1.Instance->CR2 &= ~(SPI_CR2_TXDMAEN);

	__HAL_TIM_CLEAR_IT(&htim4, TIM_IT_CC1);

	/* stop timer */
	htim4.Instance->CR1 &= ~TIM_CR1_CEN;
	htim4.Instance->CNT = 0;
}

/**
 * @brief 	Enables SPI DMA to move data from iio buffer
 *          to SPI TX buffer
 * @param   desc[in]- Pointer to ad3552r device descriptor
 * @param   iio_dev_data[in]- Pointer to IIO device data structure
 * @param   num_of_bytes_transfer[in]- number of bytes to transfer
            for each ldac cycle.
 * @param   start_addr[in]- starting address need to be transferred
            in streaming mode of ad3552r
 * @return	0 in case of success, negative error code otherwise
 */
int32_t stm32_spi_dma_enable(struct stm32_spi_desc* spidesc,
			     struct iio_device_data* iio_dev_data, uint16_t num_of_bytes_transfer,
			     uint8_t start_addr)
{
	struct stm32_gpio_desc* gpiodesc;
	int32_t ret;

	gpiodesc = spidesc->chip_select->extra;

	ret = HAL_DMA_Start(hspi1.hdmarx, (uint32_t)&hspi1.Instance->DR, spi_dma_rx_buf,
			    num_of_bytes_transfer);
	if (ret) {
		return ret;
	}

	ret = HAL_DMA_Start(hspi1.hdmatx, iio_dev_data->buffer->buf->buff,
			    (uint32_t)&hspi1.Instance->DR, iio_dev_data->buffer->size);
	if (ret) {
		return ret;
	}

	/* Start PWM timer which will stop SPI-DMA transactions */
	ret = HAL_TIM_PWM_Start_IT(&htim4, TIM_CHANNEL_1);
	if (ret) {
		return ret;
	}

	/* Stop PWM timer which will stop SPI-DMA transactions*/
	htim4.Instance->CR1 &= ~TIM_CR1_CEN;
	htim4.Instance->CNT = 0;

	/* Assert the chip select pin */
	gpiodesc->port->BSRR = NO_OS_BIT(spidesc->chip_select->number) << 16;

	__HAL_SPI_ENABLE(&hspi1);

	/* sending starting address of channel register */
	hspi1.Instance->DR = start_addr;

	return 0;
}

/**
 * @brief 	Disable SPI DMA which move data from iio buffer
 *          to SPI TX buffer
 * @param   desc[in]- Pointer to ad3552r device descriptor
 * @return	0 in case of success, negative error code otherwise
 */
int32_t stm32_spi_dma_disable(struct stm32_spi_desc* spidesc)
{
	struct stm32_gpio_desc* gpiodesc;
	int32_t ret;

	gpiodesc = spidesc->chip_select->extra;

	/* disable the dma */
	ret = HAL_DMA_Abort(hspi1.hdmarx);
	if (ret) {
		return ret;
	}

	ret = HAL_DMA_Abort(hspi1.hdmatx);
	if (ret) {
		return ret;
	}

	hspi1.Instance->CR2 &= ~(SPI_CR2_TXDMAEN | SPI_CR2_RXDMAEN);

	/* Disable spi */
	__HAL_SPI_DISABLE(&hspi1);

	/* De-assert the chip select pin */
	gpiodesc->port->BSRR = NO_OS_BIT(spidesc->chip_select->number);

	/* Stop PWM timer which will stop SPI-DMA transactions*/
	ret = HAL_TIM_PWM_Stop_IT(&htim4, TIM_CHANNEL_1);
	if (ret) {
		return ret;
	}

	return 0;
}
