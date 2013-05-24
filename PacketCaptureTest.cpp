#include "IPacketCapture.h"
#include "PacketCaptureReceiver.h"
#include "PacketCaptureTest.h"
#include "PacketCapturePCapClientThread.h"
#include <sys/socket.h>
#include <algorithm>

using namespace std;
string MockPacketCapturePCap::mAnError = "this is an error";

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

int MockPacketCapturePCap::GetPacketFromPCap(ctb_ppacket& packet, unsigned int& hash) {
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

         hash = mPacketAllocator.PopulatePacketData(rawPacket, phdr, packet);
         delete []rawPacket;
         return 1;
      } else {
         return mFakePCapRetVal;
      }

   }
   returnVal = PacketCapturePCap::GetPacketFromPCap(packet, hash);
   if (returnVal > 0) {
      m_numberCaptured++;
   }
   return returnVal;
}

TEST_F(PacketCaptureTest, ConstructAndInitialize) {
   if (geteuid() == 0) {
#ifdef LR_DEBUG
      PacketCaptureReceiver receiver(t_serverAddr);
      MockPacketCapturePCap capturer(t_clientAddr, t_interface, mConf);

      capturer.SetNumberOfPacketToCapture(0);
      ctb_ppacket packet;
      unsigned int hash(0);
      ASSERT_EQ(-1, capturer.GetPacketFromPCap(packet, hash));
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
      int I, dont, care;
      size_t atall;
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

TEST_F(PacketCaptureTest, GetSomethingFromTheInterfaceSendOverExisting) {
#ifdef LR_DEBUG
   if (geteuid() == 0) {

      PacketCaptureReceiver receiver(t_serverAddr);
      MockPacketCapturePCap capturer(receiver.GetZMQ(), t_interface, mConf);

      ASSERT_TRUE(capturer.Initialize());

      capturer.DisplayStats(t_packetsReceived, t_packetsDropped,
              t_packetsIfDropped, t_totalData);
      //cout << "4" << endl;
      //zclock_sleep(100);
      capturer.Shutdown(true);
      //cout << "5" << endl;
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
   PacketCapturePCap packetCapturer(t_clientAddr, t_interface, filename, mConf);
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
   PacketCapturePCapClientThread pThreadClass(&capturer);
}

TEST_F(PacketCaptureTest, PacketCaptureDynamicConstruction) {
   PacketCaptureReceiver receiver(t_serverAddr);
   PacketCapturePCap capturer(receiver.GetZMQ(), t_interface, mConf);
   PacketCapturePCapClientThread* pThreadClass =
           new PacketCapturePCapClientThread(&capturer);
   delete pThreadClass;
   pThreadClass = new PacketCapturePCapClientThread(&capturer);
   delete pThreadClass;
}


TEST_F(PacketCaptureTest, PacketCapturePCapClientThread_GetPackets) {
#ifdef LR_DEBUG
   PacketCaptureReceiver receiver(t_serverAddr);
   MockPacketCapturePCap capturer(receiver.GetZMQ(), t_interface, mConf);

   MockPacketCapturePCapClientThread clientThread(&capturer, receiver.GetZMQ());

   ctb_ppacket packet(NULL);
   unsigned int hash(0);
   capturer.mFakePCap = true;
   ASSERT_EQ(1, clientThread.GetPacket(packet, hash));
   ASSERT_EQ(capturer.mBogusHeader.ts.tv_sec, packet->timestamp.tv_sec);
   ASSERT_EQ(capturer.mBogusHeader.ts.tv_usec, packet->timestamp.tv_usec);
   string rawData;
   rawData.insert(0, reinterpret_cast<char*> (packet->data), packet->len);
   ASSERT_EQ(capturer.mBogusHeader.len, rawData.size());
   ASSERT_EQ(0, memcmp(capturer.mBogusPacket, rawData.c_str(), rawData.size()));
   
   capturer.mFakePCapRetVal = 0;
   ASSERT_EQ(0, clientThread.GetPacket(packet, hash));
#endif

}

TEST_F(PacketCaptureTest, FailPackets) {
   PacketCaptureReceiver receiver(t_serverAddr);
   MockPacketCapturePCap capturer(receiver.GetZMQ(), t_interface, mConf);
   ctb_ppacket packet(NULL);
   unsigned int hash(0);

   MockPacketCapturePCapClientThread clientThread(&capturer, receiver.GetZMQ());
   std::vector<std::pair<void*, unsigned int> > packets;
   clientThread.FailPackets(packets);
   clientThread.FailPacket(packet);
   uint8_t* rawPacket;
   struct pcap_pkthdr *phdr;
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

   hash = mPacketAllocator.PopulatePacketData(rawPacket, phdr, packet);
   clientThread.FailPacket(packet);
   EXPECT_TRUE(packet==NULL);
   hash = mPacketAllocator.PopulatePacketData(rawPacket, phdr, packet);
   packets.push_back(make_pair(packet,hash));
   clientThread.FailPackets(packets);
   EXPECT_TRUE(packets.empty());
   delete []rawPacket;
}
