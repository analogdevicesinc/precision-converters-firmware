Evaluation Boards/Products Supported
------------------------------------ 
DC2873A-B/LTC2688
DC2904A-B/LTC2686

 
Overview
--------
This is a console application to evalute the LTC2688 using DC2873A-B/DC2904A-B evaluation board.
This code was developed and tested on SDP-K1 controller board: https://os.mbed.com/platforms/SDP_K1/.
Use of Mbed platform allows code to port on other Mbed supported target boards with little or no modifications.
 
Product details: https://www.analog.com/en/products/LTC2688.html
Product Evaluation board details: 
https://www.analog.com/en/design-center/evaluation-hardware-and-software/evaluation-boards-kits/DC2873A.html
https://www.analog.com/en/design-center/evaluation-hardware-and-software/evaluation-boards-kits/DC2904A.html
User Guide for this code: https://wiki.analog.com/resources/tools-software/product-support-software/ltc268x
Communication Protocol: SPI
 
 
Hardware Setup
--------------
Required: SDP-K1 (or alternative Mbed enabled controller board), DC2873A-B board and USB cable.
Connect the DC2873A-B or DC2904A-B board to the SDP-K1 board (or any other Mbed enabled controller board)
using the Arduino connector and jumper wires (refer software wiki page to identify suitable interface).
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