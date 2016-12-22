// See LICENSE for license details.

#ifndef CLOCK_GENERATOR_H_
#define CLOCK_GENERATOR_H_

#include "systemc.h"

class ClockGenerator  : public sc_module
{
public:
  sc_out<bool> clk ;
  sc_out<bool> rst;
  SC_HAS_PROCESS(ClockGenerator);

  bool reset_state;
  static const size_t reset_length = 3;
  float half_clock;

  ClockGenerator(sc_module_name modname, float clk_period = 1.0) :
      sc_module(modname), clk("clk"), rst("rst") {
    half_clock = clk_period/2;
    SC_THREAD(gen1);
  }

  void gen() {
    // half clock in ns
    clk = false;
    rst = true;
    wait(half_clock, SC_NS);
    clk = true;
    rst = false;
    wait(half_clock, SC_NS);
    clk = false;
    wait(half_clock, SC_NS);
    clk = true;
    rst = true;
    wait(half_clock, SC_NS);
    while (1) {
      clk = false;
      wait(half_clock, SC_NS);
      clk = true;
      wait(half_clock, SC_NS);
    }
  }

  void gen1() {
    clk = false;
    rst = true;
    // half clock in ns
    while (1) {
      wait(half_clock, SC_NS);
      clk = true;
      if (reset_state) {
        rst = false;
        wait(half_clock, SC_NS);
        clk = false;
        wait(half_clock, SC_NS);
        clk = true;
        wait(half_clock, SC_NS);
        clk = false;
        wait(half_clock, SC_NS);
        clk = true;
        rst = true;
        reset_state = false;
        sc_pause();
      }
      wait(half_clock, SC_NS);
      clk = false;
    }
  }
};


#endif /* CLOCK_GENERATOR_H_ */
