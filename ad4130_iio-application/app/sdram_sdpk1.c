/***************************************************************************//**
 *   @file    sdram_sdpk1.c
 *   @brief   SDP-K1 SDRAM interfaces
********************************************************************************
 * Copyright (c) 2022 Analog Devices, Inc.
 * All rights reserved.
 *
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 * By using this software you agree to the terms of the associated
 * Analog Devices Software License Agreement.
*******************************************************************************/

#if defined (TARGET_SDP_K1)

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include "sdp_k1_sdram.h"
#include "no_os_error.h"

/******************************************************************************/
/************************ Functions Definitions *******************************/
/******************************************************************************/

/**
 * @brief 	Initialize the SDP-K1 SDRAM
 * @return	0 in case of success, negative error code otherwise
 */
int32_t sdram_init(void)
{
	if (SDP_SDRAM_Init() != SDRAM_OK) {
		return -EIO;
	}

	return 0;
}

#endif	/* TARGET_SDP_K1 */
