#!/bin/bash
# 1 - output directory
# 2 - SAI path
# 3 - lib version
# 4 - sai version

LIB_NAME="mrvllibsai_$3"
pushd $1 > /dev/null
if [ ! -d "$LIB_NAME/usr/include/sai" ]; then
	mkdir -p $LIB_NAME/usr/include/sai
	cp $2/inc/* $LIB_NAME/usr/include/sai/
fi
if [ ! -d "$LIB_NAME/usr/lib" ]; then
	mkdir -p $LIB_NAME/usr/lib	
fi
cp lib/libsai.so $LIB_NAME/usr/lib/

if [ ! -d "$LIB_NAME/DEBIAN" ]; then
	mkdir -p $LIB_NAME/DEBIAN	
	echo "package: mrvllibsai" > $LIB_NAME/DEBIAN/control
	echo "Version: $3" >> $LIB_NAME/DEBIAN/control
	echo "Section: base" >> $LIB_NAME/DEBIAN/control
	echo "Priority: optional" >> $LIB_NAME/DEBIAN/control
	echo "Architecture: amd64" >> $LIB_NAME/DEBIAN/control
	echo "Depends:" >> $LIB_NAME/DEBIAN/control
	echo "Maintainer: Vladimir Kuk <vkuk@marvell.com>" >> $LIB_NAME/DEBIAN/control
	echo "Description: SAI for marvell asic" >> $LIB_NAME/DEBIAN/control
	echo " based on sai v$4" >> $LIB_NAME/DEBIAN/control
fi
dpkg-deb --build $LIB_NAME/
popd > /dev/null

