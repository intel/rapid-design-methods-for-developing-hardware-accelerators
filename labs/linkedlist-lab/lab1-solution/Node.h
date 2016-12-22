// See LICENSE for license details.

//ty=Node
#ifndef Node_H_
#define Node_H_

#ifndef __SYNTHESIS__
#include <cstddef>
#include <cassert>
#endif

class Node {
public:
  unsigned long long next_offset;
  unsigned long long val;
};

#endif
