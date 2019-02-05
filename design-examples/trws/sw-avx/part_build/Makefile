# See LICENSE for license details.
HLD_ROOT = ../../../..

SOURCES=tb.cpp AcclApp.cpp
TARGET=trws_part

CFLAGS=${USER_DEFS} -I. -DUSE_SOFTWARE -O2 -g -I../tbb/include -march=native

ifdef USE_TBB
CFLAGS += -DUSE_TBB
endif

ifdef NUM_OF_THREADS
CFLAGS += -DNUM_OF_THREADS=$(NUM_OF_THREADS)
endif


LDFLAGS = 
ifdef USE_TBB
LDFLAGS += -L ../tbb/build/linux_intel64_gcc_cc4.8.3_libc2.11.3_kernel3.0.101_release -ltbb -lrt
endif

include $(HLD_ROOT)/common/Makefile.inc
