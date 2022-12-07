/*************************************************************************//**
 *   @file   app_config.h
 *   @brief  Configuration file for AD717x/AD411x firmware example
******************************************************************************
* Copyright (c) 2020,2022 Analog Devices, Inc.
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

// **** Note for User: ACTIVE_DEVICE selection ****//
/* Define the device type here from the list of below device type defines
 * (one at a time. Defining more than one device can result into compile error).
 * e.g. #define DEV_AD4111 -> This will make AD4111 as an ACTIVE_DEVICE.
 * The ACTIVE_DEVICE is default set to AD4111, if device type is not defined.
 * */
//#define	DEV_AD4111

/* NOTE: Only EVAL-AD4114SDZ, EVAL-AD4115SDZ and EVAL-AD4116ASDZ support Arduino and SDP_120
 * interface. The other EVAL Boards (EVAL-AD4111SDZ, EVAL-AD4112SDZ,
 * EVAL-AD7172-4SDZ, EVAL-AD7172-2SDZ , EVAL-AD7173-8SDZ, EVAL-AD7175-2SDZ,
 * EVAL-AD7175-8SDZ, EVAL-AD7176-2SDZ, EVAL-AD7177-2SDZ) support only the
 * SDP-120 interface.
 */

 /* NOTE: Uncomment the SDP_120 #define below to enable the SDP-120 connector */

// #define  SDP_120

#ifdef SDP_120
/* SPI Pins on SDP-K1 SDP-120 Interface */
#define I2C_SCL		SDP_I2C_SCL		// PH_7
#define I2C_SDA		SDP_I2C_SDA		// PC_9

#define SPI_CSB		SDP_SPI_CS_A	// PB_9
#define SPI_HOST_SDI	SDP_SPI_MISO	// PF_8
#define SPI_HOST_SDO	SDP_SPI_MOSI	// PF_9
#define SPI_SCK		SDP_SPI_SCK		// PH_6
#else
/* SPI Pins on SDP-K1-Arduino Interface */
#define SPI_CSB		ARDUINO_UNO_D10		// SPI_CS
#define SPI_HOST_SDO	ARDUINO_UNO_D11		// SPI_MOSI
#define SPI_HOST_SDI	ARDUINO_UNO_D12		// SPI_MISO
#define SPI_SCK		ARDUINO_UNO_D13		// SPI_SCK
#define I2C_SCL		ARDUINO_UNO_D15
#define I2C_SDA		ARDUINO_UNO_D14
#endif

// Common pin mappings
#define LED_GREEN	LED3


#if defined(DEV_AD4111)
#define ACTIVE_DEVICE_NAME	"AD4111"
#elif defined(DEV_AD4112)
#define ACTIVE_DEVICE_NAME	"AD4112"
#elif defined(DEV_AD4114)
#define ACTIVE_DEVICE_NAME	"AD4114"
#elif defined(DEV_AD4115)
#define ACTIVE_DEVICE_NAME	"AD4115"
#elif defined(DEV_AD7172_2)
#define AD7172_2_INIT
#define ACTIVE_DEVICE_NAME	"AD7172-2"
#elif defined(DEV_AD7172_4)
#define AD7172_4_INIT
#define ACTIVE_DEVICE_NAME	"AD7172-4"
#elif defined(DEV_AD7173_8)
#define AD7173_8_INIT
#define ACTIVE_DEVICE_NAME	"AD7173-8"
#elif defined(DEV_AD7175_2)
#define AD7175_2_INIT
#define ACTIVE_DEVICE_NAME	"AD7175-2"
#elif defined(DEV_AD7175_8)
#define AD7175_8_INIT
#define ACTIVE_DEVICE_NAME	"AD7175-8"
#elif defined(DEV_AD7176_2)
#define AD7176_2_INIT
#define ACTIVE_DEVICE_NAME	"AD7176-2"
#elif defined(DEV_AD7177_2)
#define AD7177_2_INIT
#define ACTIVE_DEVICE_NAME	"AD7177-2"
#else
#warning No/Unsupported ADxxxxy symbol defined. AD4111 defined
#define DEV_AD4111
#define ACTIVE_DEVICE_NAME	"AD4111"
#endif

#endif //_APP_CONFIG_H_
