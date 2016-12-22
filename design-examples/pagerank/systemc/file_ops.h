// See LICENSE for license details.
#ifndef __FILE_OPS__H__
#define __FILE_OPS__H__

#include "pr_types.h"

#include <assert.h>
//#include <iostream>
#include <fstream>
using std::ifstream ;
using std::ofstream ;

#include "hld_alloc.h"

extern VtxId vtxCnt;
extern EdgeId edgeCnt;

extern VertexData *vertexData;
extern VtxDegree * otherVtxIds;
extern VtxDegree * inEdgeCnts;
extern VtxId * edgeOffsets;
extern PageRankType * scaledPageRanks;
extern PRreal *prRealPageRanks;
extern PRreal *storeArr;
extern PageRankType * neighbOrderedPageRanks;

extern PageRankType * scaledPageRanks_temp;
extern  unsigned char *WORKSPACE;
extern  size_t WORKSPACE_SIZE;
extern bool * bv_ping;
extern bool * bv_pong;

void graph_read(string prefix){
  //Only in edges are read
  string filename = prefix + ".in";
  ifstream ifs(filename.c_str());
  cout << "file " << filename << " is read next" << endl;
  assert(ifs.is_open());

  ifs>>vtxCnt;

  ifs>>edgeCnt;
  cout<<"Reading graph: "<<prefix<<endl;
  cout<<"Number of Vtxs: "<<vtxCnt<<" number of edges: "<<edgeCnt<<endl;

  //
  //	DEALLOC_ARR(scaledPageRanks);
  //	DEALLOC_ARR(otherVtxIds);
  //	DEALLOC_ARR(edgeOffsets);



  hld_alloc altor((char *)WORKSPACE,WORKSPACE_SIZE);

  vertexData = altor.allocate<VertexData>(vtxCnt);
  scaledPageRanks = altor.allocate<PageRankType>(vtxCnt);
  prRealPageRanks = altor.allocate<PRreal>(vtxCnt);
  storeArr = altor.allocate<PRreal>(edgeCnt/8 + 1);
  neighbOrderedPageRanks = altor.allocate<PageRankType>(2*edgeCnt);
  scaledPageRanks_temp = altor.allocate<PageRankType> (vtxCnt);
  otherVtxIds = altor.allocate<VtxDegree>(2*edgeCnt);
  edgeOffsets = altor.allocate<EdgeId>(vtxCnt+1);
  memset(otherVtxIds, 0, sizeof(VtxDegree)*2*edgeCnt);
  cout<<"Buffer used: "<< altor.getOffset() << " bytes" << endl;
  EdgeId currEdgeId = 0;

  for(unsigned int vtxIter=0; vtxIter<vtxCnt; vtxIter++){
    //COUT<<"VtxId: "<<vtxIter<<endl;
    VtxDegree numEdges;
    VtxDegree numInEdges;

    ifs>>numEdges;
    ifs>>numInEdges;
    //COUT<<"- numEdges: "<<numEdges<<" numInEdges: "<<numInEdges<<endl;
    edgeOffsets[vtxIter]=currEdgeId;
    vertexData[vtxIter].ovdOffset = currEdgeId;
    vertexData[vtxIter].ovdSize = numInEdges;
    for(unsigned int edgeIter=0; edgeIter<numEdges; edgeIter++){
      EdgeId edgeId;
      VtxId otherVtxId;
      ifs>>edgeId;
      ifs>>otherVtxId;

      if(edgeIter<numInEdges){
        otherVtxIds[currEdgeId]=otherVtxId;
        //COUT<<" "<<otherVtxId;
        currEdgeId++;
      }
    }
    //COUT<<endl;
    float scaledPageRank;
    float oneOverVtxDegree;

    ifs>>oneOverVtxDegree;
    ifs>>scaledPageRank;

    prRealPageRanks[vtxIter] = scaledPageRank;

    scaledPageRanks[vtxIter].setPr((PRreal)scaledPageRank);
    vertexData[vtxIter].setOneOverVtxDegree((PRreal)oneOverVtxDegree);

    //COUT<<"- scaledPageRank: "<<scaledPageRank<<" oneOverVtxDegree: "<<oneOverVtxDegree<<endl;
  }
  edgeOffsets[vtxCnt]=currEdgeId;



}

void graph_write(string prefix){
  string filename = prefix + ".out";
  ofstream ofs(filename.c_str());
  cout<<"Writing graph: "<<prefix<<endl;
  ofs<<vtxCnt<<endl;
  for(unsigned i=0; i<vtxCnt; i++){
    ofs<<scaledPageRanks[i].getPr()<<endl;
  }
}

#endif
