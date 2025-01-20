-------------------------------------------
adistream for streaming

Commands to use:
-------------------------------------------
For help:
py adistream.py -h

Note: Please ake sure channels are set to normal operating mode before initiating data streaming.

py adistream.py ad3530r serial:COM37,230400 0 2.5
// For generating and streaming data to default ch 0

py adistream.py ad3530r serial:COM37,230400 0 2.5 -cl 0 1 2
// for streaming to channels 0, 1, and 2

---------------------------------------------

More info:
https://confluence.analog.com/display/LPTSWDEV/Using+PyADI+Command+Line+Interface+Tools

---------------------------------------------

