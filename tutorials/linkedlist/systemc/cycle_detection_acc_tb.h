// See LICENSE for license details.
#ifndef CD_TB_H_
#define CD_TB_H_

#include "cycle_detection_hls.h"
#include "accl_tb.h"

typedef acc_top <dut_t, 150/*Mhz*/, 720/*mem latency in ns*/, RD_CHANNELS, WR_CHANNELS> AcclApp;

#endif
