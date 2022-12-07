/*************************************************************************//**
 *   @file   app_config.h
 *   @brief  Configuration file of AD5933 firmware example program
******************************************************************************
* Copyright (c) 2019, 2022 Analog Devices, Inc.  
* 
* All rights reserved.
*
* This software is proprietary to Analog Devices, Inc. and its licensors.
* By using this software you agree to the terms of the associated
* Analog Devices Software License Agreement.
*****************************************************************************/

#ifndef _APP_CONFIG_H_
#define _APP_CONFIG_H_

#include <stdint.h>
#include <PinNames.h>
#include "ad5933.h"

/**
  The ADI SDP_K1 can be used with both arduino headers
  or the 120-pin SDP connector found on ADI evaluation
  boards. The default is the Arduino connector

  Uncomment the ARDUINO #define below to enable the ARDUINO connector

*/
//#define  ARDUINO

//#warning  check this
#ifdef ARDUINO
/* Arduino interface pins*/
#define I2C_SCL     ARDUINO_UNO_D15
#define I2C_SDA     ARDUINO_UNO_D14

#define SPI_CS		ARDUINO_UNO_D10
#define SPI_MISO	ARDUINO_UNO_D12
#define SPI_MOSI	ARDUINO_UNO_D11
#define SPI_SCK		ARDUINO_UNO_D13

#define GAIN_PIN	ARDUINO_UNO_D8 
#define RESET_PIN	ARDUINO_UNO_D9 
#define LDAC_PIN	ARDUINO_UNO_D7 
#define ADDR0_PIN	ARDUINO_UNO_D6 
#else
/* SDP-120 interface pins*/
#define I2C_SCL     SDP_I2C_SCL
#define I2C_SDA     SDP_I2C_SDA

#define SPI_CS		SDP_SPI_CS_A
#define SPI_MISO	SDP_SPI_MISO
#define SPI_MOSI	SDP_SPI_MOSI
#define SPI_SCK		SDP_SPI_SCK

#define GAIN_PIN	SDP_GPIO_0
#define RESET_PIN	SDP_GPIO_2
#define LDAC_PIN	SDP_GPIO_3
#define ADDR0_PIN	SDP_GPIO_4
#endif

#endif //_APP_CONFIG_H_