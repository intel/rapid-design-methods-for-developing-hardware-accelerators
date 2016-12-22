// See LICENSE for license details.

#ifndef ACC_TYPED_IO_H_
#define ACC_TYPED_IO_H_


#include "systemc.h"
#include "types.h"
#include "ga_tlm_fifo.h"


struct CacheLineMarshalBase {
  CacheLineType cl;
  CacheLineType::IndexType cur_cl_pos;
  bool valid;
  CacheLineMarshalBase () :
    cur_cl_pos(0), valid(false)
  {}

  CacheLineType getCL() const {
    return cl;
  }

  void start () {
    cl.clear();
    cur_cl_pos = 0;
    valid = true;
  }

  void start (CacheLineType::IndexType offset) {
    cl.clear();
    cur_cl_pos = offset;
    valid = true;
  }

  void start(CacheLineType clin) {
    cl = clin;
    cur_cl_pos = 0;
    valid = true;
  }

  void start(CacheLineType clin, CacheLineType::IndexType offset) {
    cl = clin;
    cur_cl_pos = offset;
    valid = true;
  }


  bool active()  {
    return valid;
  }

  void finish () {
    valid = false;
  }

};
template <size_t MARSHAL_STEP>
struct CacheLineMarshal : CacheLineMarshalBase {
  CacheLineMarshal() : CacheLineMarshalBase()
  {
    assert(MARSHAL_STEP > 0);
    assert(MARSHAL_STEP <= CacheLineType::getBitCnt());
    assert(CacheLineType::getBitCnt()%MARSHAL_STEP == 0);
  }

  bool done() const {
    // simplified the condition with an added assert in the ctor that requires the data type width be integer parts that fit completely in the cacheline
    if ((CacheLineType::IndexType)(cur_cl_pos*MARSHAL_STEP == CacheLineType::getBitCnt()))
      return true;
    else
      return false;
  }

  bool done_next() const {
    // simplified the condition with an added assert in the ctor that requires the data type width be integer parts that fit completely in the cacheline
    if ((CacheLineType::IndexType)(cur_cl_pos*MARSHAL_STEP == CacheLineType::getBitCnt()-MARSHAL_STEP))
      return true;
    else
      return false;
  }

  void next() {
    assert((CacheLineType::IndexType)(MARSHAL_STEP*cur_cl_pos + MARSHAL_STEP - 1) < CacheLineType::getBitCnt());
    ++cur_cl_pos;
  }
  template<typename T>
  void getSlice(T& data) {
    assert((CacheLineType::IndexType)(MARSHAL_STEP*cur_cl_pos + MARSHAL_STEP - 1) < CacheLineType::getBitCnt());
    CacheLineType::IndexType fieldIndex = cur_cl_pos*MARSHAL_STEP;
    CL_MARSH_GET_SLICE_UNROLL_FOR: for (size_t i = 0; i < T::numberOfFields(); ++i) {
      data.putField(i, cl.getDataInRange(fieldIndex, T::fieldWidth(i)));
      fieldIndex += T::fieldWidth(i);
    }
  }

  template<typename T>
  void putSlice(const T &d) {
    assert((CacheLineType::IndexType)(MARSHAL_STEP*cur_cl_pos + MARSHAL_STEP - 1) < CacheLineType::getBitCnt());
    CacheLineType::IndexType fieldIndex = MARSHAL_STEP*cur_cl_pos;
    CL_MARSH_PUT_SLICE_UNROLL_FOR: for (size_t i = 0; i < T::numberOfFields(); ++i) {
      cl.putDataInRange(fieldIndex, T::fieldWidth(i), d.getField(i));
      fieldIndex += T::fieldWidth(i);
    }
  }

  //  UInt64 getSlice() const {
  //    assert((CacheLineType::IndexType)(cur_cl_pos + MARSHAL_STEP - 1) < CacheLineType::getBitCnt());
  //    return cl.getDataInRange(cur_cl_pos, cur_cl_pos+MARSHAL_STEP-1);
  //  }
  //  void putSlice(UInt64 d) {
  //    assert((CacheLineType::IndexType)(cur_cl_pos + MARSHAL_STEP - 1) < CacheLineType::getBitCnt());
  //    cl.putDataInRange(cur_cl_pos, MARSHAL_STEP, d);
  //  }

  CacheLineType::IndexType getCurrFieldIndex() {
    // you probably don't want the default implmentation
    assert(0);
    return MARSHAL_STEP*cur_cl_pos;
  }
};
template<> inline
CacheLineType::IndexType CacheLineMarshal<128>::getCurrFieldIndex() {
  CacheLineType::IndexType fieldIndex= 0;
  switch (cur_cl_pos) {
  case 0:
  case 1:
  case 2:
  case 3:
    fieldIndex = cur_cl_pos*128;
    break;
  default:
    assert(0);
  }
  return fieldIndex;
}

template<> inline
CacheLineType::IndexType CacheLineMarshal<64>::getCurrFieldIndex() {
  CacheLineType::IndexType fieldIndex= 0;
  switch (cur_cl_pos) {
  case 0:
  case 1:
  case 2:
  case 3:
  case 4:
  case 5:
  case 6:
  case 7:
    fieldIndex = cur_cl_pos*64;
    break;
  default:
    assert(0);
  }
  return fieldIndex;
}

template<> inline
CacheLineType::IndexType CacheLineMarshal<32>::getCurrFieldIndex() {
  CacheLineType::IndexType fieldIndex= 0;
  switch (cur_cl_pos) {
  case 0:
  case 1:
  case 2:
  case 3:
  case 4:
  case 5:
  case 6:
  case 7:
  case 8:
  case 9:
  case 10:
  case 11:
  case 12:
  case 13:
  case 14:
  case 15:
    fieldIndex = cur_cl_pos*32;
    break;
  default:
    assert(0);
  }
  return fieldIndex;
}



template <>
struct CacheLineMarshal<512> : CacheLineMarshalBase {
  enum {
    MARSHAL_STEP = 512
  };

  CacheLineMarshal() : CacheLineMarshalBase()
  {
  }

  bool done() const {
    // simplified the condition with an added assert in the ctor that requires the data type width be integer parts that fit completely in the cacheline
    if ((CacheLineType::IndexType)(cur_cl_pos*MARSHAL_STEP == CacheLineType::getBitCnt()))
      return true;
    else
      return false;
  }

  bool done_next() const {
    // simplified the condition with an added assert in the ctor that requires the data type width be integer parts that fit completely in the cacheline
    if ((CacheLineType::IndexType)(cur_cl_pos*MARSHAL_STEP == CacheLineType::getBitCnt()-MARSHAL_STEP))
      return true;
    else
      return false;
  }

  void next() {
    assert((CacheLineType::IndexType)(MARSHAL_STEP*cur_cl_pos + MARSHAL_STEP - 1) < CacheLineType::getBitCnt());
    ++cur_cl_pos;
  }
  template<typename T>
  void getSlice(T& data) {
    CacheLineType::IndexType fieldIndex = 0;
    CL_MARSH_GET_SLICE_UNROLL_FOR: for (size_t i = 0; i < T::numberOfFields(); ++i) {
      data.putField(i, cl.getDataInRange(fieldIndex, T::fieldWidth(i)));
      fieldIndex += T::fieldWidth(i);
    }
  }

  template<typename T>
  void putSlice(const T &d) {
    assert(cur_cl_pos == 0);
    CacheLineType::IndexType fieldIndex = 0;
    CL_MARSH_PUT_SLICE_UNROLL_FOR: for (size_t i = 0; i < T::numberOfFields(); ++i) {
      cl.putDataInRange(fieldIndex, T::fieldWidth(i), d.getField(i));
      fieldIndex += T::fieldWidth(i);
    }
  }

  inline CacheLineType::IndexType getCurrFieldIndex() {
    assert (cur_cl_pos == 0);
    return 0;
  }

};

//
//template <>
//struct CacheLineMarshal<128> : CacheLineMarshalBase {
//  enum {
//    MARSHAL_STEP = 128
//  };
//
//  CacheLineMarshal() : CacheLineMarshalBase()
//  {
//  }
//
//  bool done() const {
//    // simplified the condition with an added assert in the ctor that requires the data type width be integer parts that fit completely in the cacheline
//    if ((CacheLineType::IndexType)(cur_cl_pos*MARSHAL_STEP == CacheLineType::getBitCnt()))
//      return true;
//    else
//      return false;
//  }
//
//  void next() {
//    assert((CacheLineType::IndexType)(MARSHAL_STEP*cur_cl_pos + MARSHAL_STEP - 1) < CacheLineType::getBitCnt());
//    ++cur_cl_pos;
//  }
//  template<typename T>
//  void getSlice(T& data) {
//    assert((CacheLineType::IndexType)(MARSHAL_STEP*cur_cl_pos + MARSHAL_STEP - 1) < CacheLineType::getBitCnt());
//    CacheLineType::IndexType fieldIndex = 0;
//    switch (cur_cl_pos) {
//    case 0:
//    case 1:
//    case 2:
//    case 3:
//      fieldIndex = cur_cl_pos*MARSHAL_STEP;
//      break;
//    default:
//      assert(0);
//    }
//    for (size_t i = 0; i < T::numberOfFields(); ++i) {
//      data.putField(i, cl.getDataInRange(fieldIndex, T::fieldWidth(i)));
//      fieldIndex += T::fieldWidth(i);
//    }
//  }
//
//  template<typename T>
//  void putSlice(const T &d) {
//    assert((CacheLineType::IndexType)(MARSHAL_STEP*cur_cl_pos + MARSHAL_STEP - 1) < CacheLineType::getBitCnt());
//    CacheLineType::IndexType fieldIndex = 0;
//    switch (cur_cl_pos) {
//    case 0:
//    case 1:
//    case 2:
//    case 3:
//      fieldIndex = cur_cl_pos*MARSHAL_STEP;
//      break;
//    default:
//      assert(0);
//    }
//    for (size_t i = 0; i < T::numberOfFields(); ++i) {
//      cl.putDataInRange(fieldIndex, T::fieldWidth(i), d.getField(i));
//      fieldIndex += T::fieldWidth(i);
//    }
//  }
//
//  size_t boundValue (size_t val) {
//
//  }
//
//};

template<size_t MAX_REQ_COUNT>
struct OutstandingRequestType {
  typename SizeT<MAX_REQ_COUNT>::Type size;
  CacheLineType::ByteIndexType addr_offset;
  OutstandingRequestType() : size(0), addr_offset(0) {}
  OutstandingRequestType(MemRequestSizeType size, CacheLineType::ByteIndexType addr_offset) :
    size(size), addr_offset(addr_offset)
  {
    assert(size <= MAX_REQ_COUNT);
  }

  inline friend void sc_trace(sc_trace_file* tf,
      const OutstandingRequestType& d, const std::string& name) {
  }

  inline friend std::ostream& operator<<(std::ostream& os,
      const OutstandingRequestType& d) {
    os << "size: " << d.size << std::endl;
    os << "addr_offset: " << d.addr_offset << std::endl;
    return os;
  }

  inline bool operator==(const OutstandingRequestType& rhs) {
    bool result = true;
    result = result && (size == rhs.size);
    result = result && (addr_offset == rhs.addr_offset);
    return result;
  }
};

template <typename T, size_t OUTSTANDING_BUFFER_SIZE=32, size_t MAX_COUNT_PER_REQUEST=256>
struct AccMemTypedIn : sc_module {
#ifdef STRATUS
  HLS_INLINE_MODULE;
#endif
public:
  // params
  //static const unsigned int OUTSTANDING_BUFFER_SIZE = 32;
  typedef OutstandingRequestType<MAX_COUNT_PER_REQUEST> OutstandingRequestTypeS;

  // clk/rst
  sc_in_clk clk;
  sc_in<bool> rst;

  // functional ports
  ga::tlm_fifo_out<MemTypedReadRespType<T> > acc_resp_out;
  ga::tlm_fifo_out<AccMemReadReqType> mem_req_out;
  ga::tlm_fifo_in<MemTypedReadReqType<T> > acc_req_in;
  ga::tlm_fifo_in<AccMemReadRespType> mem_resp_in;

  // keep track of how many elements requested
  ga::ga_storage_fifo<OutstandingRequestTypeS, OUTSTANDING_BUFFER_SIZE> out_requests;

  SC_HAS_PROCESS(AccMemTypedIn);

  AccMemTypedIn(sc_module_name name) :
    clk("clk"), rst("rst"), acc_resp_out("acc_resp_out"), mem_req_out(
        "mem_req_out"), acc_req_in("acc_req_in"), mem_resp_in("mem_resp_in") {
    SC_CTHREAD(request_handler, clk.pos());
    async_reset_signal_is(rst, false);
    SC_CTHREAD(response_handler, clk.pos());
    async_reset_signal_is(rst, false);
    mem_resp_in.clk_rst(clk, rst);
    mem_req_out.clk_rst(clk, rst);
    acc_resp_out.clk_rst(clk, rst);
    acc_req_in.clk_rst(clk, rst);
  }

  void request_handler() {
    {
#ifdef STRATUS
      HLS_DEFINE_PROTOCOL("reset");
#endif
      mem_req_out.reset_put();
      out_requests.reset_put();
      acc_req_in.reset_get();
      wait();
    }
    while (1) {
#ifdef STRATUS
      HLS_BREAK_PROTOCOL("computation");
#endif
      {
        if (mem_req_out.nb_can_put() && acc_req_in.nb_can_get() && out_requests.nb_can_put()) {
          MemTypedReadReqType<T> mem_req;
          acc_req_in.nb_get(mem_req);
          // make sure the bit cnt is integer of cache size
          assert (CacheLineType::getBitCnt() >= T::getBitCnt() && CacheLineType::getBitCnt() % T::getBitCnt() == 0);
          CacheLineType::ByteIndexType addr_offset = CacheLineType::getCacheLineOffset(mem_req.addr);
          AddressType cache_aligned_addr = CacheLineType::getCacheAlignedAddress(mem_req.addr);
          UInt64 total_size = (UInt64)mem_req.size*T::getBitCnt();
          // need to add offset when calculating number of cachelines to request because we request cachelines starting at cache aligned address
          SizeInCLType cl_size = CacheLineType::computeCLSize (total_size+8*addr_offset);
          DBG_OUT << sc_time_stamp().to_string() << " " << basename() << " AccMemTypedIn:request_handler " << (UInt32)cl_size << " cache lines requested at " << cache_aligned_addr << " with address offset = " << (unsigned int)addr_offset << endl;
          mem_req_out.nb_put(AccMemReadReqType(cache_aligned_addr, cl_size));
          if (mem_req.size > MAX_COUNT_PER_REQUEST ) {
            cout << " AASSERT " << mem_req.size  << " > " << MAX_COUNT_PER_REQUEST << endl;
          }
          assert(mem_req.size <= MAX_COUNT_PER_REQUEST);//
          out_requests.nb_put(OutstandingRequestTypeS(mem_req.size, addr_offset));
        }

      }
      wait();
    }
  }

  void response_handler() {
    CacheLineMarshal<T::BitCnt> cl_marsh;
    OutstandingRequestTypeS request;
    MemRequestSizeType num_to_go;
    {
#ifdef STRATUS
      HLS_DEFINE_PROTOCOL("reset");
#endif
      num_to_go = 0;
      out_requests.reset_get();
      mem_resp_in.reset_get();
      acc_resp_out.reset_put();
      wait();
    }
    while (1) {
#ifdef STRATUS
      HLS_BREAK_PROTOCOL("computation");
#endif
      {
        bool num_to_go_eq0 = num_to_go == 0;
        bool num_to_go_eq1 = num_to_go == 1;
        bool num_to_go_max = false;
        // get next request count
        if (num_to_go_eq0 && out_requests.nb_can_get()) {
          out_requests.nb_get(request);
          assert( request.size > 0);
          num_to_go = request.size;
          num_to_go_max = true;
          num_to_go_eq0 = false;
          num_to_go_eq1 = num_to_go == 1;
        }
        if ( !num_to_go_eq0) {
          if(!cl_marsh.active()) {
            DBG_OUT << sc_time_stamp().to_string() << " " << basename() << " cl_marsh.inactive: " << endl;
            AccMemReadRespType mem_resp = mem_resp_in.get();
            DBG_OUT << sc_time_stamp().to_string() << " " << basename() << " AccMemTypedIn:response_handler received cache line with data " << mem_resp.data << endl;
            // for the first cacheline we set and offset in marshaling
            CacheLineType::IndexType offset = (num_to_go_max)?8*(CacheLineType::IndexType)request.addr_offset:0;
            assert(offset%T::BitCnt == 0);
            cl_marsh.start(mem_resp.data, offset/T::BitCnt);
          }
          if (cl_marsh.active()) {
            DBG_OUT << sc_time_stamp().to_string() << " " << basename() << " cl_marsh.active: " << endl;
            // valid cache line and some number of acc data to go (num_to_go)
            // process one piece of data to go in one cycle

            T data;
            cl_marsh.getSlice(data);

            DBG_OUT << sc_time_stamp().to_string() << " " << basename() << " acc_resp_out data: " << data << endl;


            acc_resp_out.put(MemTypedReadRespType<T>(data));
            DBG_OUT << sc_time_stamp().to_string() << " " << basename() << " AccMemTypedIn:response_handler pushed typed data back to acc: " << data << endl;

            --num_to_go;
            num_to_go_eq0 = num_to_go_eq1;
            DBG_OUT << sc_time_stamp().to_string() << " " << basename() << " AccMemTypedIn:response_handler num to go: " << num_to_go << endl;
            bool save_done = cl_marsh.done_next();
            cl_marsh.next();
            // when the next position is out of the scope of the current cache line. we are done with this cache line
            if (num_to_go_eq0 || save_done) {
              cl_marsh.finish();
            }
          }
        }
      }

      wait();
    }
  }

};
template <typename T>
struct AccMemTypedOut : sc_module {
#ifdef STRATUS
  HLS_INLINE_MODULE;
#endif
  // clk/rst
  sc_in_clk clk;
  sc_in<bool> rst;

  // functional ports
  ga::tlm_fifo_in<MemTypedWriteReqType<T> > acc_req_in;
  ga::tlm_fifo_in<MemTypedWriteDataType<T> > acc_data_in;
  ga::tlm_fifo_out<AccMemWriteReqType> mem_req_out;
  ga::tlm_fifo_out<AccMemWriteDataType> mem_data_out;

  // TODO: still need to assess whether we need write response
  // what if we start reading at that address in CPU? will the coherency mechanism take care waiting for the updated value

  SC_HAS_PROCESS(AccMemTypedOut);

  AccMemTypedOut(sc_module_name name) :
    clk("clk"), rst("rst"), acc_req_in("acc_req_in"), acc_data_in(
        "acc_data_in"), mem_req_out("mem_req_out"), mem_data_out(
            "mem_data_out") {
    SC_CTHREAD(request_handler, clk.pos());
    async_reset_signal_is(rst, false);
    mem_data_out.clk_rst(clk, rst);
    mem_req_out.clk_rst(clk, rst);
    acc_req_in.clk_rst(clk, rst);
    acc_data_in.clk_rst(clk, rst);
  }

  void request_handler() {
    MemRequestSizeType remaining_size;
    CacheLineMarshal<T::BitCnt> cl_marsh;
    {

#ifdef STRATUS
      HLS_DEFINE_PROTOCOL("reset");
#endif
      remaining_size = 0;
      mem_req_out.reset_put();
      mem_data_out.reset_put();
      acc_req_in.reset_get();
      acc_data_in.reset_get();
      wait();
    }
    while (1) {
#ifdef STRATUS
      HLS_BREAK_PROTOCOL("computation");
#endif
      // we get write requests here
      bool remaining_size_eq0 = remaining_size == 0;
      bool remaining_size_eq1 = remaining_size == 1;

      if ( remaining_size_eq0 && acc_req_in.nb_can_get() && mem_req_out.nb_can_put() && !cl_marsh.active()) {
        MemTypedWriteReqType<T> req;
        acc_req_in.nb_get(req);
        DBG_OUT << sc_time_stamp() << " AccMemTypedOut:request_handler got write request with size " << (unsigned int)req.size << " and address " << req.addr << endl;
        remaining_size = req.size;
        remaining_size_eq0 = remaining_size == 0;
        remaining_size_eq1 = remaining_size == 1;

        UInt64 total_size = req.size*T::getBitCnt();
        SizeInCLType cl_size = CacheLineType::computeCLSize (total_size);
        mem_req_out.nb_put(AccMemWriteReqType(req.addr, cl_size));
        cl_marsh.start();
      }
      // if have something to send and the data is available at the port, fetch and put to current cache line we marshal
      bool save_done = cl_marsh.done();
      if ( !remaining_size_eq0 && acc_data_in.nb_can_get() && cl_marsh.active() && !save_done) {
        // get data
        MemTypedWriteDataType<T> req_data;
        acc_data_in.nb_get(req_data);
        DBG_OUT << sc_time_stamp() << " AccMemTypedOut:request_handler got data to write " << req_data.data << endl;
        // send request to memory
        cl_marsh.putSlice(req_data.data);
        DBG_OUT << sc_time_stamp() << " AccMemTypedOut:request_handler wrote slice of data to cache line  " << cl_marsh.getCL() << endl;

        save_done = cl_marsh.done_next();
        cl_marsh.next();
        --remaining_size;

        remaining_size_eq0 = remaining_size_eq1;
      }
      // ready to send the cache line out

      if (cl_marsh.active() && (remaining_size_eq0 || save_done)) {
        assert((cl_marsh.cur_cl_pos*T::BitCnt)%8 == 0);
        mem_data_out.put(AccMemWriteDataType(cl_marsh.getCL(),0, (cl_marsh.cur_cl_pos*T::BitCnt)/8));
        // when done with cl_marsh go the next CL, when done with remaining size finish cl_marsh
        if (remaining_size_eq0) {
          cl_marsh.finish();
        } else if ( save_done) {
          // continue with remaining size != 0
          cl_marsh.start();
        }
      }
      wait();
    }
  }

};



#endif /* ACC_TYPED_IO_H_ */
