/***************************************************************************//**
 *   @file    app_support.c
 *   @brief   Implementation of Application support functions
 *   @details This module has all the support file necessary for working of app
********************************************************************************
 * Copyright (c) 2025 Analog Devices, Inc.
 *
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 * By using this software you agree to the terms of the associated
 * Analog Devices Software License Agreement.
*******************************************************************************/

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/
#include <stdint.h>

#include "ad405x.h"
#include "app_config.h"

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

extern const struct ad405x_support_desc ad405x_support_descriptor;

/******************************************************************************/
/********************** Variables and User Defined Data Types *****************/
/******************************************************************************/
const struct ad405x_support_desc *support_desc[] =  {
	[ID_AD4050] = &ad405x_support_descriptor,
	[ID_AD4052] = &ad405x_support_descriptor
};

/* Global variable for number of samples */
uint32_t nb_of_bytes_g;

/* Global variable for data read from CB functions */
uint32_t data_read;
