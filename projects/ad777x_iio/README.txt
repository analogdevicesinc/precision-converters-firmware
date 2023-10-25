Evaluation Boards/Products Supported
------------------------------------ 
-Products supported: AD7770, AD7771, AD7779

Overview
--------
This is a IIO based firmware application to evalute the AD777x device.
This code was developed and tested on SDP-K1 controller board for mbed platform : https://os.mbed.com/platforms/SDP_K1/.
and the NucleoL552ZEQ Board for STM32 platform: https://os.mbed.com/platforms/ST-Nucleo-L552ZE-Q/

Product details: 
https://www.analog.com/en/products/ad7770.html
https://www.analog.com/en/products/ad7771.html
https://www.analog.com/en/products/ad7779.html
Communication Protocol: SPI, SAI-TDM (Applicable only for STM32 platform)


Hardware Setup
--------------
Required: SDP-K1 (or alternative Mbed/STM32 enabled controller board), EVAL-AD777x board and USB cable.

Find further instructions on the hardware connections here:
https://wiki.analog.com/resources/tools-software/product-support-software/ad777x_iio_support#hardware_connections


How to Get Started
------------------
The Firmware supports MBED and STM32 platforms. Import code into the respective platform and compile it to generate the executable binary.

Find detailed instructions for MBED platform here: 
https://wiki.analog.com/resources/tools-software/product-support-software/pcg-fw-mbed-build-guide

Find detailed instructions for STM32 platform here: 
https://wiki.analog.com/resources/tools-software/product-support-software/pcg-fw-stm32-build-guide


Notes
-----
A detailed user guide on SDP-K1 controller board is available here:
https://os.mbed.com/platforms/SDP_K1/
https://www.analog.com/en/design-center/evaluation-hardware-and-software/evaluation-boards-kits/SDP-K1.html.

A Detailed user guide on Nucleo-L552ZEQ board is available here:
https://os.mbed.com/platforms/ST-Nucleo-L552ZE-Q/
https://www.st.com/content/ccc/resource/technical/document/user_manual/group1/ad/a4/bd/5e/14/15/4e/e8/DM00615305/files/DM00615305.pdf/jcr:content/translations/en.DM00615305.pdf

Copyright (c) 2023 Analog Devices, Inc.  All rights reserved.

