// See LICENSE for license details.
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
 
int sc_main(int argc, char *argv[]) {
  int64_t i,n = (1L<<6);
  //int rc=0;
  unsigned iter = 10;
  srand(iter);
  cout << " first rand() = " << rand() << endl;
  
  AcclApp acc_app("30b1cf9b9bee84e7fd2e242e7efcb9d8");
  
  size_t workspace_size = n*sizeof(listnode_t)+sizeof(int);

  void *workspace = acc_app.alloc(workspace_size);  
  
  
  assert(workspace);  
  Config config;
  
  hld_alloc alloc((char*)workspace, workspace_size);

  //SW: nodes = (listnode_t*)malloc(n*sizeof(listnode_t));
  Node *nodes = alloc.allocate<Node>(n);
  CycleExistInfo *cyc_exist = alloc.allocate<CycleExistInfo>(1);
  unsigned *rc_ptr = &(cyc_exist->info);
  *rc_ptr = 0;
  
  config.set_aInp((long long unsigned) nodes);
  config.set_aOut((long long unsigned) cyc_exist );
  
  assert(nodes);
  memset(nodes, 0, n*sizeof(listnode_t));

  // generate linked list
  for(i=0;i<n;i++) {
    nodes[i].val = (int64_t)i;
    if((i+1) < n)
      nodes[i].next_offset = reinterpret_cast<size_t>(&nodes[i+1]);
  }

  for (size_t c = 0; c < iter; ++c) {
    i = rand() % (2*n);
    cout << " i = " << i << endl;
    if(i <n ) {
      nodes[n-1].next_offset = reinterpret_cast<size_t>(nodes+i);
      cout << "tail points to " << i << endl;
    }
    else {
      nodes[n-1].next_offset = 0;
      cout << "tail points to NULL" << endl;
    }
    
    //SW: rc = cycle_detect(nodes);
    acc_app.compute(&config, sizeof(Config));
    acc_app.join();
    
    printf("cycle %s - %u\n", (*rc_ptr) ? "found" : "not found", *rc_ptr); 
    // check result
    if(*rc_ptr && i >= n) {
      printf("TEST FAILED - no cycle in reference implementation\n");
      assert(i < n);
    }
    if (!(*rc_ptr) && i < n) {
      printf("TEST FAILED - there is a cycle in reference implementation\n");
      assert(i >= n);
    }
    printf("TEST PASSED\n");
  }
  acc_app.free();
  return 0;
}
