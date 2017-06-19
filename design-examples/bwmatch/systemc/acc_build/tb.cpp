// See LICENSE for license details.

#include <string.h>

#include "gtest/gtest.h"
#include "hld_alloc.h"

#if defined(__AAL_USER__) || defined(USE_SOFTWARE)
#include "AcclApp.h"
typedef unsigned long long UInt64;
#include "Config.h"
#ifndef __BWCacheLineLoadParamsSlots__
#define __BWCacheLineLoadParamsSlots__ 96
#endif
#ifndef NUM_AUS
#define NUM_AUS 1
#endif
#else
#if defined(KERNEL_TEST)
#include "bwmatch_hls_tb.h"
#else
#include "bwmatch_acc_tb.h"
#endif
#endif

#include "ConfigOld.h"

#if defined(__AAL_USER__)
#define PATH_PREFIX "../../data/"
#else
#define PATH_PREFIX "../data/"
#endif

const char *cl_fn  = PATH_PREFIX "X.bin";
const char *pat_fn = PATH_PREFIX "Y.bin";
const char *res_fn = PATH_PREFIX "Z.bin";
const char *cfg_fn = PATH_PREFIX "W.bin";
const char *pre_fn = PATH_PREFIX "U.bin";

#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>

off_t file_size_in_bytes( const char *fn)
{
  int fd = open( fn, O_RDONLY);
  assert( fd != -1);
  
  off_t result = lseek( fd, 0, SEEK_END);

  int rc = close( fd);
  assert( rc == 0);

  return result;
}

TEST(AccelTest, SimpleTest) {

#if defined(KERNEL_TEST)
  AcclApp theApp("theApp");
#else
  AcclApp theApp("ffff");
#endif

  unsigned int ni = file_size_in_bytes( pat_fn) / 8;

  unsigned long long sz = 2*1024ULL*1024ULL*1024ULL;

  // make sure ni is a multiple of 32
  // ni = (ni >> 5) << 5;
  // really make sure ni is a multiple of 8*NUM_AUS
  ni = (ni / (8*NUM_AUS)) * 8 * NUM_AUS;

  std::cout << "ni = " << ni << std::endl;

  int precomp_len = 0;
#ifndef DISABLE_PRECOMP_LEN
  if ( access( pre_fn, R_OK) != -1 ) {
    off_t precomp_size = file_size_in_bytes( pre_fn) / 8;
    int k;
    for ( k=0; k<16; ++k) {
      if ( precomp_size == (1U << (2*k))) {
        precomp_len = k;
        break;
      }
    }
    assert( k != 16);
    std::cout << "precomp_size: " << precomp_size << " precomp_len: " << precomp_len << std::endl;
  }
#endif

  if ( theApp.alloc( sz)) {
    unsigned char *WORKSPACE = theApp.m_JointVirt;
    size_t WORKSPACE_SIZE    = theApp.m_JointSize;

    hld_alloc altor((char *)WORKSPACE, WORKSPACE_SIZE);

    std::vector<BWResult> ref_res( ni);

    std::cout << "Reading " << cfg_fn << ", " << cl_fn << ", " << pat_fn << " and " << res_fn << "." << std::endl;

    Config config;

#if 1
    ConfigOld configold;

    std::cout << "config fn: " << cfg_fn << std::endl;
    {
      FILE *ifp = fopen( cfg_fn, "rb");
      assert( ifp);
      size_t records = fread( &configold, sizeof(ConfigOld), 1, ifp);
      assert( records == 1);
      int rc = fclose(ifp);
      assert( rc==0);
    }

    std::cout << "configold: " << configold << std::endl;

    std::cout << "sizeof(ConfigOld): " << sizeof(ConfigOld) << std::endl;

    std::cout << "u3: " << configold.get_u3() << std::endl;

    config.set_end_pos( configold.get_end_pos());
    config.set_u0( configold.get_u0());
    config.set_u1( configold.get_u1());
    config.set_u2( configold.get_u2());
    config.set_u3( configold.get_u3());

    std::cout << "m: " << config.m() << " n: " << config.n() << std::endl;

    {
      FILE *ofp = fopen( (std::string(cfg_fn) + std::string("-new2")).c_str(), "wb");
      assert( ofp);
      size_t records = fwrite( &config, sizeof(Config), 1, ofp);
      assert( 1 == records);
      int rc = fclose(ofp);
      assert( rc == 0);
    }

    std::cout << "sizeof(Config): " << sizeof(Config) << std::endl;

#endif

    {
      FILE *ifp = fopen( (std::string(cfg_fn) + std::string("-new2")).c_str(), "rb");
      assert( ifp);
      size_t records = fread( &config, sizeof(Config), 1, ifp);
      assert( 1 == records);
      int rc = fclose(ifp);
      assert( rc == 0);
    }

    config.set_precomp_len( precomp_len);

    BWCacheLine *cl_ptr  = altor.allocate<BWCacheLine>( config.m());
    BWResult    *pre_ptr = altor.allocate<BWResult>( 1<<(2*config.get_precomp_len()));
    BWPattern   *pat_ptr = altor.allocate<BWPattern>( ni);
    BWResult    *res_ptr = altor.allocate<BWResult>( ni);


    std::cout << "WORKSPACE:        " << "0x" << std::setfill('0') << std::setw(16) << std::hex << (AddrType) WORKSPACE << std::endl;
    std::cout << "cl_ptr-WORKSPACE: " << "0x" << std::setfill('0') << std::setw(16) << std::hex << (AddrType) cl_ptr - (AddrType) WORKSPACE << std::endl;
    std::cout << "cl_ptr:           " << "0x" << std::setfill('0') << std::setw(16) << std::hex << (AddrType) cl_ptr << std::endl;
    std::cout << "pre_ptr-cl_ptr:   " << "0x" << std::setfill('0') << std::setw(16) << std::hex << (AddrType) pre_ptr - (AddrType) cl_ptr << std::endl;
    std::cout << "pre_ptr:          " << "0x" << std::setfill('0') << std::setw(16) << std::hex << (AddrType) pre_ptr << std::endl;
    std::cout << "pat_ptr-pre_ptr:  " << "0x" << std::setfill('0') << std::setw(16) << std::hex << (AddrType) pat_ptr - (AddrType) pre_ptr << std::endl;
    std::cout << "pat_ptr:          " << "0x" << std::setfill('0') << std::setw(16) << std::hex << (AddrType) pat_ptr << std::endl;
    std::cout << "res_ptr-pat_ptr:  " << "0x" << std::setfill('0') << std::setw(16) << std::hex << (AddrType) res_ptr - (AddrType) pat_ptr << std::endl;
    std::cout << "res_ptr:          " << "0x" << std::setfill('0') << std::setw(16) << std::hex << (AddrType) res_ptr << std::endl;

    std::cout << std::dec;

    assert( (AddrType) pre_ptr >  (AddrType) cl_ptr);
    assert( (AddrType) pat_ptr >= (AddrType) pre_ptr);
    assert( (AddrType) res_ptr >  (AddrType) pat_ptr);

    assert( ((AddrType) pre_ptr - (AddrType) cl_ptr) >= config.m()*sizeof(BWCacheLine));
    assert( ((AddrType) pat_ptr - (AddrType) pre_ptr) >= (1<<(2*config.get_precomp_len()))*sizeof(BWResult));
    assert( ((AddrType) res_ptr - (AddrType) pat_ptr) >= ni*sizeof(BWPattern));

    config.set_aVD( (AddrType) cl_ptr);

    config.set_aCl( (AddrType) cl_ptr); 
    config.set_aPre( (AddrType) pre_ptr); 

    std::cout << "set_oPat: " << "0x" << std::setfill('0') << std::setw(16) << std::hex << (AddrType) pat_ptr - (AddrType) cl_ptr << std::endl;
    std::cout << "set_oRes: " << "0x" << std::setfill('0') << std::setw(16) << std::hex << (AddrType) res_ptr - (AddrType) cl_ptr << std::endl;
    std::cout << std::dec;

    config.set_aPat( (AddrType) pat_ptr);
    config.set_aRes( (AddrType) res_ptr);
    config.set_nPat( ni);
    config.set_max_recirculating( __BWCacheLineLoadParamsSlots__);
    //config.set_max_recirculating( 48);

    std::cout << "aPre: " << config.get_aPre() << std::endl;
    std::cout << "aPat: " << config.get_aPat() << std::endl;
    std::cout << "aRes: " << config.get_aRes() << std::endl;
    std::cout << "nPat: " << config.get_nPat() << std::endl;
    std::cout << "max_recirculating: " << config.get_max_recirculating() << std::endl;
    std::cout << "precomp_len: " << config.get_precomp_len() << std::endl;
    std::cout << config;

    assert( config.get_aPre() >= config.get_aCl());
    assert( config.get_aPat() >= config.get_aPre());
    assert( config.get_aRes() >= config.get_aPat());

    assert( config.get_aCl() % 64 == 0);
    assert( config.get_aPre() % 64 == 0);
    assert( config.get_aPat() % 64 == 0);
    assert( config.get_aRes() % 64 == 0);

    std::cout << "getPatAddr(0): " << "0x" << std::setfill('0') << std::setw(16) << std::hex << (AddrType) config.getPatAddr( 0) << std::endl;
    std::cout << "getResAddr(0): " << "0x" << std::setfill('0') << std::setw(16) << std::hex << (AddrType) config.getResAddr( 0) << std::endl;
    std::cout << std::dec;

    {
      FILE *ifp = fopen( cl_fn, "rb");
      for (unsigned int i=0; i<config.m(); ++i) {
        size_t records = fread( &cl_ptr[i], sizeof(BWCacheLine), 1, ifp);
        assert( 1 == records);
      }
      int rc = fclose(ifp);
      assert( rc==0);
    }

    {
      FILE *ifp = fopen( pat_fn, "rb");
      assert( ifp);
      for (unsigned int ip=0; ip<ni; ++ip) {
        size_t records = fread( &pat_ptr[ip], sizeof(BWPattern), 1, ifp);
        assert( 1 == records);
      }
      int rc = fclose(ifp);
      assert( rc==0);
    }

    {
      FILE *ifp = fopen( res_fn, "rb");
      assert( ifp);
      for (unsigned int ip=0; ip<ni; ++ip) {
        size_t records = fread( &ref_res[ip], sizeof(BWResult), 1, ifp);
        assert( 1 == records);
      }
      int rc = fclose(ifp);
      assert( rc==0);
    }

#ifndef DISABLE_PRECOMP_LEN
    if ( config.get_precomp_len() > 0) {
      FILE *ifp = fopen( pre_fn, "rb");
      unsigned int p = 1U<<(2*config.get_precomp_len());
      assert( ifp);
      for (unsigned int ip=0; ip<p; ++ip) {
        size_t records = fread( &pre_ptr[ip], sizeof(BWResult), 1, ifp);
        assert( 1 == records);
      }
      fclose(ifp);
    }
#endif

    std::cout << "Finished reading files." << std::endl;

    // Fill in a specific unlikely to be used value
    {
      for( unsigned int i=0; i<ni; ++i) {
        res_ptr[i].l = 0xffffffff;
        res_ptr[i].u = 0xffffffff;
      }
    }

    theApp.compute( &config, sizeof( config));
    theApp.join();

    // check
    unsigned int correct = 0;
    unsigned int uninitialized = 0;
    for (unsigned int ip=0; ip<ni; ++ip) {
      if ( res_ptr[ip] == ref_res[ip]) {
        ++correct;
      } else if ( (res_ptr[ip].l == 0xffffffff) ||
                  (res_ptr[ip].u == 0xffffffff)) {
        ++uninitialized;
        std::cout << res_ptr[ip] << " " << ref_res[ip] << " " << ip << std::endl;
      } else if ( (res_ptr[ip].l == res_ptr[ip].u) &&
                  (ref_res[ip].l == ref_res[ip].u)) {
        ++correct;
      } else {
        std::cout << res_ptr[ip] << " " << ref_res[ip] << " " << ip << std::endl;
      }
    }
    std::cout << "Results checked. " << correct
              << " of " << ni << " correct." << std::endl;

    EXPECT_EQ( correct, ni);
    EXPECT_EQ( uninitialized, 0);

    theApp.free();
  } else {
    ASSERT_TRUE( 0);
  }
}

#if defined(__AAL_USER__) || defined(USE_SOFTWARE)
int main (int argc, char *argv[]) {
#else
int sc_main (int argc, char *argv[]) {
  sc_report_handler::set_actions("/IEEE_Std_1666/deprecated", SC_DO_NOTHING);
#endif
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}


