#
# *  Copyright (C) 2016. Marvell International Ltd. ALL RIGHTS RESERVED.
# *
# *    Licensed under the Apache License, Version 2.0 (the "License"); you may
# *    not use this file except in compliance with the License. You may obtain
# *    a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
# *
# *    THIS CODE IS PROVIDED ON AN  *AS IS* BASIS, WITHOUT WARRANTIES OR
# *    CONDITIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT
# *    LIMITATION ANY IMPLIED WARRANTIES OR CONDITIONS OF TITLE, FITNESS
# *    FOR A PARTICULAR PURPOSE, MERCHANTABLITY OR NON-INFRINGEMENT.
# *
# *    See the Apache Version 2.0 License for specific language governing
# *    permissions and limitations under the License.
# *
#
 
BUILD_MODE ?= DEBUG
BUILD_DIR ?= build_dir
BASE_DIR ?= ..
ARCH ?= arm

MODULES := 

FILES := \
 src/mrvl_sai_fdb.c \
 src/mrvl_sai_interfacequery.c \
 src/mrvl_sai_port.c \
 src/mrvl_sai_switch.c \
 src/mrvl_sai_vlan.c \
 src/mrvl_sai_host_interface.c \
 src/mrvl_sai_acl.c \
 src/mrvl_sai_router_intf.c \
 src/mrvl_sai_neighbor.c \
 src/mrvl_sai_nexthop.c \
 src/mrvl_sai_route.c \
 src/mrvl_sai_router.c \
 src/utils/mrvl_sai_utils.c \
 src/utils/mrvl_sai_hash.c \
 src/utils/mrvl_sai_dlist.c

TEST_FILES := \
 test/src/mrvl_sai_test.c \
 test/src/mrvl_sai_fdb_test.c \
 test/src/mrvl_sai_vlan_test.c \
 test/src/mrvl_sai_rif_test.c \
 test/src/mrvl_sai_port_test.c \
 test/src/mrvl_sai_RxTx_test.c \
 test/src/mrvl_sai_neighbor_test.c \
 test/src/mrvl_sai_nexthop_test.c \
 test/src/mrvl_sai_route_test.c \
 test/src/mrvl_sai_vr_test.c
 
CWD := $(dir $(lastword $(MAKEFILE_LIST)))
SAI_SRC := $(patsubst %,$(CWD)%,$(FILES))
SAI_TEST_SRC := $(patsubst %,$(CWD)%,$(TEST_FILES))

include $(patsubst %,$(CWD)%/$(MAKEFILE),$(MODULES))


OUT_DIR = $(BASE_DIR)/$(BUILD_DIR)
.PHONY: all
all:

CC = $(CROSS_COMPILE)gcc
LD = $(CROSS_COMPILE)gcc
AR = $(CROSS_COMPILE)ar

ECHO = echo

# Name of makefile to include in each sub-module
MAKEFILE := Makefile

ifeq ($(BUILD_MODE),DEBUG)
    DEBUG_CFLAGS = -g -O0
else
    DEBUG_CFLAGS = -O2
endif

LIB_OUT_DIR :=	lib

CFLAGS += -DCPU_LE $(DEBUG_CFLAGS) -Wall -pthread -DUNIX -fPIC

CFLAGS += -Wno-pointer-sign #-nostdinc -D__USE_GNU
DEP :=

# Function compile(flags, output_folder)
#
# Compile a .c file into .o file using the defined toolchain, including dependecy .d file.
# Object and dependency files will be located together in output_folder,
# under the relative path to the source.
#
# Example: $(call compile $(CFLAGS),/local/objects)
# Source file: foo/bar/baz.c
# Targets: /local/objects/foo/bar/baz.o
#          /local/objects/foo/bar/baz.d

define compile
@# Create ouput folder
	$(MKDIR_P) $(dir $@)
@# create dependency file
	$(CC) -M -c $1 $< -o $2/$*.d
@# Duplicate it
	cp $2/$*.d $2/$*.tmp
@# Create empty rule per file in dependency rule to avoid "No rule to make target ..." Errors 
	sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' -e '/^$$/d' -e 's/$$/ :/' < $2/$*.d >> $2/$*.tmp
@# Change object filename to include full path
	sed -e 's/$(subst .,\.,$(notdir $@))/$(subst /,\/,$@)/'  < $2/$*.tmp > $2/$*.d
@# Remove temporary file
	rm $2/$*.tmp
@# print compilation msg
	@ $(ECHO) CC $<
@# compile
	$(CC) -c $1 $< -o $@
endef

include $(patsubst %,%/$(MAKEFILE),$(MODULES))

SRC += \
 $(SAI_SRC)

SAI_OBJ := $(patsubst %.c,$(OUT_DIR)/$(ARCH)/%.o,$(SAI_SRC))
SAI_TEST_OBJ := $(patsubst %.c,$(OUT_DIR)/$(ARCH)/%.o,$(SAI_TEST_SRC))

SAI_LIB:= 

DEP += $(patsubst %.o,%.d,$(SAI_OBJ))
DEP += $(patsubst %.o,%.d,$(SAI_TEST_OBJ))

MKDIR_P = mkdir -p


EXT_INC_DIR             = $(BASE_DIR)/FPA/ext_h
SAI_INC_DIR             = SAI/inc/
PLUGIN_INC_DIR          = inc/

CFLAGS += \
    -I$(EXT_INC_DIR) \
	-I$(SAI_INC_DIR) \
	-I$(PLUGIN_INC_DIR)
	
LIBS := \
 -lpthread -lm -lrt -ldl -rdynamic 
LDFLAGS = -Wl,-Map=$(OUT_DIR)/$(ARCH)/$(LIB_OUT_DIR)/link.map

###############################################################################
#
# %.o - compile .c files to objects 

$(OUT_DIR)/$(ARCH)/%.o:  %.c
	$(call compile,$(CFLAGS),$(OUT_DIR)/$(ARCH))

clean:
	@ $(ECHO) rm -rf $(OUT_DIR)/$(ARCH)/src
	@ rm -rf $(OUT_DIR)/$(ARCH)/src
	@ $(ECHO) rm -fr $(OUT_DIR)/$(ARCH)/$(LIB_OUT_DIR)
	@ rm -fr $(OUT_DIR)/$(ARCH)/$(LIB_OUT_DIR)

sai: $(SAI_OBJ) $(SAI_TEST_OBJ) $(SAI_LIBS)
	@ $(MKDIR_P) $(OUT_DIR)/$(ARCH)/$(LIB_OUT_DIR)
	@ $(AR) -rcs $(OUT_DIR)/$(ARCH)/$(LIB_OUT_DIR)/lib$(@).o $(SAI_OBJ) $(SAI_LIBS)
	@ $(AR) -rcs $(OUT_DIR)/$(ARCH)/$(LIB_OUT_DIR)/libsaitest.o $(SAI_TEST_OBJ) 
	

-include $(DEP)
