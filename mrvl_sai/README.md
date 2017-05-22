# Marvell SAI Plugin

This repository contains SAI implementation for Marvell hardware.

SAI headers are based on release v0.9.4. SAI headers can be downloaded from https://github.com/opencomputeproject/SAI/

The implementation is written over Marvell FPA interfaces API release v1.8. The API and documentation for it, are available in https://github.com/Marvell-switching/fpa-switch

The make command will download relevent SAI and FPA h files from github.

Repository contains:
    
    * Source code for Marvell SAI plugin implementation

    * Deb package for Marvell SAI plugin

    * Deb package for Marvell FPA library

The "plugin/test" directory contains examples for API usage and is not part of the libsai.so
