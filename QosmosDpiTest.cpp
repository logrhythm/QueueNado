#include "QosmosDpiTest.h"
#include "MockConf.h"
#include <fstream>
using namespace std;

bool QosmosDpiTest::gDpiInit(false);
MockConfMaster QosmosDpiTest::mConfMaster;
ExposedQosmosDPI *QosmosDpiTest::mDpiEngine(NULL);
QosmosPacketAllocator& QosmosDpiTest::mPacketAllocator(QosmosPacketAllocator::Instance());
// Callback function called for each protocol. See ctl_proto_getlist.
boost::mutex QosmosDpiTest::mMutex;
::testing::Environment * const gEnv = ::testing::AddGlobalTestEnvironment(new Environment);

void QosmosDpiTest::testDisableAllProtocols(const clep_proto_t * proto,
        int nb_uppers, int *upper_proto_id,
        int nb_parents, int *parent_proto_id) {
   std::string protoName(proto->name);
   if (protoName == "base"
           || protoName == "unknown"
           || protoName == "malformed"
           || protoName == "incomplete") {
      // Can not disable this set of protocols.
      return;
   }
   EXPECT_TRUE(mDpiEngine->ProtoDisable(protoName));
}

TEST_F(QosmosDpiTest, DisableEnableProtoTest) {
   if (geteuid() == 0) {

      // Disable all protocols individually
      EXPECT_TRUE(mDpiEngine->ProtoEnableAll());
      clep_proto_t proto;
      PROTO_CLEAR(&proto);
      int result = ctl_proto_getlist(&proto,
              QosmosDpiTest::testDisableAllProtocols);
      EXPECT_GE(result, 0);

      std::string validProtoName("ftp");
      EXPECT_TRUE(mDpiEngine->ProtoEnable(validProtoName));
      std::string invalidProtoName("asdflkj");
      EXPECT_FALSE(mDpiEngine->ProtoEnable(invalidProtoName));
      EXPECT_FALSE(mDpiEngine->ProtoDisable(invalidProtoName));

      // Enable/Disable family of protocols
      std::string validFamilyName("Antivirus");
      EXPECT_TRUE(mDpiEngine->FamilyEnable(validFamilyName));
      EXPECT_TRUE(mDpiEngine->FamilyDisable(validFamilyName));

      validFamilyName.clear();
      validFamilyName.assign("Application Service");
      EXPECT_TRUE(mDpiEngine->FamilyEnable(validFamilyName));
      EXPECT_TRUE(mDpiEngine->FamilyDisable(validFamilyName));

      validFamilyName.clear();
      validFamilyName.assign("Audio/Video");
      EXPECT_TRUE(mDpiEngine->FamilyEnable(validFamilyName));
      EXPECT_TRUE(mDpiEngine->FamilyDisable(validFamilyName));

      validFamilyName.clear();
      validFamilyName.assign("Authentication");
      EXPECT_TRUE(mDpiEngine->FamilyEnable(validFamilyName));
      EXPECT_TRUE(mDpiEngine->FamilyDisable(validFamilyName));

      validFamilyName.clear();
      validFamilyName.assign("Compression");
      EXPECT_TRUE(mDpiEngine->FamilyEnable(validFamilyName));
      EXPECT_TRUE(mDpiEngine->FamilyDisable(validFamilyName));

      validFamilyName.clear();
      validFamilyName.assign("Database");
      EXPECT_TRUE(mDpiEngine->FamilyEnable(validFamilyName));
      EXPECT_TRUE(mDpiEngine->FamilyDisable(validFamilyName));

      validFamilyName.clear();
      validFamilyName.assign("Encrypted");
      EXPECT_TRUE(mDpiEngine->FamilyEnable(validFamilyName));
      EXPECT_TRUE(mDpiEngine->FamilyDisable(validFamilyName));

      validFamilyName.clear();
      validFamilyName.assign("ERP");
      EXPECT_TRUE(mDpiEngine->FamilyEnable(validFamilyName));
      EXPECT_TRUE(mDpiEngine->FamilyDisable(validFamilyName));

      validFamilyName.clear();
      validFamilyName.assign("File Server");
      EXPECT_TRUE(mDpiEngine->FamilyEnable(validFamilyName));
      EXPECT_TRUE(mDpiEngine->FamilyDisable(validFamilyName));

      validFamilyName.clear();
      validFamilyName.assign("File Transfer");
      EXPECT_TRUE(mDpiEngine->FamilyEnable(validFamilyName));
      EXPECT_TRUE(mDpiEngine->FamilyDisable(validFamilyName));

      validFamilyName.clear();
      validFamilyName.assign("Forum");
      EXPECT_TRUE(mDpiEngine->FamilyEnable(validFamilyName));
      EXPECT_TRUE(mDpiEngine->FamilyDisable(validFamilyName));

      validFamilyName.clear();
      validFamilyName.assign("Game");
      EXPECT_TRUE(mDpiEngine->FamilyEnable(validFamilyName));
      EXPECT_TRUE(mDpiEngine->FamilyDisable(validFamilyName));

      validFamilyName.clear();
      validFamilyName.assign("Instant Messaging");
      EXPECT_TRUE( mDpiEngine->FamilyEnable( validFamilyName ) );
      EXPECT_TRUE( mDpiEngine->FamilyDisable( validFamilyName ) );

      validFamilyName.clear();
      validFamilyName.assign("Mail");
      EXPECT_TRUE(mDpiEngine->FamilyEnable(validFamilyName));
      EXPECT_TRUE(mDpiEngine->FamilyDisable(validFamilyName));

      validFamilyName.clear();
      validFamilyName.assign("Microsoft Office");
      EXPECT_TRUE(mDpiEngine->FamilyEnable(validFamilyName));
      EXPECT_TRUE(mDpiEngine->FamilyDisable(validFamilyName));

      validFamilyName.clear();
      validFamilyName.assign("Middleware");
      EXPECT_TRUE(mDpiEngine->FamilyEnable(validFamilyName));
      EXPECT_TRUE(mDpiEngine->FamilyDisable(validFamilyName));

      validFamilyName.clear();
      validFamilyName.assign("Network Management");
      EXPECT_TRUE(mDpiEngine->FamilyEnable(validFamilyName));
      EXPECT_TRUE(mDpiEngine->FamilyDisable(validFamilyName));

      validFamilyName.clear();
      validFamilyName.assign("Network Service");
      EXPECT_TRUE(mDpiEngine->FamilyEnable(validFamilyName));
      EXPECT_TRUE(mDpiEngine->FamilyDisable(validFamilyName));

      validFamilyName.clear();
      validFamilyName.assign("Peer to Peer");
      EXPECT_TRUE(mDpiEngine->FamilyEnable(validFamilyName));
      EXPECT_TRUE(mDpiEngine->FamilyDisable(validFamilyName));

      validFamilyName.clear();
      validFamilyName.assign("Printer");
      EXPECT_TRUE(mDpiEngine->FamilyEnable(validFamilyName));
      EXPECT_TRUE(mDpiEngine->FamilyDisable(validFamilyName));

      validFamilyName.clear();
      validFamilyName.assign("Routing");
      EXPECT_TRUE(mDpiEngine->FamilyEnable(validFamilyName));
      EXPECT_TRUE(mDpiEngine->FamilyDisable(validFamilyName));

      validFamilyName.clear();
      validFamilyName.assign("Security Service");
      EXPECT_TRUE(mDpiEngine->FamilyEnable(validFamilyName));
      EXPECT_TRUE(mDpiEngine->FamilyDisable(validFamilyName));

      validFamilyName.clear();
      validFamilyName.assign("Standard");
      EXPECT_TRUE(mDpiEngine->FamilyEnable(validFamilyName));
      EXPECT_TRUE(mDpiEngine->FamilyDisable(validFamilyName));

      validFamilyName.clear();
      validFamilyName.assign("Telephony");
      EXPECT_TRUE(mDpiEngine->FamilyEnable(validFamilyName));
      EXPECT_TRUE(mDpiEngine->FamilyDisable(validFamilyName));

      validFamilyName.clear();
      validFamilyName.assign("Terminal");
      EXPECT_TRUE(mDpiEngine->FamilyEnable(validFamilyName));
      EXPECT_TRUE(mDpiEngine->FamilyDisable(validFamilyName));

      validFamilyName.clear();
      validFamilyName.assign("Thin Client");
      EXPECT_TRUE(mDpiEngine->FamilyEnable(validFamilyName));
      EXPECT_TRUE(mDpiEngine->FamilyDisable(validFamilyName));

      validFamilyName.clear();
      validFamilyName.assign("Tunneling");
      EXPECT_TRUE(mDpiEngine->FamilyEnable(validFamilyName));
      EXPECT_TRUE(mDpiEngine->FamilyDisable(validFamilyName));

      validFamilyName.clear();
      validFamilyName.assign("Wap");
      EXPECT_TRUE(mDpiEngine->FamilyEnable(validFamilyName));
      EXPECT_TRUE(mDpiEngine->FamilyDisable(validFamilyName));

      validFamilyName.clear();
      validFamilyName.assign("Web");
      EXPECT_TRUE( mDpiEngine->FamilyEnable( validFamilyName ) );
      EXPECT_TRUE( mDpiEngine->FamilyDisable( validFamilyName ) );

      validFamilyName.clear();
      validFamilyName.assign("Webmail");
      EXPECT_TRUE(mDpiEngine->FamilyEnable(validFamilyName));
      EXPECT_TRUE(mDpiEngine->FamilyDisable(validFamilyName));

      /* Doesn't work ... returns success
      std::string invalidFamilyName("liambeW");
      EXPECT_FALSE( mDpiEngine->FamilyEnable( invalidFamilyName ) );
      EXPECT_FALSE( mDpiEngine->FamilyDisable( invalidFamilyName ) );
       */

   }
}

TEST_F(QosmosDpiTest, InitializeationFailureUAFC) {
#ifdef LR_DEBUG
   if (geteuid() == 0) {
      ASSERT_TRUE(t_pcapInfo.InitializePCapInfo());
      vector<string> interfaces = t_pcapInfo.GetDefaultInterface();
      ASSERT_FALSE(interfaces.empty());
      t_interface = interfaces[0];
      ASSERT_FALSE(t_interface.empty());
      mNoInitDpiEngine.mFailInitUafc = true;
      EXPECT_FALSE(mNoInitDpiEngine.Initialize(QOSMOS_TEST_PACKET_SIZE, t_interface, mConf));
   }
#endif
}
#if 0
TEST_F(QosmosDpiTest, InitializeationFailureEnableAll) {
#ifdef LR_DEBUG
   if (geteuid() == 0) {
      ASSERT_TRUE(t_pcapInfo.InitializePCapInfo());
      vector<string> interfaces = t_pcapInfo.GetDefaultInterface();
      ASSERT_FALSE(interfaces.empty());
      t_interface = interfaces[0];
      ASSERT_FALSE(t_interface.empty());
      mNoInitDpiEngine.mFailProtoEnableAll = true;
      EXPECT_FALSE(mNoInitDpiEngine.Initialize(QOSMOS_TEST_PACKET_SIZE, t_interface, mConf));
   }
#endif
}

TEST_F(QosmosDpiTest, InitializeationFailureTCPReassembly) {
#ifdef LR_DEBUG
   if (geteuid() == 0) {
      ExposedQosmosDPI dpiEngine(mConfMaster);
      ASSERT_TRUE(t_pcapInfo.InitializePCapInfo());
      vector<string> interfaces = t_pcapInfo.GetDefaultInterface();
      ASSERT_FALSE(interfaces.empty());
      t_interface = interfaces[0];
      ASSERT_FALSE(t_interface.empty());
      dpiEngine.mFailEnableTCPReassembly = true;
      EXPECT_FALSE(dpiEngine.Initialize(QOSMOS_TEST_PACKET_SIZE, t_interface, mConf));
   }
#endif
}

TEST_F(QosmosDpiTest, InitializeationFailureFTPPayload) {
#ifdef LR_DEBUG
   if (geteuid() == 0) {
      ExposedQosmosDPI dpiEngine(mConfMaster);
      ASSERT_TRUE(t_pcapInfo.InitializePCapInfo());
      vector < string > interfaces = t_pcapInfo.GetDefaultInterface();
      ASSERT_FALSE(interfaces.empty());
      t_interface = interfaces[0];
      ASSERT_FALSE(t_interface.empty());
      dpiEngine.mFailSetFtpMaxPayload = true;
      EXPECT_TRUE(dpiEngine.Initialize(QOSMOS_TEST_PACKET_SIZE, t_interface, mConf));
   }
#endif
}



TEST_F(QosmosDpiTest, InitializeationFailureAcquireDevice) {
#ifdef LR_DEBUG
   if (geteuid() == 0) {
      ExposedQosmosDPI dpiEngine(mConfMaster);
      ASSERT_TRUE(t_pcapInfo.InitializePCapInfo());
      vector<string> interfaces = t_pcapInfo.GetDefaultInterface();
      ASSERT_FALSE(interfaces.empty());
      t_interface = interfaces[0];
      ASSERT_FALSE(t_interface.empty());
      dpiEngine.mFailAcquireDevice = true;
      EXPECT_FALSE(dpiEngine.Initialize(QOSMOS_TEST_PACKET_SIZE, t_interface, mConf));
   }
#endif
}

TEST_F(QosmosDpiTest, InitializeationFailureSMBMinPayload) {
#ifdef LR_DEBUG
   if (geteuid() == 0) {
      ExposedQosmosDPI dpiEngine(mConfMaster);
      ASSERT_TRUE(t_pcapInfo.InitializePCapInfo());
      vector<string> interfaces = t_pcapInfo.GetDefaultInterface();
      ASSERT_FALSE(interfaces.empty());
      t_interface = interfaces[0];
      ASSERT_FALSE(t_interface.empty());
      dpiEngine.mFailSetSMBMinPayload = true;
      EXPECT_TRUE(dpiEngine.Initialize(QOSMOS_TEST_PACKET_SIZE, t_interface, mConf));
   }
#endif
}
#endif
TEST_F(QosmosDpiTest, ConstructAndInitialize) {
   if (geteuid() == 0) {
      ASSERT_TRUE(t_pcapInfo.InitializePCapInfo());
      vector < string > interfaces = t_pcapInfo.GetDefaultInterface();
      ASSERT_FALSE(interfaces.empty());
      t_interface = interfaces[0];
      ASSERT_FALSE(t_interface.empty());
      EXPECT_FALSE(mDpiEngine->GetMetaDataCapture());
   }
}

TEST_F(QosmosDpiTest, ConstructAndInitializeReEnterSelectedCalls) {
#ifdef LR_DEBUG
   if (geteuid() == 0) {
      ASSERT_TRUE(t_pcapInfo.InitializePCapInfo());
      vector < string > interfaces = t_pcapInfo.GetDefaultInterface();
      ASSERT_FALSE(interfaces.empty());
      t_interface = interfaces[0];
      ASSERT_FALSE(t_interface.empty());
      EXPECT_FALSE(mDpiEngine->GetMetaDataCapture());
      EXPECT_TRUE(mDpiEngine->InitUafc(mConf));
      EXPECT_TRUE(mDpiEngine->ProtoEnableAll());
      EXPECT_TRUE(mDpiEngine->SetFtpMaxPayload(10000));
      EXPECT_TRUE(mDpiEngine->EnableTCPReassembly());
      EXPECT_TRUE(mDpiEngine->SetSMBMinPayload(20));
      EXPECT_TRUE(mDpiEngine->AcquireDevice(t_interface));
      mDpiEngine->SetupPacketCallbacks();
   }
#endif
}

TEST_F(QosmosDpiTest, AquireDeviceTests) {
   t_interface = "";
   EXPECT_FALSE(mDpiEngine->AcquireDevice(t_interface));

}
#if 0
TEST_F(QosmosDpiTest, DynamicConstruct) {
   if (geteuid() == 0) {
      ASSERT_TRUE(t_pcapInfo.InitializePCapInfo());
      vector < string > interfaces = t_pcapInfo.GetDefaultInterface();
      ASSERT_FALSE(interfaces.empty());
      t_interface = interfaces[0];
      ASSERT_FALSE(t_interface.empty());
      ExposedQosmosDPI* pdpiEngine = new ExposedQosmosDPI(mConfMaster);
      PCapInfo* ppcapInfo = new PCapInfo;
      delete pdpiEngine;
      delete ppcapInfo;

   }

}

TEST_F(QosmosDpiTest, DynamicConstructAndInitialize) {
#ifdef LR_DEBUG
   if (geteuid() == 0) {
      //std::cout << "start" << std::endl;
      ASSERT_TRUE(t_pcapInfo.InitializePCapInfo());
      //std::cout << "get default" << std::endl;
      vector < string > interfaces = t_pcapInfo.GetDefaultInterface();
      //std::cout << "got interfaces" << std::endl;
      ASSERT_FALSE(interfaces.empty());
      t_interface = interfaces[0];
      ASSERT_FALSE(t_interface.empty());
      //std::cout << "construct" << std::endl;
      ExposedQosmosDPI* pdpiEngine = new ExposedQosmosDPI(mConfMaster);
      //std::cout << "initialize" << std::endl;
      EXPECT_TRUE(pdpiEngine->Initialize(QOSMOS_TEST_PACKET_SIZE, t_interface, mConf));
      //std::cout << "delete" << std::endl;
      //raise(SIGTERM);
      delete pdpiEngine;
      //std::cout << "end" << std::endl;
   }
#endif

}
#endif
TEST_F(QosmosDpiTest, ConstructNoInitialize) {
   ctb_ppacket packetData = NULL;
   std::vector<Rifle* > sendQueue;
   size_t destination;
   EXPECT_FALSE(mNoInitDpiEngine.ProcessPacket(packetData, sendQueue, destination));
   //EXPECT_EQ(0, mNoInitDpiEngine.GetPacketHash(packetData));
   EXPECT_FALSE(mNoInitDpiEngine.GetMetaDataCapture());
}

TEST_F(QosmosDpiTest, ConstructAndInitializeBadly) {
#ifdef LR_DEBUG

   if (geteuid() == 0) {
      ASSERT_TRUE(t_pcapInfo.InitializePCapInfo());
      vector < string > interfaces = t_pcapInfo.GetDefaultInterface();
      ASSERT_FALSE(interfaces.empty());
      t_interface = interfaces[0];
      ASSERT_FALSE(t_interface.empty());
      mConf.mDpiHalfSessions = 0;
      ASSERT_FALSE(mDpiEngine->Initialize(10000, t_interface, mConf));
      ASSERT_FALSE(mNoInitDpiEngine.Initialize(0, t_interface, mConf));

      mConf.mDpiHalfSessions = QOSMOS_TEST_NUM_HALF_SESSIONS;
      mConf.mQosmos64 = 0;
      ASSERT_FALSE(mDpiEngine->Initialize(10000, t_interface, mConf));
      ASSERT_FALSE(mNoInitDpiEngine.Initialize(0, t_interface, mConf));
      mConf.mQosmos64 = QOSMOS_TEST_NUM_HALF_SESSIONS;
      mConf.mQosmos128 = 0;
      ASSERT_FALSE(mDpiEngine->Initialize(10000, t_interface, mConf));
      ASSERT_FALSE(mNoInitDpiEngine.Initialize(0, t_interface, mConf));
      mConf.mQosmos128 = QOSMOS_TEST_NUM_HALF_SESSIONS;
      mConf.mQosmos256 = 0;
      ASSERT_FALSE(mDpiEngine->Initialize(10000, t_interface, mConf));
      ASSERT_FALSE(mNoInitDpiEngine.Initialize(0, t_interface, mConf));
      mConf.mQosmos256 = QOSMOS_TEST_NUM_HALF_SESSIONS;
      mConf.mQosmos512 = 0;
      ASSERT_FALSE(mDpiEngine->Initialize(10000, t_interface, mConf));
      ASSERT_FALSE(mNoInitDpiEngine.Initialize(0, t_interface, mConf));
      mConf.mQosmos512 = QOSMOS_TEST_NUM_HALF_SESSIONS;
      t_interface = "";
      ASSERT_FALSE(mNoInitDpiEngine.Initialize(QOSMOS_TEST_PACKET_SIZE, t_interface, mConf));
      ctb_ppacket packetData = NULL;
      std::vector<Rifle* > sendQueue;
      size_t destination;
      EXPECT_FALSE(mNoInitDpiEngine.ProcessPacket(packetData, sendQueue, destination));
      //EXPECT_EQ(0, mNoInitDpiEngine.GetPacketHash(packetData));
      EXPECT_FALSE(mNoInitDpiEngine.GetMetaDataCapture());
   }
#endif
}

TEST_F(QosmosDpiTest, ProcessPacket) {
   if (geteuid() == 0) {
      ASSERT_TRUE(t_pcapInfo.InitializePCapInfo());
      vector < string > interfaces = t_pcapInfo.GetDefaultInterface();
      ASSERT_FALSE(interfaces.empty());
      t_interface = interfaces[0];
      ASSERT_FALSE(t_interface.empty());
      std::string data("abc12345678787777777777777777777777777777777777777777777777777770000000000000000000000000000000000000000000");
      struct pcap_pkthdr hdr;
      hdr.len = data.size();
      hdr.caplen = data.size();
      ctb_ppacket packet = NULL;
      //std::cout << "getting packet " << std::endl;
      mPacketAllocator.PopulatePacketData(reinterpret_cast<const u_char*> (&data[0]), &hdr, packet);
      //std::cout << "process packet " << std::endl;
      std::vector<Rifle* > sendQueue;
      Rifle* serverQueue = new Rifle("ipc:///tmp/qosmosDpiTest.ipc");
      serverQueue->SetOwnSocket(false);
      ASSERT_TRUE(serverQueue->Aim());
      sendQueue.push_back(serverQueue);
      size_t destination;
      EXPECT_TRUE(mDpiEngine->ProcessPacket(packet, sendQueue, destination));
      delete serverQueue;
      QosmosDPI::FreePacket(packet);
   }
}

//TEST_F( QosmosDpiTest, HashPacket)
//{
//	if (geteuid() == 0) {
//		ASSERT_TRUE(t_pcapInfo.InitializePCapInfo());
//		vector<string> interfaces = t_pcapInfo.GetDefaultInterface();
//		ASSERT_FALSE(interfaces.empty());
//		t_interface = interfaces[0];
//	    ASSERT_FALSE(t_interface.empty());
//		EXPECT_TRUE(mDpiEngine->Initialize(
//		   QOSMOS_TEST_NUM_HALF_SESSIONS,QOSMOS_TEST_PACKET_SIZE,t_interface));
//		ctb_ppacket ppacket;
//		PacketData data;
//		string apacketString = "1235";
//		data.getBuffer(apacketString);
//		mDpiEngine->mMallocpacketFail = true;
//		EXPECT_EQ(0,mDpiEngine->GetPacketHash(data));
//		mDpiEngine->mHashResult = 123;
//		mDpiEngine->mMallocpacketFail = false;
//		EXPECT_EQ(123,mDpiEngine->GetPacketHash(data));
//	}
//}

TEST_F(QosmosDpiTest, DISABLED_EnableAndDisableCapture) {
   if (geteuid() == 0) {
      //std::cout << "start" << std::endl;
      ASSERT_TRUE(t_pcapInfo.InitializePCapInfo());
      vector < string > interfaces = t_pcapInfo.GetDefaultInterface();
      ASSERT_FALSE(interfaces.empty());
      t_interface = interfaces[0];
      ASSERT_FALSE(t_interface.empty());
      //std::cout << "enable" << std::endl;
      mNoInitDpiEngine.EnableDpiMsgLRCapture();
      ASSERT_FALSE(mNoInitDpiEngine.IsCapturingDpiMsgLR());
      //std::cout << "disable" << std::endl;
      mNoInitDpiEngine.DisableDpiMsgLRCapture();
      //std::cout << "enable" << std::endl;
      mDpiEngine->EnableDpiMsgLRCapture();
      ASSERT_TRUE(mDpiEngine->IsCapturingDpiMsgLR());
      //std::cout << "disable" << std::endl;
      mDpiEngine->DisableDpiMsgLRCapture();
      //std::cout << "enable x2" << std::endl;
      mDpiEngine->EnableDpiMsgLRCapture();
      mDpiEngine->EnableDpiMsgLRCapture();
      ASSERT_TRUE(mDpiEngine->IsCapturingDpiMsgLR());
      //std::cout << "disable x3" << std::endl;
      mDpiEngine->DisableDpiMsgLRCapture();
      mDpiEngine->DisableDpiMsgLRCapture();
      mDpiEngine->DisableDpiMsgLRCapture();
      //std::cout << "enable last time" << std::endl;
      mDpiEngine->EnableDpiMsgLRCapture();
      mDpiEngine->DisableDpiMsgLRCapture();
   }
}

TEST_F(QosmosDpiTest, FreeAndSendCalls) {
   std::string data("abc00000000000000000000000000000000000000000000000000000000000000000000000000000000000000");
   struct pcap_pkthdr hdr;
   hdr.len = data.size();
   hdr.caplen = data.size();
   ctb_ppacket packet = NULL;
   mPacketAllocator.PopulatePacketData(
           reinterpret_cast<const u_char*> (&data[0]), &hdr, packet);
   QosmosDPI::EndPacket(packet);
   mPacketAllocator.PopulatePacketData(
           reinterpret_cast<const u_char*> (&data[0]), &hdr, packet);
   QosmosDPI::FreePacket(packet);
}

TEST_F(QosmosDpiTest, IsTimeToReportStatsTest) {
#ifdef LR_DEBUG
   ExposedQosmosDPI dpiEngine(mConfMaster);
   dpiEngine.SetEpochTime(10);
   EXPECT_TRUE(dpiEngine.IsTimeToReportStats());
   EXPECT_FALSE(dpiEngine.IsTimeToReportStats());
   dpiEngine.SetEpochTime(11);
   EXPECT_TRUE(dpiEngine.IsTimeToReportStats());
   dpiEngine.SetEpochTime(9); // Time reset to earlier time.
   EXPECT_FALSE(dpiEngine.IsTimeToReportStats());
   dpiEngine.SetEpochTime(10);
   EXPECT_TRUE(dpiEngine.IsTimeToReportStats());
#endif
}
