/***************************************************************************//**
 *   @file   adt7xxx_support.c
 *   @brief  Support Source File for adt7xxx
********************************************************************************
 * Copyright (c) 2021-2022 Analog Devices, Inc.
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
#include "adt7420.h"
#include "no_os_error.h"
#include "adt7xxx_support.h"

/******************************************************************************/
/********************** Macros and Constants Definitions **********************/
/******************************************************************************/

/******************************************************************************/
/************************ Functions Definitions *******************************/
/******************************************************************************/

/***************************************************************************//**
 * @brief Reads the value of a register SPI/I2C.
 *
 * @param dev              - The device structure.
 * @param register_address - Address of the register.
 * @param reg_val - Value of the concerned register
 *
 * @return 0 in case of success, negative error code otherwise
*******************************************************************************/
int32_t adt7420_get_register_address_and_value(struct adt7420_dev *dev,
		uint16_t register_address, uint16_t *reg_val)
{
	int32_t ret;

	//remap register map
	if(adt7420_is_spi(dev)) {
		switch (register_address) {
		case REG_TEMP:
			register_address = ADT7320_REG_TEMP;
			break;	 // Temperature value
		case REG_STATUS :
			register_address = ADT7320_REG_STATUS;
			break; // status info
		case REG_CONFIG :
			register_address = ADT7320_REG_CONFIG;
			break; // Configuration
		case REG_T_CRIT :
			register_address = ADT7320_REG_T_CRIT;
			break; // Temperature CRIT setpoint (147'C)
		case REG_HIST :
			register_address = ADT7320_REG_HIST;
			break;   // Temperature HYST setpoint (5'C)
		case REG_T_HIGH :
			register_address = ADT7320_REG_T_HIGH;
			break; // Temperature HIGH setpoint (64'C)
		case REG_T_LOW :
			register_address = ADT7320_REG_T_LOW;
			break;  // Temperature LOW setpoint (10'C)
		case REG_ID :
			register_address = ADT7320_REG_ID;
			break;	 // ID value
		}

		ret = adt7420_reg_read(dev, register_address, reg_val);
		if (ret) {
			return ret;
		}
	} else {
		switch (register_address) {
		/* For configurations that have an MSB register, it is enough to
		 * read the MSB register as the reigster address pointer would
		 * get auto-incremented during a 2-byte transaction */
		case REG_TEMP:
			ret = adt7420_reg_read(dev, ADT7420_REG_TEMP_MSB, reg_val);
			if (ret) {
				return ret;
			}
			break;

		case REG_STATUS:
			ret  = adt7420_reg_read(dev, ADT7420_REG_STATUS, reg_val);
			if (ret) {
				return ret;
			}
			break;

		case REG_CONFIG:
			ret = adt7420_reg_read(dev, ADT7420_REG_CONFIG, reg_val);
			if (ret) {
				return ret;
			}
			break;

		case REG_T_HIGH:
			ret  = adt7420_reg_read(dev, ADT7420_REG_T_HIGH_MSB, reg_val);
			if (ret) {
				return ret;
			}
			break;

		case REG_T_LOW:
			ret  = adt7420_reg_read(dev, ADT7420_REG_T_LOW_MSB, reg_val);
			if (ret) {
				return ret;
			}
			break;

		case REG_T_CRIT:
			ret  = adt7420_reg_read(dev, ADT7420_REG_T_CRIT_MSB, reg_val);
			if (ret) {
				return ret;
			}
			break;

		case REG_HIST:
			ret = adt7420_reg_read(dev, ADT7420_REG_HIST, reg_val);
			if (ret) {
				return ret;
			}
			break;

		case REG_ID:
			ret = adt7420_reg_read(dev, ADT7420_REG_ID, reg_val);
			if (ret) {
				return ret;
			}
			break;
		}
	}

	return ret;
}


/**************************************************************************//**
 * @brief Configure write typev register based on communication interface.
 *
 * @param dev					- The device structure.
 * @param register_address		- Register type.
 *
 * @return  register_address	- Register Address.
******************************************************************************/
uint16_t configure_write_type_registers(struct adt7420_dev *dev,
					uint16_t register_address)
{
	if (adt7420_is_spi(dev)) {
		//simple address re-map for SPI devices
		switch(register_address) {
		case REG_TEMP:
			return ADT7320_REG_TEMP;
			break; // Temperature value
		case REG_STATUS :
			return ADT7320_REG_STATUS;
			break; // status info
		case REG_CONFIG :
			return ADT7320_REG_CONFIG;
			break; // Configuration
		case REG_T_CRIT :
			return ADT7320_REG_T_CRIT;
			break; // Temperature CRIT setpoint (147'C)
		case REG_HIST :
			return ADT7320_REG_HIST;
			break; // Temperature HYST setpoint (5'C)
		case REG_T_HIGH :
			return ADT7320_REG_T_HIGH;
			break; // Temperature HIGH setpoint (64'C)
		case REG_T_LOW :
			return ADT7320_REG_T_LOW;
			break; // Temperature LOW setpoint (10'C)
		case REG_ID :
			return ADT7320_REG_ID;
			break; // ID value
		}
	} else {
		//simple address re-map for I2Cdevices
		switch(register_address) {
		case REG_TEMP:
			return ADT7420_REG_T_HIGH_MSB;
			break; // Temperature value
		case REG_STATUS :
			return ADT7420_REG_STATUS;
			break; // status info
		case REG_CONFIG :
			return ADT7420_REG_CONFIG;
			break; // Configuration
		case REG_T_CRIT :
			return ADT7420_REG_T_CRIT_MSB;
			break; // Temperature CRIT setpoint (147'C)
		case REG_HIST :
			return ADT7420_REG_HIST;
			break; // Temperature HYST setpoint (5'C)
		case REG_T_HIGH :
			return ADT7420_REG_T_HIGH_MSB;
			break; // Temperature HIGH setpoint (64'C)
		case REG_T_LOW :
			return ADT7420_REG_T_LOW_MSB;
			break; // Temperature LOW setpoint (10'C)
		case REG_ID :
			return ADT7420_REG_ID;
			break; // ID value
		}
	}

	return 0;
}

/**************************************************************************//**
 * @brief Write to a setpoint register.
 *
 * @param dev            - The device structure.
 * @param register_value - Command control bits.
 * @param data           - Data to be written in input register.
 *
 * @return  0 in case of success, negative error code otherwise
******************************************************************************/
int32_t adt7420_wr_setpoint_reg(struct adt7420_dev *dev,
				uint16_t register_value,
				uint16_t data)
{
	int32_t ret;
	uint16_t address;
	uint16_t read_back_data;

	address = configure_write_type_registers(dev, register_value);

	ret = adt7420_reg_write(dev, address, data);
	if (ret) {
		return ret;
	}

	ret  = adt7420_reg_read(dev, address, &read_back_data);
	if (ret) {
		return ret;
	}

	if (register_value == REG_HIST) {
		data &= 0x000F; //msbits are all low for HIST register
		read_back_data &= 0x000F; //msbits are all low for HIST register
	}

	return read_back_data == data ? 0 : -EINVAL;
}

/***************************************************************************//**
 * @brief Sets the Fault Queue option for ADT7420/ADT7320.
 *
 * @param dev        - The device structure.
 * @param mode       - Fault Queue selection.
 *                     Example: 1 - 1 fault (default).
 *                            	2 - 2 faults.
 *			                    3 - 3 faults.
 *			                    4 - 4 faults.
 *
 * @return 0 in case of success, negative error code otherwise.
*******************************************************************************/
int32_t adt7420_set_fault_queue(struct adt7420_dev *dev,
				uint8_t mode)
{
	uint8_t address;
	int32_t ret;

	if (adt7420_is_spi(dev)) {
		address = ADT7320_REG_CONFIG;
	} else {
		address = ADT7420_REG_CONFIG;
	}

	ret = adt7420_reg_update_bits(dev, address,
				      ADT7420_CONFIG_FAULT_QUEUE(ADT7420_FAULT_QUEUE_4_FAULTS),
				      ADT7420_CONFIG_FAULT_QUEUE(mode));
	if (ret) {
		return ret;
	}

	return ret;
}

/***************************************************************************//**
 * @brief Sets comparator/interrupt (CT/INT) mode for ADT7420/ADT7320.
 *
 * @param dev        - The device structure.
 * @param setting    - Mode selection.
 *                     Example: 0 - Interrupt (default).
 *                            	1 - Comparator.
 *
 *
 * @return 0 in case of success, negative error code otherwise.
*******************************************************************************/
int32_t adt7420_set_ct_int_mode(struct adt7420_dev *dev,
				uint8_t setting)
{
	uint8_t address;
	int32_t ret;

	if (adt7420_is_spi(dev)) {
		address = ADT7320_REG_CONFIG;
	} else {
		address = ADT7420_REG_CONFIG;
	}

	ret = adt7420_reg_update_bits(dev, address, ADT7420_CONFIG_INT_CT_MODE,
				      (setting * ADT7420_CONFIG_INT_CT_MODE));
	if (ret) {
		return ret;
	}

	return ret;
}

/***************************************************************************//**
 * @brief Sets output polarity for the pins CT/INT (Critical Temp - Over/Under Temp).
 *
 * @param dev        - The device structure.
 * @param polarity   - Polarity selection.
 *                     Example: 0 - Active Low (default).
 *                            	1 - Active High.
 *
 *
 * @return 0 in case of success, negative error code otherwise.
*******************************************************************************/
int32_t adt7420_set_ct_int_polarity(struct adt7420_dev *dev,
				    uint8_t polarity)
{
	uint8_t address;
	uint8_t bit_mask, bit_value;
	int32_t ret;

	if (adt7420_is_spi(dev)) {
		address = ADT7320_REG_CONFIG;
	} else {
		address = ADT7420_REG_CONFIG;
	}

	bit_mask = (ADT7420_CONFIG_CT_POL) | (ADT7420_CONFIG_INT_POL);
	bit_value = (polarity * ADT7420_CONFIG_CT_POL) | (polarity *
			ADT7420_CONFIG_INT_POL);

	ret = adt7420_reg_update_bits(dev, address, bit_mask, bit_value);
	if (ret) {
		return ret;
	}

	return ret;
}
