##
## Define base source files
##

include ./common_include.mk

BASE_FILE_PATH = .
BASE_FILE_SRC = opae_svc_wrapper.cpp
BASE_FILE_INC = $(BASE_FILE_PATH)/opae_svc_wrapper.h $(BASE_FILE_PATH)/csr_mgr.h

VPATH = .:$(BASE_FILE_PATH)

CPPFLAGS += -I.
LDFLAGS += -lboost_program_options -lMPF-cxx -lMPF -lopae-cxx-core
