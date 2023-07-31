# Default platform is set as Mbed
PLATFORM = mbed

# IIO client (REMOTE/LOCAL)
IIO_CLIENT = REMOTE

ROOT_DRIVE = $(realpath ../..)
LIBRARIES_PATH = $(ROOT_DRIVE)/libs
NO-OS = $(LIBRARIES_PATH)/no-OS
PROJECT_APP_PATH = $(realpath .)/app
MBED_OS_DIRECTORY_PATH = $(NO-OS)/libraries/mbed

# Get all .lib files in libraries directory
LIB_FILES_IN_LIBRARIES_FOLDER = $(wildcard $(LIBRARIES_PATH)/*.lib)
MBED_OS_LIB_FILE_PATH = $(ROOT_DRIVE)/mbed-os.lib
STM32F769_DISCO_BSP_LIB_FILE_PATH = $(LIBRARIES_PATH)/stm32_lvgl/BSP_DISCO_F769NI.lib
STM32F769_DISCO_LVGL_LIB_FILE_PATH = $(LIBRARIES_PATH)/stm32_lvgl/lv_port_stm32f769_disco.lib

ifeq ($(OS),Windows_NT)
LIB_FILES_IN_LIBRARIES_FOLDER := $(subst /,\,$(sort $(LIB_FILES_IN_LIBRARIES_FOLDER)))
read_a_file = type $(subst /,\,$1) 2> NUL
command_separator = & 
HIDE_OUTPUT = > nul
else
read_a_file = cat $1 2> /dev/null
HIDE_OUTPUT = > /dev/null
command_separator = ;
endif

MUTE_COMMAND = @
HASH_SYMBOL = \#
REPOSITORY_URL = $(firstword $(subst /$(HASH_SYMBOL), ,$(shell $(call read_a_file ,$1))))
LIB_FILE_FOLDER_NAME = $(basename $(lastword $(subst /, ,$(call REPOSITORY_URL,$1))))
REPOSITORY_COMMIT_ID = $(lastword $(subst /$(HASH_SYMBOL), ,$(shell $(call read_a_file ,$1))))

# Rule to clone dependance repositories in libraries folder
PHONY_TARGET += clone-lib-repos
clone-lib-repos :
	$(MUTE_COMMAND) $(foreach lib-file,$(LIB_FILES_IN_LIBRARIES_FOLDER),$(if $(wildcard $(LIBRARIES_PATH)/$(call LIB_FILE_FOLDER_NAME,$(lib-file))/.*),echo . $(HIDE_OUTPUT),$(if $(findstring https://os.mbed.com/,$(call REPOSITORY_URL,$(lib-file))),cd $(LIBRARIES_PATH) $(command_separator) hg clone $(call REPOSITORY_URL,$(lib-file)),cd $(LIBRARIES_PATH) $(command_separator) git clone $(call REPOSITORY_URL,$(lib-file)))) $(command_separator) ) echo . $(HIDE_OUTPUT)
	$(MUTE_COMMAND) $(foreach lib-file,$(LIB_FILES_IN_LIBRARIES_FOLDER),$(if $(findstring https://os.mbed.com/,$(call REPOSITORY_URL,$(lib-file))),cd $(LIBRARIES_PATH)/$(call LIB_FILE_FOLDER_NAME,$(lib-file)) $(command_separator) hg checkout $(call REPOSITORY_COMMIT_ID,$(lib-file)),cd $(LIBRARIES_PATH)/$(call LIB_FILE_FOLDER_NAME,$(lib-file)) $(command_separator) git checkout $(call REPOSITORY_COMMIT_ID,$(lib-file))) $(command_separator)) echo . $(HIDE_OUTPUT)
	$(if $(wildcard $(MBED_OS_DIRECTORY_PATH)/mbed-os/.git),echo .,cd $(MBED_OS_DIRECTORY_PATH) $(command_separator) git clone $(call REPOSITORY_URL,$(MBED_OS_LIB_FILE_PATH))) $(HIDE_OUTPUT)
	cd $(MBED_OS_DIRECTORY_PATH)/mbed-os $(command_separator) git checkout $(call REPOSITORY_COMMIT_ID,$(MBED_OS_LIB_FILE_PATH))

ifeq ($(TARGET_BOARD), DISCO_F769NI)
ifeq ($(IIO_CLIENT), LOCAL)
	cd $(LIBRARIES_PATH)/stm32_lvgl $(command_separator) hg clone https://os.mbed.com/teams/ST/code/BSP_DISCO_F769NI $(command_separator) cd BSP_DISCO_F769NI $(command_separator) hg checkout $(call REPOSITORY_COMMIT_ID,$(STM32F769_DISCO_BSP_LIB_FILE_PATH)) $(HIDE_OUTPUT)
	cd $(LIBRARIES_PATH)/stm32_lvgl $(command_separator) git clone https://github.com/lvgl/lv_port_stm32f769_disco $(command_separator) cd lv_port_stm32f769_disco $(command_separator) git checkout $(call REPOSITORY_COMMIT_ID,$(STM32F769_DISCO_LVGL_LIB_FILE_PATH)) $(HIDE_OUTPUT)
endif
endif
