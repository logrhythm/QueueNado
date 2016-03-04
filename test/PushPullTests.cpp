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
   void FireStrings(int howManyPushs,
                    int howManyShots,
                    int howManyPulls,
                    const std::string& location,
                    const std::string& data) {
      LOG(DEBUG) << "FireStrings";
      auto pushList = CreatePushs(howManyPushs, howManyPulls, location);
      auto pullList = CreatePulls(howManyPushs, howManyPulls, location);
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
      LOG(DEBUG) << "FireStrings finished";
   }
   void FireVector(int howManyPushs,
                   int howManyShots,
                   int howManyPulls,
                   const std::string& location,
                   std::vector<std::pair<void*, unsigned int>>& data) {
      LOG(DEBUG) << "FireStrings";
      auto pushList = CreatePushs(howManyPushs, howManyPulls, location);
      auto pullList = CreatePulls(howManyPushs, howManyPulls, location);
      auto pushLambda = [&](std::shared_ptr<Push> threadGun) {
         for (int i = 0; i < howManyShots / howManyPushs; i++) {
            auto thread_id = std::this_thread::get_id();
            LOG(DEBUG) << thread_id << " about to fire ";
            threadGun->Send(data);
            LOG(DEBUG) << thread_id << " fired push shot";
         }
      };
      auto pullLambda = [&](std::shared_ptr<Pull> threadPull) {
         auto thread_id = std::this_thread::get_id();
         std::vector<std::pair<void*, unsigned int>> receiveVector;
         for (int i = 0; i < howManyShots / howManyPulls; i++) {
            LOG(DEBUG) << thread_id << " waiting for push shot";
            threadPull->GetVector(receiveVector);
            EXPECT_EQ(receiveVector.size(), data.size()) <<
               " size of received vector should be the same as sent";
            EXPECT_EQ(receiveVector, data) <<
               " received vector contents should be the same as sent";
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
      LOG(DEBUG) << "FireStrings finished";
   }
   void FirePointers(int howManyPushs,
                     int howManyShots,
                     int howManyPulls,
                     const std::string& location,
                     std::string& realData) {
      LOG(DEBUG) << "FirePointers";
      auto sizeOfData = realData.size();
      auto pushList = CreatePushs(howManyPushs, howManyPulls, location);
      auto pullList = CreatePulls(howManyPushs, howManyPulls, location);
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
      LOG(DEBUG) << "FirePointers finished";
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
   auto testIpcFile = std::string("FireOneString_OneToOne");
   auto const location = std::string("ipc:///tmp/" + testIpcFile);
   auto const exampleData = std::string("test data");
   FireStrings(numPushs,
               numShots,
               numPulls,
               location,
               exampleData);
   LOG(DEBUG) << "FireOneString_OneToOne finished";
}
TEST_F(PushPullTests, FireManyStrings_OneToOne) {
   LOG(DEBUG) << "FireManyStrings_OneToOne";
   auto const numPushs = 1;
   auto const numShots = 16;
   auto const numPulls = 1;
   auto testIpcFile = std::string("FireManyString_OneToOne");
   auto const location = std::string("ipc:///tmp/" + testIpcFile);
   auto const exampleData = std::string("test data");
   FireStrings(numPushs,
               numShots,
               numPulls,
               location,
               exampleData);
   LOG(DEBUG) << "FireManyStrings_OneToOne finished";
}
TEST_F(PushPullTests, FireManyStrings_OneToMany) {
   LOG(DEBUG) << "FireManyStrings_OneToMany";
   auto const numPushs = 1;
   auto const numShots = 16;
   auto const numPulls = 4;
   auto testIpcFile = std::string("FireManyStrings_OneToMany");
   auto const location = std::string("ipc:///tmp/" + testIpcFile);
   auto const exampleData = std::string("test data");
   FireStrings(numPushs,
               numShots,
               numPulls,
               location,
               exampleData);
   LOG(DEBUG) << "FireManyStrings_OneToMany finished";
}
TEST_F(PushPullTests, FireManyStrings_ManyToOne) {
   LOG(DEBUG) << "FireManyStrings_ManyToOne";
   auto const numPushs = 4;
   auto const numShots = 16;
   auto const numPulls = 1;
   auto testIpcFile = std::string("FireManyStrings_ManyToOne");
   auto const location = std::string("ipc:///tmp/" + testIpcFile);
   auto const exampleData = std::string("test data");
   FireStrings(numPushs,
               numShots,
               numPulls,
               location,
               exampleData);
   LOG(DEBUG) << "FireManyStrings_ManyToOne finished";
}
TEST_F(PushPullTests, FireOnePointer_OneToOne) {
   LOG(DEBUG) << "FireOnePointer_OneToOne";
   auto const numPushs = 1;
   auto const numShots = 1;
   auto const numPulls = 1;
   auto testIpcFile = std::string("FireOnePointer_OneToOne");
   auto const location = std::string("ipc:///tmp/" + testIpcFile);
   auto exampleData = std::string("test data");
   FirePointers(numPushs,
                numShots,
                numPulls,
                location,
                exampleData);
   LOG(DEBUG) << "FireOnePointer_OneToOne finished";
}
TEST_F(PushPullTests, FireManyPointers_OneToMany) {
   LOG(DEBUG) << "FireManyPointers_OneToMany";
   auto const numPushs = 1;
   auto const numShots = 16;
   auto const numPulls = 4;
   auto testIpcFile = std::string("FireManyPointers_OneToMany");
   auto const location = std::string("ipc:///tmp/" + testIpcFile);
   auto exampleData = std::string("test data");
   FirePointers(numPushs,
                numShots,
                numPulls,
                location,
                exampleData);
   LOG(DEBUG) << "FireManyPointers_OneToMany finished";
}
TEST_F(PushPullTests, FireManyPointers_ManyToOne) {
   LOG(DEBUG) << "FireManyPointers_ManyToOne";
   auto const numPushs = 4;
   auto const numShots = 16;
   auto const numPulls = 1;
   auto testIpcFile = std::string("FireManyPointers_ManyToOne");
   auto const location = std::string("ipc:///tmp/" + testIpcFile);
   auto exampleData = std::string("test data");
   FirePointers(numPushs,
                numShots,
                numPulls,
                location,
                exampleData);
   LOG(DEBUG) << "FireManyPointers_ManyToOne finished";
}
TEST_F(PushPullTests, FireOneVector_OneToOne) {
   LOG(DEBUG) << "FireOneVector_OneToOne";
   auto const numPushs = 1;
   auto const numShots = 1;
   auto const numPulls = 1;
   auto testIpcFile = std::string("FireOneVector_OneToOne");
   auto const location = std::string("ipc:///tmp/" + testIpcFile);
   std::vector<std::pair<void*, unsigned int>> exampleData;
   auto vectorData = std::pair<void*, unsigned int>(const_cast<int*>(&numPushs),
                                                    numShots);
   exampleData.push_back(vectorData);
   FireVector(numPushs,
              numShots,
              numPulls,
              location,
              exampleData);
   LOG(DEBUG) << "FireOneVector_OneToOne finished";
}
TEST_F(PushPullTests, FireManyVectors_OneToMany) {
   LOG(DEBUG) << "FireManyVectors_OneToMany";
   auto const numPushs = 1;
   auto const numShots = 16;
   auto const numPulls = 4;
   auto testIpcFile = std::string("FireManyVectors_OneToMany");
   auto const location = std::string("ipc:///tmp/" + testIpcFile);
   std::vector<std::pair<void*, unsigned int>> exampleData;
   auto vectorData = std::pair<void*, unsigned int>(const_cast<int*>(&numPushs),
                                                    numShots);
   exampleData.push_back(vectorData);
   FireVector(numPushs,
              numShots,
              numPulls,
              location,
              exampleData);
   LOG(DEBUG) << "FireManyVectors_OneToMany finished";
}
TEST_F(PushPullTests, FireManyVectors_ManyToOne) {
   LOG(DEBUG) << "FireManyVectors_ManyToOne";
   auto const numPushs = 4;
   auto const numShots = 16;
   auto const numPulls = 1;
   auto testIpcFile = std::string("FireManyVectors_ManyToOne");
   auto const location = std::string("ipc:///tmp/" + testIpcFile);
   std::vector<std::pair<void*, unsigned int>> exampleData;
   auto vectorData = std::pair<void*, unsigned int>(const_cast<int*>(&numPushs),
                                                    numShots);
   exampleData.push_back(vectorData);
   FireVector(numPushs,
              numShots,
              numPulls,
              location,
              exampleData);
   LOG(DEBUG) << "FireManyVectors_ManyToOne finished";
}
