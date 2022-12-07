Evaluation Boards/Products Supported
------------------------------------ 
-Products supported: AD4130


Overview
--------
This is an IIO ecosystem based firmware application to evalute the AD4130 devices.
This code was developed and tested on SDP-K1 controller board: https://os.mbed.com/platforms/SDP_K1/.
Use of Mbed platform allows code to port on other Mbed supported target boards with little or no modifications.

Product details: AD4130-8
Product Evaluation board details: EV-AD4130-8ASDZ-U1
User Guide for this code: https://wiki.analog.com/resources/eval/user-guides/ad4130/mbed_iio_app
Communication Protocol: SPI


Hardware Setup
--------------
Required: SDP-K1 (or alternative Mbed enabled controller board), EVAL-AD4130 board and USB cable.
Plug in the EVAL-AD4130 board on SDP-K1 board (or any other Mbed enabled controller board)
using the Arduino connector (refer software wiki page for connection interface).
Connect SDP-K1 board to the PC using the USB cable. AD4130 EVB is powered through USB supply of SDP-K1.


How to Get Started
------------------
Mbed web/online compiler: https://studio.keil.arm.com/auth/login/
Import code into compiler and compile it to generate executable binary file. 
Drag and drop binary file into USB drive hosted by SDP-K1 controller board. 
Find detailed instructions here: https://wiki.analog.com/resources/tools-software/product-support-software/pcg-fw-mbed-build-guide


Notes
-----
A detailed user guide on SDP-K1 controller board is available here:
https://os.mbed.com/platforms/SDP_K1/
https://www.analog.com/en/design-center/evaluation-hardware-and-software/evaluation-boards-kits/SDP-K1.html.


Copyright (c) 2022 Analog Devices, Inc.  All rights reserved.
