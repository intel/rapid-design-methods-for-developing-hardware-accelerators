# See LICENSE for license details.

HLD_ROOT ?= ../
ifneq ("$(wildcard /home/smburns)","")
SC_DIR=/home/smburns/pg-sobel/systemc-2.3.0-install
GTEST_DIR = /home/smburns/pg-sobel/googletest
CTOS_DIR = /home/smburns/pg-sobel/ctos
else
SC_DIR ?= /p/hdk/rtl/cad/x86-64_linux26/accellera/systemc/systemc-2.3.0
GTEST_DIR ?= /nfs/site/disks/scl.work.58/ppt/aayupov/gtest/googletest/googletest
CTOS_DIR ?= /nfs/site/disks/ccdo.soc.cad_root.0/cad/x86-64_linux26/cadence/ctos/14.2/share/ctos
endif

SC_LIB ?= $(SC_DIR)/lib-linux64/libsystemc.a

CFLAGS+= -std=c++11 -Wall -I$(HLD_ROOT)/common -I$(HLD_ROOT)/accio -I$(HLD_ROOT)/acctempl -Wno-virtual-move-assign -I$(SC_DIR)/include -Wno-unused-variable
#add warning waiving for some HLS related code practices

ifdef USE_HLS
#HLS flags
HLS_FLAGS= -I$(CTOS_DIR)/include/ctos_tlm -I$(CTOS_DIR)/include/ctos_flex_channels -I$(CTOS_DIR)/include/ctos_fx -DUSE_HLS -DSC_INCLUDE_DYNAMIC_PROCESSES
CFLAGS+= ${HLS_FLAGS}
endif

CFLAGS+= -Wno-unused-label

#GTEST flags 
CFLAGS+=-I${GTEST_DIR}/include

OBJECTS=$(SOURCES:.cpp=.o)

LDFLAGS += ${SC_LIB} $(GTEST_DIR)/make/gtest_main.a

all: $(OBJECTS)
	g++ -pthread -o $(TARGET) $(OBJECTS) $(LDFLAGS)
 

-include $(OBJECTS:.o=.d)


.cpp.o:
	g++ $(DEBUG_FLAGS) $(CFLAGS) $(MACRODEFS) -o $@ -c $<
	g++ -MM $(CFLAGS) $(MACRODEFS) $< > $*.d

clean: 
	@rm -f *.o *.d $(TARGET)


#/usr/intel/pkgs/gdb/7.5.1/bin/gdb
