// See LICENSE for license details.
#ifndef __DUT_ACC_TB_H__
#define __DUT_ACC_TB_H__


#include "accl_tb.h"
#include "cycle_detection_hls.h"
  
//typedef multi_acc_template_Np<4, cycle_detection_sched<4>, cycle_detection_acc, Config, 4, 1> dut;
//typedef acc_top <dut, 100/*Mhz*/, 1000/*mem latency in ns*/, 4/*number of mem read ports*/, 1/*number of mem write ports*/> AcclApp;
typedef acc_top <cycle_detection_acc, 100/*Mhz*/, 1000/*mem latency in ns*/, 1/*number of mem read ports*/, 1/*number of mem write ports*/> AcclApp;


#endif //__DUT_ACC_TB_H__

