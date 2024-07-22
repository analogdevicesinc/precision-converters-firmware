/*************************************************************************//**
 *   @file   cn0586_support.h
 *   @brief  Header file for CN0586 supporting APIs
******************************************************************************
* Copyright (c) 2024 Analog Devices, Inc.
*
* This software is proprietary to Analog Devices, Inc. and its licensors.
* By using this software you agree to the terms of the associated
* Analog Devices Software License Agreement.
*****************************************************************************/

#ifndef AD5754R_SUPPORT_H
#define AD5754R_SUPPORT_H

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/
#include <stdint.h>
#include "app_config.h"
/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

/******************************************************************************/
/********************** Variables and User Defined Data Types *****************/
/******************************************************************************/
/**
 * @enum cn0586_range
 * @brief CN0586 HVOUT Range options
 */
enum cn0586_range {
	HVOUT_0V_100V,
	HVOUT_M100V_100V,
	HVOUT_M50V_50V,
	HVOUT_0V_200V,
	NUM_OF_HVOUT_RANGES
};

/**
 * @enum cn0586_hvout_state
 * @brief CN0586 HVOUT State options
 */
enum cn0586_hvout_state {
	HVOUT_DISABLED,
	HVOUT_ENABLED,
	NUM_OF_HVOUT_STATES
};

/**
 * @struct cn0586_dev
 * @brief cn0586 CFTL structure.
 */
struct cn0586_dev {
	/* Converter Descriptor */
	struct ad5754r_dev *dev;
	/* HVOUT State */
	bool state;
	/* HVOUT Range */
	enum cn0586_range range;
	/* HVOUT volts */
	float hvout_volts;
};

/******************************************************************************/
/********************** Public/Extern Declarations ****************************/
/******************************************************************************/
int32_t cn0586_init(struct cn0586_dev **dev,
		    struct ad5754r_dev *ad5754r_device);

int32_t cn0586_set_hvout_range(struct cn0586_dev *dev, enum cn0586_range range);

int32_t cn0586_set_hvout_volts(struct cn0586_dev *dev, float volts);

int32_t cn0586_get_hvout_volts(struct cn0586_dev *dev);

#endif /* CN0586_SUPPORT_H */