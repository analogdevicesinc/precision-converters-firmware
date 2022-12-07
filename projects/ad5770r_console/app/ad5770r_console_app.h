/*!
 *****************************************************************************
  @file:  ad5770r_console_app.h

  @brief: defines the console menu structure for the AD5770R example code

  @details:
 -----------------------------------------------------------------------------
 *
Copyright (c) 2020-2021 Analog Devices, Inc. All Rights Reserved.

This software is proprietary to Analog Devices, Inc. and its licensors.
By using this software you agree to the terms of the associated
Analog Devices Software License Agreement.
 ******************************************************************************/

#ifndef AD5770R_CONSOLE_APP_H_
#define AD5770R_CONSOLE_APP_H_

#include <stdint.h>

#include "adi_console_menu.h"

/* #defines */

/* Public Declarations */
int32_t ad5770r_app_initialize(void);

extern console_menu ad5770r_main_menu;

#endif /* AD5770R_CONSOLE_APP_H_ */

