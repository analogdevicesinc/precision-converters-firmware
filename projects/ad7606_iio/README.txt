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
Refer "Hardware Connections" section from below page link:
https://analogdevicesinc.github.io/precision-converters-firmware/source/projects/ad7606_iio/ad7606_iio.html


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

Copyright (c) 2020 Analog Devices, Inc.  All rights reserved.