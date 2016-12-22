// See LICENSE for license details.

#ifndef REINTERPRET_UTILS_H_
#define REINTERPRET_UTILS_H_


#include "systemc.h"

#ifdef __AAL_USER__
#include "sysc/datatypes/fx/sc_fixed.h"
#include "sysc/datatypes/fx/sc_ufixed.h"
typedef long long unsigned UInt64;
#else
#include "types.h"
#endif


struct cast_utils {

  // setRange implementation

template <size_t N>
static void setRange(unsigned long int &d, UInt64 rhs) {
  d = (unsigned long int)rhs;
}

template <size_t N>
static void setRange(unsigned int &d, UInt64 rhs) {
  d = (unsigned int) rhs;
}

template <size_t N>
static void setRange(long int &d, UInt64 rhs) {
  d = (long int)rhs;
}

template <size_t N>
static void setRange(float &d, unsigned long long rhs) {
  memcpy(&d, &rhs, N/8);
}

template <size_t N>
static void setRange(double &d, unsigned long long rhs) {
  memcpy(&d, &rhs, N/8);
}

template <int N>
static void setRange(sc_uint<N> &d, UInt64 rhs) {
  d = rhs;
}

template <int N>
static void setRange(sc_int<N> &d, UInt64 rhs) {
  d = rhs;
}

#ifdef __SYNTHESIS__
template <int N, int M, sc_dt::sc_q_mode Q, sc_dt::sc_o_mode O, int BITS>
static void setRange(ctos_sc_dt::sc_fixed<N,M,Q,O,BITS> &d, UInt64 rhs) {
  CTOS_FX_ASSIGN_RANGE(d, rhs);
}
template <int N, int M, sc_dt::sc_q_mode Q, sc_dt::sc_o_mode O, int BITS>
static void setRange(ctos_sc_dt::sc_ufixed<N,M,Q,O,BITS> &d, UInt64 rhs) {
  CTOS_FX_ASSIGN_RANGE(d, rhs);
}
#else
template <int N, int M, sc_dt::sc_q_mode Q, sc_dt::sc_o_mode O, int BITS>
static void setRange(sc_dt::sc_fixed<N,M,Q,O,BITS> &d, UInt64 rhs) {
  d.range(N-1,0) = rhs;
}
template <int N, int M, sc_dt::sc_q_mode Q, sc_dt::sc_o_mode O, int BITS>
static void setRange(sc_dt::sc_ufixed<N,M,Q,O,BITS> &d, UInt64 rhs) {
  d.range(N-1,0) = rhs;
}
#endif


// getRange implementation

//
//template <size_t N, typename T2, typename T>
//static T2 getRange(const T &d) {
//  //should call specialization
//  assert(0);
//}

//template <size_t N>
//static UInt64 getRange(const unsigned long int &d) {
//  return d;
//}

template <size_t N>
static UInt64 getRange(const unsigned int &d) {
  return d;
}


template <size_t N>
static UInt64 getRange(const long unsigned int &d) {
  return d;
}

template <size_t N>
static UInt64 getRange(const long long unsigned int &d) {
  return d;
}


template <size_t N>
static UInt64 getRange(const long int &d) {
  UInt64 result = 0;
  assert(N < 64);
  memcpy(&result, &d, N/8);
  return result;
}

template <size_t N>
static UInt64 getRange(const long long int &d) {
  UInt64 result = 0;
  assert(N <= 64);
  memcpy(&result, &d, N/8);
  return result;
}



template <size_t N, int M>
static UInt64 getRange(const sc_dt::sc_uint<M> &d) {
  assert(M>=N);
  return d.range(N-1, 0);
}

template <size_t N, int M>
static UInt64 getRange(const sc_int<M> &d) {
  assert(M>=N);
  return d.range(N-1, 0);
}

template <size_t N>
static UInt64 getRange(const double &d) {
  unsigned long long result = 0;
  assert(N <= 64);
  memcpy(&result, &d, N/8);
  return (UInt64)result;
}

template <size_t N>
static UInt64 getRange(const float &d) {
  unsigned long long result = 0;
  assert(N <= 64);
  memcpy(&result, &d, N/8);
  return (UInt64)result;
}



#ifdef __SYNTHESIS__

template <int N, int M, sc_dt::sc_q_mode Q, sc_dt::sc_o_mode O, int BITS>
static UInt64 getRange(const ctos_sc_dt::sc_fixed<N,M,Q,O,BITS> &t) {
  return t.range();
}
template <int N, int M, sc_dt::sc_q_mode Q, sc_dt::sc_o_mode O, int BITS>
static UInt64 getRange(const ctos_sc_dt::sc_ufixed<N,M,Q,O,BITS> &t) {
  return t.range();
}
#else

template <int N, int M, sc_dt::sc_q_mode Q, sc_dt::sc_o_mode O, int BITS>
static UInt64 getRange(const sc_dt::sc_fixed<N,M,Q,O,BITS> &d) {
  return d.range(N-1, 0).to_uint64();
}
template <int N, int M, sc_dt::sc_q_mode Q, sc_dt::sc_o_mode O, int BITS>
static UInt64 getRange(const sc_dt::sc_ufixed<N,M,Q,O,BITS> &d) {
  return d.range(N-1, 0).to_uint64();
}
#endif

template <size_t N, typename T2, typename T1>
static T2 reinterpret_type(const T1 &t) {
  T2 result;
  cast_utils::setRange<N>(result, cast_utils::getRange<N>(t));
  return result;
}

//
//#ifdef __SYNTHESIS__
//template <typename T2, typename T1, size_t N, size_t M>
//static T2 reinterpret_type(const ctos_sc_dt::sc_fixed<N, M> &t) {
//  T2 result = T2(t.range());
//  return result;
//}
//#else
//template <typename T2, typename T1, size_t N, size_t M>
//static T2 reinterpret_type(const sc_dt::sc_fixed<N, M> &t) {
//  T2 result = t.range();
//  return result;
//}
//#endif
//template <size_t N, typename T2, typename T1>
//static T2 reinterpret_type(const T1 &t) {
//  T2 result;
//  assert(N == 8*sizeof(t));
//  assert(N == 8*sizeof(result));
//  memcpy(&result, &t, sizeof(t));
//  return result;
//}
// to match sc_fixed type signature
//template <typename T2, typename T1, size_t N, size_t M>
//static T2 reinterpret_type(const T1 &t) {
//  T2 result;
//  assert(N == 8*sizeof(t));
//  assert(N == 8*sizeof(result));
//  memcpy(&result, &t, sizeof(t));
//  return result;
//}

};



#endif /* REINTERPRET_UTILS_H_ */
