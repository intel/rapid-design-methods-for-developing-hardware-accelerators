// See LICENSE for license details.
/*
* kernel_tb_stub.h
*
*  Created on: Oct 6, 2016
*      Author: aayupov
*/

//
// This is a stub for the kernel-level testbench, before you integrated your kernel with memory load store units and arbiters
// The stub is designed for a DUT (design under test) module that uses one read and one write ports.
// You may need to replicate some code if your design requires more ports
//
#include "kernel_test.h"

// USER: Copy/paste the following portion inside comments to the main.cpp file
// USER: the filename should correspond the where the KernelTestTop is defined in case you rename this file
int sc_main (int argc, char *argv[]) {
  KernelTestTop top("top");
  sc_start();
  return 0;
}

