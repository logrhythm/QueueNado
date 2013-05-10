
#include "DiskPacketCaptureTest.h"
#include "MockConf.h"
#include "dpi/modules/upacket.h"
#include "pcap/pcap.h"
#include <thread>
#include <future>

TEST_F(DiskPacketCaptureTest, Construct) {
   DiskPacketCapture capture;
   DiskPacketCapture* pCapture = new DiskPacketCapture;
   delete pCapture;
}

TEST_F(DiskPacketCaptureTest, GetRunningPackets) {
   MockDiskPacketCapture capture;
   std::pair<InMemoryPacketBuffer*, size_t>* sessionInfo;
   std::pair<InMemoryPacketBuffer*, size_t>* sessionInfo2;
   capture.GetRunningPackets("abc123", sessionInfo);
   capture.GetRunningPackets("abc123", sessionInfo2);
   ASSERT_EQ(sessionInfo, sessionInfo2);
   capture.GetRunningPackets("def456", sessionInfo2);
   ASSERT_NE(sessionInfo, sessionInfo2);

   //A promise object that can be used for sharing a variable between threads
   std::promise<InMemoryPacketBuffer* > promisedPacket;

   // A future variable that reads from the promise, allows you to wait on the other thread
   std::future<InMemoryPacketBuffer*> futurePacket = promisedPacket.get_future();

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
   std::thread([](std::promise<InMemoryPacketBuffer* >& pointer,
           MockDiskPacketCapture & capture) {
      std::pair<InMemoryPacketBuffer*, size_t>* sessionInfo3;
      capture.GetRunningPackets("abc123", sessionInfo3);
              pointer.set_value(sessionInfo3->first); }, std::ref(promisedPacket), std::ref(capture)).detach();

   // Wait for the thread to set the promise object to a new value
   futurePacket.wait();

   // Verify that the code run in the thread gives you a different pointer 
   capture.GetRunningPackets("abc123", sessionInfo);
   ASSERT_NE(sessionInfo->first, futurePacket.get());
}

TEST_F(DiskPacketCaptureTest, GetFilenamesTest) {
   MockDiskPacketCapture capture;
   MockConf conf;
   conf.mPCapCaptureLocation = "testLocation";
   std::time_t time = 123456789;
   std::string fileName = capture.BuildFilename(conf, "TestUUID", "TestAppName", time);
   ASSERT_EQ("testLocation/TestUUID_TestAppName_1973-11-29-14:33:09", fileName);
}

TEST_F(DiskPacketCaptureTest, Initialize) {
   MockDiskPacketCapture capture;
   MockConf conf;
   conf.mPCapCaptureLocation = "testLocation";
   ASSERT_FALSE(capture.Initialize(conf));
   conf.mPCapCaptureLocation = "/tmp";
   ASSERT_TRUE(capture.Initialize(conf));
   conf.mPCapCaptureLocation = "/etc/passwd";
   ASSERT_FALSE(capture.Initialize(conf));
}

TEST_F(DiskPacketCaptureTest, TooMuchPCap) {
   if (geteuid() == 0) {
      MockDiskPacketCapture capture;
      MockConf conf;
      conf.mPCapCaptureLocation = "testLocation";
      conf.mPCapCaptureFileLimit = 10000;
      conf.mPCapCaptureSizeLimit = 10000;
      ASSERT_FALSE(capture.Initialize(conf));
      ASSERT_FALSE(capture.TooMuchPCap(conf));

      std::stringstream testDir;
      testDir << "/tmp/TooMuchPcap." << pthread_self();

      conf.mPCapCaptureLocation = testDir.str();

      std::string makeADir = "mkdir -p ";
      makeADir += testDir.str();

      ASSERT_EQ(0, system(makeADir.c_str()));

      EXPECT_TRUE(capture.Initialize(conf));
      EXPECT_FALSE(capture.TooMuchPCap(conf));
      conf.mPCapCaptureFileLimit = 0;
      EXPECT_TRUE(capture.TooMuchPCap(conf));
      conf.mPCapCaptureFileLimit = 10000;
      conf.mPCapCaptureSizeLimit = 0;
      EXPECT_TRUE(capture.TooMuchPCap(conf));
      conf.mPCapCaptureFileLimit = 1;
      conf.mPCapCaptureSizeLimit = 1;
      EXPECT_FALSE(capture.TooMuchPCap(conf));
      std::string makeSmallFile = "touch ";
      makeSmallFile += testDir.str();
      makeSmallFile += "/smallFile";

      EXPECT_EQ(0, system(makeSmallFile.c_str()));
      EXPECT_TRUE(capture.TooMuchPCap(conf));
      conf.mPCapCaptureFileLimit = 10;
      EXPECT_FALSE(capture.TooMuchPCap(conf));
      std::string make1MFileFile = "dd bs=1024 count=1024 if=/dev/zero of=";
      make1MFileFile += testDir.str();
      make1MFileFile += "/1MFile";


      EXPECT_EQ(0, system(make1MFileFile.c_str()));
      EXPECT_TRUE(capture.TooMuchPCap(conf));
      conf.mPCapCaptureSizeLimit = 2;
      EXPECT_FALSE(capture.TooMuchPCap(conf));
      make1MFileFile = "dd bs=1048575 count=1 if=/dev/zero of=";
      make1MFileFile += testDir.str();
      make1MFileFile += "/1MFilelessone";
      EXPECT_EQ(0, system(make1MFileFile.c_str()));
      EXPECT_FALSE(capture.TooMuchPCap(conf));

      conf.mPCapCaptureFileLimit = 3;
      EXPECT_TRUE(capture.TooMuchPCap(conf));
      capture.RemoveOldestPCapFile(conf);
      EXPECT_FALSE(capture.TooMuchPCap(conf));
      conf.mPCapCaptureSizeLimit = 1;
      EXPECT_TRUE(capture.TooMuchPCap(conf));
      capture.RemoveOldestPCapFile(conf);
      EXPECT_FALSE(capture.TooMuchPCap(conf));
      conf.mPCapCaptureFileLimit = 1;
      EXPECT_TRUE(capture.TooMuchPCap(conf));
      capture.RemoveOldestPCapFile(conf);
      EXPECT_FALSE(capture.TooMuchPCap(conf));

      makeADir = "";
      makeADir = "rm -rf ";
      makeADir += testDir.str();
      ASSERT_EQ(0, system(makeADir.c_str()));
      ASSERT_FALSE(capture.Initialize(conf));
   }
}

TEST_F(DiskPacketCaptureTest, CleanupOldPcapFiles) {
   if (geteuid() == 0) {
      MockDiskPacketCapture capture;
      MockConf conf;
      conf.mPCapCaptureLocation = "testLocation";
      conf.mPCapCaptureFileLimit = 10000;
      conf.mPCapCaptureSizeLimit = 10000;
      ASSERT_FALSE(capture.Initialize(conf));
      ASSERT_FALSE(capture.TooMuchPCap(conf));

      std::stringstream testDir;
      testDir << "/tmp/TooMuchPcap." << pthread_self();

      conf.mPCapCaptureLocation = testDir.str();

      std::string makeADir = "mkdir -p ";
      makeADir += testDir.str();

      ASSERT_EQ(0, system(makeADir.c_str()));

      EXPECT_TRUE(capture.Initialize(conf));
      std::string makeSmallFile = "touch ";
      makeSmallFile += testDir.str();
      makeSmallFile += "/smallFile";
      EXPECT_EQ(0, system(makeSmallFile.c_str()));

      std::string make1MFileFile = "dd bs=1024 count=1024 if=/dev/zero of=";
      make1MFileFile += testDir.str();
      make1MFileFile += "/1MFile";
      EXPECT_EQ(0, system(make1MFileFile.c_str()));

      make1MFileFile = "dd bs=1048575 count=1 if=/dev/zero of=";
      make1MFileFile += testDir.str();
      make1MFileFile += "/1MFilelessone";
      EXPECT_EQ(0, system(make1MFileFile.c_str()));

      conf.mPCapCaptureFileLimit = 3;
      EXPECT_TRUE(capture.TooMuchPCap(conf));
      capture.CleanupOldPcapFiles(conf);
      EXPECT_FALSE(capture.TooMuchPCap(conf));
      conf.mPCapCaptureSizeLimit = 1;
      EXPECT_TRUE(capture.TooMuchPCap(conf));
      capture.CleanupOldPcapFiles(conf);
      EXPECT_FALSE(capture.TooMuchPCap(conf));
      conf.mPCapCaptureFileLimit = 1;
      EXPECT_TRUE(capture.TooMuchPCap(conf));
      capture.CleanupOldPcapFiles(conf);
      EXPECT_FALSE(capture.TooMuchPCap(conf));

      makeADir = "";
      makeADir = "rm -rf ";
      makeADir += testDir.str();
      ASSERT_EQ(0, system(makeADir.c_str()));
      ASSERT_FALSE(capture.Initialize(conf));
   }
}

TEST_F(DiskPacketCaptureTest, MemoryLimits) {
   MockConf conf;
   MockDiskPacketCapture capture;
   conf.mPCapCaptureLocation = "testLocation";
   conf.mPCapCaptureFileLimit = 10000;
   conf.mPCapCaptureSizeLimit = 10000;
   conf.mPCapCaptureMemoryLimit = 10;
   std::stringstream testDir;
   testDir << "/tmp/TooMuchPcap." << pthread_self();

   conf.mPCapCaptureLocation = testDir.str();

   std::string makeADir = "mkdir -p ";
   makeADir += testDir.str();

   ASSERT_EQ(0, system(makeADir.c_str()));
   ASSERT_TRUE(capture.Initialize(conf));

   EXPECT_EQ(0, capture.NewTotalMemory(0));
   EXPECT_EQ(0, capture.CurrentMemoryForFlow("notThere"));
   struct upacket packet;
   ctb_pkt p;
   packet.p = &p;
   unsigned char data[(1024 * 1024) - sizeof(struct pcap_pkthdr)];
   p.len = (1024 * 1024) - sizeof(struct pcap_pkthdr);
   p.data = data;
   capture.SavePacket("FlowOne", &packet,conf);
   EXPECT_EQ(1, capture.NewTotalMemory(0));
   EXPECT_EQ(1, capture.CurrentMemoryForFlow("FlowOne"));
   p.len = (1024 * 1024) - sizeof(struct pcap_pkthdr) - sizeof(struct pcap_pkthdr) - 1;
   capture.SavePacket("FlowOne", &packet,conf);
   EXPECT_EQ(1, capture.NewTotalMemory(0));
   EXPECT_EQ(1, capture.CurrentMemoryForFlow("FlowOne"));
   p.len = 1;
   capture.SavePacket("FlowTwo", &packet,conf);
   EXPECT_EQ(2, capture.NewTotalMemory(0));
   EXPECT_EQ(1, capture.CurrentMemoryForFlow("FlowOne"));
   EXPECT_EQ(0, capture.CurrentMemoryForFlow("FlowTwo"));
   
   conf.mPCapCaptureMemoryLimit = 2;
   p.len = (1024 * 1024) - sizeof(struct pcap_pkthdr);
   capture.SavePacket("FlowTwo", &packet,conf);
   EXPECT_EQ(2, capture.NewTotalMemory(0));
   EXPECT_EQ(1, capture.CurrentMemoryForFlow("FlowOne"));
   EXPECT_EQ(0, capture.CurrentMemoryForFlow("FlowTwo"));
   
   conf.mPCapCaptureMemoryLimit = 3;
   EXPECT_EQ(3, capture.NewTotalMemory((1024 * 1024)));
   
   capture.RemoveDataFromRunningPackets("FlowOne");
   EXPECT_EQ(0, capture.NewTotalMemory(0));
   EXPECT_EQ(0, capture.CurrentMemoryForFlow("FlowOne"));  
   
   makeADir = "rm -rf ";
   makeADir += testDir.str();
   ASSERT_EQ(0, system(makeADir.c_str()));
   ASSERT_FALSE(capture.Initialize(conf));
}