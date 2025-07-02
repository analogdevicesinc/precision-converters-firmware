/***************************************************************************//**
 * @file    stm32_usb_uart.c
 * @brief   VCOM driver for stm32 as a no_os_uart implementation.
********************************************************************************
* Copyright (c) 2025 Analog Devices, Inc.
* All rights reserved.
*
* This software is proprietary to Analog Devices, Inc. and its licensors.
* By using this software you agree to the terms of the associated
* Analog Devices Software License Agreement.
*******************************************************************************/

#include <errno.h>
#include <stdlib.h>
#include "no_os_util.h"
#include "no_os_lf256fifo.h"
#include "no_os_alloc.h"
#include "no_os_gpio.h"

#include "stm32_usb_uart.h"
#include "ux_device_descriptors.h"
#include "ux_dcd_stm32.h"

#define USBD_DEFAULT_TIMEOUT				   0xFFFFFF
#define USBD_EPINCMD_ADDR                      0x82U
#define USBD_EPINCMD_FS_MPS                    8U
#define USBD_EPINCMD_HS_MPS                    8U
#define USBD_EPIN_ADDR                         0x81U
#define USBD_EPOUT_ADDR                        0x01U

#ifndef STM32_USB_CDC_ACM_RXBUF_LEN
#define STM32_USB_CDC_ACM_RXBUF_LEN UX_SLAVE_REQUEST_DATA_MAX_LENGTH
#endif

#ifndef STM32_USB_CDC_ACM_TXBUF_LEN
#define STM32_USB_CDC_ACM_TXBUF_LEN UX_SLAVE_REQUEST_DATA_MAX_LENGTH
#endif

/* USB slave device descriptor */
UX_SLAVE_CLASS_CDC_ACM *gusbdevice;

/* Store transaction status. */
static volatile bool tx_pending;

/* Default line coding for CDC ACM USB class */
UX_SLAVE_CLASS_CDC_ACM_LINE_CODING_PARAMETER cdc_line_coding = {
	230400, /* baud rate */
	0x00,   /* stop bits-1 */
	0x00,   /* parity - none */
	0x08    /* nb. of bits 8 */
};

/* This global FIFO is needed because of how the STM32 CDC API is implemented.
 * The function ops do not allow passing context parameters and so it is
 * impossible to pass this fifo to CDC receive API in any other way.
 * The implication is that only 1 USB CDC ACM interface may be used at once
 * but this is an acceptable compromise, since having more than one doesn't
 * make much sense anyway. */
static struct lf256fifo *gfifo;

/*
 * @brief Callback called when the CDC ACM class is activated. Store
 * 		  the CDC ACM descriptor for future use.
 * @param cdc_acm_instance[in] - The CDC ACM descriptor.
 * @return None.
 */
static void CDC_Activate(VOID *cdc_acm_instance)
{
	gusbdevice = (UX_SLAVE_CLASS_CDC_ACM *) cdc_acm_instance;
}

/*
 * @brief Callback called when the CDC ACM class is deactivated.Clear
 * 		  the stored CDC ACM descriptor.
 * @param cdc_acm_instance[in] - The CDC ACM descriptor(Unused).
 * @return None.
 */
static void CDC_Deactivate(VOID *cdc_acm_instance)
{
	UX_PARAMETER_NOT_USED(cdc_acm_instance);
	/* Reset the cdc acm instance */
	gusbdevice = NULL;

	return;
}

/*
 * @brief Callback called when the CDC ACM class is modified.
 * @param cdc_acm_instance[in] - The CDC ACM descriptor(Unused).
 * @return None.
 */
static void CDC_ParameterChange(VOID *cdc_acm_instance)
{
	UX_PARAMETER_NOT_USED(cdc_acm_instance);
}

/*
 * @brief Callback called when the CDC ACM class is Tx is completed.
 * @param cdc_acm_instance[in] - The CDC ACM descriptor(Unused).
 * @param status[in] - Status of transfer(Unused).
 * @param length[in] - The byte count of the transfer.
 * @return None.
 */
static UINT CDC_TxCplt(struct UX_SLAVE_CLASS_CDC_ACM_STRUCT *cdc_acm_instance,
		       UINT status,
		       ULONG length)
{
	UX_PARAMETER_NOT_USED(cdc_acm_instance);
	UX_PARAMETER_NOT_USED(status);
	tx_pending = 0;
	return 0;
}

/*
 * @brief Callback called when the CDC ACM class is activated.Clear
 * 		  the stored CDC ACM descriptor.
 * @param cdc_acm_instance[in] - The CDC ACM descriptor(Unused).
 * @param status[in] - Status of transfer(Unused).
 * @param data[in] - Pointer to the received data.
 * @param length[in] - The byte count of the data received.
 * @return None.
 */
static UINT CDC_RxCplt(struct UX_SLAVE_CLASS_CDC_ACM_STRUCT *cdc_acm_instance,
		       UINT status,
		       UCHAR *data,
		       ULONG length)
{
	unsigned int i = 0;
	int ret;

	UX_PARAMETER_NOT_USED(cdc_acm_instance);
	UX_PARAMETER_NOT_USED(status);
	while (i < length) {
		ret = lf256fifo_write(gfifo, data[i]);
		if (ret) {
			break;
		}
		i++;
	}

	return 0;
}

/**
 * @brief Initialize the UART communication peripheral.
 * @param desc[in, out] - The UART descriptor.
 * @param param[in] - The structure that contains the UART parameters.
 * @return 0 in case of success, error code otherwise.
 */
static int32_t stm32_usb_uart_init(struct no_os_uart_desc **desc,
				   struct no_os_uart_init_param *param)
{
	int ret;
	struct stm32_usb_uart_init_param *suip;
	struct no_os_uart_desc *descriptor;
	struct stm32_usb_uart_desc *sdesc;
	uint64_t cdc_acm_interface_number;
	uint64_t cdc_acm_config_number;
	uint32_t timeout = USBD_DEFAULT_TIMEOUT;

	UX_SLAVE_CLASS_CDC_ACM_PARAMETER cdc_acm_parameter = {
		CDC_Activate,
		CDC_Deactivate,
		CDC_ParameterChange
	};

	UX_SLAVE_CLASS_CDC_ACM_CALLBACK_PARAMETER ux_callback = {
		.ux_device_class_cdc_acm_parameter_write_callback = CDC_TxCplt,
		.ux_device_class_cdc_acm_parameter_read_callback = CDC_RxCplt
	};

	if (!desc || !param) {
		return -EINVAL;
	}

	if (!param->extra) {
		return -EINVAL;
	}

	descriptor = no_os_calloc(1, sizeof(*descriptor));
	if (!descriptor) {
		return -ENOMEM;
	}

	sdesc = no_os_calloc(1, sizeof(*sdesc));
	if (!sdesc) {
		ret = -ENOMEM;
		goto err_sdesc;
	}

	descriptor->extra = sdesc;
	suip = param->extra;

	ret = lf256fifo_init(&sdesc->fifo);
	if (ret) {
		goto err_fifo;
	}

	gfifo = sdesc->fifo;

	/* Unregister the class */
	ret = (int32_t) ux_device_stack_class_unregister(
		      _ux_system_slave_class_cdc_acm_name,
		      ux_device_class_cdc_acm_entry);
	if (ret) {
		goto err_fifo;
	}

	/* Get cdc acm configuration number */
	cdc_acm_config_number = USBD_Get_Configuration_Number(CLASS_TYPE_CDC_ACM, 0);

	/* Find cdc acm interface number */
	cdc_acm_interface_number = USBD_Get_Interface_Number(CLASS_TYPE_CDC_ACM, 0);

	ret = (int32_t) ux_device_stack_class_register(
		      _ux_system_slave_class_cdc_acm_name,
		      ux_device_class_cdc_acm_entry,
		      cdc_acm_config_number,
		      cdc_acm_interface_number,
		      &cdc_acm_parameter);
	if (ret) {
		goto err_fifo;
	}

	/* TODO: Figure how the address (last arg) is calculated and use a macro */
	HAL_PCDEx_PMAConfig(suip->hpcd, 0x00, PCD_SNG_BUF, 0x20);
	HAL_PCDEx_PMAConfig(suip->hpcd, 0x80, PCD_SNG_BUF, 0x60);
	HAL_PCDEx_PMAConfig(suip->hpcd, USBD_EPIN_ADDR, PCD_SNG_BUF, 0xA0);
	HAL_PCDEx_PMAConfig(suip->hpcd, USBD_EPOUT_ADDR, PCD_SNG_BUF, 0xE0);
	HAL_PCDEx_PMAConfig(suip->hpcd, USBD_EPINCMD_ADDR, PCD_SNG_BUF, 0x120);

	/* Initialize the device controller driver */
	ux_dcd_stm32_initialize((ULONG)USB_DRD_FS, (ULONG)suip->hpcd);

	/* Start the USB device */
	ret = HAL_PCD_Start(suip->hpcd);
	if (ret) {
		goto err_ux;
	}

	while (!gusbdevice && timeout--) {
		ux_device_stack_tasks_run();
	}

	if (!gusbdevice && !timeout) {
		goto err_ux;
	}

	ret = (int32_t) ux_device_class_cdc_acm_ioctl(gusbdevice,
			UX_SLAVE_CLASS_CDC_ACM_IOCTL_TRANSMISSION_START,
			(void *) &ux_callback);
	if (ret) {
		goto err_ux;
	}

	cdc_line_coding.ux_slave_class_cdc_acm_parameter_baudrate = param->baud_rate;
	/*
	 * param->size is an enum starting from 0 and ux_slave_class_cdc_acm_parameter_data_bit
	 * takes number of data bits. +5 since NO_OS_UART_CS_5=0.
	 */
	cdc_line_coding.ux_slave_class_cdc_acm_parameter_data_bit = (param->size + 5);
	cdc_line_coding.ux_slave_class_cdc_acm_parameter_parity = param->parity;
	cdc_line_coding.ux_slave_class_cdc_acm_parameter_stop_bit = param->stop;

	ret = (int32_t) ux_device_class_cdc_acm_ioctl(gusbdevice,
			UX_SLAVE_CLASS_CDC_ACM_IOCTL_SET_LINE_CODING,
			(void *) &cdc_line_coding);
	if (ret) {
		goto err_ux;
	}

	sdesc->husbdevice = gusbdevice;
	*desc = descriptor;

	return 0;
err_ux:
	HAL_PCD_Stop(suip->hpcd);
	ux_device_stack_class_unregister(_ux_system_slave_class_cdc_acm_name,
					 ux_device_class_cdc_acm_entry);
err_fifo:
	lf256fifo_remove(sdesc->fifo);
	no_os_free(sdesc);
err_sdesc:
	no_os_free(descriptor);

	return ret;
}

/**
 * @brief Free the resources allocated by stm32_uart_init().
 * @param desc[in] - The UART descriptor.
 * @return 0 in case of success, -1 otherwise.
 */
static int32_t stm32_usb_uart_remove(struct no_os_uart_desc *desc)
{
	struct stm32_usb_uart_desc *sdesc = desc->extra;

	HAL_PCD_Stop(sdesc->hpcd);
	/* Unregister the class */
	ux_device_stack_class_unregister(_ux_system_slave_class_cdc_acm_name,
					 ux_device_class_cdc_acm_entry);
	lf256fifo_remove(sdesc->fifo);
	no_os_free(desc->extra);
	no_os_free(desc);

	return 0;
};

/**
 * @brief Write data to UART device.
 * @param desc[in] - Instance of UART.
 * @param data[in] - Pointer to buffer containing data.
 * @param bytes_number[in] - Number of bytes to write.
 * @return Positive number of transmitted bytes in case of success, negative error code otherwise.
 */
static int32_t stm32_usb_uart_write(struct no_os_uart_desc *desc,
				    const uint8_t *data,
				    uint32_t bytes_number)
{
	int ret;
	unsigned int len = no_os_min(bytes_number, STM32_USB_CDC_ACM_TXBUF_LEN);
	uint32_t timeout = USBD_DEFAULT_TIMEOUT;

	tx_pending = 1;
	ret = ux_device_class_cdc_acm_write_with_callback(gusbdevice, (uint8_t *)data,
			len);
	if (ret) {
		tx_pending = 0;
		return -EFAULT;
	}

	while (tx_pending && timeout--) {
		ux_device_stack_tasks_run();
	}

	if (tx_pending && !timeout) {
		return -ETIMEDOUT;
	}

	return len;
}

/**
 * @brief Read data from UART device.
 * @param desc[in] - Instance of UART.
 * @param data[in, out] - Pointer to buffer containing data.
 * @param bytes_number[in] - Number of bytes to read.
 * @return positive number of received bytes in case of success, negative error code otherwise.
 */
static int32_t stm32_usb_uart_read(struct no_os_uart_desc *desc, uint8_t *data,
				   uint32_t bytes_number)
{
	int ret;
	unsigned int i = 0;
	struct stm32_usb_uart_desc *sdesc = desc->extra;

	while (i < bytes_number) {
		ret = lf256fifo_read(sdesc->fifo, &data[i]);
		if (ret) {
			break;
		}
		i++;
	}

	return i;
}

/**
 * @brief STM32 platform specific UART platform ops structure
 */
const struct no_os_uart_platform_ops stm32_usb_uart_ops = {
	.init = &stm32_usb_uart_init,
	.read = &stm32_usb_uart_read,
	.write = &stm32_usb_uart_write,
	.remove = &stm32_usb_uart_remove
};
