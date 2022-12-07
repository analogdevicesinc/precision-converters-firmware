############## Platform Name #############
PLATFORM_NAME = mbed

################ All the .c and .cpp Files in SOURCE_DIRS are used in Build (Not Recursive) #####################
################ PROJECT_APP Variable Points to "Project_Name/app" Directory
SOURCE_DIRS += $(PROJECT_APP)
SOURCE_DIRS += $(PROJECT_APP)/no-OS/drivers/adc/ad7689
SOURCE_DIRS += $(PROJECT_APP)/no-OS/drivers/eeprom/24xx32a
SOURCE_DIRS += $(PROJECT_APP)/no-OS/drivers/api
SOURCE_DIRS += $(PROJECT_APP)/no-OS/drivers/platform/mbed
SOURCE_DIRS += $(PROJECT_APP)/no-OS/iio
SOURCE_DIRS += $(PROJECT_APP)/no-OS/util
SOURCE_DIRS += $(LIBRARIES_PATH)/precision-converters-library/board_info
SOURCE_DIRS += $(LIBRARIES_PATH)/precision-converters-library/sdp_k1_sdram

################# If Need to Add Only One File in any Directory Use "SOURCE_FILES" Variable #####################
################ Example    SOURCE_FILES += $(PROJECT_APP)/main.c

################ SOURCE_INCLUDE_PATHS contain path of Header Files Used in Build ################################
SOURCE_INCLUDE_PATHS += $(PROJECT_APP)
SOURCE_INCLUDE_PATHS += $(PROJECT_APP)/no-OS/drivers/adc/ad7689
SOURCE_INCLUDE_PATHS += $(PROJECT_APP)/no-OS/drivers/eeprom/24xx32a
SOURCE_INCLUDE_PATHS += $(PROJECT_APP)/no-OS/drivers/platform/mbed
SOURCE_INCLUDE_PATHS += $(PROJECT_APP)/no-OS/iio
SOURCE_INCLUDE_PATHS += $(PROJECT_APP)/no-OS/include
SOURCE_INCLUDE_PATHS += $(LIBRARIES_PATH)/precision-converters-library/board_info
SOURCE_INCLUDE_PATHS += $(LIBRARIES_PATH)/precision-converters-library/sdp_k1_sdram
