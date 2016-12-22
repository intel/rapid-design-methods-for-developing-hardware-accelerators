from cog_acctempl import *

dut = DUT("pagerank")

dut.add_rds( [TypedRead("VertexData","vtx"),
        TypedRead("VertexIdType","ovId"),
        SingleRead("PageRankType","pagerank_hls::Tag","ovpr"),
        TypedRead("PageRankType","lvpr")] )
        
dut.add_wrs( [TypedWrite("PageRankType","prWrite")] )

VTX_ROW_SIZE = 128

dut.add_cthread(CThread("vertex_fetcher"))
dut.add_cthread(CThread("gather_proc"))
dut.add_cthread(CThread("apply_proc"))
dut.add_cthread(CThread("apply_proc2"))
dut.add_cthread(CThread("commit_proc"))

dut.add_storage_fifo(StorageFIFO("Tag", VTX_ROW_SIZE, "applyQ"))
dut.add_storage_fifo(StorageFIFO("Tag", VTX_ROW_SIZE, "gatherQ"))
dut.add_storage_fifo(StorageFIFO("PageRankType", 2, "writePrQ"))
dut.add_storage_fifo(StorageFIFO("Tag", 2, "commitQ"))

dut.get_cthread("vertex_fetcher").add_port(RdReqPort("vtx"))
dut.get_cthread("vertex_fetcher").add_port(RdRespPort("vtx"))
dut.get_cthread("vertex_fetcher").add_port(RdReqPort("lvpr"))
dut.get_cthread("vertex_fetcher").add_port(RdRespPort("lvpr"))
dut.get_cthread("vertex_fetcher").add_port(RdReqPort("ovId"))
dut.get_cthread("vertex_fetcher").add_port(EnqueuePort("prVertexFetcherToApplyQ"))
dut.get_cthread("vertex_fetcher").add_port(EnqueuePort("vtxIdFetchToGatherQ"))
dut.get_cthread("vertex_fetcher").add_port(EnqueuePort("gatherQ"))

dut.get_cthread("gather_proc").add_port(RdRespPort("ovId"))
dut.get_cthread("gather_proc").add_port(RdReqPort("ovpr"))
dut.get_cthread("gather_proc").add_port(DequeuePort("gatherQ"))
dut.get_cthread("gather_proc").add_port(EnqueuePort("applyQ"))

dut.get_cthread("apply_proc").add_port(RdRespPort("ovpr"))

dut.get_cthread("apply_proc2").add_port(DequeuePort("applyQ"))
dut.get_cthread("apply_proc2").add_port(EnqueuePort("commitQ"))
dut.get_cthread("apply_proc2").add_port(EnqueuePort("writePrQ"))

dut.get_cthread("commit_proc").add_port(DequeuePort("commitQ"))
dut.get_cthread("commit_proc").add_port(DequeuePort("writePrQ"))
dut.get_cthread("commit_proc").add_port(WrReqPort("prWrite"))
dut.get_cthread("commit_proc").add_port(WrDataPort("prWrite"))

dut.semantic()

if __name__ == "__main__":
    dut.dump_dot(dut.nm+".dot")
