from adi.context_manager import context_manager
from adi.rx_tx import rx
from adi.attribute import attribute

class ad4170_system_config(rx, context_manager):
    """ AD4170 System Config """

    _complex_data = False
    _device_name = ""

    def __init__(self, uri="", device_name=""):

        context_manager.__init__(self, uri, self._device_name)

        compatible_parts = [
            "system_config"
        ]

        self._ctrl = None

        if not device_name:
            device_name = compatible_parts[0]
        else:
            if device_name not in compatible_parts:
                raise Exception("Not a compatible device: " + device_name)

        # Select the device matching device_name as working device
        for device in self._ctx.devices:
            if device.name == device_name :
                self._ctrl = device
                break

        # Initialize channels
        self.channels = []
        for ch in self._ctrl.channels:
            self.channels.append(self._channel(self._ctrl, ch._id))
    
    @property
    def reconfigure_system(self):
        """Get reconfigure system."""
        return self._get_iio_dev_attr_str("reconfigure_system")

    @property
    def reconfigure_system_avail(self):
        """Get reconfigure system options.
        Options: Enable
        """
        return self._get_iio_dev_attr_str("reconfigure_system_available")

    @reconfigure_system.setter
    def reconfigure_system(self, value):
        """Set reconfigure system."""
        if value in self.reconfigure_system_avail.split():
            self._set_iio_dev_attr_str("reconfigure_system", value)
        else:
            raise ValueError(
                "Error: reconfigure system option not supported. "
                f"Use one of: {self.reconfigure_system_avail}"
            )
    
    class _channel(attribute):
        """AD4170 Channel Configuration"""

        def __init__(self, ctrl, channel_name):
            self.name = channel_name
            self._ctrl = ctrl
            self._chan = None

            # Find the channel by name
            for ch in ctrl.channels:
                if ch.id == channel_name:
                    self._chan = ch
                    break

            if self._chan is None:
                raise Exception(f"Channel not found: {channel_name}")
            
        @property
        def ch_en_avail(self):
            """Get available ch_en options.
            Options: enabled, disabled
            """
            return self._chan.attrs["ch_en_available"].value.split(" ")
         
        @property
        def ch_en(self):
            """Set the ch_en value."""
            return self._chan.attrs["ch_en"].value

        @ch_en.setter
        def ch_en(self, value):
            """Enable or disable the current channel."""
            if value in self.ch_en_avail:
                self._chan.attrs["ch_en"].value = value
            else:
                raise ValueError(
                    f"Error: Configuration '{value}' not supported for channel {self.name}. "
                    f"Use one of: {self.ch_en}"
                )
  

