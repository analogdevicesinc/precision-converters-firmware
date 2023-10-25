# @file    ad777x_xattr.py
# @brief   Extended attributes for the AD777x
#
# Copyright (c) 2023 Analog Devices, Inc.
# All rights reserved.
# 
# This software is proprietary to Analog Devices, Inc. and its licensors.
# By using this software you agree to the terms of the associated
# Analog Devices Software License Agreement.
#

from adi.ad777x import *
from adi.attribute import attribute
from decimal import Decimal

# Create a child class of ad777x parent class for defining extended iio attributes (the ones which
# are not part of original linux iio drivers and created for non-linux iio applications)
class ad777x_xattr(ad777x):

    #------------------------------------------------
    # Device extended attributes
    #------------------------------------------------
    
    @property
    def conversion_mode(self):
        """AD777x Conversion Mode"""
        return self._get_iio_dev_attr_str("conversion_mode")

    @property
    def error_status_1(self):
        """AD777x Error Status 1"""
        return self._get_iio_dev_attr_str("error_status1")

    @property
    def error_status_2(self):
        """AD777x Error Status 2"""
        return self._get_iio_dev_attr_str("error_status2")

    @property
    def error_status_3(self):
        """AD777x Error Status 3"""
        return self._get_iio_dev_attr_str("error_status3")

    @property
    def auxainp_auxainn_mux(self):
        return self._get_iio_dev_attr_str("auxainp_auxainn")

    @property
    def dvbe_avssx_mux(self):
        return self._get_iio_dev_attr_str("dvbe_avssx")
   
    @property
    def ref1p_ref1n_mux(self):
        return self._get_iio_dev_attr_str("ref1p_ref1n")

    @property
    def ref2p_ref2n_mux(self):
        return self._get_iio_dev_attr_str("ref2p_ref2n")
    
    @property
    def ref_out_avssx_mux(self):
        return self._get_iio_dev_attr_str("ref_out_avssx")

    @property
    def vcm_avssx_mux(self):
        return self._get_iio_dev_attr_str("vcm_avssx")

    @property
    def areg1cap_avssx_mux(self):
        return self._get_iio_dev_attr_str("areg1cap_avssx")
    
    @property
    def areg2cap_avssx_mux(self):
        return self._get_iio_dev_attr_str("areg2cap_avssx")

    @property
    def dregcap_dgnd_mux(self):
        return self._get_iio_dev_attr_str("dregcap_dgnd")

    @property
    def avdd1a_avssx_mux(self):
        return self._get_iio_dev_attr_str("avdd1a_avssx")

    @property
    def avdd1b_avssx_mux(self):
        return self._get_iio_dev_attr_str("avdd1b_avssx")

    @property
    def avdd2a_avssx_mux(self):
        return self._get_iio_dev_attr_str("avdd2a_avssx")

    @property
    def avdd2b_avssx_mux(self):
        return self._get_iio_dev_attr_str("avdd2b_avssx")

    @property
    def iovdd_dgnd_mux(self):
        return self._get_iio_dev_attr_str("iovdd_dgnd")

    @property
    def avdd4_avssx_mux(self):
        return self._get_iio_dev_attr_str("avdd4_avssx")
    
    @property
    def dgnd_avss1a_mux(self):
        return self._get_iio_dev_attr_str("dgnd_avss1a")

    @property
    def avdd4_avssx_mux(self):
        return self._get_iio_dev_attr_str("dgnd_avss1b")

    @property
    def ref1p_avssx_mux(self):
        return self._get_iio_dev_attr_str("ref1p_avssx")

    @property
    def ref2p_avssx_mux(self):
        return self._get_iio_dev_attr_str("ref2p_avssx")

    @property
    def avssx_avdd4_mux(self):
        return self._get_iio_dev_attr_str("avssx_avdd4")

    @property
    def sampling_rate_converter_int(self):
        return self._get_iio_dev_attr_str("sampling_rate_converter_int")

    @property
    def sampling_rate_converter_dec(self):
        return self._get_iio_dev_attr_str("sampling_rate_converter_dec")

    @property
    def sample_rate(self):
        """AD777x device sample_rate"""
        return int(self._get_iio_dev_attr_str("sampling_frequency"))