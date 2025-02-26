# All the .c and .cpp and .h Files in SRC_DIRS are used in Build (Recursive)
SRC_DIRS += $(PROJECT_APP_PATH)
SRC_DIRS += $(LIBRARIES_PATH)/no-OS/drivers/adc-dac/ad5592r
SRC_DIRS += $(LIBRARIES_PATH)/no-OS/drivers/api
SRC_DIRS += $(LIBRARIES_PATH)/no-OS/drivers/platform/mbed
SRC_DIRS += $(LIBRARIES_PATH)/no-OS/iio
SRC_DIRS += $(LIBRARIES_PATH)/no-OS/util
SRC_DIRS += $(LIBRARIES_PATH)/no-OS/include
SRC_DIRS += $(LIBRARIES_PATH)/precision-converters-library/adi_console_menu

ifeq 'mbed' '$(PLATFORM)'
# ALL_IGNORED_FILES variable used for excluding particular source files in SRC_DIRS in Build
SRC_DIRS += $(LIBRARIES_PATH)/no-OS/drivers/platform/mbed
ALL_IGNORED_FILES += $(PROJECT_APP_PATH)/app_config_stm32.c
ALL_IGNORED_FILES += $(PROJECT_APP_PATH)/stm32_gpio_irq_generated.c
ALL_IGNORED_FILES += $(PROJECT_APP_PATH)/app_config_stm32.h

# Extra Macros
override NEW_CFLAGS += -DACTIVE_PLATFORM=MBED_PLATFORM
endif