
#include <czmq.h>
#include <boost/thread.hpp>
#include "RifleVampireTests.h"
#include "Death.h"
#include "FileIO.h"
std::atomic<int> RifleVampireTests::mShotsDeleted;

void TestDeleteString(void*, void* data) {
   std::string* theString = reinterpret_cast<std::string*> (data);
   RifleVampireTests::mShotsDeleted++; // yes this is threadsafe
   delete theString;
}

std::string RifleVampireTests::GetTcpLocation() {
   int max = 9000;
   int min = 7000;
   int port = (rand() % (max - min)) + min;
   std::string tcpLocation("tcp://127.0.0.1:");
   tcpLocation.append(std::to_string(port));
   return tcpLocation;
}

std::string RifleVampireTests::GetIpcLocation() {
   int pid = getpid();
   std::string ipcLocation("ipc:///tmp/");
   ipcLocation.append("RifleVampireTests");
   ipcLocation.append(std::to_string(pid));
   ipcLocation.append(".ipc");
   return ipcLocation;
}

void RifleVampireTests::ShootZeroCopyThread(int numberOfMessages,
        std::string& location, std::string& exampleData, int hwm, int ioThreads,
        bool ownSocket) {
   Rifle rifle(location);
#ifdef OLD_HAT
   rifle.SetHighWater(hwm);
   rifle.SetIOThreads(ioThreads);
   if (!ownSocket) {
      rifle.SetOwnSocket(false);
   } else {
      rifle.SetOwnSocket(true);
   }
   rifle.Aim();
#endif
   std::stringstream ss;
   for (int i = 0; i < numberOfMessages && !zctx_interrupted; i++) {
#ifdef OLD_HAT
      std::string* zero = new std::string(exampleData);
      bool result = rifle.FireZeroCopy(zero, zero->size(), TestDeleteString, 10000);
      if (!result) {
         std::cout << "failed to fire" << std::endl;
      }
#endif
   }
   //std::cout << "Done Shooting" << std::endl;
   while (!zctx_interrupted) {
      boost::this_thread::sleep(boost::posix_time::seconds(1));
   }

}

/**
 * Start a vampire thread and pull off the set number of messages.
 * @param numberOfMessages
 * @param location
 * @param exampleData
 * @param hwm
 * @param ioThreads
 * @param ownSocket
 */
void RifleVampireTests::VampireThread(int numberOfMessages,
        std::string& location, std::string& exampleData, int hwm,
        int ioThreads, bool ownSocket) {
   Vampire vampire(location);
#ifdef OLD_HAT
   vampire.SetHighWater(hwm);
   vampire.SetIOThreads(ioThreads);
   vampire.SetOwnSocket(ownSocket);
   ASSERT_TRUE(vampire.PrepareToBeShot());
#endif
   for (int i=0; i < numberOfMessages && !zctx_interrupted; i++) {
      std::string bullet;
#ifdef OLD_HAT
      if (vampire.GetShot(bullet, 2000)) {
         EXPECT_EQ(bullet, exampleData);
      } else {
         //no shot in time try again
         i--;
      }
#endif
   }
   while (!zctx_interrupted) {
      boost::this_thread::sleep(boost::posix_time::seconds(1));
   }
}
