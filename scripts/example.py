import adi

USE_AD353XR = 0 #Change this to switch device

if USE_AD353XR:
    device = adi.ad353xr("serial:COM4,230400", "ad3531r")
    print(device.all_ch_operating_mode)
    device.all_ch_operating_mode = "normal_operation"
    device.reference_select = "internal_ref"
    print(device.all_ch_operating_mode)
    device.channel[0].raw = 25000
    print(device.channel[0].raw)
else:
    device = adi.ad405x("serial:COM59,230400", "ad4050")
    print(device.sampling_frequency)
    device.sampling_frequency = 100000
    print(device.sampling_frequency)
    # print(device.channels[0].raw)