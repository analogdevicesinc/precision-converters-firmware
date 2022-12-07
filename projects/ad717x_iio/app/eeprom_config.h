/***************************************************************************//**
 *   @file    eeprom_config.h
 *   @brief   EEPROM coniguration header file
********************************************************************************
 * Copyright (c) 2022 Analog Devices, Inc.
 * All rights reserved.
 *
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 * By using this software you agree to the terms of the associated
 * Analog Devices Software License Agreement.
*******************************************************************************/

#ifndef EEPROM_CONFIG_H_
#define EEPROM_CONFIG_H_

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include <stdint.h>
#include "24xx32a.h"
#include "no_os_eeprom.h"

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

/* EEPROM valid device address range */
#define EEPROM_DEV_ADDR_START	0x50
#define EEPROM_DEV_ADDR_END		0x57

#define eeprom_ops	eeprom_24xx32a_ops

/******************************************************************************/
/************************ Public Declarations *********************************/
/******************************************************************************/

extern struct eeprom_24xx32a_init_param eeprom_extra_init_params;

int32_t load_eeprom_dev_address(struct no_os_eeprom_desc *eeprom_desc,
	uint8_t dev_addr);

#endif	/* end of EEPROM_CONFIG_H_ */