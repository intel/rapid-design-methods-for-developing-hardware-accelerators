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
//thread_nm=precomp_fill
//[[[end]]] (checksum: c81dc7aaa3d836f15bbd033f1330ab99)

// Declare helper methods and class variables

/*[[[cog
     c = dut.get_cthread(thread_nm)
     cog.outl("void %s() {" % (c.nm,))
     for p in c.ports:
       cog.outl("  %s; // type: %s" % (p.reset,p.type(dut)))
  ]]]*/
void precomp_fill() {
  preRespIn.reset_get(); // type: MemTypedReadRespType<BWResult>
//[[[end]]] (checksum: b7770b7ad260dd52bd454ff97815f73c)

    UInt16 precomp_i = 0;

    phase.write( 0);

  /*[[[cog
       if c.writes_to_done:
         cog.outl("done = false;")
    ]]]*/
  //[[[end]]] (checksum: d41d8cd98f00b204e9800998ecf8427e)
  wait();
  while (1) {
    // std::cout << "precomp_fill: phase,precomp_i: " << phase.read() << " " << precomp_i << std::endl;
    if ( start && phase.read() == 0) {
      UInt16 precomp_size = 1U<<(2*config.read().get_precomp_len());

      precomp_res[precomp_i] = preRespIn.get().data;

      ++precomp_i;
      if ( precomp_i < precomp_size) {
      } else {
        phase.write(1);
      }
    }
    wait();
  }
}
