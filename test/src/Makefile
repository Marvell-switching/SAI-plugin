# $(warning "Started") 


#List of subfolders to include in build
MODULES :=

#List of files to include in list
FILES := \
 mrvl_sai_switch_test.c  \
 mrvl_sai_fdb_test.c \
 mrvl_sai_vlan_test.c  \
 mrvl_sai_acl_test.c  \
# mrvl_sai_rif_test.c \
 mrvl_sai_port_test.c \
 mrvl_sai_lag_test.c  \
 mrvl_sai_vr_test.c 

# mrvl_sai_route_test.c \
# mrvl_sai_RxTx_test.c \
# mrvl_sai_neighbor_test.c \
# mrvl_sai_nexthop_test.c \
# mrvl_sai_nexthopgroup_test.c \
# mrvl_sai_hostif_test.c 
#  

CWD := $(dir $(lastword $(MAKEFILE_LIST)))

#Add relative path to original Makefile, to each file in the list
SAI_TEST_SRC := $(patsubst %,$(CWD)%,$(FILES))

#Enable this line to add include path of this folder
#CFLAGS := $(CFLAGS) -I$(CWD)

#Recursively go into list of subfolders
include $(patsubst %,$(CWD)%/$(MAKEFILE),$(MODULES))
