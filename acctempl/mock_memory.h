// See LICENSE for license details.
/*
 * mock_memory.h
 *
 *  Created on: Sep 22, 2016
 *      Author: aayupov
 */

#ifndef MOCK_MEMORY_H_
#define MOCK_MEMORY_H_

#include "systemc.h"
#include "fpga_app_sw.h"
#include <deque>
#include <utility>
#include <queue>
#include <random>
#include "acc_mem_arbiter_trait.h"

template <size_t ACCELERATOR_FREQ_IN_MHZ, size_t LATENCY_IN_NS, size_t RD_PORTS, size_t WR_PORTS, size_t SPREAD_IN_NS = 0>
class MockMemorySimulator : public sc_module
{
public:

  typedef std::pair<std::pair<unsigned long long,unsigned long long>,SplMemReadRespType> PQ_packet;
  struct PQ_cmp {
    bool operator()( const PQ_packet& a, const PQ_packet& b) {
      return a.first > b.first;
    }
  };

  sc_in_clk clk;
  sc_in<bool> rst;

  // cacheline size in bytes
  enum {
    CLSIZE = 64
  };

  sc_vector<ga::tlm_fifo_in<SplMemReadReqType> > memReadReqIn;
  sc_vector<ga::tlm_fifo_out<SplMemReadRespType> > memReadRespOut;
  sc_vector<ga::tlm_fifo_in<SplMemWriteReqType> > memWriteReqIn;
  sc_vector<ga::tlm_fifo_out<SplMemWriteRespType> > memWriteRespOut;

  const unsigned int LATENCY;
  const unsigned int SPREAD;

  const IMemRangeHolder *mem_range;

  SC_HAS_PROCESS(MockMemorySimulator);

  MockMemorySimulator(sc_module_name modname, const IMemRangeHolder *memRange = NULL) :
    sc_module(modname)
  , clk("clk")
  , rst("rst")
  , memReadReqIn("memReadReqIn", RD_PORTS)
  , memReadRespOut("memReadRespOut", RD_PORTS)
  , memWriteReqIn("memReadReqIn", WR_PORTS)
  , memWriteRespOut("memReadRespOut", WR_PORTS)
  , LATENCY ( (unsigned int)(LATENCY_IN_NS/ ((float)1000/ACCELERATOR_FREQ_IN_MHZ)+0.5))
  , SPREAD ( (unsigned int)(SPREAD_IN_NS/ ((float)1000/ACCELERATOR_FREQ_IN_MHZ)+0.5))
  , mem_range(memRange)
  {
    SC_CTHREAD(mem_proc, clk.pos());
    async_reset_signal_is(rst, false);
    for( unsigned int i=0; i<RD_PORTS; ++i) {
      memReadReqIn[i].clk_rst( clk, rst);
      memReadRespOut[i].clk_rst( clk, rst);
    }
    for( unsigned int i=0; i<WR_PORTS; ++i) {
      memWriteReqIn[i].clk_rst( clk, rst);
      memWriteRespOut[i].clk_rst( clk, rst);
    }

    assert (LATENCY > 0);
    assert( SPREAD <= LATENCY);

    assert(mem_range != NULL);
  }

  size_t rd_tx_count[RD_PORTS];
  size_t wr_tx_count[WR_PORTS];
  size_t rdwr_cycles;
  double after_reset;
  size_t rd_chan_max_capacity[RD_PORTS];
  void mem_proc() {

    std::priority_queue<PQ_packet,std::deque<PQ_packet>,PQ_cmp> rd_response_queue;
    std::deque<SplMemReadRespType> rd_chan_queues[RD_PORTS];

    std::default_random_engine generator;
    std::uniform_int_distribution<int> distribution(-SPREAD,SPREAD);

    cout << "MEM NUM OF READ PORTS is " << RD_PORTS << endl;
    cout << "MEM NUM OF WRITE PORTS is " << WR_PORTS << endl;

    for( unsigned int i=0; i<RD_PORTS; ++i) {
      memReadReqIn[i].reset_get();
      memReadRespOut[i].reset_put();
      // Read response latency is LATENCY
      cout << "MEM LATENCY is " << LATENCY << " cycles" << endl;
      rd_tx_count[i] = 0;
      rd_chan_max_capacity[i] = 0;
    }
    for( unsigned int i=0; i<WR_PORTS; ++i) {
      memWriteReqIn[i].reset_get();
      memWriteRespOut[i].reset_put();
      wr_tx_count[i] = 0;
    }
    rdwr_cycles = 0;
    {
    }
    after_reset = sc_time_stamp().to_seconds();
    wait();
    while (1) {
      {
        for( unsigned i=0; i<RD_PORTS; ++i) {
          if(memReadReqIn[i].nb_can_get()) {
            SplMemReadReqType req;
            SplMemReadRespType resp;

            memReadReqIn[i].nb_get(req);
            checkAddressRange(req.addr);
            resp.io_unit_id = req.io_unit_id;
            resp.tag = req.tag;
            void *data = (void *)(size_t)(req.addr);
            memcpy(&resp.data.words[0], data, 8*sizeof(resp.data.words[0]));
            //            std::cerr << sc_time_stamp() << " MockMemorySimulator::mem_proc: cacheline read requested at addr: " << req.addr << " with tag " << req.tag << endl;

            int incr = distribution( generator );

            rd_response_queue.push( std::make_pair( std::make_pair( rdwr_cycles+LATENCY+incr, rd_tx_count[i]), resp));
            ++rd_tx_count[i];
          }
          // popping RD_PORTS at a cycle and sending to RD_PORTS queues (modeling async fifos)
          if ( !rd_response_queue.empty()) {
            const PQ_packet& t = rd_response_queue.top();
            if (t.first.first <= rdwr_cycles) { // we should send the response
              SplMemReadRespType resp = t.second;
              // select the right channel
              unsigned rd_chan_index = (RD_PORTS == 1) ? 0 : (unsigned) ArbiterTagResolver<AU_ARBITER>::template extractTag<RD_PORTS>(resp.io_unit_id);
              rd_chan_queues[rd_chan_index].push_back(resp);
              rd_response_queue.pop();
            }
          }

          if ( !rd_chan_queues[i].empty() ) {
            rd_chan_max_capacity[i] = std::max (rd_chan_max_capacity[i], rd_chan_queues[i].size());
            SplMemReadRespType resp = rd_chan_queues[i].front();
            // sending to accelerator
            if ( !memReadRespOut[i].nb_can_put()) {
              std::cout << "WARNING: Expect to be able to put response to request " << resp << std::endl;
            }
            memReadRespOut[i].put(resp);
            rd_chan_queues[i].pop_front();
          }

        }
        // write
        for( unsigned i=0; i<WR_PORTS; ++i) {
          if(memWriteReqIn[i].nb_can_get()) {
            SplMemWriteReqType wr_req;
            memWriteReqIn[i].nb_get(wr_req);
            //DBG_OUT << sc_time_stamp() << " MockMemorySimulator::mem_proc: cacheline write requested at addr: " << wr_req.addr << endl;
            // partial write
            memcpy((void *)(size_t)wr_req.addr, &wr_req.data+wr_req.offset, wr_req.width);
            ++wr_tx_count[i];

          }
        }
        ++rdwr_cycles;
      }
      wait();
    }
  }
  ~MockMemorySimulator() {
    cout << " MOCK_MEMORY: Average cycle period = " <<  (sc_time_stamp().to_seconds()-after_reset)*1.e9/rdwr_cycles << "ns" << endl ;;
    std::ios_base::fmtflags before_fflags = std::cout.flags();
    cout << " MOCK_MEMORY bandwidth stats:" << std::setprecision (2) << std::fixed << endl;
    cout << " MOCK_MEMORY latency "<< LATENCY_IN_NS << "ns, or ~" << LATENCY << "cycles @" << ACCELERATOR_FREQ_IN_MHZ << "Mhz" << endl;
    cout << " MOCK_MEMORY latency spread "<< SPREAD_IN_NS << "ns, or ~" << SPREAD << "cycles @" << ACCELERATOR_FREQ_IN_MHZ << "Mhz" << endl;
    size_t total_rd_tx = 0;
    for( unsigned i=0; i<RD_PORTS; ++i) {
      cout << "    rd channel " << i << " bandwidth = " << (float)rd_tx_count[i]/rdwr_cycles;
      cout /*<< " clock domain X fifo capacity " << rd_chan_max_capacity[i]*/ << endl;
      total_rd_tx += rd_tx_count[i];
    }

    size_t total_wr_tx = 0;
    for( unsigned i=0; i<WR_PORTS; ++i) {
      cout << "    wr channel " << i << " bandwidth = " << (float)wr_tx_count[i]/rdwr_cycles << endl;
      total_wr_tx += wr_tx_count[i];
    }

    cout << "    rd channel average bandwidth = " << (float)total_rd_tx/rdwr_cycles/RD_PORTS << ", which is ~" << (float)total_rd_tx/rdwr_cycles*calcBWInGBs(ACCELERATOR_FREQ_IN_MHZ) << "GB/s @("<<RD_PORTS<<"*"<<ACCELERATOR_FREQ_IN_MHZ<<")Mhz " <<  endl;
    cout << "    wr channel average bandwidth = " << (float)total_wr_tx/rdwr_cycles/WR_PORTS << ", which is ~" << (float)total_wr_tx/rdwr_cycles*calcBWInGBs(ACCELERATOR_FREQ_IN_MHZ) << "GB/s @("<<WR_PORTS<<"*"<<ACCELERATOR_FREQ_IN_MHZ<<")Mhz " <<  endl;

    // restore old formating
    std::cout.flags(before_fflags);
  }

  void checkAddressRange(SplAddressType addr) {
    if (mem_range != NULL) {
      assert (addr >= (size_t)mem_range->getMemRange().offset);
      assert (addr <  (size_t)mem_range->getMemRange().offset + mem_range->getMemRange().size);
    }
  }

  static float calcBWInGBs(float freq_in_mhz) {
    return CLSIZE*freq_in_mhz*1.e6/1024/1024/1024;
  }
};





#endif /* MOCK_MEMORY_H_ */
