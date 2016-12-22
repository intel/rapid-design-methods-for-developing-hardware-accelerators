// See LICENSE for license details.
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

#include "cycle_detection.h"
#include "fpga_app_sw.h"

struct AcclApp : public FpgaAppSwAlloc /*this class provides allocation service*/ {

   AcclApp(const char *auid = NULL) : FpgaAppSwAlloc (auid) {}
   
   void compute (const void * config_ptr, const unsigned int config_size) {
     cout << "SW compute.." << endl;
     const Config *config = static_cast<const Config *>(config_ptr);
     listnode_t *head = reinterpret_cast<listnode_t *>(config->getInpAddr(0));
     int *result = reinterpret_cast<int *>(config->getOutAddr(0));
     *result = cycle_detect(head);
   }
   
   int cycle_detect(listnode_t *head) {
     listnode_t *fast, *slow;

     slow = head;
     fast = head;
 
     while(!(slow == NULL || fast == NULL)) {
       /* bump fast pointer second time */
       cout << "fast node data = " << fast->val << endl;
       fast = fast->next;
       if(!fast)
         return 0;

       if(fast == slow)
         return 1;

       cout << "fast node data = " << fast->val << endl;
       cout << "slow node data = " << slow->val << endl;

       fast = fast->next;
       slow = slow->next;


     }

     return 0;
   }
   
   void join() {}
};






