// See LICENSE for license details.
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

#include "fpga_app_sw.h"

struct AcclApp : public FpgaAppSwAlloc {

   void compute (const void * config_ptr, const unsigned int config_size) {
     const Config& config = *static_cast<const Config *>(config_ptr);

     HeadPtr *heads   = config.getInpPtr();
     HeadPtr *results = config.getOutPtr();
     for( unsigned int ip = 0; ip < config.get_m(); ++ip) {
       results[ip] = heads[ip];
       results[ip].set_found( cycle_detect(heads[ip].head));
     }
   }
   
   int cycle_detect(Node *head) {
     Node *fast = head;
     Node *slow = head;
 
     while( fast) {
       fast = fast->next;
       if(!fast) {
         //         std::cout << "No cycle after odd fast advances" << std::endl;
         return 0;
       }
       if(fast == slow) {
         //         std::cout << "Cycle found" << std::endl;
         return 1;
       }
       fast = fast->next;
       slow = slow->next;
     }

     //     std::cout << "No cycle after even fast advances" << std::endl;
     return 0;
   }
   
   void join() {}
};






