/*************************************************************************//**
 *   @file   app_config.h
 *   @brief  Configuration file of nanodac firmware example program
******************************************************************************
* Copyright (c) 2020, 2022 Analog Devices, Inc.
*
* All rights reserved.
*
* This software is proprietary to Analog Devices, Inc. and its licensors.
* By using this software you agree to the terms of the associated
* Analog Devices Software License Agreement.
*****************************************************************************/

#ifndef _APP_CONFIG_H_
#define _APP_CONFIG_H_

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include <stdint.h>
#include <PinNames.h>

/******************************************************************************/
/********************** Macros and Constants Definitions **********************/
/******************************************************************************/

/**
  The ADI SDP_K1 can be used with both arduino headers
  or the 120-pin SDP connector found on ADI evaluation
  boards. The default is the SDP connector.
  Uncomment the ARDUINO #define below to enable the ARDUINO connector
*/

//#define  ARDUINO

// **** Note for User: ACTIVE_DEVICE selection ****
// Define the device type here from the list of below device type defines
// e.g. #define DEV_AD5677R -> This will make AD5677R as an ACTIVE_DEVICE.
// The ACTIVE_DEVICE is default set to AD5686, if device type is not defined.

//#define DEV_AD5677R

#if defined(DEV_AD5671R)
#define ACTIVE_DEVICE		ID_AD5671R
#define ACTIVE_DEVICE_NAME	"AD5671R"
#elif defined(DEV_AD5672R)
#define ACTIVE_DEVICE		ID_AD5672R
#define ACTIVE_DEVICE_NAME	"AD5672R"
#elif defined(DEV_AD5673R)
#define ACTIVE_DEVICE		ID_AD5673R
#define ACTIVE_DEVICE_NAME	"AD5673R"
#elif defined(DEV_AD5674)
#define ACTIVE_DEVICE		ID_AD5674
#define ACTIVE_DEVICE_NAME	"AD5674"
#elif defined(DEV_AD5674R)
#define ACTIVE_DEVICE		ID_AD5674R
#define ACTIVE_DEVICE_NAME	"AD5674R"
#elif defined(DEV_AD5675R)
#define ACTIVE_DEVICE		ID_AD5675R
#define ACTIVE_DEVICE_NAME	"AD5675R"
#elif defined(DEV_AD5676)
#define ACTIVE_DEVICE		ID_AD5676
#define ACTIVE_DEVICE_NAME	"AD5676"
#elif defined(DEV_AD5676R)
#define ACTIVE_DEVICE		ID_AD5676R
#define ACTIVE_DEVICE_NAME	"AD5676R"
#elif defined(DEV_AD5677R)
#define ACTIVE_DEVICE		ID_AD5677R
#define ACTIVE_DEVICE_NAME	"AD5677R"
#elif defined(DEV_AD5679)
#define ACTIVE_DEVICE		ID_AD5679
#define ACTIVE_DEVICE_NAME	"AD5679"
#elif defined(DEV_AD5679R)
#define ACTIVE_DEVICE		ID_AD5679R
#define ACTIVE_DEVICE_NAME	"AD5679R"
#elif defined(DEV_AD5686)
#define ACTIVE_DEVICE		ID_AD5686
#define ACTIVE_DEVICE_NAME	"AD5686"
#elif defined(DEV_AD5684R)
#define ACTIVE_DEVICE		ID_AD5684R
#define ACTIVE_DEVICE_NAME	"AD5684R"
#elif defined(DEV_AD5685R)
#define ACTIVE_DEVICE		ID_AD5685R
#define ACTIVE_DEVICE_NAME	"AD5685R"
#elif defined(DEV_AD5686R)
#define ACTIVE_DEVICE		ID_AD5686R
#define ACTIVE_DEVICE_NAME	"AD5686R"
#elif defined(DEV_AD5687)
#define ACTIVE_DEVICE		ID_AD5687
#define ACTIVE_DEVICE_NAME	"AD5687"
#elif defined(DEV_AD5687R)
#define ACTIVE_DEVICE		ID_AD5687R
#define ACTIVE_DEVICE_NAME	"AD5687R"
#elif defined(DEV_AD5689)
#define ACTIVE_DEVICE		ID_AD5689
#define ACTIVE_DEVICE_NAME	"AD5689"
#elif defined(DEV_AD5689R)
#define ACTIVE_DEVICE		ID_AD5689R
#define ACTIVE_DEVICE_NAME	"AD5689R"
#elif defined(DEV_AD5697R)
#define ACTIVE_DEVICE		ID_AD5697R
#define ACTIVE_DEVICE_NAME	"AD5697R"
#elif defined(DEV_AD5694)
#define ACTIVE_DEVICE		ID_AD5694
#define ACTIVE_DEVICE_NAME	"AD5694"
#elif defined(DEV_AD5694R)
#define ACTIVE_DEVICE		ID_AD5694R
#define ACTIVE_DEVICE_NAME	"AD5694R"
#elif defined(DEV_AD5695R)
#define ACTIVE_DEVICE		ID_AD5695R
#define ACTIVE_DEVICE_NAME	"AD5695R"
#elif defined(DEV_AD5696)
#define ACTIVE_DEVICE		ID_AD5696
#define ACTIVE_DEVICE_NAME	"AD5696"
#elif defined(DEV_AD5696R)
#define ACTIVE_DEVICE		ID_AD5696R
#define ACTIVE_DEVICE_NAME	"AD5696R"
#elif defined(DEV_AD5681R)
#define ACTIVE_DEVICE		ID_AD5681R
#define ACTIVE_DEVICE_NAME	"AD5681R"
#elif defined(DEV_AD5682R)
#define ACTIVE_DEVICE		ID_AD5682R
#define ACTIVE_DEVICE_NAME	"AD5682R"
#elif defined(DEV_AD5683R)
#define ACTIVE_DEVICE		ID_AD5683R
#define ACTIVE_DEVICE_NAME	"AD5683R"
#elif defined(DEV_AD5683)
#define ACTIVE_DEVICE		ID_AD5683
#define ACTIVE_DEVICE_NAME	"AD5683"
#elif defined(DEV_AD5691R)
#define ACTIVE_DEVICE		ID_AD5691R
#define ACTIVE_DEVICE_NAME	"AD5691R"
#elif defined(DEV_AD5692R)
#define ACTIVE_DEVICE		ID_AD5692R
#define ACTIVE_DEVICE_NAME	"AD5692R"
#elif defined(DEV_AD5693R)
#define ACTIVE_DEVICE		ID_AD5693R
#define ACTIVE_DEVICE_NAME	"AD5693R"
#elif defined(DEV_AD5693)
#define ACTIVE_DEVICE		ID_AD5693
#define ACTIVE_DEVICE_NAME	"AD5693"
#else
#warning No/Unsupported ADxxxxy symbol defined. AD5686R defined
#define DEV_AD5686R
#define ACTIVE_DEVICE		ID_AD5686R
#define ACTIVE_DEVICE_NAME	"AD5686R"
#endif


// Pin mapping of nanoDAC+ with SDP-120 way or Arduino connectors
#ifdef ARDUINO
#define I2C_SCL		ARDUINO_UNO_D15
#define I2C_SDA		ARDUINO_UNO_D14

#define SPI_CSB			ARDUINO_UNO_D10
#define SPI_HOST_SDO	ARDUINO_UNO_D11
#define SPI_HOST_SDI	ARDUINO_UNO_D12
#define SPI_SCK			ARDUINO_UNO_D13

#define GAIN_PIN	ARDUINO_UNO_D8
#define RESET_PIN	ARDUINO_UNO_D9
#define LDAC_PIN	ARDUINO_UNO_D7
#define ADDR0_PIN	ARDUINO_UNO_D6
#else
#define I2C_SCL		SDP_I2C_SCL		// PH_7
#define I2C_SDA		SDP_I2C_SDA		// PC_9

#define SPI_CSB		    SDP_SPI_CS_A	// PB_9
#define SPI_HOST_SDO	SDP_SPI_MOSI	// PF_9
#define SPI_HOST_SDI	SDP_SPI_MISO	// PF_8
#define SPI_SCK		    SDP_SPI_SCK		// PH_6
#endif

// Define the other GPIO mapping based on the compatible EVAL board
// *Note: The 7-bit I2C slave address mentioned below is the default address for the
//        device, set by combination of slave address bits (7:3) from the device
//        datasheet and default logic level of A1 and A0 pins (bits 2:1) on the
//        respective device EVAL board. For more information, refer the device
//        datasheet and EVAL board manual.

#if defined(DEV_AD5686R) || defined(DEV_AD5686) || \
    defined(DEV_AD5684R) || defined(DEV_AD5684) || \
    defined(DEV_AD5685R)
// These devices support EVAL-AD5686RSDZ board
#if !defined ARDUINO
#define GAIN_PIN	SDP_GPIO_0
#define RESET_PIN	SDP_GPIO_2
#define LDAC_PIN	SDP_GPIO_3
#endif
#elif defined(DEV_AD5696R) || defined(DEV_AD5696) || \
      defined(DEV_AD5694R) || defined(DEV_AD5694) || \
      defined(DEV_AD5695R) || defined(DEV_AD5697R)
// These devices support EVAL-AD5696RSDZ board
#if !defined ARDUINO
#define GAIN_PIN	SDP_GPIO_0
#define RESET_PIN	SDP_GPIO_2
#define LDAC_PIN	SDP_GPIO_3
#endif
#define I2C_SLAVE_ADDRESS	0x18
#elif defined(DEV_AD5683) || defined(DEV_AD5683R) || defined(DEV_AD5682R) || \
      defined(DEV_AD5681R)
// These devices uses EVAL-AD5683R board
#if !defined ARDUINO
#define GAIN_PIN	SDP_GPIO_2
#define RESET_PIN	SDP_GPIO_1
#define LDAC_PIN	SDP_GPIO_0
#endif
#elif defined(DEV_AD5693) || defined(DEV_AD5693R) || defined(DEV_AD5692R) || \
      defined(DEV_AD5691R)
// These devices uses EVAL-AD5693R board
#if !defined ARDUINO
#define GAIN_PIN	SDP_GPIO_2
#define RESET_PIN	SDP_GPIO_1
#define LDAC_PIN	SDP_GPIO_0
#endif
#define I2C_SLAVE_ADDRESS	0x98
#elif defined (DEV_AD5674R) || defined (DEV_AD5674) || \
      defined (DEV_AD5679R) || defined (DEV_AD5679) || \
      defined (DEV_AD5677R) || defined (DEV_AD5673R)
// These devices uses EVAL-AD5679RSDZ/EVAL-AD567xRSDZ board
#if !defined ARDUINO
#define GAIN_PIN	SDP_GPIO_0
#define RESET_PIN	SDP_GPIO_2
#define LDAC_PIN	SDP_GPIO_1
#endif
#define I2C_SLAVE_ADDRESS	0x1E
#elif defined (DEV_AD5676R) || defined (DEV_AD5676) || \
      defined (DEV_AD5672R)
// These devices uses EVAL-AD5676RSDZ board
#if !defined ARDUINO
#define GAIN_PIN	SDP_GPIO_2
#define RESET_PIN	SDP_GPIO_1
#define LDAC_PIN	SDP_GPIO_0
#endif
#elif defined (DEV_AD5671R) || defined (DEV_AD5675R)
// These devices uses EVAL-AD5675RSDZ board
#if !defined ARDUINO
#define GAIN_PIN	SDP_GPIO_2
#define RESET_PIN	SDP_GPIO_1
#define LDAC_PIN	SDP_GPIO_0
#endif
#define I2C_SLAVE_ADDRESS	0x18
#else
#warning No/Unsupported EVAL board found. Using EVAL-AD5686R as default.
#if !defined ARDUINO
#define GAIN_PIN	SDP_GPIO_0
#define RESET_PIN	SDP_GPIO_2
#define LDAC_PIN	SDP_GPIO_3
#endif
#endif


// Common pin mappings
#define LED_GREEN	LED3	// PK_5

#endif //_APP_CONFIG_H_
