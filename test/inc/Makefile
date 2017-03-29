# $(warning "Started") 


#List of subfolders to include in build
MODULES := 

#List of files to include in list
FILES := 


CWD := $(dir $(lastword $(MAKEFILE_LIST)))

#Add relative path to original Makefile, to each file in the list
#CPSS_SRC := $(patsubst %,$(CWD)%,$(FILES))

#Enable this line to add include path of this folder
#CFLAGS := $(CFLAGS) -I$(CWD)
SAI_TEST_INC := -I$(CWD)

#Recursively go into list of subfolders
include $(patsubst %,$(CWD)%/$(MAKEFILE),$(MODULES))

