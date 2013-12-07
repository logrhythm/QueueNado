#include "IPacketCapture.h"
#include "PacketCaptureReceiver.h"
#include "PacketCaptureTest.h"
#include "PacketCapturePCapClientThread.h"
#include "MockSendStats.h"
#include <sys/socket.h>
#include "QosmosDPI.h"
#include <algorithm>
#include <limits>

using namespace std;
string MockPacketCapturePCap::mAnError = "this is an error";

struct PacketInfo {
   pcap_pkthdr* phdr;
   u_char* packet;
};

void MockPacketCapturePCap::SetNumberOfPacketToCapture(unsigned int number) {
   m_numberToCapture = number;
   m_numberOfEmptyReadsToTolerate = number << 1;
}

unsigned int MockPacketCapturePCap::GetNumberOfPacketsCaptured() {
   return m_numberCaptured;
}

bool MockPacketCapturePCap::IsDone() {
   if (mRealIsDone) {
      return PacketCapturePCap::IsDone();
   }
   m_numberOfEmptyReadsToTolerate--;
   if (m_numberOfEmptyReadsToTolerate == 0) {
      m_numberCaptured = m_numberToCapture;
   }
   return (m_numberCaptured >= m_numberToCapture);
}

int MockPacketCapturePCap::GetPacketFromPCap(ctb_ppacket& packet) {
   int returnVal = -1;
   if (mFakePCap) {
      if (mFakePCapRetVal > 0) {
         uint8_t* rawPacket;
         struct pcap_pkthdr *phdr;
         returnVal = 1;
         phdr = &mBogusHeader;
         timeval tv;
         tv.tv_sec = 12;
         tv.tv_usec = 34;
         phdr->ts = tv;
         phdr->caplen = 100;
         phdr->len = 100;
         rawPacket = new uint8_t[100];
         memset(mBogusPacket, 'a', 100);
         memcpy(rawPacket, mBogusPacket, 100);

         mPacketAllocator.PopulatePacketData(rawPacket, phdr, packet);
         delete []rawPacket;
         return 1;
      } else {
         return mFakePCapRetVal;
      }

   }
   returnVal = PacketCapturePCap::GetPacketFromPCap(packet);
   if (returnVal > 0) {
      m_numberCaptured++;
   }
   return returnVal;
}

TEST_F(PacketCaptureTest, PacketCapturePCapClientThread_GetPacketsFromPCap) {
#ifdef LR_DEBUG
   PacketCaptureReceiver receiver(t_serverAddr);
   MockPacketCapturePCap capturer(receiver.GetZMQ(), t_interface, mConf);
   capturer.Initialize();

   MockPacketCapturePCapClientThread clientThread(&capturer, receiver.GetZMQ());
   std::vector<std::pair<void*, unsigned int> > packets = clientThread.GetPackets(100);
   EXPECT_EQ(100, packets.size());
   PacketCapturePCap::CleanupPackets(packets);

   capturer.SetDispatchReturn(-2);
   packets = clientThread.GetPackets(100);
   EXPECT_EQ(1, packets.size());
   PacketCapturePCap::CleanupPackets(packets);

   capturer.SetDispatchReturn(-3);
   packets = clientThread.GetPackets(100);
   EXPECT_EQ(1, packets.size());
   PacketCapturePCap::CleanupPackets(packets);

   capturer.SetDispatchReturn(-4);
   packets = clientThread.GetPackets(100);
   EXPECT_EQ(1, packets.size());
   PacketCapturePCap::CleanupPackets(packets);

   capturer.SetDispatchReturn(1);
   packets = clientThread.GetPackets(100);
   EXPECT_EQ(100, packets.size());
   PacketCapturePCap::CleanupPackets(packets);

   capturer.SetDispatchCount(30);
   packets = clientThread.GetPackets(50);
   EXPECT_GE(packets.size(), 50);
   PacketCapturePCap::CleanupPackets(packets);

#endif

}

TEST_F(PacketCaptureTest, PacketCapturePCapClientThread_process_packets) {

   std::vector < std::pair< void*, unsigned int >> packets;
   uint8_t* rawPacket;
   struct pcap_pkthdr *phdr;
   phdr = &mBogusHeader;
   timeval tv;
   tv.tv_sec = 12;
   tv.tv_usec = 34;
   phdr->ts = tv;
   phdr->caplen = 100;
   phdr->len = 100;
   rawPacket = (u_char*) malloc(100);

   memset(mBogusPacket, 'a', 100);
   memcpy(rawPacket, mBogusPacket, 100);
   //NULL
   PacketCapturePCap::process_packets((u_char*) & packets, NULL, NULL);
   EXPECT_EQ(0, packets.size());
   PacketCapturePCap::CleanupPackets(packets);

   //NULL PACKET
   PacketCapturePCap::process_packets((u_char*) & packets, phdr, NULL);
   EXPECT_EQ(0, packets.size());
   PacketCapturePCap::CleanupPackets(packets);

   //NULL hdr
   PacketCapturePCap::process_packets((u_char*) & packets, NULL, rawPacket);
   EXPECT_EQ(0, packets.size());

   //valid data
   PacketCapturePCap::process_packets((u_char*) & packets, phdr, rawPacket);
   EXPECT_EQ(1, packets.size());


   PacketCapturePCap::CleanupPackets(packets);
   EXPECT_EQ(0, packets.size());
   if (rawPacket) {
      free(rawPacket);
   }
}

TEST_F(PacketCaptureTest, PcapVersion) {
   if (geteuid() == 0) {
#ifdef LR_DEBUG
      std::string interface = "NOPE";
      MockPacketCapturePCap capturer(t_clientAddr, interface, mConf);
      EXPECT_EQ("libpcap version 1.4.0", capturer.LogLibPCapVersion());
#endif
   }
}

TEST_F(PacketCaptureTest, NonExistantInterface) {
   if (geteuid() == 0) {
#ifdef LR_DEBUG
      std::string interface = "NOPE";
      MockPacketCapturePCap capturer(t_clientAddr, interface, mConf);
      pcap_t* handle = capturer.CreatePCapHandle();
      ASSERT_FALSE(NULL == handle);

      int status = capturer.ActivatePCapHandle(handle);
      EXPECT_EQ(PCAP_ERROR_NO_SUCH_DEVICE, status);
      EXPECT_FALSE(capturer.WasActivationSuccessful(handle, status));
      EXPECT_FALSE(capturer.Initialize(true));
      pcap_close(handle);

#endif
   }
}

TEST_F(PacketCaptureTest, ConstructAndInitialize) {
   if (geteuid() == 0) {
#ifdef LR_DEBUG
      PacketCaptureReceiver receiver(t_serverAddr);
      MockPacketCapturePCap capturer(t_clientAddr, t_interface, mConf);

      capturer.SetNumberOfPacketToCapture(0);
      ctb_ppacket packet;
      ASSERT_EQ(-1, capturer.GetPacketFromPCap(packet));
      ASSERT_TRUE(capturer.Initialize());
      //zclock_sleep(100);
      capturer.Shutdown(true);
      //EXPECT_EQ(0, capturer.GetNumberOfPacketsCaptured());
      capturer.Shutdown(true);
#endif
   }
}

TEST_F(PacketCaptureTest, DynamicConstructAndInitialize) {
   if (geteuid() == 0) {
#ifdef LR_DEBUG
      MockPacketCapturePCap* pcapturer = new MockPacketCapturePCap(t_clientAddr,
         t_interface, mConf);
      PacketCaptureReceiver* preceiver = new PacketCaptureReceiver(
         t_serverAddr);
      delete pcapturer;
      delete preceiver;
#endif
   }

}

TEST_F(PacketCaptureTest, InitializationFailureCantCreatePCapHandle) {
#ifdef LR_DEBUG
   MockPacketCapturePCap capturer(t_clientAddr, t_interface, mConf);
   capturer.mFailCreatePCapHandle = true;
   EXPECT_FALSE(capturer.Initialize());
#endif
}

TEST_F(PacketCaptureTest, InitializationFailureCantSetTimeout) {
#ifdef LR_DEBUG
   MockPacketCapturePCap capturer(t_clientAddr, t_interface, mConf);
   capturer.mFailSetPCapTimeout = true;
   EXPECT_FALSE(capturer.Initialize());
#endif
}

TEST_F(PacketCaptureTest, InitializationFailureCantSetBufferSize) {
#ifdef LR_DEBUG
   MockPacketCapturePCap capturer(t_clientAddr, t_interface, mConf);
   capturer.mFailSetBufferSize = true;
   EXPECT_FALSE(capturer.Initialize());
#endif
}

TEST_F(PacketCaptureTest, InitializationFailureCantSetPromiscuous) {
#ifdef LR_DEBUG
   MockPacketCapturePCap capturer(t_clientAddr, t_interface, mConf);
   capturer.mFailSetPromiscuous = true;
   EXPECT_FALSE(capturer.Initialize());
#endif
}

TEST_F(PacketCaptureTest, InitializationFailureCantActivate) {
#ifdef LR_DEBUG
   MockPacketCapturePCap capturer(t_clientAddr, t_interface, mConf);
   capturer.mFailActivatePCapHandle = true;
   EXPECT_FALSE(capturer.Initialize());
#endif
}

TEST_F(PacketCaptureTest, WasActivationSuccessfulCodeParsing) {
#ifdef LR_DEBUG
   MockPacketCapturePCap capturer(t_clientAddr, t_interface, mConf);
   EXPECT_FALSE(capturer.WasActivationSuccessful(NULL, -1));
   EXPECT_TRUE(capturer.WasActivationSuccessful(NULL, 0));
   EXPECT_TRUE(capturer.WasActivationSuccessful(NULL, 1));
#endif
}

TEST_F(PacketCaptureTest, CheckShutdownStuff) {
#ifdef LR_DEBUG
   MockPacketCapturePCap capturer(t_clientAddr, t_interface, mConf);
   capturer.mRealIsDone = true;
   EXPECT_FALSE(capturer.IsDone());
   capturer.Shutdown(false);
   EXPECT_TRUE(capturer.IsDone());
#endif
}

TEST_F(PacketCaptureTest, GetStatsFailure) {
#ifdef LR_DEBUG
   if (geteuid() == 0) {
      MockPacketCapturePCap capturer(t_clientAddr, t_interface, mConf);
      capturer.mFailGetStats = true;
      capturer.mPsRecv = 10;
      capturer.mPsDrop = 1;
      capturer.mPsIfDrop = 2;
      size_t I, dont, care, atall;
      capturer.SetNumberOfPacketToCapture(100);
      ASSERT_TRUE(capturer.Initialize());
      capturer.DisplayStats(I, dont, care, atall);
      EXPECT_EQ(0, I);
      EXPECT_EQ(0, dont);
      EXPECT_EQ(0, care);
      capturer.Shutdown(true);
   }
#endif
}

TEST_F(PacketCaptureTest, TimeToReportInterfaceStats) {
#ifdef LR_DEBUG
   if (geteuid() == 0) {
      MockPacketCapturePCap capturer(t_clientAddr, t_interface, mConf);
      struct timeval tv = {5, 0};
      time_t time = std::time(NULL) + 5;
      EXPECT_TRUE(capturer.InterfaceStatsTime(time));
      time = time + 6;
      EXPECT_TRUE(capturer.InterfaceStatsTime(time));
      time = time + 4;
      EXPECT_FALSE(capturer.InterfaceStatsTime(time));
      time = time + 1;
      EXPECT_TRUE(capturer.InterfaceStatsTime(time));
      capturer.Shutdown(true);
   }
#endif
}

TEST_F(PacketCaptureTest, GetSomethingFromTheInterfaceSendOverExisting) {
#ifdef LR_DEBUG
   if (geteuid() == 0) {

      PacketCaptureReceiver receiver(t_serverAddr);
      MockPacketCapturePCap capturer(receiver.GetZMQ(), t_interface, mConf);

      ASSERT_TRUE(capturer.Initialize());

      capturer.mFailGetStats = false;
      capturer.mPsRecv = 10;
      capturer.mPsDrop = 1;
      capturer.mPsIfDrop = 2;

      capturer.DisplayStats(t_packetsReceived, t_packetsDropped,
         t_packetsIfDropped, t_totalData);
      EXPECT_EQ(9, t_packetsReceived); // mPsRecv - mPsDrop
      EXPECT_EQ(1, t_packetsDropped);
      EXPECT_EQ(2, t_packetsIfDropped);

      capturer.mPsRecv = 20;
      capturer.mPsDrop = 2;
      capturer.mPsIfDrop = 4;

      capturer.DisplayStats(t_packetsReceived, t_packetsDropped,
         t_packetsIfDropped, t_totalData);
      EXPECT_EQ(18, t_packetsReceived); // mPsRecv - mPsDrop
      EXPECT_EQ(2, t_packetsDropped);
      EXPECT_EQ(4, t_packetsIfDropped);

      capturer.Shutdown(true);
   }
#endif
}

TEST_F(PacketCaptureTest, SendWrappedValues) {
#ifdef LR_DEBUG
   if (geteuid() == 0) {
      PacketCaptureReceiver receiver(t_serverAddr);
      MockPacketCapturePCap capturer(receiver.GetZMQ(), t_interface, mConf);
      capturer.SetupSendStatForwarder(); // pseudo SendStat

      // dynamic cast only in the unit test. GetStatForwarder would normally return the real class type
      MockSendStats& statForwarder = dynamic_cast<MockSendStats&> (capturer.GetSendStatForwarder());
      auto& receivedStats = statForwarder.mSendStatValues;
      ASSERT_TRUE(capturer.Initialize());

      static const uint32_t max = std::numeric_limits<uint32_t>::max();
      capturer.mFailGetStats = false;
      capturer.mPsRecv = max;
      capturer.mPsDrop = 100;
      capturer.mPsIfDrop = 200;

      capturer.DisplayStats(t_packetsReceived, t_packetsDropped,
         t_packetsIfDropped, t_totalData);
      ASSERT_EQ(receivedStats.size(), 4);
      EXPECT_EQ(capturer.mPsRecv, max);
      EXPECT_EQ(max - 100, t_packetsReceived); // mPsRecv - mPsDrop
      EXPECT_EQ(receivedStats[0], max - 100); // SendStat (rec-dropped - last)

      EXPECT_EQ(100, t_packetsDropped);
      EXPECT_EQ(receivedStats[1], 100); // SendStat (dropped - last)

      EXPECT_EQ(200, t_packetsIfDropped);
      EXPECT_EQ(receivedStats[2], 200); // SendStat (ifDropped -last)

      // wrap the values, i.e. lower value than earlier
      receivedStats.clear();
      capturer.mPsRecv = 10; // 10+1+100 more
      capturer.mPsDrop = 0; // max-100+1 more
      capturer.mPsIfDrop = 0; // max-200+1 more
      capturer.DisplayStats(t_packetsReceived, t_packetsDropped,
         t_packetsIfDropped, t_totalData);

      ASSERT_EQ(receivedStats.size(), 4);
      EXPECT_EQ(10, t_packetsReceived); // mPsRecv - mPsDrop
      EXPECT_EQ(receivedStats[0], 111); // SendState rec - last: wrap(10 + 1, max-100)

      EXPECT_EQ(0, t_packetsDropped);
      EXPECT_EQ(receivedStats[1], max - 100 + 1); // SendStat drop: (max -100 +1)

      EXPECT_EQ(0, t_packetsIfDropped);
      EXPECT_EQ(receivedStats[2], max - 200 + 1);

      capturer.Shutdown(true);
   }
#endif
}

TEST_F(PacketCaptureTest, TotalDataWrappedValues) {
#ifdef LR_DEBUG
   if (geteuid() == 0) {
      PacketCaptureReceiver receiver(t_serverAddr);
      MockPacketCapturePCap capturer(receiver.GetZMQ(), t_interface, mConf);
      capturer.SetupSendStatForwarder(); // pseudo SendStat

      // dynamic cast only in the unit test. GetStatForwarder would normally return the real class type
      MockSendStats& statForwarder = dynamic_cast<MockSendStats&> (capturer.GetSendStatForwarder());
      auto& receivedStats = statForwarder.mSendStatValues;
      ASSERT_TRUE(capturer.Initialize());

      capturer.mFailGetStats = false;
      int64_t iMax = std::numeric_limits<int64_t>::max();
      capturer.IncrementTotalData(iMax);
      // start from max
      capturer.DisplayStats(t_packetsReceived, t_packetsDropped,
         t_packetsIfDropped, t_totalData);
      size_t totalIndex = 3;
      EXPECT_EQ(t_totalData, iMax);
      EXPECT_EQ(receivedStats[totalIndex], iMax);
      receivedStats.clear();


      // wrap around
      capturer.IncrementTotalData(10); // i.e. 10+max --> wrap to 9
      capturer.DisplayStats(t_packetsReceived, t_packetsDropped,
         t_packetsIfDropped, t_totalData);
      EXPECT_EQ(t_totalData, 9); // latest update
      EXPECT_EQ(receivedStats[totalIndex], 10); // diff 10 in size from earlier
      receivedStats.clear();

      // small increment
      capturer.IncrementTotalData(15);
      capturer.DisplayStats(t_packetsReceived, t_packetsDropped,
         t_packetsIfDropped, t_totalData);
      EXPECT_EQ(t_totalData, 24); // 9+15
      EXPECT_EQ(receivedStats[totalIndex], 15); // 24-9
      receivedStats.clear();

      // wrap around again. This big increment represents many small
      capturer.IncrementTotalData(iMax); // 24 + max --> wrap --> 23
      capturer.DisplayStats(t_packetsReceived, t_packetsDropped,
         t_packetsIfDropped, t_totalData);
      EXPECT_EQ(t_totalData, 23);
      EXPECT_EQ(receivedStats[totalIndex], iMax);
      receivedStats.clear();

      capturer.Shutdown(true);
   }
#endif
}
//TEST_F( PacketCaptureTest, GetSomethingFromABigFile) {
//   if (geteuid() == 0) {
//      int expectedPackets = 583;
//
//      string filename = "FtpUploadLinuxSideCapture_2.pcapng";
//      MockPacketCapturePCap capturer(t_clientAddr, t_interface, filename);
//      PacketCaptureReceiver receiver(t_serverAddr);
//      capturer.SetNumberOfPacketToCapture(expectedPackets);
//
//      if (capturer.Initialize(64)) {
//
//         PacketDataContainer* capturedPackets;
//         capturedPackets = receiver.GetLatestPackets(expectedPackets, 1000000);
//         EXPECT_EQ(expectedPackets, capturedPackets->Size());
//         if (capturer.WaitForShutdown()) {
//            EXPECT_EQ(expectedPackets, capturer.GetNumberOfPacketsCaptured());
//         }
//
//      }
//      capturer.Shutdown(true);
//   }
//}
//TEST_F (PacketCaptureTest, TestFileDoesntExist) {
//   if (geteuid() == 0) {
//      string filename = "/path/is/invalid/nofile";
//      MockPacketCapturePCap capturer(t_clientAddr, t_interface, filename);
//      EXPECT_FALSE(capturer.Initialize(1));
//   }
//}

TEST_F(PacketCaptureTest, InterfaceDoesntExist) {
   if (geteuid() == 0) {
      string noInterface = "nointerface";
      MockPacketCapturePCap capturer(t_clientAddr, noInterface, mConf);
      EXPECT_FALSE(capturer.Initialize());
   }
}

TEST_F(PacketCaptureTest, StatsWorksEvenWithNullPCap) {
   PacketCapturePCap packetCapturer(t_clientAddr, t_interface, mConf);
   packetCapturer.DisplayStats(t_packetsReceived, t_packetsDropped,
      t_packetsIfDropped, t_totalData);
   EXPECT_EQ(0, t_packetsReceived);
   EXPECT_EQ(0, t_packetsDropped);
   EXPECT_EQ(0, t_packetsIfDropped);
   packetCapturer.Shutdown(true);
}

TEST_F(PacketCaptureTest, StatsWorksEvenWithFile) {
   string filename = "FtpUploadLinuxSideCapture.pcapng";
   PacketCapturePCap packetCapturer(filename, mConf);
   packetCapturer.DisplayStats(t_packetsReceived, t_packetsDropped,
      t_packetsIfDropped, t_totalData);
   EXPECT_EQ(0, t_packetsReceived);
   EXPECT_EQ(0, t_packetsDropped);
   EXPECT_EQ(0, t_packetsIfDropped);
   packetCapturer.Shutdown(true);
}

TEST_F(PacketCaptureTest, PCapInfo_Interface_List) {
   vector < string > interfaces;

   if (geteuid() == 0) {
      vector < string > defaultInterface = t_pcapInfo.GetDefaultInterface();
      ASSERT_EQ(1, defaultInterface.size());
      ASSERT_FALSE(defaultInterface[0].empty());

      interfaces = t_pcapInfo.GetAllInterfaces();
      ASSERT_TRUE(interfaces.size() > 0);
      EXPECT_TRUE(
         find(interfaces.begin(), interfaces.end(), defaultInterface[0]) != interfaces.end());

   } else {
      interfaces = t_pcapInfo.GetAllInterfaces();
      ASSERT_TRUE(interfaces.size() == 0);
   }
}

TEST_F(PacketCaptureTest, PCapInfo_FailToPopulate) {
#ifdef LR_DEBUG
   MockPCapInfo info;
   info.mFailPopulate = true;
   EXPECT_FALSE(info.InitializePCapInfo());
   vector < string > interfaces;
   interfaces = info.GetAllInterfaces();
   EXPECT_TRUE(interfaces.empty());
   interfaces = info.GetDefaultInterface();
   EXPECT_TRUE(interfaces.empty());
#endif
}

TEST_F(PacketCaptureTest, PCapInfo_FailNothingFound) {
#ifdef LR_DEBUG
   MockPCapInfo info;
   info.mNothingFound = true;
   EXPECT_FALSE(info.InitializePCapInfo());
   vector < string > interfaces;
   interfaces = info.GetAllInterfaces();
   EXPECT_TRUE(interfaces.empty());
   interfaces = info.GetDefaultInterface();
   EXPECT_TRUE(interfaces.empty());
#endif
}

TEST_F(PacketCaptureTest, PCapInfo_IsSupportedTests) {
#ifdef LR_DEBUG
   MockPCapInfo info;
   info.mFailIsLoopback = true;
   info.mPassIsIPReadable = true;

   pcap_if_t * currentDev(NULL);

   ASSERT_FALSE(info.IsSupported(currentDev));
   currentDev = new pcap_if_t;
   currentDev->next = NULL;
   currentDev->name = NULL;
   currentDev->description = NULL;
   currentDev->addresses = NULL;
   currentDev->flags = 0;
   ASSERT_FALSE(info.IsSupported(currentDev));
   string name("ethtest");
   char aBuffer[100];
   strncpy(aBuffer, name.c_str(), name.size());
   aBuffer[name.size()] = '\0';
   currentDev->name = aBuffer;
   ASSERT_FALSE(info.IsSupported(currentDev));
   currentDev->addresses = new pcap_addr;
   currentDev->addresses->addr = new sockaddr;
   currentDev->addresses->addr->sa_family = AF_INET;
   ASSERT_TRUE(info.IsSupported(currentDev));
   info.mAlwaysLoopBack = true;
   info.mFailIsLoopback = false;
   ASSERT_FALSE(info.IsSupported(currentDev));
   info.mAlwaysLoopBack = false;
   info.mNeverReadable = true;
   info.mPassIsIPReadable = false;
   delete currentDev->addresses->addr;
   currentDev->addresses->addr = NULL;
   ASSERT_FALSE(info.IsSupported(currentDev));

   delete currentDev->addresses;
   delete currentDev;
#endif
}

TEST_F(PacketCaptureTest, PCapInfo_IsLoopBack) {
   MockPCapInfo info;
   EXPECT_TRUE(info.IsLoopback(PCAP_IF_LOOPBACK));
   EXPECT_FALSE(info.IsLoopback(0xffffffff & ~PCAP_IF_LOOPBACK));
}

TEST_F(PacketCaptureTest, PCapInfo_IsIPReadable) {
   MockPCapInfo info;
   EXPECT_FALSE(info.IsIPReadable(NULL));
   struct sockaddr sa;
   sa.sa_family = 0xffff & ~AF_INET & ~AF_INET6 & ~AF_PACKET;
   EXPECT_FALSE(info.IsIPReadable(&sa));
   sa.sa_family = AF_INET;
   EXPECT_TRUE(info.IsIPReadable(&sa));
   sa.sa_family = AF_INET6;
   EXPECT_TRUE(info.IsIPReadable(&sa));
   sa.sa_family = AF_INET6;
   EXPECT_TRUE(info.IsIPReadable(&sa));

}

TEST_F(PacketCaptureTest, PCapInfo_GetDefault) {
#ifdef LR_DEBUG
   MockPCapInfo info;
   info.mQuickInit = true;
   info.mFailGetDefaultDevice = true;
   vector < string > interfaces = info.GetDefaultInterface();

   EXPECT_TRUE(interfaces.empty());
   info.mFailGetDefaultDevice = false;
   interfaces = info.GetDefaultInterface();
   EXPECT_FALSE(interfaces.empty());
#endif
}

TEST_F(PacketCaptureTest, PacketCapturePCapClientThreadConstruct) {
   PacketCaptureReceiver receiver(t_serverAddr);
   PacketCapturePCap capturer(receiver.GetZMQ(), t_interface, mConf);
   PacketReader pThreadClass(&capturer);
}

TEST_F(PacketCaptureTest, PacketCaptureDynamicConstruction) {
   PacketCaptureReceiver receiver(t_serverAddr);
   PacketCapturePCap capturer(receiver.GetZMQ(), t_interface, mConf);
   PacketReader* pThreadClass =
      new PacketReader(&capturer);
   delete pThreadClass;
   pThreadClass = new PacketReader(&capturer);
   delete pThreadClass;
}

TEST_F(PacketCaptureTest, PacketCapturePCapClientThread_GetPacket) {
#ifdef LR_DEBUG
   PacketCaptureReceiver receiver(t_serverAddr);
   MockPacketCapturePCap capturer(receiver.GetZMQ(), t_interface, mConf);

   MockPacketCapturePCapClientThread clientThread(&capturer, receiver.GetZMQ());

   ctb_ppacket packet(NULL);
   capturer.mFakePCap = true;
   ASSERT_EQ(1, clientThread.GetPacket(packet));
   ASSERT_EQ(capturer.mBogusHeader.ts.tv_sec, packet->timestamp.tv_sec);
   ASSERT_EQ(capturer.mBogusHeader.ts.tv_usec, packet->timestamp.tv_usec);
   string rawData;
   rawData.insert(0, reinterpret_cast<char*> (packet->data), packet->len);
   ASSERT_EQ(capturer.mBogusHeader.len, rawData.size());
   ASSERT_EQ(0, memcmp(capturer.mBogusPacket, rawData.c_str(), rawData.size()));
   free(packet->data);
   free(packet);
   capturer.mFakePCapRetVal = 0;
   ASSERT_EQ(0, clientThread.GetPacket(packet));
#endif

}

TEST_F(PacketCaptureTest, FailPackets) {
   QosmosDPI dpiEngine(mConf);
   PacketCaptureReceiver receiver(t_serverAddr);
   MockPacketCapturePCap capturer(receiver.GetZMQ(), t_interface, mConf);
   ctb_ppacket packet(NULL);

   MockPacketCapturePCapClientThread clientThread(&capturer, receiver.GetZMQ());
   std::vector<std::pair<void*, unsigned int> > packets;
   clientThread.FailPackets(packets);
   EXPECT_EQ(0, clientThread.GetInternalFailedPacketCount());
   clientThread.FailPacket(packet);
   EXPECT_EQ(1, clientThread.GetInternalFailedPacketCount());
   u_char* rawPacket;
   struct pcap_pkthdr *phdr;
   phdr = &mBogusHeader;
   timeval tv;
   tv.tv_sec = 12;
   tv.tv_usec = 34;
   phdr->ts = tv;
   phdr->caplen = 100;
   phdr->len = 100;
   rawPacket = new u_char[100];
   memset(mBogusPacket, 'a', 100);
   memcpy(rawPacket, mBogusPacket, 100);

   clientThread.ResetInternalFailedPacketCount();
   EXPECT_EQ(0, clientThread.GetInternalFailedPacketCount());
   mPacketAllocator.PopulatePacketData(rawPacket, phdr, packet);
   clientThread.FailPacket(packet);
   EXPECT_TRUE(packet == NULL);
   EXPECT_EQ(1, clientThread.GetInternalFailedPacketCount());
   mPacketAllocator.PopulatePacketData(rawPacket, phdr, packet);
   packets.push_back(make_pair(packet, 0));
   clientThread.FailPackets(packets);
   EXPECT_TRUE(packets.empty());
   EXPECT_EQ(2, clientThread.GetInternalFailedPacketCount());
   delete []rawPacket;
}
