MODULES :=

FILES := \
 mrvl_sai_dlist.c \
 mrvl_sai_hash.c \
 mrvl_sai_utils.c

CWD := $(dir $(lastword $(MAKEFILE_LIST)))
SAI_UTILS_SRC := $(patsubst %,$(CWD)%,$(FILES))

include $(patsubst %,$(CWD)%/$(MAKEFILE),$(MODULES))

