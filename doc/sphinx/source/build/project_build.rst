===========
Build Guide
===========

.. collapse:: Build Prerequisites
   :open:

      Prior to building a firmware project, it is required to set up an environment so 
      that the build process may find the necessary tools (compiler, linker, SDK etc.). 
      Use the following steps to prepare your environment for building firmware projects 
      for respective platform.

      .. collapse:: Windows

         .. collapse:: Mbed

            .. collapse:: For Web/Online Build

                  * Visit `Arm Keil website <https://www.keil.arm.com/>`_ to create an user account for accessing the web based Keil Studio IDE.

                  * Open `Keil Studio Web IDE <https://studio.keil.arm.com/auth/login/>`_ with registered user account

            .. collapse:: For Make Build

                  * Clone Precision Converters Firmware repository with the --recursive flag (not needed if building with web IDE for Mbed platform):
                  
                  .. code-block:: bash

                        git clone --recursive https://github.com/analogdevicesinc/precision-converters-firmware

                  * If however you've already cloned the repository without the --recursive flag, you may initialize all the submodules in an existing cloned repo with:
                     
                  .. code-block:: bash

                        git submodule update --recursive --init

                  * Install `Make <https://gnuwin32.sourceforge.net/packages/make.htm>`_ in the root of 'C' drive without any spaces in the installation path. The path must be **C:\\GnuWin32\\…**. Add this path into the system environmental path variable (as shown in below screenshot).
                  
                  * Install `Git <https://git-scm.com/downloads>`_ and add a path of **C:\\Program Files\\Git\\usr\\bin\\** directory into system environmental path variable (please verify your git installation path is correct).
                  
                  * Install Mbed CLI 1 as per guide `here <https://os.mbed.com/docs/mbed-os/v6.15/build-tools/install-and-set-up.html>`_.
                  
                  * Install `GNU Arm Embedded compiler <https://developer.arm.com/downloads/-/gnu-rm.>`_ (for the development, 9-2019-q4-major version is used) and add a path of GNU Arm Embedded Toolchain bin directory into the system environmental path variable (as shown in below screenshot).
                  
                  * Configure the compiler location with Mbed CLI. This can be carried out by running the **mbed config -G GCC_ARM_PATH <path-to-your-gcc-compiler>** in Command Prompt. For example you can run **mbed config -G GCC_ARM_PATH "C:\\Program Files (x86)\\GNU Tools ARM Embedded\\9 2019-q4-major\\bin"** in command prompt. It will set mentioned compiler path to all the Mbed Projects.

                     .. image:: /source/build/environmental_variables.png
                        :width: 400
         

         .. collapse:: STM32

            .. collapse:: For STM32 IDE based Build
                  
                  * Install `STM32CubeIDE <https://www.st.com/en/development-tools/stm32cubeide.html>`_

                  * Install `STM32CubeMX <https://www.st.com/en/development-tools/stm32cubemx.html>`_

                  * Install `Git <https://git-scm.com/downloads>`_ and add a path of **C:\\Program Files\\Git\\usr\\bin\\** directory into system environmental path variable (please verify your git installation path is correct).

                  * Install `Make <https://gnuwin32.sourceforge.net/packages/make.htm>`_ in the root of 'C' drive without any spaces in the installation path. The path must be **C:\\GnuWin32\\…**. Add this path into the system environmental path variable (as shown in below screenshot).

                  * Install `GNU Arm Embedded compiler <https://developer.arm.com/downloads/-/gnu-rm.>`_ (for the development, 9-2019-q4-major version is used) and add a path of GNU Arm Embedded Toolchain bin directory into the system environmental path variable (as shown in below screenshot).

                     .. image:: /source/build/environmental_variables.png
                        :width: 400


.. collapse:: Building a project
   :open:

      Once the build enviornment is setup, follow the guide below to build your project
      and generate executable file (.bin/.hex)

      .. collapse:: Windows

         
         .. collapse:: Mbed

            .. collapse:: Build Using Web/Online IDE
               
                  * Clone the `Precision Converters Firmware repository <https://github.com/analogdevicesinc/precision-converters-firmware>`_ into Keil Studio using "File->clone…" menu.

                     .. image:: /source/build/mbed_keil_studio_project_clone.png
                        :width: 600

                  * Once the project repository is imported, wait until all library dependencies are imported as shown in below screenshot. Now, open the '.medignore' file present in the root directory of repository. Add comment syntax (two forward slashes) in front of the project name which you want to build. This will ignore all other projects and build only the comment syntax selected project.

                     .. image:: /source/build/mbedignore_changes.png
                        :width: 600

                  * Select the target device (default used for development is SDP-K1) and click on 'Clean build' option to build the project. After a successful build a binary will be downloaded to your computer- store this on your drive. Drag and drop this binary to the USB drive hosted by your controller board to flash the MCU.

                     .. image:: /source/build/mbed_build.png
                        :width: 400

                  .. note::

                        If you intend to build different project, then modify the .medignore file in the root directory as 
                        mentioned in step2 and clean build project as mentioned in step3


            .. collapse:: Build Using Make

                  * Open Git bash and change current directory to project directory (eg. **precision-converters-firmware/projects/ad4130_iio** directory) which you want to build.

                  * Type **make** on the git bash command prompt to build a project.

                  * After successful build, binary file will be created into the **Project_Name/build** directory.

                  * If you want to clean build, type **make reset** on git bash command which deletes all generated build files for the given project.

                  .. note::

                        Default TARGET_BOARD is SDP_K1 and COMPILER is GCC_ARM. Current Make based build only support GCC_ARM Compiler.

                  * By default project is built for "SDP_K1" Board and "GCC_ARM" Compiler. If you want to build for other Mbed Board, For example If you want to build the project for "DISCO_F769NI" Board then run **make TARGET_BOARD=DISCO_F769NI** command in git bash command prompt. If you want to clean build, run **make reset TARGET_BOARD=DISCO_F769NI** command to delete the generated build files for the given project.


         .. collapse:: STM32

            .. collapse:: Build Using STM32 IDE

                  * Open the respective project directory by navigating into the "precision_converters_firmware/projects/" folder.
                  
                  * In the "STM32" folder present within the project directory, double click and open the .ioc file present within.

                     .. image:: /source/build/stm32_project_directory.png
                        :width: 600

                  * Click on the "Generate Code" option seen on the top right corner

                     .. image:: /source/build/stm32_generate_code.png
                        :width: 600

                  * Upon successful generation of drivers for the selected MCU, the autogenerated files would be seen in the same directory where the .ioc file was present. Double click and open the ".project" file seen in the list of files

                     .. image:: /source/build/stm32_auto_generated_files.png
                        :width: 600

                  * After the project is loaded to the STM32CubeIDE, unfold the adxxxx_iio project seen in the project explorer, right click the "app" folder, select "Settings" under the "C/C++ Build" section on the left pane and un-check the "Exclude resources from build" checkbox. This would ensure that the project specific files are included by the build system

                     .. image:: /source/build/stm32_exclude_files_option.png
                        :width: 600

                  * In order to choose STM32 platform in the firmware, select the "ACTIVE_PLATFORM" as "STM32_PLATFORM" in the app_config.h from the respective project.

                  * Add compiler flags "-u _printf_float" to the project settings.

                     .. image:: /source/build/stm32_compiler_flag.png
                        :width: 600

                  * Program by clicking on the "Run adxxxx_iio" option seen or by performing a copy->paste option of the .hex file seen in the STM32/Debug folder

                     .. image:: /source/build/stm32_run_option.png
                        :width: 600



.. collapse::  Running a project
   :open:

      Once the firmware build is successful and binary file is generated, copy the 
      generated binary into USB drive hosted by your MCU board (e.g. USB drive hosted 
      by SDP-K1 board on windows). This will flash the binary file into MCU present 
      on the controller board. Programming might vary based on the tools used for building
      a project. The 'Project Build' section above talks about this exception at the end
      of all build steps.
