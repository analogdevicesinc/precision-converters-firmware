/***************************************************************************//**
 *   @file    sdram_sdpk1.c
 *   @brief   SDP-K1 SDRAM interfaces header file
********************************************************************************
 * Copyright (c) 2022 Analog Devices, Inc.
 * All rights reserved.
 *
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 * By using this software you agree to the terms of the associated
 * Analog Devices Software License Agreement.
*******************************************************************************/

#if defined (TARGET_SDP_K1)

#ifndef SDRAM_SDPK1_
#define SDRAM_SDPK1_

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include <stdint.h>

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

/* SDRAM configs for SDP-K1 */
#define SDRAM_START_ADDRESS		(volatile int8_t *)0xC0000000
#define SDRAM_SIZE_BYTES		(16777216)	// 16MBytes

/******************************************************************************/
/************************ Public Declarations *********************************/
/******************************************************************************/

int32_t sdram_init(void);

#endif	/* end of SDRAM_SDPK1_ */

#endif	/* TARGET_SDP_K1 */