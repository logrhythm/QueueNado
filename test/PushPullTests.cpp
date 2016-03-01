#include <czmq.h>
#include <boost/thread.hpp>
#include "PushPullTests.h"
#include "Death.h"
#include "FileIO.h"
#include <g3log/g3log.hpp>
#include <functional>
namespace {
   std::string GetTcpLocation() {
      int max = 9000;
      int min = 7000;
      int port = (rand() % (max - min)) + min;
      std::string tcpLocation("tcp://127.0.0.1:");
      tcpLocation.append(std::to_string(port));
      return tcpLocation;
   }

   std::string GetIpcLocation() {
      int pid = getpid();
      std::string ipcLocation("ipc:///tmp/");
      ipcLocation.append("PushPullTests");
      ipcLocation.append(std::to_string(pid));
      ipcLocation.append(".ipc");
      return ipcLocation;
   }

   std::vector<std::shared_ptr<Push>> CreatePushs(int howManyPushs,
                                                  int howManyPulls,
                                                  const std::string& location) {
      std::vector<std::shared_ptr<Push>> toReturn;
      for (int i = 0; i < howManyPushs ; i ++) {
         if (howManyPushs == 1 && howManyPulls == 1) {
            // one to one, use default behavior (connect)
            LOG(DEBUG) << "one to one push setup";
            toReturn.push_back(std::make_shared<Push>(location));
         } else if (howManyPushs > 1) {
            // many pushes to one pull, use default behavior (connect)
            LOG(DEBUG) << "many to one push setup";
            toReturn.push_back(std::make_shared<Push>(location));
         } else if (howManyPulls > 1) {
            // many pulls to one push, bind instead of connect
            LOG(DEBUG) << "one to many push setup";
            toReturn.push_back(std::make_shared<Push>(location, false));
         }
      }
      return toReturn;
   }
   std::vector<std::shared_ptr<Pull>> CreatePulls(int howManyPushs,
                                                  int howManyPulls,
                                                  const std::string& location) {
      std::vector<std::shared_ptr<Pull>> toReturn;
      for (int i = 0; i < howManyPulls ; i ++) {
         if (howManyPushs == 1 && howManyPulls == 1) {
            // one to one, use default behavior (bind)
            LOG(DEBUG) << "one to one pull setup";
            toReturn.push_back(std::make_shared<Pull>(location));
         } else if (howManyPushs > 1) {
            LOG(DEBUG) << "many to one pull setup";
            // many pushes to one pull, use default behavior (bind)
            toReturn.push_back(std::make_shared<Pull>(location));
         } else if (howManyPulls > 1) {
            LOG(DEBUG) << "one to many pull setup";
            // many pulls to one push, connect instead of bind
            toReturn.push_back(std::make_shared<Pull>(location, true));
         }
      }
      return toReturn;
   }
   std::vector<std::thread> ListenWithPulls(std::vector<std::shared_ptr<Pull>>& pulls,
                                            std::function<void(std::shared_ptr<Pull>)> pullLambda) {
      std::vector<std::thread> threads;
      for (auto& pull:pulls) {
         LOG(DEBUG) << "Creating Pull thread";
         threads.push_back(std::thread(pullLambda, pull));
      }
      return threads;
   }
   std::vector<std::thread> ShootWithPushs(std::vector<std::shared_ptr<Push>>& pushs,
                        std::function<void(std::shared_ptr<Push>)> pushLambda) {
      std::vector<std::thread> threads;
      for (auto& push:pushs) {
         LOG(DEBUG) << "Creating push thread";
         threads.push_back(std::thread(pushLambda, push));
      }
      return threads;
   }
   void FireStringsAtPulls(int howManyPushs,
                           int howManyShots,
                           int howManyPulls,
                           const std::string& location,
                           const std::string& data) {
      LOG(DEBUG) << "FireStringsAtPulls";
      auto pushList = CreatePushs(howManyPushs, howManyPulls, location);
      auto pullList = CreatePulls(howManyPushs, howManyPulls, location);
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
      auto pushLambda = [&](std::shared_ptr<Push> threadGun) {
         for (int i = 0; i < howManyShots / howManyPushs; i++) {
            auto thread_id = std::this_thread::get_id();
            LOG(DEBUG) << thread_id << " about to fire ";
            threadGun->Send(data);
            LOG(DEBUG) << thread_id << " fired push shot: " << data;
         }
      };
      auto pullLambda = [&](std::shared_ptr<Pull> threadPull) {
         auto thread_id = std::this_thread::get_id();
         for (int i = 0; i < howManyShots / howManyPulls; i++) {
            LOG(DEBUG) << thread_id << " waiting for push shot";
            auto queueString = threadPull->GetString();
            LOG(DEBUG) << thread_id <<  " received push shot: " << queueString;
            EXPECT_EQ(queueString, data);
         }
      };
      auto pullThreads = ListenWithPulls(pullList, pullLambda);
      auto pushThreads = ShootWithPushs(pushList, pushLambda);
      for (auto& thread:pullThreads) {
         thread.join();
      }
      for (auto& thread:pushThreads) {
         thread.join();
      }
      LOG(DEBUG) << "FireStringsAtPulls finished";
   }
   void FirePointersAtPulls(int howManyPushs,
                               int howManyShots,
                               int howManyPulls,
                               const std::string& location,
                               std::string& realData) {
      LOG(DEBUG) << "FirePointersAtPulls";
      auto sizeOfData = realData.size();
      auto pushList = CreatePushs(howManyPushs, howManyPulls, location);
      auto pullList = CreatePulls(howManyPushs, howManyPulls, location);
      auto reconstructedString = std::string(realData.c_str(), sizeOfData);
      ASSERT_EQ(reconstructedString, realData) <<
         "Should be able to convert back to string here";
      auto pushLambda = [&](std::shared_ptr<Push> threadGun) {
         for (int i = 0; i < howManyShots / howManyPushs; i++) {
            auto thread_id = std::this_thread::get_id();
            threadGun->Send(&realData);
            LOG(DEBUG) << thread_id << " fired push pointer: " << &realData;
         }
      };
      auto pullLambda = [&](std::shared_ptr<Pull> threadPull) {
         auto thread_id = std::this_thread::get_id();
         void * buf = {nullptr};
         for (int i = 0; i < howManyShots / howManyPulls; i++) {
            auto size_received = threadPull->GetPointer(buf);
            LOG(DEBUG) << thread_id <<  " received push pointer: " << buf
                       << " and " << size_received << " bytes";
            auto nanoString = static_cast<std::string*>(buf);
            EXPECT_EQ(*nanoString, realData);
         }
      };
      auto pullThreads = ListenWithPulls(pullList, pullLambda);
      auto pushThreads = ShootWithPushs(pushList, pushLambda);
      for (auto& thread:pullThreads) {
         thread.join();
      }
      for (auto& thread:pushThreads) {
         thread.join();
      }
      LOG(DEBUG) << "FirePointersAtPulls finished";
   }
}
TEST_F(PushPullTests, NonBlockingPullShouldThrow) {
   const int timeoutInMs = {5};
   Pull pull (GetIpcLocation(), timeoutInMs);
   EXPECT_THROW(auto received = {pull.GetString()}, std::runtime_error);
}
TEST_F(PushPullTests, NonBlockingPushShouldThrow) {
   const int timeoutInMs = {5};
   Push push(GetIpcLocation(), timeoutInMs);
   std::string testString("test string");
   EXPECT_THROW(push.Send(testString), std::runtime_error);
}
TEST_F(PushPullTests, ipcFilesCleanedOnDelete) {
   LOG(DEBUG) << "ipcFilesCLeanedOnDelete";
   auto ipcFile = GetIpcLocation();
   std::shared_ptr<Push> push = std::make_shared<Push>(ipcFile);
   std::shared_ptr<Pull> pull = std::make_shared<Pull>(ipcFile);
   std::string addressRealPath(ipcFile, ipcFile.find("ipc://")+6);
   ASSERT_TRUE(FileIO::DoesFileExist(addressRealPath));
   push.reset();
   pull.reset();
   ASSERT_FALSE(FileIO::DoesFileExist(addressRealPath));
   LOG(DEBUG) << "ipcFilesCLeanedOnDelete finished";
}
TEST_F(PushPullTests, FireOneString_OneToOne) {
   LOG(DEBUG) << "FireOneString_OneToOne";
   auto const numPushs = 1;
   auto const numShots = 1;
   auto const numPulls = 1;
   auto testIpcFile = std::string("FireOneString");
   auto const location = std::string("ipc:///tmp/" + testIpcFile);
   auto const exampleData = std::string("test data");
   FireStringsAtPulls(numPushs,
                         numShots,
                         numPulls,
                         location,
                         exampleData);
   LOG(DEBUG) << "FireOneString_OneToOne finished";
}
TEST_F(PushPullTests, FireOneString_OneToMany) {
   LOG(DEBUG) << "FireOneString_OneToMany";
   auto const numPushs = 1;
   auto const numShots = 8;
   auto const numPulls = 4;
   auto testIpcFile = std::string("FireOneString_OneToMany");
   auto const location = std::string("ipc:///tmp/" + testIpcFile);
   auto const exampleData = std::string("test data");
   FireStringsAtPulls(numPushs,
                         numShots,
                         numPulls,
                         location,
                         exampleData);
   LOG(DEBUG) << "FireOneString_OneToMany finished";
}
TEST_F(PushPullTests, FireOneString_ManyToOne) {
   LOG(DEBUG) << "FireOneString_ManyToOne";
   auto const numPushs = 2;
   auto const numShots = 8;
   auto const numPulls = 1;
   auto testIpcFile = std::string("FireOneString_ManyToOne");
   auto const location = std::string("ipc:///tmp/" + testIpcFile);
   auto const exampleData = std::string("test data");
   FireStringsAtPulls(numPushs,
                      numShots,
                      numPulls,
                      location,
                      exampleData);
   LOG(DEBUG) << "FireOneString_ManyToOne finished";
}
TEST_F(PushPullTests, FireMultipleStrings_OneToOne) {
   LOG(DEBUG) << "FireMultipleStrings";
   auto const numPushs = 1;
   auto const numShots = 10;
   auto const numPulls = 1;
   auto testIpcFile = std::string("FireOneString");
   auto const location = std::string("ipc:///tmp/" + testIpcFile);
   auto const exampleData = std::string("test data");
   FireStringsAtPulls(numPushs,
                      numShots,
                      numPulls,
                      location,
                      exampleData);
   LOG(DEBUG) << "FireMultipleStrings finished";
}
TEST_F(PushPullTests, FireOnePointer_OneToOne) {
   LOG(DEBUG) << "FireOneString";
   auto const numPushs = 1;
   auto const numShots = 1;
   auto const numPulls = 1;
   auto testIpcFile = std::string("FireOneString");
   auto const location = std::string("ipc:///tmp/" + testIpcFile);
   auto exampleData = std::string("test data");
   FirePointersAtPulls(numPushs,
                       numShots,
                       numPulls,
                       location,
                       exampleData);
   LOG(DEBUG) << "FireOneString finished";
}
