// See LICENSE for license details.
/*
 * acc_single_io_types.h
 *
 *  Created on: Nov 8, 2016
 *      Author: aayupov
 */

#ifndef ACC_SINGLE_IO_TYPES_H_
#define ACC_SINGLE_IO_TYPES_H_



template<typename TAG>
struct Tag {
  TAG value;
};

template<typename T, typename UTAG>
struct AccRequest {
  UTAG utag;
  bool is_write;
  bool is_evict;
  CacheLineType::ByteIndexType word_pos;
  T data;
  AccRequest() : is_write(false), is_evict(false) {}
  AccRequest(UTAG utag, bool is_write, CacheLineType::ByteIndexType word_pos, T data) :
    utag(utag), is_write(is_write), is_evict(false), word_pos(word_pos), data(data) {}
  AccRequest(UTAG utag, bool is_write, CacheLineType::ByteIndexType word_pos) :
    utag(utag), is_write(is_write), is_evict(false), word_pos(word_pos), data(data) {
    assert(is_write == false);
  }

  inline friend void sc_trace(sc_trace_file* tf, const AccRequest& d,
      const std::string& name) {
  }

  inline friend std::ostream& operator<<(std::ostream& os,
      const AccRequest& d) {
    os << "utag: " << d.utag << std::endl;
    os << "is_write: " << d.is_write << std::endl;
    os << "is_evict: " << d.is_evict << std::endl;
    os << "word_pos: " << d.word_pos << std::endl;
    os << "data: " << d.data << std::endl;
    return os;
  }

  inline bool operator==(const AccRequest& rhs) {
    bool result = true;
    result = result && (utag == rhs.utag);
    result = result && (is_write == rhs.is_write);
    result = result && (is_evict == rhs.is_evict);
    result = result && (word_pos == rhs.word_pos);
    result = result && (data == rhs.data);
    return result;
  }
};


template <typename TAG, typename T, typename UTAG, size_t MSHR_SIZE, size_t REQ_Q_SIZE>
struct TagRecord {
  typedef enum {
    READ_PENDING,
    MODIFIED,
    WRITE_PENDING,
    UP_TO_DATE
  } RecordState;
  bool valid;
  bool has_pending_reqs;
  bool being_evicted;
  TAG tag;
  RecordState state;
  typename SizeT<MSHR_SIZE>::Type outstanding_req_q_index;
  CacheLineType cl;
};


template <typename TAG, typename T, typename UTAG, size_t MSHR_SIZE, size_t REQ_Q_SIZE>
struct AccInOutWay {
  typedef TagRecord<TAG, T, UTAG, MSHR_SIZE, REQ_Q_SIZE> TagRecordT;
  TagRecordT data;
};

template <size_t NUM_OF_WAYS, size_t NUM_OF_SETS>
struct CacheIndexWayPair {

  enum {
    //BYTE_INDEX_MAX_VALUE = 63 for cacheline width of 512bits
    WORD_WIDTH = Log2<CacheLineType::BYTE_INDEX_MAX_VALUE+1>::Value,
    TAG_INDEX_WIDTH = ADDRESS_TYPE_BIT_WIDTH - WORD_WIDTH,
    INDEX_WIDTH = Log2<NUM_OF_SETS>::Value,
    TAG_WIDTH = TAG_INDEX_WIDTH - INDEX_WIDTH
  };
  typedef typename SizeT<1<<INDEX_WIDTH>::Type IndexType;
  typedef typename SizeT<(size_t)1<<TAG_WIDTH>::Type TagType;
  typedef typename SizeT<1<<WORD_WIDTH>::Type WordType;
  typedef typename SizeT<NUM_OF_WAYS>::Type WayIndexType;

  static IndexType getIndex(AddressType addr) {
    return BIT_RANGE(addr,INDEX_WIDTH + WORD_WIDTH - 1, WORD_WIDTH);
  }

  static TagType getTag(AddressType addr) {
    return BIT_RANGE(addr,ADDRESS_TYPE_BIT_WIDTH - 1, INDEX_WIDTH + WORD_WIDTH);
  }

  static WordType getWordIndex(AddressType addr) {
    return BIT_RANGE(addr,WORD_WIDTH - 1, 0);
  }
  static SplAddressType computeSplAddress(IndexType index, TagType tag) {
    SplAddressType result = 0;
    BF_SET(result,tag, CacheLineType::BYTE_INDEX_MAX_VALUE, CacheLineType::BYTE_INDEX_MAX_VALUE+1-TAG_WIDTH);
    BF_SET(result,index, CacheLineType::BYTE_INDEX_MAX_VALUE-TAG_WIDTH, WORD_WIDTH);
    return result;
  }
  CacheIndexWayPair() : index(0), way(0) {}
  CacheIndexWayPair(IndexType index, WayIndexType way) :
    index (index), way(way) {}
  CacheIndexWayPair(const CacheIndexWayPair &rhs) :
    index(rhs.index), way(rhs.way) {}

  IndexType index;
  WayIndexType way;

  inline friend void sc_trace(sc_trace_file* tf, const CacheIndexWayPair& d,
      const std::string& name) {
  }

  inline friend std::ostream& operator<<(std::ostream& os,
      const CacheIndexWayPair& d) {
    os << "index: " << d.index << " way: " << d.way << std::endl;
    return os;
  }

  inline bool operator==(const CacheIndexWayPair& rhs) const {
    bool result = true;
    result = result && (index == rhs.index);
    result = result && (way == rhs.way);
    return result;
  }
};

template <size_t NUM_OF_WAYS, size_t NUM_OF_SETS, size_t MSHR_SIZE>
struct IndexWayMshrIndexTuple {
  CacheIndexWayPair<NUM_OF_WAYS, NUM_OF_SETS> index_way;
  typename SizeT<MSHR_SIZE>::Type mshr_index;
  IndexWayMshrIndexTuple() {}
  IndexWayMshrIndexTuple(CacheIndexWayPair<NUM_OF_WAYS, NUM_OF_SETS> index_way,
      typename SizeT<MSHR_SIZE>::Type mshr_index) :
        index_way(index_way), mshr_index(mshr_index) {}

  inline friend void sc_trace(sc_trace_file* tf,
      const IndexWayMshrIndexTuple& d, const std::string& name) {
  }

  inline friend std::ostream& operator<<(std::ostream& os,
      const IndexWayMshrIndexTuple& d) {
    os << "index: " << d.index_way << " mshr_index: " << d.mshr_index;
    return os;
  }

  inline bool operator==(const IndexWayMshrIndexTuple& rhs) const {
    bool result = true;
    result = result && (index_way == rhs.index_way);
    result = result && (mshr_index == rhs.mshr_index);
    return result;
  }
};

template <typename TAG, size_t NUM_OF_WAYS, typename T, typename UTAG, size_t MSHR_SIZE, size_t SAME_CL_REQ_SIZE>
struct AccInOutSet {
  typedef AccInOutWay<TAG, T, UTAG, MSHR_SIZE, SAME_CL_REQ_SIZE> AccInOutWayT;
  AccInOutWayT ways[NUM_OF_WAYS];
  typename SizeT<NUM_OF_WAYS>::Type evictSelector;

  bool findWayWithMatchingTag (TAG tag, typename SizeT<NUM_OF_WAYS>::Type &way_index, bool &being_evicted) {
    bool found = false;
    bool found_available = false;
    unsigned available_way_index;
    UNROLL_FIND_WAY: for (unsigned int i = 0; i < NUM_OF_WAYS; ++i) {
      if (ways[i].data.valid && ways[i].data.tag == tag) {
        way_index = i;
        found = true;
        being_evicted = ways[i].data.being_evicted;
      } else if (!ways[i].data.valid) {
        available_way_index = i;
        found_available = true;
      }
    }
    if (found) return true;
    else if (found_available) {
      way_index = available_way_index;
      return false;
    }
    else {
      //      way_index = evictSelector;
      //      evictSelector++;
      return false;
    }
  }

};

template <size_t NUM_OF_SETS, size_t NUM_OF_WAYS>
struct MshrRecord {
  typedef CacheIndexWayPair<NUM_OF_WAYS, NUM_OF_SETS> IndexWayPair;
  typename IndexWayPair::IndexType index;
  typename IndexWayPair::TagType tag;
  bool valid;

  MshrRecord() {}
  MshrRecord(typename IndexWayPair::IndexType index, typename IndexWayPair::TagType tag)
  : index(index), tag(tag), valid(true){}

  void reset() {
    valid = false;
  }
};

template <size_t NUM_OF_SETS, size_t NUM_OF_WAYS>
struct CacheLineWriteRequest {
  typedef CacheIndexWayPair<NUM_OF_WAYS, NUM_OF_SETS> IndexWayPair;
  typename IndexWayPair::IndexType index;
  typename IndexWayPair::TagType tag;
  typename IndexWayPair::WayIndexType way;
  CacheLineType cl;

  CacheLineWriteRequest() {}
  CacheLineWriteRequest(typename IndexWayPair::IndexType index, typename IndexWayPair::TagType tag, CacheLineType cl) :
    index(index), tag(tag), cl(cl) {}

};

enum AccReqType {
  ACC_REQ_READ = 0,
  ACC_REQ_WRITE = 1
};

#endif /* ACC_SINGLE_IO_TYPES_H_ */
