/***************************************************************************//**
 *   @file    app_config_mbed.h
 *   @brief   Header file for Mbed platform configurations
********************************************************************************
 * Copyright (c) 2022 Analog Devices, Inc.
 * All rights reserved.
 *
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 * By using this software you agree to the terms of the associated
 * Analog Devices Software License Agreement.
*******************************************************************************/
#ifndef APP_CONFIG_MBED_H_
#define APP_CONFIG_MBED_H_

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/
#include <PinNames.h>
#include "mbed_spi.h"

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/
/* Pin mapping for LTC268X w.r.t Arduino Headers */
#define SPI_CSB		    ARDUINO_UNO_D10
#define SPI_HOST_SDO	ARDUINO_UNO_D11
#define SPI_HOST_SDI	ARDUINO_UNO_D12
#define SPI_SCK		    ARDUINO_UNO_D13

/******************************************************************************/
/********************** Public/Extern Declarations ****************************/
/******************************************************************************/
extern struct mbed_spi_init_param mbed_spi_extra_init_params;

#endif /* APP_CONFIG_MBED_H_ */
