# Marvell SAI Plugin
This repository contains SAI implementation for Marvell hardware.

SAI headers are based on release v0.9.4. SAI headers can be downloaded from https://github.com/opencomputeproject/SAI/

The implementation is written over Marvell FPA interfaces API release v1.8. The API and documentation for it, are available in https://github.com/Marvell-switching/fpa-switch

The make command will download relevent SAI and FPA h files from github.

The "plugin/test" directory contains examples for API usage and is not part of the libsai.so

Usage:
    Following will download the code and build debian packages using existing libraries:

        git clone https://github.com/Marvell-switching/SAI-plugin.git
        cd SAI-plugin
        make deb

    To rebuild only library:
        make clean
        make lib

    To rebuild only debian package with updated libraries:
        make deb

    To build with debug info:
    	make clean
        make DEBUG=yes

    Resulting binaries reside in bin/intel/ folder.
 
