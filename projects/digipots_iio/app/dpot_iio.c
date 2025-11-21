/***************************************************************************//**
 *   @file    dpot_iio.c
 *   @brief   Digipots IIO interface module
********************************************************************************
Copyright 2025(c) Analog Devices, Inc.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of Analog Devices, Inc. nor the names of its
   contributors may be used to endorse or promote products derived from this
   software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY ANALOG DEVICES, INC. “AS IS” AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
EVENT SHALL ANALOG DEVICES, INC. BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*******************************************************************************/

#include <stdbool.h>
#include "dpot_iio.h"
#include "dpot_user_config.h"
#include "dpot_support.h"
#include "version.h"
#include "no_os_delay.h"

/******** Forward declaration of getter/setter functions ********/
static int dpot_iio_attr_get(void *device, char *buf, uint32_t len,
			     const struct iio_ch_info *channel, intptr_t priv);

static int dpot_iio_attr_set(void *device, char *buf, uint32_t len,
			     const struct iio_ch_info *channel, intptr_t priv);

static int dpot_iio_attr_available_get(void *device, char *buf, uint32_t len,
				       const struct iio_ch_info *channel, intptr_t priv);

static int dpot_iio_attr_available_set(void *device, char *buf, uint32_t len,
				       const struct iio_ch_info *channel, intptr_t priv);
static struct scan_type chn_scan = {
	.sign = 'u',
	.realbits = 8,
	.storagebits = 32,
	.is_big_endian = false
};

/******************************************************************************/
/************************ Macros/Constants ************************************/
/******************************************************************************/

/* Number of IIO devices */
#ifdef DPOT_ADD_BOARD_DEVICE
#define NUM_OF_IIO_DEV   2
#else
#define NUM_OF_IIO_DEV   1
#endif

#define DPOT_CHN_ATTR(_name, _priv) {\
	.name = _name,\
	.priv = _priv,\
	.show = dpot_iio_attr_get,\
	.store = dpot_iio_attr_set\
}

#define DPOT_CHN_AVAIL_ATTR(_name, _priv) {\
	.name = _name,\
	.priv = _priv,\
	.show = dpot_iio_attr_available_get,\
	.store = dpot_iio_attr_available_set\
}

#define DPOT_CH(_name, _dev, _idx, _type,attr) {\
	.name = _name, \
	.ch_type = _type,\
	.ch_out = 0,\
	.indexed = true,\
	.channel = _idx,\
	.scan_index = _idx,\
	.scan_type = &chn_scan,\
	.attributes = attr[_dev]\
}

/******************************************************************************/
/*************************** Types Declarations *******************************/
/******************************************************************************/

/* Digipot device descriptor */
struct dpot_dev *dpot_dev_desc;

static enum dpot_dev_id supported_generics[] = {
	DEV_AD5141,
	DEV_AD5142,
	DEV_AD5142A,
	DEV_AD5143,
	DEV_AD5144,
	DEV_AD5160,
	DEV_AD5161,
	DEV_AD5165,
	DEV_AD5171,
	DEV_AD5241,
	DEV_AD5242,
	DEV_AD5245,
	DEV_AD5246,
	DEV_AD5258,
	DEV_AD5259,
	DEV_AD5273,
};

#ifdef DPOT_ADD_BOARD_DEVICE
/* IIO Board device descriptor */
typedef struct dpot_board {
	uint8_t active_device_family;
	uint8_t operating_mode;
} dpot_board;

/* Digipot IIO descriptor for system level config. */
static dpot_board *board_dev;
#endif

/* Digipot IIO descriptor */
static struct iio_desc *dpot_iio_desc;

/* Digipot IIO device descriptor */
static struct iio_device *dpot_iio_dev[NUM_OF_IIO_DEV];

/* Digipot IIO device init parameters */
static struct iio_device_init dpot_iio_dev_init_params[NUM_OF_IIO_DEV];

/* Digipot IIO interface init parameters */
static struct iio_init_param dpot_iio_init_params = {
	.phy_type = USE_UART,
};

/* Attribute IDs */
enum dpot_iio_attr_id {
	// Channel attributes
	DPOT_RAW_ATTR_ID,
	DPOT_SCALE_ATTR_ID,
	DPOT_INPUT_REG_VAL_ATTR_ID,
	DPOT_EEPROM_VAL_ATTR_ID,
	DPOT_RDAC_LINEAR_ATTR_ID,
	DPOT_RDAC_LINEAR_AVL_ATTR_ID,
	DPOT_RDAC_6DB_ATTR_ID,
	DPOT_RDAC_6DB_AVL_ATTR_ID,
	DPOT_SHUTDOWN_VAL_ATTR_ID,
	DPOT_SHUTDOWN_VAL_AVL_ATTR_ID,
	DPOT_SW_LRDAC_ATTR_ID,
	DPOT_SW_LRDAC_AVL_ATTR_ID,
	DPOT_COPY_RDAC_TO_EEPROM_ATTR_ID,
	DPOT_COPY_RDAC_TO_EEPROM_AVL_ATTR_ID,
	DPOT_COPY_EEPROM_TO_RDAC_ATTR_ID,
	DPOT_COPY_EEPROM_TO_RDAC_AVL_ATTR_ID,
	DPOT_TOP_SCALE_ID,
	DPOT_TOP_SCALE_ID_AVL_ATTR_ID,
	DPOT_BOTTOM_SCALE_ID,
	DPOT_BOTTOM_SCALE_ID_AVL_ATTR_ID,
	DPOT_TOLERANCE_ATTR_ID,
	DPOT_SET_MID_SCALE_ATTR_ID,
	DPOT_SET_MID_SCALE_AVL_ATTR_ID,
	NUM_OF_CHN_ATTR,
	// Device attributes
	DPOT_OPERATING_MODE_ATTR_ID,
	DPOT_OPERATING_MODE_AVL_ATTR_ID,
	DPOT_RDAC_WP_ATTR_ID,
	DPOT_RDAC_WP_AVL_ATTR_ID,
	DPOT_NVM_PROGRAMMING_ATTR_ID,
	DPOT_NVM_PROGRAMMING_AVL_ATTR_ID,
	DPOT_RESTART_IIO_ATTR_ID,
	DPOT_DEVICE_GENERIC_ATTR_ID,
	DPOT_DEVICE_GENERIC_AVL_ATTR_ID,
	DPOT_INTERFACE_ATTR_ID,
	DPOT_INTERFACE_AVL_ATTR_ID,

	NUM_OF_DEV_ATTR = DPOT_INTERFACE_AVL_ATTR_ID - NUM_OF_CHN_ATTR
};

/* IIOD channels attributes list */
static struct iio_attribute dpot_iio_chn_attr[NUM_OF_IIO_DEV][NUM_OF_CHN_ATTR +
			1]
	= {
	{
		DPOT_CHN_ATTR("raw", DPOT_RAW_ATTR_ID),
		DPOT_CHN_ATTR("scale", DPOT_SCALE_ATTR_ID),
		DPOT_CHN_ATTR("input_reg_val", DPOT_INPUT_REG_VAL_ATTR_ID),
		DPOT_CHN_ATTR("eeprom_value", DPOT_EEPROM_VAL_ATTR_ID),
		DPOT_CHN_ATTR("rdac_linear", DPOT_RDAC_LINEAR_ATTR_ID),
		DPOT_CHN_AVAIL_ATTR("rdac_linear_available", DPOT_RDAC_LINEAR_AVL_ATTR_ID),
		DPOT_CHN_ATTR("rdac_6db", DPOT_RDAC_6DB_ATTR_ID),
		DPOT_CHN_AVAIL_ATTR("rdac_6db_available", DPOT_RDAC_6DB_AVL_ATTR_ID),
		DPOT_CHN_ATTR("shutdown", DPOT_SHUTDOWN_VAL_ATTR_ID),
		DPOT_CHN_AVAIL_ATTR("shutdown_available", DPOT_SHUTDOWN_VAL_AVL_ATTR_ID),
		DPOT_CHN_ATTR("sw_lrdac", DPOT_SW_LRDAC_ATTR_ID),
		DPOT_CHN_AVAIL_ATTR("sw_lrdac_available", DPOT_SW_LRDAC_AVL_ATTR_ID),
		DPOT_CHN_ATTR("copy_rdac_to_eeprom", DPOT_COPY_RDAC_TO_EEPROM_ATTR_ID),
		DPOT_CHN_AVAIL_ATTR("copy_rdac_to_eeprom_available", DPOT_COPY_RDAC_TO_EEPROM_AVL_ATTR_ID),
		DPOT_CHN_ATTR("copy_eeprom_to_rdac", DPOT_COPY_EEPROM_TO_RDAC_ATTR_ID),
		DPOT_CHN_AVAIL_ATTR("copy_eeprom_to_rdac_available", DPOT_COPY_EEPROM_TO_RDAC_AVL_ATTR_ID),
		DPOT_CHN_ATTR("top_scale_option", DPOT_TOP_SCALE_ID),
		DPOT_CHN_AVAIL_ATTR("top_scale_option_available", DPOT_TOP_SCALE_ID_AVL_ATTR_ID),
		DPOT_CHN_ATTR("bottom_scale_option", DPOT_BOTTOM_SCALE_ID),
		DPOT_CHN_AVAIL_ATTR("bottom_scale_option_available", DPOT_BOTTOM_SCALE_ID_AVL_ATTR_ID),
		END_ATTRIBUTES_ARRAY
	}
};

/* IIOD channels attributes list */
static struct iio_attribute
	dpot_iio_chn_attr_5259[NUM_OF_IIO_DEV][NUM_OF_CHN_ATTR + 1]
	= {
	{
		DPOT_CHN_ATTR("raw", DPOT_RAW_ATTR_ID),
		DPOT_CHN_ATTR("eeprom_value", DPOT_EEPROM_VAL_ATTR_ID),
		DPOT_CHN_ATTR("copy_rdac_to_eeprom", DPOT_COPY_RDAC_TO_EEPROM_ATTR_ID),
		DPOT_CHN_AVAIL_ATTR("copy_rdac_to_eeprom_available", DPOT_COPY_RDAC_TO_EEPROM_AVL_ATTR_ID),
		DPOT_CHN_ATTR("copy_eeprom_to_rdac", DPOT_COPY_EEPROM_TO_RDAC_ATTR_ID),
		DPOT_CHN_AVAIL_ATTR("copy_eeprom_to_rdac_available", DPOT_COPY_EEPROM_TO_RDAC_AVL_ATTR_ID),
		DPOT_CHN_ATTR("read_tolerance", DPOT_TOLERANCE_ATTR_ID),
		END_ATTRIBUTES_ARRAY
	}
};

static struct iio_attribute
	dpot_iio_chn_attr_5161[NUM_OF_IIO_DEV][NUM_OF_CHN_ATTR + 1] = {
	{
		DPOT_CHN_ATTR("raw", DPOT_RAW_ATTR_ID),
		DPOT_CHN_ATTR("shutdown", DPOT_SHUTDOWN_VAL_ATTR_ID),
		DPOT_CHN_AVAIL_ATTR("shutdown_available", DPOT_SHUTDOWN_VAL_AVL_ATTR_ID),
		DPOT_CHN_ATTR("enable_mid_scale", DPOT_SET_MID_SCALE_ATTR_ID),
		DPOT_CHN_AVAIL_ATTR("enable_mid_scale_available", DPOT_SET_MID_SCALE_AVL_ATTR_ID),
		END_ATTRIBUTES_ARRAY
	}
};

static struct iio_attribute
	dpot_iio_chn_attr_5242[NUM_OF_IIO_DEV][NUM_OF_CHN_ATTR + 1] = {
	{
		DPOT_CHN_ATTR("raw", DPOT_RAW_ATTR_ID),
		DPOT_CHN_ATTR("shutdown", DPOT_SHUTDOWN_VAL_ATTR_ID),
		DPOT_CHN_AVAIL_ATTR("shutdown_available", DPOT_SHUTDOWN_VAL_AVL_ATTR_ID),
		DPOT_CHN_ATTR("enable_mid_scale", DPOT_SET_MID_SCALE_ATTR_ID),
		DPOT_CHN_AVAIL_ATTR("enable_mid_scale_available", DPOT_SET_MID_SCALE_AVL_ATTR_ID),
		END_ATTRIBUTES_ARRAY
	}
};

static struct iio_attribute
	dpot_iio_chn_attr_5246[NUM_OF_IIO_DEV][NUM_OF_CHN_ATTR + 1] = {
	{
		DPOT_CHN_ATTR("raw", DPOT_RAW_ATTR_ID),
		END_ATTRIBUTES_ARRAY
	}
};

/* IIOD device (global) attributes list */
static struct iio_attribute dpot_iio_dev_attr[NUM_OF_IIO_DEV][NUM_OF_DEV_ATTR
			+2]
	= {
	{
		/* No Global Attributes */
		END_ATTRIBUTES_ARRAY
	},
#ifdef DPOT_ADD_BOARD_DEVICE
	{

		DPOT_CHN_ATTR("device_generic", DPOT_DEVICE_GENERIC_ATTR_ID),
		DPOT_CHN_AVAIL_ATTR("device_generic_available", DPOT_DEVICE_GENERIC_AVL_ATTR_ID),
		DPOT_CHN_ATTR("comm_interface", DPOT_INTERFACE_ATTR_ID),
		DPOT_CHN_AVAIL_ATTR("comm_interface_available", DPOT_INTERFACE_AVL_ATTR_ID),
		DPOT_CHN_ATTR("operating_mode", DPOT_OPERATING_MODE_ATTR_ID),
		DPOT_CHN_AVAIL_ATTR("operating_mode_available", DPOT_OPERATING_MODE_AVL_ATTR_ID),
		DPOT_CHN_ATTR("reconfigure_system", DPOT_RESTART_IIO_ATTR_ID),
		DPOT_CHN_AVAIL_ATTR("reconfigure_system_available", DPOT_RESTART_IIO_ATTR_ID),
		END_ATTRIBUTES_ARRAY
	}
#endif
};

/* IIOD device (global) attributes list for AD514x */
static struct iio_attribute
	dpot_iio_dev_attr_ad514x[NUM_OF_IIO_DEV][NUM_OF_DEV_ATTR
			+ 2] = {
	{
		DPOT_CHN_ATTR("rdac_wp", DPOT_RDAC_WP_ATTR_ID),
		DPOT_CHN_AVAIL_ATTR("rdac_wp_available", DPOT_RDAC_WP_AVL_ATTR_ID),
		DPOT_CHN_ATTR("nvm_programming", DPOT_NVM_PROGRAMMING_ATTR_ID),
		DPOT_CHN_AVAIL_ATTR("nvm_programming_available", DPOT_NVM_PROGRAMMING_AVL_ATTR_ID),
		END_ATTRIBUTES_ARRAY
	},
#ifdef DPOT_ADD_BOARD_DEVICE
	{

		DPOT_CHN_ATTR("device_generic", DPOT_DEVICE_GENERIC_ATTR_ID),
		DPOT_CHN_AVAIL_ATTR("device_generic_available", DPOT_DEVICE_GENERIC_AVL_ATTR_ID),
		DPOT_CHN_ATTR("comm_interface", DPOT_INTERFACE_ATTR_ID),
		DPOT_CHN_AVAIL_ATTR("comm_interface_available", DPOT_INTERFACE_AVL_ATTR_ID),
		DPOT_CHN_ATTR("operating_mode", DPOT_OPERATING_MODE_ATTR_ID),
		DPOT_CHN_AVAIL_ATTR("operating_mode_available", DPOT_OPERATING_MODE_AVL_ATTR_ID),
		DPOT_CHN_ATTR("reconfigure_system", DPOT_RESTART_IIO_ATTR_ID),
		DPOT_CHN_AVAIL_ATTR("reconfigure_system_available", DPOT_RESTART_IIO_ATTR_ID),
		END_ATTRIBUTES_ARRAY
	}
#endif
};

static struct iio_channel dpot_iio_chans_PotMode[MAX_CHNS * 3] = {
	DPOT_CH("RDAC1", 0, DPOT_CHN_RDAC1, IIO_RESISTANCE, dpot_iio_chn_attr),
	DPOT_CH("RDAC2", 0, DPOT_CHN_RDAC2, IIO_RESISTANCE, dpot_iio_chn_attr),
	DPOT_CH("RDAC3", 0, DPOT_CHN_RDAC3, IIO_RESISTANCE, dpot_iio_chn_attr),
	DPOT_CH("RDAC4", 0, DPOT_CHN_RDAC4, IIO_RESISTANCE, dpot_iio_chn_attr),
};

static struct iio_channel dpot_iio_chans_linGMode[MAX_CHNS * 3] = {
	DPOT_CH("R_AW1", 0, DPOT_CHN_R_AW1, IIO_RESISTANCE, dpot_iio_chn_attr),
	DPOT_CH("R_WB1", 0, DPOT_CHN_R_WB1, IIO_RESISTANCE, dpot_iio_chn_attr),
	DPOT_CH("R_AW2", 0, DPOT_CHN_R_AW2, IIO_RESISTANCE, dpot_iio_chn_attr),
	DPOT_CH("R_WB2", 0, DPOT_CHN_R_WB2, IIO_RESISTANCE, dpot_iio_chn_attr),
	DPOT_CH("R_AW3", 0, DPOT_CHN_R_AW3, IIO_RESISTANCE, dpot_iio_chn_attr),
	DPOT_CH("R_WB3", 0, DPOT_CHN_R_WB3, IIO_RESISTANCE, dpot_iio_chn_attr),
	DPOT_CH("R_AW4", 0, DPOT_CHN_R_AW4, IIO_RESISTANCE, dpot_iio_chn_attr),
	DPOT_CH("R_WB4", 0, DPOT_CHN_R_WB4, IIO_RESISTANCE, dpot_iio_chn_attr),
};

static struct iio_channel dpot_iio_chans_AD5259[1] = {
	DPOT_CH("RDAC1", 0, DPOT_CHN_RDAC1, IIO_RESISTANCE, dpot_iio_chn_attr_5259)
};

static struct iio_channel dpot_iio_chans_AD5161[1] = {
	DPOT_CH("RDAC1", 0, DPOT_CHN_RDAC1, IIO_RESISTANCE, dpot_iio_chn_attr_5161)
};

static struct iio_channel dpot_iio_chans_AD5242[2] = {
	DPOT_CH("RDAC1", 0, DPOT_CHN_RDAC1, IIO_RESISTANCE, dpot_iio_chn_attr_5242),
	DPOT_CH("RDAC2", 0, DPOT_CHN_RDAC2, IIO_RESISTANCE, dpot_iio_chn_attr_5242)
};

static struct iio_channel dpot_iio_chans_AD5246[1] = {
	DPOT_CH("RDAC1", 0, DPOT_CHN_RDAC1, IIO_RESISTANCE, dpot_iio_chn_attr_5246)
};

/* Dpot scale values per channel.
 * Scale is used to convert input resistance to RDAC data
 * and vice a versa */
static float dpot_scale[NUM_OF_DPOT_CHN] = {[0 ... NUM_OF_DPOT_CHN - 1] = 1};

static uint8_t dpot_TS_status[NUM_OF_DPOT_CHN] = { 0, 0, 0, 0 };
static uint8_t dpot_BS_status[NUM_OF_DPOT_CHN] = { 0, 0, 0, 0};
/* Dpot channel shutdown status value */
static bool dpot_chn_shutdown[NUM_OF_DPOT_CHN] = {
	DPOT_RDAC1_DEFAULT_SHUTDOWN,
	DPOT_RDAC2_DEFAULT_SHUTDOWN,
	DPOT_RDAC3_DEFAULT_SHUTDOWN,
	DPOT_RDAC4_DEFAULT_SHUTDOWN,
	DPOT_RAW1_DEFAULT_SHUTDOWN,
	DPOT_RWB1_DEFAULT_SHUTDOWN,
	DPOT_RAW2_DEFAULT_SHUTDOWN,
	DPOT_RWB2_DEFAULT_SHUTDOWN,
	DPOT_RAW3_DEFAULT_SHUTDOWN,
	DPOT_RWB3_DEFAULT_SHUTDOWN,
	DPOT_RAW4_DEFAULT_SHUTDOWN,
	DPOT_RWB4_DEFAULT_SHUTDOWN,
};

/* Dpot operating mode value index */
static enum dpot_operating_mode dpot_operating_mode_indx =
	DPOT_DEFAULT_OPERATING_MODE;

/* Dpot RDAC linear value index */
static enum dpot_rdac_linear_status dpot_rdac_linear_indx;

/* Dpot RDAC 6dB value index */
static enum dpot_rdac_6db_status dpot_rdac_6db_indx;

/* Dpot software lrdac status */
static bool dpot_sw_lrdac_enable;

/* Dpot rdac to eeprom copy status */
static bool dpot_copy_rdac_to_eeprom_enable;

/* Dpot eeprom to rdac copy status */
static bool dpot_copy_eeprom_to_rdac_enable;

/* Dpot RDAC WP status index */
static uint8_t dpot_rdac_wp_indx;

/* Dpot NVM programming status index */
static uint8_t dpot_nvm_programming_indx;

/* Operating mode available options */
static const char *dpot_operating_mode[] = {
	"potentiometer",
	"linear_gain_setting"
};

/* Linear RDAC increment/decrement status available options */
static const char *dpot_rdac_linear_status[] = {
	"increment",
	"decrement"
};

/* 6dB RDAC increment/decrement status available options */
static const char *dpot_rdac_6db_status[] = {
	"increment",
	"decrement"
};

/* Channel shutdown status available options */
static const char *dpot_chn_shutdown_status[] = {
	"disable",
	"enable"
};

/* SW LRDAC status available options */
static const char *dpot_sw_lrdac_status[] = {
	"disable",
	"enable"
};

/* RDAC to EEPROM and vice a versa copy status available options */
static const char *dpot_rdac_eeprom_copy_status[] = {
	"disable",
	"enable"
};

/* RDAC write protect status available options */
static const char *dpot_rdac_wp_status[] = {
	"disable",
	"enable"
};

/* NVM programming status available options */
static const char *dpot_nvm_programming_status[] = {
	"enable",
	"disable"
};

static const char *dpot_mid_scale_options[] = {
	"disable",
	"enable"
};

static const char *dpot_scale_option[] = {
	"enter",
	"exit"
};

static const char *dpot_restart_iio_options[] = {
	"enable",
};
static const char *dpot_interface_options[] = {
	"SPI",
	"I2C"
};
static uint8_t dpot_interface_indx = 0;

/* Restart IIO flag */
bool restart_iio_flag = false;

/* EVB HW validation status */
static bool hw_mezzanine_is_valid;

/******************************************************************************/
/************************ Functions Definitions *******************************/
/******************************************************************************/
/**
 * @brief	Initialize default values for the active device and interface type if unset
 */
int init_default(void)
{
	if (oactive_dev.active_device == 0XFF) {
		oactive_dev.active_device =  DEV_AD5144;
	}
	if (oactive_dev.intf_type == 0xFF) {
		oactive_dev.intf_type =
			dpot_info[oactive_dev.active_device].dpot_init_params.intf_type;
	}
	return (0);
}

/**
 * @brief	Place holder for calculation the Scale if required.
 */
static void dpot_calculate_scale(void)
{
	if (dpot_operating_mode_indx == DPOT_POTENTIOMETER_MODE) {

	} else {

	}
}

/*!
 * @brief	Getter function for IIO attributes
 * @param	device[in]- Pointer to IIO device instance
 * @param	buf[in]- IIO input data buffer
 * @param	len[in]- Number of input bytes
 * @param	channel[in] - Input channel structure
 * @param	priv[in] - Attribute private ID
 * @return	len in case of success, negative error code otherwise
 */
static int dpot_iio_attr_get(void *device, char *buf, uint32_t len,
			     const struct iio_ch_info *channel, intptr_t priv)
{
	int ret;
	uint8_t data;
	uint8_t nTolerance[2] = { 0x00, 0x00 };

	switch (priv) {
	case DPOT_RAW_ATTR_ID:
		ret = dpot_chn_read(dpot_dev_desc, channel->ch_num, &data);
		if (ret) {
			return ret;
		}

		len = sprintf(buf, "%d", data);
		break;

	case DPOT_SCALE_ATTR_ID:
		len = sprintf(buf, "%0.10f", dpot_scale[channel->ch_num]);
		break;

	case DPOT_INPUT_REG_VAL_ATTR_ID:
		ret = dpot_input_reg_read(dpot_dev_desc, channel->ch_num, &data);
		if (ret) {
			return ret;
		}

		len = sprintf(buf, "%d", data);
		break;

	case DPOT_EEPROM_VAL_ATTR_ID:
		ret = dpot_nvm_read(dpot_dev_desc, channel->ch_num, &data);
		if (ret) {
			return ret;
		}

		len = sprintf(buf, "%d", data);
		break;

	case DPOT_SHUTDOWN_VAL_ATTR_ID:
		if (!dpot_chn_shutdown[channel->ch_num]) {
			len = sprintf(buf, "%s", dpot_chn_shutdown_status[0]);
		} else {
			len = sprintf(buf, "%s", dpot_chn_shutdown_status[1]);
		}
		break;
	case DPOT_SET_MID_SCALE_ATTR_ID:
		len = sprintf(buf, "%s", dpot_mid_scale_options[0]);
		break;
	case DPOT_RDAC_LINEAR_ATTR_ID:
		len = sprintf(buf, "%s", dpot_rdac_linear_status[dpot_rdac_linear_indx]);
		break;

	case DPOT_RDAC_6DB_ATTR_ID:
		len = sprintf(buf, "%s", dpot_rdac_6db_status[dpot_rdac_6db_indx]);
		break;

	case DPOT_SW_LRDAC_ATTR_ID:
		if (!dpot_sw_lrdac_enable) {
			len = sprintf(buf, "%s", dpot_sw_lrdac_status[0]);
		} else {
			len = sprintf(buf, "%s", dpot_sw_lrdac_status[1]);
		}
		break;

	case DPOT_COPY_RDAC_TO_EEPROM_ATTR_ID:
		if (!dpot_copy_rdac_to_eeprom_enable) {
			len = sprintf(buf, "%s", dpot_rdac_eeprom_copy_status[0]);
		} else {
			len = 	sprintf(buf, "%s", dpot_rdac_eeprom_copy_status[1]);
		}
		break;

	case DPOT_COPY_EEPROM_TO_RDAC_ATTR_ID:
		if (!dpot_copy_eeprom_to_rdac_enable) {
			len = sprintf(buf, "%s", dpot_rdac_eeprom_copy_status[0]);
		} else {
			len = 	sprintf(buf, "%s", dpot_rdac_eeprom_copy_status[1]);
		}
		break;

	case DPOT_OPERATING_MODE_ATTR_ID:
		len = sprintf(buf, "%s", dpot_operating_mode[oactive_dev.mode]);
		break;

	case DPOT_RDAC_WP_ATTR_ID:
		len = sprintf(buf, "%s", dpot_rdac_wp_status[dpot_rdac_wp_indx]);
		break;

	case DPOT_NVM_PROGRAMMING_ATTR_ID:
		len = sprintf(buf, "%s",
			      dpot_nvm_programming_status[dpot_nvm_programming_indx]);
		break;

	case DPOT_TOP_SCALE_ID:
		if (dpot_TS_status[channel->ch_num] == 1) {
			len = sprintf(buf, "%s", dpot_scale_option[0]);
		} else {
			len = sprintf(buf, "%s", dpot_scale_option[1]);
		}
		break;
	case DPOT_BOTTOM_SCALE_ID:
		if (dpot_BS_status[channel->ch_num] == 1) {
			len = sprintf(buf, "%s", dpot_scale_option[0]);
		} else {
			len = sprintf(buf, "%s", dpot_scale_option[1]);
		}

		break;
	case DPOT_DEVICE_GENERIC_ATTR_ID:
		if (oactive_dev.active_device != 0XFF) {
			len = sprintf(buf, "%s", dpot_info[oactive_dev.active_device].device_name);
		} else {
			len = sprintf(buf, "%s", dpot_info[DEV_AD5144].device_name);

		}
		break;

	case DPOT_RESTART_IIO_ATTR_ID:
		len = sprintf(buf, "%s", dpot_restart_iio_options[0]);
		break;
	case DPOT_INTERFACE_ATTR_ID:
		len = sprintf(buf, "%s", dpot_interface_options[dpot_interface_indx]);
		break;

	case DPOT_TOLERANCE_ATTR_ID:
		ret = dpot_tolerance_read(dpot_dev_desc, channel->ch_num, nTolerance);
		if (ret != 0) {
			len = 0;
			break;
		}
		len = sprintf(buf, "%f", (float) nTolerance[0] + (float)nTolerance[1] / pow(2,
				8));
		break;
	default:
		return -EINVAL;
	}

	return len;
}

/*!
 * @brief	Setter function for IIO attributes
 * @param	device[in]- Pointer to IIO device instance
 * @param	buf[in]- IIO input data buffer
 * @param	len[in]- Number of input bytes
 * @param	channel[in] - Input channel structure
 * @param	priv[in] - Attribute private ID
 * @return	len in case of success, negative error code otherwise
 */
static int dpot_iio_attr_set(void *device, char *buf, uint32_t len,
			     const struct iio_ch_info *channel, intptr_t priv)
{
	int ret;
	uint8_t val;
	uint8_t nScVal = 0;

	switch (priv) {
	case DPOT_TOP_SCALE_ID:
		if (!strcmp(buf, "enter")) {
			nScVal = 1;
		} else if (!strcmp(buf, "exit")) {
			nScVal = 0;
		} else {
			return -EINVAL;
		}

		ret = dpot_enable_top_scale(dpot_dev_desc,
					    channel->ch_num,
					    nScVal);
		if (ret) {
			return ret;
		}
		dpot_TS_status[channel->ch_num] = nScVal;
		break;
	case DPOT_BOTTOM_SCALE_ID:
		if (!strcmp(buf, "enter")) {
			nScVal = 1;
		} else if (!strcmp(buf, "exit")) {
			nScVal = 0;
		} else {
			return -EINVAL;
		}

		ret = dpot_enable_bottom_scale(dpot_dev_desc,
					       channel->ch_num,
					       nScVal);
		if (ret) {
			return ret;
		}
		dpot_BS_status[channel->ch_num] = nScVal;
		break;
	case DPOT_RAW_ATTR_ID:
		val = no_os_str_to_uint32(buf);
		ret = dpot_chn_write(dpot_dev_desc, channel->ch_num, (uint8_t)val);
		if (ret) {
			return ret;
		}

		if (dpot_copy_rdac_to_eeprom_enable) {
			ret = dpot_copy_rdac_to_nvm(dpot_dev_desc, channel->ch_num);
			if (ret) {
				return ret;
			}
		}
		break;
	case DPOT_TOLERANCE_ATTR_ID:
		break;
	case DPOT_SCALE_ATTR_ID:
		/* Read only */
		break;

	case DPOT_SET_MID_SCALE_ATTR_ID :
		if (!strcmp(buf, "enable")) {
			val = 1;
		} else if (!strcmp(buf, "disable")) {
			val = 0;
		} else {
			return -EINVAL;
		}

		if ((oactive_dev.active_device == DEV_AD5161)
		    && (oactive_dev.intf_type == AD_SPI_INTERFACE)) {
			return -EINVAL;
		}

		ret =  dpot_set_mid_scale(dpot_dev_desc, channel->ch_num, val);
		if (ret) {
			return ret;
		}
		break;

	case DPOT_INPUT_REG_VAL_ATTR_ID:
		val = no_os_str_to_uint32(buf);
		ret = dpot_input_reg_write(dpot_dev_desc, channel->ch_num, (uint8_t)val);
		if (ret) {
			return ret;
		}

		break;

	case DPOT_EEPROM_VAL_ATTR_ID:
		val = no_os_str_to_uint32(buf);
		ret = dpot_nvm_write(dpot_dev_desc, channel->ch_num, (uint8_t)val);
		if (ret) {
			return ret;
		}

		if (dpot_copy_eeprom_to_rdac_enable) {
			ret = dpot_copy_nvm_to_rdac(dpot_dev_desc, channel->ch_num);
			if (ret) {
				return ret;
			}
		}
		break;

	case DPOT_SHUTDOWN_VAL_ATTR_ID:
		if (!strcmp(buf, "enable")) {
			val = 1;
		} else if (!strcmp(buf, "disable")) {
			val = 0;
		} else {
			return -EINVAL;
		}

		if ((oactive_dev.active_device == DEV_AD5161)
		    && (oactive_dev.intf_type == AD_SPI_INTERFACE)) {
			return -EINVAL;
		}

		ret = dpot_shutdown(dpot_dev_desc, channel->ch_num, (bool)val);
		if (ret) {
			return ret;
		}

		dpot_chn_shutdown[channel->ch_num] = (bool)val;
		break;

	case DPOT_RDAC_LINEAR_ATTR_ID:
		if (!strcmp(buf, "increment")) {
			dpot_rdac_linear_indx = DPOT_RDAC_LINEAR_INCREMENT;
		} else if (!strcmp(buf, "decrement")) {
			dpot_rdac_linear_indx = DPOT_RDAC_LINEAR_DECREMENT;
		} else {
			return -EINVAL;
		}

		ret = dpot_rdac_linear_update(dpot_dev_desc, channel->ch_num,
					      dpot_rdac_linear_indx);
		if (ret) {
			return ret;
		}
		break;

	case DPOT_RDAC_6DB_ATTR_ID:
		if (!strcmp(buf, "increment")) {
			dpot_rdac_6db_indx = DPOT_RDAC_6DB_INCREMENT;
		} else if (!strcmp(buf, "decrement")) {
			dpot_rdac_6db_indx = DPOT_RDAC_6DB_DECREMENT;
		} else {
			return -EINVAL;
		}

		ret = dpot_rdac_6db_update(dpot_dev_desc, channel->ch_num, dpot_rdac_6db_indx);
		if (ret) {
			return ret;
		}
		break;

	case DPOT_OPERATING_MODE_ATTR_ID:
		if (!strcmp(buf, "potentiometer")) {
			dpot_operating_mode_indx = DPOT_POTENTIOMETER_MODE;
			oactive_dev.mode  = DPOT_POTENTIOMETER_MODE;
		} else if (!strcmp(buf, "linear_gain_setting")) {
			dpot_operating_mode_indx = DPOT_LINEAR_GAIN_SETTING_MODE;
			oactive_dev.mode  = DPOT_LINEAR_GAIN_SETTING_MODE;
		} else {
			return -EINVAL;
		}

		ret = dpot_set_operating_mode(dpot_dev_desc, dpot_operating_mode_indx);
		if (ret) {
			return ret;
		}

		/* Recalculate the scale value based on operating mode */
		dpot_calculate_scale();

		break;

	case DPOT_SW_LRDAC_ATTR_ID:
		if (!strcmp(buf, "enable")) {
			ret = dpot_sw_lrdac_update(dpot_dev_desc, channel->ch_num);
			if (ret) {
				return ret;
			}
		} else {
			return -EINVAL;
		}
		break;

	case DPOT_COPY_RDAC_TO_EEPROM_ATTR_ID:
		if (!strcmp(buf, "enable")) {
			ret = dpot_copy_rdac_to_nvm(dpot_dev_desc, channel->ch_num);
			if (ret) {
				return ret;
			}
		} else {
			return -EINVAL;
		}
		break;

	case DPOT_COPY_EEPROM_TO_RDAC_ATTR_ID:
		if (!strcmp(buf, "enable")) {
			ret = dpot_copy_nvm_to_rdac(dpot_dev_desc, channel->ch_num);
			if (ret) {
				return ret;
			}
		} else {
			return -EINVAL;
		}
		break;

	case DPOT_RDAC_WP_ATTR_ID:
		if (!strcmp(buf, "disable")) {
			dpot_rdac_wp_indx = 0;
		} else if (!strcmp(buf, "enable")) {
			dpot_rdac_wp_indx = 1;
		} else {
			return -EINVAL;
		}

		ret = dpot_set_rdac_wp(dpot_rdac_wp_indx);
		if (ret) {
			return ret;
		}
		break;

	case DPOT_NVM_PROGRAMMING_ATTR_ID:
		if (!strcmp(buf, "enable")) {
			dpot_nvm_programming_indx = 0;
		} else if (!strcmp(buf, "disable")) {
			dpot_nvm_programming_indx = 1;
		} else {
			return -EINVAL;
		}

		ret = dpot_set_nvm_programming(dpot_nvm_programming_indx);
		if (ret) {
			return ret;
		}
		break;
	case DPOT_DEVICE_GENERIC_ATTR_ID:
		for (int i = 0; i < DPOT_NUM_SUPPORTED_DEVICES; i++) {
			if (!strcmp(buf, dpot_info[i].device_name)) {
				oactive_dev.active_device = i;
				break;
			}
		}
		break;

	case DPOT_RESTART_IIO_ATTR_ID:
		/* Set flag to true */
		len = 0;
		init_default();
		restart_iio_flag = true;
		break;

	case DPOT_INTERFACE_ATTR_ID:
		/* Set flag to true */
		if (!strcmp(buf, "I2C")) {
			oactive_dev.intf_type = AD_I2C_INTERFACE;
			dpot_interface_indx = 1;
		} else if (!strcmp(buf, "SPI")) {
			dpot_interface_indx = 0;
			oactive_dev.intf_type = AD_SPI_INTERFACE;
		} else {
			return -EINVAL;
		}
		break;

	default:
		return -EINVAL;
	}

	return len;
}

/*!
 * @brief	Getter function for IIO available attributes
 * @param	device[in]- Pointer to IIO device instance
 * @param	buf[in]- IIO input data buffer
 * @param	len[in]- Number of input bytes
 * @param	channel[in] - Input channel structure
 * @param	priv[in] - Attribute private ID
 * @return	len in case of success, negative error code otherwise
 */
static int dpot_iio_attr_available_get(void *device, char *buf, uint32_t len,
				       const struct iio_ch_info *channel, intptr_t priv)
{
	uint32_t i;
	switch (priv) {
	case DPOT_SHUTDOWN_VAL_AVL_ATTR_ID:
		len = sprintf(buf, "%s %s", dpot_chn_shutdown_status[0],
			      dpot_chn_shutdown_status[1]);
		break;

	case DPOT_RDAC_LINEAR_AVL_ATTR_ID:
		len = sprintf(buf, "%s %s", dpot_rdac_linear_status[0],
			      dpot_rdac_linear_status[1]);
		break;

	case DPOT_RDAC_6DB_AVL_ATTR_ID:
		len = sprintf(buf, "%s %s", dpot_rdac_6db_status[0], dpot_rdac_6db_status[1]);
		break;

	case DPOT_SW_LRDAC_AVL_ATTR_ID:
		len = sprintf(buf, "%s %s", dpot_sw_lrdac_status[0], dpot_sw_lrdac_status[1]);
		break;

	case DPOT_COPY_RDAC_TO_EEPROM_AVL_ATTR_ID:
	case DPOT_COPY_EEPROM_TO_RDAC_AVL_ATTR_ID:
		len = sprintf(buf,
			      "%s %s",
			      dpot_rdac_eeprom_copy_status[0],
			      dpot_rdac_eeprom_copy_status[1]);
		break;

	case DPOT_OPERATING_MODE_AVL_ATTR_ID:
		if (oactive_dev.active_device != 0xFF) {
			len = sprintf(buf, "%s", dpot_operating_mode[0]);
			if ((oactive_dev.active_device == DEV_AD5141)
			    || (oactive_dev.active_device == DEV_AD5142)
			    || (oactive_dev.active_device == DEV_AD5142A)
			    || (oactive_dev.active_device == DEV_AD5143)
			    || (oactive_dev.active_device == DEV_AD5144)) {
				len += sprintf(buf + len, " %s", dpot_operating_mode[1]);
			}
		} else {
			len = sprintf(buf, "%s", dpot_operating_mode[0]);
		}
		break;

	case DPOT_RDAC_WP_AVL_ATTR_ID:
		len = sprintf(buf, "%s %s", dpot_rdac_wp_status[0], dpot_rdac_wp_status[1]);
		break;
	case DPOT_BOTTOM_SCALE_ID_AVL_ATTR_ID:
		len = sprintf(buf,
			      "%s %s",
			      dpot_scale_option[0],
			      dpot_scale_option[1]);
		break;
	case DPOT_TOP_SCALE_ID_AVL_ATTR_ID:
		len = sprintf(buf,
			      "%s %s",
			      dpot_scale_option[0],
			      dpot_scale_option[1]);
		break;

	case DPOT_NVM_PROGRAMMING_AVL_ATTR_ID:
		len = sprintf(buf,
			      "%s %s",
			      dpot_nvm_programming_status[0],
			      dpot_nvm_programming_status[1]);
		break;
	case DPOT_DEVICE_GENERIC_AVL_ATTR_ID:
		len = 0;

		for (i = 0 ; i < sizeof(supported_generics) / sizeof(enum dpot_dev_id) ; i++) {
			len += sprintf(buf + len, "%s ", dpot_info[supported_generics[i]].device_name);
		}
		len = len - 1;
		break;

	case DPOT_RESTART_IIO_ATTR_ID:
		len = sprintf(buf, "%s", dpot_restart_iio_options[0]);
		break;

	case DPOT_INTERFACE_AVL_ATTR_ID:
		len = 0;
		if (oactive_dev.active_device != 0xFF) {
			if (dpot_info[oactive_dev.active_device].nSupportedInterface & AD_SPI_INTERFACE)
				len = sprintf(buf,
					      "%s ",
					      dpot_interface_options[0]);
			if (dpot_info[oactive_dev.active_device].nSupportedInterface & AD_I2C_INTERFACE)
				len += sprintf(buf + len,
					       "%s",
					       dpot_interface_options[1]);
			if (oactive_dev.intf_type == 0xFF)
				oactive_dev.intf_type =
					dpot_info[oactive_dev.active_device].dpot_init_params.intf_type;
		} else {
			len = sprintf(buf, "%s ", dpot_interface_options[0]);
		}

		break;

	case DPOT_SET_MID_SCALE_AVL_ATTR_ID:
		len = sprintf(buf,
			      "%s %s",
			      dpot_mid_scale_options[0],
			      dpot_mid_scale_options[1]);
		break;

	default:
		return -EINVAL;
	}

	return len;
}

/*!
 * @brief	Setter function for IIO available attributes
 * @param	device[in]- Pointer to IIO device instance
 * @param	buf[in]- IIO input data buffer
 * @param	len[in]- Number of input bytes
 * @param	channel[in] - Input channel structure
 * @param	priv[in] - Attribute private ID
 * @return	len in case of success, negative error code otherwise
 */
static int dpot_iio_attr_available_set(void *device, char *buf, uint32_t len,
				       const struct iio_ch_info *channel, intptr_t priv)
{
	return len;
}

/**
 * @brief	Init for reading/writing and parameterization of a
 * 			digipot IIO device
 * @param 	desc[in,out] - IIO device descriptor
 * @param	dev_indx[in] - IIO Device index
 * @return	0 in case of success, negative error code otherwise
 */
static int dpot_iio_params_init(struct iio_device **desc,
				uint8_t dev_indx)
{
	struct iio_device *iio_dev;

	if (!desc) {
		return -EINVAL;
	}
	iio_dev = calloc(1, sizeof(*iio_dev));
	if (!iio_dev) {
		return -ENOMEM;
	}

	iio_dev->attributes = dpot_iio_dev_attr[dev_indx];
	iio_dev->num_ch =    dpot_info[oactive_dev.active_device].num_of_channels;

	switch (oactive_dev.active_device) {
	case DEV_AD5141:
	case DEV_AD5142:
	case DEV_AD5142A:
	case DEV_AD5143:
	case DEV_AD5144:
	case DEV_AD5121:
	case DEV_AD5122:
	case DEV_AD5123:
	case DEV_AD5124:
	case DEV_AD5122A:
		if (oactive_dev.mode == DPOT_POTENTIOMETER_MODE) {
			iio_dev->channels =  dpot_iio_chans_PotMode;
		} else if (oactive_dev.mode == DPOT_LINEAR_GAIN_SETTING_MODE) {
			iio_dev->channels =  dpot_iio_chans_linGMode;
			iio_dev->num_ch =    dpot_info[oactive_dev.active_device].num_of_channels * 2;
		}
		iio_dev->attributes = dpot_iio_dev_attr_ad514x[dev_indx];
		break;
	case DEV_AD5259:
	case DEV_AD5258:
		iio_dev->channels =  dpot_iio_chans_AD5259;
		break;
	case DEV_AD5161:
	case DEV_AD5245:
		iio_dev->channels =  dpot_iio_chans_AD5161;
		break;
	case DEV_AD5241:
	case DEV_AD5242:
		iio_dev->channels =  dpot_iio_chans_AD5242;
		break;
	case DEV_AD5246:
	case DEV_AD5171:
	case DEV_AD5273:
	case DEV_AD5160:
	case DEV_AD5165:
		iio_dev->channels =  dpot_iio_chans_AD5246;
		break;
	default:
		return -ENOMEM;
	}

	iio_dev->pre_enable = NULL;
	iio_dev->post_disable = NULL;
	iio_dev->submit = NULL;
	iio_dev->debug_reg_read = NULL;
	iio_dev->debug_reg_write = NULL;

	/* Calculate the scale value */
	dpot_calculate_scale();

	*desc = iio_dev;

	return 0;
}

#ifdef DPOT_ADD_BOARD_DEVICE
/**
 * @brief	Init for reading/writing and parameterization of a
 * 			digipot Board IIO device
 * @param 	desc[in,out] - IIO device descriptor
 * @param	dev_indx[in] - IIO Device index
 * @return	0 in case of success, negative error code otherwise
 */
static int board_iio_params_init(struct iio_device **desc,
				 uint8_t dev_indx)
{
	struct iio_device *iio_dev;

	if (!desc) {
		return -EINVAL;
	}

	iio_dev = calloc(1, sizeof(*iio_dev));
	if (!iio_dev) {
		return -ENOMEM;
	}

	iio_dev->num_ch =  0;

	iio_dev->channels = NULL;
	iio_dev->attributes = dpot_iio_dev_attr[dev_indx];

	iio_dev->pre_enable = NULL;
	iio_dev->post_disable = NULL;
	iio_dev->submit = NULL;
	iio_dev->debug_reg_read = NULL;
	iio_dev->debug_reg_write = NULL;

	/* Calculate the scale value */
	dpot_calculate_scale();

	*desc = iio_dev;

	return 0;
}
#endif

/**
 * @brief	DeInitialize the IIO params.
 * @return	0 in case of success, negative error code otherwise
 */
int iio_params_deinit(void)
{
	uint8_t i = 0;
	while (i < dpot_iio_init_params.nb_devs) {
		if (dpot_iio_dev[i] != NULL) {
			free(dpot_iio_dev[i]);
			dpot_iio_dev[i] = NULL;
		}
		i++;
	}
	dpot_iio_init_params.nb_devs = 0;
	return (0);
}

/**
 * @brief	Initialize the IIO interface for digipot IIO device
 * @return	0 in case of success, negative error code otherwise
 */
int dpot_iio_init(void)
{
	static bool entered = false;
	int ret;

	/* Init the application specific system peripherals */
	if (!entered) {
		ret = init_system();
		if (ret) {
			return ret;
		}

		/* Add delay between the i2c init and the eeprom read */
		no_os_mdelay(2000);
	}

	/* Read context attributes */
	ret = get_iio_context_attributes_ex(&dpot_iio_init_params.ctx_attrs,
					    &dpot_iio_init_params.nb_ctx_attr,
					    eeprom_desc,
					    HW_MEZZANINE_NAME,
					    STR(HW_CARRIER_NAME),
					    &hw_mezzanine_is_valid,
					    FIRMWARE_VERSION);
	if (ret) {
		return ret;
	}

#ifndef DPOT_ADD_BOARD_DEVICE
	/* Set default active device and interface type */
	init_default();
#endif

	if (hw_mezzanine_is_valid) {
		dpot_iio_init_params.nb_devs = 0;
		if (oactive_dev.active_device != 0XFF) {

			/* Change the I2C slave address for communicating with digipot device.
			* I2C bus is shared between EEPROM and digipot device but both
			* uses different slave address */
			i2c_init_params.slave_address =
				dpot_info[oactive_dev.active_device].device_i2c_addr;
			dpot_info[oactive_dev.active_device].dpot_init_params.intf_type =
				oactive_dev.intf_type;
			dpot_info[oactive_dev.active_device].dpot_init_params.operating_mode  =
				oactive_dev.mode;
			/* Initialize digipot (no-os) device drivers */
			ret = dpot_init(&dpot_dev_desc,
					&dpot_info[oactive_dev.active_device].dpot_init_params);

			if (ret) {
				return ret;
			}

			/* Initialize digipot IIO parameters */
			ret = dpot_iio_params_init(&dpot_iio_dev[dpot_iio_init_params.nb_devs], 0);
			if (ret) {
				return ret;
			}
			dpot_iio_dev_init_params[dpot_iio_init_params.nb_devs].name =
				dpot_info[oactive_dev.active_device].device_name;
			dpot_iio_dev_init_params[dpot_iio_init_params.nb_devs].dev = dpot_dev_desc;
			dpot_iio_dev_init_params[dpot_iio_init_params.nb_devs].dev_descriptor =
				dpot_iio_dev[dpot_iio_init_params.nb_devs];
			dpot_iio_init_params.nb_devs++;
		}
		/* Add Board IIO device */
#ifdef DPOT_ADD_BOARD_DEVICE
		/* Initialize board IIO paramaters */
		ret = board_iio_params_init(&dpot_iio_dev[dpot_iio_init_params.nb_devs],
					    1);
		if (ret) {
			return ret;
		}
		dpot_iio_dev_init_params[dpot_iio_init_params.nb_devs].name = "system_config";
		dpot_iio_dev_init_params[dpot_iio_init_params.nb_devs].dev = board_dev;
		dpot_iio_dev_init_params[dpot_iio_init_params.nb_devs].dev_descriptor =
			dpot_iio_dev[dpot_iio_init_params.nb_devs];
		dpot_iio_init_params.nb_devs++;
#endif

	}

	/* Initialize the IIO interface */
	dpot_iio_init_params.uart_desc = uart_iio_com_desc;
	dpot_iio_init_params.devs = dpot_iio_dev_init_params;

	ret = iio_init(&dpot_iio_desc, &dpot_iio_init_params);
	if (ret) {
		return ret;
	}

	entered = true;

	return 0;
}

/**
 * @brief 	Run the digipot IIO event handler
 * @return	none
 * @details	This function monitors the new IIO client event
 */
void dpot_iio_event_handler(void)
{
#ifdef DPOT_ADD_BOARD_DEVICE
	if (restart_iio_flag) {
		/* Free resources from the dpot_dev */
		dpot_remove(dpot_dev_desc);

		/* Free the board descriptor */
		free(board_dev);

		iio_params_deinit();

		board_dev = NULL;

		/* Free IIO desc resources */
		iio_remove(dpot_iio_desc);

		/* Reset the restart_iio flag */
		restart_iio_flag = false;

		/* Invoke init again */
		dpot_iio_init();
	}
#endif
	(void)iio_step(dpot_iio_desc);
}
