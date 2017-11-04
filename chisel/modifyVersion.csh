#!/bin/csh -f

foreach dir ( firrtl chisel3 firrtl-interpreter chisel-testers)
  (cd $dir; sed '1,$s/scalaVersion := "2.11.11"/scalaVersion := "2.12.3"/' < build.sbt > _tmp; diff _tmp build.sbt; mv _tmp build.sbt)
end

