#pragma once

#include "gtest/gtest.h"
#include "IPacketCapture.h"
#include "PacketCapturePCap.h"
#include "PacketCapturePCapClientThread.h"
#include "pcap.h"
#include "MockConfMaster.h"
#include "QosmosPacketAllocator.h"

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
      //std::cout << "teardownStart" << std::endl;
#ifdef LR_DEBUG
      if (geteuid() == 0) {
         raise(SIGTERM);
      }
#endif
      //std::cout << "teardownEnd" << std::endl;

   }
   std::string t_clientAddr;
   std::string t_serverAddr;
   int t_packetsReceived;
   int t_packetsDropped;
   int t_packetsIfDropped;
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
   mFailStartWorkers(false),
   mFailCreatePCapHandle(false),
   mFailSetPCapTimeout(false),
   mFailSetBufferSize(false),
   mFailSetPromiscuous(false),
   mFailActivatePCapHandle(false),
   mRealIsDone(false),
   mFailGetStats(false),
   mFakePutEvent(false),
   mFakePutEventTriggered(false),
   mFakePCap(false),
   mFakePCapRetVal(1) {
   }

   explicit MockPacketCapturePCap(std::string& sendToAddress, std::string& interface, std::string& filename, networkMonitor::ConfProcessor& conf) : PacketCapturePCap(sendToAddress, interface, filename, conf),
   m_numberCaptured(0),
   m_numberToCapture(0),
   m_numberOfEmptyReadsToTolerate(0),
   mFailStartWorkers(false),
   mFailCreatePCapHandle(false),
   mFailSetPCapTimeout(false),
   mFailSetBufferSize(false),
   mFailSetPromiscuous(false),
   mFailActivatePCapHandle(false),
   mRealIsDone(false),
   mFailGetStats(false),
   mFakePutEvent(false),
   mFakePutEventTriggered(false),
   mFakePCap(false),
   mFakePCapRetVal(1) {
   }

   explicit MockPacketCapturePCap(const ReceivePacketZMQ* TransferQueue, std::string& interface, networkMonitor::ConfProcessor& conf) : PacketCapturePCap(TransferQueue, interface, conf),
   m_numberCaptured(0),
   m_numberToCapture(0),
   m_numberOfEmptyReadsToTolerate(0),
   mFailStartWorkers(false),
   mFailCreatePCapHandle(false),
   mFailSetPCapTimeout(false),
   mFailSetBufferSize(false),
   mFailSetPromiscuous(false),
   mFailActivatePCapHandle(false),
   mRealIsDone(false),
   mFailGetStats(false),
   mFakePutEvent(false),
   mFakePutEventTriggered(false),
   mFakePCap(false),
   mFakePCapRetVal(1) {
   }

   virtual ~MockPacketCapturePCap() {
   }

   virtual bool StartWorkers() {
      if (mFailStartWorkers) {
         return false;
      }
      return PacketCapturePCap::StartWorkers();
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

   boost::posix_time::time_duration GetRetrySleepInterval() {
      return std::move(boost::posix_time::microseconds(5));
   }

   int GetStats(struct pcap_stat* ps) {
      if (mFailGetStats) {
         return -1;
      }
      return PacketCapturePCap::GetStats(ps);
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
   int GetPacketFromPCap(ctb_ppacket& packet, unsigned int& hash);

   unsigned int m_numberCaptured;
   unsigned int m_numberToCapture;
   unsigned int m_numberOfEmptyReadsToTolerate;
   bool mFailStartWorkers;
   bool mFailCreatePCapHandle;
   bool mFailSetPCapTimeout;
   bool mFailSetBufferSize;
   bool mFailSetPromiscuous;
   bool mFailActivatePCapHandle;
   bool mWarnActivatePCapHandle;
   bool mRealIsDone;
   bool mFailGetStats;
   bool mFakePutEvent;
   bool mFakePutEventTriggered;
   bool mFakePCap;
   pcap_pkthdr mBogusHeader;
   uint8_t mBogusPacket[100];
   int mFakePCapRetVal;
   static std::string mAnError;

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

class MockPacketCapturePCapClientThread : public PacketCapturePCapClientThread {
public:

   explicit MockPacketCapturePCapClientThread(PacketCapturePCap *masterThread) : PacketCapturePCapClientThread(masterThread) {
   }

   MockPacketCapturePCapClientThread(PacketCapturePCap *masterThread, const SendPacketZMQ& TransferQueue) :
   PacketCapturePCapClientThread(masterThread, TransferQueue) {
   }

   MockPacketCapturePCapClientThread(PacketCapturePCap *masterThread, const ReceivePacketZMQ* TransferQueue) :
   PacketCapturePCapClientThread(masterThread, TransferQueue) {
   }

   void FailPacket(ctb_ppacket& packet) {
      return PacketCapturePCapClientThread::FailPacket(packet);
   }

   void FailPackets(std::vector<std::pair<void*, unsigned int> >& packets) {
      return PacketCapturePCapClientThread::FailPackets(packets);
   }
};
