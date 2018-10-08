// See LICENSE for license details.
#include <stdint.h>
#include <string.h>
#include <assert.h>

#include "systemc.h"
#include "hld_alloc.h"

#include "AcclApp.h"

struct Config {
  unsigned long long in0Addr : 64;
  unsigned long long in1Addr : 64;
  unsigned long long outAddr : 64;
  unsigned vecCount : 32;
};

int sc_main(int argc, char *argv[]) {

  AcclApp acc_app("5cdf79ed09527e2cbfa1f873fa0e52cf");

  size_t count = 256;

  size_t workspace_size = 3*count*sizeof(unsigned int) ;
  
  void *workspace = acc_app.alloc(workspace_size);

  cout << "WORKSPACE ALLOCATED OF SIZE " << workspace_size << " @ " << hex << (size_t)workspace << endl << dec;
  assert(workspace);  
  

  hld_alloc alloc((char*)workspace, workspace_size);
  unsigned int *invec1 = alloc.allocate<unsigned int>(count);
  unsigned int *invec2 = alloc.allocate<unsigned int>(count);
  unsigned int *outvec = alloc.allocate<unsigned int>(count);
  
  Config config;

  for(unsigned i = 0; i < count; ++i) {
    invec1[i] = rand()%100;
    invec2[i] = rand()%100;
  }

  config.in0Addr = reinterpret_cast<unsigned long long>(invec1);
  config.in1Addr = reinterpret_cast<unsigned long long>(invec2);
  config.outAddr = reinterpret_cast<unsigned long long>(outvec);
  config.vecCount = count/16; // count is in terms of vectors of int

  acc_app.compute(&config, sizeof(Config));
  acc_app.join();

  // verification
  size_t failed = 0;
  for(unsigned i = 0; i < count; ++i) {
    bool match = invec1[i]+invec2[i]==outvec[i];
    cout << i << " - invec1[i] = " << invec1[i] << " invec2[i] " << invec2[i] << " outvec[i] = " << outvec[i] << " " << (match)?"SUCCESS":"FAILURE"<<endl;
    if(!match) ++failed;
  }
  assert(!failed);

  acc_app.free();
  return 0;
}
