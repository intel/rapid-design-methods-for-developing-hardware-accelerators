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
#ifndef __SOBEL_SCHED_H__
#define __SOBEL_SCHED_H__
//[[[end]]] (checksum: 5e0c2d162a80a29adc75a198ca74f7b1)

#include "multi_acc_template.h"

template <size_t N>
/*[[[cog
     cog.outl("class %s_sched : public sc_module, public acc_scheduler_interface<N, Config> {" % (dut.nm,))
  ]]]*/
class sobel_sched : public sc_module, public acc_scheduler_interface<N, Config> {
//[[[end]]] (checksum: c2ccc2c2c6e23ac8f66f48d65ead1944)
public:

  /*[[[cog
       cog.outl("SC_HAS_PROCESS(%s_sched);" % (dut.nm,))
    ]]]*/
  SC_HAS_PROCESS(sobel_sched);
  //[[[end]]] (checksum: d125acd04808a0e6acf481d7463d2fb4)
  /*[[[cog
       cog.outl("%s_sched(sc_module_name name = sc_gen_unique_name(\"%s_sched\")) :" % (dut.nm,dut.nm))
    ]]]*/
  sobel_sched(sc_module_name name = sc_gen_unique_name("sobel_sched")) :
  //[[[end]]] (checksum: 385ceb4618e000c69f631cc8bdefba11)
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

              assert( conf.get_num_of_images() == N);

              acc_conf.set_aInp( conf.get_aInp() + i*conf.get_image_size_in_bytes());
              acc_conf.set_aOut( conf.get_aOut() + i*conf.get_image_size_in_bytes());
              acc_conf.set_num_of_images( 1);

              std::cout << "i,aInp,aOut,nInp,num_of_images:"
                << " " << i
                << "," << acc_conf.get_aInp()
                << "," << acc_conf.get_aOut()
                << "," << acc_conf.get_nInp()
                << "," << acc_conf.get_num_of_images()
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

