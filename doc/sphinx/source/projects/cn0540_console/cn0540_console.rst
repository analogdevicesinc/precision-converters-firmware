CN0540 Console Application
""""""""""""""""""""""""

==================
Supported Hardware
==================

**Supported Devices:**

* `AD7768-1 <https://www.analog.com/en/products/ad7768-1.html>`_
* `LTC2606 <https://www.analog.com/en/products/ltc2606.html>`_

**Supported Evaluation Boards:**

* `EVAL-CN0540-ARDZ <https://www.analog.com/en/design-center/reference-designs/circuits-from-the-lab/cn0540.html#rd-overview>`_

**Supported Carrier Boards:**

* `SDP-K1  <https://www.analog.com/en/resources/evaluation-hardware-and-software/evaluation-boards-kits/sdp-k1.html#eb-overview>`_

============
Introduction
============

CN0540 is a high resolution, wide-bandwidth, high dynamic range data acquisition system (DAQ) that interfaces with Integrated Circuit Piezo (ICP)/IEPE piezo vibration sensors. It incorporates a precision 24-bit, 1024kSPS Sigma-delta ADC AD7768-1 and a 16-bit voltage output DAC LTC2606. This page gives an overview of using the CN0540 firmware example with SDP-K1 EVAL board. The firmware example comprises 3 layers of software (from top to bottom): Console Application Layer, Device No-OS Layer and Platform Drivers layer.

   .. image:: /source/projects/cn0540_console/cn0540_software_architecture.jpg
      :width: 200

The application layer uses the ADI Console Libraries to create console-based User Interactive (UI). The middle layer of No-OS device library has device specific APIs to interface with CN0540 devices. These APIs allows direct access to device register map in order to read/write device registers. The bottom layer of Platform Drivers is responsible for Low Level Interface. The platform drivers use underlying libraries to access low level peripheral (like GPIOs, SPI, I2C, etc). AD7768-1 uses SPI and LTC2606 uses I2C communication.

The Platform simplifies the overall software development process by providing the low-level driver support. This reduces the hardware dependency as any STM32 board can be used with same firmware with little modifications (changing the pin mapping).

====================
Interface Diagram
====================

   .. image:: /source/projects/cn0540_console/cn0540_interface.jpg
      :width: 600

CN0540 is connected to SDP-K1 using the Arduino Headers. The SDP-K1 is connected to PC through a USB cable. The firmware (binary executable) can be loaded into SDP-K1 board through this USB interface from the PC. The SDP-K1 acts as a Serial Device (UART) and firmware loaded into it interacts with any serial terminal (like Teraterm, Putty, Coolterm, etc) by configuring terminal for proper serial settings (COM Port, Baud Rate, data bits, etc)

.. Useful links Section

.. include:: /source/useful_links_stm32.rst

====================
Hardware Connections
====================

The SDP-K1 is powered by the USB connection to a PC. The SDP-K1 appears as a USB serial device, and the host PC creates a serial or COM Port that can be connected to a Terminal software such as Teraterm, Putty, etc. The serial port assigned to a device can be found using the Device Manager for a Windows based OS.

   .. image:: /source/projects/cn0540_console/cn0540_connections.png
      :width: 600

.. Project Build Section:

.. include:: /source/build/project_build_stm32.rst

===========
Quick Start
===========

If you have some familiarity with STM32 platform, the following is a basic list of steps required to start running the code, see below for more detail:

* Connect the CN0540 EVAL-board to the SDP-K1 controller board.
* Connect the SDP-K1 controller board to your computer over USB.  (Make sure that the VIO_ADJUST is set to 3.3 volts)
* Go to the link provided above in the 'Build Guide' section and import code into Keil Studio Web IDE.
* Follow the steps mentioned in the Build Guide section above.
* Start up a serial terminal emulator (e.g., Tera Term)
* Find the com-port your controller board is connected on and select it.
* Set the baud-rate to 230400
* Reset the controller board and connect.
* Use the menu provided over the terminal window to access the evaluation board.

==================
Using the Firmware
==================

The CN0540 firmware example is configured to have following serial settings:

* Baud rate: 230400
* Data bits: 8-bits
* Parity: None
* Stop bits: 1

Configure your serial terminal with below settings:

   .. image:: /source/projects/cn0540_console/baud_rate_update.jpg
      :width: 600

The CN0540 Main Menu (with Tera Term):

   .. image:: /source/projects/cn0540_console/cn0540_teraterm.jpg
      :width: 300

The firmware is designed to be intuitive, and requires little explanation, simply enter the letter corresponding to the required command and follow the on-screen prompts.

The console menu application provides the following main features:

* Configure the ADC parameters such as power mode, filter type and output mode.
* Reset or power down the ADC
* Perform data capture
* Perform FFT of the data
* Set DAC output
* Perform offset calibration

It is hoped that the most of the features are coded, but it's likely that some special functionality is not implemented.

=======
Support
=======

Feel free to ask questions in the `EngineerZone <https://ez.analog.com/data_converters>`_