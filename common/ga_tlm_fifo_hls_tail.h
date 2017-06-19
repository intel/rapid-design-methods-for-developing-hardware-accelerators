// See LICENSE for license details.
/*
 * ga_tlm_fifo_hls.h
 *
 *  Created on: Mar 13, 2015
 *      Author: aayupov
 */

namespace ga {

struct TLM_TRAITS_MULTI
{
    static const bool Level     = 0;
    static const unsigned int Put_Get_Channel_FIFO_Size = 2;
};

struct SIG_TRAITS_MULTI
{
    static const bool Level      = 1;
    static const bool PosEdgeClk = 1;
    static const bool ResetLevel = 0;
    static const bool ResetData  = 0;
    static const bool ResetSync  = 0;

    static const bool Allow_Multiple_Calls_Per_Cycle     = 1;
    static const bool Allow_Multiple_Calls_Per_Cycle_RTL = 1;
};

#define FLEX_TRAIT DEFAULT_TRAITS
//#define FLEX_TRAIT SIG_TRAITS_MULTI

template<typename PAYLOAD>
class tlm_fifo_hier_in<PAYLOAD, HLS_FLEX>: public hier_get_initiator<PAYLOAD, FLEX_TRAIT>
{
public:
	tlm_fifo_hier_in() : hier_get_initiator<PAYLOAD, FLEX_TRAIT>() {

	}

	tlm_fifo_hier_in(const char *name) : hier_get_initiator<PAYLOAD, FLEX_TRAIT>(name) {

	}
	template <typename CLK_T, typename RST_T>
	void clk_rst(CLK_T clk, RST_T rst) { }

};

template<typename PAYLOAD>
class tlm_fifo_hier_out<PAYLOAD, HLS_FLEX> : public hier_put_initiator<PAYLOAD, FLEX_TRAIT>
{
public:
	tlm_fifo_hier_out() : hier_put_initiator<PAYLOAD, FLEX_TRAIT>() {
	}

	tlm_fifo_hier_out(const char *name) : hier_put_initiator<PAYLOAD, FLEX_TRAIT>(name) {
	}
	template <typename CLK_T, typename RST_T>
	void clk_rst(CLK_T clk, RST_T rst) { }

};

template<typename PAYLOAD>
class tlm_fifo_in<PAYLOAD, HLS_FLEX> : public nb_get_initiator<PAYLOAD, FLEX_TRAIT>
{
public:
  tlm_fifo_in() : nb_get_initiator<PAYLOAD, FLEX_TRAIT>() {
  }


  tlm_fifo_in(const char *name) : nb_get_initiator<PAYLOAD, FLEX_TRAIT>(name) {

  }

  bool nb_get(PAYLOAD &data) {
    return nb_get_initiator<PAYLOAD, FLEX_TRAIT>::nb_get(data);
  }

  //bool nb_peek(PAYLOAD &data) {
  //        return (*this)->nb_peek(data) ;
  //}

  bool nb_can_get( ctos_tlm::tlm_tag<PAYLOAD> *t = 0 ) {
    return nb_get_initiator<PAYLOAD, FLEX_TRAIT>::nb_can_get(t);
  }

  sc_event_finder& ok_to_get() const
    {
    return nb_get_initiator<PAYLOAD, FLEX_TRAIT>::ok_to_get();
    }


  void get(PAYLOAD &data) {
    while(!nb_can_get()) sc_core::wait();
    nb_get_initiator<PAYLOAD, FLEX_TRAIT>::nb_get(data);
  }

  PAYLOAD get(ctos_tlm::tlm_tag<PAYLOAD> *t = 0) {
    while(!nb_can_get()) sc_core::wait();
    PAYLOAD data;
    nb_get_initiator<PAYLOAD, FLEX_TRAIT>::nb_get(data);
    return data;
  }

  template <typename CHAN>
  void operator()(CHAN& chan) {
    nb_get_initiator<PAYLOAD, FLEX_TRAIT>::operator() (chan);
  }

};


template<typename PAYLOAD>
class tlm_fifo_out<PAYLOAD, HLS_FLEX> : public nb_put_initiator<PAYLOAD, FLEX_TRAIT>
{
public:

  tlm_fifo_out() : nb_put_initiator<PAYLOAD, FLEX_TRAIT>() {
  }

  tlm_fifo_out(const char *name) : nb_put_initiator<PAYLOAD, FLEX_TRAIT>(name) {
  }

  bool nb_put( const PAYLOAD &t ) {
    return nb_put_initiator<PAYLOAD, FLEX_TRAIT>::nb_put(t);
  }

  bool nb_can_put(ctos_tlm::tlm_tag<PAYLOAD> *t = 0) const
        { return nb_put_initiator<PAYLOAD, FLEX_TRAIT>::nb_can_put(t); }

    // use for static sensitivity to data read event
    sc_event_finder& ok_to_put() const
    {
      return nb_put_initiator<PAYLOAD, FLEX_TRAIT>::ok_to_put();
    }

  void put( const PAYLOAD &t ) {
    while(!nb_can_put()) sc_core::wait();
    nb_put_initiator<PAYLOAD, FLEX_TRAIT>::nb_put(t);
  }

  template <typename CHAN>
  void operator()(CHAN& chan) {
    nb_put_initiator<PAYLOAD, FLEX_TRAIT>::operator() (chan);
  }

};

template<typename PAYLOAD>
class tlm_nb_fifo_in<PAYLOAD, HLS_FLEX> : public nb_get_initiator<PAYLOAD>
{
public:
  tlm_nb_fifo_in() : nb_get_initiator<PAYLOAD>() {
  }


  tlm_nb_fifo_in(const char *name) : nb_get_initiator<PAYLOAD>(name) {

  }

  bool nb_get(PAYLOAD &data) {
    return nb_get_initiator<PAYLOAD>::nb_get(data);
  }

  //bool nb_peek(PAYLOAD &data) {
  //        return (*this)->nb_peek(data) ;
  //}

  bool nb_can_get( ctos_tlm::tlm_tag<PAYLOAD> *t = 0 ) {
    return nb_get_initiator<PAYLOAD>::nb_can_get(t);
  }

  sc_event_finder& ok_to_get() const
    {
    return nb_get_initiator<PAYLOAD>::ok_to_get();
    }


  void get(PAYLOAD &data) {
    while(!nb_can_get()) sc_core::wait();
    nb_get_initiator<PAYLOAD>::nb_get(data);
  }

  PAYLOAD get(ctos_tlm::tlm_tag<PAYLOAD> *t = 0) {
    while(!nb_can_get()) sc_core::wait();
    PAYLOAD data;
    nb_get_initiator<PAYLOAD>::nb_get(data);
    return data;
  }

  template <typename CHAN>
  void operator()(CHAN& chan) {
    nb_get_initiator<PAYLOAD>::operator() (chan);
  }


};


template<typename PAYLOAD>
class tlm_nb_fifo_out<PAYLOAD, HLS_FLEX> : public nb_put_initiator<PAYLOAD>
{
public:

  tlm_nb_fifo_out() : nb_put_initiator<PAYLOAD>() {
  }

  tlm_nb_fifo_out(const char *name) : nb_put_initiator<PAYLOAD>(name) {
  }

  bool nb_put( const PAYLOAD &t ) {
    return nb_put_initiator<PAYLOAD>::nb_put(t);
  }

  bool nb_can_put(ctos_tlm::tlm_tag<PAYLOAD> *t = 0) const
        { return nb_put_initiator<PAYLOAD>::nb_can_put(t); }

    // use for static sensitivity to data read event
    sc_event_finder& ok_to_put() const
    {
      return nb_put_initiator<PAYLOAD>::ok_to_put();
    }

  void put( const PAYLOAD &t ) {
    while(!nb_can_put()) sc_core::wait();
    nb_put_initiator<PAYLOAD>::nb_put(t);
  }

  template <typename CHAN>
  void operator()(CHAN& chan) {
    nb_put_initiator<PAYLOAD>::operator() (chan);
  }
};


template<typename PAYLOAD, unsigned int SIZE>
class tlm_fifo<PAYLOAD, SIZE, HLS_FLEX> : public put_get_channel<PAYLOAD, FLEX_TRAIT>
{
public:
  tlm_fifo(sc_module_name modname) :
    put_get_channel<PAYLOAD, FLEX_TRAIT>(modname)
  {
  }

  tlm_fifo() :
    put_get_channel<PAYLOAD, FLEX_TRAIT>()
  {
  }


  // do not include reads/writes happened in this cycle
  int num_of_elems_as_of_start_of_cycle() const {
    //return tlm_fifo<PAYLOAD>::m_num_readable;
    assert(0);
  }

  // size of the fifo
  int size() const {
    return 0;
  }
};

template<typename PAYLOAD, unsigned int SIZE>
class ga_storage_fifo<PAYLOAD, SIZE, HLS_FLEX> : public ctos_tlm::tlm_fifo<PAYLOAD, SIZE>
{
public:
  ga_storage_fifo(sc_module_name modname) :
    ctos_tlm::tlm_fifo<PAYLOAD, SIZE>(modname)
  {
  }

  ga_storage_fifo() :
     ctos_tlm::tlm_fifo<PAYLOAD, SIZE>()
  {
  }

  virtual bool nb_can_get( ctos_tlm::tlm_tag<PAYLOAD> *t = 0 ) const {
    return ctos_tlm::tlm_fifo<PAYLOAD, SIZE>::nb_can_get(t);
  }

  virtual bool nb_can_put( ctos_tlm::tlm_tag<PAYLOAD> *t = 0 ) const {
    return ctos_tlm::tlm_fifo<PAYLOAD, SIZE>::nb_can_put(t);
  }


  virtual PAYLOAD get( tlm_tag<PAYLOAD> *t = 0 ) {
    return ctos_tlm::tlm_fifo<PAYLOAD, SIZE>::get(t);
  }

  virtual bool nb_get( PAYLOAD& d) {
    return ctos_tlm::tlm_fifo<PAYLOAD, SIZE>::nb_get(d);
  }

  virtual void reset_get(tlm_tag<PAYLOAD> *t = 0) {
    ctos_tlm::tlm_fifo<PAYLOAD, SIZE>::reset_get(t);
  }

  virtual void put( const PAYLOAD& d) {
    ctos_tlm::tlm_fifo<PAYLOAD, SIZE>::put(d);
  }

  virtual bool nb_put( const PAYLOAD& d) {
    return ctos_tlm::tlm_fifo<PAYLOAD, SIZE>::nb_put(d);
  }

  virtual void reset_put(tlm_tag<PAYLOAD> *t = 0) {
    ctos_tlm::tlm_fifo<PAYLOAD, SIZE>::reset_put(t);
  }

  virtual bool nb_peek( PAYLOAD& t) const {
    return ctos_tlm::tlm_fifo<PAYLOAD, SIZE>::nb_peek(t);
  }

  virtual bool nb_can_peek( tlm_tag<PAYLOAD> *t = 0 ) const {
    return ctos_tlm::tlm_fifo<PAYLOAD, SIZE>::nb_can_peek(t);
  }


  virtual bool is_full() const {return ctos_tlm::tlm_fifo<PAYLOAD, SIZE>::is_full();}
  virtual bool is_empty() const {return ctos_tlm::tlm_fifo<PAYLOAD, SIZE>::is_empty();}
  virtual int num_items() const {return ctos_tlm::tlm_fifo<PAYLOAD, SIZE>::num_items();}

  // do not include reads/writes happened in this cycle
  int num_of_elems_as_of_start_of_cycle() const {
    //return tlm::tlm_fifo<PAYLOAD>::m_num_readable;
    assert(0);
  }


  // size of the fifo
  int size() const {
    return ctos_tlm::tlm_fifo<PAYLOAD, SIZE>::size();
  }
};

template<typename PAYLOAD, unsigned int SIZE>
class ga_storage_fifo_reg<PAYLOAD, SIZE, HLS_FLEX> : public ctos_tlm::tlm_fifo_reg<PAYLOAD, SIZE>
{
public:
  ga_storage_fifo_reg(sc_module_name modname) :
    ctos_tlm::tlm_fifo_reg<PAYLOAD, SIZE>(modname)
  {
  }

  ga_storage_fifo_reg() :
     ctos_tlm::tlm_fifo_reg<PAYLOAD, SIZE>()
  {
  }

  virtual bool nb_can_get( ctos_tlm::tlm_tag<PAYLOAD> *t = 0 ) const {
    return ctos_tlm::tlm_fifo_reg<PAYLOAD, SIZE>::nb_can_get(t);
  }

  virtual bool nb_can_put( ctos_tlm::tlm_tag<PAYLOAD> *t = 0 ) const {
    return ctos_tlm::tlm_fifo_reg<PAYLOAD, SIZE>::nb_can_put(t);
  }


  virtual PAYLOAD get( tlm_tag<PAYLOAD> *t = 0 ) {
    return ctos_tlm::tlm_fifo_reg<PAYLOAD, SIZE>::get(t);
  }

  virtual bool nb_get( PAYLOAD& d) {
    return ctos_tlm::tlm_fifo_reg<PAYLOAD, SIZE>::nb_get(d);
  }

  virtual void reset_get(tlm_tag<PAYLOAD> *t = 0) {
    ctos_tlm::tlm_fifo_reg<PAYLOAD, SIZE>::reset_get(t);
  }

  virtual void put( const PAYLOAD& d) {
    ctos_tlm::tlm_fifo_reg<PAYLOAD, SIZE>::put(d);
  }

  virtual bool nb_put( const PAYLOAD& d) {
    return ctos_tlm::tlm_fifo_reg<PAYLOAD, SIZE>::nb_put(d);
  }

  virtual void reset_put(tlm_tag<PAYLOAD> *t = 0) {
    ctos_tlm::tlm_fifo_reg<PAYLOAD, SIZE>::reset_put(t);
  }

  virtual bool nb_peek( PAYLOAD& t) const {
    return ctos_tlm::tlm_fifo_reg<PAYLOAD, SIZE>::nb_peek(t);
  }

  virtual bool nb_can_peek( tlm_tag<PAYLOAD> *t = 0 ) const {
    return ctos_tlm::tlm_fifo_reg<PAYLOAD, SIZE>::nb_can_peek(t);
  }


  virtual bool is_full() const {return ctos_tlm::tlm_fifo_reg<PAYLOAD, SIZE>::is_full();}
  virtual bool is_empty() const {return ctos_tlm::tlm_fifo_reg<PAYLOAD, SIZE>::is_empty();}
  virtual int num_items() const {return ctos_tlm::tlm_fifo_reg<PAYLOAD, SIZE>::num_items();}

  // do not include reads/writes happened in this cycle
  int num_of_elems_as_of_start_of_cycle() const {
    //return tlm::tlm_fifo_reg<PAYLOAD>::m_num_readable;
    assert(0);
  }


  // size of the fifo
  int size() const {
    return ctos_tlm::tlm_fifo_reg<PAYLOAD, SIZE>::size();
  }
};

template<typename PAYLOAD, unsigned int SIZE>
class ga_storage_fifo_fast<PAYLOAD, SIZE, HLS_FLEX>
{
  ctos_tlm::tlm_fifo<PAYLOAD, SIZE> fifo;

  sc_in_clk clk;
  sc_in<bool> rst;

  sc_signal<PAYLOAD> flop;
  sc_signal<bool> flop_valid;

public:
  SC_HAS_PROCESS(ga_storage_fifo_fast);
  ga_storage_fifo_fast(sc_module_name modname) :
    fifo(modname)
  {
  }

  ga_storage_fifo_fast()
  {
  }


  virtual bool nb_can_get( ctos_tlm::tlm_tag<PAYLOAD> *t = 0 ) {
    return flop_valid;
  }

  virtual bool nb_can_put( ctos_tlm::tlm_tag<PAYLOAD> *t = 0 ) const {
    return fifo.nb_can_put(t);
  }


  virtual PAYLOAD get( tlm_tag<PAYLOAD> *t = 0 ) {
    while (!flop_valid) wait();
    return flop;
  }

  virtual bool nb_get( PAYLOAD& d) {
    PAYLOAD result = flop;
    bool valid = flop_valid;
    flop_valid = fifo.nb_get(flop);
    d = result;
    return valid;
  }

  virtual void reset_get(tlm_tag<PAYLOAD> *t = 0) {
    fifo.reset_get(t);
    flop_valid = false;
  }

  virtual void put( const PAYLOAD& d) {
    fifo.put(d);
  }

  virtual bool nb_put( const PAYLOAD& d) {
    return fifo.nb_put(d);
  }

  virtual void reset_put(tlm_tag<PAYLOAD> *t = 0) {
    fifo.reset_put(t);
  }

  virtual bool is_full() const {assert(0);return fifo.is_full();}
  virtual bool is_empty() const {assert(0);return fifo.is_empty();}
  virtual int num_items() const {assert(0);return fifo.num_items();}

  // do not include reads/writes happened in this cycle
  int num_of_elems_as_of_start_of_cycle() const {
    //return tlm::tlm_fifo<PAYLOAD>::m_num_readable;
    assert(0);
  }


  // size of the fifo
  int size() const {
    return ctos_tlm::tlm_fifo<PAYLOAD, SIZE>::size();
  }
};



}

