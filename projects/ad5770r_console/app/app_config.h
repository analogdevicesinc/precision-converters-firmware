/*****************************************************************************
 *   @file   app_config.h
 *
 *   @brief  Configuration file of AD5770R firmware example program
******************************************************************************
 *
Copyright (c) 2020-2022 Analog Devices, Inc. All Rights Reserved.

This software is proprietary to Analog Devices, Inc. and its licensors.
By using this software you agree to the terms of the associated
Analog Devices Software License Agreement.
 ******************************************************************************/

#ifndef APP_CONFIG_H_
#define APP_CONFIG_H_

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include <stdint.h>
#include <PinNames.h>

/******************************************************************************/
/********************** Macros and Constants Definitions **********************/
/******************************************************************************/

/**
  The ADI SDP_K1 can be used with either arduino headers
  or the 120-pin SDP connector found on ADI evaluation
  boards. The default is the SDP-120 connector.

  Uncomment the ARDUINO #define to enable the ARDUINO connector
*/

//#define  ARDUINO

// Pin mapping of AD5770R with SDP-K1/Arduino
#ifdef ARDUINO
#define SPI_CSB			ARDUINO_UNO_D10
#define SPI_HOST_SDO	ARDUINO_UNO_D11
#define SPI_HOST_SDI	ARDUINO_UNO_D12
#define SPI_SCK			ARDUINO_UNO_D13
#define HW_LDACB		ARDUINO_UNO_D2
#else
#define SPI_CSB		    SDP_SPI_CS_A	// PB_9
#define SPI_HOST_SDO	SDP_SPI_MOSI	// PF_9
#define SPI_HOST_SDI	SDP_SPI_MISO	// PF_8
#define SPI_SCK		    SDP_SPI_SCK		// PH_6
#define HW_LDACB        SDP_GPIO_0      // PJ_0
#endif

#endif /* APP_CONFIG_H_ */
