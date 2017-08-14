# See LICENSE for license details.
HLD_ROOT ?= ../../../..

SOURCES=tb.cpp
CFLAGS = -O2

ifdef DISABLE_PRECOMP_LEN
CFLAGS += -DDISABLE_PRECOMP_LEN
endif

ifdef MAX_PRECOMP_LEN
CFLAGS += -DMAX_PRECOMP_LEN=$(MAX_PRECOMP_LEN)
endif

USER_DEFS ?= -DKERNEL_TEST
TARGET=kernel_test

CFLAGS+=${USER_DEFS} -I..
include ${HLD_ROOT}/common/Makefile.inc
