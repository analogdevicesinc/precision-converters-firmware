Evaluation Boards/Products Supported
------------------------------------ 
AD5933
AD5934

Recommended Evaluation Board DIGILENT PMOD IA
 
Overview
--------
This is a console application to evalute the AD5933/34 device.
This code was developed and tested on SDP-K1 controller board: https://os.mbed.com/platforms/SDP_K1/ 
with the Digilent PMOD IA board connected to it.
Use of Mbed platform allows code to port on other Mbed supported target boards with little or no modifications.
 
Product details: https://www.analog.com/en/products/ad5933.html
Eval board details: https://digilent.com/reference/pmod/pmodia/start
User Guide for this code: https://wiki.analog.com/resources/tools-software/product-support-software/ad5933-mbed
Communication Protocol: I2C


Hardware Setup
--------------
Required: SDP-K1 (or alternative MBED enabled controller board), recommend Digilent PMOD IA, USB cable.
Connect the DIGILENT PMOD IA board to SDP-K1 board (or any other Mbed enabled controller board) 
using the SDP-120 or Arduino on-board connector (refer software wiki page to identify suitable interface) and short jumper-wires.
Connect SDP-K1 board to the PC using the USB cable.


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