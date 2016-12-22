// See LICENSE for license details.
#include "systemc.h" 
#include "gtest/gtest.h"
#include "accio_memread_test.h"
#include "accio_memwrite_test.h"
#include "accio_typed_memread_test.h"
#include "accio_typed_memwrite_test.h"
#include "accio_arbiter_test.h"
#include "command_queue_handler.h"
#include "acc_single_io_test.h"
#include "accio_simple_arbiter_test.h"
#include <set>
//TEST(MyTest, Test) {
//  EXPECT_EQ(1, 0);
//}
struct MyAccType {
  UInt64 data;
  MyAccType() {}
  MyAccType(int d) : data(d) {}
  MyAccType(UInt64 d) : data(d) {}

  enum {
    BitCnt = 64
  };

  static size_t getBitCnt() {
    return BitCnt;
  }

  static size_t numberOfFields() {
    return 1;
  }

  static size_t fieldWidth(size_t index) {
    switch(index) {
    case 0:
      return 64;
    default:
      assert(0);
    }
    return 0;
  }
  void putField(size_t index, UInt64 d) {
    switch(index) {
    case 0:
      data = d;
      break;
    default:
      assert(0);
    }
  }

  UInt64 getField(size_t index) const {
    switch(index) {
    case 0:
      return data;
    default:
      assert(0);
    }
    return 0;
  }


  inline friend void sc_trace(sc_trace_file* tf, const MyAccType& d,
      const std::string& name) {
  }

  inline friend std::ostream& operator<<(std::ostream& os, const MyAccType& d) {
    os << "data: " << d.data << std::endl;
    return os;
  }

  inline bool operator==(const MyAccType& rhs) const {
    bool result = true;
    result = result && (data == rhs.data);
    return result;
  }
};

TestMemReadTop* test_read_top = new TestMemReadTop("read_top");

TestMemWriteTop* test_write_top = new TestMemWriteTop("write_top");
TypedTestMemReadTop<MyAccType>* test64_read_top = new TypedTestMemReadTop<MyAccType>("read64_top");
TypedTestMemWriteTop<MyAccType>* test64_write_top = new TypedTestMemWriteTop<MyAccType>("write64_top");
AccTbTop* test_arbiter_top = new AccTbTop("arbiter_top");
TestSingleIOTop<MyAccType, UInt16>* single_io_top = new TestSingleIOTop<MyAccType, UInt16>("single_io_top");

AccArbTbTop *simple_arb_test = new AccArbTbTop("AccArbTbTop");
AccArbSimpleTbTop *notag_arb_test = new AccArbSimpleTbTop("AccArbSimpleTbTop");
//TestMemWriteTop* test_write_top = NULL;
//TypedTestMemReadTop<MyAccType>* test64_read_top = NULL;
//TypedTestMemWriteTop<MyAccType>* test64_write_top = NULL;
//TestMemArbiterTop* test_arbiter_top = NULL;


//CommandQueueHandler<1,1, MyAccType,MyAccType> *cqh = new CommandQueueHandler<1,1, MyAccType,MyAccType> ("CMDQHandler");


void setup_dram(std::array<CacheLineType, tb_params::DRAM_SIZE_IN_CLS> &dram) {
  dram[0]=CacheLineType(0,1,2,3,4,5,6,7);
  dram[1]=CacheLineType(1,1,1,1,1,1,1,1);
  dram[2]=CacheLineType(2,2,2,2,2,2,2,2);
  dram[3]=CacheLineType(10,10,10,10,10,10,10,10);
  dram[4]=CacheLineType(1,3,5,7,9,11,13,15);
  dram[8]=CacheLineType(9,9,9,9,9,9,9,9);
  dram[12]=CacheLineType(11,12,13,14,15,16,17,18);
  dram[16]=CacheLineType(100,101,102,103,104,105,106,107);
  // real array is much bigger but the data is undefined or rather zeros

}
template <typename T>
void setup_queue(std::queue<T> &q) {
  std::queue<T> empty;
  std::swap( q, empty );
}

TEST(TestMemRead, TestRead) {
  setup_dram(test_read_top->dram);
  setup_queue(test_read_top->test_in.stream);
  setup_queue(test_read_top->test_out.stream);
  // set up input
  test_read_top->test_in.stream.push(AccMemReadReqType(0, 1));
  test_read_top->test_in.stream.push(AccMemReadReqType(1*64, 2));
  // start reset
  test_read_top->reset();
  sc_start();
  // start main execution
  test_read_top->execute();
  sc_start();
  // return from main execution. do checks
  ASSERT_TRUE(test_read_top->test_in.stream.empty());
  //check output
  ASSERT_FALSE(test_read_top->test_out.stream.empty());
  ASSERT_EQ(test_read_top->test_out.stream.front(),AccMemReadRespType(CacheLineType(0,1,2,3,4,5,6,7)));
  test_read_top->test_out.stream.pop();
  ASSERT_FALSE(test_read_top->test_out.stream.empty());
  ASSERT_EQ(test_read_top->test_out.stream.front(),AccMemReadRespType(CacheLineType(1,1,1,1,1,1,1,1)));
  test_read_top->test_out.stream.pop();
  ASSERT_FALSE(test_read_top->test_out.stream.empty());
  ASSERT_EQ(test_read_top->test_out.stream.front(),AccMemReadRespType(CacheLineType(2,2,2,2,2,2,2,2)));
  test_read_top->test_out.stream.pop();
  // should be no extra output
  ASSERT_TRUE(test_read_top->test_out.stream.empty());
}

TEST(TestMemRead, TestRead1) {
  setup_dram(test_read_top->dram);
  setup_queue(test_read_top->test_in.stream);
  setup_queue(test_read_top->test_out.stream);

  // set up input
  test_read_top->test_in.stream.push(AccMemReadReqType(3*64, 1));

  // start reset
  test_read_top->reset();
  sc_start();
  // start main execution
  test_read_top->execute();
  sc_start();
  // return from main execution. do checks
  ASSERT_TRUE(test_read_top->test_in.stream.empty());
  ASSERT_FALSE(test_read_top->test_out.stream.empty());
  ASSERT_EQ(test_read_top->test_out.stream.front(),AccMemReadRespType(CacheLineType(10,10,10,10,10,10,10,10)));
  test_read_top->test_out.stream.pop();
  // should be no extra output
  ASSERT_TRUE(test_read_top->test_out.stream.empty()) << "should be no extra output";
}

TEST(TestMemWrite, TestWrite1) {
  setup_dram(test_write_top->dram);
  setup_queue(test_write_top->test_in.stream_req);
  setup_queue(test_write_top->test_in.stream_data);
  // set up input commands
  test_write_top->test_in.stream_req.push(AccMemWriteReqType(0, 2));
  test_write_top->test_in.stream_data.push(AccMemWriteDataType(CacheLineType(10,10,10,10,10,10,10,10), 0, 32));
  test_write_top->test_in.stream_data.push(AccMemWriteDataType(CacheLineType(9,10,10,10,10,10,10,9)));
  // start reset
  test_write_top->reset();
  sc_start();
  // start main execution
  test_write_top->execute();
  sc_start();
  // return from main execution. do checks
  ASSERT_TRUE(test_write_top->test_in.stream_req.empty());
  ASSERT_TRUE(test_write_top->test_in.stream_data.empty());
  ASSERT_EQ(test_write_top->dram[0],CacheLineType(10,10,10,10,4,5,6,7));
  ASSERT_EQ(test_write_top->dram[1],CacheLineType(9,10,10,10,10,10,10,9));

  //sc_stop();
}

TEST(BitRange, BitRangeTest32) {
  //01010110010011100111011000111111
  //^       ^       ^       ^      ^
  //31      23      15      7      0
  unsigned int A = 1447982655;
  sc_uint<32> scA = 1447982655;
  //100011
  unsigned int B = 35;
  EXPECT_EQ(B, BIT_RANGE(A, 9, 4));
  EXPECT_EQ(B, scA.range(9, 4));

  //10101100
  unsigned int C = 172;
  EXPECT_EQ(C, BIT_RANGE(A, 30, 23));
  EXPECT_EQ(C, scA.range(30, 23));

  //00111111
  unsigned int D = 63;
  EXPECT_EQ(D, BIT_RANGE(A, 7, 0));
  EXPECT_EQ(D, scA.range(7, 0));

  //EXPECT_EQ(BIT_RANGE(A, 31, 0), scA.range(31,0));

}

TEST(BitRange, BitRangeTest64) {
  //10111101111010110010011100111011000111111
  // ^       ^       ^       ^       ^      ^
  // 39      31      23      15      7      0
  unsigned long long  A = 1631388071487;
  sc_uint<64> scA = 1631388071487;
  //100011
  unsigned int B = 35;
  EXPECT_EQ(B, BIT_RANGE(A, 9, 4));
  EXPECT_EQ(B, scA.range(9, 4));

  //10101100
  unsigned int C = 172;
  EXPECT_EQ(C, BIT_RANGE(A, 30, 23));
  EXPECT_EQ(C, scA.range(30, 23));

  //00111111
  unsigned int D = 63;
  EXPECT_EQ(D, BIT_RANGE(A, 7, 0));
  EXPECT_EQ(D, BF_GET(A, 7, 0));
  EXPECT_EQ(D, scA.range(7, 0));

  UInt9 msb = 127;
  EXPECT_EQ(msb - 64, 63);
  EXPECT_EQ(A, BIT_RANGE(A, msb-64, 0));
  EXPECT_EQ(D, scA.range(7, 0));


  //11011110
  unsigned long long E = 222;
  EXPECT_EQ(E, BIT_RANGE(A, (unsigned long long)36, (unsigned long long)29));
  EXPECT_EQ(E, BF_GET(A, 36, 29));
  EXPECT_EQ(E, scA.range(36, 29));
  IOUnitIdType id = 64;
  id = BF_GET(id, 7, 7 - 1);
  EXPECT_EQ(1, id);

  //10111101111010110010011100111011000111111
  //    11011110
  //10111111111110110010011100111011000111111
  UInt64 F = 1649104811583;
  EXPECT_EQ(F, BF_SET(A, 255, 36, 29));
  //10111101111010110010011100111011000111111
  //    00000001
  //10110000000110110010011100111011000111111
  unsigned long long  G = 1512739599935;
  BF_SET(A, 1, 36, 29);
  EXPECT_EQ(G, A);
}

TEST(Test64MemRead, Test64Read1) {
  setup_dram(test64_read_top->dram);
  setup_queue(test64_read_top->test_in.stream);
  setup_queue(test64_read_top->test_out.stream);

  // set up input
  test64_read_top->test_in.stream.push(MemTypedReadReqType<MyAccType>(32, 6));
  test64_read_top->test_in.stream.push(MemTypedReadReqType<MyAccType>(0*64, 4));
  test64_read_top->test_in.stream.push(MemTypedReadReqType<MyAccType>(2*64, 10));
  test64_read_top->test_in.stream.push(MemTypedReadReqType<MyAccType>(3*64, 8));
  test64_read_top->test_in.stream.push(MemTypedReadReqType<MyAccType>(1*64, 2));

  // start reset
  test64_read_top->reset();
  sc_start();
  // start main execution
  test64_read_top->execute();
  sc_start();
  // return from main execution. do checks
  ASSERT_TRUE(test64_read_top->test_in.stream.empty());
  ASSERT_FALSE(test64_read_top->test_out.stream.empty());


  ASSERT_EQ(test64_read_top->test_out.stream.front(),MemTypedReadRespType<MyAccType>(4));
  test64_read_top->test_out.stream.pop();
  ASSERT_FALSE(test64_read_top->test_out.stream.empty());
  ASSERT_EQ(test64_read_top->test_out.stream.front(),MemTypedReadRespType<MyAccType>(5));
  test64_read_top->test_out.stream.pop();
  ASSERT_FALSE(test64_read_top->test_out.stream.empty());
  ASSERT_EQ(test64_read_top->test_out.stream.front(),MemTypedReadRespType<MyAccType>(6));
  test64_read_top->test_out.stream.pop();
  ASSERT_FALSE(test64_read_top->test_out.stream.empty());
  ASSERT_EQ(test64_read_top->test_out.stream.front(),MemTypedReadRespType<MyAccType>(7));
  test64_read_top->test_out.stream.pop();
  ASSERT_FALSE(test64_read_top->test_out.stream.empty());
  ASSERT_EQ(test64_read_top->test_out.stream.front(),MemTypedReadRespType<MyAccType>(1));
  test64_read_top->test_out.stream.pop();
  ASSERT_FALSE(test64_read_top->test_out.stream.empty());
  ASSERT_EQ(test64_read_top->test_out.stream.front(),MemTypedReadRespType<MyAccType>(1));
  test64_read_top->test_out.stream.pop();
  ASSERT_FALSE(test64_read_top->test_out.stream.empty());


  ASSERT_EQ(test64_read_top->test_out.stream.front(),MemTypedReadRespType<MyAccType>(0));
  test64_read_top->test_out.stream.pop();
  ASSERT_FALSE(test64_read_top->test_out.stream.empty());

  ASSERT_EQ(test64_read_top->test_out.stream.front(),MemTypedReadRespType<MyAccType>(1));
  test64_read_top->test_out.stream.pop();
  ASSERT_FALSE(test64_read_top->test_out.stream.empty());

  ASSERT_EQ(test64_read_top->test_out.stream.front(),MemTypedReadRespType<MyAccType>(2));
  test64_read_top->test_out.stream.pop();
  ASSERT_FALSE(test64_read_top->test_out.stream.empty());

  ASSERT_EQ(test64_read_top->test_out.stream.front(),MemTypedReadRespType<MyAccType>(3));
  test64_read_top->test_out.stream.pop();
  ASSERT_FALSE(test64_read_top->test_out.stream.empty());


  for (int i = 0; i < 8; ++i) {
    ASSERT_EQ(test64_read_top->test_out.stream.front(),MemTypedReadRespType<MyAccType>(2));
    test64_read_top->test_out.stream.pop();
    ASSERT_FALSE(test64_read_top->test_out.stream.empty());
  }

  ASSERT_EQ(test64_read_top->test_out.stream.front(),MemTypedReadRespType<MyAccType>(10));
  test64_read_top->test_out.stream.pop();
  ASSERT_FALSE(test64_read_top->test_out.stream.empty());

  ASSERT_EQ(test64_read_top->test_out.stream.front(),MemTypedReadRespType<MyAccType>(10));
  test64_read_top->test_out.stream.pop();

  for (int i = 0; i < 8; ++i) {
    ASSERT_FALSE(test64_read_top->test_out.stream.empty());
    ASSERT_EQ(test64_read_top->test_out.stream.front(),MemTypedReadRespType<MyAccType>(10));
    test64_read_top->test_out.stream.pop();
  }

  for (int i = 0; i < 2; ++i) {
    ASSERT_FALSE(test64_read_top->test_out.stream.empty());
    ASSERT_EQ(test64_read_top->test_out.stream.front(),MemTypedReadRespType<MyAccType>(1));
    test64_read_top->test_out.stream.pop();
  }

  // should be no extra output
  ASSERT_TRUE(test64_read_top->test_out.stream.empty()) << "should be no extra output";

  //sc_stop();
}

TEST(Test64MemWrite, Test64Write1) {
  setup_dram(test64_write_top->dram);
  setup_queue(test64_write_top->test_in.stream_req);
  setup_queue(test64_write_top->test_in.stream_data);

  // set up input
  test64_write_top->test_in.stream_req.push(MemTypedWriteReqType<MyAccType>(0*64, 10));
  test64_write_top->test_in.stream_data.push(MemTypedWriteDataType<MyAccType>(11));
  test64_write_top->test_in.stream_data.push(MemTypedWriteDataType<MyAccType>(12));
  test64_write_top->test_in.stream_data.push(MemTypedWriteDataType<MyAccType>(1));
  test64_write_top->test_in.stream_data.push(MemTypedWriteDataType<MyAccType>(2));
  test64_write_top->test_in.stream_data.push(MemTypedWriteDataType<MyAccType>(4));
  test64_write_top->test_in.stream_data.push(MemTypedWriteDataType<MyAccType>(5));
  test64_write_top->test_in.stream_data.push(MemTypedWriteDataType<MyAccType>(1));
  test64_write_top->test_in.stream_data.push(MemTypedWriteDataType<MyAccType>(2));
  test64_write_top->test_in.stream_data.push(MemTypedWriteDataType<MyAccType>(7));
  test64_write_top->test_in.stream_data.push(MemTypedWriteDataType<MyAccType>(8));

  // start reset
  test64_write_top->reset();
  sc_start();
  // start main execution
  test64_write_top->execute();
  sc_start();
  // return from main execution. do checks
  ASSERT_TRUE(test64_write_top->test_in.stream_req.empty());
  ASSERT_TRUE(test64_write_top->test_in.stream_data.empty());

  // the entire line is overwritten, so zeroes in the end if just 4 our of 8 words were writted
  ASSERT_EQ(CacheLineType(11, 12, 1, 2, 4, 5, 1, 2), test64_write_top->dram[0]);
  ASSERT_EQ(CacheLineType(7, 8, 0, 0, 0, 0, 0, 0), test64_write_top->dram[1]);
}
template <typename T, size_t SIZE>
void CHECK_DATA_QUEUE(std::queue<T> &q, const T (&golden)[SIZE]) {
  for (size_t i = 0; i < SIZE; ++i) {
    ASSERT_FALSE(q.empty());
    ASSERT_EQ(golden[i], q.front());
    q.pop();
  }
}

TEST(TestArbiter, Test2Rd1Wr) {
  setup_dram(test_arbiter_top->dram);
  setup_queue(test_arbiter_top->test_in.rd_req1);
  setup_queue(test_arbiter_top->test_in.rd_req2);
  setup_queue(test_arbiter_top->test_in.wr_data1);
  setup_queue(test_arbiter_top->test_in.wr_req1);
  setup_queue(test_arbiter_top->test_out.rd_resp1);
  setup_queue(test_arbiter_top->test_out.rd_resp2);

  // set up input
  test_arbiter_top->test_in.rd_req1.push(MemTypedReadReqType<AccDataType>(0*64, 4));
  test_arbiter_top->test_in.rd_req2.push(MemTypedReadReqType<AccDataType>(2*64, 4));
  test_arbiter_top->test_in.rd_req1.push(MemTypedReadReqType<AccDataType>(3*64, 4));
  test_arbiter_top->test_in.rd_req2.push(MemTypedReadReqType<AccDataType>(1*64, 4));

  test_arbiter_top->test_in.wr_req1.push(MemTypedWriteReqType<AccDataType>(4*64, 2));
  test_arbiter_top->test_in.wr_data1.push(MemTypedWriteDataType<AccDataType>(125));
  test_arbiter_top->test_in.wr_data1.push(MemTypedWriteDataType<AccDataType>(250));

  // start reset
  test_arbiter_top->reset();
  sc_start();
  // start main execution
  test_arbiter_top->execute();
  sc_start();
  // return from main execution. do checks
  ASSERT_TRUE(test_arbiter_top->test_in.rd_req1.empty());
  ASSERT_TRUE(test_arbiter_top->test_in.rd_req2.empty());
  ASSERT_TRUE(test_arbiter_top->test_in.wr_req1.empty());
  ASSERT_TRUE(test_arbiter_top->test_in.wr_data1.empty());

  //dram[0]=CacheLineType(0,1,2,3,4,5,6,7);
  //dram[1]=CacheLineType(1,1,1,1,1,1,1,1);
  //dram[2]=CacheLineType(2,2,2,2,2,2,2,2);
  //dram[3]=CacheLineType(10,10,10,10,10,10,10,10);
  CHECK_DATA_QUEUE<MemTypedReadRespType<AccDataType>, 4>(test_arbiter_top->test_out.rd_resp1, {AccDataType(0), AccDataType(1), AccDataType(2), AccDataType(3)});
  CHECK_DATA_QUEUE<MemTypedReadRespType<AccDataType>, 4>(test_arbiter_top->test_out.rd_resp1, {AccDataType(10), AccDataType(10), AccDataType(10), AccDataType(10)});
  CHECK_DATA_QUEUE<MemTypedReadRespType<AccDataType>, 4>(test_arbiter_top->test_out.rd_resp2, {AccDataType(2), AccDataType(2), AccDataType(2), AccDataType(2)});
  CHECK_DATA_QUEUE<MemTypedReadRespType<AccDataType>, 4>(test_arbiter_top->test_out.rd_resp2, {AccDataType(1), AccDataType(1), AccDataType(1), AccDataType(1)});

  ASSERT_TRUE(test_arbiter_top->test_out.rd_resp1.empty());
  ASSERT_TRUE(test_arbiter_top->test_out.rd_resp2.empty());

  ASSERT_EQ(test_arbiter_top->dram[4], CacheLineType(125, 250, 5, 7, 9, 11, 13, 15));
}


TEST(TestAccInOutCache, WriteRead) {
  setup_dram(single_io_top->dram);
  setup_queue(single_io_top->test_in.stream_rd_req);
  setup_queue(single_io_top->test_in.stream_wr_req);
  setup_queue(single_io_top->test_in.stream_order);
  setup_queue(single_io_top->test_out.stream_rd_resp);

//  dram[0]=CacheLineType(0,1,2,3,4,5,6,7);
//  dram[1]=CacheLineType(1,1,1,1,1,1,1,1);
//  dram[2]=CacheLineType(2,2,2,2,2,2,2,2);
//  dram[3]=CacheLineType(10,10,10,10,10,10,10,10);
//  dram[4]=CacheLineType(1,3,5,7,9,11,13,15);

//  struct TbInType {
//    std::queue<MemSingleWriteReqType<T, UTAG> > stream_wr_req;
//    std::queue<MemSingleReadReqType<T, UTAG> > stream_rd_req;
//    std::queue<ReqOrderEnum> stream_order;
//  };
//  struct TbOutType {
//    std::queue<MemSingleReadRespType<T, UTAG> > stream_rd_resp;
//  };

  AccSingleIOTestbench<MyAccType, UInt16>::ReqOrderEnum READ_CMD = AccSingleIOTestbench<MyAccType, UInt16>::RO_READ;
  AccSingleIOTestbench<MyAccType, UInt16>::ReqOrderEnum WRITE_CMD = AccSingleIOTestbench<MyAccType, UInt16>::RO_WRITE;

  size_t MAX_ITER_TAG = 5 ;
  size_t i = 0;
    // set up input
    single_io_top->test_in.stream_wr_req.push(MemSingleWriteReqType<MyAccType, UInt16>(32, (i*MAX_ITER_TAG)+1,0));
    single_io_top->test_in.stream_order.push(WRITE_CMD);
    single_io_top->test_in.stream_wr_req.push(MemSingleWriteReqType<MyAccType, UInt16>(0*64, (i*MAX_ITER_TAG)+2,1));
    single_io_top->test_in.stream_order.push(WRITE_CMD);
    single_io_top->test_in.stream_wr_req.push(MemSingleWriteReqType<MyAccType, UInt16>(2*64, (i*MAX_ITER_TAG)+3,0));
    single_io_top->test_in.stream_order.push(WRITE_CMD);
    single_io_top->test_in.stream_wr_req.push(MemSingleWriteReqType<MyAccType, UInt16>(3*64, (i*MAX_ITER_TAG)+4,0));
    single_io_top->test_in.stream_order.push(WRITE_CMD);
    single_io_top->test_in.stream_wr_req.push(MemSingleWriteReqType<MyAccType, UInt16>(1*64, (i*MAX_ITER_TAG)+5,0));
    single_io_top->test_in.stream_order.push(WRITE_CMD);

    single_io_top->test_in.stream_rd_req.push(MemSingleReadReqType<MyAccType, UInt16>(32, (i*MAX_ITER_TAG)+1));
    single_io_top->test_in.stream_order.push(READ_CMD);
    single_io_top->test_in.stream_rd_req.push(MemSingleReadReqType<MyAccType, UInt16>(0*64, (i*MAX_ITER_TAG)+2));
    single_io_top->test_in.stream_order.push(READ_CMD);
    single_io_top->test_in.stream_rd_req.push(MemSingleReadReqType<MyAccType, UInt16>(2*64, (i*MAX_ITER_TAG)+3));
    single_io_top->test_in.stream_order.push(READ_CMD);
    single_io_top->test_in.stream_rd_req.push(MemSingleReadReqType<MyAccType, UInt16>(3*64, (i*MAX_ITER_TAG)+4));
    single_io_top->test_in.stream_order.push(READ_CMD);
    single_io_top->test_in.stream_rd_req.push(MemSingleReadReqType<MyAccType, UInt16>(1*64, (i*MAX_ITER_TAG)+5));
    single_io_top->test_in.stream_order.push(READ_CMD);

  // start reset
  single_io_top->reset();
  sc_start();
  // start main execution
  single_io_top->execute();
  sc_start();

  // return from main execution. do checks
  ASSERT_TRUE(single_io_top->test_in.stream_order.empty());
  ASSERT_FALSE(single_io_top->test_out.stream_rd_resp.empty());

  size_t num_of_resp = single_io_top->test_out.stream_rd_resp.size();
  std::map<int, MyAccType> data_reads;
  std::map<int, MyAccType>::iterator data_read_it;
  //for (size_t i = 0; i < REPEAT; ++i) {
    for (size_t rq = 0; rq < MAX_ITER_TAG; ++rq) {
      MemSingleReadRespType<MyAccType, UInt16> read = single_io_top->test_out.stream_rd_resp.front();
      data_reads.insert(std::make_pair(read.utag, read.data));
      single_io_top->test_out.stream_rd_resp.pop();
      //cout << "READ tag: " << read.utag << " data: " << read.data << endl;
    }
  //}
  // should be no extra output
  ASSERT_TRUE(single_io_top->test_out.stream_rd_resp.empty()) << "should be no extra output, but there are : " << single_io_top->test_out.stream_rd_resp.size() << " more, when originally it was " << num_of_resp;

  //for (size_t i = 0; i < REPEAT; ++i) {
    data_read_it = data_reads.find((i*MAX_ITER_TAG)+1);
    ASSERT_FALSE(data_read_it == data_reads.end());
    ASSERT_EQ(MyAccType(0),data_read_it->second) << "iter " << i;
    data_reads.erase(data_read_it);

    data_read_it = data_reads.find((i*MAX_ITER_TAG)+2);
    ASSERT_FALSE(data_read_it == data_reads.end());
    ASSERT_EQ(MyAccType(1),data_read_it->second) << "iter " << i;
    data_reads.erase(data_read_it);

    data_read_it = data_reads.find((i*MAX_ITER_TAG)+3);
    ASSERT_FALSE(data_read_it == data_reads.end()) << "can't find " << (i*MAX_ITER_TAG)+3;
    ASSERT_EQ(MyAccType(0),data_read_it->second) << "iter " << i;
    data_reads.erase(data_read_it);

    data_read_it = data_reads.find((i*MAX_ITER_TAG)+4);
    ASSERT_FALSE(data_read_it == data_reads.end());
    ASSERT_EQ(MyAccType(0),data_read_it->second) << "iter " << i;
    data_reads.erase(data_read_it);

    data_read_it = data_reads.find((i*MAX_ITER_TAG)+5);
    ASSERT_FALSE(data_read_it == data_reads.end());
    ASSERT_EQ(MyAccType(0),data_read_it->second) << "iter " << i;
    data_reads.erase(data_read_it);
  //}
    //  dram[0]=CacheLineType(0,1,2,3,4,5,6,7);
    //  dram[1]=CacheLineType(1,1,1,1,1,1,1,1);
    //  dram[2]=CacheLineType(2,2,2,2,2,2,2,2);
    //  dram[3]=CacheLineType(10,10,10,10,10,10,10,10);
    //  dram[4]=CacheLineType(1,3,5,7,9,11,13,15);

    ASSERT_EQ(CacheLineType(1, 1, 2, 3, 0, 5, 6, 7), single_io_top->dram[0]);
    ASSERT_EQ(CacheLineType(0, 1,1,1,1,1,1,1), single_io_top->dram[1]);
    ASSERT_EQ(CacheLineType(0, 2,2,2,2,2,2,2), single_io_top->dram[2]);
    ASSERT_EQ(CacheLineType(0, 10,10,10,10,10,10,10), single_io_top->dram[3]);

  //sc_stop();
}

TEST(TestAccInOutCache, WriteReplace) {
  setup_dram(single_io_top->dram);
  setup_queue(single_io_top->test_in.stream_rd_req);
  setup_queue(single_io_top->test_in.stream_wr_req);
  setup_queue(single_io_top->test_in.stream_order);
  setup_queue(single_io_top->test_out.stream_rd_resp);

  AccSingleIOTestbench<MyAccType, UInt16>::ReqOrderEnum READ_CMD = AccSingleIOTestbench<MyAccType, UInt16>::RO_READ;
  AccSingleIOTestbench<MyAccType, UInt16>::ReqOrderEnum WRITE_CMD = AccSingleIOTestbench<MyAccType, UInt16>::RO_WRITE;

  size_t MAX_ITER_TAG = 5 ;
  size_t i = 0;
    // set up input
    single_io_top->test_in.stream_wr_req.push(MemSingleWriteReqType<MyAccType, UInt16>(0*64, (i*MAX_ITER_TAG)+2,50));
    single_io_top->test_in.stream_order.push(WRITE_CMD);
    single_io_top->test_in.stream_wr_req.push(MemSingleWriteReqType<MyAccType, UInt16>(4*64, (i*MAX_ITER_TAG)+3,51));
    single_io_top->test_in.stream_order.push(WRITE_CMD);
    single_io_top->test_in.stream_wr_req.push(MemSingleWriteReqType<MyAccType, UInt16>(8*64, (i*MAX_ITER_TAG)+4,52));
    single_io_top->test_in.stream_order.push(WRITE_CMD);
    single_io_top->test_in.stream_wr_req.push(MemSingleWriteReqType<MyAccType, UInt16>(12*64, (i*MAX_ITER_TAG)+5,53));
    single_io_top->test_in.stream_order.push(WRITE_CMD);
    single_io_top->test_in.stream_wr_req.push(MemSingleWriteReqType<MyAccType, UInt16>(16*64, (i*MAX_ITER_TAG)+5,54));
    single_io_top->test_in.stream_order.push(WRITE_CMD);

  // start reset
  single_io_top->reset();
  sc_start();
  // start main execution
  single_io_top->execute();
  sc_start();

  // return from main execution. do checks
  ASSERT_TRUE(single_io_top->test_in.stream_order.empty());
  ASSERT_TRUE(single_io_top->test_out.stream_rd_resp.empty());
  //for (size_t i = 0; i < REPEAT; ++i) {
  //  dram[0]=CacheLineType(0,1,2,3,4,5,6,7);
  //  dram[1]=CacheLineType(1,1,1,1,1,1,1,1);
  //  dram[2]=CacheLineType(2,2,2,2,2,2,2,2);
  //  dram[3]=CacheLineType(10,10,10,10,10,10,10,10);
  //  dram[4]=CacheLineType(1,3,5,7,9,11,13,15);
  //  dram[8]=CacheLineType(9,9,9,9,9,9,9,9);
  //  dram[12]=CacheLineType(11,12,13,14,15,16,17,18);
  //  dram[16]=CacheLineType(100,101,102,103,104,105,106,107);

    ASSERT_EQ(CacheLineType(50,1,2,3,4,5,6,7), single_io_top->dram[0]);
    ASSERT_EQ(CacheLineType(51,3,5,7,9,11,13,15), single_io_top->dram[4]);
    ASSERT_EQ(CacheLineType(52,9,9,9,9,9,9,9), single_io_top->dram[8]);
    ASSERT_EQ(CacheLineType(53,12,13,14,15,16,17,18), single_io_top->dram[12]);
    ASSERT_EQ(CacheLineType(54,101,102,103,104,105,106,107), single_io_top->dram[16]);
    single_io_top->acc_io_unit.cache.print_stats();

  //sc_stop();
}

TEST(TestAccInOutCache, RandomReadWrite) {
  setup_dram(single_io_top->dram);
  setup_queue(single_io_top->test_in.stream_rd_req);
  setup_queue(single_io_top->test_in.stream_wr_req);
  setup_queue(single_io_top->test_in.stream_order);
  setup_queue(single_io_top->test_out.stream_rd_resp);

  AccSingleIOTestbench<MyAccType, UInt16>::ReqOrderEnum READ_CMD = AccSingleIOTestbench<MyAccType, UInt16>::RO_READ;
  AccSingleIOTestbench<MyAccType, UInt16>::ReqOrderEnum WRITE_CMD = AccSingleIOTestbench<MyAccType, UInt16>::RO_WRITE;


  size_t DRAM_RANGE = 5*8;
  size_t NUM_TX = 1<<12;
  std::array<CacheLineType, tb_params::DRAM_SIZE_IN_CLS> expected_dram = single_io_top->dram;
  size_t seed = time(NULL);
  cout << "Seed = " << seed << endl;
  srand (seed);
  std::map<int, MyAccType> data_reads;
  std::map<int, MyAccType>::iterator data_read_it;
  size_t num_of_reads = 0, num_of_writes = 0;
  for (unsigned i = 0; i<NUM_TX; ++i) {
    size_t random_addr = 8*(rand()%DRAM_RANGE);
    AccSingleIOTestbench<MyAccType, UInt16>::ReqOrderEnum random_cmd = (rand()%2)?READ_CMD:WRITE_CMD;
    size_t random_val = rand()%100;
    if (random_cmd == READ_CMD) {
      single_io_top->test_in.stream_rd_req.push(MemSingleReadReqType<MyAccType, UInt16>(random_addr, i));
    }else {
      single_io_top->test_in.stream_wr_req.push(MemSingleWriteReqType<MyAccType, UInt16>(random_addr, i,random_val));
    }
    single_io_top->test_in.stream_order.push(random_cmd);

    size_t cl_addr = random_addr/64;
    size_t cl_word = (random_addr%64)/8;
    if (random_cmd == READ_CMD) {
      cout << "Cache Random Test: read cmd: tag=" << i << " address=" << random_addr << " expected data = " << std::hex << MyAccType((UInt64)expected_dram[cl_addr].words[cl_word]) << std::dec<<endl;
      data_reads.insert(std::make_pair(i, MyAccType((UInt64)expected_dram[cl_addr].words[cl_word])));
      num_of_reads++;
    } else {
      cout << "Cache Random Test: write cmd: tag=" << i << " address=" << random_addr << " value = " << std::hex << random_val << std::dec<<  endl;
      cout << "cl addr " << cl_addr << " cl word " << cl_word << endl;
      expected_dram[cl_addr].words[cl_word] = random_val;
      num_of_writes++;
    }

  }



  // start reset
  single_io_top->reset();
  sc_start();
  // start main execution
  single_io_top->execute();
  sc_start();

  // return from main execution. do checks
  ASSERT_TRUE(single_io_top->test_in.stream_order.empty());
  ASSERT_TRUE(single_io_top->test_out.stream_rd_resp.size() == num_of_reads) << " num of read reqs = " << num_of_reads << " num of rd responses = " << single_io_top->test_out.stream_rd_resp.size() << endl;

  for (size_t i = 0; i < num_of_reads; ++i) {
    MemSingleReadRespType<MyAccType, UInt16> read = single_io_top->test_out.stream_rd_resp.front();
    single_io_top->test_out.stream_rd_resp.pop();
    data_read_it = data_reads.find(read.utag);
    ASSERT_FALSE(data_read_it == data_reads.end());
    ASSERT_EQ(data_read_it->second,read.data) << "iter " << i << " tag = " << read.utag;
    data_reads.erase(data_read_it);

  }


  // dram check
  for (size_t i = 0; i < DRAM_RANGE/8; ++i) {
    ASSERT_EQ(expected_dram[i], single_io_top->dram[i]);
  }

  single_io_top->acc_io_unit.cache.print_stats();
  //sc_stop();
}


TEST(TestAccInOutCache, Read) {
  setup_dram(single_io_top->dram);
  setup_queue(single_io_top->test_in.stream_rd_req);
  setup_queue(single_io_top->test_in.stream_wr_req);
  setup_queue(single_io_top->test_in.stream_order);
  setup_queue(single_io_top->test_out.stream_rd_resp);

//  dram[0]=CacheLineType(0,1,2,3,4,5,6,7);
//  dram[1]=CacheLineType(1,1,1,1,1,1,1,1);
//  dram[2]=CacheLineType(2,2,2,2,2,2,2,2);
//  dram[3]=CacheLineType(10,10,10,10,10,10,10,10);
//  dram[4]=CacheLineType(1,3,5,7,9,11,13,15);

//  struct TbInType {
//    std::queue<MemSingleWriteReqType<T, UTAG> > stream_wr_req;
//    std::queue<MemSingleReadReqType<T, UTAG> > stream_rd_req;
//    std::queue<ReqOrderEnum> stream_order;
//  };
//  struct TbOutType {
//    std::queue<MemSingleReadRespType<T, UTAG> > stream_rd_resp;
//  };

  AccSingleIOTestbench<MyAccType, UInt16>::ReqOrderEnum READ_CMD = AccSingleIOTestbench<MyAccType, UInt16>::RO_READ;
  AccSingleIOTestbench<MyAccType, UInt16>::ReqOrderEnum WRITE_CMD = AccSingleIOTestbench<MyAccType, UInt16>::RO_WRITE;

  size_t REPEAT = 10 ;
  size_t MAX_ITER_TAG = 5 ;
  for (size_t i = 0; i < REPEAT; ++i) {
    // set up input
    single_io_top->test_in.stream_rd_req.push(MemSingleReadReqType<MyAccType, UInt16>(32, (i*MAX_ITER_TAG)+1));
    single_io_top->test_in.stream_order.push(READ_CMD);
    single_io_top->test_in.stream_rd_req.push(MemSingleReadReqType<MyAccType, UInt16>(0*64, (i*MAX_ITER_TAG)+2));
    single_io_top->test_in.stream_order.push(READ_CMD);
    single_io_top->test_in.stream_rd_req.push(MemSingleReadReqType<MyAccType, UInt16>(2*64, (i*MAX_ITER_TAG)+3));
    single_io_top->test_in.stream_order.push(READ_CMD);
    single_io_top->test_in.stream_rd_req.push(MemSingleReadReqType<MyAccType, UInt16>(3*64, (i*MAX_ITER_TAG)+4));
    single_io_top->test_in.stream_order.push(READ_CMD);
    single_io_top->test_in.stream_rd_req.push(MemSingleReadReqType<MyAccType, UInt16>(1*64, (i*MAX_ITER_TAG)+5));
    single_io_top->test_in.stream_order.push(READ_CMD);
  }
  // start reset
  single_io_top->reset();
  sc_start();
  // start main execution
  single_io_top->execute();
  sc_start();

  // return from main execution. do checks
  ASSERT_TRUE(single_io_top->test_in.stream_order.empty());
  ASSERT_FALSE(single_io_top->test_out.stream_rd_resp.empty());


  std::map<int, MyAccType> data_reads;
  std::map<int, MyAccType>::iterator data_read_it;
  for (size_t i = 0; i < REPEAT; ++i) {
    for (size_t rq = 0; rq < MAX_ITER_TAG; ++rq) {
      MemSingleReadRespType<MyAccType, UInt16> read = single_io_top->test_out.stream_rd_resp.front();
      data_reads.insert(std::make_pair(read.utag, read.data));
      single_io_top->test_out.stream_rd_resp.pop();
      //cout << "READ tag: " << read.utag << " data: " << read.data << endl;
    }
  }
  // should be no extra output
  ASSERT_TRUE(single_io_top->test_out.stream_rd_resp.empty()) << "should be no extra output, but there are : " << single_io_top->test_out.stream_rd_resp.size() << " more";

  for (size_t i = 0; i < REPEAT; ++i) {
    data_read_it = data_reads.find((i*MAX_ITER_TAG)+1);
    ASSERT_FALSE(data_read_it == data_reads.end());
    ASSERT_EQ(MyAccType(4),data_read_it->second) << "iter " << i;
    data_reads.erase(data_read_it);

    data_read_it = data_reads.find((i*MAX_ITER_TAG)+2);
    ASSERT_FALSE(data_read_it == data_reads.end());
    ASSERT_EQ(MyAccType(0),data_read_it->second) << "iter " << i;
    data_reads.erase(data_read_it);

    data_read_it = data_reads.find((i*MAX_ITER_TAG)+3);
    ASSERT_FALSE(data_read_it == data_reads.end()) << "can't find " << (i*MAX_ITER_TAG)+3;
    ASSERT_EQ(MyAccType(2),data_read_it->second) << "iter " << i;
    data_reads.erase(data_read_it);

    data_read_it = data_reads.find((i*MAX_ITER_TAG)+4);
    ASSERT_FALSE(data_read_it == data_reads.end());
    ASSERT_EQ(MyAccType(10),data_read_it->second) << "iter " << i;
    data_reads.erase(data_read_it);

    data_read_it = data_reads.find((i*MAX_ITER_TAG)+5);
    ASSERT_FALSE(data_read_it == data_reads.end());
    ASSERT_EQ(MyAccType(1),data_read_it->second) << "iter " << i;
    data_reads.erase(data_read_it);
  }

  //sc_stop();
}

TEST(TestSimpleArb, Test) {
  setup_queue(simple_arb_test->test_in.req1);
  setup_queue(simple_arb_test->test_in.req2);
  setup_queue(simple_arb_test->test_in.req3);
  setup_queue(simple_arb_test->test_in.req4);
  setup_queue(simple_arb_test->test_out.resp1);
  setup_queue(simple_arb_test->test_out.resp2);
  setup_queue(simple_arb_test->test_out.resp3);
  setup_queue(simple_arb_test->test_out.resp4);
  // set up input
  unsigned tag = 0;
  simple_arb_test->test_in.req1.push(SplMemReadReqType(0, 0, tag++));
  simple_arb_test->test_in.req2.push(SplMemReadReqType(1, 0, tag++));
  simple_arb_test->test_in.req3.push(SplMemReadReqType(2, 0, tag++));
  simple_arb_test->test_in.req4.push(SplMemReadReqType(3, 0, tag++));
  simple_arb_test->test_in.req1.push(SplMemReadReqType(5, 0, tag++));
  simple_arb_test->test_in.req1.push(SplMemReadReqType(2, 0, tag++));
  simple_arb_test->test_in.req4.push(SplMemReadReqType(4, 0, tag++));
  simple_arb_test->test_in.req4.push(SplMemReadReqType(5, 0, tag++));
  // start reset
  simple_arb_test->reset();
  sc_start();
  // start main execution
  simple_arb_test->execute();
  sc_start();
  // return from main execution. do checks
  ASSERT_TRUE(simple_arb_test->test_in.req1.empty());
  ASSERT_TRUE(simple_arb_test->test_in.req2.empty());
  ASSERT_TRUE(simple_arb_test->test_in.req3.empty());
  ASSERT_TRUE(simple_arb_test->test_in.req4.empty());
  //check output
  ASSERT_FALSE(simple_arb_test->test_out.resp1.empty());
  ASSERT_FALSE(simple_arb_test->test_out.resp2.empty());
  ASSERT_FALSE(simple_arb_test->test_out.resp3.empty());
  ASSERT_FALSE(simple_arb_test->test_out.resp4.empty());

  SplMemReadRespType resp = simple_arb_test->test_out.resp1.front();
  ASSERT_EQ(CacheLineType(0,0,0,0,0,0,0,0), resp.data);
  simple_arb_test->test_out.resp1.pop();
  resp = simple_arb_test->test_out.resp1.front();
  ASSERT_EQ(CacheLineType(0,0,0,0,0,0,0,10), resp.data);
  simple_arb_test->test_out.resp1.pop();
  resp = simple_arb_test->test_out.resp1.front();
  ASSERT_EQ(CacheLineType(0,0,0,0,0,0,0,4), resp.data);
  simple_arb_test->test_out.resp1.pop();

  resp = simple_arb_test->test_out.resp2.front();
  ASSERT_EQ(CacheLineType(0,0,0,0,0,0,0,2), resp.data);
  simple_arb_test->test_out.resp2.pop();
  resp = simple_arb_test->test_out.resp3.front();
  ASSERT_EQ(CacheLineType(0,0,0,0,0,0,0,4), resp.data);
  simple_arb_test->test_out.resp3.pop();

  resp = simple_arb_test->test_out.resp4.front();
  ASSERT_EQ(CacheLineType(0,0,0,0,0,0,0,6), resp.data);
  simple_arb_test->test_out.resp4.pop();
  resp = simple_arb_test->test_out.resp4.front();
  ASSERT_EQ(CacheLineType(0,0,0,0,0,0,0,8), resp.data);
  simple_arb_test->test_out.resp4.pop();
  resp = simple_arb_test->test_out.resp4.front();
  ASSERT_EQ(CacheLineType(0,0,0,0,0,0,0,10), resp.data);
  simple_arb_test->test_out.resp4.pop();


  // should be no extra output
  ASSERT_TRUE(simple_arb_test->test_out.resp1.empty());
  ASSERT_TRUE(simple_arb_test->test_out.resp2.empty());
  ASSERT_TRUE(simple_arb_test->test_out.resp3.empty());
  ASSERT_TRUE(simple_arb_test->test_out.resp4.empty());
}

TEST(TestNoTagArb, Test) {
  setup_queue(notag_arb_test->test_in.req1);
  setup_queue(notag_arb_test->test_in.req2);
  setup_queue(notag_arb_test->test_in.req3);
  setup_queue(notag_arb_test->test_in.req4);
  setup_queue(notag_arb_test->test_out.resp1);
  setup_queue(notag_arb_test->test_out.resp2);
  setup_queue(notag_arb_test->test_out.resp3);
  setup_queue(notag_arb_test->test_out.resp4);
  // set up input
  unsigned tag = 0;
  notag_arb_test->test_in.req1.push(SplMemReadReqType(0, 0<<5, tag++));
  notag_arb_test->test_in.req2.push(SplMemReadReqType(1, 1<<5, tag++));
  notag_arb_test->test_in.req3.push(SplMemReadReqType(2, 2<<5, tag++));
  notag_arb_test->test_in.req4.push(SplMemReadReqType(3, 3<<5, tag++));
  notag_arb_test->test_in.req1.push(SplMemReadReqType(5, 0<<5, tag++));
  notag_arb_test->test_in.req1.push(SplMemReadReqType(2, 0<<5, tag++));
  notag_arb_test->test_in.req4.push(SplMemReadReqType(4, 3<<5, tag++));
  notag_arb_test->test_in.req4.push(SplMemReadReqType(5, 3<<5, tag++));
  // start reset
  notag_arb_test->reset();
  sc_start();
  // start main execution
  notag_arb_test->execute();
  sc_start();
  // return from main execution. do checks
  ASSERT_TRUE(notag_arb_test->test_in.req1.empty());
  ASSERT_TRUE(notag_arb_test->test_in.req2.empty());
  ASSERT_TRUE(notag_arb_test->test_in.req3.empty());
  ASSERT_TRUE(notag_arb_test->test_in.req4.empty());
  //check output
  ASSERT_FALSE(notag_arb_test->test_out.resp1.empty());
  ASSERT_FALSE(notag_arb_test->test_out.resp2.empty());
  ASSERT_FALSE(notag_arb_test->test_out.resp3.empty());
  ASSERT_FALSE(notag_arb_test->test_out.resp4.empty());

  SplMemReadRespType resp = notag_arb_test->test_out.resp1.front();
  ASSERT_EQ(CacheLineType(0,0,0,0,0,0,0,0), resp.data);
  notag_arb_test->test_out.resp1.pop();
  resp = notag_arb_test->test_out.resp1.front();
  ASSERT_EQ(CacheLineType(0,0,0,0,0,0,0,10), resp.data);
  notag_arb_test->test_out.resp1.pop();
  resp = notag_arb_test->test_out.resp1.front();
  ASSERT_EQ(CacheLineType(0,0,0,0,0,0,0,4), resp.data);
  notag_arb_test->test_out.resp1.pop();

  resp = notag_arb_test->test_out.resp2.front();
  ASSERT_EQ(CacheLineType(0,0,0,0,0,0,0,2), resp.data);
  notag_arb_test->test_out.resp2.pop();
  resp = notag_arb_test->test_out.resp3.front();
  ASSERT_EQ(CacheLineType(0,0,0,0,0,0,0,4), resp.data);
  notag_arb_test->test_out.resp3.pop();

  resp = notag_arb_test->test_out.resp4.front();
  ASSERT_EQ(CacheLineType(0,0,0,0,0,0,0,6), resp.data);
  notag_arb_test->test_out.resp4.pop();
  resp = notag_arb_test->test_out.resp4.front();
  ASSERT_EQ(CacheLineType(0,0,0,0,0,0,0,8), resp.data);
  notag_arb_test->test_out.resp4.pop();
  resp = notag_arb_test->test_out.resp4.front();
  ASSERT_EQ(CacheLineType(0,0,0,0,0,0,0,10), resp.data);
  notag_arb_test->test_out.resp4.pop();


  // should be no extra output
  ASSERT_TRUE(notag_arb_test->test_out.resp1.empty());
  ASSERT_TRUE(notag_arb_test->test_out.resp2.empty());
  ASSERT_TRUE(notag_arb_test->test_out.resp3.empty());
  ASSERT_TRUE(notag_arb_test->test_out.resp4.empty());
}


TEST(LeadingOneTest, Test) {
  bool bv[] = {0,1,1,0};
  unsigned lo, ao;
  bool found_lo = bitvec_utils<unsigned, 4>::find_leading_one(bv, lo);
  bool found_ao = bitvec_utils<unsigned, 4>::find_any_one(bv, ao);

  ASSERT_TRUE(found_lo);
  ASSERT_TRUE(found_ao);
  ASSERT_EQ(lo, ao);
  ASSERT_EQ(lo, (unsigned)1);

  bool bv1[] = {0,0,0,0,0,1,0};
  found_lo = bitvec_utils<unsigned, 7>::find_leading_one(bv1, lo);
  found_ao = bitvec_utils<unsigned, 7>::find_any_one(bv1, ao);
  ASSERT_TRUE(found_lo);
  ASSERT_TRUE(found_ao);
  ASSERT_EQ(lo, ao);
  ASSERT_EQ(lo, (unsigned)5);

  bool bv2[] = {0,0,0,0,0,0,0};
  found_lo = bitvec_utils<unsigned, 7>::find_leading_one(bv2, lo);
  found_ao = bitvec_utils<unsigned, 7>::find_any_one(bv2, ao);
  ASSERT_FALSE(found_lo);
  ASSERT_FALSE(found_ao);

}

TEST(UInt64, BasicTest) {
  sc_uint<64> val = 0x3fd0000000000000;
  unsigned long int val_li = val;
  cout << std::hex << val << " ? " << val_li << endl;
  ASSERT_TRUE ((unsigned long int)val == val_li);

  val_li = 0x3fd0000000000000;
  val = (sc_uint<64>)val_li;
  cout << val << " ? " << val_li << endl;
  ASSERT_TRUE (val == (sc_uint<64>)val_li);

}

#include "sysc/datatypes/fx/sc_fixed.h"

int sc_main (int argc, char *argv[]) {
  ::testing::InitGoogleTest(&argc, argv);

  //TestMemReadTop* test_read_top = new TestMemReadTop("read_top", dram, test_read_in, test_read_out);
  //TestMemWriteTop* test_write_top = new TestMemWriteTop("write_top", dram);
  return RUN_ALL_TESTS();
  //delete test_read_top;


  //return 0;
}
