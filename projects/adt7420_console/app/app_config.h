/***************************************************************************//*
 *   @file   app_config.h
 *   @brief  Application configuration file
******************************************************************************
 * Copyright (c) 2019, 2021-2022 Analog Devices, Inc.
 * All rights reserved.
 *
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 * By using this software you agree to the terms of the associated
 * Analog Devices Software License Agreement.
******************************************************************************/

#ifndef _APP_CONFIG_H_
#define _APP_CONFIG_H_

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include <stdint.h>
#include "PinNames.h"
#include "mbed_spi.h"
#include "mbed_i2c.h"

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

#ifndef ACTIVE_DEVICE
/** 
   #define your chosen device here from the 
   adt7420_type (adt7420.h) enum
*/
#define ACTIVE_DEVICE ID_ADT7320
#endif // !

/**
   ADT7420 is a 7-bit I2C address
*/
#define EXT_I2C_ADDRESS  0x49
#define INT_I2C_ADDRESS  0x48

/**
  Add a line-ending constant as different emulators 
  implement it in various ways - simple to change it here
*/
#define EOL "\r\n"

/**
* The ADI SDP_K1 can be used with either arduino headers
*  or the 120-pin SDP connector found on ADI evaluation
*  boards. The default is the Arduino connector
*  **/

/* NOTE: Uncomment the SDP_120 #define below to enable the SDP-120 connector */


// #define  SDP_120

#ifdef SDP_120
#define I2C_SCL     SDP_I2C_SCL
#define I2C_SDA     SDP_I2C_SDA

#define SPI_CSB		SDP_SPI_CS_A
#define SPI_HOST_SDI	SDP_SPI_MISO
#define SPI_HOST_SDO	SDP_SPI_MOSI
#define SPI_SCK		SDP_SPI_SCK

#define SPI_CSE 	SDP_SPI_CS_B	
#else
#define I2C_SCL     ARDUINO_UNO_D15
#define I2C_SDA     ARDUINO_UNO_D14

#define SPI_CSB		ARDUINO_UNO_D10
#define SPI_HOST_SDI	ARDUINO_UNO_D12
#define SPI_HOST_SDO	ARDUINO_UNO_D11
#define SPI_SCK		ARDUINO_UNO_D13

#define SPI_CSE 	ARDUINO_UNO_D9 
#endif

#define spi_platform_ops    mbed_spi_ops
#define i2c_platform_ops    mbed_i2c_ops

#endif //_APP_CONFIG_H_