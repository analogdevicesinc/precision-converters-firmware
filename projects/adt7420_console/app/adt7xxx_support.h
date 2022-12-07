/*****************************************************************************
 *   @file   adt7xxx_support.h
 *   @brief  Support Header File for adt7xxx
******************************************************************************
 * Copyright (c) 2021-2022 Analog Devices, Inc.
 * All rights reserved.
 *
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 * By using this software you agree to the terms of the associated
 * Analog Devices Software License Agreement.
*****************************************************************************/

#ifndef _ADT7XXX_SUPPORT_H
#define _ADT7XXX_SUPPORT_H

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

/* ADT7420 bit mask */
#define ADT7420_LSB_MASK	            0x00FF
#define ADT7420_MSB_MASK	            0xFF00
#define ADT7420_LSB_OFFSET	            0
#define ADT7420_MSB_OFFSET          	    8
#define ADT7420_CONFIG_CT_POL		    NO_OS_BIT(2)
#define ADT7420_CONFIG_FAULT_QUEUE(x)	    ((x) & 0x3)
#define ADT7420_CONFIG_INT_POL		    NO_OS_BIT(3)
#define ADT7420_CONFIG_INT_CT_MODE	    NO_OS_BIT(4)
#define ADT7420_MASK_SET_PT_REGISTER	    0x00

/* ADT7420_CONFIG_FAULT_QUEUE(x) options */
#define ADT7420_FAULT_QUEUE_1_FAULT	    0
#define ADT7420_FAULT_QUEUE_2_FAULTS	    1
#define ADT7420_FAULT_QUEUE_3_FAULTS	    2
#define ADT7420_FAULT_QUEUE_4_FAULTS	    3

/* ADT7xxx default ID */
#define ADT7320_DEFAULT_ID		    0xC3
#define ADT7420_DEFAULT_ID		    0xCB

/******************************************************************************/
/******************** Variables and User Defined Data Types *******************/
/******************************************************************************/

typedef enum {
	REG_TEMP,		// Temperature value
	REG_STATUS,		// status info
	REG_CONFIG,		// Configuration
	REG_T_CRIT,		// Temperature CRIT setpoint (147'C)
	REG_HIST,		// Temperature HYST setpoint (5'C)
	REG_T_HIGH,		// Temperature HIGH setpoint (64'C)
	REG_T_LOW,		// Temperature LOW setpoint (10'C)
	REG_ID,			// ID value
	REG_RESET
} registers_e;

/******************************************************************************/
/************************** Functions Declarations ****************************/
/******************************************************************************/

/*! Sets the Fault Queue option for ADT7420/ADT7320.*/
int32_t adt7420_set_fault_queue(struct adt7420_dev *dev,
			     uint8_t mode);

/*! Sets comparator/interrupt (CT/INT) mode for ADT7420/ADT7320.*/
int32_t adt7420_set_ct_int_mode(struct adt7420_dev *dev,
			     uint8_t setting);

/*! Sets output polarity for the pins CT/INT (Critical Temp - Over/Under Temp).*/
int32_t adt7420_set_ct_int_polarity(struct adt7420_dev *dev,
				 uint8_t polarity);

/*! Writes data to temperature registers*/
int32_t adt7420_wr_setpoint_reg(struct adt7420_dev *device,
				uint16_t register_value,
				uint16_t data);

/*! Get the register address of write type registers */
uint16_t configure_write_type_registers(struct adt7420_dev *dev,
					uint16_t register_address);

/*! Get the register address based on the register type enum- registers_e */
int32_t adt7420_get_register_address_and_value(struct adt7420_dev *dev, uint16_t register_address, uint16_t *reg_val);
#endif // _ADT7XXX_SUPPORT_H