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

#ifdef GRAPHGEN
#include "benchmark.h"
#include "builder.h"
#include "command_line.h"
#include "graph.h"
#include "pvector.h"
#endif


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
#ifdef GRAPHGEN

void graph_generate(string graphSize){
  //Only in edges are read
  const char *argv[] = {"pagerank", "-g", graphSize.c_str()};
  int argc = 3;
  cout << " graph size = 2^" << graphSize << endl;
  CLPageRank cli(argc, const_cast<char**>(argv), "pagerank", 1e-4, 20);
  if (!cli.ParseArgs())
    assert(0);
    
  Builder b(cli);
  Graph g = b.MakeGraph();
  
  vtxCnt = g.num_nodes();
  edgeCnt = g.num_edges_directed();
  
  cout<<"Generating graph of size: "<< graphSize << endl;
  cout<<"Number of Vtxs: "<<vtxCnt<<" number of edges: "<<edgeCnt<<endl;

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

  EdgeId currEdgeId = 0;

  for (int vi = 0; vi < g.num_nodes(); ++vi) {
      
    vertexData[vi].setOneOverVtxDegree((g.out_degree(vi)==0) ? 1.0f : 1.0f/g.out_degree(vi));
      
    vertexData[vi].ovdOffset = g.out_neigh(vi).begin() - g.out_neigh(0).begin();;
      //cout << "vi = " << vi << " vd.ovdOffset = " << vd.ovdOffset << " g.out_degree(vi) = " << g.out_degree(vi) << " total_out_degree = " << total_out_degree << endl;
    vertexData[vi].ovdSize = g.out_neigh(vi).end() - g.out_neigh(vi).begin();
    
    
    for (NodeID neigh: g.out_neigh(vi)) {
      assert ((int)currEdgeId < g.num_edges_directed());
      otherVtxIds[currEdgeId] = neigh;
      ++currEdgeId;
    }
    float initScore = 1.0/vtxCnt;
    prRealPageRanks[vi] = initScore;
    scaledPageRanks[vi].setPr((PRreal)initScore);
  }

}
#endif

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
