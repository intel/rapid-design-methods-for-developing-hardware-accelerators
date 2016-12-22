#!/bin/bash

for slots in 16 32 64 96 128 160 192 224 256
do
    echo "Compiling ${slots}..."
    make clean
    make __inp_Slots__=${slots}
    echo "Running ${slots}..."
    ./accel_test > LOG-${slots}
done
