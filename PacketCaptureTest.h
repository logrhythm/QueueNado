#pragma once

#include "gtest/gtest.h"
#include "IPacketCapture.h"
#include "PacketCapturePCap.h"
#include "PacketCapturePCapClientThread.h"
#include "pcap.h"
#include "MockConfMaster.h"
#include "QosmosPacketAllocator.h"
#include "MockSendStats.h"
#include "SendStats.h"
#include <memory>

class PacketCaptureTest : public ::testing::Test {
public:

   PacketCaptureTest() :
   mPacketAllocator(QosmosPacketAllocator::Instance()) {
   }
protected:

   virtual void SetUp() {
      t_packetsReceived = 0;
      t_packetsDropped = 0;
      t_packetsIfDropped = 0;
      std::vector<std::string> interfaces;
      if (!(interfaces = t_pcapInfo.GetDefaultInterface()).empty()) {
         t_interface = t_pcapInfo.GetDefaultInterface()[0];
      } else {
         t_interface = "";
      }
      zctx_interrupted = false;

      int max = 9000;
      int min = 7000;
      int port = (rand() % (max - min)) + min;
      std::string tcpLocation("tcp://127.0.0.1:");
      tcpLocation.append(boost::lexical_cast<std::string > (port));
      t_serverAddr.assign(tcpLocation.c_str());
      t_clientAddr.assign(tcpLocation.c_str());
   }
   std::string t_interface;

   virtual void TearDown() {

   }
   std::string t_clientAddr;
   std::string t_serverAddr;
   size_t t_packetsReceived;
   size_t t_packetsDropped;
   size_t t_packetsIfDropped;
   size_t t_totalData;
   PCapInfo t_pcapInfo;
   networkMonitor::MockConfMaster mConf;
   QosmosPacketAllocator& mPacketAllocator;
   pcap_pkthdr mBogusHeader;
   uint8_t mBogusPacket[100];
};

class MockPacketCapturePCap : public PacketCapturePCap {
public:

   explicit MockPacketCapturePCap(std::string& sendToAddress, std::string& interface, networkMonitor::ConfProcessor& conf) : PacketCapturePCap(sendToAddress, interface, conf),
   m_numberCaptured(0),
   m_numberToCapture(0),
   m_numberOfEmptyReadsToTolerate(0),
   mFailCreatePCapHandle(false),
   mFailSetPCapTimeout(false),
   mFailSetBufferSize(false),
   mFailSetPromiscuous(false),
   mFailActivatePCapHandle(false),
   mRealIsDone(false),
   mFailGetStats(false),
   mPsRecv(0),
   mPsDrop(0),
   mPsIfDrop(0),
   mFakePutEvent(false),
   mFakePutEventTriggered(false),
   mFakePCap(false),
   mFakePCapRetVal(1),
   mDispatchReturn(0),
   mDispatchCount(1) {
   }

   explicit MockPacketCapturePCap(std::string& filename, networkMonitor::ConfProcessor& conf) : PacketCapturePCap(filename, conf),
   m_numberCaptured(0),
   m_numberToCapture(0),
   m_numberOfEmptyReadsToTolerate(0),
   mFailCreatePCapHandle(false),
   mFailSetPCapTimeout(false),
   mFailSetBufferSize(false),
   mFailSetPromiscuous(false),
   mFailActivatePCapHandle(false),
   mRealIsDone(false),
   mFailGetStats(false),
   mPsRecv(0),
   mPsDrop(0),
   mPsIfDrop(0),
   mFakePutEvent(false),
   mFakePutEventTriggered(false),
   mFakePCap(false),
   mFakePCapRetVal(1),
   mDispatchReturn(0),
   mDispatchCount(1) {
   }

   explicit MockPacketCapturePCap(const ReceivePacketZMQ* TransferQueue, std::string& interface, networkMonitor::ConfProcessor& conf) : PacketCapturePCap(TransferQueue, interface, conf),
   m_numberCaptured(0),
   m_numberToCapture(0),
   m_numberOfEmptyReadsToTolerate(0),
   mFailCreatePCapHandle(false),
   mFailSetPCapTimeout(false),
   mFailSetBufferSize(false),
   mFailSetPromiscuous(false),
   mFailActivatePCapHandle(false),
   mRealIsDone(false),
   mFailGetStats(false),
   mPsRecv(0),
   mPsDrop(0),
   mPsIfDrop(0),
   mFakePutEvent(false),
   mFakePutEventTriggered(false),
   mFakePCap(false),
   mFakePCapRetVal(1),
   mDispatchReturn(0),
   mDispatchCount(1) {
   }

   virtual ~MockPacketCapturePCap() {
   }

   virtual std::string LogLibPCapVersion() {
      return PacketCapturePCap::LogLibPCapVersion();
   }

   virtual pcap_t* CreatePCapHandle() {
      if (mFailCreatePCapHandle) {
         return NULL;
      }
      return PacketCapturePCap::CreatePCapHandle();
   }

   virtual int SetPCapTimeout(pcap_t* pcaph, int timeout) {
      if (mFailSetPCapTimeout) {
         return -1;
      }
      return PacketCapturePCap::SetPCapTimeout(pcaph, timeout);
   }

   virtual int SetBufferSize(pcap_t* pcaph, int bufsizeInMB) {
      if (mFailSetBufferSize) {
         return -1;
      }
      return PacketCapturePCap::SetBufferSize(pcaph, bufsizeInMB);
   }

   virtual int SetPromiscuous(pcap_t* pcaph) {
      if (mFailSetPromiscuous) {
         return -1;
      }
      return PacketCapturePCap::SetPromiscuous(pcaph);
   }

   virtual int ActivatePCapHandle(pcap_t* pcaph) {
      if (mFailActivatePCapHandle) {
         return -1;
      }
      return PacketCapturePCap::ActivatePCapHandle(pcaph);
   }

   virtual const char* GetPCapErrorFromDevice(pcap_t* pcaph) {
      return mAnError.c_str();
   }

   bool WasActivationSuccessful(pcap_t* pcaph, int status) {
      return PacketCapturePCap::WasActivationSuccessful(pcaph, status);
   }

   int GetNumberOfRetries() {
      return 2;
   }

   void SetDispatchReturn(const int code) {
      mDispatchReturn = code;
   }

   void SetDispatchCount(const int count) {
      mDispatchCount = count;
   }

   boost::posix_time::time_duration GetRetrySleepInterval() {
      return std::move(boost::posix_time::microseconds(5));
   }

   int GetStats(struct pcap_stat* ps) {
      if (mFailGetStats) {
         return -1;
      } else {
         ps->ps_recv = mPsRecv;
         ps->ps_drop = mPsDrop;
         ps->ps_ifdrop = mPsIfDrop;
         return 0;
      }
   }

   void PutEvent() {
      if (mFakePutEvent) {
         mFakePutEventTriggered = true;
         return;
      }
      //PacketCapturePCap::PutEvent();
   }
   virtual bool IsDone();
   void SetNumberOfPacketToCapture(unsigned int number);
   unsigned int GetNumberOfPacketsCaptured();

   virtual int PcapDispatch(pcap_t * pcapT, int count, pcap_handler handler, u_char * uData) LR_OVERRIDE {

      for (int i = 1; i <= mDispatchCount; ++i) {
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
         PacketCapturePCap::process_packets(uData, phdr, rawPacket);
         if(rawPacket) {
            free(rawPacket);
         }
      }
      return mDispatchReturn;
   }
   int GetPacketFromPCap(ctb_ppacket& packet);

   void IncrementTotalData(int64_t incomingData) LR_OVERRIDE {
      PacketCapturePCap::IncrementTotalData(incomingData);
   }

   SendStats& GetSendStatForwarder() LR_OVERRIDE {
      if (mMockSendStatsForwarder) {
         return *(mMockSendStatsForwarder.get());
      }
      return PacketCapturePCap::GetSendStatForwarder();
   }

   void SetupSendStatForwarder() {
      mMockSendStatsForwarder.reset(new MockSendStats);
   }


   unsigned int m_numberCaptured;
   unsigned int m_numberToCapture;
   unsigned int m_numberOfEmptyReadsToTolerate;
   bool mFailCreatePCapHandle;
   bool mFailSetPCapTimeout;
   bool mFailSetBufferSize;
   bool mFailSetPromiscuous;
   bool mFailActivatePCapHandle;
   bool mWarnActivatePCapHandle;
   bool mRealIsDone;
   bool mFailGetStats;


   uint32_t mPsRecv;
   uint32_t mPsDrop;
   uint32_t mPsIfDrop;
   bool mFakePutEvent;
   bool mFakePutEventTriggered;
   bool mFakePCap;
   pcap_pkthdr mBogusHeader;
   uint8_t mBogusPacket[100];
   int mFakePCapRetVal;
   int mDispatchReturn;
   int mDispatchCount;
   static std::string mAnError;

private:
   std::unique_ptr<SendStats> mMockSendStatsForwarder;


};

class MockPCapInfo : public PCapInfo {
public:

   MockPCapInfo() : PCapInfo::PCapInfo(),
   mFailPopulate(false),
   mNothingFound(false),
   mFailIsLoopback(false),
   mPassIsIPReadable(false),
   mFailGetDefaultDevice(false),
   aName(NULL),
   mQuickInit(false),
   mAlwaysLoopBack(false),
   mNeverReadable(false) {
   }

   virtual ~MockPCapInfo() {
      if (aName) {
         delete [] aName;
      }
   }

   virtual int PopulateInterfaces() {
      if (mFailPopulate) {
         return -1;
      } else if (mNothingFound) {
         m_allDevices = NULL;
         return 0;
      }
      return PCapInfo::PopulateInterfaces();
   }

   virtual bool IsSupported(pcap_if_t* currentDev) {
      return PCapInfo::IsSupported(currentDev);
   }

   virtual bool IsLoopback(u_int flags) {
      if (mFailIsLoopback) {
         return false;
      } else if (mAlwaysLoopBack) {
         return true;
      }

      return PCapInfo::IsLoopback(flags);
   }

   virtual bool IsIPReadable(struct sockaddr* sa) {
      if (mPassIsIPReadable) {
         return true;
      } else if (mNeverReadable) {
         return false;
      }
      return PCapInfo::IsIPReadable(sa);
   }

   virtual char* GetDefaultDevice() {
      if (mFailGetDefaultDevice) {
         return NULL;
      }
      if (!aName) {
         std::string name("ethtest");
         aName = new char[100];
         strncpy(aName, name.c_str(), name.size());
         aName[name.size()] = '\0';
      }
      return aName;
   }

   virtual bool InitializePCapInfo() {
      if (mQuickInit) {
         return (m_initialized = true);

      }
      return PCapInfo::InitializePCapInfo();
   }
   bool mFailPopulate;
   bool mNothingFound;
   bool mFailIsLoopback;
   bool mPassIsIPReadable;
   bool mFailGetDefaultDevice;
   char* aName;
   bool mQuickInit;
   bool mAlwaysLoopBack;
   bool mNeverReadable;
};

class MockPacketCapturePCapClientThread : public PacketReader {
public:

   explicit MockPacketCapturePCapClientThread(PacketCapturePCap *masterThread) : PacketReader(masterThread) {
   }

   MockPacketCapturePCapClientThread(PacketCapturePCap *masterThread, const SendPacketZMQ& TransferQueue) :
   PacketReader(masterThread, TransferQueue) {
   }

   MockPacketCapturePCapClientThread(PacketCapturePCap *masterThread, const ReceivePacketZMQ* TransferQueue) :
   PacketReader(masterThread, TransferQueue) {
   }

   void FailPacket(ctb_ppacket& packet) {
      return PacketReader::FailPacket(packet);
   }

   void FailPackets(std::vector<std::pair<void*, unsigned int> >& packets) {
      return PacketReader::FailPackets(packets);
   }

};
