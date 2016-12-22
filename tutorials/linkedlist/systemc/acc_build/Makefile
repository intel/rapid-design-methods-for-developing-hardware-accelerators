# See LICENSE for license details.
HLD_ROOT = ../../../..

SOURCES=cycle_detection_sw.cpp 
CFLAGS =

USER_DEFS ?= -DSC_ACC
TARGET=cycle_detection_acc

SOURCES=cycle_detection_sw.cpp 

CFLAGS+=${USER_DEFS} -I.. -I${HLD_ROOT}/fpga_sw/5.0.3/AcclAppVtp/
include ${HLD_ROOT}/common/Makefile.inc
