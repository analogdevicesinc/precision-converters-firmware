/***************************************************************************//**
* @file   ad2s1210_iio.h
* @brief  Header file for AD2S1210 IIO interface
********************************************************************************
* Copyright (c) 2023 Analog Devices, Inc.
* Copyright (c) 2023 BayLibre, SAS.
* All rights reserved.
*
* This software is proprietary to Analog Devices, Inc. and its licensors.
* By using this software you agree to the terms of the associated
* Analog Devices Software License Agreement.
*******************************************************************************/
#ifndef _AD2S1210_IIO_H_
#define _AD2S1210_IIO_H_

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include "iio.h"

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

/******************************************************************************/
/********************** Public/Extern Declarations ****************************/
/******************************************************************************/
/* Init the IIO interface */
int32_t ad2s1210_iio_initialize(void);

/* Run the IIO event handler */
void ad2s1210_iio_event_handler(void);

#endif /* _AD2S1210_IIO_H_ */
