// See LICENSE for license details.
// See LICENSE for license details.
#ifndef __CYCLE_DETECTION_H__
#define __CYCLE_DETECTION_H__

#include "types.h"
#include "Config.h"
#include "Node.h"
#include "CycleExistInfo.h"

typedef struct node {
  struct node *next;
  int64_t val;
} listnode_t;


#endif //__CYCLE_DETECTION_H__
