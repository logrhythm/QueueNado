
#include "DiskPacketCaptureTest.h"

#include "dpi/modules/upacket.h"
#include "pcap/pcap.h"
#include <thread>
#include <future>
#include <map>

#ifdef LR_DEBUG
#include "MockConf.h"
#include "MockDiskPacketCapture.h"
#else
#include "Conf.h"
#include "DiskPacketCapture.h"
#endif

#ifdef LR_DEBUG

#include "tempFileCreate.h"
#include "MsgUuid.h"

// System test -- for now not disabled
TEST_F(DiskPacketCaptureTest, SystemTest_VerifyGetCaptureFirstLocation) {
   Conf conf;
   const auto& first = conf.GetFirstPcapCaptureLocation();
   ASSERT_TRUE((first == "/pcap0/" || first == "/usr/local/probe/pcap/")) 
           << "PCAP first storage location: " << first;
   LOG(INFO) << "pcap location is: " <<  first;
}


TEST_F(DiskPacketCaptureTest, SystemTest_GetCaptureFirstLocationExpectNoThrow) {
   MockConf conf;
   std::string unlikelyDir = {"/yalla/abc/abc123/"};
   conf.mPCapCaptureLocations.push_back(unlikelyDir);
   auto first = conf.GetFirstPcapCaptureLocation();
   ASSERT_EQ(first, unlikelyDir);
   conf.mOverrideGetPcapCaptureLocations = false; // use real calculations
    
   std::vector<std::string> locations;
   EXPECT_NO_THROW(locations = conf.GetPcapCaptureLocations());
   ASSERT_EQ(locations.size(), 1);
   ASSERT_EQ(locations[0], unlikelyDir);
}


TEST_F(DiskPacketCaptureTest, TestVerifyGetCaptureOneLocation) {
   MockConf conf;
   // Verify test setup
   EXPECT_EQ("", conf.GetFirstPcapCaptureLocation());
   EXPECT_EQ(0, conf.GetPcapCaptureLocations().size());
   
   
   conf.mOverrideGetPcapCaptureLocations = false; // use real calculations
   ASSERT_EQ(conf.mPCapCaptureLocations.size(), 0);
   conf.mPCapCaptureLocations.push_back("/tmp/pcap0/");
   EXPECT_EQ("/tmp/pcap0/", conf.GetFirstPcapCaptureLocation());
   auto locations = conf.GetPcapCaptureLocations();
   ASSERT_EQ(locations.size(), 1);
   EXPECT_EQ("/tmp/pcap0/", locations[0]);
}

TEST_F(DiskPacketCaptureTest,  ConfCreatesCorrectCaptureLocations) {
   MockConf conf;
      
   // Create FF locations 
   const std::string baseDir = testDir.str();
   std::vector<std::string> putLocations;
   for(int idx = 0; idx <= 0xFF; ++idx) {
      std::string dir = baseDir;
      dir.append("/pcap").append(std::to_string(idx)).append({"/"});
      putLocations.push_back(dir);
   }
   // Creates capture directories that should be seen by the Conf::GetPcapCaptureLocations
   ASSERT_EQ(putLocations.size(), 256); // i.e. 256 = 0xFF +1 locations
   for(const auto& dir : putLocations) {
      std::string mkdir = {"mkdir -p "};
      mkdir.append(dir); // auto cleanup at test exit: Ref DiskPacketCaptureTest::TearDown
      system(mkdir.c_str());
   }
   
   // Verify that we see all the created pcap storage directories
   conf.mOverrideGetPcapCaptureLocations = false; // use real calculations
   conf.mPCapCaptureLocations.clear();
   conf.mPCapCaptureLocations.push_back(putLocations[0]); // first location
   auto locations = conf.GetPcapCaptureLocations();
   ASSERT_EQ(locations.size(), putLocations.size());
   size_t count = 0;
   for(auto& put: putLocations)
   {
      ASSERT_EQ(put, locations[count++]);
   }
   EXPECT_TRUE(std::equal(putLocations.begin(), putLocations.end(), locations.begin()));  
}

TEST_F(DiskPacketCaptureTest, IntegrationTestWithSizeLimitNothingPrior) {
   MockConf conf;
   conf.mUnknownCaptureEnabled = true;
   conf.mPCapCaptureLocations.push_back(testDir.str());
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
   conf.mPCapCaptureLocations.push_back(testDir.str());
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

   conf.mPCapCaptureLocations.push_back("testLocation");
   std::string fileName = capture.BuildFilenameWithPath("TestUUID");
   ASSERT_EQ("testLocation/TestUUID", fileName);
   
   fileName = capture.BuildFilenameWithPath("");
   ASSERT_EQ("", fileName);
#endif
}



TEST_F(DiskPacketCaptureTest, GetFilenamesOneRoundRobin) {
#ifdef LR_DEBUG
   MockConf conf;
   conf.mUnknownCaptureEnabled = true;
   MockDiskPacketCapture capture(conf);

   conf.mPCapCaptureLocations.push_back("testLocation0");
   size_t count0 =0;
   for(size_t idx = 0; idx <= 0xF; ++idx) {
      size_t index = idx % 1; // size of one
      std::string fileName = capture.BuildFilenameWithPath("TestUUID");
      std::string expected = "testLocation";
      expected.append(std::to_string(index)).append("/TestUUID");
      ASSERT_EQ(expected, fileName);
      if(0 == index) {++count0;}   
   }
   ASSERT_EQ(count0, 16);      
#endif
}

TEST_F(DiskPacketCaptureTest, GetFilenamesOneRoundRobin2) {
#ifdef LR_DEBUG
   MockConf conf;
   conf.mUnknownCaptureEnabled = true;
   MockDiskPacketCapture capture(conf);

   conf.mPCapCaptureLocations.push_back("testLocation");
   for(size_t digit = 0; digit <= 0xFF; ++digit) {
      std::string uuid = MockUuidGenerator::GetMsgUuid(digit); 
      std::string fileName = {};
      EXPECT_NO_THROW(fileName = capture.BuildFilenameWithPath(uuid));
      ASSERT_EQ("testLocation/" + uuid, fileName);
   }    
#endif
}



TEST_F(DiskPacketCaptureTest, GetFilenamesEvenRoundRobinManyBuckets) {
#ifdef LR_DEBUG
   MockConf conf;
   conf.mUnknownCaptureEnabled = true;
   MockDiskPacketCapture capture(conf);
   auto& generator = networkMonitor::MsgUuid::Instance();
   
   std::string base = {"testLocation"};
   const size_t buckets = 256;
   for(int i = 0; i <= 0xFF; ++i) {
      std::string location = base;
      location.append(std::to_string(i));
      conf.mPCapCaptureLocations.push_back(location);
   }
   
   const auto max = 2040;
   std::map<int, int> counters;
   for(auto loop = 0; loop < max; ++loop) {
      
      
      std::string uuid = generator.GetMsgUuid();
      std::size_t digitFromHex = 0;
      EXPECT_NO_THROW(digitFromHex = std::stoul(&uuid[uuid.size()-2], 0, 16));              
      std::string fileName = {};
      EXPECT_NO_THROW(fileName = capture.BuildFilenameWithPath(uuid));
      size_t whichBucket = digitFromHex % buckets;
      
      counters[whichBucket]++;
      std::string checkName = "testLocation";
      checkName.append(std::to_string(whichBucket)).append("/").append(uuid);
      ASSERT_EQ(checkName, fileName);
   }
   for( auto& count: counters) {
      LOG(INFO) << "bucket: " << count.first << " : " << count.second;
      ASSERT_NE(0, count.second);
   }
   auto locations = conf.GetPcapCaptureLocations();
   ASSERT_EQ(locations.size(), 256);
   ASSERT_EQ(counters.size(), buckets);

#endif
}

TEST_F(DiskPacketCaptureTest, GetFilenamesMaxRoundRobin) {
#ifdef LR_DEBUG
   MockConf conf;
   conf.mUnknownCaptureEnabled = true;  
   MockDiskPacketCapture capture(conf);

   
   for(size_t digit = 0; digit <= 0xFF; ++digit) {
      std::string dir = "testLocation";
      dir.append(std::to_string(digit));
      conf.mPCapCaptureLocations.push_back(dir);        
   }
   std::vector<std::string> locations = conf.GetPcapCaptureLocations();
   ASSERT_EQ(locations.size(), 256);
   std::map<size_t, size_t> counters;
      
   for(size_t digit = 0; digit <= 0xFF; ++digit) {
      std::string uuid = MockUuidGenerator::GetMsgUuid(digit); 
      
      // Mimic the convertion in DiskPacketCapture to verify it works as intended
      std::size_t digitFromHex = 0;
      EXPECT_NO_THROW(digitFromHex = std::stoul(&uuid[uuid.size()-2], 0, 16));  
      std::string fileName = {};
      EXPECT_NO_THROW(fileName = capture.BuildFilenameWithPath(uuid));
      size_t bucket = digitFromHex % 256;      
      ++counters[bucket];
      std::string checkName = "testLocation";
      checkName.append(std::to_string(bucket)).append("/").append(uuid);
      ASSERT_EQ(checkName, fileName);
   }
   ASSERT_EQ(counters.size(), 256);
   size_t total = 0;
   size_t idx = 0;
   for(const auto& count: counters) {
      ASSERT_EQ(count.first, idx++); // make sure that all the buckets exist 0-255
      total += count.second; // add total saved
      ASSERT_EQ(count.second, 1); // only one saved per bucket
   }
   EXPECT_EQ(256, total);
#endif
}


// Identical UUID must be created in the same bucket
TEST_F(DiskPacketCaptureTest, GetFilenamesAvoidDuplicates) {
#ifdef LR_DEBUG
   MockConf conf;
   tempFileCreate tempFile(conf);
   tempFile.Init();
   conf.mUnknownCaptureEnabled = true;
   MockDiskPacketCapture capture(conf);
   
   std::string base = tempFile.mTestDir.str();
   std::string mkdir = {"mkdir -p "};
   auto dir1 = base, dir2 = base, dir3=base;
   auto mkdir1 = mkdir, mkdir2 = mkdir, mkdir3=mkdir;
   dir1.append("/testLocation0/");
   dir2.append("/testLocation1/");
   dir3.append("/testLocation2/");
   mkdir1.append(dir1);
   mkdir2.append(dir2);
   mkdir3.append(dir3);
   EXPECT_EQ(0, system(mkdir1.c_str()));
   EXPECT_EQ(0, system(mkdir2.c_str()));
   EXPECT_EQ(0, system(mkdir3.c_str()));
   
   conf.mPCapCaptureLocations.clear();
   conf.mOverrideGetPcapCaptureLocations = false; // use real logic
   conf.mPCapCaptureLocations.push_back(dir1);
   EXPECT_EQ(conf.GetPcapCaptureLocations().size(), 3); 
   std::string fileName1 = capture.BuildFilenameWithPath("TestUUID");   
   std::string fileName2 = capture.BuildFilenameWithPath("TestUUID");
   std::string fileName3 = capture.BuildFilenameWithPath("TestUUID");
   
   // Make sure the filenames goes in the same hashed buckets since no file exists in 
   // buckets
   EXPECT_EQ(fileName1, fileName2);
   EXPECT_EQ(fileName1, fileName3);
   EXPECT_EQ(fileName2, fileName3);
   
   // Make sure that the filenames go in the same bucket no matter if it existed or not
   std::string touch = {"touch "};
   touch.append(fileName1);
   system(touch.c_str());
   fileName1 = capture.BuildFilenameWithPath("TestUUID");   
   fileName2 = capture.BuildFilenameWithPath("TestUUID");
   fileName3 = capture.BuildFilenameWithPath("TestUUID");
   EXPECT_EQ(fileName1, fileName2);
   EXPECT_EQ(fileName1, fileName3);
   EXPECT_EQ(fileName2, fileName3);
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
      conf.mPCapCaptureLocations.clear();
      conf.mPCapCaptureLocations.push_back("testLocation");
      ASSERT_FALSE(capture.Initialize());
      conf.mPCapCaptureLocations.clear();
      conf.mPCapCaptureLocations.push_back(tempFile.mTestDir.str());
      ASSERT_TRUE(capture.Initialize());
      conf.mPCapCaptureLocations.clear();
      conf.mPCapCaptureLocations.push_back("/etc/passwd");
      
      ASSERT_FALSE(capture.Initialize());
   }
#endif 
}

TEST_F(DiskPacketCaptureTest, MemoryLimits) {
#ifdef LR_DEBUG
   MockConf conf;
   conf.mUnknownCaptureEnabled = true;
   MockDiskPacketCapture capture(conf);
   conf.mPCapCaptureLocations.push_back("testLocation");
   ASSERT_FALSE(capture.Initialize());
   
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


TEST_F(DiskPacketCaptureTest, AutoFlushOnMemoryLimit_FlushingToManyLocations) {
#ifdef LR_DEBUG
   MockConf conf;
   conf.mUnknownCaptureEnabled = true;
   MockDiskPacketCapture capture(conf);
   conf.mPCapCaptureLocations.push_back("testLocation"); 
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
      EXPECT_EQ(0, capture.NewTotalMemory(0));
      capture.SavePacket(&dpiMsg, &packet);
      EXPECT_EQ(1, capture.NewTotalMemory(0));
      ASSERT_EQ(0, capture.mFilesWritten.size()); // Not yet flushed

      
      dpiMsg.set_sessionid("FlowTwo");
      capture.SavePacket(&dpiMsg, &packet);
      
      // Verify that all is still kept in memory and not flushed
      ASSERT_EQ(0, capture.mFilesWritten.size()); 
      EXPECT_EQ(2, capture.NewTotalMemory(0));
      EXPECT_EQ(1, capture.CurrentMemoryForFlow("FlowOne"));
      EXPECT_EQ(1, capture.CurrentMemoryForFlow("FlowTwo")); 

      
      capture.mFailFlush = false;
      // The next one will force some flow to be flushed to disk
      conf.mPCapCaptureMemoryLimit = 0; 
      EXPECT_EQ(0, conf.GetPcapCaptureMemoryLimit());


      // 
      dpiMsg.set_sessionid("FlowTwo");
      capture.SavePacket(&dpiMsg, &packet);
      EXPECT_EQ(1, capture.NewTotalMemory(0)); // FlowOne still left
      ASSERT_EQ(1, capture.mFilesWritten.size()); // FlowTwo flushed
      EXPECT_EQ(1, capture.CurrentMemoryForFlow("FlowOne")); // not flushed
      EXPECT_EQ(0, capture.CurrentMemoryForFlow("FlowTwo")); // flushed

      // Post: thread-id used in SavePacket
      // Any attempt to save a packet when we are over the limit will result in a discarded package
      // This is because the previous save of FlowTwo erased the pointed to flow for this thread
      // 
      // Since we are doing it in the same thread we must ensure that new packets end up in the 
      //'bigFlows' (to be flushed)
      // I.e. 1) Raise the bar so that it is not discareded but added to the bigflows
      //      2) Save packets 
      //      3) Lower the bar and save another packet --> which triggers the flush of the file (and the erase mentioned above)
      conf.mPCapCaptureMemoryLimit = 10; 
      dpiMsg.set_sessionid("FlowOne");
      capture.SavePacket(&dpiMsg, &packet); // 1 -> 2
      capture.SavePacket(&dpiMsg, &packet); // 2 -> 3
      EXPECT_EQ(3, capture.CurrentMemoryForFlow("FlowOne")); // not flushed
      conf.mPCapCaptureMemoryLimit = 1; 
      capture.SavePacket(&dpiMsg, &packet); // FlowOne is now also flushed

      EXPECT_EQ(0, capture.CurrentMemoryForFlow("FlowOne")); // not flushed
      EXPECT_EQ(0, capture.CurrentMemoryForFlow("FlowTwo")); // flushed
      ASSERT_EQ(2, capture.mFilesWritten.size()); // FlowTwo flushed

   }
   ASSERT_FALSE(capture.Initialize());
#endif
}

TEST_F(DiskPacketCaptureTest, AutoFlushOnMemoryLimit) {
#ifdef LR_DEBUG
   MockConf conf;
   conf.mUnknownCaptureEnabled = true;
   MockDiskPacketCapture capture(conf);
   conf.mPCapCaptureLocations.push_back("testLocation"); 
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

   conf.mPCapCaptureLocations.push_back("testLocation");
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

   conf.mPCapCaptureLocations.push_back("testLocation");
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
