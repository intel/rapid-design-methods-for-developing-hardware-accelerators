// See LICENSE for license details.
/*
 * fpga_app_sw.h
 *
 *  Created on: Oct 7, 2016
 *      Author: aayupov
 */

#ifndef COMMON_FPGA_APP_SW_H_
#define COMMON_FPGA_APP_SW_H_

#include "fpga_app_if.h"

#include "stdlib.h"
#include "stddef.h"

struct MemRange {
  // all in bytes
  size_t offset;
  size_t size;

  MemRange() {}
  MemRange(size_t offset, size_t size) :
    offset(offset), size(size) {}
};

struct IMemRangeHolder {
  virtual MemRange getMemRange() const = 0;
};

class FpgaAppSwAlloc : public IFpgaApp, public IMemRangeHolder {

public:
  // used for app memory allocation
  unsigned char      *m_JointVirt;     ///< Joint workspace pointer.
  size_t              m_JointSize;     ///< Joint workspace size in bytes.

  FpgaAppSwAlloc(const char *afuid = NULL){} 

  virtual void *alloc( unsigned long long size_in_bytes) { // Returns zero on success
    // Doesn't align the memory so you don't necessarily get has much usable memory as you exect
    //    m_JointVirt = (unsigned char *)malloc(size_in_bytes);
    // Works on virtualbox but not EC machines (probably due to gcc 4.7.0 being too old)
    //    m_JointVirt = (unsigned char *) aligned_alloc( 64, size_in_bytes);
    // Works on EC machines with gcc 4.7.0
    if (!posix_memalign( (void **) (&m_JointVirt), 64, size_in_bytes))
    {
      m_JointSize = size_in_bytes;
      return m_JointVirt;
    } else {
      return NULL;
    }
  }
  virtual void  free() {
    ::free(m_JointVirt);
    m_JointVirt = NULL;
  }
  
  virtual void join() {}

  virtual MemRange getMemRange() const {
    return MemRange(reinterpret_cast<size_t>(m_JointVirt), m_JointSize);
  }

};



#endif /* COMMON_FPGA_APP_SW_H_ */
