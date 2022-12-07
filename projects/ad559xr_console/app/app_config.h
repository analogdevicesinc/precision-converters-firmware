/*!
 *****************************************************************************
  @file:  app_config.h
  @brief: AD5592R/AD5593R device selection. Pin mappings.
  @details:
 -----------------------------------------------------------------------------
 Copyright (c) 2020, 2022 Analog Devices, Inc.
 All rights reserved.

 This software is proprietary to Analog Devices, Inc. and its licensors.
 By using this software you agree to the terms of the associated
 Analog Devices Software License Agreement.
*****************************************************************************/

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/
#ifndef _APP_CONFIG_H_
#define _APP_CONFIG_H_

#include <stdint.h>
#include <PinNames.h>

/******************************************************************************/
/************************* Macros & Constant Definitions ***************************/
/******************************************************************************/
// Supported Devices
#define DEV_AD5592R 0
#define DEV_AD5593R 1

#define AD5593R_A0_STATE 0

// **** Note for User: ACTIVE_DEVICE selection ****
// Define the device type here from the list of below device type defines
// e.g. #define ACTIVE_DEVICE ID_AD5593R -> This will set AD5593R as an ACTIVE_DEVICE.
// The ACTIVE_DEVICE is default set to AD5592R, if device type is not defined.

#if !defined(ACTIVE_DEVICE)
#define ACTIVE_DEVICE  DEV_AD5592R
#endif

/**
  The ADI SDP_K1 can be used with either arduino headers
  or the 120-pin SDP connector found on ADI evaluation
  boards. The default is the SDP connector.

  Uncomment the ARDUINO #define below to enable the ARDUINO connector
  This is required for most other boards
*/

//#define  ARDUINO

#define NUM_CHANNELS 8

#define AD5593R_I2C (0x10 | (AD5593R_A0_STATE & 0x01))

// Pin mapping of with SDP-120 or Arduino connectors
#ifdef ARDUINO
#define I2C_SCL		    ARDUINO_UNO_D15		// I2C_SCL
#define I2C_SDA		    ARDUINO_UNO_D14		// I2C_SDA

#define SPI_CSB	        ARDUINO_UNO_D10		// SPI_CS
#define SPI_HOST_SDO 	ARDUINO_UNO_D11		// SPI_MOSI
#define SPI_HOST_SDI  	ARDUINO_UNO_D12		// SPI_MISO
#define SPI_SCK		    ARDUINO_UNO_D13		// SPI_SCK

#define GAIN_PIN	    ARDUINO_UNO_D8
#define RESET_PIN	    ARDUINO_UNO_D9
#define LDAC_PIN	    ARDUINO_UNO_D7
#define ADDR0_PIN	    ARDUINO_UNO_D6
#else
// SDP-120 connector
#define I2C_SCL		    SDP_I2C_SCL		// PH_7
#define I2C_SDA		    SDP_I2C_SDA		// PC_9

#define SPI_CSB		    SDP_SPI_CS_A	// PB_9
#define SPI_HOST_SDI 	SDP_SPI_MISO	// PF_8
#define SPI_HOST_SDO	SDP_SPI_MOSI	// PF_9
#define SPI_SCK		    SDP_SPI_SCK		// PH_6
#endif

#endif //_APP_CONFIG_H_
