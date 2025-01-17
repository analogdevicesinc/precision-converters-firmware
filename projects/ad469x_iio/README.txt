Evaluation Boards/Products Supported
------------------------------------ 
-Products supported: AD4695, AD4696
-EVAL-AD4696FMCZ


Overview
--------
This is an IIO ecosystem based firmware application to evalute the AD4696 family ADCs.
The ADI developed ARM Mbed SDP-K1 controlled board acts as a target device, where firmware application is running.
The ARM GCC/ARMC compiler is used for firmware development on ARM Mbed platform.
The EVAL-AD4696FMCZ board is interfaced with SDP-K1 (or any Arduino compatible board) controller board using on-board Arduino Headers.


Hardware Setup
--------------
Refer "Hardware Connections" section from below page:
https://analogdevicesinc.github.io/precision-converters-firmware/source/projects/ad4696_iio/ad4696_iio.html


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