/***************************************************************************//**
 *   @file    24xx32a_eeprom_iio.c
 *   @brief   Implementation of 24XX32A EEPROM device IIO application interfaces
********************************************************************************
 * Copyright (c) 2022-23 Analog Devices, Inc.
 * All rights reserved.
 *
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 * By using this software you agree to the terms of the associated
 * Analog Devices Software License Agreement.
*******************************************************************************/

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "24xx32a_eeprom_iio.h"
#include "app_config.h"
#include "common.h"
#include "no_os_error.h"
#include "iio.h"
#include "iio_types.h"

/* Forward declaration of functions */
static int get_eeprom_dev_addr(void *device, char *buf,
			       uint32_t len, const struct iio_ch_info *channel,
			       intptr_t id);
static int set_eeprom_dev_addr(void *device, char *buf,
			       uint32_t len, const struct iio_ch_info *channel,
			       intptr_t id);

/******************************************************************************/
/********************* Macros and Constants Definition ************************/
/******************************************************************************/

/*	Number of IIO devices */
#define NUM_OF_IIO_DEVICES	1

/******************************************************************************/
/******************** Variables and User Defined Data Types *******************/
/******************************************************************************/

/* IIO interface descriptor */
static struct iio_desc *evb_discovery_iio_desc;

/* EVB HW validation status */
static bool hw_mezzanine_is_valid;

/* IIOD channels attributes list */
struct iio_attribute channel_input_attributes[] = {
	END_ATTRIBUTES_ARRAY
};

/* IIOD device (global) attributes list */
static struct iio_attribute global_attributes[] = {
	{
		.name = "dev_address",
		.show = get_eeprom_dev_addr,
		.store = set_eeprom_dev_addr,
	},

	END_ATTRIBUTES_ARRAY
};

/******************************************************************************/
/************************** Functions Declarations ****************************/
/******************************************************************************/

/******************************************************************************/
/************************ Functions Definitions *******************************/
/******************************************************************************/

/*!
 * @brief	Getter/Setter for the EEPROM device address attribute
 * @param	device[in,out]- pointer to IIO device structure
 * @param	buf- pointer to buffer holding attribute value
 * @param	len- length of buffer string data
 * @param	channel- pointer to IIO channel structure
 * @param	id- Attribute ID (optional)
 * @return	Number of characters read/written
 */
static int get_eeprom_dev_addr(void *device,
			       char *buf,
			       uint32_t len,
			       const struct iio_ch_info *channel,
			       intptr_t id)
{
	uint8_t dev_addr;

	dev_addr = get_eeprom_detected_dev_addr();
	return sprintf(buf, "0x%x", dev_addr);
}

static int set_eeprom_dev_addr(void *device,
			       char *buf,
			       uint32_t len,
			       const struct iio_ch_info *channel,
			       intptr_t id)
{
	/* Read-only attribute */
	return len;
}

/*!
 * @brief	Read the device register value
 * @param	dev[in]- Pointer to IIO device instance
 * @param	reg[in]- Register address to read from
 * @param	readval[out]- Pointer to variable to read data into
 * @return	0 in case of success, negative error code otherwise
 */
int32_t debug_reg_read(void *dev, uint32_t reg, uint32_t *readval)
{
	int32_t ret;

	if (!dev || !readval || (reg > MAX_REGISTER_ADDRESS)) {
		return -EINVAL;
	}

	/* Read data from device register */
	ret = no_os_eeprom_read(dev, reg, (uint8_t *)readval, 1);
	if (ret) {
		return ret;
	}

	return 0;
}

/*!
 * @brief	Write into the device register (single-byte write)
 * @param	dev[in] - Pointer to IIO device instance
 * @param	reg[in] - Register address to write into
 * @param	writeval[in] - Register value to write
 * @return	0 in case of success, negative error code otherwise
 */
int32_t debug_reg_write(void *dev, uint32_t reg, uint32_t writeval)
{
	int32_t ret;

	if (!dev || (reg > MAX_REGISTER_ADDRESS)) {
		return -EINVAL;
	}

	/* Write data into device register */
	ret = no_os_eeprom_write(dev, reg, (uint8_t *)&writeval, 1);
	if (ret) {
		return ret;
	}

	return 0;
}

/**
 * @brief	Init IIO device parameters
 * @param 	desc[in,out] - IIO device descriptor
 * @return	0 in case of success, negative error code otherwise
 */
static int32_t evb_discovery_iio_dev_init(struct iio_device **desc)
{
	struct iio_device *iio_dev;

	iio_dev = calloc(1, sizeof(struct iio_device));
	if (!iio_dev) {
		return -ENOMEM;
	}

	iio_dev->num_ch = 0;
	iio_dev->channels = NULL;
	iio_dev->attributes = global_attributes;
	iio_dev->debug_reg_read = debug_reg_read;
	iio_dev->debug_reg_write = debug_reg_write;

	*desc = iio_dev;

	return 0;
}

/**
 * @brief	Initialize the IIO interface
 * @return	none
 * @return	0 in case of success, negative error code otherwise
 */
int32_t evb_discovery_iio_init(void)
{
	int32_t init_status;

	/* IIO device descriptors */
	struct iio_device *evb_discovery_iio_dev[NUM_OF_IIO_DEVICES] = {
		NULL
	};

	/* IIO device init parameters */
	static struct iio_device_init iio_device_init_params[NUM_OF_IIO_DEVICES];

	/* IIO interface init parameters */
	static struct iio_init_param iio_init_params = {
		.phy_type = USE_UART,
	};

	/* Init the system peripherals */
	init_status = init_system();
	if (init_status) {
		return init_status;
	}

	/* Read context attributes */
	init_status = get_iio_context_attributes(&iio_init_params.ctx_attrs,
			&iio_init_params.nb_ctx_attr,
			eeprom_desc,
			NULL,
			STR(HW_CARRIER_NAME),
			&hw_mezzanine_is_valid);
	if (init_status) {
		return init_status;
	}

#if defined(ENABLE_EVB_EEPROM_IIO_DEV)
	if (is_eeprom_valid_dev_addr_detected()) {
		/* Initialize the IIO device */
		init_status = evb_discovery_iio_dev_init(&evb_discovery_iio_dev[0]);
		if (init_status) {
			return init_status;
		}

		iio_device_init_params[0].name = "24xx32a";
		iio_device_init_params[0].dev = eeprom_desc;
		iio_device_init_params[0].dev_descriptor = evb_discovery_iio_dev[0];

		iio_init_params.nb_devs++;
	}
#endif

	/* Initialize the IIO interface */
	iio_init_params.uart_desc = uart_desc;
	iio_init_params.devs = iio_device_init_params;
	init_status = iio_init(&evb_discovery_iio_desc, &iio_init_params);
	if (init_status) {
		return init_status;
	}

	return 0;
}

/**
 * @brief 	Run the IIO event handler
 * @return	none
 * @details	This function monitors the new IIO client event
 */
void evb_discovery_iio_event_handler(void)
{
	(void)iio_step(evb_discovery_iio_desc);
}
