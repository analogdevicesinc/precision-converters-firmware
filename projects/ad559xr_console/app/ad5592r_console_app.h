/*!
 *****************************************************************************
 *   @file   ad5592r_console_app.h
 *   @brief  AD5592R console application interfaces
 *   @details:
 -----------------------------------------------------------------------------
 Copyright (c) 2020 Analog Devices, Inc.
 All rights reserved.

 This software is proprietary to Analog Devices, Inc. and its licensors.
 By using this software you agree to the terms of the associated
 Analog Devices Software License Agreement.
*****************************************************************************/

#ifndef AD5592R_CONSOLE_APP_H_
#define AD5592R_CONSOLE_APP_H_

#ifdef __cplusplus
extern "C"
{
#endif

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/
#include "adi_console_menu.h"

/******************************************************************************/
/*****************************  Public Declarations ********************************/
/******************************************************************************/
int32_t ad5592r_app_initalization(void);
extern console_menu ad5592r_main_menu;

#ifdef __cplusplus
}
#endif

#endif  /* AD5592R_CONSOLE_APP_H_ */
