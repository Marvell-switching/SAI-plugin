# $(warning "Started") 


#List of subfolders to include in build
MODULES := \
 utils

#List of files to include in list
FILES := \
 mrvl_sai_fdb.c \
 mrvl_sai_interfacequery.c \
 mrvl_sai_port.c \
 mrvl_sai_switch.c \
 mrvl_sai_vlan.c \
 mrvl_sai_host_interface.c \
 mrvl_sai_acl.c \
 mrvl_sai_router_intf.c \
 mrvl_sai_neighbor.c \
 mrvl_sai_nexthop.c \
 mrvl_sai_nexthopgroup.c \
 mrvl_sai_route.c \
 mrvl_sai_router.c \
 mrvl_sai_stubs.c \
 mrvl_sai_lag.c
 

CWD := $(dir $(lastword $(MAKEFILE_LIST)))

#Add relative path to original Makefile, to each file in the list
SAI_SRC := $(patsubst %,$(CWD)%,$(FILES))

#Enable this line to add include path of this folder
#CFLAGS := $(CFLAGS) -I$(CWD)

#Recursively go into list of subfolders
include $(patsubst %,$(CWD)%/$(MAKEFILE),$(MODULES))

SAI_SRC += \
 $(SAI_UTILS_SRC)

