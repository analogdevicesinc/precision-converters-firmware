/*************************************************************************//**
 *   @file   app_config.h
 *   @brief  Configuration file of AD7124 firmware example program
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

#include <stdint.h>
#include <PinNames.h>

/**
*  The ADI SDP_K1 can be used with both arduino headers
*  or the 120-pin SDP connector found on ADI evaluation
*  boards. The default is the Arduino connector.
*
* Uncomment the SDP_120 #define below to enable the SDP-120 connector
*/

//#define  SDP_120

#ifdef SDP_120
#define SPI_CSB 	    SDP_SPI_CS_A
#define SPI_HOST_SDI	SDP_SPI_MISO
#define SPI_HOST_SDO	SDP_SPI_MOSI
#define SPI_SCK		    SDP_SPI_SCK
#else
#define SPI_CSB 	    ARDUINO_UNO_D10
#define SPI_HOST_SDI 	ARDUINO_UNO_D12
#define SPI_HOST_SDO 	ARDUINO_UNO_D11
#define SPI_SCK 		  ARDUINO_UNO_D13
#endif

// Common pin mappings
#define LED_GREEN	LED3	// PK_5

#endif //_APP_CONFIG_H_
