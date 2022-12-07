Evaluation Boards/Products Supported
------------------------------------ 
EVAL-AD7380FMCZ


Overview
--------
This is an IIO ecosystem based firmware application to evaluate the AD738X family devices.
This code was developed and tested on SDP-K1 controller board: https://os.mbed.com/platforms/SDP_K1/
Use of Mbed platform allows code to port on other Mbed supported target boards with little or no modifications.

Product details: AD7380-2
Product Evaluation board details: EVAL-AD7380FMCZ
User Guide for this code: TBD
Communication Protocol: SPI


Hardware Setup
--------------
Required: SDP-K1 (or alternative Mbed enabled controller board), EVAL-AD7380 board, 9V adapter and USB cable.
Connect the EVAL-AD7380 board to SDP-K1 board (or any other Mbed enabled controller board) using jumper wires
(refer software wiki page for connection setup).
Connect SDP-K1 board to the PC using the USB cable. AD7380 EVB is powered through external 9-12V DC adapter.


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


Copyright (c) 2021 Analog Devices, Inc.  All rights reserved.
