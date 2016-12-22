#!/bin/bash

for dir in latency*
do
   echo $dir        
   gnuplot -persist -e "dir=\"$dir\"" experiment.gp
done
