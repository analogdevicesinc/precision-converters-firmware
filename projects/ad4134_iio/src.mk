# All the .c and .cpp and .h Files in SRC_DIRS are used in Build (Recursive)
# Source Paths for Mbed and STM32 Platform
SRC_DIRS += $(PROJECT_APP_PATH)
SRC_DIRS += $(LIBRARIES_PATH)/no-OS/drivers/adc/ad713x
SRC_DIRS += $(LIBRARIES_PATH)/no-OS/drivers/eeprom/24xx32a
SRC_DIRS += $(LIBRARIES_PATH)/no-OS/drivers/api
SRC_DIRS += $(LIBRARIES_PATH)/no-OS/util
SRC_DIRS += $(LIBRARIES_PATH)/no-OS/iio
SRC_DIRS += $(LIBRARIES_PATH)/no-OS/include
SRC_DIRS += $(LIBRARIES_PATH)/precision-converters-library/common
SRC_DIRS += $(LIBRARIES_PATH)/precision-converters-library/board_info
SRC_DIRS += $(LIBRARIES_PATH)/precision-converters-library/sdp_k1_sdram

ifeq 'mbed' '$(PLATFORM)'
# ALL_IGNORED_FILES variable used for excluding particular source files in SRC_DIRS in Build
SRC_DIRS += $(LIBRARIES_PATH)/no-OS/drivers/platform/mbed
ALL_IGNORED_FILES += $(PROJECT_APP_PATH)/app_config_stm32.c
ALL_IGNORED_FILES += $(PROJECT_APP_PATH)/stm32_tdm_support.c
ALL_IGNORED_FILES += $(PROJECT_APP_PATH)/stm32_gpio_irq_generated.c
ALL_IGNORED_FILES += $(PROJECT_APP_PATH)/app_config_stm32.h
ALL_IGNORED_FILES += $(PROJECT_APP_PATH)/stm32_tdm_support.h

# Extra Macros
override NEW_CFLAGS += -DACTIVE_PLATFORM=MBED_PLATFORM
endif
