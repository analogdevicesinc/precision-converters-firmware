/*************************************************************************//**
 *   @file   ltc2488_user_config.h
 *   @brief  Header for ltc2488 user configuration file
******************************************************************************
* Copyright (c) 2021-22 Analog Devices, Inc.
*
* All rights reserved.
*
* This software is proprietary to Analog Devices, Inc. and its licensors.
* By using this software you agree to the terms of the associated
* Analog Devices Software License Agreement.
*****************************************************************************/

#ifndef LTC2488_USER_CONFIG_H_
#define LTC2488_USER_CONFIG_H_

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/
#include <PinNames.h>

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

/**
  The ADI SDP_K1 can be used with both arduino headers
  or the 120-pin SDP connector found on ADI evaluation
  boards. The default is the Arduino connector

  Comment the SDP_120 #define below to enable the Arduino connector
*/

//#define SDP_120

#ifdef SDP_120

#define SPI_SS		SDP_SPI_CS_A
#define SPI_MISO	SDP_SPI_MISO
#define SPI_MOSI	SDP_SPI_MOSI
#define SPI_SCK		SDP_SPI_SCK

#else // ARDUINO

#define SPI_SS		ARDUINO_UNO_D10
#define SPI_MOSI	ARDUINO_UNO_D11
#define SPI_MISO	ARDUINO_UNO_D12
#define SPI_SCK		ARDUINO_UNO_D13

#endif

/******************************************************************************/
/********************** Variables and User Defined Data Types *****************/
/******************************************************************************/
extern struct ltc2488_dev_init ltc2488_init_str;

#endif
