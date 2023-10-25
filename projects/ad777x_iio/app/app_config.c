/***************************************************************************//**
 *   @file    app_config.c
 *   @brief   Application configurations module for AD777x firmware application
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

#include "app_config.h"
#include "common.h"
#include "no_os_gpio.h"
#include "no_os_irq.h"
#include "no_os_tdm.h"
#include "no_os_pwm.h"

/******************************************************************************/
/************************ Macros/Constants ************************************/
/******************************************************************************/

/******************************************************************************/
/*************************** Types Declarations *******************************/
/******************************************************************************/

/* UART init parameters for IIO comm port */
struct no_os_uart_init_param uart_iio_comm_init_params = {
	.device_id = UART_DEVICE_ID,
	.baud_rate = IIO_UART_BAUD_RATE,
	.size = NO_OS_UART_CS_8,
	.parity = NO_OS_UART_PAR_NO,
	.stop = NO_OS_UART_STOP_1_BIT,
#if (ACTIVE_PLATFORM == STM32_PLATFORM)
	.asynchronous_rx = true,
	.irq_id = UART_IRQ_ID,
#endif
#if defined(USE_VIRTUAL_COM_PORT)
	.platform_ops = &vcom_ops,
	.extra = &vcom_extra_init_params
#else
	.platform_ops = &uart_ops,
	.extra = &uart_extra_init_params
#endif
};

/* UART init parameters for console comm port */
struct no_os_uart_init_param uart_console_stdio_init_params = {
	.device_id = UART_DEVICE_ID,
	.asynchronous_rx = false,
	.baud_rate = IIO_UART_BAUD_RATE,
	.size = NO_OS_UART_CS_8,
	.parity = NO_OS_UART_PAR_NO,
	.stop = NO_OS_UART_STOP_1_BIT,
#if defined(USE_VIRTUAL_COM_PORT)
	/* If virtual com port is primary IIO comm port, use physical port for stdio
	 * console. Applications which does not support VCOM, should not satisfy this
	 * condition */
	.platform_ops = &uart_ops,
	.extra = &uart_extra_init_params
#else
#if defined(CONSOLE_STDIO_PORT_AVAILABLE)
	/* Applications which uses phy COM port as primary IIO comm port,
	 * can use VCOM as console stdio port provided it is available.
	 * Else, alternative phy com port can be used for console stdio ops if available */
	.platform_ops = &vcom_ops,
	.extra = &vcom_extra_init_params
#endif
#endif
};

struct no_os_gpio_init_param drdy_init_param = {
	.number = GPIO_DRDY_PIN,
	.port = GPIO_DRDY_PORT,
	.extra = &gpio_drdy_extra_init_params,
	.platform_ops = &gpio_platform_ops
};

/* Trigger GPIO IRQ parameters */
struct no_os_irq_init_param trigger_gpio_irq_params = {
	.irq_ctrl_id = DRDY_IRQ_CTRL_ID,
	.platform_ops = &trigger_gpio_irq_ops,
	.extra = &trigger_gpio_irq_extra_params
};

/* External interrupt callback descriptor */
static struct no_os_callback_desc ext_int_callback_desc = {
	.callback = data_capture_callback,
	.event = NO_OS_EVT_GPIO,
	.peripheral = NO_OS_GPIO_IRQ,
};

#if (INTERFACE_MODE == TDM_MODE)
/* SAI-TDM init parameters */
struct no_os_tdm_init_param tdm_init_param = {
	.mode = NO_OS_TDM_SLAVE_RX,
	.data_size = TDM_DATA_SIZE,
	.data_offset = 0,
	.data_lsb_first = false,
	.slots_per_frame = TDM_SLOTS_PER_FRAME,
	.fs_active_low = true,
	.fs_active_length = TDM_FS_ACTIVE_LENGTH,
	.fs_lastbit = false,
	.rising_edge_sampling = true,
	.irq_id = DMA_IRQ_ID,
	.rx_complete_callback = ad777x_dma_rx_cplt,
#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
	.rx_half_complete_callback = ad777x_dma_rx_half_cplt,
#endif
	.platform_ops = &tdm_platform_ops,
	.extra = &tdm_extra_init_params
};

struct no_os_tdm_desc *ad777x_tdm_desc;
#endif

/* I2C init parameters */
static struct no_os_i2c_init_param no_os_i2c_init_params = {
	.device_id = I2C_DEVICE_ID,
	.platform_ops = &i2c_ops,
	.max_speed_hz = 100000,
#if (ACTIVE_PLATFORM == MBED_PLATFORM)
	.extra = &i2c_extra_init_params
#endif
};

/* EEPROM init parameters */
static struct eeprom_24xx32a_init_param eeprom_extra_init_params = {
	.i2c_init = &no_os_i2c_init_params
};

/* EEPROM init parameters */
static struct no_os_eeprom_init_param eeprom_init_params = {
	.device_id = 0,
	.platform_ops = &eeprom_24xx32a_ops,
	.extra = &eeprom_extra_init_params
};

/* Error LED init parameters */
struct no_os_gpio_init_param gpio_error_init_param = {
	.number = GPIO_ERROR_LED,
	.port = GPIO_ERROR_LED_PORT,
	.extra = &gpio_error_extra_init_params,
	.platform_ops = &gpio_platform_ops
};

/* PWM Init Parameters */
struct no_os_pwm_init_param mclk_pwm_init_param = {
	.id = MCLK_PWM_ID,
	.period_ns = AD777x_MCLK_PERIOD,
	.duty_cycle_ns = AD777x_MCLK_PERIOD / 2,
	.platform_ops = &pwm_ops,
	.extra = &pwm_extra_init_params
};

/* UART IIO Descriptor */
struct no_os_uart_desc *uart_iio_com_desc;

/* UART console descriptor */
struct no_os_uart_desc *uart_console_stdio_desc;

struct no_os_gpio_desc *gpio_drdy_desc;

/* Error GPIO Descriptor */
struct no_os_gpio_desc *gpio_error_desc;

struct no_os_irq_ctrl_desc *trigger_irq_desc;

/* EEPROM descriptor */
struct no_os_eeprom_desc *eeprom_desc;

/*External MCLK PWM descriptor */
struct no_os_pwm_desc *ext_mclk_pwm_desc;

/******************************************************************************/
/************************ Functions Prototypes ********************************/
/******************************************************************************/

/******************************************************************************/
/************************ Functions Definitions *******************************/
/******************************************************************************/

/**
 * @brief 	Initialize the UART peripheral
 * @return	0 in case of success, negative error code otherwise
 */
static int32_t init_uart(void)
{
	int32_t ret;

	ret = no_os_uart_init(&uart_iio_com_desc, &uart_iio_comm_init_params);
	if (ret) {
		return ret;
	}

#if defined(CONSOLE_STDIO_PORT_AVAILABLE)
	/* Initialize the serial link for console stdio communication */
	ret = no_os_uart_init(&uart_console_stdio_desc,
			      &uart_console_stdio_init_params);
	if (ret) {
		return ret;
	}
#endif

	return 0;
}


/**
 * @brief 	Initialize the GPIO peripheral
 * @return	0 in case of success, negative error code otherwise
 */
static int32_t init_gpio(void)
{
	int32_t ret;

#if (INTERFACE_MODE == SPI_MODE)
	ret =  no_os_gpio_get(&gpio_drdy_desc, &drdy_init_param);
	if (ret) {
		return ret;
	}

	ret = no_os_gpio_direction_input(gpio_drdy_desc);
	if (ret) {
		return ret ;
	}
#endif

	ret =  no_os_gpio_get(&gpio_error_desc, &gpio_error_init_param);
	if (ret) {
		return ret;
	}

	ret = no_os_gpio_direction_output(gpio_error_desc, NO_OS_GPIO_LOW);
	if (ret) {
		return ret ;
	}

	return 0;
}


/**
 * @brief Initialize the trigger GPIO and associated IRQ event
 * @return 0 in case of success, negative error code otherwise
 */
static int32_t gpio_trigger_init(void)
{
	int32_t ret;

	/* Initialize the IRQ controller */
	ret = no_os_irq_ctrl_init(&trigger_irq_desc, &trigger_gpio_irq_params);
	if (ret) {
		return ret;
	}

#if (DATA_CAPTURE_MODE == BURST_DATA_CAPTURE)
	/* The RDY pin has been tied as the interrupt source to sense the
	 * End of Conversion. The registered callback function is responsible
	 * of reading the raw samples via the SPI bus */
	ret = no_os_irq_register_callback(trigger_irq_desc, IRQ_INT_ID,
					  &ext_int_callback_desc);
	if (ret) {
		return ret;
	}

	ret = no_os_irq_trigger_level_set(trigger_irq_desc, IRQ_INT_ID,
					  NO_OS_IRQ_EDGE_FALLING);
	if (ret) {
		return ret;
	}
#endif
	return 0;
}


/**
 * @brief Initialize the TDM Peripheral
 * @return 0 in case of success, negative error code otherwise
 */
int32_t init_tdm(void)
{
	int32_t ret;

#if (INTERFACE_MODE == TDM_MODE)
	ret = no_os_tdm_init(&ad777x_tdm_desc, &tdm_init_param);
	if (ret) {
		return ret;
	}
#endif

	return 0;
}

/**
 * @brief Initialize the PWM Peripheral
 * @return 0 in case of success, negative error code otherwise
 */
static int32_t init_pwm(void)
{
	int32_t ret;

#if defined (ENABLE_EXT_MCLK)
	ret = no_os_pwm_init(&ext_mclk_pwm_desc, &mclk_pwm_init_param);
	if (ret) {
		return ret;
	}

	/* Enable the MCLK PWM */
	ret = no_os_pwm_enable(ext_mclk_pwm_desc);
	if (ret) {
		return ret;
	}
#endif

	return 0;
}

/**
 * @brief 	Initialize the system peripherals
 * @return	0 in case of success, negative error code otherwise
 */
int32_t init_system(void)
{
	int32_t ret;

#if (ACTIVE_PLATFORM == STM32_PLATFORM)
	stm32_system_init();
#endif

	ret = init_uart();
	if (ret) {
		return ret;
	}

	ret = init_tdm();
	if (ret) {
		return ret;
	}

	ret = init_gpio();
	if (ret) {
		return ret;
	}

	ret = gpio_trigger_init();
	if (ret) {
		return ret;
	}

#if defined(USE_SDRAM)
	ret = sdram_init();
	if (ret) {
		return ret;
	}
#endif

#if (ACTIVE_PLATFORM == MBED_PLATFORM)
	ret = eeprom_init(&eeprom_desc, &eeprom_init_params);
	if (ret) {
		return ret;
	}
#endif

	ret = init_pwm();
	if (ret) {
		return ret;
	}

	return 0;
}
