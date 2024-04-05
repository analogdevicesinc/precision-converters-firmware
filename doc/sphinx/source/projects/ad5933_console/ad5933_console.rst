AD5933 Console Application
""""""""""""""""""""""""""

==================
Supported Hardware
==================

**Supported Devices:**

* `AD5933 <https://www.analog.com/en/products/ad5933.html>`_

**Supported Evaluation Boards:**

* `Pmod IA <https://digilent.com/reference/pmod/pmodia/start>`_

**Supported Carrier Boards:**

* `SDP-K1 With Mbed <https://os.mbed.com/platforms/SDP_K1/>`_

============
Introduction
============

The AD5933 Mbed support software (also supports AD5934) can be used as a starting point for developing your own code for Analog Devices AD5933 products in your own environment utilizing the benefits of the Mbed platform. Analog Devices is an MBED Partner and develops code on the platform for multiple products. The Analog Devices Mbed code-repo can be found in the links below.

This guide will focus on the Analog Devices SDP-K1 controller board, as it is directly compatible with the AD5933 family of evaluation boards and is an MBED-Enabled device. Customers are of course, not limited to using the SDP-K1 board for code development, given that any ARM-based, MBED-enabled board that satisfies a small set of requirements can use the provided code and it will work with only minor changes to the source (see below).

This guide uses the Pmod IA evaluation board. This is a convenient, inexpensive path to evaluating the AD5933.

.. Useful links Section

.. include:: /source/useful_links.rst

====================
Hardware Connections
====================

The SDP-K1 board has two ways to connect to most ADI evaluation boards, it can use the 120-pin SDP connector on the underside of the board, or the Arduino connector can be used together with jumper wires as described below. Currently an ADI evaluation board with an SDP connector does not exist for the AD5933. As such, it is necessary to connect to the Arduino headers using short jumper wires.

The Getting Started with Mbed page describes the Arduino Uno Header, the SDP connector, pinouts and other information related to understanding the SDP-K1 controller board.

The SDP-K1 can operate with the 120-pin SDP connector of the evaluation board supports it, or, as in this case, it can also use the Arduino header pins (or indeed any available I2C port on the controller board) using wires to the evaluation board. This is shown here for the SDP-K1 board connected to the Digilent Pmod IA evaluation board using the Arduino Header, but different boards might have their SPI/I2C/GPIO ports exposed differently. The pins on the Arduino header must be shorted to the evaluation board as follows. The pin mappings for these are controlled in the app_config.h file and should match your controller board.

+-----------------------+-----------------+--------------+
|Arduino PIN            | MBED NAME       | Pmod IA PIN  |
+-----------------------+-----------------+--------------+
|D15                    | I2C_SCL         | SCLK/A0      |
+-----------------------+-----------------+--------------+
|D14                    | I2C_SDA         | SDO/SDA      |
+-----------------------+-----------------+--------------+

.. note::
    If using the Arduino header pins, compile the software only after adding the #define ARDUINO to app_config.h (set by default)

.. image:: /source/projects/ad5933_console/ad5933_hardware_connections.jpg
   :width: 500

.. note::
    One thing to note here is that power and ground for the evaluation need to be provided and can be conveniently taken from the Arduino header as shown above. If using a different evaluation board to the DIGILENT PMOD IA, then consult the relevant evaluation board guides available through the product-page for your selected board.

====================
AD5933 Mbed Firmware
====================

This guide focuses on the SDP-K1, connected to the Pmod IA evaluation board, but it should be general enough to cover any compatible controller board (the controller board should be Mbed-enabled, and expose I2C and some GPIO's).

The software described below allows for an Mbed enabled controller board to be connected with the Pmod IA. Unmodified, the code will communicate over any serial terminal emulator (CoolTerm, putty, etc) using the UART provided by the controller board over USB.

.. Project Build Section:
    
.. include:: /source/build/project_build.rst

===========
Quick Start
===========

If you have some familiarity with the Mbed platform, the following is a basic list of steps required to start running the code, 
see below for more detail:

* Connect the evaluation-board to the Mbed-enabled controller board.
* Connect the controller board to your computer over USB.
* Follow the steps mentioned in the Build Guide section above (Edit app_config.h file (defaults to SDP connector and AD5686R device) if evaluating any other device).
* Start up a serial terminal emulator (e.g. Tera Term)
   * Find the com-port your controller board is connected on and select it.
   * Set the baud-rate for 230400
   * Reset the controller board and connect.
* Use the menu provided over the terminal window to access the evaluation board.

==================
Using the Firmware
==================

The firmware is delivered as a basic, text-based user-interface that operates through a UART on the controller board using the same USB cable that is used to flash the firmware to the boards. Any terminal-emulator should work, but it is not possible for Analog Devices to test everyone. It is necessary to connect a serial terminal-emulator to interact with the running firmware.

Here CoolTerm is used as an example, Analog Devices does not endorse any particular program for this, but CoolTerm works well and is made freely available, other terminals such as Tera Term, or PuTTY will work just as well. Set the baud-rate for 115200 and keep the defaults for everything else. The actual values used can be found by looking at the source code in main.cpp

   .. image:: /source/projects/ad5933_console/ad5933_coolterm_menu.jpg
      :width: 400

The software is provided as a demo. The demo covers the essential operation of the AD5933 and it is hoped to be a good starting point for developing your own firmware. The code is also written with a view to keeping things simple, you do not have to be a coding-ninja to understand and expand upon the delivered functions.

It is hoped that the most common functions of the AD5933 family are coded, but it's likely that some special functionality is not implemented.

The software comes with an app_config.h file which allows the pins for the I2C interface to be selected.

* Configure the pins you want to use to connect the controller board to the evaluation board.
* They default to the I2C exposed on the Arduino header.

===========
AD5933 DEMO
===========

This demo will keep things simple by only using resistances. The AD5933 operates is a ratiometric device and because of this it requires a calibration gain-factor to be calculated. This demo will use a 200KΩ calibration and will test the operation of the impedance calculation with a different resistance (300KΩ is arbitrarily chosen).

.. note::
    Use Command option 1 to read the temperature from the AD5933. This ensures basic connectivity is established. The firmware does a temperature read following a board reset.

Step 1: Configure System

1. Place a 200KΩ resistance between the 2 SMB connectors on the PMOD IA
2. Select Option 2 and provide the data will prompted, example for 200KΩ is done here.

   1. Select option 3: 1Vpp typical (to ensure amplifiers are not saturated)
   2. Select PGA gain of X1
   3. Select Internal Clock
   4. Enter start frequency of 10Khz (this is arbitrary, as we are using only resistances for the demo)
   5. Enter frequency increment of 10 (again arbitrary)
   6. Give the number of increments (20 for example)
   7. Let the number of settling sample = 5
   8. Settling-time multiplier = X1
3. The software will report the values chosen - this configuration only has to be done once, the values are stored, both in the software and on the AD5933.

Step 2: Calculate the Gain Factor
The gain factor is the calibration for the signal path and only needs to be set once.

1. Select option 3 from the main-menu
2. Enter your calibration resistance, in Ohms - e.g 200000
3. The calculated gain-factor will be returned and stored in software and on-chip

Step 3: Calculate unknown impedance
Any impedance can now be placed between the SMB connectors on the PMOD IA board and option 4 from the main-menu will perform a sweep according to the settings configured in Step 1. The results will be displayed on the terminal. For this demo the 200KΩ was replaced with a 300KΩ and an impedance sweep performed. It returned the results shown below. Consult the extensive documentation available on the product page to help understand the process    

   .. image:: /source/projects/ad5933_console/ad5933_demo.jpg
      :width: 400

=======
Support
=======

Feel free to ask questions in the `EngineerZone <https://ez.analog.com/data_converters>`_
