# $(warning "Started") 

#List of subfolders to include in build
MODULES := \
 utils

#List of files to include in list
FILES :=

CWD := $(dir $(lastword $(MAKEFILE_LIST)))

SAI_INC := -I$(CWD)
#Recursively go into list of subfolders
include $(patsubst %,$(CWD)%/$(MAKEFILE),$(MODULES))

SAI_INC += \
 $(SAI_UTILS_INC)

