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

ifdef NUM_AUS
CFLAGS += -DNUM_AUS=$(NUM_AUS)
endif

ifdef RD_CHANNELS
CFLAGS += -DRD_CHANNELS=$(RD_CHANNELS)
endif

ifdef WR_CHANNELS
CFLAGS += -DWR_CHANNELS=$(WR_CHANNELS)
endif

ifdef BWPatternLoadParamsSlots
CFLAGS += -D__BWPatternLoadParamsSlots__=$(BWPatternLoadParamsSlots)
endif

ifdef BWResultLoadParamsSlots
CFLAGS += -D__BWResultLoadParamsSlots__=$(BWResultLoadParamsSlots)
endif

ifdef BWCacheLineLoadParamsSlots
CFLAGS += -D__BWCacheLineLoadParamsSlots__=$(BWCacheLineLoadParamsSlots)
endif

USER_DEFS ?= 
TARGET=acc_test

CFLAGS+=${USER_DEFS} -I..
include ${HLD_ROOT}/common/Makefile.inc
