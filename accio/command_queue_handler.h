// See LICENSE for license details.
/*
 * command_queue_handler.h
 *
 *  Created on: Feb 11, 2016
 *      Author: aayupov
 */

#ifndef COMMAND_QUEUE_HANDLER_H_
#define COMMAND_QUEUE_HANDLER_H_

#include "systemc.h"
#include "types.h"
#include "ga_tlm_fifo.h"

struct AccCommandType {
  CacheLineType cmd;
  AccCommandType() {}
  AccCommandType(AddressType src, MemRequestSizeType src_size, AddressType dest, MemRequestSizeType dest_size) {
    cmd.words[0] = src;
    cmd.words[1] = src_size;
    cmd.words[2] = dest;
    cmd.words[3] = dest_size;
  }

  inline friend void sc_trace(sc_trace_file* tf, const AccCommandType& d,
      const std::string& name) {
  }

  inline friend std::ostream& operator<<(std::ostream& os,
      const AccCommandType& d) {
    os << "cmd: " << d.cmd << std::endl;
    return os;
  }

  inline bool operator==(const AccCommandType& rhs) {
    bool result = true;
    result = result && (cmd == rhs.cmd);
    return result;
  }
};

// hack to support variable nubmer of parameters, up to 4 in this case. we can support 3 readers, 1 writers or 2 readers 2 writers, or 1 reader 1 writer and all in the middle
template<typename CONFIG, size_t NUM_OF_READERS, size_t NUM_OF_WRITERS, typename T1=AccNoType, typename T2=AccNoType, typename T3=AccNoType, typename T4=AccNoType>
class CommandQueueHandler {};

template<typename CONFIG, typename T1, typename T2>
class CommandQueueHandler<CONFIG, 1, 1, T1, T2> : public sc_module {
public:
  sc_in_clk clk;
  sc_in<bool> rst;
  sc_in<bool> start;
  sc_in<CONFIG> config;

  ga::tlm_fifo_out<MemTypedReadReqType<T1> > rd_req_out;
  ga::tlm_fifo_out<MemTypedWriteReqType<T2> > wr_req_out;

  SC_HAS_PROCESS(CommandQueueHandler);

  CommandQueueHandler(sc_module_name name) :
    clk("clk"), rst("rst"), config("config"), rd_req_out("rd_req_out"), wr_req_out(
        "wr_req_out") {
    SC_CTHREAD(fetch_proc, clk.pos());
    async_reset_signal_is(rst, false);
    rd_req_out.clk_rst(clk, rst);
    wr_req_out.clk_rst(clk, rst);
  }

  void parse_cmd(const AccCommandType &cmd, MemTypedReadReqType<T1> &rd_req, MemTypedWriteReqType<T2> &wr_req) {
    rd_req = MemTypedReadReqType<T1>(cmd.cmd.words[0], cmd.cmd.words[1]);
    wr_req = MemTypedWriteReqType<T2>(cmd.cmd.words[2], cmd.cmd.words[3]);
  }

  void parse_config(const CONFIG &cfg, MemTypedReadReqType<T1> &rd_req, MemTypedWriteReqType<T2> &wr_req) {
    rd_req = MemTypedReadReqType<T1>(cfg.getSrcAddr(), cfg.getNumberOfRequests());
    wr_req = MemTypedWriteReqType<T2>(cfg.getDstAddr(), cfg.getNumberOfRequests());
  }

  void fetch_proc() {
    bool started = false;
    rd_req_out.reset_put();
    wr_req_out.reset_put();
    wait();
    while(1) {
      if (!started && start.read() && rd_req_out.nb_can_put() && wr_req_out.nb_can_put()) {
        MemTypedReadReqType<T1> rd_req;
        MemTypedWriteReqType<T2> wr_req;
        parse_config(config.read(), rd_req, wr_req);

        rd_req_out.nb_put(rd_req);
        wr_req_out.nb_put(wr_req);
        started = true;
      }
      wait();
    }
  }
};



#endif /* COMMAND_QUEUE_HANDLER_H_ */
