Evaluation Boards/Products Supported
------------------------------------ 
ADT7420, ADT7320

Overview
--------
These code files provide the temperature measurement console application and device libraries to 
interface with AD7420 product evaluation board. This code was developed and tested on SDP-K1 
controller board: https://os.mbed.com/platforms/SDP_K1/

Product details: https://www.analog.com/en/products/adt7420.html, https://www.analog.com/en/products/adt7320.html
Product Evaluation board details: https://www.analog.com/en/design-center/evaluation-hardware-and-software/evaluation-boards-kits/EV-TempSense-ARDZ.html
User Guide for this code: https://analogdevicesinc.github.io/precision-converters-firmware/source/projects/adt7420_console/adt7420_console.html
Communication Protocol: SPI/I2C


Hardware Setup
--------------
Required: SDP-K1 (or alternative Mbed enabled controller board), EV_Tempsense-ARDZ
USB cable.
Plug in the EV_Tempsense-ARDZ board on SDP-K1 board (or any other Mbed enabled controller board) 
using the SDP or Arduino on-board connector (default set in software is Arduino).
Connect SDP-K1 board to the PC using the USB cable.


How to Get Started
------------------
Mbed web/online compiler: https://studio.keil.arm.com/auth/login/
Import code into compiler and compile it to generate executable binary file. 
Drag and drop binary file into USB drive hosted by SDP-K1 controller board. 
Find detailed instructions here: https://analogdevicesinc.github.io/precision-converters-firmware/source/build/project_build.html


Notes
-----
A detailed user guide on SDP-K1 controller board is available here:
https://os.mbed.com/platforms/SDP_K1/
https://www.analog.com/en/design-center/evaluation-hardware-and-software/evaluation-boards-kits/SDP-K1.html.

Copyright (c) 2021 Analog Devices, Inc.  All rights reserved.

