# Marvell SAI Plugin

This repository contains SAI implementation for Marvell hardware.

SAI headers are based on release v0.9.1 (as of Feb 24, 2016). SAI headers can be downloaded from https://github.com/opencomputeproject/SAI/

The implementation is written over Marvell FPA interfaces API. The API and documentation for it, are available in https://github.com/Marvell-switching/fpa-api

Variable EXT_INC_DIR in the Makefile should be updated to point the right path to FPA header files
Compilation is done per CPU architecture by setting the flap ARCH=(arm, INTEL64, ...). for example make ARCH=arm sai. the result of the compilation will be placed under build_dir/$(ARCH)/lib, called libsai.o

User applications can then link with this library, in order to use the SAI implementation

mrvl_plugin/test directory contains few examples for sai API usage and are not part of the libsai.o
