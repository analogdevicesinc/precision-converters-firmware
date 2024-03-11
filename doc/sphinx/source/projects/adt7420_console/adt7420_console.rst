ADT7420 Console Application
---------------------------

==================
Supported Hardware
==================

**Supported Devices:**

* `ADT7420 <https://www.analog.com/en/products/adt7420.html>`_ 
* `ADT7320 <https://www.analog.com/en/products/adt7320.html>`_

**Supported Evaluation Boards:**

* `EV-TempSense-ARDZ <https://www.analog.com/en/design-center/evaluation-hardware-and-software/evaluation-boards-kits/EV-TempSense-ARDZ.html>`_ 

**Supported Carrier Boards:**

* `SDP-K1 With Mbed <https://os.mbed.com/platforms/SDP_K1/>`_

============
Introduction
============

The ADT74XX and ADT73XX are a family of high accuracy digital temperature sensors offering breakthrough performance over a wide industrial range. The EV-Tempsense-ARDZ Mbed support software can be used as a starting point for developing your own code for Analog Devices EV-TempSense-ARDZ board in your own environment utilizing the benefits of the Mbed platform. Analog Devices is an MBED Partner and develops code on the platform for multiple products.
The firmware example comprises 3 layers of software (from top to bottom): Console Application Layer, Device No-OS Layer and Platform Drivers (Mbed-OS) layer.

   .. image:: /source/projects/adt7420_console/adt7420_architecture.png
      :width: 200

The application layer uses the ADI Console Libraries to create console-based User Interactive (UI). The middle layer of No-OS device library has device specific APIs to interface with ADT7420/ADT7320 devices. These APIs allows direct access to device register map in order to read/write device registers. The bottom layer of Platform Drivers is responsible for Low Level Interface. The platform drivers use mbed-os libraries to access low level peripheral (like GPIOs, SPI, I2C, etc). The devices from ADT74XX and ADT73XX family use I2C and SPI communication interfaces.

This guide will focus on the Analog Devices SDP-K1 controller board, as it is directly compatible with the TempSense evaluation board and is an MBED-Enabled device. Customers are of course, not limited to using the SDP-K1 board for code development, given that any ARM-based, MBED-enabled board that satisfies a small set of requirements can use the provided code and it will work with only minor changes to the source (see below).

It is further assumed that SDP-K1 board will be connected to the appropriate ADT74XX/ADT73XX eval-board such as the EV-TempSense-ARDZ Evaluation board which has the ADT7320 (SPI) and ADT7420 (I2C) built in with the ability to connect external sensor via headers on the board.

=================
Interface Diagram
=================

   .. image:: /source/projects/adt7420_console/adt7420_interface_diagram.png
      :width: 600

The EV-TempSense-ARDZ board supports remote ADT74XX and ADT73XX devices. These can be connected to pins at the top of the eval board using ribbon cables or jumper wires, these pins are marked in the image below. SPI devices (EVAL-ADT7320-MBZ) and I2C devices (EV-ADT7420-MBZ) connect to their respective header pins by matching the pins on the board to those labelled on the remote board.

.. Useful links Section

.. include:: /source/useful_links.rst

====================
Hardware Connections
====================

Connecting the EV-TempSense evaluation board using the SDP connector on the K1 is the simplest and most convenient way to get up and running quickly, simply mate the two boards to together.

.. note::
   If using the Arduino header pins, compile the software only after uncommenting the #define ARDUINO in app_config.h
   
The EV-TempSense-ARDZ can be connected to the SDP-K1 by stacking the EV-TempSense-ARDZ ARDZ pins on top of the SDP-K1's Arduino header.

   .. image:: /source/projects/adt7420_console/adt7420_hardware_connections.png
      :width: 600

.. Project Build Section:
    
.. include:: /source/build/project_build.rst

===========
Quick Start
===========

If you have some familiarity with the Mbed platform, the following is a basic list of steps required to start running the code, 
see below for more detail:

* Connect the evaluation-board to the Mbed-enabled controller board using the Arduino connector or using the SDP-120 connector.
* Connect the controller board to your computer over USB. (Make sure that the VIO_ADJUST is set to 3.3 volts)
* Follow the steps mentioned in the Build Guide section above.
* Start up a serial terminal emulator (e.g. Tera Term)
   * Find the com-port your controller board is connected on and select it.
   * Set the baud-rate for 230400 - other defaults should be fine.
   * Reset the controller board and connect.
* Use the menu provided over the terminal window to access the evaluation board.

==================
Using the Firmware
==================

The firmware is delivered as a basic, text-based user-interface that operates through a UART on the controller board using the same USB cable that is used to flash the firmware to the boards. Any terminal-emulator should work, but it is not possible for Analog Devices to test everyone. It is necessary to connect a serial terminal-emulator to interact with the running firmware.

Here CoolTerm is used as an example, Analog Devices does not endorse any particular program for this, but CoolTerm works well and is made freely available, other terminals such as TeraTerm, or PuTTY will work.

Set the baud-rate for 115200, configure the console terminal settings as shown in the picture above and select the connected controller board’s COM port. If using CoolTerm, you should be able to keep the defaults, however adjustments may need to be made to how carriage return (CR) is handled in order for everything to display correctly.

The ADT7420 console main menu looks like below (with CoolTerm):

   .. image:: /source/projects/adt7420_console/adt7420_console_menu.png
      :width: 600

The software is designed to be straight forward to use, and requires little explanation. Simply select which interface (SPI/I2C) you would like to use, whether you want to use the internal sensor or a remote one and then simply enter a number corresponding to the required command and follow the on-screen prompts. The code is also written with a view to keeping things simple, you do not have to be a coding-ninja to understand and expand upon the delivered functions.

=======
Support
=======

Feel free to ask questions in the `EngineerZone <https://ez.analog.com/data_converters>`_