// See LICENSE for license details.
// Copyright(c) 2007-2016, Intel Corporation
//
// Redistribution  and  use  in source  and  binary  forms,  with  or  without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of  source code  must retain the  above copyright notice,
//   this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
// * Neither the name  of Intel Corporation  nor the names of its contributors
//   may be used to  endorse or promote  products derived  from this  software
//   without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,  BUT NOT LIMITED TO,  THE
// IMPLIED WARRANTIES OF  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED.  IN NO EVENT  SHALL THE COPYRIGHT OWNER  OR CONTRIBUTORS BE
// LIABLE  FOR  ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR
// CONSEQUENTIAL  DAMAGES  (INCLUDING,  BUT  NOT LIMITED  TO,  PROCUREMENT  OF
// SUBSTITUTE GOODS OR SERVICES;  LOSS OF USE,  DATA, OR PROFITS;  OR BUSINESS
// INTERRUPTION)  HOWEVER CAUSED  AND ON ANY THEORY  OF LIABILITY,  WHETHER IN
// CONTRACT,  STRICT LIABILITY,  OR TORT  (INCLUDING NEGLIGENCE  OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,  EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//****************************************************************************
#include "AcclApp.h"

AcclApp::AcclApp(const char *auid) :
   m_afuId(auid),
   m_Runtime(this),
   m_pAALService(NULL),
   m_pALIBufferService(NULL),
   m_pALIMMIOService(NULL),
   m_pALIResetService(NULL),
   m_Result(0),
   m_pVTP_AALService(NULL),
   m_pVTPService(NULL),
   m_VTPDFHOffset(-1),
   m_DSMVirt(NULL),
   m_DSMSize(0),
   m_JointVirt(NULL),
   m_JointSize(0)
{
   // Register our Client side interfaces so that the Service can acquire them.
   //   SetInterface() is inherited from CAASBase
   SetInterface(iidServiceClient, dynamic_cast<IServiceClient *>(this));
   SetInterface(iidRuntimeClient, dynamic_cast<IRuntimeClient *>(this));

   // Initialize our internal semaphore
   m_Sem.Create(0, 1);

   // Start the AAL Runtime, setting any startup options via a NamedValueSet

   // Using Hardware Services requires the Remote Resource Manager Broker Service
   //  Note that this could also be accomplished by setting the environment variable
   //   AALRUNTIME_CONFIG_BROKER_SERVICE to librrmbroker
   NamedValueSet configArgs;
   NamedValueSet configRecord;

#if defined( HWAFU )
   // Specify that the remote resource manager is to be used.
   configRecord.Add(AALRUNTIME_CONFIG_BROKER_SERVICE, "librrmbroker");
   configArgs.Add(AALRUNTIME_CONFIG_RECORD, &configRecord);
#endif

   // Start the Runtime and wait for the callback by sitting on the semaphore.
   //   the runtimeStarted() or runtimeStartFailed() callbacks should set m_bIsOK appropriately.
   if(!m_Runtime.start(configArgs)){
	   m_bIsOK = false;
      return;
   }
   m_Sem.Wait();
   m_bIsOK = true;
}

/// @brief   Destructor
///
AcclApp::~AcclApp()
{
   m_Sem.Destroy();
}

void *AcclApp::alloc( unsigned long long size_in_bytes)
{
   cout <<"========================"<<endl;
   cout <<"= Hello ALI NLB Sample ="<<endl;
   cout <<"========================"<<endl;

   // Request the Servcie we are interested in.

   // NOTE: This example is bypassing the Resource Manager's configuration record lookup
   //  mechanism.  Since the Resource Manager Implementation is a sample, it is subject to change.
   //  This example does illustrate the utility of having different implementations of a service all
   //  readily available and bound at run-time.
   NamedValueSet Manifest;
   NamedValueSet ConfigRecord;

#if defined( HWAFU )                /* Use FPGA hardware */
   // Service Library to use
   ConfigRecord.Add(AAL_FACTORY_CREATE_CONFIGRECORD_FULL_SERVICE_NAME, "libHWALIAFU");

   // the AFUID to be passed to the Resource Manager. It will be used to locate the appropriate device.
   //ConfigRecord.Add(keyRegAFU_ID,"C000C966-0D82-4272-9AEF-FE5F84570612");
   ConfigRecord.Add(keyRegAFU_ID, m_afuId);


   // indicate that this service needs to allocate an AIAService, too to talk to the HW
   ConfigRecord.Add(AAL_FACTORY_CREATE_CONFIGRECORD_FULL_AIA_NAME, "libaia");

   #elif defined ( ASEAFU )         /* Use ASE based RTL simulation */
   Manifest.Add(keyRegHandle, 20);

   ConfigRecord.Add(AAL_FACTORY_CREATE_CONFIGRECORD_FULL_SERVICE_NAME, "libASEALIAFU");
   ConfigRecord.Add(AAL_FACTORY_CREATE_SOFTWARE_SERVICE,true);

   #else                            /* default is Software Simulator */
#if 0 // NOT CURRRENTLY SUPPORTED
   ConfigRecord.Add(AAL_FACTORY_CREATE_CONFIGRECORD_FULL_SERVICE_NAME, "libSWSimALIAFU");
   ConfigRecord.Add(AAL_FACTORY_CREATE_SOFTWARE_SERVICE,true);
#endif
   return NULL;
#endif

   // Add the Config Record to the Manifest describing what we want to allocate
   Manifest.Add(AAL_FACTORY_CREATE_CONFIGRECORD_INCLUDED, &ConfigRecord);

   // in future, everything could be figured out by just giving the service name
   Manifest.Add(AAL_FACTORY_CREATE_SERVICENAME, "Hello ALI NLB");

   MSG("Allocating Service");

   // Allocate the Service and wait for it to complete by sitting on the
   //   semaphore. The serviceAllocated() callback will be called if successful.
   //   If allocation fails the serviceAllocateFailed() should set m_bIsOK appropriately.
   //   (Refer to the serviceAllocated() callback to see how the Service's interfaces
   //    are collected.)
   m_Runtime.allocService(dynamic_cast<IBase *>(this), Manifest, m_AcclAppTranID);
   m_Sem.Wait();
   if(!m_bIsOK){
      ERR("Allocation failed\n");
      goto done_0;
   }

// VTP service

   // Reuse Manifest and Configrecord for VTP service
   Manifest.Empty();
   ConfigRecord.Empty();

   // Allocate VTP service
   // Service Library to use
   ConfigRecord.Add(AAL_FACTORY_CREATE_CONFIGRECORD_FULL_SERVICE_NAME, "libMPF");
   ConfigRecord.Add(AAL_FACTORY_CREATE_SOFTWARE_SERVICE,true);

   // Add the Config Record to the Manifest describing what we want to allocate
   Manifest.Add(AAL_FACTORY_CREATE_CONFIGRECORD_INCLUDED, &ConfigRecord);

   // the VTPService will reuse the already established interfaces presented by
   // the ALIAFU service
   Manifest.Add(ALIAFU_IBASE_KEY, static_cast<ALIAFU_IBASE_DATATYPE>(m_pAALService));

   // MPFs feature ID, used to find correct features in DFH list
   Manifest.Add(MPF_FEATURE_ID_KEY, static_cast<MPF_FEATURE_ID_DATATYPE>(1));

   // in future, everything could be figured out by just giving the service name
   Manifest.Add(AAL_FACTORY_CREATE_SERVICENAME, "VTP");

   MSG("Allocating VTP Service");

   m_Runtime.allocService(dynamic_cast<IBase *>(this), Manifest, m_VTPTranID);
   m_Sem.Wait();
   if(!m_bIsOK){
      ERR("VTP Service allocation failed\n");
      goto done_0;
   }

// VTP service DONE

   // Now that we have the Service and have saved the IALIBuffer interface pointer
   //  we can now Allocate the 3 Workspaces used by the NLB algorithm. The buffer allocate
   //  function is synchronous so no need to wait on the semaphore

   // Device Status Memory (DSM) is a structure defined by the NLB implementation.

   // User Virtual address of the pointer is returned directly in the function
   m_DSMSize = LPBK1_DSM_SIZE;
   if( ali_errnumOK != m_pVTPService->bufferAllocate(m_DSMSize, &m_DSMVirt)){
      m_bIsOK = false;
      m_Result = -1;
      goto done_1;
   }

   m_JointSize = size_in_bytes; 
   if( ali_errnumOK !=  m_pVTPService->bufferAllocate(m_JointSize, &m_JointVirt)){
     std::cout << "-E- Failed allocating a buffer of size: " << m_JointSize << std::endl;
      m_bIsOK = false;
      m_Result = -1;
      goto done_2;
   }

   //=============================
   // Now we have the NLB Service
   //   now we can use it
   //=============================
   MSG("Running Test");
   if(true == m_bIsOK){

      // Clear the DSM
      ::memset( m_DSMVirt, 0, m_DSMSize);

      // Initialize the source and destination buffers
      ::memset( m_JointVirt, 0, m_JointSize);    // Output initialized to 0

      // Original code puts DSM Reset prior to AFU Reset, but ccipTest
      //    reverses that. We are following ccipTest here.

      // Initiate AFU Reset
      m_pALIResetService->afuReset();

      // AFU Reset clear VTP, too, so reinitialize hardware
      m_pVTPService->vtpReset();

      // Initiate DSM Reset
      // Set DSM base, high then low
      // Set DSM base, high then low
      m_pALIMMIOService->mmioWrite64(CSR_AFU_DSM_BASEL, (btUnsigned64bitInt)m_DSMVirt);

      return reinterpret_cast<void *>(m_JointVirt);
   }

   // Clean-up and return
done_3:
   m_pALIBufferService->bufferFree(m_JointVirt);
done_2:
   m_pALIBufferService->bufferFree(m_DSMVirt);

done_1:
   // Freed all three so now Release() the VTP Service through the Services IAALService::Release() method
   (dynamic_ptr<IAALService>(iidService, m_pVTP_AALService))->Release(TransactionID());
   m_Sem.Wait();

   // Freed all so now Release() the Service through the Services IAALService::Release() method
   (dynamic_ptr<IAALService>(iidService, m_pAALService))->Release(TransactionID());
   m_Sem.Wait();

done_0:
   m_Runtime.stop();
   m_Sem.Wait();

   return NULL;

}

void AcclApp::compute( const void *config_ptr, const unsigned int config_size) 
{
      ::memset( m_DSMVirt, 0, m_DSMSize);             // clear DSM

      // Assert AFU reset
      m_pALIMMIOService->mmioWrite32(CSR_CTL, 0);

      //De-Assert AFU reset
      m_pALIMMIOService->mmioWrite32(CSR_CTL, 1);

      // If ASE, give it some time to catch up
      /*
      #if defined ( ASEAFU )
      SleepSec(5);
      #endif*/ /* ASE AFU */


      // Write to the accelerator config
      {
        const unsigned int num_of_ulls_in_config = (config_size+7)/8;

        //        static_assert( num_of_ulls_in_config <= 9, "Size of config currently limited to 72 bytes. (This is easy to change in the RTL if necessary.)");
        assert( num_of_ulls_in_config <= 9);

        const unsigned long long *config_as_ull = reinterpret_cast<const unsigned long long *>( config_ptr);
        for (unsigned int i=0; i<num_of_ulls_in_config; ++i) {
          m_pALIMMIOService->mmioWrite64(CSR_GENERIC_CONFIG+i*8, config_as_ull[i]);
        }
      }
      
      csr_type cfg = (csr_type)NLB_TEST_MODE_LPBK1;
      //cfg |= (csr_type)NLB_TEST_MODE_RDI;
      //cfg |= (csr_type)NLB_TEST_MODE_VH0;
      //cfg |= (csr_type)NLB_TEST_MODE_VL0;

      m_pALIMMIOService->mmioWrite32(CSR_CFG, cfg);

      // Set the test mode
      //m_pALIMMIOService->mmioWrite32(CSR_CFG,0);

      // Start the test
      m_pALIMMIOService->mmioWrite32(CSR_CTL, 3);
}

void AcclApp::join() 
{
      volatile bt32bitCSR *StatusAddr = (volatile bt32bitCSR *)
                                         (m_DSMVirt  + DSM_STATUS_TEST_COMPLETE);
      // Wait for test completion
      while( 0 == ((*StatusAddr)&0x1) ) {
         SleepMicro(100);
      }
      MSG("Done Running Test");

      // Stop the device
      m_pALIMMIOService->mmioWrite32(CSR_CTL, 7);

      {
        volatile unsigned long long *q = reinterpret_cast<volatile unsigned long long *>(reinterpret_cast<volatile unsigned char *>(StatusAddr)+8);
        // Forty bits defined
        std::cout << "DSM: Num_ticks: " << (0xffffffffff & *q) << std::endl;
      }

      {
        volatile unsigned int *q = reinterpret_cast<volatile unsigned int *>(reinterpret_cast<volatile unsigned char *>(StatusAddr)+16);
        std::cout << "DSM: Num_Reads: " << *q << std::endl;
      }
      {
        volatile unsigned int *q = reinterpret_cast<volatile unsigned int *>(reinterpret_cast<volatile unsigned char *>(StatusAddr)+20);
        std::cout << "DSM: Num_Writes: " << *q << std::endl;
      }
      
      unsigned int rd_ticks = 0;
      unsigned int wr_ticks = 0;
      m_pALIMMIOService->mmioRead32(CSR_STATUS0, &wr_ticks);
      m_pALIMMIOService->mmioRead32(CSR_STATUS0+4, &rd_ticks);
      std::cout << "CSR: Num rd_ticks: " << rd_ticks << std::endl;
      std::cout << "CSR: Num wr_ticks: " << wr_ticks << std::endl;
      unsigned int afu_idle = 0;
      unsigned int ivp_idle = 0;
      m_pALIMMIOService->mmioRead32(CSR_STATUS1, &afu_idle);
      m_pALIMMIOService->mmioRead32(CSR_STATUS1+4, &ivp_idle);
      std::cout << "CSR: Afu idle cycles: " << afu_idle << std::endl;
      std::cout << "CSR: Ivp idle cycles: " << ivp_idle << std::endl;

      
      t_cci_mpf_vtp_stats stats;
      m_pVTPService->vtpGetStats(&stats);
      cout << " numTLBHits4KB = " <<     stats.numTLBHits4KB << std::endl;
      cout << " numTLBMisses4KB = " <<     stats.numTLBMisses4KB << std::endl;
      cout << " numTLBHits2MB = " <<     stats.numTLBHits2MB << std::endl;
      cout << " numTLBMisses2MB = " <<     stats.numTLBMisses2MB << std::endl;

      cout << " numPTWalkBusyCycles = " <<     stats.numPTWalkBusyCycles << std::endl;
      cout << " numFailedTranslations = " <<     stats.numFailedTranslations << std::endl;








}


void AcclApp::free() 
{
   MSG("Done Running Test");

   // Clean-up and return
done_3:
   m_pALIBufferService->bufferFree(m_JointVirt);
done_2:
   m_pALIBufferService->bufferFree(m_DSMVirt);

done_1:
   // Freed all three so now Release() the VTP Service through the Services IAALService::Release() method
   (dynamic_ptr<IAALService>(iidService, m_pVTP_AALService))->Release(TransactionID());
   m_Sem.Wait();

   // Freed all so now Release() the Service through the Services IAALService::Release() method
   (dynamic_ptr<IAALService>(iidService, m_pAALService))->Release(TransactionID());
   m_Sem.Wait();

done_0:
   m_Runtime.stop();
   m_Sem.Wait();

}

//=================
//  IServiceClient
//=================

// <begin IServiceClient interface>
void AcclApp::serviceAllocated(IBase *pServiceBase,
                                      TransactionID const &rTranID)
{
   // This application will allocate two different services (HWALIAFU and
   //  VTPService). We can tell them apart here by looking at the TransactionID.
   if (rTranID ==  m_AcclAppTranID) {
     // Save the IBase for the Service. Through it we can get any other
     //  interface implemented by the Service
     m_pAALService = pServiceBase;
     ASSERT(NULL != m_pAALService);
     if ( NULL == m_pAALService ) {
        m_bIsOK = false;
        return;
     }

     // Documentation says HWALIAFU Service publishes
     //    IALIBuffer as subclass interface. Used in Buffer Allocation and Free
     m_pALIBufferService = dynamic_ptr<IALIBuffer>(iidALI_BUFF_Service, pServiceBase);
     ASSERT(NULL != m_pALIBufferService);
     if ( NULL == m_pALIBufferService ) {
        m_bIsOK = false;
        return;
     }

     // Documentation says HWALIAFU Service publishes
     //    IALIMMIO as subclass interface. Used to set/get MMIO Region
     m_pALIMMIOService = dynamic_ptr<IALIMMIO>(iidALI_MMIO_Service, pServiceBase);
     ASSERT(NULL != m_pALIMMIOService);
     if ( NULL == m_pALIMMIOService ) {
        m_bIsOK = false;
        return;
     }

     // Documentation says HWALIAFU Service publishes
     //    IALIReset as subclass interface. Used for resetting the AFU
     m_pALIResetService = dynamic_ptr<IALIReset>(iidALI_RSET_Service, pServiceBase);
     ASSERT(NULL != m_pALIResetService);
     if ( NULL == m_pALIResetService ) {
        m_bIsOK = false;
        return;
     }
   }
   else if (rTranID == m_VTPTranID) {
     // Save the IBase for the VTP Service.
     m_pVTP_AALService = pServiceBase;
     ASSERT(NULL != m_pVTP_AALService);
     if ( NULL == m_pVTP_AALService ) {
        m_bIsOK = false;
        return;
     }

     // Documentation says VTP Service publishes
     //    IVTP as subclass interface. Used for allocating shared
     //    buffers that support virtual addresses from AFU
     m_pVTPService = dynamic_ptr<IMPFVTP>(iidMPFVTPService, pServiceBase);
     ASSERT(NULL != m_pVTPService);
     if ( NULL == m_pVTPService ) {
        m_bIsOK = false;
        return;
     }

   }   
   else
   {
      ERR("Unknown transaction ID encountered on serviceAllocated().");
      m_bIsOK = false;
      return;
   }
   
   MSG("Service Allocated");
   m_Sem.Post(1);
}

void AcclApp::serviceAllocateFailed(const IEvent &rEvent)
{
   ERR("Failed to allocate Service");
    PrintExceptionDescription(rEvent);
   ++m_Result;                     // Remember the error
   m_bIsOK = false;

   m_Sem.Post(1);
}

 void AcclApp::serviceReleased(TransactionID const &rTranID)
{
    MSG("Service Released");
   // Unblock Main()
   m_Sem.Post(1);
}

 void AcclApp::serviceReleaseFailed(const IEvent        &rEvent)
 {
    ERR("Failed to release a Service");
    PrintExceptionDescription(rEvent);
    m_bIsOK = false;
    m_Sem.Post(1);
 }
 
 void AcclApp::serviceReleaseRequest(IBase *pServiceBase, const IEvent &rEvent)
 {
    MSG("Service unexpected requested back");
    if(NULL != m_pAALService){
       IAALService *pIAALService = dynamic_ptr<IAALService>(iidService, m_pAALService);
       ASSERT(pIAALService);
       pIAALService->Release(TransactionID());
    }
 }


 void AcclApp::serviceEvent(const IEvent &rEvent)
{
   ERR("unexpected event 0x" << hex << rEvent.SubClassID());
   // The state machine may or may not stop here. It depends upon what happened.
   // A fatal error implies no more messages and so none of the other Post()
   //    will wake up.
   // OTOH, a notification message will simply print and continue.
}

 void AcclApp::runtimeStarted( IRuntime            *pRuntime,
                                      const NamedValueSet &rConfigParms)
 {
    m_bIsOK = true;
    m_Sem.Post(1);
 }

 void AcclApp::runtimeStopped(IRuntime *pRuntime)
  {
     MSG("Runtime stopped");
     m_bIsOK = false;
     m_Sem.Post(1);
  }

 void AcclApp::runtimeStartFailed(const IEvent &rEvent)
 {
    ERR("Runtime start failed");
    PrintExceptionDescription(rEvent);
 }

 void AcclApp::runtimeStopFailed(const IEvent &rEvent)
 {
     MSG("Runtime stop failed");
     m_bIsOK = false;
     m_Sem.Post(1);
 }

 void AcclApp::runtimeAllocateServiceFailed( IEvent const &rEvent)
 {
    ERR("Runtime AllocateService failed");
    PrintExceptionDescription(rEvent);
 }

 void AcclApp::runtimeAllocateServiceSucceeded(IBase *pClient,
                                                     TransactionID const &rTranID)
 {
     MSG("Runtime Allocate Service Succeeded");
 }

 void AcclApp::runtimeEvent(const IEvent &rEvent)
 {
     MSG("Generic message handler (runtime)");
 }

