.PHONY: all

all: lib fpa_deb deb

###############################################################################
ARCH ?= intel
DEBUG ?= yes
DEMO ?= yes
CROSS_COMPILE ?= 
BASE_DIR ?= $(shell pwd)
OUT_DIR ?= $(BASE_DIR)/bin/$(ARCH)
FPA_PATH ?= $(BASE_DIR)/FPA-switch/
SAI_H_PATH ?= $(BASE_DIR)/SAI/
SAI_OBJ_PATH ?= $(OUT_DIR)
SCRIPT_PATH ?= $(BASE_DIR)/scripts
PLUGIN_VERSION =`cat version.txt | grep plugin_version | awk {'print $$2'}`
SAI_VERSION    =`cat version.txt | grep sai_version    | awk {'print $$2'}`
SAI_BASELINE   =`cat version.txt | grep sai_baseline   | awk {'print $$2'}`
FPA_BASELINE   =`cat version.txt | grep fpa_baseline   | awk {'print $$2'}`

###############################################################################
CC = $(CROSS_COMPILE)gcc
LD = $(CROSS_COMPILE)gcc
AR = $(CROSS_COMPILE)ar
ECHO = echo
MKDIR_P = mkdir -p
# Name of makefile to include in each sub-module
MAKEFILE := Makefile
# Function compile(flags, output_folder)
#
# Compile a .c file into .o file using the defined toolchain, including dependecy .d file.
# Object and dependency files will be located together in output_folder,
# under the relative path to the source.
#
# Example: $(call compile $(MY_CFLAGS),/local/objects)
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
###############################################################################
MY_CFLAGS = -Wno-pointer-sign -DCPU_LE -Wall -DLINUX -DUNIX -fPIC
ifeq ("$(DEBUG)","yes")
    DEBUG_MY_CFLAGS = -g -O0
else
    DEBUG_MY_CFLAGS = -O2
endif

MY_CFLAGS += $(DEBUG_MY_CFLAGS) 

ifeq ("$(DEMO)", "yes")
  MY_CFLAGS += -DDEMO
endif

###############################################################################
MODULES := \
 inc \
 src
 
CWD := $(dir $(lastword $(MAKEFILE_LIST)))
include $(patsubst %,$(CWD)%/$(MAKEFILE),$(MODULES))

SAI_H_INC := $(SAI_H_PATH)/inc
FPA_H_INC := $(FPA_PATH)/inc

MY_CFLAGS += \
    -I$(BASE_DIR)/linux_include/ \
    -I$(FPA_H_INC) \
    -I$(SAI_H_INC) \
	$(SAI_INC)

###############################################################################
SAI_OBJ := $(patsubst %.c,$(SAI_OBJ_PATH)/%.o,$(SAI_SRC))
SAI_LIB := $(SAI_OBJ_PATH)/lib/libsai
FPA_LIB_PATH := $(FPA_PATH)/bin/$(ARCH)/lib

DEP = $(patsubst %.o,%.d,$(SAI_OBJ))

###############################################################################
MAKE_FPA := make ARCH=$(ARCH) DEBUG=$(DEBUG) DEMO=$(DEMO)
###############################################################################

# %.o - compile .c files to objects 
$(OUT_DIR)/%.o:  %.c
	$(call compile,$(MY_CFLAGS),$(OUT_DIR))

uninstall:
	@ rm -fr $(SAI_H_PATH)
	@ rm -fr $(FPA_PATH)

clean:
	@ rm -fr $(BASE_DIR)/bin

install:
	@ $(SCRIPT_PATH)/install_sai.sh $(SAI_H_PATH) $(SAI_BASELINE)
	@ $(SCRIPT_PATH)/install_fpa.sh $(FPA_PATH) $(FPA_BASELINE)

lib: install $(SAI_OBJ)
	@ $(MKDIR_P) $(SAI_OBJ_PATH)/lib
	@ $(AR) -rcs $(SAI_LIB).a $(SAI_OBJ)
	@ $(CC) -shared -o $(SAI_LIB).so $(SAI_OBJ) -L$(FPA_LIB_PATH) -lfpa
	@ $(ECHO) $(SAI_LIB) Done..

fpa_deb: install
	@ cd $(FPA_PATH) && $(MAKE_FPA) deb && cd -

$(SAI_LIB).so:
	@ $(ECHO) "SAI lib does not exist !!!"
   
deb: $(SAI_LIB).so
	@ $(SCRIPT_PATH)/make_deb.sh $(OUT_DIR) $(SAI_H_PATH) $(PLUGIN_VERSION) $(SAI_VERSION)


-include $(DEP)

