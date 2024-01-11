Contributing
------------

When contributing to Precision Converters Firmware please consider the following checklist:

    * Copyright header has been added to source and header files

    * Artistic Style (astyle) has been run to lint new code

    * Add EEPROM validation code

    * Add all context attributes (e.g. fw_version, hw_carrier, hw_mezzanine, hw_name, hw_mezzanine_status)

    * IIO attributes in the firmware match the corresponding Linux driver attributes

    * Create a readme.md in the project directory explaining how to use the project

    * Validate EEPROM detection and context attributes creation using IIO clients (e.g. ACE, IIO Oscilloscope)

    * Capture and verify ADC data using an IIO client

    * Test on the target platform with USE_SDRAM and NO_SDRAM where applicable

    * Test on the target platform with physical and virtual serial ports where applicable

    * Test using the STM32 HAL where applicable

    * Validate attributes, calibration, temperature sensing, etc using a client

    * Update corresponding ADI Wiki pages

    * Request links to those pages on the product page

    * Open a pull request on Github
    