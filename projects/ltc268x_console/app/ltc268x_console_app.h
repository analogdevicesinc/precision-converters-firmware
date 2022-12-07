/***************************************************************************//**
 *   @file   ltc268x_console_app.h
 *   @brief  Header for ltc268x console application interfaces
********************************************************************************
 * Copyright (c) 2022 Analog Devices, Inc.
 *
 * All rights reserved.
 *
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 * By using this software you agree to the terms of the associated
 * Analog Devices Software License Agreement.
*******************************************************************************/
#ifndef LTC268x_CONSOLE_APP_H_
#define LTC268x_CONSOLE_APP_H_

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/
#include "adi_console_menu.h"

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

/******************************************************************************/
/*****************************  Public Declarations ***************************/
/******************************************************************************/
int32_t ltc268x_app_initialize(void);

extern console_menu ltc268x_main_menu;
#endif  /* LTC268x_CONSOLE_APP_H_ */
