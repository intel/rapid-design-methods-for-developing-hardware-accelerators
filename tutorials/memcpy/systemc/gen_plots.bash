#!/bin/bash

for dir in latency*
do
   echo $dir        
   /usr/intel/pkgs/gnuplot/4.6.6/bin/gnuplot -persist -e "dir=\"$dir\"" experiment.gp
done
