#!/usr/bash

make clean
premake4 clean
premake4 gmake
make $1 config=release64 -j8

