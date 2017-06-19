// See LICENSE for license details.
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

#include "Config.h"
#include "fpga_app_sw.h"


struct AcclApp : public FpgaAppSwAlloc /*this class provides allocation service*/ {

   AcclApp () : FpgaAppSwAlloc() {}
   AcclApp (const char *auid = NULL) : FpgaAppSwAlloc(auid) {}

   void compute (const void * config_ptr, const unsigned int config_size) {
     cout << "SW compute.." << endl;
     const Config *config = static_cast<const Config *>(config_ptr);
     
     // LAB1 CODE: convert config interface to original SW interface and call the cycle_detect function

     // END OF LAB1 CODE
   }
   
   int cycle_detect(Node *head) {
     Node *fast, *slow;

     slow = head;
     fast = head;
 
     while(!(slow == NULL || fast == NULL)) {
       /* bump fast pointer second time */
       cout << "fast node data = " << fast->val << endl;
       fast = reinterpret_cast<Node *>(fast->next_offset);
       if(!fast)
         return 0;

       if(fast == slow)
         return 1;

       cout << "fast node data = " << fast->val << endl;
       cout << "slow node data = " << slow->val << endl;

       fast = reinterpret_cast<Node *>(fast->next_offset);
       slow = reinterpret_cast<Node *>(slow->next_offset);
     }

     return 0;
   }
   
   void join() {}
};






