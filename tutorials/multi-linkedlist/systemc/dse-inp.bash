#!/bin/bash

for slots in 1
do
    echo "Compiling ${slots}..."
    cd acc_build
    make clean
    make __lst_Slots__=128 __inp_Slots__=${slots}
    echo "Running ${slots}..."
    ./acc_test > LOG-${slots}
    cd -
done
