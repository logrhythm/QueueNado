
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



TEST_F(DiskPacketCaptureTest, ConstructAndDeconstructFilename) {
   MockConf conf;
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
   DiskPacketCapture capture(conf);
   DiskPacketCapture* pCapture = new DiskPacketCapture(conf);
   delete pCapture;
#endif
}

TEST_F(DiskPacketCaptureTest, GetRunningPackets) {
#ifdef LR_DEBUG
   MockConf conf;
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
   MockDiskPacketCapture capture(conf);

   conf.mPCapCaptureLocation = "testLocation";
   std::string fileName = capture.BuildFilenameWithPath("TestUUID");
   ASSERT_EQ("testLocation/TestUUID", fileName);
#endif
}

TEST_F(DiskPacketCaptureTest, Initialize) {
#ifdef LR_DEBUG
   MockConf conf;
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
      packet.p = & p;
      unsigned char data[(1024 * 1024) - sizeof (struct pcap_pkthdr)];
      p.len = (1024 * 1024) - sizeof (struct pcap_pkthdr);
      p.data = data;
      networkMonitor::DpiMsgLR dpiMsg;
      dpiMsg.set_sessionid("FlowOne");
      capture.SavePacket(&dpiMsg, &packet);
      EXPECT_EQ(1, capture.NewTotalMemory(0));
      EXPECT_EQ(1, capture.CurrentMemoryForFlow("FlowOne"));
      p.len = (1024 * 1024) - sizeof (struct pcap_pkthdr) - sizeof (struct pcap_pkthdr) - 1;
      capture.SavePacket(&dpiMsg, &packet);
      EXPECT_EQ(1, capture.NewTotalMemory(0));
      EXPECT_EQ(1, capture.CurrentMemoryForFlow("FlowOne"));
      p.len = 1;
      dpiMsg.set_sessionid("FlowTwo");
      capture.SavePacket(&dpiMsg, &packet);
      EXPECT_EQ(2, capture.NewTotalMemory(0));
      EXPECT_EQ(1, capture.CurrentMemoryForFlow("FlowOne"));
      EXPECT_EQ(0, capture.CurrentMemoryForFlow("FlowTwo"));

      conf.mPCapCaptureMemoryLimit = 2;
      p.len = (1024 * 1024) - sizeof (struct pcap_pkthdr);
      capture.mFailFlush = true;
      capture.SavePacket(&dpiMsg, &packet);
      EXPECT_EQ(2, capture.NewTotalMemory(0));
      EXPECT_EQ(1, capture.CurrentMemoryForFlow("FlowOne"));
      EXPECT_EQ(0, capture.CurrentMemoryForFlow("FlowTwo"));

      conf.mPCapCaptureMemoryLimit = 3;
      EXPECT_EQ(3, capture.NewTotalMemory((1024 * 1024)));

      capture.RemoveDataFromRunningPackets("FlowOne");
      EXPECT_EQ(0, capture.NewTotalMemory(0));
      EXPECT_EQ(0, capture.CurrentMemoryForFlow("FlowOne"));

   }
   ASSERT_FALSE(capture.Initialize());
#endif
}

TEST_F(DiskPacketCaptureTest, AutoFlushOnMemoryLimit) {
#ifdef LR_DEBUG
   MockConf conf;
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
      packet.p = & p;
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
      if ("FlowOne" == * capture.mFilesWritten.begin()) {
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
      packet.p = & p;

      unsigned char* data = new unsigned char[(1024*1024*10) - sizeof (struct pcap_pkthdr)];
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