#!/bin/bash

export CC=gcc-4.6
export CXX=g++-4.6
export LOGFILE=gcc46.log

source ./tools/ci/scripts/init.sh

aptget_install gcc-4.6 g++-4.6 \
    make autoconf automake autopoint gettext libphysfs-dev \
    libxml2-dev libcurl4-gnutls-dev libpng-dev \
    libsdl-gfx1.2-dev libsdl-image1.2-dev libsdl-mixer1.2-dev libsdl-net1.2-dev libsdl-ttf2.0-dev

do_init
run_configure --enable-werror
run_make

source ./tools/ci/scripts/exit.sh

exit 0
