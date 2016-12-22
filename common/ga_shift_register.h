// See LICENSE for license details.

#ifndef __GA_SHIFT_REGISTER_H__
#define __GA_SHIFT_REGISTER_H__

#include <systemc.h>

typedef uint16_t ShiftRegSize ;

//#define SREG_DEBUG

#ifdef SREG_DEBUG
#define SREG_DBG cout << "[SREG " << sc_time_stamp() << "] "
#else
#define SREG_DBG if(0) cout
#endif

// The synthesizable version
template<class DataType, ShiftRegSize Delay>
class ShiftRegisterSyn : public sc_module {
  
  sc_signal<DataType> regs_[Delay] ;
  sc_signal<DataType> outBuf_ ;
  
  sc_signal<bool> isRegValid_[Delay] ; // the 0-th element is ignored. See below for state encoding instead.
  sc_signal<bool> validEntryWrittenToOutBuf_ ;

  //scide_waive SCIDE.8.8
  // State encoding:
  // shift_thread() tries to make readOpnLastCycle false, while nb_get
  // tries to make it true. We use state encoding to allow multiple writers.
  // shift_thread() writes to V1, nb_get() writes to V0. If V0 and V1 are different,
  // we treat readOpnLastCycle as true, and vice versa.
  // In each cycle, shift_thread() tries to make the value false by making V1 <= V0.
  // On the other hand, nb_get() flips to value of V0 to avoid shift_thread() making
  // it false. If there's no nb_get() in a cycle, shift_thread() succeeds, and
  // vice versa.
  sc_signal<bool> readOpnLastCycleV0_ ;
  sc_signal<bool> readOpnLastCycleV1_ ;

  // State encoding:
  // When there's a successful nb_put(), it's guaranteed that regs_[0] will be
  // shifted in the same cycle in shift_thread(). 
  // Any time there's a shift of regs_[0], shift_thread() tries to invalidate
  // the first elt by making V1_ = V0_. If there's an nb_put() in the same cycle,
  // it will flip the bit to counter this invalidation.
  sc_signal<bool> firstEltValidV0_ ;
  sc_signal<bool> firstEltValidV1_ ;


  bool isReadOpnLastCycle_() const {
    return readOpnLastCycleV0_.read() ^ readOpnLastCycleV1_.read() ;
  }

  // 
  // If there was a read opn last cycle, it's guaranteed that outBuf is invalid.
  // Consider the two cases:
  //   1) The elt read was regs[Delay-1]: This is only possible if the outBuf
  //      was invalid to begin with. The data shifted from regs_[Delay-1] to outBuf
  //      should be ignored.
  //   2) The elt read was outBuf_: In this case, there cannot be a shift from
  //      regs_[Delay-1] to outBuf_ in the same cycle, because the shift_thread() 
  //      doesn't know if there'll be a read or not in the same cycle.
  // 
  bool isOutBufValid_() const {
    return validEntryWrittenToOutBuf_.read() && !isReadOpnLastCycle_() ;
  }

  bool isFirstEltValid_() const {
    return firstEltValidV0_.read() ^ firstEltValidV1_.read() ;
  }

  bool isEltValid_(ShiftRegSize index) const {
    assert(index >= 0 && index < Delay) ;
    return (index == 0) ? isFirstEltValid_() : isRegValid_[index].read() ;
  }

  bool allSlotsFull_() const {
    bool allSlotsFull = isOutBufValid_() & isFirstEltValid_() ; // initialize
    for (ShiftRegSize i=1; i < Delay; ++i) {
      allSlotsFull &= isEltValid_(i) ;
    }
    return allSlotsFull ;
  }

  void shift_thread_() {
    
    reset_shift() ;
    wait() ;
    while (true)  {
      bool nextEntryValid = isEltValid_(Delay-1) ;
      bool movedNextEntry = !isOutBufValid_() && nextEntryValid ;
      if (movedNextEntry) {
        outBuf_.write(regs_[Delay-1].read()) ;
      }
      validEntryWrittenToOutBuf_.write(movedNextEntry | isOutBufValid_()) ;
      
      bool currEntryValid = nextEntryValid ;
      bool movedCurrEntry = movedNextEntry ;

      // The semantic of "next" in the following loop is (i-1), i.e. the next iteration
    SR_UNROLL_LOOP:
      for (ShiftRegSize i=Delay-1; i>0; --i) {
        nextEntryValid = isEltValid_(i-1) ;
      
        movedNextEntry = nextEntryValid & (!currEntryValid | movedCurrEntry) ;
        if (movedNextEntry) {
          regs_[i].write(regs_[i-1].read()) ;
        }
        isRegValid_[i].write(nextEntryValid | (currEntryValid & !movedCurrEntry)) ;

      
        currEntryValid = nextEntryValid ;
        movedCurrEntry = movedNextEntry ;
      }

      if (movedCurrEntry) { // if moved the first entry
        firstEltValidV1_.write(firstEltValidV0_.read()) ;
      }

      
      // unless nb_get() is called this cycle, readOpnLastCycle will be logically false
      readOpnLastCycleV1_.write(readOpnLastCycleV0_.read()) ;
   
#if 0
#ifdef SREG_DEBUG
      SREG_DBG << "The regValid values after shift: " << endl ;
      for (ShiftRegSize i=0; i < Delay; ++i)
        SREG_DBG << i << ": " << isEltValid_(i) << endl ;
      SREG_DBG << "Out buf: " << isOutBufValid_() << endl ;
#endif
#endif

      wait() ;
    }
  }
 public:
  sc_in<bool> Port_Rst ;
  sc_in<bool> Port_Clk ;
    
  
  SC_HAS_PROCESS(ShiftRegisterSyn) ;
  ShiftRegisterSyn(sc_module_name name = sc_gen_unique_name("SR"))
   : sc_module(name) {


    SREG_DBG << "Constructor called" << endl ;

//    reset_put() ;
//    reset_get() ;
//    reset_shift() ;
    
    SC_THREAD(shift_thread_) ;
    sensitive << Port_Clk.pos() ;
    async_reset_signal_is(Port_Rst, false);
  }
  
  template <typename T1,typename T2>
  void clk_rst( T1 clk, T2 rst) {
    Port_Clk( clk);
    Port_Rst( rst);
  }

  // To be called from the process that calls nb_put()
  void reset_put() {
    firstEltValidV0_.write(false) ;
  }

  // To be called from the process that calls nb_get()
  void reset_get() {
    readOpnLastCycleV0_.write(false) ;
  }
  

  // To be called from the shift_thread()
  void reset_shift() {
    for (ShiftRegSize i=0; i < Delay; ++i) {
      isRegValid_[i].write(false) ;
    }
    validEntryWrittenToOutBuf_.write(false) ;
    //readOpnLastCycleV0_.write(false) ;
    readOpnLastCycleV1_.write(false) ;
    //firstEltValidV0_.write(false) ;
    firstEltValidV1_.write(false) ;
  }
  
  // The following is just to have compatible API with the non-synth version.
  // Shift is done internally by the shift_thread(). No need for external call.
  void shift() {}
  
  
  bool nb_can_put() const {

    SREG_DBG << "nb_can_put returns " << !allSlotsFull_() << endl ;
    return !allSlotsFull_() ;
  }
  
  bool nb_can_get() const {
    return isOutBufValid_() || isEltValid_(Delay-1) ; 
  }

  DataType get() {
    while( !nb_can_get()) wait();
    DataType result;
    bool successful = nb_get( result);
    assert( successful);
    return result;
  }

  bool nb_get(DataType& data) {
    
    bool success = false ;
    if (isOutBufValid_()) {
      data = outBuf_.read() ;
      success = true ;
    }
    else if (isEltValid_(Delay-1)) {
      data = regs_[Delay-1].read() ;
      success = true ;
    }
    
    if (success) {
      readOpnLastCycleV0_.write(!readOpnLastCycleV0_.read()) ;
      SREG_DBG << "nb_get returns " << data << endl ;
    }
    
    SREG_DBG << "nb_get not successful" << endl ;
    return success ;
  }
  
  void put(const DataType& data) {
    while( !nb_can_put()) wait();
    bool successful = nb_put( data);
    assert( successful);
  }

  bool nb_put(const DataType& data) {
    
    if (!allSlotsFull_()) {
      regs_[0].write(data) ;
      firstEltValidV0_.write(!firstEltValidV0_.read()) ;

      SREG_DBG << "nb_put writes " << data << endl ;
      return true ;
    }
    
    SREG_DBG << "nb_put not successful" << endl ;
    return false ;
  }
    

} ;


#endif // __GA_SHIFT_REGISTER_H__
