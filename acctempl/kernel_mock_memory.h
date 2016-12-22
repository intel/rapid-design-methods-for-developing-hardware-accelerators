// See LICENSE for license details.
#ifndef __KERNEL_MOCK_MEMORY_H__
#define __KERNEL_MOCK_MEMORY_H__

// Assumes systemc.h and types.h have been included

#include <deque>

template<typename P>
class MemTypedRead {

  typedef MemTypedReadReqType<P> ReqType;
  typedef MemTypedReadRespType<P> RespType;

  ga::tlm_fifo_in<ReqType>& reqIn_;
  ga::tlm_fifo_out<RespType>& respOut_;

public:

  MemTypedRead( ga::tlm_fifo_in<ReqType>& reqIn,
                ga::tlm_fifo_out<RespType>& respOut)
    : reqIn_(reqIn), respOut_(respOut) {
    reqIn_.reset_get();
    respOut_.reset_put();
  }

  void operator()() {
    ReqType read_req = reqIn_.get();
    const P *ptr = reinterpret_cast<const P*>( (size_t) read_req.addr);
    assert( read_req.size > 0);
    for( size_t i=0; i < read_req.size; ++i) {
      respOut_.put( RespType(ptr[i]));
      if ( i+1 < read_req.size) {
        wait();
      }
    }
  }

};

template<typename P, typename T>
class MemSingleRead {

  typedef MemSingleReadReqType<P,T> ReqType;
  typedef MemSingleReadRespType<P,T> RespType;

  ga::tlm_fifo_in<ReqType>& reqIn_;
  ga::tlm_fifo_out<RespType>& respOut_;

  std::deque<RespType> deq;

public:

  MemSingleRead( ga::tlm_fifo_in<ReqType>& reqIn,
                 ga::tlm_fifo_out<RespType>& respOut)
    : reqIn_(reqIn), respOut_(respOut) {
    reqIn_.reset_get();
    respOut_.reset_put();
  }

  void operator()() {
    if ( reqIn_.nb_can_get() && deq.size()<128) {
      ReqType read_req;
      reqIn_.nb_get(read_req);

      const P *ptr = reinterpret_cast<const P*>( (size_t) read_req.addr);
      
      deq.push_back( RespType( *ptr, read_req.utag));
    }        
    if ( respOut_.nb_can_put() && !deq.empty()) {
      respOut_.nb_put( deq.front());
      deq.pop_front();
    }
  }

};

template<typename P>
class MemTypedWrite {

  typedef MemTypedWriteReqType<P> ReqType;
  typedef MemTypedWriteDataType<P> DataType;

  ga::tlm_fifo_in<ReqType>& reqIn_;
  ga::tlm_fifo_in<DataType>& dataIn_;

public:

  MemTypedWrite( ga::tlm_fifo_in<ReqType>& reqIn,
                 ga::tlm_fifo_in<DataType>& dataIn)
    : reqIn_(reqIn), dataIn_(dataIn) {
    reqIn_.reset_get();
    dataIn_.reset_get();
  }

  void operator()() {
    ReqType write_req = reqIn_.get();
    P *ptr = reinterpret_cast<P*>( (size_t) write_req.addr);
    assert(write_req.size>0);

    for( size_t i=0; i < write_req.size; ++i) {
      ptr[i] = dataIn_.get().data;
      if ( i+1 < write_req.size) {
        wait();
      }
    }
  }

};
#endif
