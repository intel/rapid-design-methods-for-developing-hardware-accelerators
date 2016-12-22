// See LICENSE for license details.
#ifndef __CONFIG_H__
#define __CONFIG_H__

class Node {
public:
  Node *next;
  unsigned long long val;

  Node *get_next() const {
    return next;
  }

  void set_next( Node *val) {
    next = val;
  }

};


class HeadPtr {
public:
  Node *head;

  Node *get_head() const {
    return head;
  }

  void set_head( Node *val) {
    head = val;
  }

  bool get_found() const {
    unsigned long long bits = reinterpret_cast<unsigned long long>( head);
    return bits & 0x1ULL;
  }
  void set_found( bool val) {
    unsigned long long bits = reinterpret_cast<unsigned long long>( head);
    if ( val) {
      head = reinterpret_cast<Node *>( bits |  0x1ULL);
    } else {
      head = reinterpret_cast<Node *>( bits & ~0x1ULL);
    }
  }
};

typedef unsigned long long AddrType;

class Config {
public:
  AddrType aInp;
  AddrType aOut;

  unsigned long long m;

  AddrType get_aInp() const {
    return aInp;
  }
  AddrType get_aOut() const {
    return aOut;
  }
  unsigned long long get_m() const {
    return m;
  }
  void set_aInp( AddrType val) {
    aInp = val;
  }
  void set_aOut( AddrType val) {
    aOut = val;
  }
  void set_m( unsigned long long val) {
    m = val;
  }

  HeadPtr *getInpPtr() const {
    return reinterpret_cast<HeadPtr*>( aInp);
  }
  HeadPtr *getOutPtr() const {
    return reinterpret_cast<HeadPtr*>( aOut);
  }

  Config() {
    aInp = aOut = m = 0;
  }

};

#endif //__CONFIG_H__
