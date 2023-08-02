#!/bin/sh
rm -rf ./build
meson setup build -DjentKernel=enabled
meson compile -C build
