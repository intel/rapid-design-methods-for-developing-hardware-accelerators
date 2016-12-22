// See LICENSE for license details.
/*
 * accelerator_interface.h
 *
 *  Created on: Mar 22, 2016
 *      Author: aayupov
 */

#ifndef ACCELERATOR_INTERFACE_H_
#define ACCELERATOR_INTERFACE_H_


#include "systemc.h"
#include "types.h"
#include "ga_tlm_fifo.h"

template <typename CONFIG>
struct accelerator_interface : public sc_module {

  typedef CONFIG Config;

  sc_in_clk clk;
  sc_in<bool> rst;
  sc_in<CONFIG> config;
  sc_in<bool> start;
  sc_out<bool> done;
  //top level ports
  //ga::tlm_fifo_hier_in<AccCommandType> cmd_in;

#ifdef __SYSTEMC_AFU__
#ifdef __SCL_FLEX_CH__
  scl_hier_get_initiator<SplMemReadRespType> spl_rd_resp;
  scl_hier_put_initiator<SplMemReadReqType> spl_rd_req;
  scl_hier_get_initiator<SplMemWriteRespType> spl_wr_resp;
  scl_hier_put_initiator<SplMemWriteReqType> spl_wr_req;
#else
  hier_get_initiator<SplMemReadRespType> spl_rd_resp;
  hier_put_initiator<SplMemReadReqType> spl_rd_req;
  hier_get_initiator<SplMemWriteRespType> spl_wr_resp;
  hier_put_initiator<SplMemWriteReqType> spl_wr_req;
#endif //__SCL_FLEX_CH__
#else
  ga::tlm_fifo_hier_in<SplMemReadRespType> spl_rd_resp;
  ga::tlm_fifo_hier_out<SplMemReadReqType> spl_rd_req;
  ga::tlm_fifo_hier_in<SplMemWriteRespType> spl_wr_resp;
  ga::tlm_fifo_hier_out<SplMemWriteReqType> spl_wr_req;
#endif //__SYSTEMC_AFU__
  // end of top level ports

  accelerator_interface(sc_module_name name) : sc_module(name),
      clk("clk"), rst("rst"), config("config"), start("start"), done("done"), spl_rd_resp(
          "spl_rd_resp"), spl_rd_req("spl_rd_req"), spl_wr_resp("spl_wr_resp"), spl_wr_req(
          "spl_wr_req") {
  }
};

template <typename CONFIG>
struct accelerator_interface_2p : public sc_module {

  sc_in_clk clk;
  sc_in<bool> rst;
  sc_in<CONFIG> config;
  sc_in<bool> start;
  sc_out<bool> done;
  //top level ports
  //ga::tlm_fifo_hier_in<AccCommandType> cmd_in;

#ifdef __SYSTEMC_AFU__
#ifdef __SCL_FLEX_CH__
  scl_hier_get_initiator<SplMemReadRespType> spl_rd_resp1;
  scl_hier_put_initiator<SplMemReadReqType> spl_rd_req1;
  scl_hier_get_initiator<SplMemReadRespType> spl_rd_resp2;
  scl_hier_put_initiator<SplMemReadReqType> spl_rd_req2;
  scl_hier_get_initiator<SplMemWriteRespType> spl_wr_resp;
  scl_hier_put_initiator<SplMemWriteReqType> spl_wr_req;
#else
  hier_get_initiator<SplMemReadRespType> spl_rd_resp1;
  hier_put_initiator<SplMemReadReqType> spl_rd_req1;
  hier_get_initiator<SplMemReadRespType> spl_rd_resp2;
  hier_put_initiator<SplMemReadReqType> spl_rd_req2;
  hier_get_initiator<SplMemWriteRespType> spl_wr_resp;
  hier_put_initiator<SplMemWriteReqType> spl_wr_req;
#endif //__SCL_FLEX_CH__
#else
  ga::tlm_fifo_hier_in<SplMemReadRespType> spl_rd_resp1;
  ga::tlm_fifo_hier_out<SplMemReadReqType> spl_rd_req1;
  ga::tlm_fifo_hier_in<SplMemReadRespType> spl_rd_resp2;
  ga::tlm_fifo_hier_out<SplMemReadReqType> spl_rd_req2;
  ga::tlm_fifo_hier_in<SplMemWriteRespType> spl_wr_resp;
  ga::tlm_fifo_hier_out<SplMemWriteReqType> spl_wr_req;
#endif //__SYSTEMC_AFU__
  // end of top level ports

  accelerator_interface_2p(sc_module_name name) : sc_module(name),
      clk("clk"), rst("rst"), config("config"), start("start"), done("done"), spl_wr_resp("spl_wr_resp"), spl_wr_req(
          "spl_wr_req") {
  }
};

template <typename CONFIG, size_t RDN = 2, size_t WRN = RDN>
struct accelerator_interface_Np : public sc_module {

  sc_in_clk clk;
  sc_in<bool> rst;
  sc_in<CONFIG> config;
  sc_in<bool> start;
  sc_out<bool> done;
  //top level ports
  //ga::tlm_fifo_hier_in<AccCommandType> cmd_in;

#ifdef __SYSTEMC_AFU__
#ifdef __SCL_FLEX_CH__
  scl_hier_get_initiator<SplMemReadRespType> spl_rd_resp[RDN];
  scl_hier_put_initiator<SplMemReadReqType> spl_rd_req[RDN];
  scl_hier_get_initiator<SplMemWriteRespType> spl_wr_resp[WRN];
  scl_hier_put_initiator<SplMemWriteReqType> spl_wr_req[WRN];
#else
  hier_get_initiator<SplMemReadRespType> spl_rd_resp[RDN];
  hier_put_initiator<SplMemReadReqType> spl_rd_req[RDN];
  hier_get_initiator<SplMemWriteRespType> spl_wr_resp[WRN];
  hier_put_initiator<SplMemWriteReqType> spl_wr_req[WRN];
#endif //__SCL_FLEX_CH__
#else
  ga::tlm_fifo_hier_in<SplMemReadRespType> spl_rd_resp[RDN];
  ga::tlm_fifo_hier_out<SplMemReadReqType> spl_rd_req[RDN];
  ga::tlm_fifo_hier_in<SplMemWriteRespType> spl_wr_resp[WRN];
  ga::tlm_fifo_hier_out<SplMemWriteReqType> spl_wr_req[WRN];
#endif //__SYSTEMC_AFU__
  // end of top level ports

  accelerator_interface_Np(sc_module_name name) : sc_module(name),
      clk("clk"), rst("rst"), config("config"), start("start"), done("done") {
  }
};


template <size_t N, typename CONFIG>
struct acc_scheduler_interface {
  sc_in_clk clk;
  sc_in<bool> rst;

  sc_in<CONFIG> config;
  sc_in<bool> start;
  sc_out<bool> done;

  sc_out<CONFIG> acc_config[N];
  sc_out<bool> acc_start[N];
  sc_in<bool> acc_done[N];

  acc_scheduler_interface()  {
  }
};

#endif /* ACCELERATOR_INTERFACE_H_ */
