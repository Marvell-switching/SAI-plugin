#!/bin/bash
if [ ! -d $1 ]; then
	mkdir -p $1
	pushd $1 > /dev/null
	git clone https://github.com/Marvell-switching/FPA-switch . 
	git checkout master
	popd > /dev/null
fi
