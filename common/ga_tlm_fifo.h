// See LICENSE for license details.

#ifndef __GA_TLM_FIFO_H__
#define __GA_TLM_FIFO_H__



#ifndef USE_HLS
#include <tlm.h>
#define TLM_NMSPC tlm
using namespace tlm;
//#define SC_SIGNAL_MULTIW(PAYLOAD, VAR)   sc_signal<PAYLOAD, SC_MANY_WRITERS> VAR
#ifdef __SNPS_SYSTEMC__
#define SC_SIGNAL_MULTIW(PAYLOAD, VAR)   sc_signal<PAYLOAD> VAR
#else
#define SC_SIGNAL_MULTIW(PAYLOAD, VAR)   sc_signal<PAYLOAD, SC_MANY_WRITERS> VAR
#endif //__SNPS_SYSTEMC__

#else

#ifdef STRATUS
#include "ga_tlm_fifo_hls_head-stratus.h"
#else
#include "ga_tlm_fifo_hls_head.h"
#endif

#endif

using namespace sc_core;

namespace ga {
enum CHANNEL_TRAIT {GA_TLM=0, HLS_FLEX=1};
#ifdef USE_HLS
#define DEFAULT_CHANNEL_TRAIT HLS_FLEX
#else
#define DEFAULT_CHANNEL_TRAIT GA_TLM
#endif
template<typename PAYLOAD, CHANNEL_TRAIT CHTRAIT=DEFAULT_CHANNEL_TRAIT>
class tlm_fifo_in : public sc_port< TLM_NMSPC::tlm_get_if< PAYLOAD > , 1 >
{
public:
	typedef TLM_NMSPC::tlm_get_if< PAYLOAD > tlm_get_if;

	tlm_fifo_in() : sc_port<tlm_get_if>() {

	}

	tlm_fifo_in(const char *name) : sc_port<tlm_get_if>(name) {

	}

	bool nb_get(PAYLOAD &data) {
		return (*this)->nb_get(data);
	}

	//bool nb_peek(PAYLOAD &data) {
	//        return (*this)->nb_peek(data) ;
	//}

	bool nb_can_get( TLM_NMSPC::tlm_tag<PAYLOAD> *t = 0 ) {
		return (*this)->nb_can_get(t);
	}

	sc_event_finder& ok_to_get() const
    {
		return *new TLM_NMSPC::tlm_event_finder_t<tlm_get_if, PAYLOAD>(
				*this, &tlm_get_if::ok_to_get);
    }


	void get(PAYLOAD &data) {
		while (!nb_can_get()) sc_core::wait();
		(*this)->nb_get(data);
	}

	PAYLOAD get(TLM_NMSPC::tlm_tag<PAYLOAD> *t = 0) {
		while (!nb_can_get(t)) sc_core::wait();
		return (*this)->get(t);
	}


  // empty reset methods for TLM
  void reset_get() {}
  // empty clk_rst binding for TLM
  template <typename CLK_T, typename RST_T>
  void clk_rst(CLK_T clk, RST_T rst) { }

};

template<typename PAYLOAD, CHANNEL_TRAIT CHTRAIT=DEFAULT_CHANNEL_TRAIT>
class tlm_fifo_out : public sc_port<TLM_NMSPC::tlm_put_if<PAYLOAD> >
{
public:
	typedef TLM_NMSPC::tlm_put_if<PAYLOAD>  tlm_put_if;

	tlm_fifo_out() : sc_port<tlm_put_if>() {
	}

	tlm_fifo_out(const char *name) : sc_port<tlm_put_if>(name) {
	}

	bool nb_put( const PAYLOAD &t ) {
		return (*this)->nb_put(t);
	}

	bool nb_can_put(TLM_NMSPC::tlm_tag<PAYLOAD> *t = 0) const
        { return (*this)->nb_can_put(t); }

    // use for static sensitivity to data read event
    sc_event_finder& ok_to_put() const
    {
      return *new TLM_NMSPC::tlm_event_finder_t<tlm_put_if, PAYLOAD>(
	    *this, &tlm_put_if::ok_to_put );
    }

	void put( const PAYLOAD &t ) {
		while(!nb_can_put()) sc_core::wait();
		(*this)->put(t);
	}

  // empty reset methods for TLM
  void reset_put() {}
  // empty clk_rst binding for TLM
  template <typename CLK_T, typename RST_T>
  void clk_rst(CLK_T clk, RST_T rst) { }

};

template<typename PAYLOAD, CHANNEL_TRAIT CHTRAIT=DEFAULT_CHANNEL_TRAIT>
class tlm_fifo_hier_in : public sc_port< TLM_NMSPC::tlm_get_if< PAYLOAD > , 1 >
{
public:
	typedef TLM_NMSPC::tlm_get_if< PAYLOAD > tlm_get_if;

	tlm_fifo_hier_in() : sc_port<tlm_get_if>() {

	}

	tlm_fifo_hier_in(const char *name) : sc_port<tlm_get_if>(name) {

	}
	template <typename CLK_T, typename RST_T>
	void clk_rst(CLK_T clk, RST_T rst) { }

};

template<typename PAYLOAD, CHANNEL_TRAIT CHTRAIT=DEFAULT_CHANNEL_TRAIT>
class tlm_fifo_hier_out : public sc_port<TLM_NMSPC::tlm_put_if<PAYLOAD> >
{
public:
	typedef TLM_NMSPC::tlm_put_if<PAYLOAD>  tlm_put_if;

	tlm_fifo_hier_out() : sc_port<tlm_put_if>() {
	}

	tlm_fifo_hier_out(const char *name) : sc_port<tlm_put_if>(name) {
	}
	template <typename CLK_T, typename RST_T>
	void clk_rst(CLK_T clk, RST_T rst) { }

};



template<typename PAYLOAD, CHANNEL_TRAIT CHTRAIT=DEFAULT_CHANNEL_TRAIT>
class tlm_nb_fifo_in : public sc_port< TLM_NMSPC::tlm_nonblocking_get_if< PAYLOAD > , 1 >
{
public:
	typedef TLM_NMSPC::tlm_nonblocking_get_if< PAYLOAD > tlm_get_if;

	tlm_nb_fifo_in() : sc_port<tlm_get_if>() {

	}

	tlm_nb_fifo_in(const char *name) : sc_port<tlm_get_if>(name) {

	}

	bool nb_get(PAYLOAD &data) {
		return (*this)->nb_get(data);
	}

	//bool nb_peek(PAYLOAD &data) {
	//        return (*this)->nb_peek(data) ;
	//}

	bool nb_can_get( TLM_NMSPC::tlm_tag<PAYLOAD> *t = 0 ) {
		return (*this)->nb_can_get(t);
	}

	sc_event_finder& ok_to_get() const
    {
		return *new TLM_NMSPC::tlm_event_finder_t<tlm_get_if, PAYLOAD>(
				*this, &tlm_get_if::ok_to_get);
    }

  // empty reset methods for TLM
  void reset_get() {}
  // empty clk_rst binding for TLM
  template <typename CLK_T, typename RST_T>
  void clk_rst(CLK_T clk, RST_T rst) { }

};




template<typename PAYLOAD, CHANNEL_TRAIT CHTRAIT=DEFAULT_CHANNEL_TRAIT>
class tlm_nb_fifo_out : public sc_port<TLM_NMSPC::tlm_nonblocking_put_if<PAYLOAD> >
{
public:
	typedef TLM_NMSPC::tlm_nonblocking_put_if<PAYLOAD>  tlm_put_if;

	tlm_nb_fifo_out() : sc_port<tlm_put_if>() {
	}

	tlm_nb_fifo_out(const char *name) : sc_port<tlm_put_if>(name) {
	}

	bool nb_put( const PAYLOAD &t ) {
		return (*this)->nb_put(t);
	}

	bool nb_can_put(TLM_NMSPC::tlm_tag<PAYLOAD> *t = 0) const
        { return (*this)->nb_can_put(t); }

	// use for static sensitivity to data read event
	sc_event_finder& ok_to_put() const
	{
	  return *new TLM_NMSPC::tlm_event_finder_t<tlm_put_if, PAYLOAD>(
	      *this, &tlm_put_if::ok_to_put );
	}

	// empty reset methods for TLM
	void reset_put() {}
  // empty clk_rst binding for TLM
  template <typename CLK_T, typename RST_T>
  void clk_rst(CLK_T clk, RST_T rst) { }

};


template<typename PAYLOAD, CHANNEL_TRAIT CHTRAIT=DEFAULT_CHANNEL_TRAIT>
class tlm_b_fifo_in : public sc_port< TLM_NMSPC::tlm_get_if< PAYLOAD > , 1 >
{
public:
	typedef TLM_NMSPC::tlm_get_if< PAYLOAD > tlm_get_if;

	tlm_b_fifo_in() : sc_port<tlm_get_if>() {

	}

	tlm_b_fifo_in(const char *name) : sc_port<tlm_get_if>(name) {

	}

	bool nb_can_get( TLM_NMSPC::tlm_tag<PAYLOAD> *t = 0 ) {
		return (*this)->nb_can_get(t);
	}
    
    void get(PAYLOAD &data) {
		while (!nb_can_get()) sc_core::wait();
		(*this)->nb_get(data);
	}

	PAYLOAD get(TLM_NMSPC::tlm_tag<PAYLOAD> *t = 0) {
		while (!nb_can_get(t)) sc_core::wait();
		return (*this)->get(t);
	}

	// empty reset methods for TLM
  void reset_get() {}
  // empty clk_rst binding for TLM
  template <typename CLK_T, typename RST_T>
  void clk_rst(CLK_T clk, RST_T rst) { }

};


template<typename PAYLOAD, CHANNEL_TRAIT CHTRAIT=DEFAULT_CHANNEL_TRAIT>
class tlm_b_fifo_out : public sc_port<TLM_NMSPC::tlm_put_if<PAYLOAD> >
{
public:
	typedef TLM_NMSPC::tlm_put_if<PAYLOAD> tlm_put_if;

	tlm_b_fifo_out() : sc_port<tlm_put_if>() {
	}

	tlm_b_fifo_out(const char *name) : sc_port<tlm_put_if>(name) {
	}
    
   	bool nb_can_put(TLM_NMSPC::tlm_tag<PAYLOAD> *t = 0) const
    { return (*this)->nb_can_put(t); }


	void put( const PAYLOAD &t ) {
		while(!nb_can_put()) sc_core::wait();
		(*this)->put(t);
	}

  // empty reset methods for TLM
  void reset_put() {}
  // empty clk_rst binding for TLM
  template <typename CLK_T, typename RST_T>
  void clk_rst(CLK_T clk, RST_T rst) { }

};


template<typename PAYLOAD>
class tlm_fifo_in_exp : public sc_export< TLM_NMSPC::tlm_get_if< PAYLOAD > >
{
public:
  typedef TLM_NMSPC::tlm_get_if< PAYLOAD > tlm_get_if;

  tlm_fifo_in_exp() : sc_export<tlm_get_if>() {

  }

  tlm_fifo_in_exp(const char *name) : sc_export<tlm_get_if>(name) {

  }

  bool nb_get(PAYLOAD &data) {
    return (*this)->nb_get(data);
  }

  //bool nb_peek(PAYLOAD &data) {
  //        return (*this)->nb_peek(data) ;
  //}

  bool nb_can_get( TLM_NMSPC::tlm_tag<PAYLOAD> *t = 0 ) {
    return (*this)->nb_can_get(t);
  }

  sc_event_finder& ok_to_get() const
    {
    return *new TLM_NMSPC::tlm_event_finder_t<tlm_get_if, PAYLOAD>(
        *this, &tlm_get_if::ok_to_get);
    }


  void get(PAYLOAD &data) {
    while (!nb_can_get()) sc_core::wait();
    (*this)->nb_get(data);
  }

  PAYLOAD get(TLM_NMSPC::tlm_tag<PAYLOAD> *t = 0) {
    while (!nb_can_get(t)) sc_core::wait();
    return (*this)->get(t);
  }
  // empty reset methods for TLM
  void reset_get() {}

};



template<typename PAYLOAD>
class tlm_fifo_out_exp : public sc_export<TLM_NMSPC::tlm_put_if<PAYLOAD> >
{
public:
  typedef TLM_NMSPC::tlm_put_if<PAYLOAD>  tlm_put_if;

  tlm_fifo_out_exp() : sc_export<tlm_put_if>() {
  }

  tlm_fifo_out_exp(const char *name) : sc_export<tlm_put_if>(name) {
  }

  bool nb_put( const PAYLOAD &t ) {
    return (*this)->nb_put(t);
  }

  bool nb_can_put(TLM_NMSPC::tlm_tag<PAYLOAD> *t = 0) const
        { return (*this)->nb_can_put(t); }

    // use for static sensitivity to data read event
    sc_event_finder& ok_to_put() const
    {
  return *new TLM_NMSPC::tlm_event_finder_t<tlm_put_if, PAYLOAD>(
      *this, &tlm_put_if::ok_to_put );
    }

  void put( const PAYLOAD &t ) {
    while(!nb_can_put()) sc_core::wait();
    (*this)->put(t);
  }
};

template<typename PAYLOAD, unsigned int SIZE=2, CHANNEL_TRAIT CHTRAIT=DEFAULT_CHANNEL_TRAIT>
class tlm_fifo {};

template<typename PAYLOAD, unsigned int SIZE=2, CHANNEL_TRAIT CHTRAIT=DEFAULT_CHANNEL_TRAIT>
class ga_storage_fifo {};

template<typename PAYLOAD, unsigned int SIZE=2, CHANNEL_TRAIT CHTRAIT=DEFAULT_CHANNEL_TRAIT>
class ga_storage_fifo_fast {};

#ifndef __SYNTHESIS__

template<typename PAYLOAD, unsigned int SIZE>
class tlm_fifo<PAYLOAD, SIZE, GA_TLM> : public TLM_NMSPC::tlm_fifo<PAYLOAD>
{
public:
  tlm_fifo(sc_module_name modname) :
     TLM_NMSPC::tlm_fifo<PAYLOAD>(modname, SIZE)
  {
  }

  tlm_fifo() :
     TLM_NMSPC::tlm_fifo<PAYLOAD>(SIZE)
  {
  }

  // empty reset methods for TLM
  void reset_put() {
    TLM_NMSPC::tlm_fifo<PAYLOAD>::init(SIZE);
  }
  void reset_get() {
    TLM_NMSPC::tlm_fifo<PAYLOAD>::init(SIZE);
  }

  // do not include reads/writes happened in this cycle
  int num_of_elems_as_of_start_of_cycle() const {
    return TLM_NMSPC::tlm_fifo<PAYLOAD>::m_num_readable;
  }

  // size of the fifo
  int size() const {
    return TLM_NMSPC::tlm_fifo<PAYLOAD>::size();
  }
};


template<typename PAYLOAD, unsigned int SIZE>
class ga_storage_fifo<PAYLOAD, SIZE, GA_TLM>  : public TLM_NMSPC::tlm_fifo<PAYLOAD>
{
public:
  ga_storage_fifo(sc_module_name modname) :
     TLM_NMSPC::tlm_fifo<PAYLOAD>(modname, SIZE)
  {
  }

  ga_storage_fifo() :
     TLM_NMSPC::tlm_fifo<PAYLOAD>(SIZE)
  {
  }

  // not quite reset methods but do the tricks. apparently tlm1 fifo doesn't have reset and its init method is bogus as it doesn't delete content of the circular buffer
  void reset_put() {
    while(this->nb_can_get()) {
      this->get();
    }
  }
  void reset_get() {
    while(this->nb_can_get()) {
      this->get();
    }
  }


  // do not include reads/writes happened in this cycle
  int num_of_elems_as_of_start_of_cycle() const {
    return TLM_NMSPC::tlm_fifo<PAYLOAD>::m_num_readable;
  }

  // size of the fifo
  int size() const {
    return TLM_NMSPC::tlm_fifo<PAYLOAD>::size();
  }
  // empty clk_rst binding for TLM
  template <typename CLK_T, typename RST_T>
  void clk_rst(CLK_T clk, RST_T rst) { }

};

template<typename PAYLOAD, unsigned int SIZE>
class ga_storage_fifo_fast<PAYLOAD, SIZE, GA_TLM>  : public ga_storage_fifo<PAYLOAD, SIZE, GA_TLM> {

};

#endif

}


#ifdef USE_HLS

#ifdef STRATUS
#include "ga_tlm_fifo_hls_tail-stratus.h"
#else
#include "ga_tlm_fifo_hls_tail.h"
#endif

#endif

#endif //__GA_TLM_FIFO_H__

