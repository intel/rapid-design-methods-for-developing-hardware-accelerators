// See LICENSE for license details.

#ifndef HLS_UTILS_H_
#define HLS_UTILS_H_

template<typename T, unsigned int N>
struct bitvec_utils {
  //static const T N2 = (N+1)/2;
  static const unsigned int N2 = (N+1)/2 ;
  static bool find_leading_one(const bool *data, T &index) {
    const bool *low_half = &data[0];
    const bool *high_half = &data[N2];

    if (bitvec_utils<T, N2>::has_one(low_half)) {
      bitvec_utils<T, N2>::find_leading_one(low_half, index);
    }
    else
    {
      bitvec_utils<T, N-N2>::find_leading_one(high_half, index);
      index += N2;
    }
    return has_one(data);
  }

  static bool find_any_one(const bool *data, T &index) {
    const bool *low_half = &data[0];
    const bool *high_half = &data[N2];

    if (bitvec_utils<T, N2>::has_one(low_half)) {
      bitvec_utils<T, N2>::find_any_one(low_half, index);
    }
    else
    {
      bitvec_utils<T, N-N2>::find_any_one(high_half, index);
      index += N2;
    }
    return has_one(data);
  }

  static bool has_one(const bool *data) {
    const bool *low_half = &data[0];
    const bool *high_half = &data[N2];
    return bitvec_utils<T, N2>::has_one(low_half) | bitvec_utils<T, N-N2>::has_one(high_half);
  }

  static T add(const bool *data) {
    const bool *low_half = &data[0];
    const bool *high_half = &data[N2];

    return bitvec_utils<T, N2>::add(low_half) + bitvec_utils<T, N-N2>::add(high_half);
  }

};



template<typename T>
struct bitvec_utils<T,1> {
  static bool find_leading_one(const bool *data, T &index) {
    index = 0;
    return data[0];
  }

  static bool find_any_one(const bool *data, T &index) {
    index = 0;
    return data[0];
  }

  static bool has_one(const bool *data) {
    return data[0];
  }

  static T add(const bool *data) {
    return data[0];
  }
};

//template<typename T>
//struct bitvec_utils<T,256> {
//  static bool find_leading_one(const bool *data, T &index){
//    const bool *low_half = &data[0];
//    const bool *high_half = &data[128];
//
//    if (bitvec_utils<T, 128>::has_one(low_half)) {
//      bool low_found = bitvec_utils<T, 128>::find_leading_one(low_half, index);
//      return low_found;
//    }
//    else
//    {
//      bool high_found = bitvec_utils<T, 128>::find_leading_one(high_half, index);
//      index += 128;
//      return high_found;
//    }
//  }
//
//  static bool has_one(const bool *data) {
//    return bitvec_utils<T, 128>::has_one(data) | bitvec_utils<T, 128>::has_one(data[128]);
//  }
//};


template <size_t V>
struct Log2
{
  enum
  {
    Value = Log2<V / 2>::Value + 1
  };
  typedef char V_MustBePowerOfTwo[((V & (V - 1)) == 0 ? 1 : -1)];
};

template<> struct Log2<1> { enum { Value = 0 }; };


template<size_t Delay>
class ShiftMonitor {
  enum {ArrSize = Delay };
  typedef typename SizeT<ArrSize>::Type CounterType;


 public:
  bool isValid_[ArrSize] ;

  void reset() {
    SM_RESET_UNROLL: for (size_t i=0; i < ArrSize; ++i)
      isValid_[i] = false ;
  }

  void shift(bool next) {
    SM_SHIFT_UNROLL: for (size_t i=ArrSize-1; i > 0; --i) {
      isValid_[i] = isValid_[i-1] ;
      isValid_[i-1] = next ;
    }
  }

  bool has_valid() {
    return bitvec_utils<CounterType, ArrSize>::has_one(isValid_);
  }
} ;


template<size_t Delay>
class IdleMonitor : public sc_module {
#ifdef STRATUS
  HLS_INLINE_MODULE;
#endif
public:
  sc_in_clk clk;
  sc_in<bool> rst;

  sc_in<bool> in_idle;
  sc_out<bool> out_idle;
  ShiftMonitor<Delay> shift_monitor;

  SC_HAS_PROCESS (IdleMonitor);

  IdleMonitor(sc_module_name name) : sc_module(name), clk("clk"), rst("rst"), in_idle("in_idle"), out_idle("out_idle") {
    SC_CTHREAD(shift_proc, clk.pos());
    async_reset_signal_is(rst, false);
  }

  void shift_proc() {
    {
      out_idle = false;
      shift_monitor.reset();
    }
    wait();
    while (1) {
      {
        shift_monitor.shift(!in_idle.read());
//        cout << sc_time_stamp() << "Idle monitor: ";
//        for (unsigned i = 0 ; i < Delay; ++i) {
//          cout << shift_monitor.isValid_[i];
//        }
//        cout << endl;
        out_idle = !shift_monitor.has_valid();
      }
      wait();
    }
  }

};

enum GATE_TYPE {
  GATE_AND,
  GATE_OR
};

template <enum GATE_TYPE>
struct GateFunction  {
  static bool compute(bool *ins, size_t M);
};

template <size_t N, enum GATE_TYPE GTYPE>
struct SimpleGate : sc_module {
#ifdef STRATUS
  HLS_INLINE_MODULE;
#endif
  sc_in<bool> ins[N];
  sc_out<bool> out;

  SC_HAS_PROCESS(SimpleGate);

  SimpleGate(sc_module_name name = sc_gen_unique_name("SimpleGate")) :
    sc_module(name)
  {
    SC_METHOD(LogicFunctionMethod);
    for (unsigned i = 0; i < N; ++i)
      sensitive << ins[i];
  }
  void LogicFunctionMethod () {
    bool in_vals[N];
    UNROLL_LOGIC_METHOD: for (unsigned i = 0; i < N; ++i)
      in_vals[i] = ins[i];
    out = GateFunction<GTYPE>::compute(in_vals, N);
  }
};

template<> inline
bool GateFunction<GATE_AND>::compute (bool *ins,size_t N) {
  bool result = true;
  UNROLL_LOGIC_FUNCTION: for (unsigned i = 0; i < N; ++i) {
    result = result & ins[i];
  }
  return result;
}
template<> inline
bool GateFunction<GATE_OR>::compute(bool *ins, size_t N) {
  bool result = false;
  UNROLL_LOGIC_FUNCTION: for (unsigned i = 0; i < N; ++i) {
    result = result | ins[i];
  }
  return result;
}


#endif /* HLS_UTILS_H_ */
