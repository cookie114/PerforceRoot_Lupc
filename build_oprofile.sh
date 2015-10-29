#!/bin/bash

cd public/popt &&  ./build_popt.sh && cd ../..
#cd public/binutils && ./buid_binutils.sh && cd ../..
cd public/oprofile && ./build_oprofile.sh && cd ../..
