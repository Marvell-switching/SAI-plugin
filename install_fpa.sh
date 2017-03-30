#!/bin/bash
if [ ! -d "FPA-switch" ]; then
	git clone https://github.com/Marvell-switching/FPA-switch 
	cd FPA-switch 
	git reset --hard e43243a
	cd .. 
fi
