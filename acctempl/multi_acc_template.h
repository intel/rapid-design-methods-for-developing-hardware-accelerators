// See LICENSE for license details.
/*
 * multi_acc_template.h
 *
 *  Created on: Mar 22, 2016
 *      Author: aayupov
 */

#ifndef MULTI_ACC_TEMPLATE_H_
#define MULTI_ACC_TEMPLATE_H_

#include "systemc.h"
#include "accelerator_interface.h"
#include "acc_mem_arbiter.h"

template <size_t N, typename SCHEDULER, typename ACC, typename CONFIG>
class multi_acc_template : public accelerator_interface<CONFIG> {
public:
  ACC aus[N];
  SCHEDULER scheduler;

  MemArbiter<N, SplMemWriteReqType, SplMemWriteRespType, AU_ARBITER> wr_arbiter;
  MemArbiter<N, SplMemReadReqType, SplMemReadRespType, AU_ARBITER> rd_arbiter;

  SimpleGate<3, GATE_AND> and_gate;
  IdleMonitor<8> idle_monitor;


  //interconnects
  sc_signal<CONFIG> cfg_ch[N];
  sc_signal<bool> start_ch[N];
  sc_signal<bool> done_ch[N];
  sc_signal<bool> wr_arb_idle, rd_arb_idle, sched_done, overall_done;

  typename MemArbiter<N, SplMemWriteReqType, SplMemWriteRespType, AU_ARBITER>::ArbiterRequestorChannel au_wr_arb_ch;
  typename MemArbiter<N, SplMemReadReqType, SplMemReadRespType, AU_ARBITER>::ArbiterRequestorChannel au_rd_arb_ch;

  SC_HAS_PROCESS(multi_acc_template);

  multi_acc_template(sc_module_name name) : accelerator_interface<CONFIG>(name),
      scheduler("acc_scheduler"), wr_arbiter(
          "au_wr_arbiter"), rd_arbiter("au_rd_arbiter"), and_gate("and_gate"), idle_monitor(
          "idle_monitor")
      , wr_arb_idle("wr_arb_idle"), rd_arb_idle(
          "rd_arb_idle"), sched_done("sched_done"), overall_done("overall_done") {
    for (unsigned i = 0; i < N; ++i) {
      aus[i].clk(this->clk);
      aus[i].rst(this->rst);
      au_wr_arb_ch.bindRequestor(i, aus[i].spl_wr_req, aus[i].spl_wr_resp, wr_arbiter);
      au_rd_arb_ch.bindRequestor(i, aus[i].spl_rd_req, aus[i].spl_rd_resp, rd_arbiter);
      aus[i].config(cfg_ch[i]);
      scheduler.acc_config[i](cfg_ch[i]);
      aus[i].done(done_ch[i]);
      scheduler.acc_done[i](done_ch[i]);
      aus[i].start(start_ch[i]);
      scheduler.acc_start[i](start_ch[i]);
    }
    wr_arbiter.clk(this->clk);
    wr_arbiter.rst(this->rst);
    rd_arbiter.clk(this->clk);
    rd_arbiter.rst(this->rst);

    scheduler.done(sched_done);
    scheduler.start(this->start);
    scheduler.config(this->config);
    scheduler.clk(this->clk);
    scheduler.rst(this->rst);
    idle_monitor.clk(this->clk);
    idle_monitor.rst(this->rst);

    wr_arbiter.out_req_fifo(this->spl_wr_req);
    wr_arbiter.in_resp_fifo(this->spl_wr_resp);
    rd_arbiter.out_req_fifo(this->spl_rd_req);
    rd_arbiter.in_resp_fifo(this->spl_rd_resp);

    wr_arbiter.idle(wr_arb_idle);
    and_gate.ins[0](wr_arb_idle);
    rd_arbiter.idle(rd_arb_idle);
    and_gate.ins[1](rd_arb_idle);

    and_gate.ins[2](sched_done);
    and_gate.out(overall_done);
    idle_monitor.in_idle(overall_done);
    idle_monitor.out_idle(this->done);
  }

};


template <size_t N, typename SCHEDULER, typename ACC, typename CONFIG>
class multi_acc_template_2p : public accelerator_interface_2p<CONFIG> {
public:
  ACC aus[N];
  SCHEDULER scheduler;

  MemArbiter<N, SplMemWriteReqType, SplMemWriteRespType, AU_ARBITER> wr_arbiter;
  MemArbiterN<N, 2, SplMemReadReqType, SplMemReadRespType, AU_ARBITER> rd_arbiter;

  SimpleGate<3, GATE_AND> and_gate;
  IdleMonitor<8> idle_monitor;


  //interconnects
  sc_signal<CONFIG> cfg_ch[N];
  sc_signal<bool> start_ch[N];
  sc_signal<bool> done_ch[N];
  sc_signal<bool> wr_arb_idle, rd_arb_idle, sched_done, overall_done;

  typename MemArbiter<N, SplMemWriteReqType, SplMemWriteRespType, AU_ARBITER>::ArbiterRequestorChannel au_wr_arb_ch;
  typename MemArbiterN<N, 2, SplMemReadReqType, SplMemReadRespType, AU_ARBITER>::ArbiterRequestorChannel au_rd_arb_ch;

  SC_HAS_PROCESS(multi_acc_template_2p);

  multi_acc_template_2p(sc_module_name name) : accelerator_interface_2p<CONFIG>(name),
      scheduler("acc_scheduler"), wr_arbiter(
          "au_wr_arbiter"), rd_arbiter("au_rd_arbiter"), and_gate("and_gate"), idle_monitor(
          "idle_monitor")
      , wr_arb_idle("wr_arb_idle"), rd_arb_idle(
          "rd_arb_idle"), sched_done("sched_done"), overall_done("overall_done") {
    for (unsigned i = 0; i < N; ++i) {
      aus[i].clk(this->clk);
      aus[i].rst(this->rst);
      au_wr_arb_ch.bindRequestor(i, aus[i].spl_wr_req, aus[i].spl_wr_resp, wr_arbiter);
      au_rd_arb_ch.bindRequestor(i, aus[i].spl_rd_req, aus[i].spl_rd_resp, rd_arbiter);
      aus[i].config(cfg_ch[i]);
      scheduler.acc_config[i](cfg_ch[i]);
      aus[i].done(done_ch[i]);
      scheduler.acc_done[i](done_ch[i]);
      aus[i].start(start_ch[i]);
      scheduler.acc_start[i](start_ch[i]);
    }
    wr_arbiter.clk(this->clk);
    wr_arbiter.rst(this->rst);
    rd_arbiter.clk(this->clk);
    rd_arbiter.rst(this->rst);

    scheduler.done(sched_done);
    scheduler.start(this->start);
    scheduler.config(this->config);
    scheduler.clk(this->clk);
    scheduler.rst(this->rst);
    idle_monitor.clk(this->clk);
    idle_monitor.rst(this->rst);

    wr_arbiter.out_req_fifo(this->spl_wr_req);
    wr_arbiter.in_resp_fifo(this->spl_wr_resp);
    rd_arbiter.out_req_fifo[0](this->spl_rd_req1);
    rd_arbiter.in_resp_fifo[0](this->spl_rd_resp1);
    rd_arbiter.out_req_fifo[1](this->spl_rd_req2);
    rd_arbiter.in_resp_fifo[1](this->spl_rd_resp2);

    wr_arbiter.idle(wr_arb_idle);
    and_gate.ins[0](wr_arb_idle);
    rd_arbiter.idle(rd_arb_idle);
    and_gate.ins[1](rd_arb_idle);

    and_gate.ins[2](sched_done);
    and_gate.out(overall_done);
    idle_monitor.in_idle(overall_done);
    idle_monitor.out_idle(this->done);
  }

};

template <size_t N, typename SCHEDULER, typename ACC, typename CONFIG, size_t RDN, size_t WRN=1>
class multi_acc_template_Np : public accelerator_interface_Np<CONFIG, RDN, WRN> {
public:
  typedef CONFIG Config;

  ACC aus[N];
  SCHEDULER scheduler;

  MemArbiterN<N, WRN, SplMemWriteReqType, SplMemWriteRespType, AU_ARBITER> wr_arbiter;
  MemArbiterN<N, RDN, SplMemReadReqType, SplMemReadRespType, AU_ARBITER> rd_arbiter;

  SimpleGate<3, GATE_AND> and_gate;
  IdleMonitor<8> idle_monitor;


  //interconnects
  sc_signal<CONFIG> cfg_ch[N];
  sc_signal<bool> start_ch[N];
  sc_signal<bool> done_ch[N];
  sc_signal<bool> wr_arb_idle, rd_arb_idle, sched_done, overall_done;

  typename MemArbiterN<N, WRN, SplMemWriteReqType, SplMemWriteRespType, AU_ARBITER>::ArbiterRequestorChannel au_wr_arb_ch;
  typename MemArbiterN<N, RDN, SplMemReadReqType, SplMemReadRespType, AU_ARBITER>::ArbiterRequestorChannel au_rd_arb_ch;

  SC_HAS_PROCESS(multi_acc_template_Np);

  multi_acc_template_Np(sc_module_name name) : accelerator_interface_Np<CONFIG, RDN, WRN>(name),
      scheduler("acc_scheduler"), wr_arbiter(
          "au_wr_arbiter"), rd_arbiter("au_rd_arbiter"), and_gate("and_gate"), idle_monitor(
          "idle_monitor")
      , wr_arb_idle("wr_arb_idle"), rd_arb_idle(
          "rd_arb_idle"), sched_done("sched_done"), overall_done("overall_done") {
    for (unsigned i = 0; i < N; ++i) {
      aus[i].clk(this->clk);
      aus[i].rst(this->rst);
      au_wr_arb_ch.bindRequestor(i, aus[i].spl_wr_req, aus[i].spl_wr_resp, wr_arbiter);
      au_rd_arb_ch.bindRequestor(i, aus[i].spl_rd_req, aus[i].spl_rd_resp, rd_arbiter);
      aus[i].config(cfg_ch[i]);
      scheduler.acc_config[i](cfg_ch[i]);
      aus[i].done(done_ch[i]);
      scheduler.acc_done[i](done_ch[i]);
      aus[i].start(start_ch[i]);
      scheduler.acc_start[i](start_ch[i]);
    }
    wr_arbiter.clk(this->clk);
    wr_arbiter.rst(this->rst);
    rd_arbiter.clk(this->clk);
    rd_arbiter.rst(this->rst);

    scheduler.done(sched_done);
    scheduler.start(this->start);
    scheduler.config(this->config);
    scheduler.clk(this->clk);
    scheduler.rst(this->rst);
    idle_monitor.clk(this->clk);
    idle_monitor.rst(this->rst);

    for (unsigned i = 0; i < RDN; ++i) {
      rd_arbiter.out_req_fifo[i](this->spl_rd_req[i]);
      rd_arbiter.in_resp_fifo[i](this->spl_rd_resp[i]);
    }
    for (unsigned i = 0; i < WRN; ++i) {
      wr_arbiter.out_req_fifo[i](this->spl_wr_req[i]);
      wr_arbiter.in_resp_fifo[i](this->spl_wr_resp[i]);
    }

    wr_arbiter.idle(wr_arb_idle);
    and_gate.ins[0](wr_arb_idle);
    rd_arbiter.idle(rd_arb_idle);
    and_gate.ins[1](rd_arb_idle);

    and_gate.ins[2](sched_done);
    and_gate.out(overall_done);
    idle_monitor.in_idle(overall_done);
    idle_monitor.out_idle(this->done);
  }

};


#endif /* MULTI_ACC_TEMPLATE_H_ */
