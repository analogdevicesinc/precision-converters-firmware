/***************************************************************************//**
 *   @file    app_config_stm32.c
 *   @brief   STM32 Specific configuration files for AD552XR IIO Application
 *   @details This module contains the STM32 platform specific configurations
********************************************************************************
 * Copyright (c) 2026 Analog Devices, Inc.
 *
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 * By using this software you agree to the terms of the associated
 * Analog Devices Software License Agreement.
*******************************************************************************/

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/
#include "app_config_stm32.h"
#include "app_config.h"

/******************************************************************************/
/********************** Macros and Constants Definitions **********************/
/******************************************************************************/

/******************************************************************************/
/********************** Variables and User Defined Data Types *****************/
/******************************************************************************/
#if (INTERFACE_MODE == SPI_DMA)
/* Tx DMA Channel Extra Init Params */
struct stm32_dma_channel txdma_channel = {
	.hdma = &TX_DMA_CH_HANDLE,
	.ch_num = TX_DMA_CH_ID,
	.mem_increment = true,
	.mem_data_alignment = DATA_ALIGN_BYTE,
	.per_data_alignment = DATA_ALIGN_BYTE,
	.dma_mode = DMA_CIRCULAR_MODE
};

/* Rx DMA Channel Extra Init Params */
struct stm32_dma_channel rxdma_channel = {
	.hdma = &RX_DMA_CH_HANDLE,
	.ch_num = RX_DMA_CH_ID,
	.mem_increment = false,
	.mem_data_alignment = DATA_ALIGN_BYTE,
	.per_data_alignment = DATA_ALIGN_BYTE,
	.dma_mode = DMA_CIRCULAR_MODE
};

/* CS GPIO Extra Init Params in PWM mode */
struct stm32_gpio_init_param stm32_gpio_cs_pwm_extra_init_params = {
	.mode = GPIO_MODE_AF_PP,
	.speed = GPIO_SPEED_FREQ_HIGH,
	.alternate = GPIO_AF1_TIM2,
};
#endif

/* SPI Extra Init Params */
struct stm32_spi_init_param stm32_spi_extra_init_params = {
	.chip_select_port = SPI_CSB_PORT,
	.get_input_clock = HAL_RCC_GetPCLK2Freq,
#if (INTERFACE_MODE == SPI_INTERRUPT)
	.dma_init = NULL,
	.rxdma_ch = NULL,
	.txdma_ch = NULL,
	.irq_num = 0,
#elif (INTERFACE_MODE == SPI_DMA)
	.dma_init = NULL,
	.rxdma_ch = &rxdma_channel,
	.txdma_ch = &txdma_channel,
	.irq_num = Rx_DMA_IRQ_ID,
#endif
	.alternate = 0,
};

/*  I2C Extra Init Params */
struct stm32_i2c_init_param stm32_i2c_extra_init_params = {
	.i2c_timing = I2C_TIMING
};

/* GPIO Extra Init Params in GPIO Output mode */
struct stm32_gpio_init_param stm32_gpio_output_extra_init_params = {
	.mode = GPIO_MODE_OUTPUT_PP,
	.speed = GPIO_SPEED_FREQ_HIGH,
	.alternate = 0
};

/* GPIO Extra Init Params in GPIO Input mode */
struct stm32_gpio_init_param stm32_gpio_input_extra_init_params = {
	.mode = GPIO_MODE_INPUT,
	.speed = GPIO_SPEED_FREQ_HIGH,
	.alternate = 0
};

/* LDAC_TGP GPIO Extra Init Params */
struct stm32_gpio_init_param stm32_gpio_ldac_tgp_pwm_extra_init_params = {
	.speed = GPIO_SPEED_FREQ_HIGH,
	.mode = GPIO_MODE_AF_PP,
	.alternate = GPIO_AF1_TIM1,
};

/* VCOM Extra Init Params */
struct stm32_usb_uart_init_param stm32_vcom_extra_init_params = {
	.husbdevice = &APP_UART_USB_HANDLE
};

/* UART Extra Init Params */
struct stm32_uart_init_param stm32_uart_extra_init_params = {
	.huart = &UART_HANDLE
};

/* STM32 PWM TGP (Free running PWM) Extra Init parameters */
struct stm32_pwm_init_param stm32_pwm_tgp_extra_init_params = {
	.htimer = &TIM_TGP_HANDLE,
	.prescaler = TIM_TGP_PRESCALER,
	.timer_autoreload = true,
	.mode = TIM_OC_PWM2,
	.timer_chn = TIM_TGP_CH_ID,
	.complementary_channel = true,
	.get_timer_clock = HAL_RCC_GetPCLK2Freq,
	.clock_divider = TIM_TGP_CLK_DIVIDER,
	.slave_mode = STM32_PWM_SM_DISABLE,
	.dma_enable = false,
	.repetitions = 0,
	.onepulse_enable = false
};

/* STM32 PWM TGP in Trigger Mode Extra Init parameters */
struct stm32_pwm_init_param stm32_pwm_tgp_trigger_mode_extra_init_params = {
	.htimer = &TIM_TGP_HANDLE,
	.prescaler = TIM_TGP_PRESCALER,
	.timer_autoreload = true,
	.mode = TIM_OC_PWM2,
	.timer_chn = TIM_TGP_CH_ID,
	.complementary_channel = true,
	.get_timer_clock = HAL_RCC_GetPCLK2Freq,
	.clock_divider = TIM_TGP_CLK_DIVIDER,
	.slave_mode = STM32_PWM_SM_TRIGGER,
	.trigger_source = PWM_TS_ITR1,
	.dma_enable = false,
	.repetitions = 0,
	.onepulse_enable = true
};

/* STM32 PWM DAC UPDATE Extra Init parameters */
struct stm32_pwm_init_param stm32_pwm_dac_update_extra_init_params = {
	.htimer = &TIM_DAC_UPDATE_HANDLE,
	.prescaler = TIM_DAC_UPDATE_PRESCALER,
	.mode = TIM_OC_PWM2,
	.timer_chn = TIM_DAC_UPDATE_CH_ID,
	.complementary_channel = false,
	.timer_autoreload = true,
	.get_timer_clock = HAL_RCC_GetPCLK1Freq,
	.clock_divider = TIM_DAC_UPDATE_CLK_DIVIDER,
	.repetitions = 0,
	.onepulse_enable = false,
	.dma_enable = false,
	.slave_mode = STM32_PWM_SM_DISABLE,
	.trigger_output = PWM_TRGO_OC1,
};

#if (INTERFACE_MODE == SPI_DMA)
/* DMA Trigger Timer Extra Init parameters */
struct stm32_pwm_init_param stm32_pwm_dma_trigger_extra_init_params = {
	.htimer = &TIM_DMA_TRIGGER_HANDLE,
	.prescaler = TIM_DMA_TRIGGER_PRESCALER,
	.timer_autoreload = false,
	.mode = TIM_OC_PWM2,
	.timer_chn = TIM_DMA_TRIGGER_CH_ID,
	.complementary_channel = false,
	.get_timer_clock = HAL_RCC_GetPCLK2Freq,
	.clock_divider = TIM_DMA_TRIGGER_CLK_DIVIDER,
	.slave_mode = STM32_PWM_SM_TRIGGER,
	.trigger_source = PWM_TS_ITR1,
	.trigger_output = PWM_TRGO_RESET,
	.dma_enable = true,
	.repetitions = sizeof(uint32_t) - 1,
	.onepulse_enable = true
};
#endif

/******************************************************************************/
/************************** Functions Declarations ****************************/
/******************************************************************************/
extern void SystemClock_Config(void);

/******************************************************************************/
/************************** Functions Definitions *****************************/
/******************************************************************************/
/**
 * @brief 	Initialize the STM32 system peripherals
 * @return	None
 */
int32_t stm32_init_system(void)
{
	/* Initialize HAL layer */
	HAL_Init();

	/* Provide some delay to initialize LL */
	HAL_Delay(2000);

	/* Initialize System Clocks */
	SystemClock_Config();

	/* Initialize UART */
	MX_UART5_Init();

	/* Initialize GPIO */
	MX_GPIO_Init();

	/* Initialize SPI */
	MX_SPI1_Init();

	/* Initialize I2C */
	MX_I2C1_Init();

	/* Initialize TGP Timer */
	MX_TIM1_Init();

	/* Initialize DAC update timer */
	MX_TIM2_Init();

#if (INTERFACE_MODE == SPI_DMA)
	/* Initialize DMA */
	MX_DMA_Init();

	/* Initialize DMA trigger timer */
	MX_TIM8_Init();
#endif

	/* Initialize USB */
	MX_USB_DEVICE_Init();

	return 0;
}
