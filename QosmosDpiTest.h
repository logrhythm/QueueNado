#pragma once

#include "gtest/gtest.h"
#include "QosmosDPI.h"
#include "PCapInfo.h"
#include "MockConfMaster.h"
#include "MockConf.h"
#include "boost/thread/mutex.hpp"
#ifndef QOSMOS_TEST_PACKET_SIZE
#define QOSMOS_TEST_PACKET_SIZE 1500
#endif
#ifndef QOSMOS_TEST_NUM_HALF_SESSIONS
#define QOSMOS_TEST_NUM_HALF_SESSIONS 50000
#endif

class ExposedQosmosDPI : public QosmosDPI {
public:

   ExposedQosmosDPI(networkMonitor::ConfMaster& confMaster) :
   QosmosDPI(confMaster), mProcessResult(true), mHashResult(1), mFailInitUafc(false), mFailProtoEnableAll(false), mFailSetFtpMaxPayload(
   false), mFailEnableTCPReassembly(false), mFailSetSMBMinPayload(
   false), mFailAcquireDevice(false) {
   }

   bool GetMetaDataCapture() {
      return mCaptureMetaData;
   }

   bool Process(ctb_ppacket& packet, unsigned int way) {
      return mProcessResult;
   }

   unsigned int GetPacketHashFromPacket(ctb_ppacket& packet) {
      return mHashResult;
   }

   bool InitUafc(Conf& conf) {
      if (mFailInitUafc) {
         return false;
      }
      return QosmosDPI::InitUafc(conf);
   }

   bool ProtoEnableAll() {
      if (mFailProtoEnableAll) {
         return false;
      }
      return QosmosDPI::ProtoEnableAll();
   }

   bool SetFtpMaxPayload(size_t maxPayload) {
      if (mFailSetFtpMaxPayload) {
         return false;
      }
      return QosmosDPI::SetFtpMaxPayload(maxPayload);
   }

   bool EnableTCPReassembly() {
      if (mFailEnableTCPReassembly) {
         return false;
      }
      return QosmosDPI::EnableTCPReassembly();
   }

   bool SetSMBMinPayload(size_t minPayload) {
      if (mFailSetSMBMinPayload) {
         return false;
      }
      return QosmosDPI::SetSMBMinPayload(minPayload);
   }

   bool AcquireDevice(const std::string& interface) {
      if (mFailAcquireDevice) {
         return false;
      }
      return QosmosDPI::AcquireDevice(interface);
   }

   virtual void CleanupUAFC() {
      QosmosDPI::CleanupUAFC();
   }

   virtual void SetupPacketCallbacks() {
      QosmosDPI::SetupPacketCallbacks();
   }

   virtual void ProtoDisableAll() {
      QosmosDPI::ProtoDisableAll();
   }
   bool mProcessResult;
   int mHashResult;
   bool mFailInitUafc;
   bool mFailProtoEnableAll;
   bool mFailSetFtpMaxPayload;
   bool mFailEnableTCPReassembly;
   bool mFailSetSMBMinPayload;
   bool mFailAcquireDevice;

};



class QosmosDpiTest : public ::testing::Test {
public:

   QosmosDpiTest() :
   t_interface("not_here"), mNoInitDpiEngine(mConfMaster) {

      boost::mutex::scoped_lock lock(mMutex);
      if (mDpiEngine == NULL) {
         mDpiEngine = new ExposedQosmosDPI(mConfMaster);
      }
   }

   ~QosmosDpiTest() {
      //delete mDpiEngine;
   }
   static ExposedQosmosDPI *mDpiEngine;
   static networkMonitor::MockConfMaster mConfMaster;
protected:

   virtual void SetUp() {

#ifdef LR_DEBUG
      mConf.mQosmosDebug = false;
      mConf.mDpiHalfSessions = QOSMOS_TEST_NUM_HALF_SESSIONS;
#endif
      if (!gDpiInit) {
         gDpiInit = mDpiEngine->Initialize(QOSMOS_TEST_PACKET_SIZE, t_interface, mConf);
      }
      EXPECT_TRUE(gDpiInit);

   }

   virtual void TearDown() {
      
      gDpiInit = false;
      
   }
   static void testDisableAllProtocols(const clep_proto_t * proto,
           int nb_uppers, int *upper_proto_id,
           int nb_parents, int *parent_proto_id);
   static bool gDpiInit;

   
   static QosmosPacketAllocator& mPacketAllocator;
   static boost::mutex mMutex;
   ExposedQosmosDPI mNoInitDpiEngine;
   PCapInfo t_pcapInfo;
   std::string t_interface;
#ifdef LR_DEBUG
   MockConf mConf;
#else
   Conf mConf;
#endif
};

class Environment : public ::testing::Environment {
public:

   virtual ~Environment() {
   }

   virtual void SetUp() {
      QosmosDpiTest::mDpiEngine = new ExposedQosmosDPI(QosmosDpiTest::mConfMaster);
   }

   virtual void TearDown() {
      delete QosmosDpiTest::mDpiEngine;
   }
};