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
#ifndef __VECTORADD_SCHED_H__
#define __VECTORADD_SCHED_H__
//[[[end]]] (checksum: 720f91c850677a26e2bb4b587b900bbe)

#include "multi_acc_template.h"

template <size_t N>
/*[[[cog
     cog.outl("class %s_sched : public sc_module, public acc_scheduler_interface<N, Config> {" % (dut.nm,))
  ]]]*/
class vectoradd_sched : public sc_module, public acc_scheduler_interface<N, Config> {
//[[[end]]] (checksum: 6cf349b79532dabbcc9a7d9c8e1df76e)
public:

  /*[[[cog
       cog.outl("SC_HAS_PROCESS(%s_sched);" % (dut.nm,))
    ]]]*/
  SC_HAS_PROCESS(vectoradd_sched);
  //[[[end]]] (checksum: a8065a2c6397250875e63d4842da86a2)
  /*[[[cog
       cog.outl("%s_sched(sc_module_name name = sc_gen_unique_name(\"%s_sched\")) :" % (dut.nm,dut.nm))
    ]]]*/
  vectoradd_sched(sc_module_name name = sc_gen_unique_name("vectoradd_sched")) :
  //[[[end]]] (checksum: 2c079cedd605498d1293b64ec7505ae3)
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

              unsigned int chunk = acc_conf.get_n() / N;
              if ( N == 3) {
                unsigned int q = (acc_conf.get_n()>>2) + (acc_conf.get_n()>>4);
                chunk = q + (q>>4) + (q>>8);
              }
              if ( i == N-1) {
                acc_conf.set_n( acc_conf.get_n()-chunk*(N-1));
              } else {
                acc_conf.set_n( chunk);
              }
              assert( acc_conf.get_n() % 16 == 0);
              acc_conf.set_aIna( acc_conf.get_aIna() + i*chunk*sizeof(unsigned int));
              acc_conf.set_aInb( acc_conf.get_aInb() + i*chunk*sizeof(unsigned int));
              acc_conf.set_aOut( acc_conf.get_aOut() + i*chunk*sizeof(unsigned int));
              std::cout << "i,n,aIna,aInb,aOut:"
                << " " << i
                << "," << acc_conf.get_n()
                << "," << acc_conf.get_aIna()
                << "," << acc_conf.get_aInb()
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

