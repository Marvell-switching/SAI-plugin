#!/bin/bash
if [ ! -d "SAI" ]; then
	git clone https://github.com/opencomputeproject/SAI.git
	cd SAI
	git reset --hard cbcaedf
	cd .. 
fi
