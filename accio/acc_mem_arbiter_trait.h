// See LICENSE for license details.
/*
 * acc_mem_arbiter_trait.h
 *
 *  Created on: Mar 21, 2016
 *      Author: aayupov
 */

#ifndef ACC_MEM_ARBITER_TRAIT_H_
#define ACC_MEM_ARBITER_TRAIT_H_

#include "types.h"
#include "hls_utils.h"

enum ARBITER_TRAIT {
  IO_ARBITER,
  AU_ARBITER
};

template <unsigned int N>
struct UpperLog2 {
    static inline int getValue() {
      return Log2<N>::Value;
    }

};
template <> inline int UpperLog2<1>::getValue() {
  return 0;
}
template <> inline int UpperLog2<2>::getValue() {
  return 1;
}
template <> inline int UpperLog2<3>::getValue() {
  return 2;
}
template <> inline int UpperLog2<4>::getValue() {
  return 2;
}
template <> inline int UpperLog2<5>::getValue() {
  return 3;
}
template <> inline int UpperLog2<6>::getValue() {
  return 3;
}
template <> inline int UpperLog2<7>::getValue() {
  return 3;
}

template <ARBITER_TRAIT TRAIT=IO_ARBITER>
class ArbiterTagResolver {
public:
  template<unsigned int N>
  static IOUnitIdType addTag(IOUnitIdType tag, IOUnitIdType to_add);
  template<unsigned int N>
  static IOUnitIdType extractTag(IOUnitIdType tag);
};
template <> template<unsigned int N>
IOUnitIdType ArbiterTagResolver<IO_ARBITER>::addTag(IOUnitIdType tag, IOUnitIdType to_add) {
  assert(to_add < N);
  assert(BF_GET(tag, UpperLog2<N>::getValue()-1, 0) == 0);
  BF_SET(tag, to_add, UpperLog2<N>::getValue()-1, 0);
  return tag;
}

template <> template<unsigned int N>
IOUnitIdType ArbiterTagResolver<AU_ARBITER>::addTag(IOUnitIdType tag, IOUnitIdType to_add) {
  assert(BF_GET(tag, 6, 6-(UpperLog2<N>::getValue()-1)) == 0);
  assert(to_add < N);
  unsigned lower = 6-(UpperLog2<N>::getValue()-1);
  BF_SET(tag, to_add, 6, lower);
  return tag;
}

template<> template<unsigned int N>
IOUnitIdType ArbiterTagResolver<IO_ARBITER>::extractTag(IOUnitIdType tag) {
  return BF_GET(tag, UpperLog2<N>::getValue()-1, 0);
}

template<> template<unsigned int N>
IOUnitIdType ArbiterTagResolver<AU_ARBITER>::extractTag(IOUnitIdType tag) {
  unsigned lower = 6-(UpperLog2<N>::getValue()-1);
  return BF_GET(tag, 6, lower);
}

#endif /* ACC_MEM_ARBITER_TRAIT_H_ */
