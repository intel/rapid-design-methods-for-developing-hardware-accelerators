# See LICENSE for license details.
HLD_ROOT ?= ../../../..

SOURCES=tb.cpp
CFLAGS = -O3

ifdef WORDS_PER_BLK
CFLAGS += -DWORDS_PER_BLK=$(WORDS_PER_BLK)
endif

USER_DEFS ?= -DKERNEL_TEST
TARGET=kernel_test

CFLAGS+=${USER_DEFS} -I..
include ${HLD_ROOT}/common/Makefile.inc
