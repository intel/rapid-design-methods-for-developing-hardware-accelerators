// See LICENSE for license details.

#include "AcclApp.h"
typedef unsigned long long UInt64;
#include "Config.h"
#include <cassert>

#include <string.h>
#include <iostream>

void AcclApp::compute( const void *config_void_ptr, const unsigned int config_size) {
  const Config &config = *static_cast<const Config *>(config_void_ptr);

  const UCacheLine* gi_ptr = config.getGiPtr();
  const CacheLine* wi_ptr = config.getWiPtr();
  const UCacheLine* mi_ptr = config.getMiPtr();
  UCacheLine* mo_ptr = config.getMoPtr();

  const unsigned int maxCLperRow = 3;
  const unsigned int log2ElementsPerCL = 5;
  const unsigned int elementsPerCL = 1<<log2ElementsPerCL;
  const unsigned int maxN = maxCLperRow<<log2ElementsPerCL;

  const unsigned int nSlices = config.get_nSlices();
  const unsigned int nCLperRow = config.get_nCLperRow();
  const unsigned int n = nCLperRow<<log2ElementsPerCL;
    
  unsigned short gamma[maxN];
  for( unsigned int jj=0; jj<nCLperRow; ++jj) {
      for( unsigned int j=0; j<elementsPerCL; ++j) {            
          gamma[(jj<<log2ElementsPerCL)+j] = gi_ptr->words[j];
      }
      ++gi_ptr;
  }
  /*
  for( unsigned int i=0; i<n; ++i) {
      std::cout << "gamma[" << i << "]: " << gamma[i] << std::endl;
  }
  */

  CacheLine weights[maxN][maxCLperRow];
  for( unsigned int i=0; i<n; ++i) {            
      for( unsigned int jj=0; jj<nCLperRow; ++jj) {
          weights[i][jj] = *wi_ptr++;
      }
  }

  UCacheLine q[maxN][maxCLperRow];

  {
    unsigned int k = 0;
    for ( unsigned int t=0; t<n; ++t) {
      unsigned int k0 = k;
      for ( unsigned int s=t+1; s<n; ++s) {
          /*
          std::cout << "t,s,n,k,nSlices: "
                    << t << ","
                    << s << ","
                    << n << ","
                    << k << ","
                    << nSlices << std::endl;
          */

        for( unsigned int jj=0; jj<nCLperRow; ++jj) {
          q[k%n][jj] = *mi_ptr; // delayed values

          for( unsigned int j=0; j<elementsPerCL; ++j) {            
            weights[t][jj].words[j] += mi_ptr->words[j];
          }
          ++mi_ptr;
        }
        ++k;
      }
      for ( unsigned int s=t+1; s<n; ++s) {
        for( unsigned int jj=0; jj<nCLperRow; ++jj) {
          CacheLine cl;  
          CacheLine cl_f;  
          for( unsigned int j=0; j<elementsPerCL; ++j) {            
            cl.words[j] = gamma[s]*weights[t][jj].words[j] - q[k0%n][jj].words[j];
            cl_f = cl; // replace with function eventually
            weights[s][jj].words[j] += cl_f.words[j];
            mo_ptr->words[j] = cl_f.words[j];
          }
          ++mo_ptr;
        }
        ++k0;
      }
      assert( k == k0);
    }
    assert( k == n*(n-1)/2);

  }

  std::cout << "gi reads: " << gi_ptr - config.getGiPtr() << std::endl;

  std::cout << "wi reads: " << wi_ptr - config.getWiPtr() << std::endl;
  std::cout << "mi reads: " << mi_ptr - config.getMiPtr() << std::endl;
  std::cout << "mo writes: " << mo_ptr - config.getMoPtr() << std::endl;



}
