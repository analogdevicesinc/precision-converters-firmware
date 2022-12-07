/***************************************************************************//*
 * @file    app_config.h
 * @brief   Configuration file of AD7124 firmware example program
 * @details
******************************************************************************
 * Copyright (c) 2021-22 Analog Devices, Inc. All Rights Reserved.
 *
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 * By using this software you agree to the terms of the associated
 * Analog Devices Software License Agreement.
******************************************************************************/

#ifndef APP_CONFIG_H_
#define APP_CONFIG_H_

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include <stdint.h>
#include <PinNames.h>

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

/* Supported AD7124 devices (One selected at a time, default is AD7124-4) */
#define		AD7124_4
//#define		AD7124_8

#if defined(AD7124_4)
#define ACTIVE_DEVICE	"AD7124-4"
#elif defined(AD7124_8)
#define ACTIVE_DEVICE	"AD7124-8"
#else
#define ACTIVE_DEVICE	"AD7124-8"
#warning "No active device selected. AD7124-8 is assumed as default"
#endif

/**
  The ADI SDP_K1 can be used with either arduino headers or the 120-pin SDP connector found on
  ADI evaluation boards. The default interface used in the firmware is Arduino.
  Uncomment the SDP_120 #define below to enable the SDP_120 connector interface.
*/

//#define  SDP_120

// Pin mapping of AD7124 w.r.t SDP_120/Arduino
#ifdef SDP_120
#define SPI_SS		SDP_SPI_CS_A	// PB_9
#define SPI_MISO	SDP_SPI_MISO	// PF_8
#define SPI_MOSI	SDP_SPI_MOSI	// PF_9
#define SPI_SCK		SDP_SPI_SCK		// PH_6
#else
#define SPI_SS		ARDUINO_UNO_D10		// SPI_CS
#define SPI_MOSI	ARDUINO_UNO_D11		// SPI_MOSI
#define SPI_MISO	ARDUINO_UNO_D12		// SPI_MISO
#define SPI_SCK		ARDUINO_UNO_D13		// SPI_SCK
#endif

/******************************************************************************/
/********************** Public/Extern Declarations ****************************/
/******************************************************************************/

#endif /* APP_CONFIG_H_ */
