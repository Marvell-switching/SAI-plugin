#!/bin/bash
if [ ! -d $1 ]; then
	mkdir -p $1
	pushd $1 > /dev/null
	git clone https://github.com/opencomputeproject/SAI.git .
	git reset --hard $2
	popd > /dev/null
fi
