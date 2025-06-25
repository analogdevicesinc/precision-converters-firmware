/***************************************************************************//**
 * @file    app_config_stm32.c
 * @brief   STM32 Specific configuration files for AD4080 IIO Application
 * @details This module contains the STM32 platform specific configurations
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

#include <stdint.h>

#include "app_config.h"

/******************************************************************************/
/********************* Macros and Constants Definition ************************/
/******************************************************************************/

/* Number of period taken into consideration for calculating oscillator frequency */
#define NUM_PERIODS	30

/******************************************************************************/
/******************** Variables and User Defined Data Types *******************/
/******************************************************************************/
/* Configuration SPI Extra Init Parameters */
struct stm32_spi_init_param stm32_config_spi_extra_init_params = {
	.chip_select_port = SPI_CS_PORT,
	.get_input_clock = HAL_RCC_GetPCLK2Freq
};

/* Data SPI Extra Init Parameters */
struct stm32_spi_init_param stm32_data_spi_extra_init_params = {
	.chip_select_port = SPI_DCS_CSB_PORT,
	.get_input_clock = HAL_RCC_GetPCLK2Freq
};

#ifdef USE_QUAD_SPI
/* DMA init params for SPI RX DMA */
struct no_os_dma_init_param stm32_qspi_dma_init_param = {
	.id = 0,
	.num_ch = QSPI_DMA_NUM_CH,
	.platform_ops = &stm32_gpdma_ops,
	.sg_handler = NULL,
};

/* QSPI DMA channel Init Parameters */
struct stm32_dma_channel stm32_qspi_dma_ch = {
	.hdma = &QSPI_DMA_HANDLE,
	.ch_num = QSPI_DMA_CH,
	.mem_increment = true,
	.per_increment = false,
	.mem_data_alignment = DATA_ALIGN_BYTE,
	.per_data_alignment = DATA_ALIGN_BYTE,
	.dma_mode = DMA_NORMAL_MODE,
};

/* XSPI command */
struct stm32_xspi_command xspi_cmd = {
	.Address = 0,
	.AddressMode = HAL_XSPI_ADDRESS_1_LINE,
	.AddressWidth = HAL_XSPI_ADDRESS_8_BITS,
	.AlternateBytes = HAL_XSPI_ALT_BYTES_NONE,
	.AlternateBytesMode = HAL_XSPI_ALT_BYTES_8_BITS,
	.AlternateBytesWidth = 0,
	.DataLength = 0,
	.DataMode = HAL_XSPI_DATA_4_LINES,
	.DummyCycles = 0,
	.Instruction = 0,
	.InstructionMode = HAL_XSPI_INSTRUCTION_NONE,
	.InstructionWidth = HAL_XSPI_INSTRUCTION_8_BITS,
};

/* Data QSPI Extra Init Parameters */
struct stm32_xspi_init_param stm32_data_qspi_extra_init_params = {
	.fifo_threshold = 1,
	.cmd = &xspi_cmd,
	.get_input_clock = HAL_RCC_GetHCLKFreq,
	.dma_init = &stm32_qspi_dma_init_param,
	.dma_ch = &stm32_qspi_dma_ch,
	.irq_num = QSPI_DMA_IRQ
};
#endif

/* UART Init Parameters */
struct stm32_uart_init_param stm32_uart_extra_init_params = {
	.huart = &APP_UART_HANDLE
};

#if defined (TARGET_SDP_K1)
/* VCOM Init Parameters */
struct stm32_usb_uart_init_param stm32_vcom_extra_init_params = {
	.husbdevice = &APP_UART_USB_HANDLE
};
#endif

/* XTAL_OSC_EN GPIO Init Parameters */
struct stm32_gpio_init_param stm32_gpio_xtal_osc_en_init_params = {
	.mode = GPIO_MODE_OUTPUT_PP,
	.speed = GPIO_SPEED_FREQ_LOW
};

/* GP1 GPIO Init Parameters */
struct stm32_gpio_init_param stm32_gpio_gp1_init_params = {
	.mode = GPIO_MODE_OUTPUT_PP,
	.speed = GPIO_SPEED_FREQ_LOW
};

/* GP2 GPIO Init Parameters */
struct stm32_gpio_init_param stm32_gpio_gp2_init_params = {
	.mode = GPIO_MODE_INPUT,
	.speed = GPIO_SPEED_FREQ_LOW
};

/* GP3 GPIO Init Parameters */
struct stm32_gpio_init_param stm32_gpio_gp3_init_params = {
	.mode = GPIO_MODE_INPUT,
	.speed = GPIO_SPEED_FREQ_LOW
};

/* 40M Oscillator Enable GPIO Init Parameters */
struct stm32_gpio_init_param stm32_gpio_40m_osc_init_params = {
	.mode = GPIO_MODE_OUTPUT_PP,
	.speed = GPIO_SPEED_FREQ_LOW
};

/* 20M Oscillator Enable GPIO Init Parameters */
struct stm32_gpio_init_param stm32_gpio_20m_osc_init_params = {
	.mode = GPIO_MODE_OUTPUT_PP,
	.speed = GPIO_SPEED_FREQ_LOW
};

/* 10M Oscillator Enable GPIO Init Parameters */
struct stm32_gpio_init_param stm32_gpio_10m_osc_init_params = {
	.mode = GPIO_MODE_OUTPUT_PP,
	.speed = GPIO_SPEED_FREQ_LOW
};

/* AFE CTRL GPIO Init Parameters **/
struct stm32_gpio_init_param stm32_gpio_afe_ctrl_init_params = {
	.mode = GPIO_MODE_OUTPUT_PP,
	.speed = GPIO_SPEED_FREQ_LOW
};

/* I2C extra init parameters */
struct stm32_i2c_init_param stm32_i2c_extra_init_params = {
	.i2c_timing = I2C_TIMING
};

/******************************************************************************/
/************************** Functions Declaration *****************************/
/******************************************************************************/
/* System Clock Configuration */
extern void SystemClock_Config(void);

/******************************************************************************/
/************************** Functions Definition ******************************/
/******************************************************************************/
/**
 * @brief 	Initialize the STM32 system peripherals
 * @return	None
 */
void stm32_system_init(void)
{
	/* Initialize HAL layer */
	HAL_Init();

	/* Initialize System Clocks */
	SystemClock_Config();

	/* Initialize GPIOs */
	MX_GPIO_Init();

#if !defined (TARGET_SDP_K1)
	/* Initialize DMA */
	MX_GPDMA1_Init();

	/* Initialize Console UART */
	MX_USART3_UART_Init();
#else
	/* Initialize Console UART */
	MX_UART5_Init();
#endif

	/* Initialize SPI */
	MX_SPI1_Init();

#ifdef USE_VIRTUAL_COM_PORT
	/* Initialize USB device */
	MX_USB_DEVICE_Init();
#endif
}
