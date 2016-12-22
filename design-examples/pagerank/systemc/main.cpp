// See LICENSE for license details.

#include "file_ops.h"
#include "time.h"
#include <chrono>

//#define PFOVD
//DATA for Pagerank
VtxId vtxCnt;
EdgeId edgeCnt;

PageRankType * scaledPageRanks;
PageRankType * neighbOrderedPageRanks;
PageRankType * scaledPageRanks_temp;
PRreal *prRealPageRanks;
PRreal *storeArr;

VertexData *vertexData;
EdgeId * edgeOffsets;

unsigned int numIter;
VtxDegree * otherVtxIds;
unsigned char *WORKSPACE;
size_t WORKSPACE_SIZE;

PRreal RANKOFFSET;
//Pagerank Implementation
void compute_pagerank(){

  cout<<"Executing ORIG Pagerank for "<<numIter<<" iterations"<<endl;
  clock_t start, stop;
  double t = 0.0;

  /* Start timer */
  assert((start = clock())!=-1);
  auto tstart = std::chrono::steady_clock::now();
  /* Do fancy calculations */
  for(unsigned int iter=0; iter<numIter; iter++){
    //Pagerank_Vtx_Loop_Parallel:
    for(VtxId vi=0;vi<vtxCnt; vi++){
      PRreal sum=0;
      VertexData vd = vertexData[vi];
      //Gather_Edge_Loop:
      for(EdgeId ei=0; ei<vd.ovdSize; ei++){
        VtxId otherVtxId = otherVtxIds[vd.ovdOffset+ei];
        sum+=scaledPageRanks[otherVtxId].getPr();
      }
      //pracc.setPr(pracc.getPr()+sum);
      scaledPageRanks_temp[vi].setPr((RANKOFFSET+BETA*sum)*vd.getOneOverVtxDegree());
    }


    Apply_Loop:
    for(VtxId vi=0;vi<vtxCnt; vi++){
      scaledPageRanks[vi].setPr(scaledPageRanks_temp[vi].getPr());
    }
    cout<<"----"<<endl;
  }
  /* Stop timer */
  stop = clock();
  t = (double) (stop-start)/(CLOCKS_PER_SEC/1000);
  cout<<"CRuntime: " << t << endl;
  cout<<"TRuntime: " << std::chrono::duration_cast<std::chrono::milliseconds>  (std::chrono::steady_clock::now() - tstart).count() << endl;
  //SYNC. HERE

}

//SERIAL Pagerank
void compute_pagerank_serial(){
  cout<<"Executing Serial Pagerank for "<<numIter<<" iterations"<<endl;
  for(unsigned int iter=0; iter<numIter; iter++){
    //Pagerank_Vtx_Loop_Parallel:
    for(VtxId vi=0;vi<vtxCnt; vi++){

      PRreal sum=0;
      //Gather_Edge_Loop:
      for(EdgeId ei=edgeOffsets[vi]; ei<edgeOffsets[vi+1]; ei++){
        VtxId otherVtxId = otherVtxIds[ei];
        sum+=scaledPageRanks[otherVtxId].getPr();
      }
      scaledPageRanks[vi].setPr((RANKOFFSET+BETA*sum)*vertexData[vi].getOneOverVtxDegree());
    }

    // for(VtxId vi=0;vi<vtxCnt; vi++){
    //  cout<<scaledPageRanks[vi]<<endl;
    // }
    // cout<<"----"<<endl;

  }
}

int sc_main(int argc, char** argv) {

  cout<<"Starting Pagerank application"<<endl;
  WORKSPACE_SIZE = 0xF0000000;//1GB

  WORKSPACE = (unsigned char *)malloc(WORKSPACE_SIZE);
  if (argc != 2) {
    cout << "No filename given." << endl
        << "Usage: ga FILENAME" << endl;
    return 1;
  }
  string graphFileName = argv[1];
  graph_read(graphFileName);
  RANKOFFSET=(1-BETA)/vtxCnt;
  numIter=1;
  compute_pagerank();
  graph_write(graphFileName);
  return 0;
}
