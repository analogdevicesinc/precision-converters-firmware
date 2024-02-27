from adi.ad579x import *
from adi.attribute import attribute

# Create a child class of ad579x parent class for defining extended iio attributes (the ones which
# are not part of original linux iio drivers and created for non-linux iio applications)
class ad579x_xattr(ad579x):

    def __init__(self, uri, device_name):
        super().__init__(uri, device_name)

    #------------------------------------------------
    # Device extended attributes
    #------------------------------------------------

    @property
    def coding_select_available(self):
        """AD579x coding select options available"""
        return self._get_iio_dev_attr_str("coding_select_available")

    @property
    def coding_select(self):
        """AD579x coding select config"""
        return self._get_iio_dev_attr_str("coding_select")

    @coding_select.setter
    def coding_select(self, value):
        if value in self.coding_select_available:
            self._set_iio_dev_attr_str("coding_select", value)
        else:
            raise ValueError("Error: coding select option not supported \nUse one of: " + str(self.coding_select_available))

    @property
    def clear_code(self):
        """AD579x clear code register value"""
        return self._get_iio_dev_attr_str("clear_code")

    @clear_code.setter
    def clear_code(self, value):
        self._set_iio_dev_attr_str("clear_code", value)

    @property
    def output_amplifier_available(self):
        """AD579x output amplifier available options"""
        return self._get_iio_dev_attr_str("output_amplifier_available")

    @property
    def output_amplifier(self):
        """AD579x output amplifier config"""
        return self._get_iio_dev_attr_str("output_amplifier")

    @output_amplifier.setter
    def output_amplifier(self, value):
        if value in self.output_amplifier_available:
            self._set_iio_dev_attr_str("output_amplifier", value)
        else:
            raise ValueError("Error: output amplifier setting not supported \nUse one of: " + str(self.output_amplifier_available))


    @property
    def clear_available(self):
        """AD579x clear available"""
        return self._get_iio_dev_attr_str("clear_available")

    @property
    def clear(self):
        """AD579x clear async"""
        return self._get_iio_dev_attr_str("clear")

    @clear.setter
    def clear(self, value):
        if value in self.clear_available:
            self._set_iio_dev_attr_str("clear", value)
        else:
            raise ValueError("Error: Clear value not supported \nUse one of: " + str(self.clear_available))

    @property
    def hw_ldac_trigger_available(self):
        """AD579x hw_ldac_trigger available"""
        return self._get_iio_dev_attr_str("hw_ldac_trigger_available")

    @property
    def hw_ldac_trigger(self):
        """AD579x HW LDAC trigger"""
        return self._get_iio_dev_attr_str("hw_ldac_trigger")

    @hw_ldac_trigger.setter
    def hw_ldac_trigger(self, value):
        if value in self.hw_ldac_trigger_available:
            self._set_iio_dev_attr_str("hw_ldac_trigger", value)
        else:
            raise ValueError("Error: Trigger value not supported \nUse one of: " + str(self.hw_ldac_trigger_available))
