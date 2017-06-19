// See LICENSE for license details.
#ifdef SC_KERNEL
#include "pagerank_hls.h"
#include "pr_kernel_tb.h"
#elif FPGA_ACC
#include "AcclApp.h"
#else
#include "pagerank_tb.h"
#endif

#include "pr_types.h"
#include "file_ops.h"

VtxId vtxCnt;
EdgeId edgeCnt;

VertexData *vertexData;
EdgeId * edgeOffsets;

PageRankType * scaledPageRanks;
PageRankType * neighbOrderedPageRanks;
PageRankType * scaledPageRanks_temp;
PRreal *prRealPageRanks;
PRreal *storeArr;

unsigned char *WORKSPACE;
size_t WORKSPACE_SIZE;


VtxDegree * otherVtxIds;

unsigned int numIter;

PRreal RANKOFFSET;

int sc_main(int argc, char *argv[])
{
   AcclApp theApp(PR_AFU_ID);
   WORKSPACE_SIZE = (unsigned long long)12*1000*1024*1024;
   WORKSPACE = (unsigned char *)theApp.alloc(WORKSPACE_SIZE); // 2GB

  if (WORKSPACE != NULL) {
   
#ifdef GRAPHGEN
    if (argc != 3) {
      cout << "No filename (without .in) or graph size given." << endl
           << "Usage: ga GRAPHNAME SIZE_IN_EXP" << endl;
      return 1;
    }
    string graphFileName = argv[1];
    string graphSize = argv[2];
    graph_generate(graphSize);
#else
    if (argc != 2) {
      cout << "No filename (without .in) given." << endl
           << "Usage: ga GRAPHNAME" << endl;
      return 1;
    }
    string graphFileName = argv[1];
    graph_read(graphFileName);
#endif
    RANKOFFSET=(1-BETA)/vtxCnt;
    numIter=10;
    Config config;
    config.setRankOffset(RANKOFFSET);
    assert((AddrType)scaledPageRanks > (AddrType)vertexData);
    assert((AddrType)otherVtxIds > (AddrType)vertexData);
    assert((AddrType)scaledPageRanks_temp > (AddrType)vertexData);

    unsigned offset = 0;
    config.setVDAddr((AddrType)vertexData);
    config.setReadPRAddr((AddrType)scaledPageRanks - (AddrType)vertexData);
    config.setVIDAddr((AddrType)otherVtxIds - (AddrType)vertexData);
    config.setWritePRAddr((AddrType)scaledPageRanks_temp - (AddrType)vertexData);
    config.vtxCnt = vtxCnt - offset;
    config.iterCount = ITER_COUNT;
    config.vtxOffset = offset;
    config.edgeCnt = edgeCnt;
 
    for (unsigned i = 0; i < ITER_COUNT; ++i) {
      config.iterCount--;
      theApp.compute(&config, sizeof(config));
      cout << "You can do stuff in parallel here on the HOST and call join when you're ready for FPGA results" << endl;
      theApp.join();
      config.swapReadWritePR();
    }
 
    //swap read/write buffers making sure scaledPageRanks gets the updated values
    if (ITER_COUNT%2!=0) {
      for(VtxId vi=0;vi<vtxCnt; vi++){
        scaledPageRanks[vi].setPr(scaledPageRanks_temp[vi].getPr());
      }
    }
 
    // check result here?
    graph_write(graphFileName);

    theApp.free();
  }


  return 0;   
}

