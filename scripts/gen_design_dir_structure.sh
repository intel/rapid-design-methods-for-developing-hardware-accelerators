#!/bin/bash
read -p "This script will generate new folders (sw, hw, systemc, etc) in the CURRENT directory. Is that what you want? (y/n)" -n 1 -r
echo    # (optional) move to a new line
if [[ $REPLY =~ ^[Yy]$ ]]
then
mkdir -p systemc
mkdir -p data
mkdir -p sw
mkdir -p hw
mkdir -p hw/par
mkdir -p hw/rtl
mkdir -p hw/sim
fi
