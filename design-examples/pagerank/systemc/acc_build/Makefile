# See LICENSE for license details.
HLD_ROOT ?= ../../../../

DEBUG_FLAGS=-O3

USER_DEFS ?= -DSC_ACC -fopenmp
# -DGRAPHGEN
SOURCES=sc_main.cpp 
TARGET=ga_acc

CFLAGS=${USER_DEFS} -g -I.. -I${HLD_ROOT}/pagerank-hybrid/gapbs/src/ -Wno-unknown-pragmas
LDFLAGS=-fopenmp
include $(HLD_ROOT)/common/Makefile.inc
