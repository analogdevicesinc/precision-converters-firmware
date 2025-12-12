/*************************************************************************//**
 * @file   app_config.h
 * @brief  Application configurations module headers for digipots IIO FW
******************************************************************************
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
*****************************************************************************/

#ifndef APP_CONFIG_H
#define APP_CONFIG_H

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/
#include <stdint.h>
#include "common_macros.h"
#include "common.h"
#include "no_os_spi.h"
#include "no_os_i2c.h"
#include "no_os_gpio.h"
#include "no_os_uart.h"
#include "dpot.h"

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/
/* Macros for stringification */
#define XSTR(s)		#s
#define STR(s)		XSTR(s)

/******************************************************************************/

// **** Note for User: Board Device selection ****//
/* Comment out the define you don't want a separate Board device in the IIO context.
 * */
#define DPOT_ADD_BOARD_DEVICE

/* Select the active platform (default is STM32) */
#if !defined(ACTIVE_PLATFORM)
#define ACTIVE_PLATFORM      STM32_PLATFORM
#endif

#if !defined(USE_PHY_COM_PORT)
#define USE_VIRTUAL_COM_PORT
#endif

#if (ACTIVE_PLATFORM == STM32_PLATFORM)
#include "app_config_stm32.h"
#else
#error "No/Invalid active platform selected"
#endif

/* HW ID of the digipots motherboard */
#define HW_MEZZANINE_NAME	"EVAL-MB-LV-ARDZ"

/****** Macros used to form a VCOM serial number ******/
#define	FIRMWARE_NAME	"digipots_iio"
#if !defined(PLATFORM_NAME)
#define PLATFORM_NAME	HW_CARRIER_NAME
#endif

#define MAX_CHNS_LINGAIN   				8
#define MAX_CHNS_POTENTIOMETER		    4
#define MAX_CHNS		  		       12

/* Baud rate for IIO application UART interface */
#define IIO_UART_BAUD_RATE	(230400)

/* Check if any serial port available for use as console stdio port */
#if defined(USE_VIRTUAL_COM_PORT)
/* If PHY com is selected, VCOM or alternate PHY com port can act as a console stdio port */
/* If VCOM is selected, PHY com port will/should act as a console stdio port */
#define CONSOLE_STDIO_PORT_AVAILABLE
#endif

/* Enable/Disable the use of SDRAM for DAC data streaming buffer */
//#define USE_SDRAM		// Uncomment to use SDRAM for data buffer

/******************************************************************************/
/********************** Board Defaults ****************************************/
/******************************************************************************/

#define DEFAULT_ACTIVE_DEVICE	    0XFF
#define DEFAULT_DEVICE_NAME	        "ad5244"
#define DEFAULT_DEVICE_I2C_ADDR	    0x2C
#define DEFAULT_NUM_CHNS_POT	    4
#define DEFAULT_NUM_CHNS_LINGAIN    8
#define DEFAULT_OPERATING_MODE     	DPOT_POTENTIOMETER_MODE
#define DEFAULT_INTERFACE_TYPE      AD_SPI_INTERFACE

/******************************************************************************/
/********************** Variables and User Defined Data Types *****************/
/******************************************************************************/
/******/

#define MAX_DEVICE_NAME_LEN 10

/**
 * @struct dpot_dev_info.
 * @brief Digital potentiometer common parameters.
 */
typedef struct dpot_device_info {
	char device_name[8];
	uint8_t max_position;
	uint8_t device_i2c_addr;
	/* Number of input channels */
	uint8_t num_of_channels;
	/* Resolution (number of wiper positions = 2^res) */
	uint8_t nSupportedInterface;
	struct dpot_init_param dpot_init_params;
} dpot_device_info;

/**
 * @struct active_dpot_device_info.
 * @brief Active device information for the initial configuration.
 */
typedef struct active_dpot_device_info {
	/* Interface to be used by the device */
	enum dpot_intf_type intf_type;
	/* Device Name  */
	char active_device_name[MAX_DEVICE_NAME_LEN];
	/* Active device ID */
	enum dpot_dev_id active_device;
	/* I2C slave address. Used only when Interface type is I2C */
	uint8_t device_i2c_addr;
	/* Number of channels in Pot mode  */
	uint8_t max_chns_pot;
	/* Number of channels in linear gain mode  */
	uint8_t max_chns_linGain;
	/* To select the Operating mode.*/
	enum  dpot_operating_mode mode;
} active_dpot_device;

/******************************************************************************/
/************************ Public Declarations *********************************/
/******************************************************************************/
extern struct no_os_uart_desc *uart_iio_com_desc;
extern struct no_os_spi_init_param spi_mode2_init_params;
extern struct no_os_spi_init_param spi_mode0_init_params;
extern struct no_os_i2c_init_param i2c_init_params;
extern struct no_os_eeprom_desc *eeprom_desc;

extern char *active_virtual_com_serial_num;
extern active_dpot_device oactive_dev;
extern const struct dpot_ops ad5144_dpot_ops;
extern const struct dpot_ops ad5141_dpot_ops;
extern const struct dpot_ops ad5142_dpot_ops;
extern const struct dpot_ops ad5143_dpot_ops;
extern const struct dpot_ops ad5259_dpot_ops;
extern const struct dpot_ops ad5161_dpot_ops;
extern const struct dpot_ops ad5246_dpot_ops;
extern const struct dpot_ops ad5242_dpot_ops;
extern const struct dpot_ops ad5171_dpot_ops;
extern const struct dpot_ops ad5165_dpot_ops;
extern const struct dpot_ops ad5228_dpot_ops;

extern struct ad5144_dpot_init_param ad5144_init_params;
extern struct ad5141_dpot_init_param ad5141_init_params;
extern struct ad5142_dpot_init_param ad5142_init_params;
extern struct ad5143_dpot_init_param ad5143_init_params;
extern struct ad5259_dpot_init_param ad5259_init_params;
extern struct ad516x_dpot_init_param ad5161_init_params;
extern struct ad5246_dpot_init_param ad5246_init_params;
extern struct ad5242_dpot_init_param ad5242_init_params;
extern struct ad5171_dpot_init_param ad5171_init_params;
extern struct ad516x_dpot_init_param ad5165_init_params;
extern struct ad5228_dpot_init_param ad5228_init_params;
int32_t init_system(void);
extern dpot_device_info dpot_info[];
#endif // APP_CONFIG_H
