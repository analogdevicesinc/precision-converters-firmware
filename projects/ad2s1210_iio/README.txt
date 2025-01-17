Evaluation Boards/Products Supported
------------------------------------
EVAL-AD2S1210


Overview
--------
This is an IIO firmware application to evaluate the AD2S1210 device.
This code was developed and tested on SDP-K1 controller board: https://os.mbed.com/platforms/SDP_K1/
Use of the Mbed platform allows code to be ported to other Mbed supported target boards with little or no modifications.

Product details: AD2S1210.
Product Evaluation board details: EVAL-AD2S1210SDZ
User Guide for this code: https://analogdevicesinc.github.io/precision-converters-firmware/source/projects/ad2s1210_iio/ad2s1210_iio.html
Communication Protocol: SPI


Hardware Setup
--------------
Required: SDP-K1, ADZS-BRKOUT, EVAL-AD2S1210 board and USB cable.
Plug in the EVAL-AD2S1210 board to the ADZS-BRKOUT, populate flywires to the
SDP-K1 board using the Arduino connector.

The connections are as follows:

SDPK1			BREAKOUT
DIGITAL-0	->	96  A0
DIGITAL-1	->	25  A1
DIGITAL-4	->	48  SAMPLE
DIGITAL-5	->	43  RES0
DIGITAL-6	->	78  RES1
DIGITAL-10	->	100 nWD
DIGITAL-11	->	12  SDI
DIGITAL-12	->	110 SDO
DIGITAL-13	->	13  SCLK
POWER-GND	->	4   GIO
POWER-3v3	->	116 VIO

2S1210-EVAL
J3 CS -> J3 GND



(refer to the software wiki page for connection setup).
Connect the SDP-K1 board to the PC using the USB cable and the AD2S1210 EVB to the
provided AC adapter.


How to Get Started
------------------
Mbed web/online compiler: https://studio.keil.arm.com/auth/login/
Import the code into the compiler and compile it to generate the executable binary file.
Drag and drop the binary file onto the USB drive hosted by the SDP-K1 controller board.
Find detailed instructions here: https://analogdevicesinc.github.io/precision-converters-firmware/source/build/project_build.html


Notes
-----
A detailed user guide on the SDP-K1 controller board is available here:
https://os.mbed.com/platforms/SDP_K1/
https://www.analog.com/en/design-center/evaluation-hardware-and-software/evaluation-boards-kits/SDP-K1.html.


Copyright (c) 2023 Analog Devices, Inc.  All rights reserved.
Copyright (c) 2023 BayLibre, SAS
