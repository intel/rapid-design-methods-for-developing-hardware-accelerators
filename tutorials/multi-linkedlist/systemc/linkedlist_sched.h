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
#ifndef __LINKEDLIST_SCHED_H__
#define __LINKEDLIST_SCHED_H__
//[[[end]]] (checksum: 288e4b8638e385855e57109d385c7464)

#include "multi_acc_template.h"

template <size_t N>
/*[[[cog
     cog.outl("class %s_sched : public sc_module, public acc_scheduler_interface<N, Config> {" % (dut.nm,))
  ]]]*/
class linkedlist_sched : public sc_module, public acc_scheduler_interface<N, Config> {
//[[[end]]] (checksum: 15ba019759d24202d247d4b9d1308d98)
public:

  /*[[[cog
       cog.outl("SC_HAS_PROCESS(%s_sched);" % (dut.nm,))
    ]]]*/
  SC_HAS_PROCESS(linkedlist_sched);
  //[[[end]]] (checksum: 97aa63d887f67bc3cf738ea60292d9d6)
  /*[[[cog
       cog.outl("%s_sched(sc_module_name name = sc_gen_unique_name(\"%s_sched\")) :" % (dut.nm,dut.nm))
    ]]]*/
  linkedlist_sched(sc_module_name name = sc_gen_unique_name("linkedlist_sched")) :
  //[[[end]]] (checksum: d654e0cfb69fedc00730ee1d86300bb7)
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

              unsigned long long chunk = acc_conf.get_m() / N;
              if ( N == 3) {
                unsigned long long q = (acc_conf.get_m()>>2) + (acc_conf.get_m()>>4);
                chunk = q + (q>>4) + (q>>8);
              }
              if ( i == N-1) {
                acc_conf.set_m( acc_conf.get_m()-chunk*(N-1));
              } else {
                acc_conf.set_m( chunk);
              }
              acc_conf.set_aInp( acc_conf.get_aInp() + i*chunk*sizeof(HeadPtr));
              acc_conf.set_aOut( acc_conf.get_aOut() + i*chunk*sizeof(HeadPtr));
              std::cout << "i,m,aInp,aOut:"
                << " " << i
                << "," << acc_conf.get_m()
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

