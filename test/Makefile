# $(warning "Started") 


#List of subfolders to include in build
MODULES := \
 src \
 inc

#List of files to include in list
FILES :=

CWD := $(dir $(lastword $(MAKEFILE_LIST)))

#Add relative path to original Makefile, to each file in the list

#Enable this line to add include path of this folder
#CFLAGS := $(CFLAGS) -I$(CWD)

#Recursively go into list of subfolders
include $(patsubst %,$(CWD)%/$(MAKEFILE),$(MODULES))
