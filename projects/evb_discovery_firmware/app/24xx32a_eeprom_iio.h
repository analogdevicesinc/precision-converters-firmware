/***************************************************************************//**
* @file   24xx32a_eeprom_iio.h
* @brief  Header file for 24XX32A EEPROM device IIO interfaces
********************************************************************************
* Copyright (c) 2022 Analog Devices, Inc.
* All rights reserved.
*
* This software is proprietary to Analog Devices, Inc. and its licensors.
* By using this software you agree to the terms of the associated
* Analog Devices Software License Agreement.
*******************************************************************************/
#ifndef _24XX32A_EEPROM_IIO_H_
#define _24XX32A_EEPROM_IIO_H_

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

/******************************************************************************/
/********************** Public/Extern Declarations ****************************/
/******************************************************************************/

/* Init the IIO interface */
int32_t evb_discovery_iio_init(void);

/* IIO event handler */
void evb_discovery_iio_event_handler(void);

#endif /* _24XX32A_EEPROM_IIO_H_ */
