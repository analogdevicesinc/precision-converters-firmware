################## For Mbed Platform these are Default but Can Change by Giving These Variable While Giving make Command #########################
TARGET_BOARD = SDP_K1
COMPILER = GCC_ARM

################## Build Directory for Mbed Platform ###########
BUILD_DIRECTORY = $(PROJECT_PATH)/build/$(TARGET_BOARD)/$(COMPILER)

############## Mbed-OS Related Paths and Files #########################
MBED_OS_DIR = $(ROOT_DIR_PATH)/mbed-os
MBED_BUILD_DIR = $(ROOT_DIR_PATH)/BUILD/$(TARGET_BOARD)/$(COMPILER)
MBED_STATIC_LIBRARY = $(MBED_BUILD_DIR)/libmbed-os.a
MBED_APP_JSON = $(ROOT_DIR_PATH)/mbed_app_json

############# Finding Linker-Scripts #######################
LINKER_SCRIPT_ROOT_DIR = $(MBED_BUILD_DIR)
FIND_LINKER_SCRIPT = $(call recursive_wildcard,$(LINKER_SCRIPT_ROOT_DIR),*.ld)
LSCRIPT_BEFORE_PREPROCESSING = $(sort $(FIND_LINKER_SCRIPT))

################### Reading Flags and Object Files and Include files ###########################
PROFILE_C_FILE = $(MBED_BUILD_DIR)/.profile-c
PROFILE_CPP_FILE = $(MBED_BUILD_DIR)/.profile-cxx
PROFILE_LINKER_FILE = $(MBED_BUILD_DIR)/.profile-ld
PROFILE_ASM_FILE = $(MBED_BUILD_DIR)/.profile-asm
GENERATED_INCLUDE_FILE = $(MBED_BUILD_DIR)/.includes_*
GENERATED_ARCHIVE_FILE = $(MBED_BUILD_DIR)/.archive_files.txt

READ_PROFILE_C_FILE = $(shell $(call read_a_file ,$(PROFILE_C_FILE)))
READ_PROFILE_CPP_FILE = $(shell $(call read_a_file ,$(PROFILE_CPP_FILE)))
READ_PROFILE_ASM_FILE = $(shell $(call read_a_file ,$(PROFILE_ASM_FILE)))
READ_PROFILE_LINKER_FILE = $(shell $(call read_a_file ,$(PROFILE_LINKER_FILE)))
READ_GENERATED_INCLUDE_FILE = $(shell $(call read_a_file ,$(GENERATED_INCLUDE_FILE)))
READ_GENERATED_ARCHIVE_FILE = $(shell $(call read_a_file ,$(GENERATED_ARCHIVE_FILE)))

NULL := 
COMMA :=,
SPACE := $(NULL) $(NULL)

READ_C_FLAGS = $(subst ",,$(subst $(COMMA), ,$(patsubst {"flags":[%,%,$(firstword $(subst ]$(COMMA)"macros", "macros",$(subst $(SPACE),,$(READ_PROFILE_C_FILE)))))))
READ_C_SYMBOLS = $(subst ",,$(subst $(COMMA), ,$(patsubst %]},%,$(lastword $(subst "symbols":[,"symbols":[ ,$(subst $(SPACE),,$(READ_PROFILE_C_FILE)))))))
C_SYMBOLS_WITH_D = $(patsubst %,-D%,$(READ_C_SYMBOLS))
C_FLAGS += $(READ_C_FLAGS)
C_FLAGS += $(C_SYMBOLS_WITH_D)
C_FLAGS += -include $(MBED_BUILD_DIR)/mbed_config.h
C_FLAGS += $(TEST_FLAGS)

READ_CPP_FLAGS = $(subst ",,$(subst $(COMMA), ,$(patsubst {"flags":[%,%,$(firstword $(subst ]$(COMMA)"macros", "macros",$(subst $(SPACE),,$(READ_PROFILE_CPP_FILE)))))))
READ_CPP_SYMBOLS = $(subst ",,$(subst $(COMMA), ,$(patsubst %]},%,$(lastword $(subst "symbols":[,"symbols":[ ,$(subst $(SPACE),,$(READ_PROFILE_CPP_FILE)))))))
CPP_SYMBOLS_WITH_D = $(patsubst %,-D%,$(READ_CPP_SYMBOLS))
CPP_FLAGS += $(READ_CPP_FLAGS)
CPP_FLAGS += $(CPP_SYMBOLS_WITH_D)
CPP_FLAGS += -include $(MBED_BUILD_DIR)/mbed_config.h
CPP_FLAGS += $(TEST_FLAGS)

READ_ASM_FLAGS = $(subst ",,$(subst $(COMMA), ,$(patsubst {"flags":[%,%,$(firstword $(subst ]$(COMMA)"macros", "macros",$(subst $(SPACE),,$(READ_PROFILE_ASM_FILE)))))))
READ_ASM_SYMBOLS = $(subst ",,$(subst $(COMMA), ,$(patsubst %]},%,$(lastword $(subst "symbols":[,"symbols":[ ,$(subst $(SPACE),,$(READ_PROFILE_ASM_FILE)))))))
ASM_SYMBOLS_WITH_D = $(patsubst %,-D%,$(READ_ASM_SYMBOLS))
ASM_FLAGS += $(READ_ASM_FLAGS)
ASM_FLAGS += $(ASM_SYMBOLS_WITH_D)
ASM_FLAGS += -include $(MBED_BUILD_DIR)/mbed_config.h
ASM_FLAGS += $(TEST_FLAGS)

READ_LINKER_FLAGS = $(subst ",,$(subst "$(COMMA)", ,$(patsubst {"flags":[%,%,$(firstword $(subst ]$(COMMA)"macros", "macros",$(subst $(SPACE),,$(READ_PROFILE_LINKER_FILE)))))))
READ_LINKER_SYMBOLS = $(subst ",,$(subst $(COMMA), ,$(patsubst %]},%,$(lastword $(subst "symbols":[,"symbols":[ ,$(subst $(SPACE),,$(READ_PROFILE_LINKER_FILE)))))))
LINKER_SYMBOLS_WITH_D = $(patsubst %,-D%,$(READ_LINKER_SYMBOLS))
LD_FLAGS += $(READ_LINKER_FLAGS)

##### Preprocessor Flags for Linker Script #########
PREPROCESSOR_FLAGS = arm-none-eabi-cpp -E -P $(LD_FLAGS)

##### Map file option #####
LD_FLAGS += $(LINKER_SYMBOLS_WITH_D) -Wl,-Map=$(PROJECT_MAP_FILE)

PLATFORM_INCLUDE_PATHS_WITH_I = $(patsubst %,-I%,$(patsubst mbed-os%,$(MBED_OS_DIR)%,$(patsubst -I%,%,$(subst ",,$(READ_GENERATED_INCLUDE_FILE)))))
MBED_OBJECT_FILES = $(foreach d,$(READ_GENERATED_ARCHIVE_FILE),$(addprefix $(ROOT_DIR_PATH)/,$d))
####################################################################################

#################### Compiler and Standard library ###########################
LINKER_LIBRARIES = -Wl,--start-group -lstdc++ -lsupc++ -lm -lc -lgcc -lnosys  -Wl,--end-group
CC = arm-none-eabi-gcc
CPP = arm-none-eabi-g++
OC = arm-none-eabi-objcopy

################## Reading Mbed-OS Commit-ID ######################
MBED_LIB_FILE = $(ROOT_DIR_PATH)/mbed-os.lib
ifeq ($(OS),Windows_NT)
MBED_LIB_FILE := $(subst /,\,$(sort $(MBED_LIB_FILE)))
endif

################## Project Build Files ############################
PROJECT_LINKER_SCRIPT = $(BUILD_DIRECTORY)/$(NAME_OF_PROJECT)-linker-file.ld

################# Directories Removed During Clean #############
DIRECTORY_TO_REMOVE += $(BUILD_DIRECTORY)
DIRECTORY_TO_REMOVE += $(MBED_BUILD_DIR)
DIRECTORY_TO_REMOVE += $(ROOT_DIR_PATH)/mbed_app_json

################## Rule for Building Mbed-OS and Copying mbed_app.json File #######################
$(PROJECT_TARGET_FILE):
	-$(MUTE_COMMAND) $(call make_dir,$(MBED_APP_JSON)) $(HIDE_OUTPUT)
	$(MUTE_COMMAND) $(call copy_a_file,$(ROOT_DIR_PATH)/mbed_app.json,$(MBED_APP_JSON)/) $(HIDE_OUTPUT)
	$(MUTE_COMMAND) $(MAKE) --no-print-directory MBED-OS-BUILD
	-$(MUTE_COMMAND) $(call make_dir,$(BUILD_DIRECTORY))
	$(MUTE_COMMAND) $(call SET_RULE,$(PROJECT_TARGET_FILE))

$(MBED_STATIC_LIBRARY):
	$(MUTE_COMMAND) $(MAKE) --no-print-directory MBED-OS-BUILD

PHONY_TARGET += MBED-OS-BUILD 
MBED-OS-BUILD:
	$(if $(wildcard $(ROOT_DIR_PATH)/mbed-os/.git),echo .,cd $(ROOT_DIR_PATH) $(command_separator) git clone $(call URL,$(MBED_LIB_FILE))) $(HIDE_OUTPUT)
	cd $(ROOT_DIR_PATH)/mbed-os $(command_separator) git checkout $(call COMMIT_ID,$(MBED_LIB_FILE))
	cd $(ROOT_DIR_PATH) $(command_separator) mbed config root . $(command_separator) mbed compile --source $(ROOT_DIR_PATH)/mbed-os --source $(ROOT_DIR_PATH)/mbed_app_json -m $(TARGET_BOARD) -t $(COMPILER) --build $(MBED_BUILD_DIR) --library

#################### Linker-Script Preprocessing #####################
$(PROJECT_LINKER_SCRIPT): $(LSCRIPT_BEFORE_PREPROCESSING)
	$(MUTE_COMMAND) $(PREPROCESSOR_FLAGS) $< -o $@

PHONY_TARGET += project-build
project-build: $(PROJECT_BINARY_FILE)

PHONY_TARGET += project-build-test
project-build-test: $(PROJECT_BINARY_FILE_FOR_TEST)