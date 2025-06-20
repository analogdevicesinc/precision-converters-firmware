/***************************************************************************//**
* @file   ad7191_iio.h
* @brief  Header file for ad7191 IIO interface
********************************************************************************
* Copyright (c) 2024 Analog Devices, Inc.
* All rights reserved.
*
* This software is proprietary to Analog Devices, Inc. and its licensors.
* By using this software you agree to the terms of the associated
* Analog Devices Software License Agreement.
*******************************************************************************/
#ifndef AD7191_IIO_H_
#define AD7191_IIO_H_

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include "iio.h"
#include "iio_types.h"
#include "no_os_gpio.h"

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

/******************************************************************************/
/********************** Public/Extern Declarations ****************************/
/******************************************************************************/

/* AD7191 Device */
struct ad7191_dev {
	/* SPI */
	struct no_os_spi_desc	*spi_desc;
	/* GPIO */
	struct no_os_gpio_desc *odr1_gpio;
	struct no_os_gpio_desc *odr2_gpio;
	struct no_os_gpio_desc *pga1_gpio;
	struct no_os_gpio_desc *pga2_gpio;
	struct no_os_gpio_desc *csb_gpio;
	struct no_os_gpio_desc *rdy_gpio;
};

/* AD7191 Init Params */
struct ad7191_init_param {
	/* SPI */
	struct no_os_spi_init_param	 *spi_init;
	/*GPIO*/
	struct no_os_gpio_init_param *odr1_gpio;
	struct no_os_gpio_init_param *odr2_gpio;
	struct no_os_gpio_init_param *pga1_gpio;
	struct no_os_gpio_init_param *pga2_gpio;
	struct no_os_gpio_init_param *csb_gpio;
	struct no_os_gpio_init_param *rdy_gpio;

};

/* Init the IIO interface */
int ad7191_iio_initialize(void);

/* Run the IIO event handler */
void ad7191_iio_event_handler(void);

#endif /* AD7191_IIO_H_ */
