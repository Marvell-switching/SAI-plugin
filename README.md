Marvell SAI implementation
============================

This repository contains SAI implementation and FPA SDK image for Marvell hardware for SONIC project (https://github.com/Azure/sonic).

SAI headers are based on latest head of release v1.0.0.
The current API compiles with SONIC.

SAI headers can be downloaded from https://github.com/opencomputeproject/SAI/

The DAI implementation is written over Marvell FPA interfaces API. The API and documentation for it, are available in
https://github.com/Marvell-switching/FPA-switch

User applications can then link with this library, in order to use the SAI implementation.
