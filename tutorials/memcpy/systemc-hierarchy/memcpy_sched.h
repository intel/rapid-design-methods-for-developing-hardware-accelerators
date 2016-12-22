// See LICENSE for license details.
/*[[[cog
     import cog
     from cog_acctempl import *
     from dut_params import *
  ]]]*/
//[[[end]]] (checksum: d41d8cd98f00b204e9800998ecf8427e)
/*[[[cog
     cog.outl("#ifndef __%s_SCHED_H__" % dut.nm.upper())
     cog.outl("#define __%s_SCHED_H__" % dut.nm.upper())
  ]]]*/
#ifndef __MEMCPY_SCHED_H__
#define __MEMCPY_SCHED_H__
//[[[end]]] (checksum: 08166874e1343127c8bd555287b0dfd8)

#include "multi_acc_template.h"

template <size_t N>
/*[[[cog
     cog.outl("class %s_sched : public sc_module, public acc_scheduler_interface<N, Config> {" % (dut.nm,))
  ]]]*/
class memcpy_sched : public sc_module, public acc_scheduler_interface<N, Config> {
//[[[end]]] (checksum: 68410f2d7a648ae9b35fc886a47bb520)
public:

  /*[[[cog
       cog.outl("SC_HAS_PROCESS(%s_sched);" % (dut.nm,))
    ]]]*/
  SC_HAS_PROCESS(memcpy_sched);
  //[[[end]]] (checksum: 36a193ce73804782c4fccc1fa469d08b)
  /*[[[cog
       cog.outl("%s_sched(sc_module_name name = sc_gen_unique_name(\"%s_sched\")) :" % (dut.nm,dut.nm))
    ]]]*/
  memcpy_sched(sc_module_name name = sc_gen_unique_name("memcpy_sched")) :
  //[[[end]]] (checksum: 39f285cac45e5d7d8cd0ef8d40cdacf6)
    sc_module(name),
    acc_scheduler_interface<N, Config>() {

    SC_CTHREAD(schedule_proc, this->clk.pos());
    async_reset_signal_is(this->rst, false);
  }

  void schedule_proc() {
    bool config_copied = false;

    {
      SCHEDULE_RESET_UNROLL: for (unsigned int i = 0; i < N; ++i) {
        this->acc_start[i] = false;
      }
    }

    this->done = false;
    wait();
    while (1) {
      {
        if (this->start.read()) {
          if ( !config_copied) {
            Config conf = this->config.read();
            SCHEDULE_START_UNROLL: for (unsigned i = 0; i < N; ++i) {
              Config acc_conf;
              acc_conf.copy(conf);

              unsigned long long chunk = acc_conf.get_nCLs() / N;
              if ( N == 3) {
                unsigned long long q = (acc_conf.get_nCLs()>>2) + (acc_conf.get_nCLs()>>4);
                chunk = q + (q>>4) + (q>>8);
              }
              if ( i == N-1) {
                acc_conf.set_nCLs( acc_conf.get_nCLs()-chunk*(N-1));
              } else {
                acc_conf.set_nCLs( chunk);
              }
              acc_conf.set_aInp( acc_conf.get_aInp() + i*chunk*sizeof(CacheLine));
              acc_conf.set_aOut( acc_conf.get_aOut() + i*chunk*sizeof(CacheLine));
              std::cout << "i,nCLs,aInp,aOut:"
                << " " << i
                << "," << acc_conf.get_nCLs()
                << "," << acc_conf.get_aInp()
                << "," << acc_conf.get_aOut()
                << std::endl;

              this->acc_config[i].write(acc_conf);
              this->acc_start[i] = true;
            }
            config_copied = true;
          } else {
            bool all_done = true;
            SCHEDULE_DONE_UNROLL: for (unsigned i = 0; i < N; ++i) {
              bool accdone = this->acc_done[i] && this->acc_start[i];
              all_done = all_done && accdone;
            }
            if (all_done) {
              this->done = true;
            }
          }
        }
      }
      wait();
    }
  }
};

#endif

