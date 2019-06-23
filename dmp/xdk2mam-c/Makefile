# This makefile triggers the targets in the application.mk

# The default value "../../.." assumes that this makefile is placed in the 
# folder xdk110/Apps/<App Folder> where the BCDS_BASE_DIR is the parent of 
# the xdk110 folder.
BCDS_BASE_DIR ?= ../../..

# Macro to define Start-up method. change this macro to "CUSTOM_STARTUP" to have custom start-up.
export BCDS_SYSTEM_STARTUP_METHOD = DEFAULT_STARTUP
export BCDS_APP_NAME = xdk2mam
export BCDS_APP_DIR = $(CURDIR)
export BCDS_APP_SOURCE_DIR = $(BCDS_APP_DIR)/source
export BCDS_APP_SOURCE_DIR_SENSORS = $(BCDS_APP_DIR)/source/sensors

#Please refer BCDS_CFLAGS_COMMON variable in application.mk file
#and if any addition flags required then add that flags only in the below macro 
#export BCDS_CFLAGS_COMMON =
export XDK_CONNECTIVITY_BLE=0
export XDK_CONNECTIVITY_LORA=0
export XDK_CONNECTIVITY_LWM2M=0
export XDK_CONNECTIVITY_MQTT=0
export XDK_CONNECTIVITY_UDP=0

# This variable should fully specify the build configuration of the Serval 
# Stack library with regards the enabled and disabled features for the HTTPS Using TLS. 
export SERVAL_TLS_MBEDTLS=1
export SERVAL_ENABLE_TLS_CLIENT=1
export SERVAL_ENABLE_TLS_ECC=1
export SERVAL_ENABLE_TLS_PSK=0
export SERVAL_MAX_NUM_MESSAGES=8
export SERVAL_MAX_SIZE_APP_PACKET=900
export SERVAL_ENABLE_TLS=1

#List all the application header file under variable BCDS_XDK_INCLUDES in a similar pattern as below
export BCDS_XDK_INCLUDES = \
		-I$(BCDS_APP_SOURCE_DIR_SENSORS)/protected \

#List all the application source file under variable BCDS_XDK_APP_SOURCE_FILES in a similar pattern as below
export BCDS_XDK_APP_SOURCE_FILES = \
	$(BCDS_APP_SOURCE_DIR)/Main.c \
	$(BCDS_APP_SOURCE_DIR)/xdk2mam.c \
	$(BCDS_APP_SOURCE_DIR_SENSORS)/EnvironmentalSensor.c \
	$(BCDS_APP_SOURCE_DIR_SENSORS)/Accelerometer.c \
	$(BCDS_APP_SOURCE_DIR_SENSORS)/Gyroscope.c \
	$(BCDS_APP_SOURCE_DIR_SENSORS)/InertialSensor.c \
	$(BCDS_APP_SOURCE_DIR_SENSORS)/LightSensor.c \
	$(BCDS_APP_SOURCE_DIR_SENSORS)/Magnetometer.c \
	$(BCDS_APP_SOURCE_DIR_SENSORS)/Acoustic.c \

	
.PHONY: clean debug release flash_debug_bin flash_release_bin

clean: 
	$(MAKE) -C $(BCDS_BASE_DIR)/xdk110/Common -f application.mk clean

debug: 
	$(MAKE) -C $(BCDS_BASE_DIR)/xdk110/Common -f application.mk debug
	
release: 
	$(MAKE) -C $(BCDS_BASE_DIR)/xdk110/Common -f application.mk release
	
clean_Libraries:
	$(MAKE) -C $(BCDS_BASE_DIR)/xdk110/Common -f application.mk clean_libraries
	
flash_debug_bin: 
	$(MAKE) -C $(BCDS_BASE_DIR)/xdk110/Common -f application.mk flash_debug_bin
	
flash_release_bin: 
	$(MAKE) -C $(BCDS_BASE_DIR)/xdk110/Common -f application.mk flash_release_bin

cdt:
	$(MAKE) -C $(BCDS_BASE_DIR)/xdk110/Common -f application.mk cdt	