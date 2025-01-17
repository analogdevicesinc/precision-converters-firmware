Evaluation Boards/Products Supported
------------------------------------ 
-Products supported: AD7770, AD7771, AD7779

Overview
--------
This is a IIO based firmware application to evalute the AD777x device.
This code was developed and tested on SDP-K1 controller board for mbed platform : https://os.mbed.com/platforms/SDP_K1/.
and the Nucleo-H563ZI Board for STM32 platform: https://www.st.com/en/evaluation-tools/nucleo-h563zi.html

Product details: 
https://www.analog.com/en/products/ad77770.html
https://www.analog.com/en/products/ad77771.html
https://www.analog.com/en/products/ad77779.html
Communication Protocol: SPI, SAI-TDM (Applicable only for STM32 platform)


Hardware Setup
--------------
Required: SDP-K1 (or alternative Mbed/STM32 enabled controller board), EVAL-AD777x board and USB cable.

Find further instructions on the hardware connections here:
https://analogdevicesinc.github.io/precision-converters-firmware/source/projects/ad777x_iio/ad777x_iio.html


How to Get Started
------------------
The Firmware supports MBED and STM32 platforms. Import code into the respective platform and compile it to generate the executable binary.

Find detailed instructions here: 
https://analogdevicesinc.github.io/precision-converters-firmware/source/build/project_build.html


Notes
-----
A detailed user guide on SDP-K1 controller board is available here:
https://os.mbed.com/platforms/SDP_K1/
https://www.analog.com/en/design-center/evaluation-hardware-and-software/evaluation-boards-kits/SDP-K1.html.

A Detailed user guide on Nucleo-H563ZI board is available here:
https://www.st.com/en/evaluation-tools/nucleo-h563zi.html
https://www.st.com/resource/en/user_manual/um3115-stm32h5-nucleo144-board-mb1404-stmicroelectronics.pdf

Copyright (c) 2023 Analog Devices, Inc.  All rights reserved.

