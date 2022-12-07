/***************************************************************************//**
 *   @file    ltc268x_console_app.c
 *   @brief   ltc268x console application interfaces
 *   @details This file is specific to ltc268x console menu application handle.
 *            The functions defined in this file performs the action
 *            based on user selected console menu.
 *
********************************************************************************
 * Copyright (c) 2022 Analog Devices, Inc.
 *
 * All rights reserved.
 *
 * This software is proprietary to Analog Devices, Inc. and its licensor's.
 * By using this software you agree to the terms of the associated
 * Analog Devices Software License Agreement.
*******************************************************************************/

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "app_config.h"
#include "ltc268x_console_app.h"
#include "ltc268x_user_config.h"
#include "ltc268x.h"
#include "no_os_error.h"

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

/******************************************************************************/
/*************************** Types Declarations *******************************/
/******************************************************************************/
/* Pointer to the structure representing the ltc268x device */
static struct ltc268x_dev *p_ltc268x_dev = NULL;

/* Variable to store the active DAC channel */
static uint8_t ltc268x_active_channel;

/* Console menu declaration */
console_menu ltc268x_channel_select_menu;
console_menu ltc268x_power_select_menu;
console_menu ltc268x_dither_toggle_set_menu;
console_menu ltc268x_span_select_menu;
console_menu ltc268x_dither_phase_select_menu;
console_menu ltc268x_dither_period_select_menu;
console_menu ltc268x_dither_clock_select_menu;
console_menu ltc268x_dac_configurations_menu;
console_menu ltc268x_dac_data_operations_menu;

/******************************************************************************/
/***************************** Function Declarations **************************/
/******************************************************************************/
static void dac_settings_header(void);
static void error_status_footer(void);
static void print_title(void);

static int32_t ltc268x_set_active_channel(uint32_t id);
static int32_t ltc268x_select_input_register(uint32_t id);
static int32_t ltc268x_set_power_mode(uint32_t id);
static int32_t ltc268x_set_dither_toggle_mode(uint32_t id);
static int32_t ltc268x_span_select(uint32_t id);
static int32_t ltc268x_dither_phase_select(uint32_t id);
static int32_t ltc268x_dither_period_select(uint32_t id);
static int32_t ltc268x_tg_dith_clock_select(uint32_t id);
static int32_t ltc268x_write_dac_voltage(uint32_t id);

/******************************************************************************/
/***************************** Function Definitions ***************************/
/******************************************************************************/
/**
 * @brief 	Initialize the ltc268x  device and user configurations
 * @return	ltc268x  device initialization status
 */
int32_t ltc268x_app_initialize(void)
{
	int32_t init_status;

	/* Initialize ltc268x  device and peripheral interface */
	init_status = ltc268x_init(&p_ltc268x_dev, ltc268x_dev_init);
	if (init_status) {
		printf(EOL "LTC268X device initialization error");
	}

	return init_status;
}

/**
 * @brief 	Prints the header containing device configurations
 * @return	none
 */
static void dac_settings_header(void)
{
	uint8_t i;

	adi_clear_console();
	printf("*************************************************************************"
	       EOL);
	if (p_ltc268x_dev->dev_id == LTC2686) {
		printf("*                       LTC2686 Current DAC Settings                    *"
		       EOL);
	} else {
		printf("*                       LTC2688 Current DAC Settings                    *"
		       EOL);
	}
	printf("-------------------------------------------------------------------------"
	       EOL);
	printf("Channel   Power   DAC Register   Toggle      Toggle     DAC      Toggle"
	       EOL);
	printf("Number    Down    Value          Enable      Select     Mode     Clock"
	       EOL);
	printf("-------------------------------------------------------------------------"
	       EOL);
	for (i = 0; i < p_ltc268x_dev->num_channels; i++) {
		if (i == ltc268x_active_channel) {
			printf(VT100_COLORED_TEXT, VT_FG_GREEN);
		}

		printf(EOL "%-9d %-10s", i,
		       (p_ltc268x_dev->pwd_dac_setting & NO_OS_BIT(i)) ? "Yes" : "No");
		printf("%-12d %-11s ", p_ltc268x_dev->dac_code[i],
		       (p_ltc268x_dev->dither_toggle_en & NO_OS_BIT(i)) ? "Yes" : "No");
		printf("%-9s %-9s ", p_ltc268x_dev->reg_select[i] == LTC268X_SELECT_A_REG ?
		       "REG A" : "REG B", p_ltc268x_dev->dither_mode[i] ? "Dither" : "Toggle");
		switch ((enum  ltc268x_clk_input)p_ltc268x_dev->clk_input[i]) {
		case LTC268X_SOFT_TGL:
			printf("Software");
			break;
		case LTC268X_TGP0:
			printf("TG0");
			break;
		case LTC268X_TGP1:
			printf("TG1");
			break;
		case LTC268X_TGP2:
			printf("TG2");
			break;
		default:
			break;
		}

		if (i == ltc268x_active_channel) {
			printf(VT100_COLORED_TEXT, VT_FG_DEFAULT);
		}

		printf(EOL);
	}
}

/**
 * @brief 	Prints the footer containing last error codes
 * @return	none
 */
static void error_status_footer(void)
{
	if (adi_console_menu_state.last_error_code) {
		printf(EOL
		       "**********************************************************************" EOL);
		printf("  Error Code from Last action : %ld",
		       adi_console_menu_state.last_error_code);
	}

	adi_clear_last_menu_error();
}

/**
 * @brief 	Prints the title block.
 * @return	none
 */
static void print_title(void)
{
	adi_clear_console();
	printf("*****************************************************************" EOL);
	printf("* DC2873A-B Demonstration Program                               *" EOL);
	printf("*                                                               *" EOL);
	printf("* This program demonstrates the features of LTC268X             *" EOL);
	printf("* a 16-Channel, 16-Bit Voltage Output SoftSpan DAC.             *" EOL);
	printf("*                                                               *" EOL);
	printf("*****************************************************************" EOL);
}

/*!
 * @brief	Saves the user selected channel to be
 *          configured later.
 * @param	id[in] - User input for mode selection
 * @return	Menu status constant
 */
static int32_t ltc268x_set_active_channel(uint32_t id)
{
	int32_t ret;

	ret = adi_handle_user_input_integer(
		      EOL "Enter the channel number: ",
		      0, 15, &ltc268x_active_channel, 5, 5, 5);

	if (ret) {
		return ret;
	}

	return MENU_CONTINUE;
}

/*!
 * @brief	Selects the input register for DAC channel
 *          configuration.
 * @param	id[in] - User input for mode selection
 * @return	Menu status constant
 */
static int32_t ltc268x_select_input_register(uint32_t id)
{
	int32_t ret;

	ret = ltc268x_select_reg(p_ltc268x_dev, ltc268x_active_channel,
				 (enum ltc268x_a_b_register) id);
	if (ret) {
		return ret;
	}

	return MENU_CONTINUE;
}

/*!
 * @brief	Sets the channel into desired power mode.
 * @param	id[in] - User input for mode selection
 * @return	Menu status constant
 */
static int32_t ltc268x_set_power_mode(uint32_t id)
{
	int32_t ret;
	uint16_t regval = p_ltc268x_dev->pwd_dac_setting;

	if (id) { // Power Up
		regval &= ~LTC268X_PWDN(ltc268x_active_channel);
	} else {  // Power Down
		regval &= ~LTC268X_PWDN(ltc268x_active_channel);
		regval |= LTC268X_PWDN(ltc268x_active_channel);
	}

	ret = ltc268x_set_pwr_dac(p_ltc268x_dev, regval);
	if (ret) {
		return ret;
	}

	return MENU_DONE;
}

/*!
 * @brief	Sets the selected channel into dither/toggle mode.
 * @param	id[in] - User input for mode selection
 * @return	Menu status constant
 */
static int32_t ltc268x_set_dither_toggle_mode(uint32_t id)
{
	int32_t ret;
	uint16_t regval = p_ltc268x_dev->dither_toggle_en;

	regval &= ~LTC268X_DITH_EN(ltc268x_active_channel);
	regval |=  LTC268X_DITH_EN(ltc268x_active_channel);

	/* Enable DAC mode first */
	ret = ltc268x_set_dither_toggle(p_ltc268x_dev, regval);
	if (ret) {
		return ret;
	}

	ret = ltc268x_set_dither_mode(p_ltc268x_dev, ltc268x_active_channel, id);
	if (ret) {
		return ret;
	}

	return MENU_DONE;
}

/*!
 * @brief	Sets voltage range for the selected channel.
 * @param	id[in] - User input for mode selection
 * @return	Menu status constant
 */
static int32_t ltc268x_span_select(uint32_t id)
{
	int32_t ret;

	ret = ltc268x_set_span(p_ltc268x_dev, ltc268x_active_channel,
			       (enum ltc268x_voltage_range)id);
	if (ret) {
		return ret;
	}

	return MENU_DONE;
}

/*!
 * @brief	Sets dither phase for the selected channel.
 * @param	id[in] - User input for mode selection
 * @return	Menu status constant
 */
static int32_t ltc268x_dither_phase_select(uint32_t id)
{
	int32_t ret;

	ret = ltc268x_set_dither_phase(p_ltc268x_dev, ltc268x_active_channel,
				       (enum ltc268x_dither_phase) id);
	if (ret) {
		return ret;
	}

	return MENU_DONE;
}

/*!
 * @brief	Sets dither period for the selected channel.
 * @param	id[in] - User input for mode selection
 * @return	Menu status constant
 */
static int32_t ltc268x_dither_period_select(uint32_t id)
{
	int32_t ret;

	ret = ltc268x_set_dither_period(p_ltc268x_dev, ltc268x_active_channel,
					(enum ltc268x_dither_period) id);
	if (ret) {
		return ret;
	}

	return MENU_DONE;
}

/*!
 * @brief	Selects the input register for DAC channel
 *          configuration.
 * @param	id[in] - User input for mode selection
 * @return	Menu status constant
 */
static int32_t ltc268x_write_dac_voltage(uint32_t id)
{
	int32_t ret;
	uint16_t channnel_no;
	float min_v, max_v, voltage;

	switch (p_ltc268x_dev->crt_range[ltc268x_active_channel]) {
	case LTC268X_VOLTAGE_RANGE_0V_5V:
		min_v = 0.0;
		max_v = 5.0;
		break;
	case LTC268X_VOLTAGE_RANGE_0V_10V:
		min_v = 0.0;
		max_v = 10.0;
		break;
	case LTC268X_VOLTAGE_RANGE_M5V_5V:
		min_v = -5.0;
		max_v = 5.0;
		break;
	case LTC268X_VOLTAGE_RANGE_M10V_10V:
		min_v = -10.0;
		max_v = 10.0;
		break;
	case LTC268X_VOLTAGE_RANGE_M15V_15V:
		min_v = -15.0;
		max_v = 15.0;
		break;
	default:
		min_v = 0.0;
		max_v = 5.0;
		break;
	}

	ret = adi_handle_user_input_float("Enter the output voltage", min_v, max_v,
					  &voltage, 5, 1, 5);
	if (ret) {
		return ret;
	}

	if (id == 1) {
		ret = adi_handle_user_input_integer(
			      EOL "Enter the channel number: ",
			      0,
			      p_ltc268x_dev->num_channels-1,
			      &channnel_no,
			      2,
			      5,
			      5);

		ret = ltc268x_set_voltage(p_ltc268x_dev, channnel_no, voltage);
		if (ret) {
			return ret;
		}
	} else {
		for (uint8_t i = 0; i < p_ltc268x_dev->num_channels; i++) {
			/* Set the same span for all the channels */
			ret = ltc268x_set_span(p_ltc268x_dev, i,
					       p_ltc268x_dev->crt_range[ltc268x_active_channel]);
			if (ret) {
				return ret;
			}

			ret = ltc268x_set_voltage(p_ltc268x_dev, i, voltage);
			if (ret) {
				return ret;
			}
		}

		printf(EOL "%f volts is set on all channels" EOL, voltage);
	}

	return MENU_CONTINUE;
}


/*!
 * @brief	Sets the input clock for Dither/Toggle mode.
 * @param	id[in] - User input for mode selection
 * @return	Menu status constant
 */
static int32_t ltc268x_tg_dith_clock_select(uint32_t id)
{
	int32_t ret;

	ret = ltc268x_select_tg_dith_clk(p_ltc268x_dev, ltc268x_active_channel,
					 (enum ltc268x_clk_input) id);
	if (ret) {
		return ret;
	}

	return MENU_DONE;
}

/*
 * Definition of the input register select menu items and menu itself
 * */
static console_menu_item ltc268x_set_input_register_menu_items[] = {
	{ "Select DAC register A", '0', ltc268x_select_input_register, NULL, LTC268X_SELECT_A_REG },
	{ "Select DAC register B", '1', ltc268x_select_input_register, NULL, LTC268X_SELECT_B_REG },
};

console_menu ltc268x_set_input_register_menu = {
	.title = EOL "Command Summary : " EOL,
	.items = ltc268x_set_input_register_menu_items,
	.itemCount = NO_OS_ARRAY_SIZE(ltc268x_set_input_register_menu_items),
	.headerItem = dac_settings_header,
	.footerItem = error_status_footer,
	.enableEscapeKey = true
};

/*
 * Definition of the power select menu items and menu itself
 * */
static console_menu_item ltc268x_power_select_menu_items[] = {
	{ "Power Down ", '0', ltc268x_set_power_mode, NULL, 0},
	{ "Power Up ", '1', ltc268x_set_power_mode, NULL, 1},
};

console_menu ltc268x_power_select_menu = {
	.title = EOL "Command Summary : " EOL,
	.items = ltc268x_power_select_menu_items,
	.itemCount = NO_OS_ARRAY_SIZE(ltc268x_power_select_menu_items),
	.headerItem = dac_settings_header,
	.footerItem = error_status_footer,
	.enableEscapeKey = true
};

/*
 * Definition of the dither/toggle set menu items and menu itself
 * */
static console_menu_item ltc268x_dither_toggle_set_menu_items[] = {
	{ "Toggle Mode", '0', ltc268x_set_dither_toggle_mode, NULL, 0 },
	{ "Dither Mode", '1', ltc268x_set_dither_toggle_mode, NULL, 1 },
};

console_menu ltc268x_dither_toggle_set_menu = {
	.title = EOL "Command Summary : " EOL,
	.items = ltc268x_dither_toggle_set_menu_items,
	.itemCount = NO_OS_ARRAY_SIZE(ltc268x_dither_toggle_set_menu_items),
	.headerItem = dac_settings_header,
	.footerItem = error_status_footer,
	.enableEscapeKey = true
};

/*
 * Definition of the voltage range select menu items and menu itself
 * */
static console_menu_item ltc268x_span_select_menu_items[] = {
	{ "Voltage Range 0V <-> 5V", '0', ltc268x_span_select, NULL, LTC268X_VOLTAGE_RANGE_0V_5V },
	{ "Voltage Range 0V <-> 10V", '1', ltc268x_span_select, NULL, LTC268X_VOLTAGE_RANGE_0V_10V },
	{ "Voltage Range -5V <-> +5V", '2', ltc268x_span_select, NULL, LTC268X_VOLTAGE_RANGE_M5V_5V },
	{ "Voltage Range -10V <-> +10V", '3', ltc268x_span_select, NULL, LTC268X_VOLTAGE_RANGE_M10V_10V },
	{ "Voltage Range -15V <-> +15V", '4', ltc268x_span_select, NULL, LTC268X_VOLTAGE_RANGE_M15V_15V },
};

console_menu ltc268x_span_select_menu = {
	.title = EOL "Command Summary : " EOL,
	.items = ltc268x_span_select_menu_items,
	.itemCount = NO_OS_ARRAY_SIZE(ltc268x_span_select_menu_items),
	.headerItem = dac_settings_header,
	.footerItem = error_status_footer,
	.enableEscapeKey = true
};

/*
 * Definition of the dither phase select menu items and menu itself
 * */
static console_menu_item ltc268x_dither_phase_select_menu_items[] = {
	{ "Dither Phase 0° ", '0', ltc268x_dither_phase_select, NULL, LTC268X_DITH_PHASE_0 },
	{ "Dither Phase 90° ", '1', ltc268x_dither_phase_select, NULL, LTC268X_DITH_PHASE_90 },
	{ "Dither Phase 180° ", '2', ltc268x_dither_phase_select, NULL, LTC268X_DITH_PHASE_180 },
	{ "Dither Phase 270° ", '3', ltc268x_dither_phase_select, NULL, LTC268X_DITH_PHASE_270 },
};

console_menu ltc268x_dither_phase_select_menu = {
	.title = EOL "Command Summary : " EOL,
	.items = ltc268x_dither_phase_select_menu_items,
	.itemCount = NO_OS_ARRAY_SIZE(ltc268x_dither_phase_select_menu_items),
	.headerItem = dac_settings_header,
	.footerItem = error_status_footer,
	.enableEscapeKey = false
};

/*
 * Definition of the dither period select menu items and menu itself
 * */
static console_menu_item ltc268x_dither_period_select_menu_items[] = {
	{ "Dither Period 4 ", '0', ltc268x_dither_period_select, NULL, LTC268X_DITH_PERIOD_4 },
	{ "Dither Period 8 ", '1', ltc268x_dither_period_select, NULL, LTC268X_DITH_PERIOD_8 },
	{ "Dither Period 16 ", '2', ltc268x_dither_period_select, NULL, LTC268X_DITH_PERIOD_16 },
	{ "Dither Period 32 ", '3', ltc268x_dither_period_select, NULL, LTC268X_DITH_PERIOD_32 },
	{ "Dither Period 64 ", '4', ltc268x_dither_period_select, NULL, LTC268X_DITH_PERIOD_64 },
};

console_menu ltc268x_dither_period_select_menu = {
	.title = EOL "Command Summary : " EOL,
	.items = ltc268x_dither_period_select_menu_items,
	.itemCount = NO_OS_ARRAY_SIZE(ltc268x_dither_period_select_menu_items),
	.headerItem = dac_settings_header,
	.footerItem = error_status_footer,
	.enableEscapeKey = false
};

/*
 * Definition of the input clock select menu items and menu itself
 * */
static console_menu_item ltc268x_clock_select_menu_items[] = {
	{ "Software Toggle Bit ", '0', ltc268x_tg_dith_clock_select, NULL, LTC268X_SOFT_TGL },
	{ "TGP 0 Pin ", '1', ltc268x_tg_dith_clock_select, NULL, LTC268X_TGP0 },
	{ "TGP 1 Pin ", '2', ltc268x_tg_dith_clock_select, NULL, LTC268X_TGP1 },
	{ "TGP 2 Pin ", '3', ltc268x_tg_dith_clock_select, NULL, LTC268X_TGP2 },
};

console_menu ltc268x_clock_select_menu = {
	.title = EOL "Command Summary : " EOL,
	.items = ltc268x_clock_select_menu_items,
	.itemCount = NO_OS_ARRAY_SIZE(ltc268x_clock_select_menu_items),
	.headerItem = dac_settings_header,
	.footerItem = error_status_footer,
	.enableEscapeKey = false
};

/*
 * Definition of the ltc268x configurations menu items and menu itself
 * */
static console_menu_item ltc268x_dac_configurations_menu_item[] = {
	{ "Set Active Channel for configuration", '0', ltc268x_set_active_channel, NULL },
	{ "Select the Input Register A/B", '1', NULL, &ltc268x_set_input_register_menu },
	{ "Power Up/Down DAC Channels", '2', NULL, &ltc268x_power_select_menu },
	{ "Set DAC Mode", '3', NULL, &ltc268x_dither_toggle_set_menu },
	{ "Set Channel Span", '4', NULL, &ltc268x_span_select_menu },
	{ "Set Channel Dither Phase", '5', NULL, &ltc268x_dither_phase_select_menu },
	{ "Set Channel Dither Period", '6', NULL, &ltc268x_dither_period_select_menu },
	{ "Set Dither/Toggle Clock Input", '7', NULL, &ltc268x_clock_select_menu },
};
console_menu ltc268x_dac_configurations_menu = {
	.title = EOL "Command Summary : " EOL,
	.items = ltc268x_dac_configurations_menu_item,
	.itemCount = NO_OS_ARRAY_SIZE(ltc268x_dac_configurations_menu_item),
	.headerItem = dac_settings_header,
	.footerItem = NULL,
	.enableEscapeKey = true
};

/*
 * Definition of the ltc268x data operations menu items and menu itself
 * */
static console_menu_item ltc268x_dac_data_operations_menu_items[] = {
	{ "Write Voltage to All Channels", '0', ltc268x_write_dac_voltage, NULL, 0 },
	{ "Write Voltage to Single Channel", '1', ltc268x_write_dac_voltage, NULL, 1 },
};
console_menu ltc268x_dac_data_operations_menu = {
	.title = EOL "Command Summary : " EOL,
	.items = ltc268x_dac_data_operations_menu_items,
	.itemCount = NO_OS_ARRAY_SIZE(ltc268x_dac_data_operations_menu_items),
	.headerItem = dac_settings_header,
	.footerItem = NULL,
	.enableEscapeKey = true
};

/*
 * Definition of the ltc268x main menu items and menu itself
 * */
static console_menu_item ltc268x_main_menu_items[] = {
	{ "Set DAC Configurations", '0', NULL, &ltc268x_dac_configurations_menu},
	{ "Set DAC Data Operations", '1', NULL, &ltc268x_dac_data_operations_menu},
};

console_menu ltc268x_main_menu = {
	.title = EOL "Command Summary : " EOL,
	.items = ltc268x_main_menu_items,
	.itemCount = NO_OS_ARRAY_SIZE(ltc268x_main_menu_items),
	.headerItem = print_title,
	.footerItem = NULL,
	.enableEscapeKey = false
};