// See LICENSE for license details.
/*[[[cog
     import cog
     from cog_acctempl import *
     from dut_params import dut
     if "thread_nm" not in globals():
       lst = cog.previous.lstrip('/').rstrip('\n').split('=')
       assert( lst[0]=="thread_nm")
       assert( len(lst)==2)
       global thread_nm
       thread_nm = lst[1]
     cog.outl( "//thread_nm=" + thread_nm)
  ]]]*/
//thread_nm=fetcher
//[[[end]]] (checksum: 703d107d9ea4388deb0005904fbed4ec)


// Declare helper methods and class variables

/*[[[cog
     c = dut.get_cthread(thread_nm)
     cog.outl("void %s() {" % (c.nm,))
     for p in c.ports:
       cog.outl("  %s;" % p.reset)
  ]]]*/
void fetcher() {
  inpRespIn.reset_get();
  outDataOut.reset_put();
  lstReqOut.reset_put();
  lstRespIn.reset_get();
//[[[end]]] (checksum: e8e65c26332602c2b85772b04206f0d5)

  unsigned long long ip = 0;
  unsigned int slack = __lst_Slots__;

  // Declare and initialize local variables

  /*[[[cog
       if c.writes_to_done:
         cog.outl("done = false;")
    ]]]*/
  done = false;
  //[[[end]]] (checksum: 872c87e47f55883a9563054415885245)
  wait();
  while (1) {
    if ( start) {
      if ( ip != config.read().get_m()) {

        if ( slack > 0 && inpRespIn.nb_can_get()) {
          MemTypedReadRespType<HeadPtr> resp;
          inpRespIn.nb_get( resp);

          slack -= 1;

          State state;

          state.head = resp.data;

          state.fast = state.head;
          state.slow = state.head;
 
#ifdef DEBUG
          std::cout << "head: " << state.head.head << std::endl;
#endif

          if ( state.fast.head) {
            state.state = 1;
            lstReqOut.put( MemSingleReadReqType<Node,State>( state.fast.head, state));
          } else {
#ifdef DEBUG
            std::cout << "found: " << 0 << std::endl;
#endif
            outDataOut.put( MemTypedWriteDataType<HeadPtr>( state.head));
            slack += 1;
            ++ip;
          }
        } else if ( lstRespIn.nb_can_get()) {
          MemSingleReadRespType<Node,State> resp;
          lstRespIn.nb_get( resp);
          State state = resp.utag;

#ifdef DEBUG
          std::cout << "head,fast,slow,state: " << state.head.head << "," << state.fast.head << "," << state.slow.head << "," << (int) state.state << std::endl;
#endif

          if ( state.state == 1) {
            state.fast.head = resp.data.next;

            if ( !state.fast.head) {
#ifdef DEBUG
              std::cout << "found: " << 0 << std::endl;
#endif
              outDataOut.put( MemTypedWriteDataType<HeadPtr>( state.head));
              slack += 1;
              ++ip;
            } else if ( state.fast.head == state.slow.head) {
#ifdef DEBUG
              std::cout << "found: " << 1 << std::endl;
#endif
              state.head.set_found( 1);
              outDataOut.put( MemTypedWriteDataType<HeadPtr>( state.head));
              slack += 1;
              ++ip;
            } else {
              state.state = 2;
              lstReqOut.put( MemSingleReadReqType<Node,State>( state.slow.head, state));
            }
          } else if ( state.state == 2) {
            state.slow.head = resp.data.next;
            state.state = 3;
            lstReqOut.put( MemSingleReadReqType<Node,State>( state.fast.head, state));
          } else if ( state.state == 3) {
            state.fast.head = resp.data.next;
            if ( state.fast.head) {
              state.state = 1;
              lstReqOut.put( MemSingleReadReqType<Node,State>( state.fast.head, state));
            } else {
#ifdef DEBUG
              std::cout << "found: " << 0 << std::endl;
#endif
              outDataOut.put( MemTypedWriteDataType<HeadPtr>( state.head));
              slack += 1;
              ++ip;
            }
          } else {
            assert(0);
          }
        }
      } else {
        done = true;
      }
    }
    wait();
  }
}
