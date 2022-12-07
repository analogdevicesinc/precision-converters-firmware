/***************************************************************************//**
 *   @file   ad590_console_app.h
 *   @brief  Header for ad590 console application interfaces
********************************************************************************
 * Copyright (c) 2021 Analog Devices, Inc.
 *
 * All rights reserved.
 *
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 * By using this software you agree to the terms of the associated
 * Analog Devices Software License Agreement.
*******************************************************************************/

#ifndef AD590_CONSOLE_APP_H_
#define AD590_CONSOLE_APP_H_

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

int32_t ltc2488_app_initialize(void);
extern console_menu ad590_main_menu;

#endif  /* AD590_CONSOLE_APP_H_ */