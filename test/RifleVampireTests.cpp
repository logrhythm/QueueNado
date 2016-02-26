#include <czmq.h>
#include <boost/thread.hpp>
#include "RifleVampireTests.h"
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
      ipcLocation.append("RifleVampireTests");
      ipcLocation.append(std::to_string(pid));
      ipcLocation.append(".ipc");
      return ipcLocation;
   }

   std::vector<std::shared_ptr<Rifle>> CreateRifles(int howMany,
                                   const std::string& location) {
      std::vector<std::shared_ptr<Rifle>> toReturn;
      for (int i = 0; i < howMany ; i ++) {
         toReturn.push_back(std::make_shared<Rifle>(location));
      }
      return toReturn;
   }
   std::vector<std::shared_ptr<Vampire>> CreateVampires(int howMany,
                                      const std::string& location) {
      std::vector<std::shared_ptr<Vampire>> toReturn;
      for (int i = 0; i < howMany ; i ++) {
         toReturn.push_back(std::make_shared<Vampire>(location));
      }
      return toReturn;
   }
   std::vector<std::thread> ListenWithVampires(std::vector<std::shared_ptr<Vampire>>& vampires,
                                               std::function<void(std::shared_ptr<Vampire>)> vampLambda) {
      std::vector<std::thread> threads;
      for (auto& vamp:vampires) {
         threads.push_back(std::thread(vampLambda, vamp));
      }
      return threads;
   }
   std::vector<std::thread> ShootWithRifles(std::vector<std::shared_ptr<Rifle>>& rifles,
                        std::function<void(std::shared_ptr<Rifle>)> rifleLambda) {
      std::vector<std::thread> threads;
      for (auto& rifle:rifles) {
         threads.push_back(std::thread(rifleLambda, rifle));
      }
      return threads;
   }
   void FireStringsAtVampires(int howManyRifles,
                            int howManyShots,
                            int howManyVampires,
                            const std::string& location,
                            const std::string& data) {
      LOG(DEBUG) << "FireStringsAtVampires";
      auto rifleList = CreateRifles(howManyRifles, location);
      auto vampireList = CreateVampires(howManyVampires, location);
      auto rifleLambda = [&](std::shared_ptr<Rifle> threadGun) {
         for (int i = 0; i < howManyShots; i++) {
            threadGun->Fire(data);
         }
      };
      auto vampireLambda = [&](std::shared_ptr<Vampire> threadVamp) {
         for (int i = 0; i < howManyShots; i++) {
            auto queueString = threadVamp->GetShot();
            EXPECT_EQ(queueString, data);
         }
      };
      auto vampThreads = ListenWithVampires(vampireList, vampireLambda);
      auto rifleThreads = ShootWithRifles(rifleList, rifleLambda);
      for (auto& thread:vampThreads) {
         thread.join();
      }
      for (auto& thread:rifleThreads) {
         thread.join();
      }
      LOG(DEBUG) << "FireStringsAtVampires finished";
   }
}
TEST_F(RifleVampireTests, VampireTimeout) {
   const int timeoutInMs = {5};
   Vampire vamp (GetIpcLocation(), timeoutInMs);
   EXPECT_THROW(auto received = {vamp.GetShot()}, std::runtime_error);
}
TEST_F(RifleVampireTests, TimeoutRifleShouldThrow) {
   const int timeoutInMs = {5};
   Rifle rifle(GetIpcLocation(), timeoutInMs);
   std::string testString("test string");
   EXPECT_THROW(rifle.Fire(testString), std::runtime_error);
}
TEST_F(RifleVampireTests, ipcFilesCleanedOnDelete) {
   LOG(DEBUG) << "ipcFilesCLeanedOnDelete";
   auto ipcFile = GetIpcLocation();
   std::shared_ptr<Rifle> rifle = std::make_shared<Rifle>(ipcFile);
   std::shared_ptr<Vampire> vamp = std::make_shared<Vampire>(ipcFile);
   std::string addressRealPath(ipcFile, ipcFile.find("ipc://")+6);
   ASSERT_TRUE(FileIO::DoesFileExist(addressRealPath));
   rifle.reset();
   vamp.reset();
   ASSERT_FALSE(FileIO::DoesFileExist(addressRealPath));
   LOG(DEBUG) << "ipcFilesCLeanedOnDelete finished";
}
TEST_F(RifleVampireTests, ipcFilesCleanedOnFatal) {
   LOG(DEBUG) << "ipcFilesCLeanedOnFatal";
   auto ipcFile = GetIpcLocation();
   Rifle rifle(ipcFile);
   Vampire vamp(ipcFile);
   std::string addressRealPath(ipcFile, ipcFile.find("ipc://")+6);
   ASSERT_TRUE(FileIO::DoesFileExist(addressRealPath));
   Death::SetupExitHandler();
   CHECK(false);
   LOG(DEBUG) << "recovered from fatal";
   std::this_thread::sleep_for(std::chrono::milliseconds(500));
   ASSERT_FALSE(FileIO::DoesFileExist(addressRealPath));
   LOG(DEBUG) << "ipcFilesCLeanedOnFatal finished";
}
