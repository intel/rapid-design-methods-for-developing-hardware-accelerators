// See LICENSE for license details.
/*
 * pagerank_tb.h
 *
 *  Created on: Sep 23, 2016
 *      Author: aayupov
 */

#ifndef PAGERANK_TB_H_
#define PAGERANK_TB_H_


#include "accl_tb.h"
#include "pagerank_hls.h"

#define NUM_AUS 4
#define RD_PORTS NUM_AUS

typedef multi_acc_template_Np<RD_PORTS, pagerank_sched<RD_PORTS>, pagerank_acc, Config, RD_PORTS, 1> prdut;
//typedef acc_top <prdut, 1000/*Mhz*/, 150/*mem latency in ns*/, 1/*number of mem read ports*/, 1/*number of mem write ports*/> AcclApp;
//typedef acc_top <pagerank_acc, 1000/*Mhz*/, 80/*mem latency in ns*/, 1/*number of mem read ports*/, 1/*number of mem write ports*/, 0> AcclApp;
typedef acc_top <prdut, 100/*Mhz*/, 800/*mem latency in ns*/, RD_PORTS/*number of mem read ports*/, 1/*number of mem write ports*/, 0> AcclApp;


#endif /* PAGERANK_TB_H_ */
