#!/bin/bash

set -x

curl --location -O http://downloads.sourceforge.net/project/elfio/ELFIO-sources/ELFIO-1.0.3/ELFIO-1.0.3.tar.gz

tar -xzf ELFIO-1.0.3.tar.gz

cd ELFIO-1.0.3
patch -p1 -i ../ELFIO_stdint.patch
./configure --prefix=`pwd`/install_root
make && make install
cd ..