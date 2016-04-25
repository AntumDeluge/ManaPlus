#!/bin/bash

export CC=gcc-4.9
export CXX=g++-4.9
export LOGFILE=gcc49.log

source ./tools/ci/scripts/init.sh

aptget_install gcc-4.9 g++-4.9 \
    make autoconf automake autopoint gettext libphysfs-dev \
    libxml2-dev libcurl4-gnutls-dev libpng-dev \
    libsdl-gfx1.2-dev libsdl-image1.2-dev libsdl-mixer1.2-dev libsdl-net1.2-dev libsdl-ttf2.0-dev

export CXXFLAGS="-fno-var-tracking"

do_init
run_configure --enable-werror $*
run_make

source ./tools/ci/scripts/exit.sh

exit 0
