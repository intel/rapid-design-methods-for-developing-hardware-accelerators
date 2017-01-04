#!/bin/bash

for slots in 16 32 64 96 128 160
do
    echo "Compiling ${slots}..."
    cd acc_build
    make clean
    make __lst_Slots__=${slots} __inp_Slots__=32
    echo "Running ${slots}..."
    ./acc_test > LOG-${slots}
    cd -
done
