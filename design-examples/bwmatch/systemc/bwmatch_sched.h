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
#ifndef __BWMATCH_SCHED_H__
#define __BWMATCH_SCHED_H__
//[[[end]]] (checksum: 5e09cf246fed37370e5a6738a437fba6)

#include "multi_acc_template.h"

template <size_t N>
/*[[[cog
     cog.outl("class %s_sched : public sc_module, public acc_scheduler_interface<N, Config> {" % (dut.nm,))
  ]]]*/
class bwmatch_sched : public sc_module, public acc_scheduler_interface<N, Config> {
//[[[end]]] (checksum: 1ea074d1d338822034fe27721360faf0)
public:

  /*[[[cog
       cog.outl("SC_HAS_PROCESS(%s_sched);" % (dut.nm,))
    ]]]*/
  SC_HAS_PROCESS(bwmatch_sched);
  //[[[end]]] (checksum: 325c86c6524bf9821e07b4ebf81cf78b)
  /*[[[cog
       cog.outl("%s_sched(sc_module_name name = sc_gen_unique_name(\"%s_sched\")) :" % (dut.nm,dut.nm))
    ]]]*/
  bwmatch_sched(sc_module_name name = sc_gen_unique_name("bwmatch_sched")) :
  //[[[end]]] (checksum: 656c831d1f0e67350c28b65c44f30a7e)
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

              unsigned long long chunk = acc_conf.get_nPat() / N;
              if ( N == 3) {
                unsigned long long q = (acc_conf.get_nPat()>>2) + (acc_conf.get_nPat()>>4);
                chunk = q + (q>>4) + (q>>8);
              }
              if ( i == N-1) {
                acc_conf.set_nPat( acc_conf.get_nPat()-chunk*(N-1));
              } else {
                acc_conf.set_nPat( chunk);
              }
              acc_conf.set_aPat( acc_conf.get_aPat() + i*chunk*sizeof(BWPattern));
              acc_conf.set_aRes( acc_conf.get_aRes() + i*chunk*sizeof(BWResult));
              std::cout << "i,nPat,aPat,aRes:"
                << " " << i
                << "," << acc_conf.get_nPat()
                << "," << acc_conf.get_aPat()
                << "," << acc_conf.get_aRes()
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

