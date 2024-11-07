from adi.ad7091r import ad7091rx
from adi.attribute import attribute

# Create a child class of AD7091r parent class for defining extended iio attributes (the ones which
# are not part of original linux iio drivers and created for non-linux iio applications)
class ad7091rx_xattr(ad7091rx):

    # List of extended iio channels
    xchannel = []

    def __init__(self, uri, device_name):
        super().__init__(uri, device_name)

        # Create an instances of the extended channel class
        for ch in self._ctrl.channels:
            name = ch._id
            self._rx_channel_names.append(name)
            self.xchannel.append(self._xchannel(self._ctrl, name))

        # Add the _xchannel class methods as _channel methods, so that they can be accessed with
        # attribute name as 'channel' instead 'xchannel'
        # e.g. channel attribute 'thresh_alert' can be accessed as either "obj.channel[chn_num].thresh_alert" Or
        # "obj.xchannel[chn_num].thresh_alert", where obj refers to object of 'ad7091rx_xattr' class
        self._channel.thresh_alert = self._xchannel.thresh_alert

    #------------------------------------------------
    # Device extended attributes
    #------------------------------------------------

    @property
    def sampling_frequency(self):
        """AD7091r sampling frequency"""
        return self._get_iio_dev_attr_str("sampling_frequency")

    @sampling_frequency.setter
    def sampling_frequency(self, value):
        """AD7091r device sampling_frequency config"""
        self._set_iio_dev_attr("sampling_frequency", value)

    @property
    def alert_bsy_gpo0_en(self):
        """AD7091r get alert bsy gpo0"""
        return self._get_iio_dev_attr_str("alert_bsy_gpo0_en")

    @property
    def alert_bsy_gpo0_en_avail(self):
        """Get list of alert_bsy_gpo0_en options"""
        return self._get_iio_dev_attr_str("alert_bsy_gpo0_en_available")

    @alert_bsy_gpo0_en.setter
    def alert_bsy_gpo0_en(self, value):
        """Set alert_bsy_gpo0_en option"""
        if value in self.alert_bsy_gpo0_en_avail:
            self._set_iio_dev_attr_str("alert_bsy_gpo0_en", value)
        else:
            raise ValueError(
                "Error: alert_bsy_gpo0_en setting not supported \nUse one of: "
                "str(self.alert_bsy_gpo0_en_avail)"
            )

    @property
    def alert_pol_or_gp0_value(self):
        """AD7091r get alert_pol_or_gp0_value"""
        return self._get_iio_dev_attr_str("alert_pol_or_gp0_value")

    @property
    def alert_pol_or_gp0_value_avail(self):
        """Get list of alert_pol_or_gp0_value options"""
        return self._get_iio_dev_attr_str("alert_pol_or_gp0_value_available")

    @alert_pol_or_gp0_value.setter
    def alert_pol_or_gp0_value(self, value):
        """Set alert_pol_or_gp0_value option"""
        if value in self.alert_pol_or_gp0_value_avail:
            self._set_iio_dev_attr_str("alert_pol_or_gp0_value", value)
        else:
            raise ValueError(
                "Error: alert_pol_or_gp0_value setting not supported \nUse one of: "
                "str(self.alert_pol_or_gp0_value_avail)"
            )

    #------------------------------------------------
    # Channel extended attributes
    #------------------------------------------------

    class _xchannel(attribute):

        def __init__(self, ctrl, channel_name):
            self.name = channel_name
            self._ctrl = ctrl

        @property
        def thresh_alert(self):
            """AD7091r get channel thresold alert"""
            return self._get_iio_attr_str(self.name, "thresh_alert", False)

        @property
        def thresh_alert_avail(self):
            """Get list of thresold alert options"""
            return self._get_iio_attr_str(self.name, "thresh_alert_available", False)

        @thresh_alert.setter
        def thresh_alert(self, value):
            """Set channel thresold alert"""
            if value in self.thresh_alert_avail:
                self._set_iio_attr(self.name, "thresh_alert", value, False)
            else:
                raise ValueError(
                    "Error: thresh_alert setting not supported \nUse one of: "
                    "str(self.thresh_alert_avail)"
                )