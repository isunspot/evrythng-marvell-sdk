
EVRYTHNG_MARVELL_SDK_PATH=$(shell pwd)
PROJECT_ROOT=$(shell pwd)

export PROJECT_ROOT

all: demo tests

include common.mk

.PHONY: all demo demo_clean tests tests_clean clean 

demo: wmsdk 
	$(AT)$(MAKE) -C $(WMSDK_BUNDLE_DIR) APP=$(PROJECT_ROOT)/apps/demo

demo_clean:
	$(AT)$(MAKE) -C $(WMSDK_BUNDLE_DIR) APP=$(PROJECT_ROOT)/apps/demo clean

demo_flashprog: demo
	cd $(WMSDK_PATH)/tools/OpenOCD; \
	sudo ./flashprog.py --$(BOARD_FW_PARTITION) $(WMSDK_BUNDLE_DIR)/bin/mw300_defconfig/mw300_rd/evrythng_demo.bin \

demo_ramload: demo
	cd $(WMSDK_PATH)/tools/OpenOCD; \
	sudo ./ramload.sh $(PROJECT_ROOT)/apps/demo/bin/evrythng_demo.axf \

demo_footprint:
	$(AT)$(WMSDK_BUNDLE_DIR)/wmsdk/tools/bin/footprint.pl -m apps/demo/bin/evrythng_demo.map



gen_config:
	$(MAKE) -C $(PROJECT_ROOT)/lib/core gen_config

tests: gen_config wmsdk
	$(AT)$(MAKE) -C $(WMSDK_BUNDLE_DIR) APP=$(PROJECT_ROOT)/apps/tests

tests_clean:
	$(AT)$(MAKE) -C $(WMSDK_BUNDLE_DIR) APP=$(PROJECT_ROOT)/apps/tests clean

tests_ramload: tests
	cd $(WMSDK_PATH)/tools/OpenOCD; \
	sudo ./ramload.py $(WMSDK_BUNDLE_DIR)/bin/mw300_defconfig/mw300_rd/evrythng_tests.axf \

tests_flashprog: tests
	cd $(WMSDK_PATH)/tools/OpenOCD; \
	sudo ./flashprog.py --$(BOARD_FW_PARTITION) $(WMSDK_BUNDLE_DIR)/bin/mw300_defconfig/mw300_rd/evrythng_tests.bin \

clean: demo_clean tests_clean wmsdk_clean 



rheem: wmsdk 
	$(AT)$(MAKE) -C $(WMSDK_BUNDLE_DIR) APP=$(PROJECT_ROOT)/apps/rheem

rheem_clean:
	$(AT)$(MAKE) -C $(WMSDK_BUNDLE_DIR) APP=$(PROJECT_ROOT)/apps/rheem clean

rheem_flashprog: demo
	cd $(WMSDK_PATH)/tools/OpenOCD; \
	sudo ./flashprog.py --$(BOARD_FW_PARTITION) $(WMSDK_BUNDLE_DIR)/bin/mw300_defconfig/mw300_rd/rheem_demo.bin \

rheem_ramload: rheem
	cd $(WMSDK_PATH)/tools/OpenOCD; \
	sudo ./ramload.sh $(PROJECT_ROOT)/apps/rheem/bin/rheem_demo.axf \

rheem_footprint:
	$(AT)$(WMSDK_BUNDLE_DIR)/wmsdk/tools/bin/footprint.pl -m apps/rheem/bin/rheem_demo.map

