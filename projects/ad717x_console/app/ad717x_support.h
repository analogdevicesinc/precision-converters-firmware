/*!
 *****************************************************************************
  @file:  ad717x_support.h

  @brief: Header for AD717x/AD411x No-OS driver supports

  @details:
 -----------------------------------------------------------------------------
 Copyright (c) 2020 Analog Devices, Inc.
 All rights reserved.

 This software is proprietary to Analog Devices, Inc. and its licensors.
 By using this software you agree to the terms of the associated
 Analog Devices Software License Agreement.
*****************************************************************************/

#ifndef AD717X_SUPPORT_H_
#define AD717X_SUPPORT_H_

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

/******************************************************************************/
/********************** Macros and Constants Definitions **********************/
/******************************************************************************/

/*
 * Create a contiguous bitmask starting at bit position @l and ending at
 * position @h.
 */
#ifndef GENMASK
#define GENMASK(h, l) (((~0UL) - (1UL << (l)) + 1) & (~0UL >> (31 - (h))))
#endif
#define BIT(x)        (1UL << (x))


/* ADC Mode Register bits */
#define AD717X_ADCMODE_REG_MODE_MSK   GENMASK(6,4)
#define AD717X_ADCMODE_REG_MODE_RD(x) (((x) >> 4) & 0x7)


/* Channel Map Register bits */
#define AD717X_CHMAP_REG_CH_EN_RD(x)   (((x) >> 15) & 0x1)

#define AD717X_CHMAP_REG_SETUP_SEL_MSK    GENMASK(14,12)
#define AD717X_CHMAP_REG_SETUP_SEL_RD(x)  (((x) >> 12) & 0x7)

#define AD717X_CHMAP_REG_AINPOS_MSK    GENMASK(9,5)
#define AD717X_CHMAP_REG_AINPOS_RD(x)  (((x) >> 5) & 0x1F)

#define AD717X_CHMAP_REG_AINNEG_MSK    GENMASK(4,0)
#define AD717X_CHMAP_REG_AINNEG_RD(x)  (((x) >> 0) & 0x1F)

/* Channel Map Register additional bits for AD4111, AD4112, AD4114, AD4115 */
#define AD4111_CHMAP_REG_INPUT_MSK     GENMASK(9,0)
#define AD4111_CHMAP_REG_INPUT_RD(x)   (((x) >> 0) & 0x3FF)


/* Setup Configuration Register bits */
#define AD717X_SETUP_CONF_REG_BI_UNIPOLAR_RD(x)  (((x) >> 12) & 0x1)

#define AD717X_SETUP_CONF_REG_REF_SEL_MSK    GENMASK(5,4)
#define AD717X_SETUP_CONF_REG_REF_SEL_RD(x)  (((x) >> 4) & 0x3)

/* Setup Configuration Register additional bits for AD7173-8 */
#define AD717X_SETUP_CONF_REG_REF_BUF_MSK    GENMASK(11,10)
#define AD717X_SETUP_CONF_REG_REF_BUF_RD(x)  (((x)>> 10) & 0x3)

#define AD717X_SETUP_CONF_REG_AIN_BUF_MSK    GENMASK(9,8)
#define AD717X_SETUP_CONF_REG_AIN_BUF_RD(x)  (((x) >> 8) & 0x3)

/* Setup Configuration Register additional bits for AD7172-2, AD7172-4, AD7175-2 */
#define AD717X_SETUP_CONF_REG_REFBUF_P_RD(x)  (((x) >> 11) & 0x1)
#define AD717X_SETUP_CONF_REG_REFBUF_N_RD(x)  (((x) >> 10) & 0x1)

#define AD717X_SETUP_CONF_REG_AINBUF_P_RD(x)  (((x) >> 9) & 0x1)
#define AD717X_SETUP_CONF_REG_AINBUF_N_RD(x)  (((x) >> 8) & 0x1)

/* Setup Configuration Register additional bits for AD4111, AD4112, AD4114, AD4115 */
#define AD4111_SETUP_CONF_REG_REFPOS_BUF_RD(x)  (((x) >> 11) & 0x1)
#define AD4111_SETUP_CONF_REG_REFNEG_BUF_RD(x)  (((x) >> 10) & 0x1)

#define AD4111_SETUP_CONF_REG_AIN_BUF_MSK     GENMASK(9,8)
#define AD4111_SETUP_CONF_REG_AIN_BUF_RD(x)   (((x) >> 8) & 0x3)


/* Filter Configuration Register bits */
#define AD717X_FILT_CONF_REG_ENHFILTEN_RD(x)  (((x) >> 11) & 0x1)

#define AD717X_FILT_CONF_REG_ENHFILT_MSK      GENMASK(10,8)
#define AD717X_FILT_CONF_REG_ENHFILT_RD(x)    (((x) >> 8) & 0x7)

#define AD717X_FILT_CONF_REG_ORDER_MSK        GENMASK(6,5)
#define AD717X_FILT_CONF_REG_ORDER_RD(x)      (((x) >> 5) & 0x3)

#define AD717X_FILT_CONF_REG_ODR_MSK          GENMASK(4,0)
#define AD717X_FILT_CONF_REG_ODR_RD(x)        (((x) >> 0) & 0x1F)


// ADC operating mode types
#define CONTINUOUS_CONVERSION		0
#define SINGLE_CONVERISION			1
#define STANDBY_MODE				2
#define POWER_DOWN_MODE				3
#define INTERNAL_OFFSET_CAL_MODE	4
#define INTERNAL_FULL_SCALE_CAL_MODE 5

/******************************************************************************/
/********************** Variables and User Defined Data Types *****************/
/******************************************************************************/

/******************************************************************************/
/************************ Public Declarations *********************************/
/******************************************************************************/

#endif /* AD717X_SUPPORT_H_ */
