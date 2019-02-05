# See LICENSE for license details.
HLD_ROOT ?= ../../../..

SOURCES=tb.cpp
CFLAGS =

USER_DEFS ?= -DKERNEL_TEST -g
TARGET=kernel_test

CFLAGS+=${USER_DEFS} -I..
include ${HLD_ROOT}/common/Makefile.inc
