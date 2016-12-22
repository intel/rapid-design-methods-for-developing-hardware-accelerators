// See LICENSE for license details.
/*[[[cog
     import cog
     from cog_acctempl import *
     from dut_params import *
  ]]]*/
//[[[end]]]
/*[[[cog
     cog.outl("#ifndef __%s_SCHED_H__" % dut.nm.upper())
     cog.outl("#define __%s_SCHED_H__" % dut.nm.upper())
  ]]]*/
//[[[end]]]

#include "multi_acc_template.h"

template <size_t N>
/*[[[cog
     cog.outl("class %s_sched : public sc_module, public acc_scheduler_interface<N, Config> {" % (dut.nm,))
  ]]]*/
//[[[end]]]
public:

  /*[[[cog
       cog.outl("SC_HAS_PROCESS(%s_sched);" % (dut.nm,))
    ]]]*/
  //[[[end]]]
  /*[[[cog
       cog.outl("%s_sched(sc_module_name name = sc_gen_unique_name(\"%s_sched\")) :" % (dut.nm,dut.nm))
    ]]]*/
  //[[[end]]]
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

              // Overwrite acc_conf with parameters for the ith AUs computation

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

