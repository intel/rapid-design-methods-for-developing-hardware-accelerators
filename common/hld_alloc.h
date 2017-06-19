// See LICENSE for license details.
#ifndef __HLD_ALLOC_H__
#define __HLD_ALLOC_H__

#ifndef __SYNTHESIS__
#include <new>
#include "stddef.h"
#include "assert.h"
#endif 

class hld_alloc {
  char *buf;
  size_t buf_size;
  size_t offset;
  static const size_t align_in_bytes = 64;

public:
  hld_alloc(char *workspace, size_t workspace_size) :
    buf(workspace), buf_size(workspace_size), offset(0)
  {
    // if workspace (buf) addr is not cache aligned, make it so here
    size_t new_addr = get_next_aligned_addr(size_t(buf));
    if (new_addr != (size_t)buf) {
      buf_size -= align_in_bytes; // remove one cacheline from available size if we had to offset the first one
      buf = (char *)new_addr;
    }
  }

  static size_t get_next_aligned_addr(size_t addr) {
    if ((addr & (align_in_bytes-1)) != 0) {
      addr = ((addr>>6)<<6) + align_in_bytes;
    }
    return addr;
  }

  template<typename T>
  T *allocate(size_t block_size) {
    offset = get_next_aligned_addr(offset);
    assert(offset + sizeof(T)*block_size <= buf_size);
    T* ptr = (T*) new ((void *)&buf[offset]) T[block_size];
    offset += sizeof(T)*block_size;
    return ptr;
  }
  
  size_t getOffset() const {
    return offset;
  }

};


#endif // __HLD_ALLOC_H__
