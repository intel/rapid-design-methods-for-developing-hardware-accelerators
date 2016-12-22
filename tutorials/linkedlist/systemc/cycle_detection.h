// See LICENSE for license details.
// See LICENSE for license details.
#ifndef __CYCLE_DETECTION_H__
#define __CYCLE_DETECTION_H__

#define ADDR_W 64
#define VAL_W 64
#define INFO_W 32

#include "types.h"
#include "Node.h"
#include "CycleExistInfo.h"
#include "Config.h"
/*struct Node {
  long long unsigned next_offset:ADDR_W;
  long long unsigned val:VAL_W;

  enum {
     BitCnt = 128 // number of bits taken by the struct in memory.
  };

  static size_t getBitCnt() { // returns the BitCnt
     assert(sizeof(Node)==(size_t)BitCnt /8); // <- very useful check. make sure you have it
     return BitCnt ;
  }

  static size_t numberOfFields() { // this is used to iterate over fields/array elements of your struct and calling put/getField. see below
     return 2;
  }

  static size_t fieldWidth(size_t index) { // this is used to iterate over fields/array elements of your struct and calling put/getField. see below
    switch(index) {
      case 0:
       return 64;
      case 1:
       return 64;
      default:
       assert(0);
   }
   return 0;
  }
  void putField(size_t index, UInt64 d) { //while packing your objects into a cacheline we call this method while iterating over all fields (see numberOfFields method)
    switch(index) {
      case 0: {
        next_offset = d;
        break;
      }
      case 1: {
        val = d;
        break;
      }
      default:
        assert(0);
      }
    }

  UInt64 getField(size_t index) const {//while parsing a cacheline and extracting objects of your type, we call this method while iterating over all fields (see numberOfFields method)
    switch(index) {
      case 0:
        return next_offset ;
      case 1:
        return val ;
      default:
        assert(0);
    }
    return 0;
  }

  inline friend void sc_trace(sc_trace_file* tf, const Node& d,
      const std::string& name) {
  }

  inline friend std::ostream& operator<<(std::ostream& os, const Node& d) {
    os << "next_offset: " << d.next_offset << " val: " << d.val << std::endl;
    return os;
  }

  inline bool operator==(const Node& rhs) const {
    bool result = true;
    result = result && (next_offset == rhs.next_offset);
    result = result && (val == rhs.val);
    return result;
  }
};

struct Config {
  long long unsigned head_offset:ADDR_W;
  long long unsigned output_offset:ADDR_W;

  inline friend void sc_trace(sc_trace_file* tf, const Config& d,
      const std::string& name) {
  }

  inline friend std::ostream& operator<<(std::ostream& os, const Config& d) {
    os << "head_offset: " << d.head_offset << std::endl;
    os << "output_offset: " << d.output_offset << std::endl;
    return os;
  }

  inline bool operator==(const Config& rhs) const {
    bool result = true;
    result = result && (head_offset == rhs.head_offset);
    result = result && (output_offset == rhs.output_offset);
    return result;
  }
};

struct CycleExistInfo {
  unsigned info:INFO_W;
  
  enum {
     BitCnt = 32 // number of bits taken by the struct in memory.
  };
  
  CycleExistInfo () {}
  
  CycleExistInfo (bool exist) {
    info = (exist)? 1 : 0;
  }

  static size_t getBitCnt() { // returns the BitCnt
     assert(sizeof(CycleExistInfo)==(size_t)BitCnt /8); // <- very useful check. make sure you have it
     return BitCnt ;
  }

  static size_t numberOfFields() { // this is used to iterate over fields/array elements of your struct and calling put/getField. see below
     return 1;
  }

  static size_t fieldWidth(size_t index) { // this is used to iterate over fields/array elements of your struct and calling put/getField. see below
    switch(index) {
      case 0:
        return 32;
      default:
       assert(0);
    }
    return 0;
  }
  
  void putField(size_t index, UInt64 d) { //while packing your objects into a cacheline we call this method while iterating over all fields (see numberOfFields method)
    switch(index) {
      case 0: {
        info = d;
        break;
      }
      default:
        assert(0);
      }
    }
  UInt64 getField(size_t index) const {//while parsing a cacheline and extracting objects of your type, we call this method while iterating over all fields (see numberOfFields method)
    switch(index) {
      case 0:
        return info ;
      default:
        assert(0);
    }
    return 0;
  }
   
  inline friend std::ostream& operator<<(std::ostream& os, const CycleExistInfo& d) {
    os << "Cycle Exists: " << d.info << std::endl;
    return os;
  }  
        
  inline bool operator==(const CycleExistInfo& rhs) const {
    bool result = true;
    result = result && (info == rhs.info);
    return result;
  }


};



*/
typedef struct node {
  struct node *next;
  int64_t val;
} listnode_t;


#endif //__CYCLE_DETECTION_H__
