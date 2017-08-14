# See LICENSE for license details.
HLD_ROOT ?= ../../../..

SOURCES=tb.cpp
CFLAGS = -O3

ifdef __inp_Slots__
CFLAGS += -D__inp_Slots__=$(__inp_Slots__)
endif

ifdef __lst_Slots__
CFLAGS += -D__lst_Slots__=$(__lst_Slots__)
endif

ifdef NUM_AUS
CFLAGS += -DNUM_AUS=$(NUM_AUS)
endif

USER_DEFS ?= 
TARGET=acc_test

CFLAGS+=${USER_DEFS} -I..
include ${HLD_ROOT}/common/Makefile.inc
