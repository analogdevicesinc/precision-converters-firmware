/***************************************************************************//**
 * @file    app_config_stm32.c
 * @brief   Source file for STM32 platform configurations
********************************************************************************
* Copyright (c) 2024-2026 Analog Devices, Inc.
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
#include "app_config.h"
#include "no_os_error.h"
#include "ad5706r_user_config.h"

/******************************************************************************/
/************************ Macros/Constants ************************************/
/******************************************************************************/

/******************************************************************************/
/******************** Variables and User Defined Data Types *******************/
/******************************************************************************/

/* STM32 SPI parameters */
struct stm32_spi_init_param stm32_spi_extra_init_params = {
	.chip_select_port = SPI_CS_PORT_NUM,
	.get_input_clock = HAL_RCC_GetPCLK2Freq,
	.alternate = GPIO_AF1_TIM2
};

/* STM32 I2C parameters */
struct stm32_i2c_init_param stm32_i2c_extra_init_params = {
	.i2c_timing = I2C_TIMING
};

/* STM32 GPIO AD0 parameters */
struct stm32_gpio_init_param stm32_gpio_ad0_init_params = {
	.mode = GPIO_MODE_OUTPUT_PP,
	.speed = GPIO_SPEED_FREQ_VERY_HIGH,
};

/* STM32 GPIO AD1 parameters */
struct stm32_gpio_init_param stm32_gpio_ad1_init_params = {
	.mode = GPIO_MODE_OUTPUT_PP,
	.speed = GPIO_SPEED_FREQ_VERY_HIGH,
};

/* STM32 GPIO LDAC TG parameters */
struct stm32_gpio_init_param stm32_gpio_ldac_tg_init_params = {
	.mode = GPIO_MODE_OUTPUT_PP,
	.speed = GPIO_SPEED_FREQ_VERY_HIGH,
};

/* STM32 GPIO Shutdown parameters */
struct stm32_gpio_init_param stm32_gpio_shutdown_init_params = {
	.mode = GPIO_MODE_OUTPUT_PP,
	.speed = GPIO_SPEED_FREQ_VERY_HIGH,
};

/* STM32 GPIO Reset parameters */
struct stm32_gpio_init_param stm32_gpio_reset_init_params = {
	.mode = GPIO_MODE_OUTPUT_PP,
	.speed = GPIO_SPEED_FREQ_VERY_HIGH,
};

/* STM32 PWM LDAC parameters */
struct stm32_pwm_init_param stm32_ldac_pwm_init_params = {
	.htimer = &PWM_TIMER_HANDLE,
	.prescaler = TIMER_1_PRESCALER,
	.timer_autoreload = true,
	.mode = TIM_OC_PWM2,
	.timer_chn = TIMER_CHANNEL_3,
	.complementary_channel = true,
	.get_timer_clock = HAL_RCC_GetPCLK2Freq,
	.clock_divider = TIMER_1_CLK_DIVIDER
};

/* STM32 PWM LDAC parameters */
struct stm32_pwm_init_param stm32_dac_update_pwm_init_params = {
	.htimer = &DAC_UPDATE_TIMER_HANDLE,
	.prescaler = TIMER_4_PRESCALER,
	.timer_autoreload = true,
	.mode = TIM_OC_PWM1,
	.timer_chn = TIMER_CHANNEL_1,
	.complementary_channel = false,
	.get_timer_clock = HAL_RCC_GetPCLK1Freq,
	.clock_divider = TIMER_4_CLK_DIVIDER,
#if (INTERFACE_MODE == SPI_DMA)
	.trigger_output = PWM_TRGO_UPDATE,
#endif
};

/* STM32 GPIO IRQ specific parameters */
struct stm32_gpio_irq_init_param stm32_trigger_gpio_irq_init_params = {
	.port_nb = DAC_UPDATE_PORT
};

/* STM32 PWM GPIO specific parameters */
struct stm32_gpio_init_param stm32_ldac_pwm_gpio_extra_init_params = {
	.mode = GPIO_MODE_AF_PP,
	.speed = GPIO_SPEED_FREQ_VERY_HIGH,
	.alternate = GPIO_AF1_TIM1
};

/* STM32 PWM GPIO specific parameters */
struct stm32_gpio_init_param stm32_dac_update_pwm_gpio_extra_init_params = {
	.mode = GPIO_MODE_AF_PP,
	.speed = GPIO_SPEED_FREQ_VERY_HIGH,
	.alternate = GPIO_AF2_TIM4
};

#if (INTERFACE_MODE == SPI_DMA)
/* STM32 PWM specific init params */
struct stm32_pwm_init_param stm32_tx_trigger_extra_init_params = {
	.htimer = &TIMER8_HANDLE,
	.prescaler = TIMER_8_PRESCALER,
	.timer_autoreload = true,
	.mode = TIM_OC_TOGGLE,
	.timer_chn = TIMER_CHANNEL_1,
	.complementary_channel = false,
	.get_timer_clock = HAL_RCC_GetPCLK2Freq,
	.clock_divider = TIMER_8_CLK_DIVIDER,
	.slave_mode = STM32_PWM_SM_TRIGGER,
	.trigger_source = PWM_TS_ITR1,
	.trigger_output = PWM_TRGO_RESET,
	.dma_enable = true,
	.onepulse_enable = true
};

/* STM32 PWM for specific parameters */
struct stm32_pwm_init_param stm32_cs_extra_init_params = {
	.htimer = &CS_TIMER_HANDLE,
	.prescaler = CS_TIMER_PRESCALER,
	.timer_autoreload = false,
	.mode = TIM_OC_PWM2,
	.timer_chn = CS_TIMER_CHANNEL,
	.complementary_channel = false,
	.get_timer_clock = HAL_RCC_GetPCLK1Freq,
	.clock_divider = TIMER_2_CLK_DIVIDER,
	.trigger_output = PWM_TRGO_OC1,
	.slave_mode = STM32_PWM_SM_TRIGGER,
	.trigger_source = PWM_TS_ITR3,
};

/* STM32 Tx DMA channel extra init params */
struct stm32_dma_channel txdma_channel = {
	.hdma = &hdma_tim8_ch1,
	.ch_num = AD5706_TxDMA_CHANNEL_NUM,
	.mem_increment = true,
	.mem_data_alignment = DATA_ALIGN_BYTE,
	.per_data_alignment = DATA_ALIGN_BYTE,
	.dma_mode = DMA_CIRCULAR_MODE
};

/* STM32 Rx DMA channel extra init params */
struct stm32_dma_channel rxdma_channel = {
	.hdma = &hdma_spi1_rx,
	.ch_num = AD5706_RxDMA_CHANNEL_NUM,
	.mem_increment = false,
	.mem_data_alignment = DATA_ALIGN_BYTE,
	.per_data_alignment = DATA_ALIGN_BYTE,
	.dma_mode = DMA_CIRCULAR_MODE,
};

/* STM32 SPI Init Parameters */
struct stm32_spi_init_param* spi_init_param;
#endif

/* STM32 CS GPIO Extra init params in GPIO Mode */
struct stm32_gpio_init_param stm32_cs_gpio_extra_init_params = {
	.mode = GPIO_MODE_OUTPUT_PP,
	.speed = GPIO_SPEED_FREQ_VERY_HIGH,
};

/* STM32 CS GPIO Extra init params in PWM Mode */
struct stm32_gpio_init_param stm32_cs_pwm_gpio_extra_init_params = {
	.mode = GPIO_MODE_AF_PP,
	.speed = GPIO_SPEED_FREQ_VERY_HIGH,
	.alternate = GPIO_AF1_TIM2
};

#ifdef USE_VIRTUAL_COM_PORT
/* STM32 VCOM init parameters */
struct stm32_usb_uart_init_param stm32_vcom_extra_init_params = {
	.husbdevice = &hUsbDeviceHS,
};
#else
/* STM32 UART parameters */
struct stm32_uart_init_param stm32_uart_extra_init_params = {
	.huart = &huart5,
};
#endif

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
	MX_TIM1_Init();
	MX_TIM4_Init();
#if (INTERFACE_MODE == SPI_DMA)
	MX_TIM8_Init();
	MX_TIM2_Init();
	MX_DMA_Init();
#endif
#ifdef USE_VIRTUAL_COM_PORT
	MX_USB_DEVICE_Init();
#endif
}

/**
 * @brief 	EXTI Lines 10- 15 IRQ Handler
 * @return	None
 */
void EXTI15_10_IRQHandler(void)
{
	HAL_GPIO_EXTI_IRQHandler(1 << GPIO_DAC_UPDATE);
}

/**
 * @brief Callback function to flag the transfer of number
 *        of requested samples.
 * @param hdma - DMA Handler (Unused)
 * @return	None
 */
void ad5706r_rx_cplt_callback(DMA_HandleTypeDef* hdma)
{
	// TODO: Remove this if not needed
	/* Do nothing here */
}

/**
 * @brief Starts the timer signal generation.
 * @return	0 in case of success, negative error code otherwise
 */
int ad5706r_timers_enable(struct ad5706r_dev *device)
{
	int ret;

	/* Set all the counters to 0 */
	TIM1->CNT = 0;
	TIM2->CNT = 0;
	TIM8->CNT = 0;
	TIM4->CNT = 0;

	/* Enable LDAC only if there is any channel configured in HW Mode*/
	if (hw_mode_enabled) {
		ret = no_os_pwm_enable(ldac_pwm_desc);
		if (ret) {
			return ret;
		}
	}

	/* Enable DAC Update PWM only if there is any channel configured in Sw Mode*/
	if (sw_mode_enabled) {
		/* Enable Tx Trigger */
		ret = no_os_pwm_enable(tx_trigger_desc);
		if (ret) {
			return ret;
		}

		/* Enable DAC Update PWM */
		ret = no_os_pwm_enable(dac_update_pwm_desc);
		if (ret) {
			return ret;
		}
	}

	return 0;
}

/**
 * @brief Abort DMA Transfers
 * @return	0 in case of success, negative error code otherwise
 */
int ad5706r_abort_dma_transfers(struct ad5706r_dev *device)
{
	int ret;
	struct stm32_spi_desc* sdesc;
	sdesc = device->spi_desc->extra;

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
 * @brief Init Tx Trigger PWM
 * @param None
 * @return	0 in case of success, negative error code otherwise
 */
int ad5706r_init_tx_trigger(void)
{
#if (INTERFACE_MODE == SPI_DMA)
	int ret;

	stm32_tx_trigger_extra_init_params.repetitions = n_bytes - 1;

	/* Initialize the Tx Trigger PWM */
	ret = no_os_pwm_init(&tx_trigger_desc, &tx_trigger_init_params);
	if (ret) {
		return ret;
	}

	ret = no_os_pwm_disable(tx_trigger_desc);
	if (ret) {
		return ret;
	}
#endif

	return 0;
}
