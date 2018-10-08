# See LICENSE for license details.
## Copyright (c) 2005-2015, Intel Corporation
##
## Redistribution  and  use  in source  and  binary  forms,  with  or  without
## modification, are permitted provided that the following conditions are met:
##
## * Redistributions of  source code  must retain the  above copyright notice,
##   this list of conditions and the following disclaimer.
## * Redistributions in binary form must reproduce the above copyright notice,
##   this list of conditions and the following disclaimer in the documentation
##   and/or other materials provided with the distribution.
## * Neither the name  of Intel Corporation  nor the names of its contributors
##   may be used to  endorse or promote  products derived  from this  software
##   without specific prior written permission.
##
## THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
## AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,  BUT NOT LIMITED TO,  THE
## IMPLIED WARRANTIES OF  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
## ARE DISCLAIMED.  IN NO EVENT  SHALL THE COPYRIGHT OWNER  OR CONTRIBUTORS BE
## LIABLE  FOR  ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR
## CONSEQUENTIAL  DAMAGES  (INCLUDING,  BUT  NOT LIMITED  TO,  PROCUREMENT  OF
## SUBSTITUTE GOODS OR SERVICES;  LOSS OF USE,  DATA, OR PROFITS;  OR BUSINESS
## INTERRUPTION)  HOWEVER CAUSED  AND ON ANY THEORY  OF LIABILITY,  WHETHER IN
## CONTRACT,  STRICT LIABILITY,  OR TORT  (INCLUDING NEGLIGENCE  OR OTHERWISE)
## ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,  EVEN IF ADVISED OF THE
## POSSIBILITY OF SUCH DAMAGE.
##****************************************************************************
##     Intel(R) Accelerator Abstraction Layer Library Software Developer
##        Kit (SDK)
##  Content:
##     samples/Hello_SPL_LB/Makefile
##  Author:
##     Tim Whisonant, Intel Corporation
##     Joseph Grecco, Intel Corporation
##  History:
##     06/15/2015    JG   Initial version based off of canonical sample makefile
##******************************************************************************
CPPFLAGS ?=
CXX      ?= g++
LDFLAGS  ?=

ifeq ($(FOR), HW)
CPPFLAGS += -DHWAFU
else 
CPPFLAGS += -DASEAFU
endif

ifneq (,$(ndebug))
else
CPPFLAGS += -DENABLE_DEBUG=1
endif
ifneq (,$(nassert))
else
CPPFLAGS += -DENABLE_ASSERT=1
endif

ifeq ($(USER),smburns)
CCI_MPF_SW_ROOT ?= /nfs/site/disks/scl.work.59/ppt/smburns/quick-assist/SR-5.0.3-Release/atd_fpga_app-qa_bblocks/BBB_cci_mpf/sw
HLD_COMMON_DIR ?= ../../../common
else
CCI_MPF_SW_ROOT ?= /nfs/site/disks/scl.work.38/ppt/aayupov/systemc/atd_fpga_app-qa_bblocks/BBB_cci_mpf/sw
ifeq (,$(HLD_ROOT))
HLD_COMMON_DIR ?= /nfs/site/disks/scl.work.38/ppt/aayupov/systemc/hld_fpga-sysc/common
else
HLD_COMMON_DIR ?= $(HLD_ROOT)/common
endif 
endif

ifeq (,$(DESTDIR))
ifneq (,$(prefix))
CPPFLAGS += -I$(prefix)/include -I./ -I$(CCI_MPF_SW_ROOT)/include  -I$(HLD_COMMON_DIR)
LDFLAGS  += -L$(prefix)/lib -Wl,-rpath-link -Wl,$(prefix)/lib -Wl,-rpath -Wl,$(prefix)/lib \
            -L$(prefix)/lib64 -Wl,-rpath-link -Wl,$(prefix)/lib64 -Wl,-rpath -Wl,$(prefix)/lib64
endif
else
ifeq (,$(prefix))
prefix = /usr/local
endif
CPPFLAGS += -I$(DESTDIR)$(prefix)/include -I./
LDFLAGS  += -L$(DESTDIR)$(prefix)/lib -Wl,-rpath-link -Wl,$(prefix)/lib -Wl,-rpath -Wl,$(DESTDIR)$(prefix)/lib \
            -L$(DESTDIR)$(prefix)/lib64 -Wl,-rpath-link -Wl,$(prefix)/lib64 -Wl,-rpath -Wl,$(DESTDIR)$(prefix)/lib64
endif

all: libAcclApp.so libAcclApp.so.0

libAcclApp.so.0: libAcclApp.so.0.0.0
	ln -sf libAcclApp.so.0.0.0 libAcclApp.so.0

libAcclApp.so: libAcclApp.so.0.0.0
	ln -sf libAcclApp.so.0.0.0 libAcclApp.so

libAcclApp.so.0.0.0: AcclApp.o Makefile
	$(CXX) -fPIC -shared -Wl,-soname -Wl,libAcclApp.so.0 AcclApp.o $(LDFLAGS) -lOSAL -lAAS -laalrt -o libAcclApp.so.0.0.0

AcclApp.o: AcclApp.cpp AcclApp.h Makefile
	$(CXX) $(CPPFLAGS) -D__AAL_USER__=1 -fPIC -DPIC -g -c -o AcclApp.o AcclApp.cpp

clean:
	$(RM) *.o *.so.* *.so

.PHONY:all clean
