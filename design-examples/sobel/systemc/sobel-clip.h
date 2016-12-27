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
//thread_nm=clip
//[[[end]]] (checksum: 23f36d0705a6b3887922d47f4c5a7b21)

// Declare helper methods and class variables

/*[[[cog
     c = dut.get_cthread(thread_nm)
     cog.outl("void %s() {" % (c.nm,))
     for p in c.ports:
       cog.outl("  %s; // type: %s" % (p.reset,p.type(dut)))
  ]]]*/
void clip() {
  outDataOut.reset_put(); // type: MemTypedWriteDataType<BlkOut>
  mid1.reset_get(); // type: BlkMid
//[[[end]]] (checksum: f1cb3dd49098083d7ffa559a941cf152)

  Int16 ip = 0;
  Int16 jc = 0;

  /*[[[cog
       if c.writes_to_done:
         cog.outl("done = false;")
    ]]]*/
  done = false;
  //[[[end]]] (checksum: 872c87e47f55883a9563054415885245)
  wait();
  while (1) {
    if ( start) {

      Int16 ni = config.read().get_num_of_rows();
      Int16 bpr = config.read().get_row_size_in_blks();
      const unsigned int words_per_blk = 64;

      if ( mid1.nb_can_get()) {
        
        BlkMid icl;
        mid1.nb_get(icl);

        BlkOut ocl;
        for( unsigned int k=0; k<words_per_blk; ++k) {
          char r;
          if ( icl.data[k] < -128) {
            r = -128;
          } else if ( icl.data[k] > 127) {
            r = 127;
          } else {
            r = icl.data[k];
          }
          ocl.data[k] = r;
        }

        outDataOut.put(MemTypedWriteDataType<BlkOut>(ocl));

        ++jc;
        if ( jc >= bpr) {
          jc = 0;
          ++ip;
          if ( ip == ni) {
            done = true;
            ip = 0;
          }
        }
      }

    }
    wait();
  }
}
