#!/bin/bash

for dir in latency-1000-spread-0-lst
do
   echo $dir        
   gnuplot -persist -e "dir=\"$dir\"" experiment-lst.gp
done

for dir in latency-1000-spread-0-inp
do
   echo $dir        
   gnuplot -persist -e "dir=\"$dir\"" experiment-inp.gp
done
