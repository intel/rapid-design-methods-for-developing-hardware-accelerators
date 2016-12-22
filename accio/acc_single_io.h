// See LICENSE for license details.
/*
 * acc_single_io.h
 *
 *  Created on: May 25, 2016
 *      Author: aayupov
 */

#ifndef ACC_SINGLE_IO_H_
#define ACC_SINGLE_IO_H_


#include "systemc.h"
#include "ga_tlm_fifo.h"

#include "acc_rename.h"

//#include "acc_rename.h"

template <typename T, typename UTAG, size_t REQ_BUF_SIZE>
class AccIn<LoadUnitSingleReqParams<T, UTAG, REQ_BUF_SIZE> > : public sc_module {
public:
  struct OutstandingRequestType {
    CacheLineType::ByteIndexType cl_offset;
    UTAG utag;
    OutstandingRequestType() {}
    OutstandingRequestType(CacheLineType::ByteIndexType addr_offset, UTAG tag) :
      cl_offset(addr_offset), utag(tag)
    {
    }

    inline friend void sc_trace(sc_trace_file* tf,
        const OutstandingRequestType& d, const std::string& name) {
    }

    inline friend std::ostream& operator<<(std::ostream& os,
        const OutstandingRequestType& d) {
      os << "offset: " << d.cl_offset << " ";
      os << "tag: " << d.utag << std::endl;
      return os;
    }

    inline bool operator==(const OutstandingRequestType& rhs) {
      bool result = true;
      result = result && (cl_offset == rhs.cl_offset);
      result = result && (utag == rhs.utagStoreUnitSingleReqParams);
      return result;
    }
  };
  struct OutstandingResponseType {
    T data;
    SplTag tag;
    OutstandingResponseType() {}
    OutstandingResponseType(const T& data, SplTag tag) :
      data(data), tag(tag)
    {
    }

    inline friend void sc_trace(sc_trace_file* tf,
        const OutstandingResponseType& d, const std::string& name) {
    }

    inline friend std::ostream& operator<<(std::ostream& os,
        const OutstandingResponseType& d) {
      os << "data: " << d.data << " ";
      os << "tag: " << d.tag << std::endl;
      return os;
    }

    inline bool operator==(const OutstandingResponseType& rhs) {
      bool result = true;
      result = result && (data == rhs.data);
      result = result && (tag == rhs.tag);
      return result;
    }
  };

  typedef T AccDataType;

  sc_in_clk clk;
  sc_in<bool> rst;

  // functional ports
  ga::tlm_fifo_out<SplMemReadReqType> spl_req_out;
  ga::tlm_fifo_in<SplMemReadRespType> spl_resp_in;

  ga::tlm_fifo_out<MemSingleReadRespType<AccDataType, UTAG> > acc_resp_out;
  ga::tlm_fifo_in<MemSingleReadReqType<AccDataType, UTAG> > acc_req_in;

  enum {
    OUTSTANDING_BUFFER_SIZE = REQ_BUF_SIZE
  };

  // this fifo takes care of indices of available entries in response_data_array
  ga::ga_storage_fifo<OutstandingRequestToken<OUTSTANDING_BUFFER_SIZE>, OUTSTANDING_BUFFER_SIZE> available_slots;

  //scide_waive SCIDE.8.2
  //scide_waive SCIDE.6.2
  // indexed by OutstandingRequestToken.tag
  ga::ga_storage_fifo<OutstandingResponseType, OUTSTANDING_BUFFER_SIZE> response_data_fifo;
  OutstandingRequestType request_tag_array[OUTSTANDING_BUFFER_SIZE];


  SC_HAS_PROCESS(AccIn);

  AccIn(sc_module_name name) :
    clk("clk"), rst(
        "rst"),spl_req_out("spl_req_out"), spl_resp_in("spl_resp_in"), acc_resp_out(
            "acc_resp_out"), acc_req_in("acc_req_in") {


    SC_CTHREAD(request_handler, clk.pos());
    async_reset_signal_is(rst, false);
    SC_CTHREAD(spl_response_handler, clk.pos());
    async_reset_signal_is(rst, false);
    SC_CTHREAD(acc_response_handler, clk.pos());
    async_reset_signal_is(rst, false);

    spl_req_out.clk_rst(clk, rst);
    spl_resp_in.clk_rst(clk, rst);
    acc_resp_out.clk_rst(clk, rst);
    acc_req_in.clk_rst(clk, rst);
  }

  size_t _full_counter;
  size_t _no_outstanding_reqs;
  size_t _consumer_not_ready;

  void request_handler() {
    {
      spl_req_out.reset_put();
      acc_req_in.reset_get();
      available_slots.reset_get();
    }
    _no_outstanding_reqs = 0;
    _full_counter = 0;

    wait();
    while (1) {
      {
        if (!available_slots.nb_can_get())
          _full_counter++;

        if (!acc_req_in.nb_can_get())
          _no_outstanding_reqs++;

        if (spl_req_out.nb_can_put() && acc_req_in.nb_can_get() && available_slots.nb_can_get()) {
          MemSingleReadReqType<T, UTAG> mem_req;
          acc_req_in.nb_get(mem_req);
          // make sure the bit cnt is integer of cache size
          assert (CacheLineType::getBitCnt() >= T::getBitCnt() && CacheLineType::getBitCnt() % T::getBitCnt() == 0);
          CacheLineType::ByteIndexType addr_offset = CacheLineType::getCacheLineOffset(mem_req.addr);
          AddressType cache_aligned_addr = CacheLineType::getCacheAlignedAddress(mem_req.addr);


          OutstandingRequestToken<OUTSTANDING_BUFFER_SIZE> req_token;
          // getting next available row
          available_slots.nb_get(req_token);
          // add token to request queue
          request_tag_array[req_token.tag] = OutstandingRequestType(addr_offset, mem_req.utag);

          // send request to memory
          SplMemReadReqType spl_req;
          spl_req.addr = cache_aligned_addr;
          spl_req.io_unit_id = 0;
          spl_req.tag = req_token.tag;

          bool put_result = spl_req_out.nb_put(spl_req);
          // should always succeed as we check nb_can_put in the condition
          assert(put_result);
        } else {
          //cout << "smth FULL " <<  spl_req_out.nb_can_put() << " "<<  acc_req_in.nb_can_get() << " "<< available_slots.nb_can_get() << endl;
        }
      }
      wait();
    }
  }

  void spl_response_handler() {
    {
      response_data_fifo.reset_put();
      spl_resp_in.reset_get();
    }
    wait();
    while (1) {
      {
        if (spl_resp_in.nb_can_get()) {
          SplMemReadRespType spl_resp;
          spl_resp_in.nb_get(spl_resp);
          DBG_OUT << sc_time_stamp() << " AccSingleIn:response_handler SPL response received " << spl_resp.data.words[0]<< endl;
          assert(response_data_fifo.nb_can_put());

          // we read from SPL to write to the response_data_fifo
          //extract first
          CacheLineMarshal<T::BitCnt> cl_marsh;
          CacheLineType::IndexType offset = 8*request_tag_array[spl_resp.tag].cl_offset;
          assert(offset%T::BitCnt == 0);
          cl_marsh.start(spl_resp.data, offset/T::BitCnt);
          T data;
          cl_marsh.getSlice(data);
          cl_marsh.finish();
          response_data_fifo.nb_put(OutstandingResponseType(data, spl_resp.tag));
        }
      }
      wait();
    }
  }

  void acc_response_handler() {
    size_t available_slots_index = OUTSTANDING_BUFFER_SIZE;
    {
      _consumer_not_ready = 0;
      response_data_fifo.reset_get();
      acc_resp_out.reset_put();
      available_slots.reset_put();
    }
    MemSingleReadRespType<AccDataType, UTAG> resp;
    bool resp_valid = false;
    wait();
    ACC_SINGLE_RESP_HANDLER_WHILE: while (1) {
      {
        if (resp_valid) {
          // return credit as we fully serviced the request at this tag
          acc_resp_out.put(resp);
          resp_valid = false;
        }
        // multi cycle reset modeled here to fill up a fifo with credits
        if (available_slots_index != 0) {
          assert(available_slots.nb_can_put());
          --available_slots_index;
          available_slots.nb_put(OutstandingRequestToken<OUTSTANDING_BUFFER_SIZE>(available_slots_index));
        } else {
          if (response_data_fifo.nb_can_get()) {
            OutstandingResponseType resp_data;
            response_data_fifo.nb_get(resp_data);
            UTAG utag = request_tag_array[resp_data.tag].utag;
            resp.data = resp_data.data;
            resp.utag = utag;
            resp_valid = true;
            assert(available_slots.nb_can_put());
            available_slots.nb_put(OutstandingRequestToken<OUTSTANDING_BUFFER_SIZE>(resp_data.tag));
          }
        }
        wait();
      }
    }
  }

  ~AccIn() {
    const char* name = basename();
    cout << "ACCIn " << name << " " << " was full (all request slots taken) for " << _full_counter << " cycles" << endl;
    cout << "AccIn " << name << " stats (idle now): " <<
        " no outstanding requests = " << _no_outstanding_reqs <<
        " acc not ready to receive " << _consumer_not_ready << endl;

  }


  struct ChannelToArbiter {
    ga::tlm_fifo<SplMemReadReqType> spl_req_ch;
    ga::tlm_fifo<SplMemReadRespType> spl_resp_ch;

    ChannelToArbiter() :
      spl_req_ch("spl_req_ch"), spl_resp_ch("spl_resp_ch") {}

    template <size_t NUM_OF_READERS>
    void bindArbiter(MemArbiter<NUM_OF_READERS, SplMemReadReqType, SplMemReadRespType> &arb, size_t arb_index, AccIn &acc_in) {
      assert(arb_index < NUM_OF_READERS);
      arb.in_req_fifo[arb_index](spl_req_ch);
      arb.out_resp_fifo[arb_index](spl_resp_ch);
      acc_in.spl_req_out(spl_req_ch);
      acc_in.spl_resp_in(spl_resp_ch);
    }
  };

};

// only implemented to handle cacheline worth types (partial writes should be handled by a load/store unit that handles both reads and writes to the same address)
template <typename T, typename UTAG, size_t REQ_BUF_SIZE>
class AccOut<StoreUnitSingleReqParams<T, UTAG, REQ_BUF_SIZE> > : public sc_module {
public:
  // clk/rst
  sc_in_clk clk;
  sc_in<bool> rst;
  //sc_out<bool> idle;
  // functional ports
  ga::tlm_fifo_in<MemSingleWriteReqType<T, UTAG> > acc_req_in;
  ga::tlm_fifo_out<MemSingleWriteRespType<UTAG> > acc_resp_out;
  ga::tlm_fifo_out<SplMemWriteReqType> spl_req_out;
  ga::tlm_fifo_in<SplMemWriteRespType> spl_resp_in;

  AccRenameBuffer<REQ_BUF_SIZE, UTAG> wr_req_rename_buffer;


  SC_HAS_PROCESS(AccOut);

  //scide_waive SCIDE.8.15
  AccOut(sc_module_name name=sc_gen_unique_name("AccOut")) :
    clk("clk"),rst("rst"),acc_req_in("acc_req_in"),acc_resp_out("acc_resp_out"),spl_req_out("spl_req_out"),spl_resp_in("spl_resp_in") {
    SC_CTHREAD(request_handler, clk.pos());
    async_reset_signal_is(rst, false);
    acc_req_in.clk_rst(clk, rst);
    acc_resp_out.clk_rst(clk, rst);
    spl_req_out.clk_rst(clk, rst);
    spl_resp_in.clk_rst(clk, rst);
    // only full cachelines supported in this store unit
    assert(sizeof(T) == 64);
  }

  void request_handler() {
    {
      acc_req_in.reset_get();
      acc_resp_out.reset_put();
      spl_req_out.reset_put();
      spl_resp_in.reset_get();
      wr_req_rename_buffer.reset_free();
      wr_req_rename_buffer.reset_alloc();
    }
    SplMemReadReqType pend_read;
    bool pend_read_valid = false;
    SplMemWriteReqType pend_write;
    bool pend_write_valid = false;
    bool write_conflict = false;
    size_t write_conflict_index = 0;
    bool read_after_write = false;
    size_t read_conflict_index = 0;

    AddressType next_addr = 0;
    SizeInCLType remaining_size = 0;
    wait();
    while (1) {
      {
        bool multi_cycle_reset_done = true;
        if (!wr_req_rename_buffer.multi_cycle_reset())
          multi_cycle_reset_done = false;

        if (multi_cycle_reset_done) {
          if (acc_req_in.nb_can_get() && wr_req_rename_buffer.can_alloc() && spl_req_out.nb_can_put()) {
            MemSingleWriteReqType<T, UTAG> req;
            bool result = acc_req_in.nb_get(req);
            assert(result);

            // send request to spl
            size_t spltag = wr_req_rename_buffer.alloc(req.utag);
            spl_req_out.nb_put(SplMemWriteReqType(req.addr, spltag, req.data));
          }

          if (spl_resp_in.nb_can_get() && acc_resp_out.nb_can_put()) {
            SplMemWriteRespType resp;
            bool result = spl_resp_in.nb_get(resp);
            assert(result);

            UTAG utag = wr_req_rename_buffer.free(resp.tag);
            acc_resp_out.nb_put(MemSingleWriteRespType<UTAG>(utag));
          }
        }
        wait();
      }
    }
  }

};


template <typename T>
class AccInOut {};

#include "acc_single_io_types.h"

#if 1
template <typename T, typename UTAG, size_t NUM_OF_SETS, size_t NUM_OF_WAYS, size_t MSHR_SIZE, size_t SAME_CL_REQ_SIZE>
struct AccTypedCache : sc_module {
  typedef CacheIndexWayPair<NUM_OF_WAYS, NUM_OF_SETS> IndexWayPair;
  struct IndexWayEvict {
    IndexWayPair loc;
    bool evict;
    IndexWayEvict() {}
    IndexWayEvict(IndexWayPair loc, bool evict) :
      loc(loc), evict(evict) {}
  };


  typedef IndexWayMshrIndexTuple<NUM_OF_WAYS, NUM_OF_SETS, MSHR_SIZE> IndexWayMshrIndexType;
  enum {
    TAG_WIDTH = CacheIndexWayPair<NUM_OF_WAYS, NUM_OF_SETS>::TAG_WIDTH
  };

  enum {
    WR_BUF_SIZE = 8
  };

  enum {
    SCHEDULED_RESPONSE_Q_SIZE = 2,
    INPUT_REQ_Q_SIZE = 2
  };
  typedef AccInOutSet<typename SizeT<TAG_WIDTH>::Type, NUM_OF_WAYS, T, UTAG, MSHR_SIZE, SAME_CL_REQ_SIZE> AccInOutSetT;
  typedef AccInOutWay<typename SizeT<TAG_WIDTH>::Type, T, UTAG, MSHR_SIZE, SAME_CL_REQ_SIZE> AccInOutWayT;
  typedef AccRequest<T, UTAG> AccRequestT;
  typedef typename SizeT<MSHR_SIZE>::Type MSHRIndexT;

  AccInOutSetT sets[NUM_OF_SETS];

  // scheduled responses
  ga::ga_storage_fifo<IndexWayMshrIndexType, SCHEDULED_RESPONSE_Q_SIZE> miss_cl_location_q;
  //  ga::ga_storage_fifo<AccRequestT, SCHEDULED_RESPONSE_Q_SIZE> hit_requests;
  //  ga::ga_storage_fifo<IndexWayPair, SCHEDULED_RESPONSE_Q_SIZE> hit_cl_location_q;
  AccRequestT hit_request;
  IndexWayPair hit_cl_location;
  bool hit_request_valid;

  // this fifo takes care of slots with request for cache misses
  ga::ga_storage_fifo<typename SizeT<MSHR_SIZE>::Type, MSHR_SIZE> available_pending_q_slots;
  // addressed by the credit token from available_slots queue
  ga::ga_storage_fifo<AccRequestT, SAME_CL_REQ_SIZE> mshr_pending_request_queue[MSHR_SIZE];
  MshrRecord<NUM_OF_SETS, NUM_OF_WAYS> mshr_records[MSHR_SIZE];

  sc_in_clk clk;
  sc_in<bool> rst;
  //from/to accelerator
  ga::tlm_fifo_in<MemSingleReadReqType<T,UTAG> > acc_rd_req_in;
  ga::tlm_fifo_in<MemSingleWriteReqType<T,UTAG> > acc_wr_req_in;
  ga::tlm_fifo_out<MemSingleWriteRespType<UTAG> > acc_wr_resp_out;
  ga::tlm_fifo_out<MemSingleReadRespType<T,UTAG> > acc_rd_resp_out;
  // to/from mshr
  ga::tlm_fifo_out<MemSingleReadReqType<CacheLineType, MSHRIndexT> > cl_rd_req_out;
  ga::tlm_fifo_in<MemSingleReadRespType<CacheLineType, MSHRIndexT> > cl_rd_resp_in;
  // goes directly to spl, block until the wr ack comes back, so no ack needed at upper levels
  ga::tlm_fifo_out<MemSingleWriteReqType<CacheLineType, UTAG> > cl_wr_req_out;
  ga::tlm_fifo_out<AccReqType> acc_req_out;

  //ga::tlm_fifo_out<SplMemWriteReqType> spl_wr_resp_in;
  SC_HAS_PROCESS(AccTypedCache);

  AccTypedCache(sc_module_name name) :
    clk("clk"), rst("rst"), acc_rd_req_in("acc_rd_req_in"), acc_wr_req_in(
        "acc_wr_req_in"), acc_rd_resp_out("acc_rd_resp_out"), cl_rd_req_out(
            "cl_rd_req_out"), cl_rd_resp_in("cl_rd_resp_in"), cl_wr_req_out(
                "cl_wr_req_out") {
    SC_CTHREAD(main_proc, clk.pos());
    async_reset_signal_is(rst, false);
    acc_rd_req_in.clk_rst(clk,rst);
    acc_wr_req_in.clk_rst(clk,rst);
    acc_rd_resp_out.clk_rst(clk,rst);
    cl_rd_req_out.clk_rst(clk,rst);
    cl_rd_resp_in.clk_rst(clk,rst);
    cl_wr_req_out.clk_rst(clk,rst);
    //spl_wr_resp_in.clk_rst(clk,rst);
    acc_req_out.clk_rst(clk, rst);
    acc_wr_resp_out.clk_rst(clk,rst);
  }

  T extractDataFromCL(CacheLineType cl, CacheLineType::IndexType offset) {
    CacheLineMarshal<T::BitCnt> cl_marsh;
    assert(offset%T::BitCnt == 0);
    cl_marsh.start(cl, offset/T::BitCnt);
    T data;
    cl_marsh.getSlice(data);
    cl_marsh.finish();
    return data;
  }

  void sliceDataInCL(CacheLineType& cl, CacheLineType::IndexType offset, const T& data) {
    CacheLineMarshal<T::BitCnt> cl_marsh;
    assert(offset%T::BitCnt == 0);
    cl_marsh.start(cl, offset/T::BitCnt);
    cl_marsh.putSlice(data);
    cl_marsh.finish();
    cl = cl_marsh.getCL();
  }

  //sc_signal<bool> eviction_needed;
  bool eviction_needed;

  size_t num_of_evicts;
  size_t num_of_hits;
  size_t num_of_misses;
  size_t num_of_mshr_hits;
  size_t num_inp_stalls;
  enum {STALL_ITEMIZED=6};
  size_t num_stall_itemzed[STALL_ITEMIZED];
  void main_proc() {
    // last stage state, write request
    CacheLineWriteRequest<NUM_OF_SETS,NUM_OF_WAYS> next_wr_req;
    bool next_wr_req_valid = false;
    ////
    // second stage, mshr_rsp flopped version
    MemSingleReadRespType<CacheLineType, MSHRIndexT> mshr_rsp;
    typename IndexWayPair::IndexType mshr_rsp_index = 0;
    typename IndexWayPair::TagType mshr_rsp_tag = 0;
    bool mshr_rsp_valid = false;
    ////
    // stage 3 next_req state
    CacheLineType next_acc_req_cl;
    AccRequestT next_acc_req;
    IndexWayPair next_acc_req_cloc;
    bool next_acc_req_valid = false;
    bool write_to_llm = false;
    ////
    // stage 3 eviction state
    eviction_needed = false;
    //bool eviction_scheduled = false;
    IndexWayPair eviction_loc;
    ////

    bool last_acc_req_was_write = false;
    // stage 1: register the read request
    MemSingleReadWriteReqType<T, UTAG> inp_req;
    bool inp_req_valid = false;
    ////
    // MSHR pending request for MSHR hit
    AccRequestT mshr_pending_req;
    bool mshr_pending_req_valid = false;
    MSHRIndexT mshr_pending_req_index;
    ////
    // last stage, eviction completion state
    IndexWayPair complete_eviction_loc;
    bool need_to_complete_eviction = false;
    ////
    {
      acc_rd_resp_out.reset_put();
      cl_rd_req_out.reset_put();
      cl_rd_resp_in.reset_get();
      cl_wr_req_out.reset_put();
      acc_req_out.reset_put();
      RESET_REQ_UNROLL1: for(unsigned int i = 0; i < NUM_OF_SETS; ++i) {
        RESET_REQ_UNROLL2: for(unsigned int j = 0; j < NUM_OF_WAYS; ++j) {
          sets[i].ways[j].data.valid = false;
        }
      }
      RESET_REQ_Q_UNROLL: for(unsigned int i = 0; i < MSHR_SIZE; ++i) {
        mshr_pending_request_queue[i].reset_put();
        mshr_pending_request_queue[i].reset_get();
      }
      RESET_MSHR_RECORDS_UNROLL: for(unsigned int i = 0; i < MSHR_SIZE; ++i) {
        mshr_records[i].reset();
      }

      acc_rd_req_in.reset_get();
      acc_wr_req_in.reset_get();
      acc_wr_resp_out.reset_put();
      hit_request_valid = false;
      //      hit_cl_location_q.reset_get();
      //      hit_cl_location_q.reset_put();
      //      hit_requests.reset_put();
      //      hit_requests.reset_get();
      miss_cl_location_q.reset_put();
      miss_cl_location_q.reset_get();
      available_pending_q_slots.reset_get();
      available_pending_q_slots.reset_put();
    }
    size_t avail_pending_q_reset_count = MSHR_SIZE;
    typename IndexWayPair::WayIndexType mshr_found_way = 0;
    num_of_evicts = num_of_misses = num_of_hits = num_of_mshr_hits = 0;
    num_inp_stalls = 0;
    RESET_INIT_COUNTERS_UNROLL: for(size_t i = 0; i < 6; ++i) {
      num_stall_itemzed[i] = 0;
    }
    wait();
    MAIN_PROC_WHILE: while (1) {
      {
        bool acc_req_tx = false;
        bool multi_cycle_reset_done = true;
        //credit fifo reset
        if (avail_pending_q_reset_count) {
          bool success = available_pending_q_slots.nb_put(--avail_pending_q_reset_count);
          multi_cycle_reset_done = false;
          assert(success);
        }



        if (multi_cycle_reset_done) {

          // early miss_cl_location read for the miss response servicing. needed to not depend on the new write to miss_cl_location
          IndexWayMshrIndexType indway_mshrind;
          bool miss_cl_location_nb_can_get = miss_cl_location_q.nb_can_get();
          bool success = miss_cl_location_q.nb_peek(indway_mshrind);
          assert(success == miss_cl_location_nb_can_get);
          ///


          // handles write transactions and has support for eviction writes
          if (next_wr_req_valid && cl_wr_req_out.nb_can_put() && acc_req_out.nb_can_put()) {
            next_wr_req_valid = false;
            SplAddressType addr = IndexWayPair::computeSplAddress(next_wr_req.index, next_wr_req.tag);
            DBG_OUT << sc_time_stamp() << "cl_wr_req_out: at="<< addr << " data " << next_wr_req.cl << endl;
            cl_wr_req_out.nb_put(MemSingleWriteReqType<CacheLineType, UTAG>(addr, next_wr_req.cl));
            acc_req_out.nb_put(ACC_REQ_WRITE);
            acc_req_tx = true;
          }
          // if it is an eviction and no write needed, we release the slot immediately
          if (!next_wr_req_valid && need_to_complete_eviction) {
            need_to_complete_eviction = false;
            sets[complete_eviction_loc.index].ways[complete_eviction_loc.way].data.valid = false;
            DBG_OUT << sc_time_stamp() << "complete_eviction_loc" << endl;

          }

          last_acc_req_was_write = false;
          // stage where we are servicing read request or modify a cache line and scheduling write request to the cache (sync write) and invalidate a cache block in case of evict
          if (next_acc_req_valid && !next_wr_req_valid) {
            // generate write to low-level memory and invalidate the cache location
            if (next_acc_req.is_evict) {
              if (sets[next_acc_req_cloc.index].ways[next_acc_req_cloc.way].data.state == AccInOutWayT::TagRecordT::MODIFIED) {
                write_to_llm = true;
              } else {
                write_to_llm = false;
              }
              need_to_complete_eviction = true;
              complete_eviction_loc = next_acc_req_cloc;
            } else if (next_acc_req.is_write) {
              // slice in the data and change state
              sliceDataInCL(next_acc_req_cl,8*next_acc_req.word_pos,next_acc_req.data);
              sets[next_acc_req_cloc.index].ways[next_acc_req_cloc.way].data.cl = next_acc_req_cl;
              DBG_OUT << sc_time_stamp() << "PWRITE: at=" << IndexWayPair::computeSplAddress(next_acc_req_cloc.index, sets[next_acc_req_cloc.index].ways[next_acc_req_cloc.way].data.tag) << " next_acc_req_cl=" << next_acc_req_cl << " word, data=" << 8*next_acc_req.word_pos << ", " << std::hex << next_acc_req.data<< std::dec<< endl;
              sets[next_acc_req_cloc.index].ways[next_acc_req_cloc.way].data.state = AccInOutWayT::TagRecordT::MODIFIED;
              last_acc_req_was_write = true;
              if (write_to_llm) {
                sets[next_acc_req_cloc.index].ways[next_acc_req_cloc.way].data.state = AccInOutWayT::TagRecordT::UP_TO_DATE;
              }
            } else {
              // if it is a read
              T data = extractDataFromCL(next_acc_req_cl,8*next_acc_req.word_pos);
              acc_rd_resp_out.nb_put( MemSingleReadRespType<T,UTAG> (data, next_acc_req.utag));
            }
            if (write_to_llm) {
              next_wr_req_valid = true;
              next_wr_req.index = next_acc_req_cloc.index;
              next_wr_req.way = next_acc_req_cloc.way;
              next_wr_req.tag = sets[next_acc_req_cloc.index].ways[next_acc_req_cloc.way].data.tag;
              next_wr_req.cl = next_acc_req_cl;
            }
            next_acc_req_valid = false;
          }


          // stage where we read a cache line from the cache to service either read or write request from either miss or hit queue (sync read)
          if (!next_acc_req_valid) {

            AccRequestT next_req;
            IndexWayPair indtag;
            write_to_llm = false;
            // arbitration between hit and miss. for now give hit a priority
            if (hit_request_valid) {
              indtag = hit_cl_location;
              next_acc_req = hit_request;
              next_acc_req_valid = true;
              write_to_llm = next_acc_req.is_write;
              hit_request_valid = false;
            } else if (miss_cl_location_nb_can_get) {
              assert(miss_cl_location_q.nb_can_peek());
              //bool success = miss_cl_location_q.nb_peek(indway_mshrind);
              //assert(success);
              indtag = IndexWayPair(indway_mshrind.index_way);
              MSHRIndexT pending_q_index = indway_mshrind.mshr_index;

              // should be at least one request who initiated this cl read
              if(mshr_pending_request_queue[pending_q_index].nb_can_get()) {
                mshr_pending_request_queue[pending_q_index].nb_get(next_acc_req);
                next_acc_req_valid = true;
                write_to_llm = next_acc_req.is_write;
              }
            }
            if (next_acc_req_valid) {
              // we need to fast-forward write to this stage from prev stage as it would take one cycle to propagate otherwise (of course it is for the same location only)
              if (last_acc_req_was_write && indtag == next_acc_req_cloc) {
                // already has the right data
                //next_acc_req_cl = next_acc_req_cl;
              } else {
                next_acc_req_cl = sets[indtag.index].ways[indtag.way].data.cl;
              }
              next_acc_req_cloc = indtag;
            }
          }

          bool new_mshr_req_added2 = false;
          MSHRIndexT mshr_added_index2 = -1;

          // mshr pending request scheduled in the last cycle being added to the queue next
          if (mshr_pending_req_valid && mshr_pending_request_queue[mshr_pending_req_index].nb_can_put()) {
            bool success = mshr_pending_request_queue[mshr_pending_req_index].nb_put(mshr_pending_req);
            assert(success);
            mshr_pending_req_valid = false;
            new_mshr_req_added2 = true;
            mshr_added_index2 = mshr_pending_req_index;
          }

          if ((acc_req_tx || !acc_req_out.nb_can_put()))
            num_stall_itemzed[0]++;

          if (!cl_rd_req_out.nb_can_put())
            num_stall_itemzed[1]++;

          if (hit_request_valid)
            num_stall_itemzed[2]++;

          if (mshr_pending_req_valid)
            num_stall_itemzed[3]++;

          if (!available_pending_q_slots.nb_can_get())
            num_stall_itemzed[4]++;

          if (eviction_needed && !hit_request_valid)
            num_stall_itemzed[5]++;

          // stage 2 where we process input request (registered in stage 1) or handle eviction and not reading anything from the input
          bool new_mshr_req_added = false;
          MSHRIndexT mshr_added_index = 0;
          // eviction will flush the pipeline, no more requests from input serviced
          if (eviction_needed && !hit_request_valid) {
            eviction_loc = IndexWayPair(mshr_records[mshr_rsp.utag].index, mshr_found_way);
            //hit_cl_location_q.nb_put(eviction_loc);
            hit_request_valid = true;
            hit_cl_location = eviction_loc;
            //assert(hit_requests.nb_can_put());
            AccRequestT evict_req;
            evict_req.is_evict = true;
            hit_request = evict_req;
            eviction_needed = false;
            num_of_evicts++;
          } else if (inp_req_valid && (!acc_req_tx && acc_req_out.nb_can_put()) && cl_rd_req_out.nb_can_put() && !hit_request_valid && !mshr_pending_req_valid && available_pending_q_slots.nb_can_get()) {
            MemSingleReadWriteReqType<T,UTAG> rdwr_req = inp_req;
            typename IndexWayPair::IndexType index = IndexWayPair::getIndex(rdwr_req.addr);
            typename IndexWayPair::TagType tag = IndexWayPair::getTag(rdwr_req.addr);
            typename IndexWayPair::TagType word_index = IndexWayPair::getWordIndex(rdwr_req.addr);

            assert(index < NUM_OF_SETS);
            typename IndexWayPair::WayIndexType way_index = 0;
            bool being_evicted = false;
            bool found = sets[index].findWayWithMatchingTag(tag, way_index, being_evicted);
            //typename AccInOutWayT::TagRecordT::RecordState state = sets[index].ways[way_index].data.state;
            //bool has_pending_reqs = sets[index].ways[way_index].data.has_pending_reqs;
            MSHRIndexT pending_q_index = 0;
            bool isInMshr = checkMshrRecords(index, tag, pending_q_index);
            new_mshr_req_added = isInMshr;
            mshr_added_index = pending_q_index;

            if (found) { // hit
              if (being_evicted) {
                //under eviction, so have to stall

                DBG_OUT << "EVICT_PENDING" << endl;
              } else if (isInMshr) {// may be processing requests accumulated from miss
                //prepare request to be added to the pending request queue next cycle
                mshr_pending_req_valid = true;
                mshr_pending_req_index = pending_q_index;
                mshr_pending_req = AccRequestT(rdwr_req.utag, rdwr_req.is_write,word_index, rdwr_req.data);
                inp_req_valid = false;
              } else {
                // found matching tag
                IndexWayPair indtag (index, way_index);
                hit_request_valid = true;
                hit_cl_location = indtag;
                hit_request = AccRequestT(rdwr_req.utag, rdwr_req.is_write, word_index, rdwr_req.data);
                inp_req_valid = false;
              }
              num_of_hits++;
            } else { // miss
              inp_req_valid = false;
              // check if the miss is in mshr already
              if (isInMshr) {
                mshr_pending_req_index = pending_q_index;
                num_of_mshr_hits++;
              } else  { // new miss
                MSHRIndexT pend_index;
                available_pending_q_slots.nb_get(pend_index);
                mshr_records[pend_index] = MshrRecord<NUM_OF_SETS, NUM_OF_WAYS>(index, tag);
                DBG_OUT << sc_time_stamp() << "new MSHR req at " << rdwr_req.addr << endl;

                cl_rd_req_out.nb_put(MemSingleReadReqType<CacheLineType, MSHRIndexT>(CacheLineType::getCacheAlignedAddress(rdwr_req.addr),pend_index));
                acc_req_out.nb_put(ACC_REQ_READ);
                num_of_misses++;
                mshr_pending_req_index = pend_index;
              }
              //waiting for read response, or has pending requests, prepare request to be added to the pending request queue next cycle
              mshr_pending_req_valid = true;
              mshr_pending_req = AccRequestT(rdwr_req.utag, rdwr_req.is_write, word_index,rdwr_req.data);

            }
          }

          bool evict = false;
          if (mshr_rsp_valid && !eviction_needed) {
            //bool set_has_slots = (eviction_scheduled) ? false : hasAvailableSlotInIndex(mshr_rsp_index,mshr_found_way, evict);
            bool set_has_slots = hasAvailableSlotInIndex(mshr_rsp_index,mshr_found_way, evict);
            eviction_needed = evict;
            IndexWayPair indway = IndexWayPair(mshr_rsp_index, mshr_found_way);
            if (evict) {
              assert(!sets[indway.index].ways[mshr_found_way].data.being_evicted);
              sets[indway.index].ways[mshr_found_way].data.being_evicted = true;
            } else if (set_has_slots && miss_cl_location_q.nb_can_put()) {
              sets[indway.index].ways[mshr_found_way].data.cl = mshr_rsp.data;
              sets[indway.index].ways[mshr_found_way].data.state = AccInOutWayT::TagRecordT::UP_TO_DATE;
              sets[indway.index].ways[mshr_found_way].data.valid = true;
              sets[indway.index].ways[mshr_found_way].data.being_evicted = false;
              sets[indway.index].ways[mshr_found_way].data.tag = mshr_rsp_tag;

              DBG_OUT << "MSHR WRITE: at=" << IndexWayPair::computeSplAddress(indway.index, mshr_rsp_tag) << " next_acc_req_cl=" << mshr_rsp.data << endl;

              sets[indway.index].ways[mshr_found_way].data.has_pending_reqs = true;
              miss_cl_location_q.nb_put(IndexWayMshrIndexType(indway, mshr_rsp.utag));
              mshr_rsp_valid = false;
            }
          }
          // need to register it to get index to check for available ways in the index next (hasAvailableSlotInIndex call)
          if (!mshr_rsp_valid && cl_rd_resp_in.nb_can_get()) {
            cl_rd_resp_in.nb_get(mshr_rsp);
            mshr_rsp_valid = true;
            mshr_rsp_index = mshr_records[mshr_rsp.utag].index;
            mshr_rsp_tag = mshr_records[mshr_rsp.utag].tag;
          }
          // check if we pulled the last element from the pending request queue before and no one added another one from the input
          // if this is the last pending request we change has_pending_request and release MSHR credit
          if (miss_cl_location_nb_can_get) {
            MSHRIndexT pending_q_index = indway_mshrind.mshr_index;
            bool has_pending_reqs = mshr_pending_request_queue[pending_q_index].nb_can_get()
                      || (new_mshr_req_added && mshr_added_index == pending_q_index)
                      || (new_mshr_req_added2 && mshr_added_index2 == pending_q_index);
            if(!has_pending_reqs ) {
              // no more outstanding requests for this read pending slot
              IndexWayMshrIndexType tmptag;
              bool success = miss_cl_location_q.nb_get(tmptag);
              assert(success);
              IndexWayPair indtag = indway_mshrind.index_way;
              // no more pending. this means one can evict this line.
              sets[indtag.index].ways[indtag.way].data.has_pending_reqs = false;
              // return credit on the pending queues
              success = available_pending_q_slots.nb_put(pending_q_index);
              assert(success);
              // free mshr record
              mshr_records[pending_q_index].reset();
            }
          }
          if (inp_req_valid)
            ++num_inp_stalls;

          MemSingleReadReqType<T, UTAG> rd_req;
          MemSingleWriteReqType<T, UTAG> wr_req;
          if (acc_rd_req_in.nb_can_get() && !inp_req_valid) {
            acc_rd_req_in.nb_get(rd_req);
            inp_req = MemSingleReadWriteReqType<T, UTAG> (rd_req);
            inp_req_valid = true;
          } else if (acc_wr_req_in.nb_can_get() && !inp_req_valid && acc_wr_resp_out.nb_can_put() ) {
            acc_wr_req_in.nb_get(wr_req);
            inp_req = MemSingleReadWriteReqType<T, UTAG> (wr_req);
            inp_req_valid = true;
            acc_wr_resp_out.nb_put(MemSingleWriteRespType<UTAG>(wr_req.utag));
          }


        }
      }
      wait();
    }
  }

  void print_stats() {
    cout << "Cache stats for " << basename() <<": "<<endl;
    cout << "Hits = " << num_of_hits <<endl;
    cout << "MSHR Hits = " << num_of_mshr_hits <<endl;
    cout << "Misses = " << num_of_misses <<endl;
    cout << "Evicts = " << num_of_evicts <<endl;
    cout << "Input stalled cycles = " << num_inp_stalls << "("
        << num_stall_itemzed[0] << ", "
        << num_stall_itemzed[1] << ", "
        << num_stall_itemzed[2] << ", "
        << num_stall_itemzed[3] << ", "
        << num_stall_itemzed[4] << ", "
        << num_stall_itemzed[5]
                             << ")" << endl;
  }

  ~AccTypedCache() {
    print_stats();
  }
  //  void read_write_proc() {
  //    {
  //      acc_rd_req_in.reset_get();
  //      acc_wr_req_in.reset_get();
  //      acc_rdwr_req_fifo.reset_put();
  //    }
  //    wait();
  //    while (1) {
  //      {
  //        MemSingleReadReqType<T, UTAG> rd_req;
  //        MemSingleWriteReqType<T, UTAG> wr_req;
  //        bool read = false;
  //        if (acc_rd_req_in.nb_can_get() && acc_rdwr_req_fifo.nb_can_put()) {
  //          acc_rd_req_in.nb_get(rd_req);
  //          MemSingleReadWriteReqType<T, UTAG> rdwr_req(rd_req);
  //          acc_rdwr_req_fifo.nb_put(rdwr_req);
  //          read = true;
  //        } else
  //        if (acc_wr_req_in.nb_can_get() && acc_rdwr_req_fifo.nb_can_put()) {
  //          acc_wr_req_in.nb_get(wr_req);
  //          // check for RW contention in the same cycle
  //          if (read)
  //            assert(wr_req.addr != rd_req.addr);
  //          MemSingleReadWriteReqType<T, UTAG> rdwr_req(wr_req);
  //          acc_rdwr_req_fifo.nb_put(rdwr_req);
  //        }
  //
  //      }
  //      wait();
  //    }
  //  }

  bool checkMshrRecords(typename IndexWayPair::IndexType index, typename IndexWayPair::TagType tag, MSHRIndexT &pending_q_index) {
    bool bitvec[MSHR_SIZE];
    UNROLL_CHECK_MSHR_RECORDS: for (unsigned i = 0; i < MSHR_SIZE; ++i) {
      bitvec[i] =  (mshr_records[i].valid && mshr_records[i].index == index && mshr_records[i].tag == tag);
    }
    return bitvec_utils<MSHRIndexT, MSHR_SIZE>::find_any_one(bitvec,pending_q_index);
  }
  bool hasAvailableSlotInIndex(typename IndexWayPair::IndexType index, typename IndexWayPair::WayIndexType &way, bool &evict) {
    bool tagmatch[NUM_OF_WAYS];
    bool invalid_match[NUM_OF_WAYS];
    typename IndexWayPair::WayIndexType evict_sel = sets[index].evictSelector;
    UNROLL_TAG_SEARCH: for (unsigned i = 0; i < NUM_OF_WAYS; ++i) {
      tagmatch[i] = !sets[index].ways[(evict_sel+i)%NUM_OF_WAYS].data.has_pending_reqs && !sets[index].ways[(evict_sel+i)%NUM_OF_WAYS].data.being_evicted;
    }
    UNROLL_INVALID_SEARCH: for (unsigned i = 0; i < NUM_OF_WAYS; ++i) {
      invalid_match[i] = !sets[index].ways[i].data.valid;
    }
    typename IndexWayPair::WayIndexType chosen_way;
    // first look if find an empty way
    if (!bitvec_utils<typename IndexWayPair::WayIndexType, NUM_OF_WAYS>::find_leading_one(invalid_match,chosen_way)) {
      // if not check if we can evict anything (one that has no pending reqs is a good candidate)
      if (bitvec_utils<typename IndexWayPair::WayIndexType, NUM_OF_WAYS>::find_leading_one(tagmatch,chosen_way)) {
        chosen_way = (evict_sel + chosen_way)%NUM_OF_WAYS;
        sets[index].evictSelector = (chosen_way+1)%NUM_OF_WAYS;
        way = chosen_way;
        evict = true;
      }
      return false;
    } else {
      way = chosen_way;
      return true;
    }
  }

  bool allPendingQsHaveAtLeastOneSlot() {
    bool cant_put_array[MSHR_SIZE];
    UNROLL_PENDING_Q_SLOT_CHECK: for (unsigned int i = 0; i< MSHR_SIZE; ++i) {
      cant_put_array[i] = !(mshr_pending_request_queue[i].nb_can_put());
    }
    return !bitvec_utils<int, MSHR_SIZE>::has_one(cant_put_array);
  }

};

//the block keeps track of outstanding write request and fast forward the newest value to read response if it is being written at the same time
template <typename T, typename RD_TAG, typename WR_TAG, size_t WR_BUF_SIZE>
class AccInOutFFwd : public sc_module {
public:
  sc_in_clk clk;
  sc_in<bool> rst;

  //make read and write go through one channel for the purpose of conflict handling
  ga::tlm_fifo_in<AccReqType> acc_req_in;

  // upper level (acc) write request
  ga::tlm_fifo_in<MemSingleWriteReqType<T, WR_TAG> > acc_wr_req_in;
  //ga::tlm_fifo_out<MemSingleWriteRespType<WR_TAG> > acc_wr_resp_out;

  // lower level (store) write request
  typedef typename SizeT<WR_BUF_SIZE>::Type LwrTag;
  ga::tlm_fifo_out<MemSingleWriteReqType<T, LwrTag> > acc_wr_req_out;
  ga::tlm_fifo_in<MemSingleWriteRespType<LwrTag> > acc_wr_resp_in;

  // upper level (acc) read request
  ga::tlm_fifo_out<MemSingleReadRespType<T, RD_TAG> > acc_rd_resp_out;
  ga::tlm_fifo_in<MemSingleReadReqType<T, RD_TAG> > acc_rd_req_in;

  // lower level (mshr) read request
  ga::tlm_fifo_in<MemSingleReadRespType<T, RD_TAG> > acc_rd_resp_in;
  ga::tlm_fifo_out<MemSingleReadReqType<T, RD_TAG> > acc_rd_req_out;

  AccRenameBuffer<WR_BUF_SIZE, MemSingleWriteReqType<T, WR_TAG> > wr_req_rename_buffer;


  SC_HAS_PROCESS(AccInOutFFwd);

  AccInOutFFwd(sc_module_name name)
  :
    clk("clk"), rst("rst") {
    SC_CTHREAD(main_proc, clk.pos());
    async_reset_signal_is(rst, false);

    acc_wr_req_in.clk_rst(clk, rst);
    acc_wr_req_out.clk_rst(clk, rst);
    acc_wr_resp_in.clk_rst(clk, rst);
    //acc_wr_resp_out.clk_rst(clk, rst);
    acc_rd_req_in.clk_rst(clk, rst);
    acc_rd_req_out.clk_rst(clk, rst);
    acc_rd_resp_in.clk_rst(clk, rst);
    acc_rd_resp_out.clk_rst(clk, rst);
    acc_req_in.clk_rst(clk, rst);
  }

  bool findBufferMatch(SplAddressType addr, size_t &index) {
    bool bitvec[WR_BUF_SIZE];
    UNROLL_FFWD_BUFFER_MATCH: for (unsigned i = 0; i < WR_BUF_SIZE; ++i) {
      bitvec[i] =  (wr_req_rename_buffer.valid[i] && (wr_req_rename_buffer.state[i].addr == addr));
    }
    typename SizeT<WR_BUF_SIZE>::Type ind = 0;
    bool result = bitvec_utils<typename SizeT<WR_BUF_SIZE>::Type, WR_BUF_SIZE>::find_any_one(bitvec,ind);
    index = ind;
    return result;
  }

  void main_proc() {
    {
      acc_req_in.reset_get();
      acc_wr_req_in.reset_get();
      acc_wr_req_out.reset_put();
      acc_wr_resp_in.reset_get();
      //acc_wr_resp_out.reset_put();
      acc_rd_req_in.reset_get();
      acc_rd_req_out.reset_put();
      acc_rd_resp_in.reset_get();
      acc_rd_resp_out.reset_put();
      wr_req_rename_buffer.reset_free();
      wr_req_rename_buffer.reset_alloc();
    }
    bool pend_req = false;
    MemSingleReadReqType<T, RD_TAG> pend_read;
    bool pend_read_valid = false;
    MemSingleWriteReqType<T, WR_TAG> pend_write;
    bool pend_write_valid = false;
    bool write_conflict = false;
    size_t write_conflict_index = 0;
    bool read_after_write = false;
    size_t read_conflict_index = 0;

    wait();
    while (1) {
      {
        bool multi_cycle_reset_done = true;
        if (!wr_req_rename_buffer.multi_cycle_reset())
          multi_cycle_reset_done = false;

        if (multi_cycle_reset_done) {
          // has write outstanding to the same location, stall
          write_conflict = pend_write_valid && findBufferMatch(pend_write.addr, write_conflict_index);
          if (pend_write_valid && !write_conflict && wr_req_rename_buffer.can_alloc() && acc_wr_req_out.nb_can_put()) {
            size_t tag = wr_req_rename_buffer.alloc(pend_write);
            acc_wr_req_out.nb_put(MemSingleWriteReqType<T, LwrTag> (pend_write.addr,tag,pend_write.data));
            pend_write_valid = false;
            pend_req = false;
          }

          // has read to the same location where write outstanding exists
          // will fast forward
          read_after_write = pend_read_valid && findBufferMatch(pend_read.addr, read_conflict_index);
          if (read_after_write && acc_rd_resp_out.nb_can_put()) {
            DBG_OUT << "FFWD RAW: AT " << pend_read.addr << "state addr " << wr_req_rename_buffer.state[read_conflict_index].addr << " data = " << wr_req_rename_buffer.state[read_conflict_index].data << std::endl;
            acc_rd_resp_out.nb_put(MemSingleReadRespType<T, RD_TAG>(wr_req_rename_buffer.state[read_conflict_index].data, pend_read.utag));
            pend_read_valid = false;
            pend_req = false;
          } else if (acc_rd_resp_in.nb_can_get() && acc_rd_resp_out.nb_can_put()) {
            MemSingleReadRespType<T, RD_TAG> resp;
            //DBG_OUT << "FFWD MSR: AT " << pend_read.addr << " data = " << wr_req_rename_buffer.state[read_conflict_index].data << std::endl;
            acc_rd_resp_in.nb_get(resp);
            acc_rd_resp_out.nb_put(resp);
          }

          if (pend_read_valid && !read_after_write && acc_rd_req_out.nb_can_put()) {
            acc_rd_req_out.nb_put(pend_read);
            pend_read_valid = false;
            pend_req = false;
          }

          AccReqType req;
          if (acc_req_in.nb_can_get() && !pend_req) {
            acc_req_in.nb_get(req);
            pend_req = true;
          }

          if (pend_req && req == ACC_REQ_WRITE && acc_wr_req_in.nb_can_get() && !pend_write_valid) {
            acc_wr_req_in.nb_get(pend_write);
            pend_write_valid = true;
          }

          if (pend_req && req == ACC_REQ_READ && acc_rd_req_in.nb_can_get() && !pend_read_valid) {
            acc_rd_req_in.nb_get(pend_read);
            pend_read_valid = true;
          }

          if (acc_wr_resp_in.nb_can_get() ) {
            //&& acc_wr_resp_out.nb_can_put()) {
            MemSingleWriteRespType<LwrTag> resp;
            acc_wr_resp_in.nb_get(resp);
            wr_req_rename_buffer.free(resp.utag);
            //acc_wr_resp_out.nb_put(resp);
          }

        }

      }
      wait();
    }
  }

};

template <typename T, typename UTAG, size_t NUM_OF_SETS, size_t NUM_OF_WAYS, size_t MSHR_SIZE, size_t SAME_ADDR_BUF_SIZE>
class AccInOut<LoadStoreUnitSingleReqParams<T, UTAG, NUM_OF_SETS, NUM_OF_WAYS, MSHR_SIZE, SAME_ADDR_BUF_SIZE> >: public sc_module {
public:
  sc_in_clk clk;
  sc_in<bool> rst;

  //acc ios
  ga::tlm_fifo_hier_in<MemSingleReadReqType<T,UTAG> > acc_rd_req_in;
  ga::tlm_fifo_hier_in<MemSingleWriteReqType<T,UTAG> > acc_wr_req_in;
  ga::tlm_fifo_hier_out<MemSingleReadRespType<T,UTAG> > acc_rd_resp_out;
  ga::tlm_fifo_hier_out<MemSingleWriteRespType<UTAG> > acc_wr_resp_out;

  // spl ios
  ga::tlm_fifo_hier_out<SplMemWriteReqType> spl_wr_req_out;
  ga::tlm_fifo_hier_in<SplMemWriteRespType> spl_wr_resp_in;
  ga::tlm_fifo_hier_out<SplMemReadReqType> spl_rd_req_out;
  ga::tlm_fifo_hier_in<SplMemReadRespType> spl_rd_resp_in;


  AccTypedCache<T, UTAG, NUM_OF_SETS, NUM_OF_WAYS, MSHR_SIZE, SAME_ADDR_BUF_SIZE> cache;
  typedef typename AccTypedCache<T, UTAG, NUM_OF_SETS, NUM_OF_WAYS, MSHR_SIZE, SAME_ADDR_BUF_SIZE>::IndexWayPair IndexWayPair;
  typedef typename AccTypedCache<T, UTAG, NUM_OF_SETS, NUM_OF_WAYS, MSHR_SIZE, SAME_ADDR_BUF_SIZE>::MSHRIndexT MSHRIndexT;

  enum {
    STORE_BUF_SIZE = 16
  };
  typedef typename SizeT<STORE_BUF_SIZE>::Type WrBufIndexT;


  AccInOutFFwd<CacheLineType, MSHRIndexT, UTAG, STORE_BUF_SIZE> acc_inout;
  AccIn<LoadUnitSingleReqParams<CacheLineType, MSHRIndexT, MSHR_SIZE> > mshr;
  AccOut<StoreUnitSingleReqParams<CacheLineType, WrBufIndexT, STORE_BUF_SIZE> > store;

  ga::tlm_fifo<AccReqType> acc_req;
  ga::tlm_fifo<MemSingleReadRespType<CacheLineType, MSHRIndexT> > acc_rd_resp;
  ga::tlm_fifo<MemSingleReadReqType<CacheLineType, MSHRIndexT> > acc_rd_req;
  ga::tlm_fifo<MemSingleWriteRespType<UTAG> > acc_wr_resp;
  ga::tlm_fifo<MemSingleWriteReqType<CacheLineType, UTAG > > acc_wr_req;

  ga::tlm_fifo<MemSingleReadRespType<CacheLineType, MSHRIndexT> > mshr_rd_resp;
  ga::tlm_fifo<MemSingleReadReqType<CacheLineType, MSHRIndexT> > mshr_rd_req;
  ga::tlm_fifo<MemSingleWriteRespType<WrBufIndexT> > store_wr_resp;
  ga::tlm_fifo<MemSingleWriteReqType<CacheLineType, WrBufIndexT > > store_wr_req;

  ga::tlm_fifo<SplMemWriteReqType> spl_wr_req;
  ga::tlm_fifo<SplMemWriteRespType> spl_wr_resp;
  ga::tlm_fifo<SplMemReadReqType> spl_rd_req;
  ga::tlm_fifo<SplMemReadRespType> spl_rd_resp;


  AccInOut(sc_module_name name) :
    sc_module(name), clk("clk"), rst("rst"), acc_rd_req_in(
        "acc_rd_req_in"), acc_wr_req_in("acc_wr_req_in"), acc_rd_resp_out(
            "acc_rd_resp_out"), spl_wr_req_out("spl_wr_req_out"), spl_wr_resp_in(
                "spl_wr_resp_in"), spl_rd_req_out("spl_rd_req_out"), spl_rd_resp_in(
                    "spl_rd_resp_in"), cache("cache"), acc_inout("acc_inout"), mshr(
                        "mshr"), store("store"), spl_wr_req("spl_wr_req"), spl_wr_resp(
                            "spl_wr_resp"), spl_rd_req("spl_rd_req"), spl_rd_resp(
                                "spl_rd_resp") {
    cache.clk(clk);
    cache.rst(rst);
    mshr.clk(clk);
    mshr.rst(rst);
    acc_inout.clk(clk);
    acc_inout.rst(rst);
    store.clk(clk);
    store.rst(rst);

    cache.cl_rd_req_out(acc_rd_req);
    cache.cl_rd_resp_in(acc_rd_resp);
    cache.cl_wr_req_out(acc_wr_req);
    //cache.cl_wr_resp_in(acc_rd_resp);

    mshr.acc_req_in(mshr_rd_req);
    mshr.acc_resp_out(mshr_rd_resp);

    store.acc_req_in(store_wr_req);
    store.acc_resp_out(store_wr_resp);

    acc_inout.acc_req_in(acc_req);
    cache.acc_req_out(acc_req);

    acc_inout.acc_wr_req_in(acc_wr_req);
    //acc_inout.acc_wr_resp_out(acc_wr_resp);

    acc_inout.acc_rd_req_out(mshr_rd_req);
    acc_inout.acc_rd_resp_in(mshr_rd_resp);

    acc_inout.acc_rd_req_in(acc_rd_req);
    acc_inout.acc_rd_resp_out(acc_rd_resp);

    acc_inout.acc_wr_req_out(store_wr_req);
    acc_inout.acc_wr_resp_in(store_wr_resp);

    mshr.spl_req_out(spl_rd_req_out);
    mshr.spl_resp_in(spl_rd_resp_in);

    store.spl_req_out(spl_wr_req_out);
    store.spl_resp_in(spl_wr_resp_in);

    cache.acc_rd_req_in(acc_rd_req_in);
    cache.acc_wr_req_in(acc_wr_req_in);
    cache.acc_wr_resp_out(acc_wr_resp_out);
    cache.acc_rd_resp_out(acc_rd_resp_out);

  }


  struct ChannelToArbiter {
    ga::tlm_fifo<SplMemReadReqType> spl_rd_req_ch;
    ga::tlm_fifo<SplMemReadRespType> spl_rd_resp_ch;
    ga::tlm_fifo<SplMemWriteReqType> spl_wr_req_ch;
    ga::tlm_fifo<SplMemWriteRespType> spl_wr_resp_ch;

    ChannelToArbiter() :
      spl_rd_req_ch("spl_rd_req_ch"), spl_rd_resp_ch("spl_rd_resp_ch"), spl_wr_req_ch("spl_wr_req_ch"), spl_wr_resp_ch("spl_wr_resp_ch") {}

    template <size_t NUM_OF_READERS, size_t NUM_OF_WRITERS>
    void bindArbiter(MemArbiter<NUM_OF_READERS, SplMemReadReqType, SplMemReadRespType> &rd_arb, size_t rd_arb_index, MemArbiter<NUM_OF_WRITERS, SplMemWriteReqType, SplMemWriteRespType> &wr_arb, size_t wr_arb_index, AccInOut &acc_io) {
      assert(rd_arb_index < NUM_OF_READERS);
      assert(wr_arb_index < NUM_OF_WRITERS);
      rd_arb.in_req_fifo[rd_arb_index](spl_rd_req_ch);
      rd_arb.out_resp_fifo[rd_arb_index](spl_rd_resp_ch);
      acc_io.spl_rd_req_out(spl_rd_req_ch);
      acc_io.spl_rd_resp_in(spl_rd_resp_ch);
      wr_arb.in_req_fifo[wr_arb_index](spl_wr_req_ch);
      wr_arb.out_resp_fifo[wr_arb_index](spl_wr_resp_ch);
      acc_io.spl_wr_req_out(spl_wr_req_ch);
      acc_io.spl_wr_resp_in(spl_wr_resp_ch);
    }
    //only read
    template <size_t NUM_OF_READERS, size_t NUM_OF_WRITERS>
    void bindArbiter(MemArbiter<NUM_OF_READERS, SplMemReadReqType, SplMemReadRespType> &rd_arb, size_t rd_arb_index, AccInOut &acc_io) {
      assert(rd_arb_index < NUM_OF_READERS);
      rd_arb.in_req_fifo[rd_arb_index](spl_rd_req_ch);
      rd_arb.out_resp_fifo[rd_arb_index](spl_rd_resp_ch);
      acc_io.spl_rd_req_out(spl_rd_req_ch);
      acc_io.spl_rd_resp_in(spl_rd_resp_ch);
      acc_io.spl_wr_req_out(spl_wr_req_ch);
      acc_io.spl_wr_resp_in(spl_wr_resp_ch);
    }
  };


};
#endif

#endif /* ACC_SINGLE_IO_H_ */
