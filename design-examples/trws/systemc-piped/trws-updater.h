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
//thread_nm=updater
//[[[end]]] (checksum: 2de9f18bb41f4f58b244dcb53e4cf258)

// Declare helper methods and class variables

/*[[[cog
     c = dut.get_cthread(thread_nm)
     cog.outl("void %s() {" % (c.nm,))
     for p in c.ports:
       cog.outl("  %s; // type: %s" % (p.reset,p.type(dut)))
  ]]]*/
void updater() {
  giRespIn.reset_get(); // type: MemTypedReadRespType<UCacheLine>
  wiRespIn.reset_get(); // type: MemTypedReadRespType<CacheLine>
  miRespIn.reset_get(); // type: MemTypedReadRespType<UCacheLine>
  res.reset_get(); // type: UCacheLine
  off.reset_put(); // type: CacheLine
  moDataOut.reset_put(); // type: MemTypedWriteDataType<UCacheLine>
//[[[end]]] (checksum: 85161d0142877497f71bffaa71bddad7)

  unsigned int phase = 0;



  bool qReadValid = false;
  UCacheLine qReadBuf;
  bool w0ReadValid = false;
  CacheLine w0ReadBuf;
  bool w1ReadValid = false;
  CacheLine w1ReadBuf;
  CacheLine w1ReadBuf2;
  bool w2ReadValid = false;
  CacheLine w2ReadBuf;

  unsigned int i = 0;
  unsigned int jj = 0; 

  SliceCounter sc0, sc1, sc2;

#if 0
  UInt1 outstandingS[maxN];
  const UInt1 maxOutstanding = 1; // need to store 0,1 => UInt1
#else
  UInt2 outstandingS[maxN];
  const UInt2 maxOutstanding = 2; // need to store 0,1,2 => UInt2
#endif

  for( unsigned int i=0; i<maxN; ++i) {
    outstandingS[i] = 0;
  }


  unsigned int phase2Clocks = 0;
  unsigned int stalls = 0;

  /*[[[cog
       if c.writes_to_done:
         cog.outl("done = false;")
    ]]]*/
  done = false;
  //[[[end]]] (checksum: 872c87e47f55883a9563054415885245)
  wait();
  while (1) {
    if ( start && !done) {
      const unsigned int nSlices = config.read().get_nSlices();
      const unsigned int nCLperRow = config.read().get_nCLperRow();
      const unsigned int n = nCLperRow<<log2ElementsPerCL;

      //      std::cout << "sc0,sc1,sc2: " << sc0 << "," << sc1 << "," << sc2 << std::endl;

      if ( phase == 0) {
        if ( giRespIn.nb_can_get()) {
          MemTypedReadRespType<UCacheLine> wrappedCL;
          giRespIn.nb_get( wrappedCL);
          for( unsigned int j=0; j<elementsPerCL; ++j) {            
            gamma[(jj<<log2ElementsPerCL)+j] = wrappedCL.data.words[j];
          }
          if ( jj != nCLperRow-1) {
            ++jj;
          } else {
            jj = 0;
            phase = 1;
          }
        }
      } else if ( phase == 1) {
        //        std::cout << "phase,i,jj: " << phase << "," << i << "," << jj << std::endl;
        if ( wiRespIn.nb_can_get()) {
          MemTypedReadRespType<CacheLine> wrappedCL;
          wiRespIn.nb_get( wrappedCL);
          weights[i][jj] = wrappedCL.data;
          weightsCopy[i][jj] = wrappedCL.data;
          for( unsigned int j=0; j<elementsPerCL; ++j) {
            weights2[i][jj].words[j] = 0;
            weights2Copy[i][jj].words[j] = 0;
          }
          if ( jj != nCLperRow-1) {
            ++jj;
          } else {
            jj = 0;
            if ( i != n-1) {
              ++i;
            } else {
              i = 0;
              phase = 2;
            }
          }
        }
      } else if ( phase == 2) {
        ++phase2Clocks;

        if ( w0ReadValid && !sc0.done && miRespIn.nb_can_get() && (sc0.k-sc1.k) < n) {
          MemTypedReadRespType<UCacheLine> wrappedCL;
          miRespIn.nb_get( wrappedCL);
          q[sc0.k%maxN][sc0.jj] = wrappedCL.data; // delayed values

          for( unsigned int j=0; j<elementsPerCL; ++j) {            
            w0ReadBuf.words[j] += wrappedCL.data.words[j];
          } 
          weights[sc0.t][sc0.jj] = w0ReadBuf;
          weightsCopy[sc0.t][sc0.jj] = w0ReadBuf;
          if ( sc0.t == sc1.t && sc0.jj == sc1.jj) {
            w1ReadBuf = w0ReadBuf;
          }
          sc0.incr( nCLperRow, n);
        }

        w0ReadValid = true;
        w0ReadBuf = weights[sc0.t][sc0.jj];

        if ( !sc1.done && off.nb_can_put() && (sc0.done || sc0.t > sc1.t)) {
          if ( outstandingS[sc1.t] > 0) {
            //            std::cout << "Stalling due to outstanding count on " << sc1.t << ": " << 0+outstandingS[sc1.t] << std::endl;
            ++stalls;
          }
        }

        if ( qReadValid && w1ReadValid
             && !sc1.done && off.nb_can_put() && (sc0.done || sc0.t > sc1.t) && outstandingS[sc1.t] == 0
             && ( sc1.jj != 0 || outstandingS[sc1.s] < maxOutstanding) /* boolean 117,14137 two 117,14109 */
             ) {
          if ( sc1.jj == 0) {
            ++outstandingS[sc1.s];
            //            std::cout << "incrementing " << sc1.s << " to " << 0+outstandingS[sc1.s] << std::endl;
            assert( outstandingS[sc1.s] <= maxOutstanding);
          }
          CacheLine ocl;
          for( unsigned int j=0; j<elementsPerCL; ++j) {            
            ocl.words[j] = gamma[sc1.s]*(w1ReadBuf.words[j]+w1ReadBuf2.words[j]) - qReadBuf.words[j];
          }
          off.nb_put( ocl);
          sc1.incr( nCLperRow, n);
        }

        qReadValid = true;
        qReadBuf = q[sc1.k%maxN][sc1.jj];          

        w1ReadValid = true;
        w1ReadBuf = weightsCopy[sc1.t][sc1.jj];
        w1ReadBuf2 = weights2Copy[sc1.t][sc1.jj];

        if ( w2ReadValid && res.nb_can_get() && moDataOut.nb_can_put()) {
          if ( sc2.jj == nCLperRow-1) {
            --outstandingS[sc2.s];
            //            std::cout << "decrementing " << sc2.s << " to " << 0+outstandingS[sc2.s] << std::endl;
            assert( outstandingS[sc2.s] >= 0);
          }
          UCacheLine cl;
          res.nb_get( cl);

          for( unsigned int j=0; j<elementsPerCL; ++j) {            
            w2ReadBuf.words[j] += cl.words[j];
          }
          weights2[sc2.s][sc2.jj] = w2ReadBuf;
          weights2Copy[sc2.s][sc2.jj] = w2ReadBuf;
          if ( sc2.s == sc1.t && sc2.jj == sc1.jj) {
            w1ReadBuf2 = w2ReadBuf;
          }
          moDataOut.nb_put( MemTypedWriteDataType<UCacheLine>( cl));
          sc2.incr( nCLperRow, n);
          if ( sc2.done) {
              std::cout << "stalls,phase2Clocks: " << stalls << "," << phase2Clocks << " " << (double) stalls/phase2Clocks << std::endl;
              done = true;
          }
        }

        w2ReadValid = true;
        w2ReadBuf = weights2[sc2.s][sc2.jj];

      }
    }
    wait();
  }
}
