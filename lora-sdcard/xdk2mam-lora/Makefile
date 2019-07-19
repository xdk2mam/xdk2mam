# This makefile triggers the targets in the application.mk

# The default value "../../.." assumes that this makefile is placed in the 
# folder xdk110/Apps/<App Folder> where the BCDS_BASE_DIR is the parent of 
# the xdk110 folder.
BCDS_BASE_DIR ?= ../../..

export BCDS_APP_NAME = xdk2mam-lora
export BCDS_APP_DIR = $(CURDIR)
export BCDS_APP_SOURCE_DIR = $(BCDS_APP_DIR)/source

export BCDS_RN2483_LORA_DRIVER=1

#Please refer BCDS_CFLAGS_COMMON variable in application.mk file
#and if any addition flags required then add that flags only in the below macro 
#export BCDS_CFLAGS_COMMON = 



#Enabled feature macros for common code
export XDK_CONNECTIVITY_LED =1
export XDK_CONNECTIVITY_LORA =1

export XDK_SENSOR_SENSOR =1
#end of settings related to optimized build

#List all the application header file under variable BCDS_XDK_INCLUDES 
export BCDS_XDK_INCLUDES = \
	
#List all the application source file under variable BCDS_XDK_APP_SOURCE_FILES in a similar pattern as below
export BCDS_XDK_APP_SOURCE_FILES = \
	$(wildcard $(BCDS_APP_SOURCE_DIR)/*.c) \
	$(wildcard $(BCDS_APP_SOURCE_DIR)/**/*.c)

.PHONY: clean	debug release flash_debug_bin flash_release_bin

clean: 
	$(MAKE) -C $(BCDS_BASE_DIR)/xdk110/Common -f application.mk clean

debug: 
	$(MAKE) -C $(BCDS_BASE_DIR)/xdk110/Common -f application.mk debug

release: 
	$(MAKE) -C $(BCDS_BASE_DIR)/xdk110/Common -f application.mk release

flash_debug_bin: 
	$(MAKE) -C $(BCDS_BASE_DIR)/xdk110/Common -f application.mk flash_debug_bin
	
flash_release_bin: 
	$(MAKE) -C $(BCDS_BASE_DIR)/xdk110/Common -f application.mk flash_release_bin
	
cdt:
	$(MAKE) -C $(BCDS_BASE_DIR)/xdk110/Common -f application.mk cdt

	
