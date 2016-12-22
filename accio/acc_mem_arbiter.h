// See LICENSE for license details.

#ifndef ACC_MEM_ARBITER_H_
#define ACC_MEM_ARBITER_H_

#include <systemc.h>
#include "types.h"
#include "hls_utils.h"
#include "ga_tlm_fifo.h"
#include "acc_mem_arbiter_trait.h"

#ifdef __SYSTEMC_AFU__
#ifdef __SCL_FLEX_CH__
#include "scl_flex_channel.h"
#else
#include "ctos_tlm.h"
#include "ctos_flex_channels.h"
#endif //__SCL_FLEX_CH__
#endif //__SYSTEMC_AFU__


template<unsigned int NR_OF_REQS, class REQUEST, class RESPONSE, enum ARBITER_TRAIT TRAIT = IO_ARBITER>
struct MemArbiter : sc_module {
#ifdef STRATUS
  HLS_INLINE_MODULE;
#endif
public:
  sc_core::sc_in_clk clk;
  sc_core::sc_in<bool> rst;
  sc_out<bool> idle;

  typedef typename SizeT<NR_OF_REQS>::Type ReqCountType;

  //ga::tlm_fifo_in<REQUEST> in_req_fifo[NR_OF_REQS];
  //ga::tlm_fifo_out<REQUEST> out_req_fifo;
  //ga::tlm_fifo_in<RESPONSE> in_resp_fifo;
  //ga::tlm_fifo_out<RESPONSE> out_resp_fifo[NR_OF_REQS];
#ifdef __SYSTEMC_AFU__
  ga::tlm_fifo_in<REQUEST> in_req_fifo[NR_OF_REQS];
#ifdef __SCL_FLEX_CH__
  scl_put_initiator<REQUEST> out_req_fifo;
  scl_get_initiator<RESPONSE> in_resp_fifo;
#else
  nb_put_initiator<REQUEST> out_req_fifo;
  nb_get_initiator<RESPONSE> in_resp_fifo;
#endif //__SCL_FLEX_CH__
  ga::tlm_fifo_out<RESPONSE> out_resp_fifo[NR_OF_REQS];
#else
  ga::tlm_fifo_in<REQUEST> in_req_fifo[NR_OF_REQS];
  ga::tlm_fifo_out<REQUEST> out_req_fifo;
  ga::tlm_fifo_in<RESPONSE> in_resp_fifo;
  ga::tlm_fifo_out<RESPONSE> out_resp_fifo[NR_OF_REQS];
#endif //__SYSTEMC_AFU__


  SC_HAS_PROCESS (MemArbiter);

  MemArbiter(sc_core::sc_module_name name = sc_core::sc_gen_unique_name(
      "ARBITER")) :
        sc_core::sc_module(name), clk("clk"), rst("rst"), idle("idle"), out_req_fifo("out_req_fifo"), in_resp_fifo("in_resp_fifo") {
    SC_CTHREAD(mux_thread,clk.pos());
    //sensitive << clk.pos();
    async_reset_signal_is(rst, false);
    SC_CTHREAD(demux_thread,clk.pos());
    //sensitive << clk.pos();
    async_reset_signal_is(rst, false);

    for (unsigned int i = 0; i < NR_OF_REQS; i++) {
      in_req_fifo[i].clk_rst(clk, rst);
      out_resp_fifo[i].clk_rst(clk, rst);
    }
    out_req_fifo.clk_rst(clk, rst);
    in_resp_fifo.clk_rst(clk, rst);
  }
  size_t idle_counter;
  size_t consumer_not_ready;

  void mux_thread() {

    start_index = 0;
    MUX_THREAD_RESET_LOOP: for (unsigned i = 0; i < NR_OF_REQS; i++) {
      in_req_fifo[i].reset_get();
    }
    idle_counter = 0;
    out_req_fifo.reset_put();
    idle = true;
    consumer_not_ready = true;
    wait();
    while (1) {
      if (out_req_fifo.nb_can_put()) {
        //select next requester
        bool bitVector[NR_OF_REQS] ;
        SELECT_ENTRY_RB_UNROLL: for (unsigned i=0; i < NR_OF_REQS; ++i) {
          ReqCountType entry_index = (start_index + i) % NR_OF_REQS ;
          bitVector[i] = in_req_fifo[entry_index].nb_can_get() ;
        }

        ReqCountType chosen_bit ;
        bool foundRequestor = bitvec_utils<ReqCountType, NR_OF_REQS>::find_leading_one(bitVector, chosen_bit);
        if (foundRequestor) {
          ReqCountType next_index = (chosen_bit+start_index) % NR_OF_REQS ;

          assert(in_req_fifo[next_index].nb_can_get());
          assert(out_req_fifo.nb_can_put());
          REQUEST req;
          in_req_fifo[next_index].nb_get(req);
          //req.io_unit_id = next_index;
          req.io_unit_id = ArbiterTagResolver<TRAIT>::template addTag<NR_OF_REQS>(req.io_unit_id, next_index);
          DBG_OUT << sc_time_stamp() << " arbiter ("<< TRAIT << ") " << basename() << " got request from unit " <<  (size_t)req.io_unit_id << " at address: " <<  req.addr << " and send it out to output " << endl;
          out_req_fifo.nb_put(req);

          // next time we'll start with the next requester
          start_index = (next_index + 1) % NR_OF_REQS ;
        }
        idle = !foundRequestor;
      } else {
        idle = false;
        consumer_not_ready++;
      }
      if (idle) idle_counter++;
      wait();
    }
  }

  void demux_thread() {
    in_resp_fifo.reset_get();
    DEMUX_THREAD_RESET_LOOP: for (unsigned int i = 0; i < NR_OF_REQS; i++) {
      out_resp_fifo[i].reset_put();
    }
    wait();
    while (1) {
      {
        //cout<<"In demux thread..."<<endl;
        if (in_resp_fifo.nb_can_get()) {
          RESPONSE resp;
          //SPLResponse resp;
          in_resp_fifo.nb_get(resp);
          IOUnitIdType io_unit_id = ArbiterTagResolver<TRAIT>::template extractTag<NR_OF_REQS>(resp.io_unit_id);
          assert(out_resp_fifo[io_unit_id].nb_can_put());
          out_resp_fifo[io_unit_id].nb_put(resp);
          DBG_OUT << sc_time_stamp() << " arbiter " << basename() << " got response for unit " << (size_t)resp.io_unit_id << " and send it out to input " << endl;


        }
      }

      wait();
    }
  }

  ~MemArbiter() {
    const char* name = basename();
    cout << "Arbiter " << name << " " << TRAIT << " was idle (not requestors) for " << idle_counter << " cycles, consumer was not ready for "<< consumer_not_ready << " cycles" << endl;
  }
  struct ArbiterRequestorChannel {
    ga::tlm_fifo<REQUEST> in_req_fifo[NR_OF_REQS];
    ga::tlm_fifo<RESPONSE> out_resp_fifo[NR_OF_REQS];

    template <typename T1, typename T2>
    void bindRequestor (unsigned int i, T1 &req, T2 &resp, MemArbiter<NR_OF_REQS, REQUEST, RESPONSE, TRAIT> &arb) {
      assert(i < NR_OF_REQS);
      req(in_req_fifo[i]);
      resp(out_resp_fifo[i]);
      arb.in_req_fifo[i](in_req_fifo[i]);
      arb.out_resp_fifo[i](out_resp_fifo[i]);
    }

    //    template <typename T>
    //    void bindServicer (T servr, MemArbiter<NR_OF_REQS, REQUEST, RESPONSE, TRAIT> arb) {
    //      ga::tlm_fifo_out<REQUEST> out_req_fifo;
    //      ga::tlm_fifo_in<RESPONSE> in_resp_fifo;
    //      servr.in_req_fifo(in_req_fifo);
    //      servr.out_resp_fifo(out_resp_fifo);
    //      arb.out_req_fifo(in_req_fifo);
    //      arb.in_resp_fifo(out_resp_fifo);
    //    }

  };

private:
  unsigned int start_index;
};

template<unsigned int NR_OF_REQS, unsigned int NR_OF_RSPS, class REQUEST, class RESPONSE, enum ARBITER_TRAIT TRAIT = IO_ARBITER>
struct MemArbiterN : sc_module {
#ifdef STRATUS
  HLS_INLINE_MODULE;
#endif
public:
  sc_core::sc_in_clk clk;
  sc_core::sc_in<bool> rst;
  sc_out<bool> idle;

  typedef typename SizeT<NR_OF_REQS>::Type ReqCountType;
  typedef unsigned int ReqIndexType;


  //ga::tlm_fifo_in<REQUEST> in_req_fifo[NR_OF_REQS];
  //ga::tlm_fifo_out<REQUEST> out_req_fifo;
  //ga::tlm_fifo_in<RESPONSE> in_resp_fifo;
  //ga::tlm_fifo_out<RESPONSE> out_resp_fifo[NR_OF_REQS];
#ifdef __SYSTEMC_AFU__
  ga::tlm_fifo_in<REQUEST> in_req_fifo[NR_OF_REQS];
#ifdef __SCL_FLEX_CH__
  scl_put_initiator<REQUEST> out_req_fifo[NR_OF_RSPS];
  scl_get_initiator<RESPONSE> in_resp_fifo[NR_OF_RSPS];
#else
  nb_put_initiator<REQUEST> out_req_fifo[NR_OF_RSPS];
  nb_get_initiator<RESPONSE> in_resp_fifo[NR_OF_RSPS];
#endif //__SCL_FLEX_CH__
  ga::tlm_fifo_out<RESPONSE> out_resp_fifo[NR_OF_REQS];
#else
  ga::tlm_fifo_in<REQUEST> in_req_fifo[NR_OF_REQS];
  ga::tlm_fifo_out<REQUEST> out_req_fifo[NR_OF_RSPS];
  ga::tlm_fifo_in<RESPONSE> in_resp_fifo[NR_OF_RSPS];
  ga::tlm_fifo_out<RESPONSE> out_resp_fifo[NR_OF_REQS];
#endif //__SYSTEMC_AFU__


  SC_HAS_PROCESS (MemArbiterN);

  MemArbiterN(sc_core::sc_module_name name = sc_core::sc_gen_unique_name(
      "ARBITER")) :
        sc_core::sc_module(name), clk("clk"), rst("rst"), idle("idle") {
    SC_CTHREAD(mux_thread,clk.pos());
    //sensitive << clk.pos();
    async_reset_signal_is(rst, false);
    SC_CTHREAD(demux_thread,clk.pos());
    //sensitive << clk.pos();
    async_reset_signal_is(rst, false);

    for (unsigned int i = 0; i < NR_OF_REQS; i++) {
      in_req_fifo[i].clk_rst(clk, rst);
      out_resp_fifo[i].clk_rst(clk, rst);
    }
    for (unsigned int i = 0; i < NR_OF_RSPS; i++) {
      out_req_fifo[i].clk_rst(clk, rst);
      in_resp_fifo[i].clk_rst(clk, rst);
    }
  }
  size_t idle_counter;
  size_t consumer_not_ready;
  void mux_thread() {
    idle.write(false);
    start_index = 0;
    MUX_THREAD_RESET_LOOP1: for (unsigned i = 0; i < NR_OF_REQS; i++) {
      in_req_fifo[i].reset_get();
    }
    idle_counter = 0;
    MUX_THREAD_RESET_LOOP2: for (unsigned i = 0; i < NR_OF_RSPS; i++) {
      out_req_fifo[i].reset_put();
    }
    bool hasReqs = false;
    consumer_not_ready = 0;
    wait();
    while (1) {
      //probe which requesters are ready
      bool reqsVector[NR_OF_REQS] ;
      SELECT_ENTRY_RB_UNROLL: for (unsigned i=0; i < NR_OF_REQS; ++i) {
        ReqCountType entry_index = (start_index + i) % NR_OF_REQS ;
        reqsVector[i] = in_req_fifo[entry_index].nb_can_get() ;
      }
      ReqIndexType orig_start_index = start_index;
      hasReqs = false;
      bool consumer_ready = false;
      UNROLL_N_SERVICE_LOOP_MUX: for (unsigned i = 0; i < NR_OF_RSPS; i++)
        if (out_req_fifo[i].nb_can_put()) {
          consumer_ready = true;
          ReqCountType chosen_bit ;
          bool foundRequestor = bitvec_utils<ReqCountType, NR_OF_REQS>::find_leading_one(reqsVector, chosen_bit);
          if (foundRequestor) {
            ReqCountType next_index = (chosen_bit+orig_start_index) % NR_OF_REQS ;

            assert(in_req_fifo[next_index].nb_can_get());
            assert(out_req_fifo[i].nb_can_put());
            REQUEST req;
            in_req_fifo[next_index].nb_get(req);
            //req.io_unit_id = next_index;
            req.io_unit_id = ArbiterTagResolver<TRAIT>::template addTag<NR_OF_REQS>(req.io_unit_id, next_index);
            DBG_OUT << sc_time_stamp() << " arbiter ("<< TRAIT << " " << basename() << " got request from unit " <<  (size_t)req.io_unit_id << " at address: " <<  req.addr << " and send it out to output " << endl;
            out_req_fifo[i].nb_put(req);

            // next time we'll start with the next requester
            start_index = (next_index + 1) % NR_OF_REQS ;
            // reset the bit in the reqsVector as we just serviced it
            reqsVector[chosen_bit] = false;
          }
          hasReqs |= foundRequestor;
        }
      if (!consumer_ready) {
        consumer_not_ready++;
      }
      if (!hasReqs) {
        idle = true;
        idle_counter++;
      }
      wait();
    }
  }

  void demux_thread() {
    DEMUX_THREAD_RESET_LOOP1: for (unsigned int i = 0; i < NR_OF_REQS; i++) {
      out_resp_fifo[i].reset_put();
    }
    DEMUX_THREAD_RESET_LOOP2: for (unsigned int i = 0; i < NR_OF_RSPS; i++) {
      in_resp_fifo[i].reset_get();
    }
    wait();
    while (1) {
      {
        UNROLL_N_SERVICE_LOOP_DEMUX: for (unsigned i = 0; i < NR_OF_RSPS; i++)
        {
          //cout<<"In demux thread..."<<endl;
          if (in_resp_fifo[i].nb_can_get()) {
            RESPONSE resp;
            //SPLResponse resp;
            in_resp_fifo[i].nb_get(resp);
            IOUnitIdType io_unit_id = ArbiterTagResolver<TRAIT>::template extractTag<NR_OF_REQS>(resp.io_unit_id);
            DBG_OUT << sc_time_stamp() << " arbiter " << basename() << " got response for unit " << (size_t)resp.io_unit_id << " and send it out to input " << (unsigned)io_unit_id << endl;
            assert(out_resp_fifo[io_unit_id].nb_can_put());
            out_resp_fifo[io_unit_id].nb_put(resp);
          }
        }
      }

      wait();
    }
  }

  ~MemArbiterN() {
    const char* name = basename();
    cout << "ArbiterN " << name << " " << TRAIT << " was idle (not requestors) for " << idle_counter-consumer_not_ready << " cycles, consumer was not ready for "<< consumer_not_ready << " cycles" << endl;
  }
  struct ArbiterRequestorChannel {
    ga::tlm_fifo<REQUEST> in_req_fifo[NR_OF_REQS];
    ga::tlm_fifo<RESPONSE> out_resp_fifo[NR_OF_REQS];

    template <typename T1, typename T2>
    void bindRequestor (unsigned int i, T1 &req, T2 &resp, MemArbiterN<NR_OF_REQS,NR_OF_RSPS, REQUEST, RESPONSE, TRAIT> &arb) {
      assert(i < NR_OF_REQS);
      req(in_req_fifo[i]);
      resp(out_resp_fifo[i]);
      arb.in_req_fifo[i](in_req_fifo[i]);
      arb.out_resp_fifo[i](out_resp_fifo[i]);
    }

    //    template <typename T>
    //    void bindServicer (T servr, MemArbiter<NR_OF_REQS, REQUEST, RESPONSE, TRAIT> arb) {
    //      ga::tlm_fifo_out<REQUEST> out_req_fifo;
    //      ga::tlm_fifo_in<RESPONSE> in_resp_fifo;
    //      servr.in_req_fifo(in_req_fifo);
    //      servr.out_resp_fifo(out_resp_fifo);
    //      arb.out_req_fifo(in_req_fifo);
    //      arb.in_resp_fifo(out_resp_fifo);
    //    }

  };

private:
  ReqIndexType start_index;
};


template<unsigned int NR_OF_REQS, class REQUEST, class RESPONSE, enum ARBITER_TRAIT TRAIT = IO_ARBITER>
struct MemArbiterNoTag : sc_module {
#ifdef STRATUS
  HLS_INLINE_MODULE;
#endif
public:
  sc_core::sc_in_clk clk;
  sc_core::sc_in<bool> rst;
  sc_out<bool> idle;

  typedef typename SizeT<NR_OF_REQS>::Type ReqCountType;
  ga::tlm_fifo_in<REQUEST> in_req_fifo[NR_OF_REQS];
  ga::tlm_fifo_out<REQUEST> out_req_fifo;
  ga::tlm_fifo_in<RESPONSE> in_resp_fifo;
  ga::tlm_fifo_out<RESPONSE> out_resp_fifo[NR_OF_REQS];

  SC_HAS_PROCESS (MemArbiterNoTag);

  MemArbiterNoTag(sc_core::sc_module_name name = sc_core::sc_gen_unique_name(
      "ARBITER")) :
        sc_core::sc_module(name), clk("clk"), rst("rst"), idle("idle"), out_req_fifo("out_req_fifo"), in_resp_fifo("in_resp_fifo") {
    SC_CTHREAD(mux_thread,clk.pos());
    //sensitive << clk.pos();
    async_reset_signal_is(rst, false);
    SC_CTHREAD(demux_thread,clk.pos());
    //sensitive << clk.pos();
    async_reset_signal_is(rst, false);

    for (unsigned int i = 0; i < NR_OF_REQS; i++) {
      in_req_fifo[i].clk_rst(clk, rst);
      out_resp_fifo[i].clk_rst(clk, rst);
    }
    out_req_fifo.clk_rst(clk, rst);
    in_resp_fifo.clk_rst(clk, rst);
  }
  size_t idle_counter;
  void mux_thread() {

    start_index = 0;
    idle_counter = 0;
    out_req_fifo.reset_put();
    idle = true;
    bool bitVector[NR_OF_REQS] ;
    REQUEST req_buf[NR_OF_REQS];
    bool req_valid[NR_OF_REQS] ;
    typename SizeT<NR_OF_REQS+1>::Type remaining = 0 ;
    MUX_THREAD_RESET_LOOP: for (unsigned i = 0; i < NR_OF_REQS; i++) {
      in_req_fifo[i].reset_get();
      req_valid[i] = false;
    }
    wait();
    while (1) {

      ReqCountType chosen_bit ;
      bool foundRequestor = bitvec_utils<ReqCountType, NR_OF_REQS>::find_leading_one(req_valid, chosen_bit);
      assert(foundRequestor == (remaining != 0));
      bool last_one = ((out_req_fifo.nb_can_put() && remaining == 1) || remaining == 0);
      if (out_req_fifo.nb_can_put() && remaining != 0) {
        ReqCountType next_index = chosen_bit;
        //ReqCountType next_index = (chosen_bit+start_index) % NR_OF_REQS ;
        assert(req_valid[next_index]);
        req_valid[next_index] = 0;
        out_req_fifo.nb_put(req_buf[next_index]);
        // next time we'll start with the next requester
        //start_index = (next_index + 1) % NR_OF_REQS ;
        remaining--;
      }
      if (last_one) {
        remaining = 0;
        SELECT_ENTRY_RB_UNROLL: for (unsigned i=0; i < NR_OF_REQS; ++i) {
          req_valid[i] = in_req_fifo[i].nb_get(req_buf[i]) ;
        }
        remaining = bitvec_utils<typename SizeT<NR_OF_REQS+1>::Type, NR_OF_REQS>::add (req_valid);
      }


      if (idle) idle_counter++;
      wait();
    }
  }

  void demux_thread() {
    in_resp_fifo.reset_get();
    DEMUX_THREAD_RESET_LOOP: for (unsigned int i = 0; i < NR_OF_REQS; i++) {
      out_resp_fifo[i].reset_put();
    }
    unsigned int rsp_index = 0;
    bool pend_resp_valid = false;
    RESPONSE pend_resp;
    IOUnitIdType io_unit_id;
    wait();
    while (1) {
      {
        //bool out_resp_can_put = out_resp_fifo[io_unit_id].nb_can_put();
        // Arbiter output never pushes back. Do assert that on the consumer ready in RTL.
        bool out_resp_can_put = true;
        bool stage2_en = !pend_resp_valid || out_resp_can_put;

        if (pend_resp_valid && out_resp_can_put) {
          out_resp_fifo[io_unit_id].nb_put(pend_resp);
          pend_resp_valid = false;
        }

        if (in_resp_fifo.nb_can_get() && stage2_en) {
          in_resp_fifo.nb_get(pend_resp);
          pend_resp_valid = true;
          io_unit_id = ArbiterTagResolver<TRAIT>::template extractTag<NR_OF_REQS>(pend_resp.io_unit_id);
          DBG_OUT << sc_time_stamp() << " arbiter " << basename() << " got response for unit " << (size_t)pend_resp.io_unit_id << " and send it out to input " << endl;
        }
      }
      wait();
    }
  }
  void demux_thread_nobackpressure() {
    in_resp_fifo.reset_get();
    DEMUX_THREAD_RESET_LOOP: for (unsigned int i = 0; i < NR_OF_REQS; i++) {
      out_resp_fifo[i].reset_put();
    }
    unsigned int rsp_index = 0;
    bool pend_resp_valid = false;
    RESPONSE pend_resp;
    IOUnitIdType io_unit_id;
    wait();
    while (1) {
      {
        if (in_resp_fifo.nb_can_get()) {
          in_resp_fifo.nb_get(pend_resp);
          io_unit_id = ArbiterTagResolver<TRAIT>::template extractTag<NR_OF_REQS>(pend_resp.io_unit_id);
          out_resp_fifo[io_unit_id].nb_put(pend_resp);

          DBG_OUT << sc_time_stamp() << " arbiter " << basename() << " got response for unit " << (size_t)pend_resp.io_unit_id << " and send it out to input " << endl;
        }
      }
      wait();
    }
  }
  ~MemArbiterNoTag() {
    const char* name = basename();
    cout << "ArbiterNoTag " << name << " " << TRAIT << " was idle (not requestors) for " << idle_counter << " cycles" << endl;
  }

private:
  unsigned int start_index;
};


#endif /* ACC_MEM_ARBITER_H_ */
