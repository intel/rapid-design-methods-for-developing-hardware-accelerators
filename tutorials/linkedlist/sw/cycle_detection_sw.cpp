// See LICENSE for license details.
//#include <stdio.h>
//#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

#include "systemc.h"
#include "cycle_detection.h"
#include "hld_alloc.h"

#ifdef SC_KERNEL
#include "cycle_detection_hls_tb.h"
#elif SC_ACC
#include "cycle_detection_acc_tb.h"
#elif FPGA_ACC
#include "AcclApp.h"
#else
#include "cycle_detection_hw_emul.h"
#endif
 
/* floyd's cycle detection algorithm */

int sc_main(int argc, char *argv[]) {
  

  
  int64_t i,n = (1L<<3);
  //int rc=0;
  unsigned iter = 10;
  srand(iter);
  cout << " first rand() = " << rand() << endl;
  listnode_t *nodes = NULL;
  
  // takes auid (needed only for fpga build)
  AcclApp acc_app("30b1cf9b9bee84e7fd2e242e7efcb9d8");
  
  size_t workspace_size = n*sizeof(listnode_t)+sizeof(int);

  void *workspace = acc_app.alloc(workspace_size);  
  
  
  assert(workspace);  
  Config config;
  
  hld_alloc alloc((char*)workspace, workspace_size);

  //SW: nodes = (listnode_t*)malloc(n*sizeof(listnode_t));
  nodes = alloc.allocate<listnode_t>(n);
  unsigned * rc_ptr = alloc.allocate<unsigned>(1);
  *rc_ptr = 0;
  
  std::cout << "rc_ptr = " << std::hex << (size_t)rc_ptr << std::dec<< " " << sizeof(*rc_ptr) << std::endl;
  config.set_aVD((long long unsigned)&nodes[0]);
  config.set_oInp(0);
  config.set_oOut((long long unsigned) rc_ptr - config.get_aVD());
  
  assert(nodes);
  memset(nodes, 0, n*sizeof(listnode_t));

  for(i=0;i<n;i++) {
    nodes[i].val = (int64_t)i;
    if((i+1) < n)
      nodes[i].next = &nodes[i+1];
  }

  for (size_t c = 0; c < iter; ++c) {
    i = rand() % (2*n);
    cout << " i = " << i << endl;
    if(i <n ) {
      nodes[n-1].next = nodes+i;
      cout << "tail points to " << i << endl;
    }
    else {
      nodes[n-1].next = NULL;
      cout << "tail points to NULL" << endl;

    }
    //SW: rc = cycle_detect(nodes);
    acc_app.compute(&config, sizeof(Config));
    acc_app.join();
    

    printf("cycle %s - %u\n", (*rc_ptr) ? "found" : "not found", *rc_ptr); 

    if(*rc_ptr)
      assert(i < n);
    else
      assert(i >= n);
    
  }
  acc_app.free();
  return 0;
}
