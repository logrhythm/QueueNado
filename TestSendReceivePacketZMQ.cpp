#include "TestSendReceivePacketZMQ.h"
using namespace std;

void TestSendReceivePacketZMQ::DummyReaderThread(PacketReaderThreadConfig *config) {

   if (!config->serverThread->Initialize()) {
      cout << "can't receive????" << endl;
      ASSERT_TRUE(false);
      return;
   }
   if (config->ready && !*config->ready) {
      *config->ready = true;
   }

   int packetsFound = 0;
   int targetPackets = config->totalPackets;
   while (packetsFound < targetPackets && *config->ready) {
      boost::this_thread::interruption_point();
      string data;
      while (!config->serverThread->ReceiveDataBlock(data, 1)) {
         boost::this_thread::interruption_point();
         // the call does a nanosleep

      }
      if (ZeroMQTests::gCurrentPacketSize == data.size()) {
         //cout << "data expected size " << gCurrentPacketSize <<  "==" << data.size() << endl;
         packetsFound++;
      } else {
         cout << "data not expected size " << gCurrentPacketSize << "!=" << data.size() << endl;
         ASSERT_TRUE(false);
      }
   }
   cout << "Finished receiving messages" << endl;

   //cout << "worker dead" << endl;
}

bool TestSendReceivePacketZMQ::RunBenchmark(size_t packets, const string& binding, size_t expectedSpeed) {

   t_data.assign((char*) t_bigData, t_bigDataSize);
   gCurrentPacketSize = t_bigDataSize;
   PacketReaderThreadConfig readerThreadConfig;
   readerThreadConfig.binding = binding;
   readerThreadConfig.totalPackets = packets;
   bool ready = false;
   readerThreadConfig.serverThread = new ReceivePacketZMQ(binding);
   readerThreadConfig.serverThread->Initialize();
   SendPacketZMQ aQueue(readerThreadConfig.serverThread->GetBinding());
   readerThreadConfig.clientThread = &aQueue;
   readerThreadConfig.ready = &ready;
   SetExpectedTime((readerThreadConfig.totalPackets), t_bigDataSize, expectedSpeed, 1000L);

   aQueue.Initialize();

   boost::thread mReaderThread;
   mReaderThread = boost::thread(&TestSendReceivePacketZMQ::DummyReaderThread, this, &readerThreadConfig);
   timespec timeToSleep;
   timeToSleep.tv_sec = 0;
   timeToSleep.tv_nsec = 100000;
   while (!ready) {
      boost::this_thread::interruption_point();
      nanosleep(&timeToSleep, NULL);
   }
   if (!aQueue.Initialize()) {
      cout << "Could not init queue" << endl;
      ready = false;
      mReaderThread.join();
      delete readerThreadConfig.serverThread;
      return false;
   }

   StartTimedSection();
   for (int i = 0; i < readerThreadConfig.totalPackets; i++) {
      memcpy(t_bigData, &i, sizeof (int));
      t_data.assign((char*) t_bigData, t_bigDataSize);
      if (!aQueue.SendData(t_data)) {
         ready = false;
         if (mReaderThread.joinable()) {
            mReaderThread.interrupt();
            mReaderThread.join();
         }
         break;
      }
   }
   EndTimedSection();
   bool succeeded = TimedSectionPassed();

   if (mReaderThread.joinable()) {
      mReaderThread.interrupt();
      mReaderThread.join();
   }
   delete readerThreadConfig.serverThread;
   timeToSleep.tv_sec = 1;
   timeToSleep.tv_nsec = 0;
   nanosleep(&timeToSleep, NULL);
   return succeeded;
}

TEST_F(TestSendReceivePacketZMQ, ConstructAndInitialize) {
   SendPacketZMQ aQueue(mTarget);
   ReceivePacketZMQ bQueue(mTarget);

   EXPECT_FALSE(aQueue.SendData(t_data));
   EXPECT_FALSE(bQueue.ReceiveDataBlock(t_data, 1));
   SendPacketZMQ* pAQueue = new SendPacketZMQ(mTarget);
   delete pAQueue;
   ReceivePacketZMQ* pBQueue = new ReceivePacketZMQ(mTarget);
   delete pBQueue;
}

TEST_F(TestSendReceivePacketZMQ, NoReader) {
   t_data.push_back(0xff);
   mTarget = "tcp://127.0.0.1:3131979";
   SendPacketZMQ aQueue(mTarget);
   ReceivePacketZMQ bQueue(mTarget);
   ASSERT_TRUE(aQueue.Initialize());
   EXPECT_FALSE(aQueue.SendData(t_data));
}

TEST_F(TestSendReceivePacketZMQ, SenderInitializeWellBehaved) {
   mTarget = "tcp://127.0.0.1:3131979";
   SendPacketZMQ aQueue(mTarget);
   ReceivePacketZMQ bQueue(mTarget);
   EXPECT_TRUE(aQueue.Initialize());
   EXPECT_TRUE(aQueue.Initialize());
   EXPECT_TRUE(bQueue.Initialize());
   EXPECT_TRUE(bQueue.Initialize());

}

TEST_F(TestSendReceivePacketZMQ, TestSenderDumbReceiver) {
   if (geteuid() == 0) {
      MakeSmallData();
      t_data.assign(t_smallData, t_smallData + 16);
      gCurrentPacketSize = 16;
      PacketReaderThreadConfig readerThreadConfig;
      stringstream ss;
      ss << pthread_self();
      readerThreadConfig.binding = "tcp://127.0.0.1:1000"; // + ss.str();
      readerThreadConfig.serverThread = new ReceivePacketZMQ(readerThreadConfig.binding);
      readerThreadConfig.totalPackets = 1;
      bool ready = false;
      readerThreadConfig.ready = &ready;
      boost::thread mReaderThread;
      mReaderThread = boost::thread(&TestSendReceivePacketZMQ::DummyReaderThread, this, &readerThreadConfig);
      SendPacketZMQ aQueue(readerThreadConfig.serverThread->GetBinding());
      ASSERT_TRUE(aQueue.Initialize());
      ASSERT_TRUE(aQueue.SendData(t_data));
      boost::this_thread::sleep(boost::posix_time::milliseconds(10));
      ASSERT_TRUE(mReaderThread.joinable());
      mReaderThread.join();
   }

}

TEST_F(TestSendReceivePacketZMQ, TestSenderDumbReceiverRapidFireTCPHugeSize) {
   if (geteuid() == 0) {
      MakeBigData(64 * 1024);
      stringstream ss;
      ss << getpid();
      string binding = "tcp://127.0.0.1:1000" + ss.str();
      EXPECT_TRUE(RunBenchmark((PACKETS_TO_TEST >> 4), binding, 1000L));
   }

}

TEST_F(TestSendReceivePacketZMQ, TestSenderDumbReceiverRapidFireIPCHugeSize) {
   if (geteuid() == 0) {

      MakeBigData(64 * 1024);
      stringstream ss;
      ss << getpid();
      string binding = "ipc:///tmp/zmqfeed0" + ss.str();
      EXPECT_TRUE(RunBenchmark((PACKETS_TO_TEST >> 4), binding, 1000L));
   }

}

TEST_F(TestSendReceivePacketZMQ, TestSenderDumbReceiverRapidFireTCPFullSize) {
   if (geteuid() == 0) {
      MakeBigData(1500);
      stringstream ss;
      ss << getpid();
      string binding = "tcp://127.0.0.1:1000" + ss.str();
      EXPECT_TRUE(RunBenchmark((PACKETS_TO_TEST), binding, 100L));

   }

}

TEST_F(TestSendReceivePacketZMQ, TestSenderDumbReceiverRapidFireIPCFullSize) {
   if (geteuid() == 0) {
      MakeBigData(1500);
      stringstream ss;
      ss << getpid();
      string binding = "ipc:///tmp/zmqfeed0" + ss.str();
      EXPECT_TRUE(RunBenchmark((PACKETS_TO_TEST), binding, 100L));

   }

}

TEST_F(TestSendReceivePacketZMQ, TestSenderDumbReceiverRapidFireTCPMedSize) {
   if (geteuid() == 0) {
      MakeBigData(512);
      stringstream ss;
      ss << getpid();
      string binding = "tcp://127.0.0.1:1000" + ss.str();
      EXPECT_TRUE(RunBenchmark((PACKETS_TO_TEST), binding, 100L));
   }
}

TEST_F(TestSendReceivePacketZMQ, TestSenderDumbReceiverRapidFireIPCMedSize) {
   if (geteuid() == 0) {
      MakeBigData(512);
      stringstream ss;
      ss << getpid();
      string binding = "ipc:///tmp/zmqfeed0" + ss.str();
      EXPECT_TRUE(RunBenchmark((PACKETS_TO_TEST), binding, 100L));
   }
}

TEST_F(TestSendReceivePacketZMQ, TestSenderDumbReceiverRapidFireTCPSmallSize) {
   if (geteuid() == 0) {
      MakeBigData(32);
      stringstream ss;
      ss << getpid();
      string binding = "tcp://127.0.0.1:1000" + ss.str();
      EXPECT_TRUE(RunBenchmark((PACKETS_TO_TEST << 1), binding, 10L));
   }
}

TEST_F(TestSendReceivePacketZMQ, TestSenderDumbReceiverRapidFireIPCSmallSize) {
   if (geteuid() == 0) {
      MakeBigData(32);
      stringstream ss;
      ss << getpid();
      string binding = "ipc:///tmp/zmqfeed0" + ss.str();
      EXPECT_TRUE(RunBenchmark((PACKETS_TO_TEST << 1), binding, 10L));
   }
}
