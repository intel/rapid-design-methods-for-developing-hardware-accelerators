// See LICENSE for license details.

#ifndef __ACCIO_H__
#define __ACCIO_H__


#include "systemc.h"
#include "types.h"
#include "ga_tlm_fifo.h"

template <size_t OUTSTANDING_BUFFER_SIZE>
struct OutstandingRequestToken {
  SplTag tag;
  OutstandingRequestToken() : tag(0) {
    // this size should be possible to address with OutstandingRequestToken.tag,
    // i.e. log2(OUTSTANDING_BUFFER_SIZE)-1 < bitwidth(OutstandingRequestToken.tag)
    //assert(OUTSTANDING_BUFFER_SIZE <= 64);
  }
  OutstandingRequestToken(SplTag tag) :
    tag(tag) {
    // this size should be possible to address with OutstandingRequestToken.tag,
    // i.e. log2(OUTSTANDING_BUFFER_SIZE)-1 < bitwidth(OutstandingRequestToken.tag)
    //assert(OUTSTANDING_BUFFER_SIZE <= 64);
  }
	inline friend std::ostream& operator<<(std::ostream& os, OutstandingRequestToken& rhs)
	{
	  return os;
	}

  inline friend void sc_trace(sc_trace_file* tf,
      const OutstandingRequestToken& d, const std::string& name) {
  }

  inline bool operator==(const OutstandingRequestToken& rhs) {
    bool result = true;
    result = result && (tag == rhs.tag);
    return result;
  }
};

template <size_t OUTSTANDING_BUFFER_SIZE=32>
struct AccMemIn : sc_module {
#ifdef STRATUS
  HLS_INLINE_MODULE;
#endif
public:
  // clk/rst
	sc_in_clk clk;
	sc_in<bool> rst;
	//sc_out<bool> idle;

	// functional ports
  ga::tlm_fifo_out<AccMemReadRespType> acc_resp_out;
	ga::tlm_fifo_out<SplMemReadReqType> spl_req_out;
	ga::tlm_fifo_in<AccMemReadReqType> acc_req_in;
	ga::tlm_fifo_in<SplMemReadRespType> spl_resp_in;

	// request queue is needed to keep track of outstanding requests (credit like mechanism). when response comes in we pop the queue.
	// also it keeps responses in order of requests
	ga::ga_storage_fifo<OutstandingRequestToken<OUTSTANDING_BUFFER_SIZE>, OUTSTANDING_BUFFER_SIZE> request_queue;

	// this fifo takes care of indices of available entries in response_data_array
  ga::ga_storage_fifo<OutstandingRequestToken<OUTSTANDING_BUFFER_SIZE>, OUTSTANDING_BUFFER_SIZE> available_slots;

  //scide_waive SCIDE.8.2
  //scide_waive SCIDE.6.2
  // indexed by OutstandingRequestToken.tag
	AccMemReadRespType response_data_array[OUTSTANDING_BUFFER_SIZE];
	SC_SIGNAL_MULTIW(bool, response_data_ready[OUTSTANDING_BUFFER_SIZE]);

  IOUnitIdType unit_id;

	SC_HAS_PROCESS(AccMemIn);

  AccMemIn(sc_module_name modname=sc_gen_unique_name("acc_mem_in")) :
      sc_module(modname), clk("clk"), rst("rst"), acc_resp_out("acc_resp_out"), spl_req_out(
          "spl_req_out"), acc_req_in("acc_req_in"), spl_resp_in("spl_resp_in"), request_queue(
          "request_queue"), available_slots("response_queue"), unit_id(-1) {
    acc_req_in.clk_rst(clk, rst);
    spl_resp_in.clk_rst(clk, rst);
    acc_resp_out.clk_rst(clk, rst);
    spl_req_out.clk_rst(clk, rst);

    SC_CTHREAD(request_handler, clk.pos());
    async_reset_signal_is(rst, false);
    SC_CTHREAD(spl_response_handler, clk.pos());
    async_reset_signal_is(rst, false);
    SC_CTHREAD(acc_response_handler, clk.pos());
    async_reset_signal_is(rst, false);
  }

  void request_handler() {
    AddressType next_addr = 0;
    SizeInCLType remaining_size = 0;
    full_counter = 0;

    {
      request_queue.reset_put();
      spl_req_out.reset_put();
      acc_req_in.reset_get();
      available_slots.reset_get();
    }
    wait();
    while (1) {
      {
        if (remaining_size == 0 && acc_req_in.nb_can_get()) {
          AccMemReadReqType req;
          acc_req_in.nb_get(req);
          DBG_OUT << sc_time_stamp() << " AccMemIn:request_handler got request with size " << (unsigned int)req.size << " and address " << req.addr << endl;
          remaining_size = req.size;
          next_addr = req.addr;
        }
        if (!available_slots.nb_can_get())
          full_counter++;

        /// check if a new request is coming, request queue is not full that would signify that we have too many outstanding requests and spl request out is ready
        if (remaining_size != 0 && available_slots.nb_can_get() && spl_req_out.nb_can_put()) {
          OutstandingRequestToken<OUTSTANDING_BUFFER_SIZE> req_token;
          assert(request_queue.nb_can_put());
          // getting next available row
          available_slots.nb_get(req_token);
          // add token to request queue
          request_queue.put(req_token);

          // send request to memory
          SplMemReadReqType spl_req;
          spl_req.addr = next_addr;
          spl_req.io_unit_id = 0;
          spl_req.tag = req_token.tag;
          assert(response_data_ready[spl_req.tag] == false);

          bool put_result = spl_req_out.nb_put(spl_req);
          // should always succeed as we check nb_can_put in the condition
          assert(put_result);

          DBG_OUT << sc_time_stamp() << " AccMemIn:request_handler SPL request sent with address " << spl_req.addr << endl;

          // prepare for the next transaction
          next_addr = AddressUtils::getNextCLAddress(next_addr);
          --remaining_size;
        }
      }
      wait();
    }
  }
  size_t full_counter;
  size_t _no_outstanding_reqs;
  size_t _consumer_not_ready;
  size_t _reorder;
  void acc_response_handler() {
    bool awaiting_response = false;
    _no_outstanding_reqs = 0;
    _consumer_not_ready = 0;
    _reorder = 0;
    OutstandingRequestToken<OUTSTANDING_BUFFER_SIZE> next_req;
    size_t available_slots_index = OUTSTANDING_BUFFER_SIZE;
    {
      request_queue.reset_get();
      available_slots.reset_put();
      RESP_DATA_READY_RESET_LOOP: for (unsigned int i = 0; i < OUTSTANDING_BUFFER_SIZE; ++i) {
        response_data_ready[i] = false;
      }
    }
    acc_resp_out.reset_put();
    bool out_data_ready = false;
    AccMemReadRespType response_data;
    SplTag response_tag;

    wait();
    ACC_MEM_IN_ACC_RESP_THREAD:while (1) {
      {
        if (!request_queue.nb_can_get())
          _no_outstanding_reqs ++;
        if (!acc_resp_out.nb_can_put())
           _consumer_not_ready ++;

        // multi cycle reset modeled here to fill up a fifo with credits
        if (available_slots_index != 0) {
          assert(available_slots.nb_can_put());
          --available_slots_index;
          available_slots.nb_put(OutstandingRequestToken<OUTSTANDING_BUFFER_SIZE>(available_slots_index));
        } else {
          if (out_data_ready && acc_resp_out.nb_can_put()) {
          //if (awaiting_response && response_data_ready[response_tag] == true && acc_resp_out.nb_can_put() ) {
             // have response come in and send it out
             assert(available_slots.nb_can_put());
             available_slots.nb_put(OutstandingRequestToken<OUTSTANDING_BUFFER_SIZE>(response_tag));
             acc_resp_out.nb_put(response_data);
             DBG_OUT << sc_time_stamp() << " AccMemIn:acc_response_handler SPL response popped sent to accelerator with data " << response_data_array[response_tag] << " and response_tag " << response_tag << endl;
             out_data_ready = false;
           }

          // if not awaiting any response, fetch a new token from request queue to wait for
          if (!awaiting_response && request_queue.nb_can_get()) {
            //get next request to check response for
            request_queue.nb_get(next_req);
            awaiting_response = true;
          }
#ifndef __SYNTHESIS__
          bool has_data_ready = false;
          size_t i = 0;
          for (i = 0; i < OUTSTANDING_BUFFER_SIZE; ++i) {
            if (response_data_ready[i]) {
              has_data_ready = true;
              break;
            }
          }
          //next_req.tag =  (has_data_ready) ? i : next_req.tag;
          if (has_data_ready && awaiting_response && response_data_ready[next_req.tag] != true && acc_resp_out.nb_can_put() ){
            _reorder++;
            DBG_OUT << sc_time_stamp() << " AccMemIn:acc_response_handler _reorder incremented to " << _reorder << endl;
          }
#endif
         if (!out_data_ready && awaiting_response && response_data_ready[next_req.tag] == true) {
           out_data_ready = true;
           response_tag = next_req.tag;
           response_data_ready[response_tag] = false;
           response_data = response_data_array[response_tag];
           awaiting_response = false;
         }

        }
      }
      wait();
    }
  }

  void spl_response_handler() {
    {
      //response_queue.reset_put();
      spl_resp_in.reset_get();
    }
    wait();
    while (1) {
      {
        AccMemReadRespType acc_resp;
        if (spl_resp_in.nb_can_get()) {
          SplMemReadRespType spl_resp;
          spl_resp_in.nb_get(spl_resp);
          DBG_OUT << sc_time_stamp() << " AccMemIn:response_handler SPL response received " << spl_resp.data.words[0]<< endl;

          // assert the response belongs to us
          // AA: commented it out because the id is now generated in the arbiter
          // AA: in case of crossbar the arbiter can do it as the connections to IO units is static
          //assert(unit_id == spl_resp.io_unit_id);
          // convert to acc mem read resp type
          AccMemReadRespType new_resp;
          new_resp.data = spl_resp.data;
          assert(response_data_ready[spl_resp.tag] == false);
          // we read from SPL to write to the response_data_array
          response_data_array[spl_resp.tag] = new_resp;
          response_data_ready[spl_resp.tag] = true;
        }

      }
      wait();
    }
  }
  ~AccMemIn() {
    const char* name = basename();
    cout << "ACCIn " << name << " " << " was full (all request slots taken) for " << full_counter << " cycles" << endl;
    cout << "AccIn " << name << " stats (idle now): " <<
        " no outstanding requests = " << _no_outstanding_reqs <<
        " acc not ready to receive " << _consumer_not_ready <<
        " reorder waste " << _reorder << endl;

  }
};

struct AccMemOut : sc_module {
#ifdef STRATUS
  HLS_INLINE_MODULE;
#endif
  // clk/rst
  sc_in_clk clk;
  sc_in<bool> rst;
  //sc_out<bool> idle;

  // functional ports
  ga::tlm_fifo_out<SplMemWriteReqType> spl_req_out;
  ga::tlm_fifo_in<AccMemWriteReqType> acc_req_in;
  ga::tlm_fifo_in<AccMemWriteDataType> acc_data_in;
  // TODO: still need to assess whether we need write response
  // what if we start reading at that address later?
  ga::tlm_fifo_in<SplMemWriteRespType> spl_resp_out;

  IOUnitIdType unit_id;


  SC_HAS_PROCESS(AccMemOut);

  //scide_waive SCIDE.8.15
  AccMemOut(sc_module_name name=sc_gen_unique_name("AccMemOut")) :
      clk("clk"), rst("rst"), spl_req_out("spl_req_out"), acc_req_in(
          "acc_req_in"), acc_data_in("acc_data_in"), spl_resp_out("spl_resp_out"), unit_id(
          -1) {
    SC_CTHREAD(request_handler, clk.pos());
    async_reset_signal_is(rst, false);
    acc_req_in.clk_rst(clk, rst);
    acc_data_in.clk_rst(clk, rst);
    spl_req_out.clk_rst(clk, rst);
    spl_resp_out.clk_rst(clk, rst);
  }

  void request_handler() {
    {
      spl_req_out.reset_put();
      acc_req_in.reset_get();
      acc_data_in.reset_get();
      spl_resp_out.reset_get();
    }
    wait();
    while (1) {
      AddressType next_addr = 0;
      SizeInCLType remaining_size = 0;
      {
      }
      wait();
      while (1) {
        {
          if (remaining_size == 0 && acc_req_in.nb_can_get()) {
            AccMemWriteReqType req;
            acc_req_in.nb_get(req);
            DBG_OUT << sc_time_stamp() << " AccMemOut:request_handler got write request with size " << (unsigned int)req.size << " and address " << req.addr << endl;
            remaining_size = req.size;
            next_addr = req.addr;
          }
          // if have something to send and the data is available at the port, fetch and send it
          if (remaining_size != 0 && acc_data_in.nb_can_get()) {
            // get data
            AccMemWriteDataType req_data;
            acc_data_in.nb_get(req_data);
            // send request to memory
            SplMemWriteReqType spl_req;
            spl_req.addr = next_addr;
            spl_req.io_unit_id = 0;
            spl_req.data = req_data.data;
            spl_req.offset = req_data.offset;
            spl_req.width = req_data.width;
            spl_req_out.put(spl_req);

            DBG_OUT << sc_time_stamp() << " AccMemOut:request_handler SPL write request sent with address " << spl_req.addr << endl;

            // prepare for the next transaction
            next_addr = AddressUtils::getNextCLAddress(next_addr);
            --remaining_size;
          }
          //idle = (remaining_size == 0) && !acc_req_in.nb_can_get();
        }
        wait();
      }
    }
  }

};


#endif //__ACCIO_H__

