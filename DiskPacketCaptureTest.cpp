
#include "DiskPacketCaptureTest.h"

#include "dpi/modules/upacket.h"
#include "pcap/pcap.h"
#include <thread>
#include <future>

#ifdef LR_DEBUG
#include "MockConf.h"
#include "MockDiskPacketCapture.h"
#else
#include "Conf.h"
#include "DiskPacketCapture.h"
#endif

#ifdef LR_DEBUG

#include "tempFileCreate.h"

TEST_F(DiskPacketCaptureTest, IntegrationTestWithSizeLimitNothingPrior) {
   MockConf conf;
   conf.mUnknownCaptureEnabled = true;
   conf.mPCapCaptureLocation = testDir.str();
   conf.mMaxIndividualPCap = 10; // MB
   conf.mPCapCaptureMemoryLimit = 99999;
   conf.mPcapCaptureMaxPackets = 99999999;
   conf.mPCapCaptureSizeLimit = 999999999;
   const size_t testPacketSize = 1024 * 1024;
   MockDiskPacketCapture capture(conf);

   ASSERT_TRUE(capture.Initialize(false));
   networkMonitor::DpiMsgLR testMessage;
   struct upacket packet;

   testMessage.set_sessionid("123456789012345678901234567890123456");
   testMessage.set_packettotal(0);
   testMessage.set_packetsdelta(0);
   testMessage.set_bytessource(0);
   testMessage.set_bytesdest(0);
   testMessage.set_bytestotal(0);
   testMessage.set_bytestotaldelta(0);



   packet.p = reinterpret_cast<ctb_ppacket> (malloc(sizeof (ctb_pkt))); // 1MB packet
   packet.p->data = reinterpret_cast<ctb_uint8*> (malloc(testPacketSize)); // 1MB packet
   packet.p->len = (testPacketSize);
   for (int i = 0; i < conf.mMaxIndividualPCap + 2; i++) {
      testMessage.set_packettotal(testMessage.packettotal() + 1);
      testMessage.set_packetsdelta(testMessage.packetsdelta() + 1);
      if (i % 2 == 0) {
         testMessage.set_bytessource(testMessage.bytessource() + packet.p->len);
      } else {
         testMessage.set_bytesdest(testMessage.bytesdest() + packet.p->len);
      }
      capture.SavePacket(&testMessage, &packet);
   }
   EXPECT_TRUE(capture.WriteSavedSessionToDisk(&testMessage));
   EXPECT_EQ(conf.mMaxIndividualPCap + 2, testMessage.packettotal());
   EXPECT_EQ((conf.mMaxIndividualPCap + 2)*(testPacketSize), testMessage.bytessource() + testMessage.bytesdest());
   struct stat statbuf;

   std::string testFile = testDir.str() + "/" + testMessage.sessionid();
   ASSERT_EQ(0, stat(testFile.c_str(), &statbuf));
   EXPECT_TRUE(conf.mMaxIndividualPCap * testPacketSize >= statbuf.st_size);
   EXPECT_TRUE((conf.mMaxIndividualPCap - 1) * testPacketSize <= statbuf.st_size);
   
   remove(testFile.c_str());
   for (int i = 0; i < conf.mMaxIndividualPCap + 2; i++) {
      testMessage.set_packettotal(testMessage.packettotal() + 1);
      testMessage.set_packetsdelta(testMessage.packetsdelta() + 1);
      if (i % 2 == 0) {
         testMessage.set_bytessource(testMessage.bytessource() + packet.p->len);
      } else {
         testMessage.set_bytesdest(testMessage.bytesdest() + packet.p->len);
      }
      capture.SavePacket(&testMessage, &packet);
      EXPECT_FALSE(capture.WriteSavedSessionToDisk(&testMessage));
   }
   
   EXPECT_EQ(2*(conf.mMaxIndividualPCap + 2), testMessage.packettotal());
   EXPECT_EQ(2*(conf.mMaxIndividualPCap + 2)*(testPacketSize), testMessage.bytessource() + testMessage.bytesdest());

   ASSERT_NE(0, stat(testFile.c_str(), &statbuf));
   
   free(packet.p->data);
   free(packet.p);
}

TEST_F(DiskPacketCaptureTest, IntegrationTestWithSizeLimitFlushedFile) {
   MockConf conf;
   conf.mUnknownCaptureEnabled = true;
   conf.mPCapCaptureLocation = testDir.str();
   conf.mMaxIndividualPCap = 10; // MB
   conf.mPCapCaptureMemoryLimit = 99999;
   conf.mPcapCaptureMaxPackets = 99999999;
   conf.mPCapCaptureSizeLimit = 999999999;
   const size_t testPacketSize = 1024 * 1024;
   MockDiskPacketCapture capture(conf);

   ASSERT_TRUE(capture.Initialize(false));
   networkMonitor::DpiMsgLR testMessage;
   struct upacket packet;

   testMessage.set_sessionid("123456789012345678901234567890123456");
   testMessage.set_packettotal(0);
   testMessage.set_packetsdelta(0);
   testMessage.set_bytessource(9 * testPacketSize);
   testMessage.set_bytesdest(0);
   testMessage.set_bytestotal(0);
   testMessage.set_bytestotaldelta(0);



   packet.p = reinterpret_cast<ctb_ppacket> (malloc(sizeof (ctb_pkt))); // 1MB packet
   packet.p->data = reinterpret_cast<ctb_uint8*> (malloc(testPacketSize)); // 1MB packet
   packet.p->len = (testPacketSize);
   for (int i = 0; i < conf.mMaxIndividualPCap + 2; i++) {
      testMessage.set_packettotal(testMessage.packettotal() + 1);
      testMessage.set_packetsdelta(testMessage.packetsdelta() + 1);
      if (i % 2 == 0) {
         testMessage.set_bytessource(testMessage.bytessource() + packet.p->len);
      } else {
         testMessage.set_bytesdest(testMessage.bytesdest() + packet.p->len);
      }
      capture.SavePacket(&testMessage, &packet);
      capture.WriteSavedSessionToDisk(&testMessage);
   }
   struct stat statbuf;

   std::string testFile = testDir.str() + "/" + testMessage.sessionid();
   ASSERT_EQ(0, stat(testFile.c_str(), &statbuf));
   EXPECT_TRUE(testPacketSize >= statbuf.st_size);
   EXPECT_TRUE(0 <= statbuf.st_size);
   free(packet.p->data);
   free(packet.p);
}

TEST_F(DiskPacketCaptureTest, ConstructAndDeconstructFilename) {
   MockConf conf;
   conf.mUnknownCaptureEnabled = true;
   MockDiskPacketCapture capture(conf);

   std::string filename = capture.BuildFilename("161122fd-6681-42a3-b953-48beb5247172");

   ASSERT_FALSE(filename.empty());
   EXPECT_EQ("161122fd-6681-42a3-b953-48beb5247172", filename);
   DiskPacketCapture::PacketCaptureFileDetails fileDetails;
   ASSERT_TRUE(capture.ParseFilename(filename, fileDetails));
   EXPECT_EQ("161122fd-6681-42a3-b953-48beb5247172", fileDetails.sessionId);

   filename = capture.BuildFilename("notAUuid");
   ASSERT_FALSE(filename.empty());
   EXPECT_EQ("notAUuid", filename);
   ASSERT_FALSE(capture.ParseFilename(filename, fileDetails));
}

/**
 * File names need to be 36 characters
 */
TEST_F(DiskPacketCaptureTest, ConstructAndDeconstructFilenameFail) {
   MockConf conf;
   conf.mUnknownCaptureEnabled = true;
   MockDiskPacketCapture capture(conf);

   std::string filename = capture.BuildFilename("testuuid");

   ASSERT_FALSE(filename.empty());
   EXPECT_EQ("testuuid", filename);
   DiskPacketCapture::PacketCaptureFileDetails fileDetails;
   ASSERT_FALSE(capture.ParseFilename(filename, fileDetails));
   EXPECT_EQ("", fileDetails.sessionId);
}
#endif

TEST_F(DiskPacketCaptureTest, Construct) {
#ifdef LR_DEBUG
   MockConf conf;
   conf.mUnknownCaptureEnabled = true;
   DiskPacketCapture capture(conf);
   DiskPacketCapture* pCapture = new DiskPacketCapture(conf);
   delete pCapture;
#endif
}

TEST_F(DiskPacketCaptureTest, GetRunningPackets) {
#ifdef LR_DEBUG
   MockConf conf;
   conf.mUnknownCaptureEnabled = true;
   MockDiskPacketCapture capture(conf);

   {
      tempFileCreate tempFile(conf);
      ASSERT_TRUE(tempFile.Init());
      conf.mPCapCaptureFileLimit = 10000;
      conf.mPCapCaptureSizeLimit = 10000;
      ASSERT_TRUE(capture.Initialize());
      SessionInfo* sessionInfo;
      SessionInfo* sessionInfo2;
      capture.GetRunningPackets("abc123", sessionInfo);
      capture.GetRunningPackets("abc123", sessionInfo2);
      ASSERT_EQ(sessionInfo, sessionInfo2);
      capture.GetRunningPackets("def456", sessionInfo2);
      ASSERT_NE(sessionInfo, sessionInfo2);

      //A promise object that can be used for sharing a variable between threads
      std::promise<SessionInfo* > promisedPacket;

      // A future variable that reads from the promise, allows you to wait on the other thread
      std::future<SessionInfo* > futurePacket = promisedPacket.get_future();

      // A thread that does a call to the capture object, stores the result in the promise.
      //
      //  This uses what is refered to as a lambda function ( expressed as [] ).  This is a full function
      // Defined as an L-Value of call.  These are broken down as follows:
      // [] ( Argument definitions ) { Normal C++ code }
      //
      // The remaining arguments of the thread constructor are the list of arguments.  using std::ref you allow
      // the arguments to be references rather than pointers in this context.  
      //
      // Finally the .detach() breaks the thread off to execute in the background
      std::thread([](std::promise<SessionInfo* >& pointer,
              MockDiskPacketCapture & capture) {
         SessionInfo* sessionInfo3;
         capture.GetRunningPackets("abc123", sessionInfo3);
                 pointer.set_value(sessionInfo3); }, std::ref(promisedPacket), std::ref(capture)).detach();

      // Wait for the thread to set the promise object to a new value
      futurePacket.wait();

      // Verify that the code run in the thread gives you a different pointer 
      capture.GetRunningPackets("abc123", sessionInfo);
      SessionInfo* otherRunningPacket = futurePacket.get();
      EXPECT_NE(sessionInfo, otherRunningPacket);
   }

#endif
}

TEST_F(DiskPacketCaptureTest, GetFilenamesTest) {
#ifdef LR_DEBUG
   MockConf conf;
   conf.mUnknownCaptureEnabled = true;
   MockDiskPacketCapture capture(conf);

   conf.mPCapCaptureLocation = "testLocation";
   std::string fileName = capture.BuildFilenameWithPath("TestUUID");
   ASSERT_EQ("testLocation/TestUUID", fileName);
#endif
}

TEST_F(DiskPacketCaptureTest, Initialize) {
#ifdef LR_DEBUG
   MockConf conf;
   conf.mUnknownCaptureEnabled = true;
   MockDiskPacketCapture capture(conf);
   {
      tempFileCreate tempFile(conf);
      ASSERT_TRUE(tempFile.Init());
      conf.mPCapCaptureLocation = "testLocation";
      ASSERT_FALSE(capture.Initialize());
      conf.mPCapCaptureLocation = tempFile.mTestDir.str();
      ASSERT_TRUE(capture.Initialize());
      conf.mPCapCaptureLocation = "/etc/passwd";
      ASSERT_FALSE(capture.Initialize());
   }
#endif 
}

TEST_F(DiskPacketCaptureTest, MemoryLimits) {
#ifdef LR_DEBUG
   MockConf conf;
   conf.mUnknownCaptureEnabled = true;
   MockDiskPacketCapture capture(conf);
   conf.mPCapCaptureLocation = "testLocation";
   conf.mPCapCaptureFileLimit = 10000;
   conf.mPCapCaptureSizeLimit = 10000;
   conf.mPCapCaptureMemoryLimit = 10;
   {
      tempFileCreate tempFile(conf);
      ASSERT_TRUE(tempFile.Init());
      ASSERT_TRUE(capture.Initialize());

      EXPECT_EQ(0, capture.NewTotalMemory(0));
      EXPECT_EQ(0, capture.CurrentMemoryForFlow("notThere"));
      struct upacket packet;
      ctb_pkt p;
      packet.p = &p;
      unsigned char data[(1024 * 1024) - sizeof (struct pcap_pkthdr)];
      p.len = (1024 * 1024) - sizeof (struct pcap_pkthdr);
      p.data = data;
      networkMonitor::DpiMsgLR dpiMsg1;
      dpiMsg1.set_sessionid("FlowOne");
      capture.SavePacket(&dpiMsg1, &packet);
      EXPECT_EQ(1, capture.NewTotalMemory(0));
      EXPECT_EQ(1, capture.CurrentMemoryForFlow("FlowOne"));
      p.len = (1024 * 1024) - sizeof (struct pcap_pkthdr) - sizeof (struct pcap_pkthdr) - 1;
      capture.SavePacket(&dpiMsg1, &packet);
      EXPECT_EQ(1, capture.NewTotalMemory(0));
      EXPECT_EQ(1, capture.CurrentMemoryForFlow("FlowOne"));
      p.len = 1;
      networkMonitor::DpiMsgLR dpiMsg2;
      dpiMsg2.set_sessionid("FlowTwo");
      capture.SavePacket(&dpiMsg2, &packet);
      EXPECT_EQ(2, capture.NewTotalMemory(0));
      EXPECT_EQ(1, capture.CurrentMemoryForFlow("FlowOne"));
      EXPECT_EQ(0, capture.CurrentMemoryForFlow("FlowTwo"));

      conf.mPCapCaptureMemoryLimit = 2;
      p.len = (1024 * 1024) - sizeof (struct pcap_pkthdr);
      capture.mFailFlush = true;
      capture.SavePacket(&dpiMsg2, &packet);
      EXPECT_EQ(2, capture.NewTotalMemory(0));
      EXPECT_EQ(1, capture.CurrentMemoryForFlow("FlowOne"));
      EXPECT_EQ(0, capture.CurrentMemoryForFlow("FlowTwo"));

      conf.mPCapCaptureMemoryLimit = 3;
      EXPECT_EQ(3, capture.NewTotalMemory((1024 * 1024)));

      capture.RemoveDataFromRunningPackets(&dpiMsg1);
      EXPECT_EQ(0, capture.NewTotalMemory(0));
      EXPECT_EQ(0, capture.CurrentMemoryForFlow("FlowOne"));

   }
   ASSERT_FALSE(capture.Initialize());
#endif
}

TEST_F(DiskPacketCaptureTest, AutoFlushOnMemoryLimit) {
#ifdef LR_DEBUG
   MockConf conf;
   conf.mUnknownCaptureEnabled = true;
   MockDiskPacketCapture capture(conf);
   conf.mPCapCaptureLocation = "testLocation";
   conf.mPCapCaptureFileLimit = 10000;
   conf.mPCapCaptureSizeLimit = 10000;
   conf.mPCapCaptureMemoryLimit = 10;
   {
      tempFileCreate tempFile(conf);
      ASSERT_TRUE(tempFile.Init());
      ASSERT_TRUE(capture.Initialize());

      EXPECT_EQ(0, capture.NewTotalMemory(0));
      EXPECT_EQ(0, capture.CurrentMemoryForFlow("notThere"));
      struct upacket packet;
      ctb_pkt p;
      packet.p = &p;
      unsigned char data[(1024 * 1024) - sizeof (struct pcap_pkthdr)];
      p.len = (1024 * 1024) - sizeof (struct pcap_pkthdr);
      p.data = data;
      networkMonitor::DpiMsgLR dpiMsg;
      dpiMsg.set_sessionid("FlowOne");
      capture.SavePacket(&dpiMsg, &packet);
      p.len = (1024 * 1024) - sizeof (struct pcap_pkthdr) - sizeof (struct pcap_pkthdr) - 1;
      capture.SavePacket(&dpiMsg, &packet);
      p.len = 1;
      dpiMsg.set_sessionid("FlowTwo");
      capture.SavePacket(&dpiMsg, &packet);
      EXPECT_EQ(2, capture.NewTotalMemory(0));
      EXPECT_EQ(1, capture.CurrentMemoryForFlow("FlowOne"));
      EXPECT_EQ(0, capture.CurrentMemoryForFlow("FlowTwo"));

      conf.mPCapCaptureMemoryLimit = 2;
      p.len = (1024 * 1024) - sizeof (struct pcap_pkthdr);
      capture.mFailFlush = false;
      capture.SavePacket(&dpiMsg, &packet);
      ASSERT_EQ(1, capture.mFilesWritten.size());
      // cleanup is an unordered map, the flow that gets picked could very based on the 
      // pointer hash
      if ("FlowOne" == *capture.mFilesWritten.begin()) {
         EXPECT_EQ(0, capture.CurrentMemoryForFlow("FlowOne"));
         EXPECT_EQ(1, capture.CurrentMemoryForFlow("FlowTwo"));
      } else {
         EXPECT_EQ(1, capture.CurrentMemoryForFlow("FlowOne"));
         EXPECT_EQ(0, capture.CurrentMemoryForFlow("FlowTwo"));
      }
      EXPECT_EQ(1, capture.NewTotalMemory(0));

   }
   ASSERT_FALSE(capture.Initialize());
#endif
}
#ifdef LR_DEBUG

TEST_F(DiskPacketCaptureTest, IndividualFileLimit) {


   MockConf conf;
   conf.mUnknownCaptureEnabled = true;
   MockDiskPacketCapture capture(conf);

   conf.mPCapCaptureLocation = "testLocation";
   conf.mPCapCaptureFileLimit = 10000;
   conf.mPCapCaptureSizeLimit = 10000;
   conf.mPCapCaptureMemoryLimit = 5;
   conf.mMaxIndividualPCap = 10;

   {
      tempFileCreate tempFile(conf);
      ASSERT_TRUE(tempFile.Init());
      ASSERT_TRUE(capture.Initialize());
      EXPECT_EQ(0, capture.NewTotalMemory(0));
      EXPECT_EQ(0, capture.CurrentMemoryForFlow("notThere"));
      struct upacket packet;
      ctb_pkt p;
      packet.p = &p;

      unsigned char* data = new unsigned char[(1024 * 1024 * 10) - sizeof (struct pcap_pkthdr)];
      p.len = 2 * (1024 * 1024) - sizeof (struct pcap_pkthdr);
      p.data = data;
      networkMonitor::DpiMsgLR dpiMsg;
      dpiMsg.set_sessionid("FlowOne");
      dpiMsg.set_written(false);
      capture.SavePacket(&dpiMsg, &packet);

      EXPECT_EQ(0, capture.CurrentDiskForFlow("FlowOne"));
      EXPECT_FALSE(dpiMsg.written());
      EXPECT_EQ(2, capture.CurrentMemoryForFlow("FlowOne"));
      p.len = 4 * (1024 * 1024) - sizeof (struct pcap_pkthdr);
      capture.SavePacket(&dpiMsg, &packet);
      EXPECT_EQ(2, capture.CurrentDiskForFlow("FlowOne"));
      EXPECT_TRUE(dpiMsg.written());
      dpiMsg.set_written(false);
      EXPECT_EQ(4, capture.CurrentMemoryForFlow("FlowOne"));
      p.len = 5 * (1024 * 1024) - sizeof (struct pcap_pkthdr);
      capture.SavePacket(&dpiMsg, &packet);

      EXPECT_EQ(6, capture.CurrentDiskForFlow("FlowOne"));
      EXPECT_TRUE(dpiMsg.written());
      dpiMsg.set_written(false);
      EXPECT_EQ(0, capture.CurrentMemoryForFlow("FlowOne"));

      conf.mMaxIndividualPCap = 11;
      p.len = 4 * (1024 * 1024) - sizeof (struct pcap_pkthdr);
      capture.SavePacket(&dpiMsg, &packet);
      EXPECT_EQ(6, capture.CurrentDiskForFlow("FlowOne"));
      EXPECT_FALSE(dpiMsg.written());
      dpiMsg.set_written(false);
      EXPECT_EQ(4, capture.CurrentMemoryForFlow("FlowOne"));
      delete []data;
   }
   ASSERT_FALSE(capture.Initialize());

}
#endif
#ifdef LR_DEBUG

TEST_F(DiskPacketCaptureTest, PacketCaptureDisabled) {


   MockConf conf;
   conf.mUnknownCaptureEnabled = false;
   MockDiskPacketCapture capture(conf);

   conf.mPCapCaptureLocation = "testLocation";
   conf.mPCapCaptureFileLimit = 10000;
   conf.mPCapCaptureSizeLimit = 10000;
   conf.mPCapCaptureMemoryLimit = 5;
   conf.mMaxIndividualPCap = 10;

   {
      tempFileCreate tempFile(conf);
      ASSERT_TRUE(tempFile.Init());
      ASSERT_TRUE(capture.Initialize());
      EXPECT_EQ(0, capture.NewTotalMemory(0));
      EXPECT_EQ(0, capture.CurrentMemoryForFlow("notThere"));
      struct upacket packet;
      ctb_pkt p;
      packet.p = &p;

      unsigned char* data = new unsigned char[(1024 * 1024 * 10) - sizeof (struct pcap_pkthdr)];
      p.len = 2 * (1024 * 1024) - sizeof (struct pcap_pkthdr);
      p.data = data;
      networkMonitor::DpiMsgLR dpiMsg;
      dpiMsg.set_sessionid("FlowOne");
      dpiMsg.set_written(false);
      capture.SavePacket(&dpiMsg, &packet);
      EXPECT_EQ(0, capture.CurrentDiskForFlow("FlowOne"));
      EXPECT_FALSE(dpiMsg.written());
      EXPECT_EQ(0, capture.CurrentMemoryForFlow("FlowOne"));

      p.len = 4 * (1024 * 1024) - sizeof (struct pcap_pkthdr);
      capture.SavePacket(&dpiMsg, &packet);
      EXPECT_EQ(0, capture.CurrentDiskForFlow("FlowOne"));
      EXPECT_FALSE(dpiMsg.written());
      EXPECT_EQ(0, capture.CurrentMemoryForFlow("FlowOne"));

      delete []data;
   }
   ASSERT_FALSE(capture.Initialize());

}
#endif
