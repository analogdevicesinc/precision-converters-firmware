/*******************************************************************************
 *   @file   app_config.c
 *   @brief  Application configurations module
********************************************************************************
Copyright 2025(c) Analog Devices, Inc.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of Analog Devices, Inc. nor the names of its
   contributors may be used to endorse or promote products derived from this
   software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY ANALOG DEVICES, INC. “AS IS” AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
EVENT SHALL ANALOG DEVICES, INC. BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*******************************************************************************/

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include <stdbool.h>

#include "app_config.h"
#include "cn0540_console_app.h"
#include "no_os_delay.h"

/******************************************************************************/
/************************ Macros/Constants ************************************/
/******************************************************************************/

/******************************************************************************/
/*************************** Types Declarations *******************************/
/******************************************************************************/

/* UART init parameters  */
struct no_os_uart_init_param uart_init_param = {
	.device_id = UART_DEVICE_ID,
	.baud_rate = UART_BAUD_RATE,
	.size = NO_OS_UART_CS_8,
	.parity = NO_OS_UART_PAR_NO,
	.stop = NO_OS_UART_STOP_1_BIT,
	.irq_id = UART_IRQ_ID,
	.asynchronous_rx = false,
	.platform_ops = &uart_ops,
	.extra = &uart_extra_init_params
};

/* Red LED init parameters */
const struct no_os_gpio_init_param gpio_red_led_param = {
	.port = RED_LED_PORT,
	.number = RED_LED_PIN,
	.platform_ops = &gpio_ops,
	.extra = &gpio_red_led_params
};

/* Blue LED init parameters */
const struct no_os_gpio_init_param gpio_blue_led_param = {
	.port = BLUE_LED_PORT,
	.number = BLUE_LED_PIN,
	.platform_ops = &gpio_ops,
	.extra = &gpio_blue_led_params
};

/* Reset GPIO init parameters */
const struct no_os_gpio_init_param gpio_rst_param = {
	.port = RESET_PORT,
	.number = RESET_PIN,
	.platform_ops = &gpio_ops,
	.extra = &gpio_rst_params
};

/* Buffer enable GPIO init parameters */
const struct no_os_gpio_init_param gpio_buf_en_param = {
	.port = BUF_EN_PORT,
	.number = BUF_EN_PIN,
	.platform_ops = &gpio_ops,
	.extra = &gpio_buf_en_params
};

/* Data ready trigger parameters */
struct no_os_irq_init_param trigger_gpio_irq_param = {
	.irq_ctrl_id = IRQ_CTRL_ID,
	.platform_ops = &gpio_irq_ops,
	.extra = &trigger_gpio_irq_params
};

/* External interrupt callback descriptor */
static struct no_os_callback_desc ext_int_callback_desc = {
	.callback = drdy_interrupt,
	.ctx = NULL,
	.event = NO_OS_EVT_GPIO,
	.peripheral = NO_OS_GPIO_IRQ
};

/*
 *  User-defined coefficients for programmable FIR filter, max 56 coeffs
 *
 *  Please note that, inserted coefficiets will be mirrored afterwards,
 *  so you must insert only one half of all the coefficients.
 *
 *  Please note your original filer must have ODD count of coefficients,
 *  allowing internal ADC circuitry to mirror the coefficients properly.
 *
 *	In case of usage lower count of coeffs than 56, please make sure, that
 *	the variable 'count_of_active_coeffs' bellow, carries the correct number
 *	of coeficients, allowing to fill the rest of the coeffs by zeroes
 *
 *	Default coeffs:
 **/
const uint8_t count_of_active_coeffs = 56;

const float programmable_FIR[56] = {

	-9.53674E-07,
		3.33786E-06,
		5.48363E-06,
		-5.48363E-06,
		-1.54972E-05,
		5.24521E-06,
		3.40939E-05,
		3.57628E-06,
		-6.17504E-05,
		-3.05176E-05,
		9.56059E-05,
		8.74996E-05,
		-0.000124693,
		-0.000186205,
		0.000128746,
		0.000333548,
		-7.70092E-05,
		-0.000524998,
		-6.98566E-05,
		0.000738144,
		0.000353813,
		-0.000924349,
		-0.000809193,
		0.001007795,
		0.00144887,
		-0.000886202,
		-0.002248049,
		0.000440598,
		0.00312829,
		0.000447273,
		-0.00394845,
		-0.001870632,
		0.004499197,
		0.003867388,
		-0.004512072,
		-0.006392241,
		0.003675938,
		0.009288311,
		-0.001663446,
		-0.012270451,
		-0.001842737,
		0.014911652,
		0.007131577,
		-0.016633987,
		-0.014478207,
		0.016674042,
		0.024231672,
		-0.013958216,
		-0.037100792,
		0.006659508,
		0.055086851,
		0.009580374,
		-0.085582495,
		-0.052207232,
		0.177955151,
		0.416601658,
	};

/* Trigger descriptor */
struct no_os_irq_ctrl_desc *trigger_irq_desc;

/* Red GPIO descriptor */
struct no_os_gpio_desc *gpio_red_led_desc;

/* Blue GPIO descriptor */
struct no_os_gpio_desc *gpio_blue_led_desc;

/* Reset GPIO descriptor */
struct no_os_gpio_desc *gpio_rst_desc;

/* Buffer enable GPIO descriptor */
struct no_os_gpio_desc *gpio_buf_en_desc;

/******************************************************************************/
/************************ Functions Prototypes ********************************/
/******************************************************************************/

/**
 * @brief	Initialize the trigger GPIO and associated IRQ event
 * @return	0 in case of success, negative error code otherwise
 */
int32_t gpio_trigger_init(void)
{
	int32_t ret;

	/* Initialize the IRQ controller */
	ret = no_os_irq_ctrl_init(&trigger_irq_desc, &trigger_gpio_irq_param);
	if (ret) {
		return ret;
	}

	ret = no_os_irq_register_callback(trigger_irq_desc,
					  0,
					  &ext_int_callback_desc);
	if (ret) {
		return ret;
	}

	ret = no_os_irq_trigger_level_set(trigger_irq_desc, 0, NO_OS_IRQ_EDGE_FALLING);
	if (ret) {
		return ret;
	}

	ret = no_os_irq_disable(trigger_irq_desc, 0);
	if (ret) {
		return ret;
	}

	return 0;
}

/**
 * @brief	Setup GPIOs for SDP-K1
 * @return	None
 */
int32_t sdpk1_gpio_setup(void)
{
	int32_t ret;

	ret = no_os_gpio_get(&gpio_red_led_desc, &gpio_red_led_param);
	if (ret) {
		return ret;
	}

	ret = no_os_gpio_direction_output(gpio_red_led_desc, NO_OS_GPIO_HIGH);
	if (ret) {
		return ret;
	}

	ret = no_os_gpio_get(&gpio_blue_led_desc, &gpio_blue_led_param);
	if (ret) {
		return ret;
	}

	ret = no_os_gpio_direction_output(gpio_blue_led_desc, NO_OS_GPIO_HIGH);
	if (ret) {
		return ret;
	}

	ret = no_os_gpio_get(&gpio_buf_en_desc, &gpio_buf_en_param);
	if (ret) {
		return ret;
	}

	ret = no_os_gpio_direction_output(gpio_buf_en_desc, NO_OS_GPIO_HIGH);
	if (ret) {
		return ret;
	}

	ret = no_os_gpio_get(&gpio_rst_desc, &gpio_rst_param);
	if (ret) {
		return ret;
	}

	ret = no_os_gpio_direction_output(gpio_rst_desc, NO_OS_GPIO_HIGH);
	if (ret) {
		return ret;
	}

	return 0;
}

/**
 * @brief	Reset ADC thru SDP-K1 GPIO
 * @return	None
 */
int32_t adc_hard_reset(void)
{
	int32_t ret;

	ret = no_os_gpio_set_value(gpio_rst_desc, NO_OS_GPIO_LOW);
	if (ret) {
		return ret;
	}

	no_os_mdelay(100); // Delay 100ms

	ret = no_os_gpio_set_value(gpio_rst_desc, NO_OS_GPIO_HIGH);
	if (ret) {
		return ret;
	}

	no_os_mdelay(100); // Delay 100ms

	return 0;
}



