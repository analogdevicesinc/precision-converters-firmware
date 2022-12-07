####################### Utility Functions #####################
####################### For	Windows ################
ifeq ($(OS), Windows_NT)
SHELL = cmd
print_line = @echo $1
make_dir = mkdir $(subst /,\,$1)
remove_file_recursively = -del /S /Q $(subst /,\,$1)
remove_file_in_folder = del /Q $(subst /,\,$1)
remove_dir_recursively = rd /S /Q $(addsuffix ",$(addprefix ",$(subst /,\,$1)))
link_a_file = mklink "$(strip $(subst /,\,$2))" "$(strip $(subst /,\,$1))"
copy_a_file = xcopy /F /Y /B "$(subst /,\,$1)" "$(subst /,\,$2)"
rename_file = ren "$(subst /,\,$1)" "$(subst /,\,$2)"
read_a_file = type $(subst /,\,$1) 2> NUL
command_separator = &
slash_change = $(subst /,\,$1)
move_file = move /Y "$(subst /,\,$1)" "$(subst /,\,$2)"
hide_output = > nul

###################### For Linux #####################
else
print_line = @printf "$1\n"
make_dir = mkdir -p $1
remove_file_recursively = rm -rf $1
remove_file_in_folder = rm -f $1
remove_dir_recursively = rm -rf $1
link_a_file = ln -sf $1 $2
copy_a_file = cp $1 $2
rename_file = mv $1 $2
read_a_file = cat $1 2> /dev/null
command_separator = ;
slash_change = $1
move_file = mv $1 $2
hide_output = > /dev/null
endif

verbose ?= 0
export verbose

ifeq ($(strip $(verbose)),0)
HIDE_OUTPUT = $(hide_output)
MUTE_COMMAND = @
endif

ifeq ($(strip $(verbose)),2)
HIDE_OUTPUT = $(hide_output)
endif

############ This will Give the File Names of Specific Pattern(eg .c Files) in the Folder #######################
recursive_wildcard = $(foreach d,$(wildcard $(1:=/*)),$(call recursive_wildcard,$d,$2) $(filter $(subst *,%,$2),$d))
find_files_in_dir = $(wildcard $1/$2) 

########### Creates File with the Given Name ################################
SET_RULE = echo Target file. Do not delete > $1

###################### Getting Relative and Full Paths of Application Files #################
RELATIVE_PATH_OF_FILE = $(patsubst $(ROOT_DIR_PATH)/libraries%,libraries%,$(patsubst $(PROJECT_PATH)/app%,app%,$1))
FULL_PATH_OF_FILE = $(patsubst libraries%,$(ROOT_DIR_PATH)/libraries%,$(patsubst app%,$(PROJECT_PATH)/app%,$1))

######################## Source files and Include Paths of Application ###############################
SOURCE_FILES += $(foreach dir, $(SOURCE_DIRS), $(call find_files_in_dir, $(dir),*.c ))
SOURCE_FILES += $(foreach dir, $(SOURCE_DIRS), $(call find_files_in_dir, $(dir),*.cpp))
SOURCE_FILES := $(filter-out $(EXCLUDE_SOURCE_FILES), $(SOURCE_FILES))
SOURCE_INCLUDE_PATHS_WITH_I = $(addprefix -I ,$(SOURCE_INCLUDE_PATHS))

####################### Binary and Elf Files Names ############################
PROJECT_ELF_FILE = $(BUILD_DIRECTORY)/$(NAME_OF_PROJECT).elf
PROJECT_ELF_FILE_FOR_TEST = $(BUILD_DIRECTORY)/$(ARTIFACT-NAME).elf
PROJECT_BINARY_FILE = $(BUILD_DIRECTORY)/$(NAME_OF_PROJECT).bin
PROJECT_BINARY_FILE_FOR_TEST = $(BUILD_DIRECTORY)/$(ARTIFACT-NAME).bin
PROJECT_MAP_FILE = $(BUILD_DIRECTORY)/$(NAME_OF_PROJECT).map

################### Checking platform ###########################
ifeq 'mbed' '$(PLATFORM_NAME)'
include $(ROOT_DIR_PATH)/tools/scripts/mbed.mk
endif

ifeq 'stm32' '$(PLATFORM_NAME)'
include $(ROOT_DIR_PATH)/tools/scripts/stm32.mk
endif
###################### Reading .lib files #########################
APP_LIB_FILE = $(call recursive_wildcard,$(PROJECT_APP),*.lib)
LIBRARY_LIB_FILE = $(call recursive_wildcard,$(LIBRARIES_PATH),*.lib)

ifeq ($(OS),Windows_NT)
APP_LIB_FILE := $(subst /,\,$(sort $(APP_LIB_FILE)))
LIBRARY_LIB_FILE := $(subst /,\,$(sort $(LIBRARY_LIB_FILE)))
endif

HASH = \#
URL = $(firstword $(subst /$(HASH), ,$(shell $(call read_a_file ,$1))))
LIB_FILE_FOLDER_NAME = $(basename $(lastword $(subst /, ,$(call URL,$1))))
COMMIT_ID = $(lastword $(subst /$(HASH), ,$(shell $(call read_a_file ,$1))))

###################### Objects files name of application Files #################
OBJECT_FILES = $(patsubst %.cpp,%.o,$(patsubst %.c,%.o,$(SOURCE_FILES)))

##################### Objects Files Relative to Build Directory #############################
OBJECTS_FILES_IN_BUILD = $(addprefix $(BUILD_DIRECTORY)/,$(call RELATIVE_PATH_OF_FILE,$(OBJECT_FILES)))

##################### Relative Path of Object Files with Respect to Root Directory ##################
RELATIVE_OBJECTS_FILES_IN_BUILD = $(patsubst $(ROOT_DIR_PATH)/%,%,$(OBJECTS_FILES_IN_BUILD))

######################## All Include Paths ####################################
ALL_INCLUDE_PATHS += $(SOURCE_INCLUDE_PATHS_WITH_I)
ALL_INCLUDE_PATHS += $(PLATFORM_INCLUDE_PATHS_WITH_I)

.DEFAULT_GOAL := all
PHONY_TARGET += all 
ifeq ($(wildcard $(PROJECT_TARGET_FILE)),)
all:
	$(call print_line,Building for $(PLATFORM_NAME) platform)
	$(MUTE_COMMAND) $(MAKE) --no-print-directory project-update
	$(MUTE_COMMAND) $(MAKE) --no-print-directory project-build
else
all:
	$(call print_line,Building for $(PLATFORM_NAME) platform)
	$(MUTE_COMMAND) $(MAKE) --no-print-directory project-build
endif

PHONY_TARGET += project-update

#################### Rule for Cloning the Application Library such as no-OS ###################
project-update:
	$(MUTE_COMMAND) $(foreach d,$(APP_LIB_FILE),$(if $(wildcard $(PROJECT_APP)/$(call LIB_FILE_FOLDER_NAME,$d)/.*),echo . $(HIDE_OUTPUT),$(if $(findstring https://os.mbed.com/,$(call URL,$d)),cd $(PROJECT_APP) $(command_separator) hg clone $(call URL,$d),cd $(PROJECT_APP) $(command_separator) git clone $(call URL,$d))) $(command_separator) ) echo . $(HIDE_OUTPUT)
	$(MUTE_COMMAND) $(foreach d,$(APP_LIB_FILE),$(if $(findstring https://os.mbed.com/,$(call URL,$d)),cd $(PROJECT_APP)/$(call LIB_FILE_FOLDER_NAME,$d) $(command_separator) hg checkout $(call COMMIT_ID,$d),cd $(PROJECT_APP)/$(call LIB_FILE_FOLDER_NAME,$d) $(command_separator) git checkout $(call COMMIT_ID,$d)) $(command_separator)) echo . $(HIDE_OUTPUT)
	$(MUTE_COMMAND) $(foreach d,$(LIBRARY_LIB_FILE),$(if $(wildcard $(LIBRARIES_PATH)/$(call LIB_FILE_FOLDER_NAME,$d)/.*),echo . $(HIDE_OUTPUT),$(if $(findstring https://os.mbed.com/,$(call URL,$d)),cd $(LIBRARIES_PATH) $(command_separator) hg clone $(call URL,$d),cd $(LIBRARIES_PATH) $(command_separator) git clone $(call URL,$d))) $(command_separator) ) echo . $(HIDE_OUTPUT)
	$(MUTE_COMMAND) $(foreach d,$(LIBRARY_LIB_FILE),$(if $(findstring https://os.mbed.com/,$(call URL,$d)),cd $(LIBRARIES_PATH)/$(call LIB_FILE_FOLDER_NAME,$d) $(command_separator) hg checkout $(call COMMIT_ID,$d),cd $(LIBRARIES_PATH)/$(call LIB_FILE_FOLDER_NAME,$d) $(command_separator) git checkout $(call COMMIT_ID,$d)) $(command_separator)) echo . $(HIDE_OUTPUT)
	$(MUTE_COMMAND) $(MAKE) --no-print-directory $(PROJECT_TARGET_FILE)

##################### Creating Objects Files From Source Files ####################################
.PRECIOUS: $(BUILD_DIRECTORY)/. $(BUILD_DIRECTORY)%/.

##################### Creating Directory for Objects files ####################################
$(BUILD_DIRECTORY)/.:
	$(MUTE_COMMAND) $(call make_dir,$@) $(HIDE_OUTPUT) 
	
$(BUILD_DIRECTORY)%/.:
	$(MUTE_COMMAND) $(call make_dir,$@) $(HIDE_OUTPUT)

################### Generating .o Files From Source Files #######################################
.SECONDEXPANSION:
$(BUILD_DIRECTORY)/%.o: $$(call FULL_PATH_OF_FILE, %).cpp | $$(@D)/.
	$(call print_line,[Compiling] $(notdir $<))
	$(MUTE_COMMAND) $(CPP) $(CPP_FLAGS) $(ALL_INCLUDE_PATHS)  $< -o $@

$(BUILD_DIRECTORY)/%.o: $$(call FULL_PATH_OF_FILE, %).c | $$(@D)/.
	$(call print_line,[Compiling] $(notdir $<))
	$(MUTE_COMMAND) $(CC) $(C_FLAGS) $(ALL_INCLUDE_PATHS)  $< -o $@ 

##################### Creating Elf File ########################################
ifeq 'mbed' '$(PLATFORM_NAME)'
$(PROJECT_ELF_FILE): $(OBJECTS_FILES_IN_BUILD) $(MBED_STATIC_LIBRARY) $(PROJECT_LINKER_SCRIPT)
	$(MUTE_COMMAND) cd $(ROOT_DIR_PATH) $(command_separator) $(CC) $(LD_FLAGS) -T $(PROJECT_LINKER_SCRIPT) -o $(PROJECT_ELF_FILE) $(RELATIVE_OBJECTS_FILES_IN_BUILD) @$(GENERATED_ARCHIVE_FILE) $(LINKER_LIBRARIES)
endif

#################### Creating Binary File #######################################
$(PROJECT_BINARY_FILE):$(PROJECT_ELF_FILE)
	$(MUTE_COMMAND) $(OC) -O binary $< $@
	$(call print_line,Binary File Ready to Flash: $@)

#################### Removing Application Elf, Binary and Object Files #######################
PHONY_TARGET += clean-app
clean-app:
	$(call remove_file_recursively,$(PROJECT_ELF_FILE) $(PROJECT_BINARY_FILE) $(BUILD_DIRECTORY)/*.o $(BUILD_DIRECTORY)/*.d)

#################### Cleaning Entire Build Directories of Mbed-OS and Application ######################
PHONY_TARGET += clean
clean:
	-$(foreach d,$(DIRECTORY_TO_REMOVE),$(call remove_dir_recursively, $d) $(command_separator)) echo. $(HIDE_OUTPUT)
	$(call print_line, Clean Successful)

###################### List all Source Files and Include Paths ####################################
PHONY_TARGET += list-files
list-files:
	$(MUTE_COMMAND) $(foreach d,$(SOURCE_FILES) $(ALL_INCLUDE_PATHS), echo $d $(command_separator)) echo. $(HIDE_OUTPUT)

###################### Rule for Test Used in Jenkins ##########################################
PHONY_TARGET += test
ifeq ($(wildcard $(PROJECT_TARGET_FILE)),)
test:
	$(call print_line,Building for $(PLATFORM_NAME) platform)
	$(MUTE_COMMAND) $(MAKE) --no-print-directory project-update
	$(MUTE_COMMAND) $(MAKE) --no-print-directory project-build-test
else
test:
	$(call print_line,Building for $(PLATFORM_NAME) platform)
	$(MUTE_COMMAND) $(MAKE) --no-print-directory project-build-test
endif

##################### Creating Elf File for Test ########################################
ifeq 'mbed' '$(PLATFORM_NAME)'
$(PROJECT_ELF_FILE_FOR_TEST): $(OBJECTS_FILES_IN_BUILD) $(MBED_STATIC_LIBRARY) $(PROJECT_LINKER_SCRIPT)
	$(MUTE_COMMAND) cd $(ROOT_DIR_PATH) $(command_separator) $(CC) $(LD_FLAGS) -T $(PROJECT_LINKER_SCRIPT) -o $(PROJECT_ELF_FILE_FOR_TEST) $(RELATIVE_OBJECTS_FILES_IN_BUILD) @$(GENERATED_ARCHIVE_FILE) $(LINKER_LIBRARIES)
endif

#################### Creating Binary File for Test #######################################
$(PROJECT_BINARY_FILE_FOR_TEST):$(PROJECT_ELF_FILE_FOR_TEST)
	$(MUTE_COMMAND) $(OC) -O binary $< $@
	$(call print_line,Binary File Ready to Flash: $@)

.PHONY: $(PHONY_TARGET)

-include $(OBJECTS_FILES_IN_BUILD:.o=.d) 

	