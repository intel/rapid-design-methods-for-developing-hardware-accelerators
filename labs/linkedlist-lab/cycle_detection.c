#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

typedef struct node {
  struct node *next;
  int64_t val;
} listnode_t;

/* floyd's cycle detection algorithm */

int cycle_detect(listnode_t *head) {
  listnode_t *fast, *slow;

  slow = head; 
  fast = head;
  
  while(!(slow == NULL || fast == NULL)) {
    /* bump fast pointer second time */
    
    printf("fast node data = %ld\n", fast->val);
    fast = fast->next;
    if(!fast)
      return 0;

    if(fast == slow)
      return 1;

    printf("fast node data = %ld\n", fast->val);
    printf("slow node data = %ld\n", slow->val);

    fast = fast->next;
    slow = slow->next;

  }

  return 0;
}

int main() {
  int64_t i,n = (1L<<3);
  int rc=0;
  unsigned iter = 10;
  listnode_t *nodes = NULL;
  
  nodes = (listnode_t*)malloc(n*sizeof(listnode_t));
  memset(nodes, 0, n*sizeof(listnode_t));

  for(i=0;i<n;i++) {
    nodes[i].val = (int64_t)i;
    if((i+1) < n)
      nodes[i].next = &nodes[i+1];
  }

  for (size_t c = 0; c < iter; ++c) {
    i = rand() % (2*n);
    printf("i = %ld n = %ld\n", i, n); 
    if(i <n )
      nodes[n-1].next = nodes+i;
    else
      nodes[n-1].next = NULL;

    rc = cycle_detect(nodes);
    
    if(rc)
      assert(i < n);
    
    printf("cycle %s\n", rc ? "found" : "not found"); 
  }
  printf("Test Passed\n");
  free(nodes);
  return 0;
}
