Evaluation Boards/Products Supported
------------------------------------ 
-Products supported: AD7606B, AD7606C, AD7605-4, AD7606-4, AD7606-6, AD7606-8, AD7608, AD7609
-EVAL-AD7606B-FMCZ


Overview
--------
This is an IIO ecosystem based firmware application to evalute the AD7606 family ADCs.
The ARM GCC compiler is used for firmware development on ARM Mbed platform.
The ADI developed ARM Mbed SDP-K1 controlled board acts as a target device, where firmware application is running.
The AD7606-Eval board is interfaced with SDP-K1 (or any Arduino compatible board) controller board using on-board Arduino Headers.


Hardware Setup
--------------
Refer "Hardware Connections" section from below wiki page link:
https://wiki.analog.com/resources/tools-software/product-support-software/ad7606_mbed_iio_application


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

Copyright (c) 2020 Analog Devices, Inc.  All rights reserved.