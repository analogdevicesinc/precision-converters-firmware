# adistream for Streaming

## Commands to Use

### For Help
```sh
py adistream.py -h
```

### Generating and Streaming Data to Default Channel 0
```sh
py adistream.py <class_name> <uri> <neg_v_ref> <pos_v_ref>
```
- *class_name* corresponds to the device PyADI device class name
- *uri* corresponds to the URI of the target device connected
- *neg_v_ref* corresponds to the negative reference voltage of DAC in Volts. Inpout 0V for unipolar DACs
- *pos_v_ref* corresponds to the positive reference voltage of DACs in Volts.

*Note: Please make sure channels are set to normal operating mode and powered up (if applicable) before initiating data streaming.*

### Streaming to Channels 0, 1, and 2
```sh
py adistream.py ad3530r serial:COM37,230400 0 2.5 -cl 0 1 2
```

### Generating a Binfile with Data (for Input to IIO Oscilloscope)
```sh
py adistream.py ad3530r serial:COM37,230400 0 2.5 -b
```
```sh
py adistream.py ad3530r serial:COM37,230400 0 2.5 -cl 0 1 2 -b
```

[Loading and streaming data from IIO Oscilloscope](https://analogdevicesinc.github.io/precision-converters-firmware/source/iio_osc/iio_osc.html#loading-and-streaming-the-data)

## More Info
For more information on adistream tool, visit the [ADIStream: For DACs](https://confluence.analog.com/pages/viewpage.action?pageId=916522245#UsingPyADICommandLineInterfaceTools-ADIStream:ForDACs) page.

# adilog for Data Capture

### For Help
```sh
py adilog.py -h
```
### Logging and Plotting data from Default Channel 0
```sh
py adilog.py <class_name> <uri>
```
- *class_name* corresponds to the device PyADI device class name
- *uri* corresponds to the URI of the target device connected

## More Info
For more information on adilog tool, visit the [ADILog: For ADCs](https://confluence.analog.com/pages/viewpage.action?pageId=916522245#UsingPyADICommandLineInterfaceTools-ADILog:ForADCs) page.