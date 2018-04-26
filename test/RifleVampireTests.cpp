
#include <czmq.h>
#include <boost/thread.hpp>
#include "RifleVampireTests.h"
#include "Death.h"
#include "FileIO.h"
#include <TimeStats.h>
#include <TriggerTimeStats.h>
#include <g3log/g3log.hpp>
#include <QAPI.h>
#include <q/spsc.hpp>
#include <q/mpmc.hpp>
#include <future>
#include <QueueNadoMacros.h>
#include <limits>

namespace {
   const int kNoWaitTimeMs = 0;
   const int kWaitTimeMs = 500;
   const int kLongWaitTimeMs = 1500;
}
std::atomic<int> RifleVampireTests::mShotsDeleted;

void TestDeleteString(void*, void* data) {
   std::string* theString = reinterpret_cast<std::string*> (data);
   RifleVampireTests::mShotsDeleted++; // yes this is threadsafe
   delete theString;
}

/**
 * Used to call shutdown just for test.
 * @param location
 */
class TestVampire : public Vampire {
 public:

   TestVampire(std::string location) : Vampire(location) {
   }

   void Destroy() {
      Vampire::Destroy();
   }
};

/**
 * Used to call shutdown just for test.
 * @param location
 */
class TestRifle : public Rifle {
 public:

   TestRifle(std::string location) : Rifle(location) {
   }

   void Destroy() {
      Rifle::Destroy();
   }
};

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


void RifleVampireTests::RifleThread(int numberOfMessages,
                                    std::string& location, std::string& exampleData, int hwm,
                                    int ioThreads, bool ownSocket) {
   Rifle rifle(location);
   rifle.SetHighWater(hwm);
   rifle.SetIOThreads(ioThreads);
   rifle.SetOwnSocket(ownSocket);
   rifle.Aim();
   std::stringstream ss;
   TimeStats stats;
   for (int i = 0; i < numberOfMessages; i++) {
      TriggerTimeStats trigger(stats);
      bool result = rifle.Fire(exampleData);
      if (!result) {
         trigger.Skip();
         std::cout << "failed to fire" << std::endl;
      }
      //      std::cout << "shot fired:" << result << std::endl;
      //      boost::this_thread::sleep(boost::posix_time::microseconds(sleepInterval));
   }
   //std::cout << "Done Shooting" << std::endl;
   while (!zctx_interrupted) {
      boost::this_thread::sleep(boost::posix_time::seconds(1));
   }
   std::cout << "Rifle : " << stats.FlushAsString() << std::endl;


}

void RifleVampireTests::ShootStakeThread(int numberOfMessages,
      std::string& location, std::vector<std::pair<void*, unsigned int> >& exampleData,
      bool ownSocket) {
   Rifle rifle(location);
   rifle.SetHighWater(1);
   rifle.SetIOThreads(1);
   if (!ownSocket) {
      rifle.SetOwnSocket(false);
   } else {
      rifle.SetOwnSocket(true);
   }
   rifle.Aim();
   std::stringstream ss;
   TimeStats stats;
   for (int i = 0; i < numberOfMessages; i++) {
      TriggerTimeStats trigger(stats);
      bool result = rifle.FireStakes(exampleData);
      if (!result) {
         trigger.Skip();
         std::cout << "failed to fire" << std::endl;
      }
      //      std::cout << "shot fired:" << result << std::endl;
      //      boost::this_thread::sleep(boost::posix_time::microseconds(sleepInterval));
   }
   std::cout << "Rifle : " << stats.FlushAsString() << std::endl;
   while (!zctx_interrupted) {
      boost::this_thread::sleep(boost::posix_time::seconds(1));
   }
}

void RifleVampireTests::ShootZeroCopyThread(int numberOfMessages,
      std::string& location, std::string& exampleData, int hwm, int ioThreads,
      bool ownSocket) {
   Rifle rifle(location);
   rifle.SetHighWater(hwm);
   rifle.SetIOThreads(ioThreads);
   if (!ownSocket) {
      rifle.SetOwnSocket(false);
   } else {
      rifle.SetOwnSocket(true);
   }
   rifle.Aim();
   std::stringstream ss;
   TimeStats stats;
   for (int i = 0; i < numberOfMessages && !zctx_interrupted; i++) {
      std::string* zero = new std::string(exampleData);
      TriggerTimeStats trigger(stats);
      bool result = rifle.FireZeroCopy(zero, zero->size(), TestDeleteString, 10000);
      if (!result) {
         trigger.Skip();
         std::cout << "failed to fire" << std::endl;
      }
      //      std::cout << "shot fired:" << result << std::endl;
      //      boost::this_thread::sleep(boost::posix_time::microseconds(sleepInterval));
   }
   std::cout << "Rifle : " << stats.FlushAsString() << std::endl;
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
                                      int ioThreads, bool ownSocket, int waitTimeMs) {
   Vampire vampire(location);
   vampire.SetHighWater(hwm);
   vampire.SetIOThreads(ioThreads);
   vampire.SetOwnSocket(ownSocket);
   ASSERT_TRUE(vampire.PrepareToBeShot());
   TimeStats stats;
   for (int i = 0; i < numberOfMessages && !zctx_interrupted; i++) {
      std::string bullet;
      TriggerTimeStats trigger(stats);
      if (vampire.GetShot(bullet, waitTimeMs)) {
         EXPECT_EQ(bullet, exampleData);
      } else {
         trigger.Skip();
         //no shot in time try again
         i--;
      }
   }
   std::cout << "Vampire : " << stats.FlushAsString() << std::endl;
   while (!zctx_interrupted) {
      boost::this_thread::sleep(boost::posix_time::seconds(1));
   }
}




void RifleVampireTests::StakeAVampireThread(int numberOfMessages,
      std::string& location,
      std::vector<std::pair<void*, unsigned int> >& exampleData,
      int hwm, int ioThreads, int waitTimeMs) {
   Vampire vampire(location);
   vampire.SetHighWater(hwm);
   vampire.SetIOThreads(ioThreads);
   ASSERT_TRUE(vampire.PrepareToBeShot());
   TimeStats stats;
   for (int i = 0; i < numberOfMessages && !zctx_interrupted; i++) {
      std::vector<std::pair<void*, unsigned int> > data;
      TriggerTimeStats trigger(stats);
      if (vampire.GetStakes(data, waitTimeMs)) {
         auto it = data.begin();
         auto jt = exampleData.begin();
         for (; it != data.end() && !zctx_interrupted; it++, jt++) {
            EXPECT_EQ(it->first, jt->first);
            EXPECT_EQ(it->second, jt->second);
         }
      } else {
         trigger.Skip();
         //no shot in time try again
         i--;
      }
   }
   std::cout << "Vampire : " << stats.FlushAsString() << std::endl;
   while (!zctx_interrupted) {
      boost::this_thread::sleep(boost::posix_time::seconds(1));
   }
}

void RifleVampireTests::OneRifleNVampiresBenchmark(int nVampires, int nIOThreads,
      int rifleHWM, int vampireHWM, std::string& location, int dataSize, int nShotsPerVampire, int expectedSpeed, int waitTimeMs) {
   bool bRifleOwnSocket = true;
   Rifle* rifle = new Rifle(location);
   rifle->SetHighWater(rifleHWM);
   rifle->SetIOThreads(nIOThreads);
   rifle->SetOwnSocket(bRifleOwnSocket);
   EXPECT_TRUE(rifle->Aim());
#if RIFLE_VAMPIRE_PRODUCTION == 0
   delete rifle;
   return;
#endif

   std::string exampleData(dataSize, 'a');
   std::vector<boost::thread*> theVampires;
   for (int i = 0; i < nVampires && !zctx_interrupted; i++) {
      boost::thread* aShooter = new boost::thread(
         &RifleVampireTests::VampireThread, this, nShotsPerVampire, location,
         exampleData, vampireHWM, nIOThreads, !bRifleOwnSocket, waitTimeMs);
      theVampires.push_back(aShooter);
   }
   sleep(2);
   //Send more then we can handle, break if we can't send any more.
   int fullSize = (nShotsPerVampire * nVampires) + ((vampireHWM * (nVampires)) * 2) + rifleHWM;
   SetExpectedTime(fullSize, exampleData.size() * sizeof (char), expectedSpeed, 20000L);
   StartTimedSection();
   TimeStats stats;
   for (int i = 0; i < fullSize && !zctx_interrupted; i++) {
      TriggerTimeStats trigger(stats);
      if (!rifle->Fire(exampleData, waitTimeMs)) {
         trigger.Skip();
         std::cout << "Failed to fire... Vampires might all be dead..." << std::endl;
         break;
      }
   }
   std::cout << "One Rifle --> #" << nVampires << " Vampires: " << stats.FlushAsString() << std::endl;
   for (auto it = theVampires.begin();
         it != theVampires.end() && !zctx_interrupted; it++) {
      (*it)->interrupt();
      (*it)->join();
      delete *it;
   }
   EndTimedSection();
   EXPECT_TRUE(TimedSectionPassed());
   delete rifle;
}


  template <typename Sender>
   size_t Push(Sender q, int dataSize, int stop, std::atomic<bool>& stopRunning) {
      using namespace std::chrono_literals;
      std::vector<std::string> expected;
      std::string exampleData(dataSize, 'a');
      size_t pushed = 0;

      using namespace std::chrono_literals;
      for (auto i = 0; i < stop; ++i) {
         std::string sendData = exampleData;
         if (stopRunning.load()) {
            return pushed;
         }
         while (false == q.push(sendData)) {
            std::this_thread::sleep_for(1us); // // yield is too aggressive
            if (stopRunning.load()) {
               return pushed;
            }
         }
         ++pushed;
      }
      std::cout << q.mStats.FlushAsString() << std::endl;
      return pushed;
   }

  template <typename Receiver>
   size_t Get(Receiver q, int dataSize, int stop, std::atomic<bool>& stopRunning) {
      using namespace std::chrono_literals;
      std::vector<std::string> expected;
      const std::string exampleData(dataSize, 'a');
      size_t received = 0;

      using namespace std::chrono_literals;
      for (auto i = 0; i < stop; ++i) {
         if (stopRunning.load()) {
            return received;
         }
         std::string incoming;
         while (false == q.pop(incoming)) {
            std::this_thread::sleep_for(1us); // // yield is too aggressive
            if (stopRunning.load()) {
               return received;
            }
         }
         ++received;
         if (!stopRunning.load()) {
            EXPECT_EQ(exampleData, incoming);
         }
      }
      std::cout << q.mStats.FlushAsString() << std::endl;
      return received;
   }




void RifleVampireTests::QueueSPSCBenchmark(int dataSize, int nHowMany, int expectedSpeed) {

   const size_t kQueueSize = 100;
   auto queue = QAPI::CreateQueue<spsc::flexible::circular_fifo<std::string>>(kQueueSize);
   const std::string exampleData(dataSize, 'a');
   SetExpectedTime(nHowMany, exampleData.size() * sizeof (char), expectedSpeed, 20000L);

   auto producer = std::get <QAPI::index::sender>(queue);
   auto consumer = std::get <QAPI::index::receiver>(queue);

   StartTimedSection();
   std::atomic<bool> stopRunning{false};

   auto sentResult = std::async(std::launch::async, Push<decltype(producer)>,
                                producer, dataSize, nHowMany, std::ref(stopRunning));
   auto receivedResult = std::async(std::launch::async, Get<decltype(consumer)>,
                                    consumer, dataSize, nHowMany, std::ref(stopRunning));
   auto sent = sentResult.get();
   auto received = receivedResult.get();
   EndTimedSection();
   EXPECT_TRUE(TimedSectionPassed());
   EXPECT_GE(received + kQueueSize, sent);
}


void RifleVampireTests::QueueMPMCBenchmark(int numSenders, int numReceivers, int dataSize, int nHowMany, int expectedSpeed) {
   
   const size_t kQueueSize = 100;
   auto queue = QAPI::CreateQueue<mpmc::dynamic_lock_queue<std::string>>(kQueueSize);
   const std::string exampleData(dataSize, 'a');
   SetExpectedTime(nHowMany, exampleData.size() * sizeof (char), expectedSpeed, 20000L);

   auto producer = std::get <QAPI::index::sender>(queue);
   auto consumer = std::get <QAPI::index::receiver>(queue);

   StartTimedSection();
   std::vector<std::future<size_t>> sentResult;
   sentResult.reserve(numSenders);
   std::vector<std::future<size_t>> receiveResult;
   receiveResult.reserve(numReceivers);
   std::atomic<bool> stopRunning{false};
   for (int i = 0; i < numReceivers; ++i) {
      receiveResult.emplace_back(std::async(std::launch::async, Get<decltype(consumer)>,
                                    consumer, dataSize, nHowMany, std::ref(stopRunning)));
   }

   for (int i = 0; i < numSenders; ++i) {
      sentResult.emplace_back(std::async(std::launch::async, Push<decltype(producer)>,
                                    producer, dataSize, nHowMany, std::ref(stopRunning)));
   }
   
   StopWatch timecheck;
   size_t totalSent = 0;
   for (int i = 0; i < numSenders; ++i) {
      totalSent = sentResult[i].get();
   }

   stopRunning.store(true);
   size_t totalReceived = 0;
   for (int i = 0; i < numSenders; ++i) {
      totalReceived = receiveResult[i].get();
   }
   EndTimedSection();
   EXPECT_TRUE(TimedSectionPassed());
   std::cout << "Actual throughput: " << totalReceived * dataSize << " bytes" << std::endl;
   std::cout << "Total time elapsed: " << timecheck.ElapsedSec() << std::endl;
   std::cout << "Transaction/second: " << totalReceived/timecheck.ElapsedSec() << std::endl;
   std::cout << "Transaction speed: " << ((totalReceived * dataSize / 1000)/timecheck.ElapsedSec()) << " Mbps" << std::endl; 
   EXPECT_GE(totalReceived + kQueueSize, totalSent);
}


void RifleVampireTests::QueueMPMCBenchmark_1Minute(int numSenders, int numReceivers, int dataSize, int nHowMany, int expectedSpeed) {
   
   const size_t kQueueSize = 100;
   auto queue = QAPI::CreateQueue<mpmc::dynamic_lock_queue<std::string>>(kQueueSize);
   const std::string exampleData(dataSize, 'a');
   SetExpectedTime(nHowMany, exampleData.size() * sizeof (char), expectedSpeed, 20000L);

   auto producer = std::get <QAPI::index::sender>(queue);
   auto consumer = std::get <QAPI::index::receiver>(queue);

   StartTimedSection();
   std::vector<std::future<size_t>> sentResult;
   sentResult.reserve(numSenders);
   std::vector<std::future<size_t>> receiveResult;
   receiveResult.reserve(numReceivers);
   std::atomic<bool> stopRunning{false};

   for (int i = 0; i < numReceivers; ++i) {
      receiveResult.emplace_back(std::async(std::launch::async, Get<decltype(consumer)>,
                                    consumer, dataSize, nHowMany, std::ref(stopRunning)));
   }

   for (int i = 0; i < numSenders; ++i) {
      sentResult.emplace_back(std::async(std::launch::async, Push<decltype(producer)>,
                                    producer, dataSize, nHowMany, std::ref(stopRunning)));
   }
   

   StopWatch timecheck;
   using namespace std::chrono_literals;
   while(timecheck.ElapsedSec() < 60) {
      std::this_thread::sleep_for(10ms);
   }
   stopRunning.store(true);

   size_t totalSent = 0;
   for (int i = 0; i < numSenders; ++i) {
      totalSent = sentResult[i].get();
   }

   stopRunning.store(true);
   size_t totalReceived = 0;
   for (int i = 0; i < numSenders; ++i) {
      totalReceived = receiveResult[i].get();
   }
   EndTimedSection();
   EXPECT_TRUE(TimedSectionPassed());
   std::cout << "Actual throughput: " << ((totalReceived * dataSize)/(1024 * 1024)) << " MBytes" << std::endl;
   std::cout << "Total time elapsed: " << timecheck.ElapsedSec() << std::endl;
   std::cout << "Transaction/second: " << totalReceived/timecheck.ElapsedSec() << std::endl;
   std::cout << "Transaction speed: " << ((totalReceived * dataSize / (1024 * 1024))/timecheck.ElapsedSec()) << " Mbyte/s" << std::endl; 
   EXPECT_GE(totalReceived + kQueueSize, totalSent);
}




void RifleVampireTests::OneRifleNVampiresStakeBenchmark(int nVampires, int nIOThreads,
      int rifleHWM, int vampireHWM, std::string& location, int dataSize, int nShotsPerVampire, int expectedSpeed, int waitTimeMs) {
   Rifle* rifle = new Rifle(location);
   rifle->SetHighWater(rifleHWM);
   rifle->SetIOThreads(nIOThreads);
   rifle->SetOwnSocket(true);
   EXPECT_TRUE(rifle->Aim());
#if RIFLE_VAMPIRE_PRODUCTION == 0
   delete rifle;
   return;
#endif
   std::string exampleString(dataSize, 'a');
   std::vector<std::pair< void*, unsigned int> > exampleData;
   for (int i = 0; i < SIZE_OF_STAKE_BUNDLE && !zctx_interrupted; i++) {
      exampleData.push_back(make_pair(&exampleString, 1234));
   }
   std::vector<boost::thread*> theVampires;

   for (int i = 0; i < nVampires && !zctx_interrupted; i++) {
      boost::thread* aShooter = new boost::thread(&RifleVampireTests::StakeAVampireThread,
            this, nShotsPerVampire, location, exampleData, vampireHWM, nIOThreads, waitTimeMs);
      theVampires.push_back(aShooter);
   }
   sleep(2);
   //Send more then we can handle, break if we can't send any more.
   int fullSize = (nShotsPerVampire * nVampires) + ((vampireHWM * (nVampires)) * 2) + rifleHWM;
   SetExpectedTime(fullSize, exampleData.size() * sizeof (char) * SIZE_OF_STAKE_BUNDLE, expectedSpeed, 20000L);
   StartTimedSection();
   TimeStats stats;
   for (int i = 0; i < fullSize && !zctx_interrupted; i++) {
      TriggerTimeStats trigger(stats);
      if (!rifle->FireStakes(exampleData, waitTimeMs)) {
         std::cout << "Failed to fire at shot " << i << " of fullSize " << fullSize << std::endl;
         std::cout << " ... Vampires might all be dead..." << std::endl;
         trigger.Skip();
         break;
      }
   }
   std::cout << "One Rifle -->  #" << nVampires << " Vampires. " << stats.FlushAsString() << std::endl;
   for (auto it = theVampires.begin();
         it != theVampires.end() && !zctx_interrupted; it++) {
      (*it)->interrupt();
      (*it)->join();
      delete *it;
   }
   EndTimedSection();
   EXPECT_TRUE(TimedSectionPassed());
   delete rifle;
}

void RifleVampireTests::NRiflesOneVampireBenchmarkZeroCopy(int nRifles, int nIOThreads,
      int rifleHWM, int vampireHWM, std::string& location, int dataSize,
      int nShotsPerRifle, int expectedSpeed, int waitTimeMs) {

   bool bVampireOwnSocket = true;
   mShotsDeleted.store(0);
   Vampire vampire(location);
   vampire.SetHighWater(vampireHWM);
   vampire.SetIOThreads(nIOThreads);
   vampire.SetOwnSocket(bVampireOwnSocket);
   ASSERT_TRUE(vampire.PrepareToBeShot());
#if RIFLE_VAMPIRE_PRODUCTION == 0
   return;
#endif

   std::string exampleData(dataSize, 'z');
   std::vector<boost::thread*> theRifles;

   for (int i = 0; i < nRifles && !zctx_interrupted; i++) {
      boost::thread* aShooter = new boost::thread(
         &RifleVampireTests::ShootZeroCopyThread, this, nShotsPerRifle, location,
         exampleData, rifleHWM, nIOThreads, !bVampireOwnSocket);
      theRifles.push_back(aShooter);
   }

   SetExpectedTime((nShotsPerRifle * nRifles), // numberOfPackets
                   (exampleData.size() * sizeof (char)), // packetSize
                   expectedSpeed, // RateInMbps
                   20000L); //transationsPerSection
   StartTimedSection();
   std::string bullet;
   TimeStats stats;
   for (int pktCount = 0; pktCount < (nShotsPerRifle * nRifles) && !zctx_interrupted; pktCount++) {
      TriggerTimeStats trigger(stats);
      if (vampire.GetShot(bullet, waitTimeMs)) {
         EXPECT_EQ(bullet, exampleData);
      } else {
         trigger.Skip();
         pktCount--; // Packet not received in time, try again.
      }
   }
   std::cout << "#" << nRifles << " Rifles --> one Vampire. " << stats.FlushAsString() << std::endl;
   for (auto it = theRifles.begin();
         it != theRifles.end() && !zctx_interrupted; it++) {
      (*it)->interrupt();
      (*it)->join();
      delete *it;
   }
   EndTimedSection();
   EXPECT_TRUE(TimedSectionPassed());
   EXPECT_EQ(nShotsPerRifle * nRifles, mShotsDeleted);
}

void RifleVampireTests::NRiflesOneVampireBenchmark(int nRifles, int nIOThreads,
      int rifleHWM, int vampireHWM, std::string& location, int dataSize,
      int nShotsPerRifle, int expectedSpeed, int waitTimeMs) {

   bool bVampireOwnSocket = true;
   Vampire vampire(location);
   vampire.SetHighWater(vampireHWM);
   vampire.SetIOThreads(nIOThreads);
   vampire.SetOwnSocket(bVampireOwnSocket);
   ASSERT_TRUE(vampire.PrepareToBeShot());
#if RIFLE_VAMPIRE_PRODUCTION == 0
   return;
#endif

   std::string exampleData(dataSize, 'z');
   std::vector<boost::thread*> theRifles;

   for (int i = 0; i < nRifles && !zctx_interrupted; i++) {
      boost::thread* aShooter = new boost::thread(
         &RifleVampireTests::RifleThread, this, nShotsPerRifle, location,
         exampleData, rifleHWM, nIOThreads, !bVampireOwnSocket);
      theRifles.push_back(aShooter);
   }

   SetExpectedTime((nShotsPerRifle * nRifles), // numberOfPackets
                   (exampleData.size() * sizeof (char)), // packetSize
                   expectedSpeed, // RateInMbps
                   20000L); //transationsPerSection
   StartTimedSection();
   std::string bullet;
   TimeStats stats;
   for (int pktCount = 0; pktCount < (nShotsPerRifle * nRifles) && !zctx_interrupted; pktCount++) {
      TriggerTimeStats trigger(stats);
      if (vampire.GetShot(bullet, waitTimeMs)) {
         EXPECT_EQ(bullet, exampleData);
      } else {
         pktCount--; // Packet not received in time, try again.
         trigger.Skip();
      }
   }

   std::cout << "#" << nRifles << " Rifles --> one Vampire. " << stats.FlushAsString() << std::endl;
   for (auto it = theRifles.begin();
         it != theRifles.end() && !zctx_interrupted; it++) {
      (*it)->interrupt();
      (*it)->join();
      delete *it;
   }
   EndTimedSection();
   EXPECT_TRUE(TimedSectionPassed());
}

TEST_F(RifleVampireTests, ipcFilesCleanedOnNormalExitRifleOwner) {
   std::string target("ipc:///rifleVampireExit");
   std::string addressRealPath(target, target.find("ipc://") + 6);
   {
      Rifle stick{target};
      {
         Vampire vamp{target};
         ASSERT_TRUE(stick.Aim());
         ASSERT_TRUE(vamp.PrepareToBeShot());
         ASSERT_TRUE(FileIO::DoesFileExist(addressRealPath));
      }
      // Vampire falls out of scope and ipc file is not deleted
      ASSERT_TRUE(FileIO::DoesFileExist(addressRealPath)) << "ipc file unexpectedly deleted: " << addressRealPath;
   }
   // Rifle falls out of scope and ipc file is cleaned up
   ASSERT_FALSE(FileIO::DoesFileExist(addressRealPath)) << "Rifle did not clean up ipc file: " << addressRealPath;
}

TEST_F(RifleVampireTests, ipcFilesCleanedOnNormalExitVampireOwner) {
   std::string target("ipc:///rifleVampireExit");
   std::string addressRealPath(target, target.find("ipc://") + 6);
   {
      Vampire vamp{target};
      vamp.SetOwnSocket(true);
      {
         Rifle stick{target};
         stick.SetOwnSocket(false);
         ASSERT_TRUE(stick.Aim());
         ASSERT_TRUE(vamp.PrepareToBeShot());
         ASSERT_TRUE(FileIO::DoesFileExist(addressRealPath));
      }
      // Rifle falls out of scope and ipc file is not deleted
      ASSERT_TRUE(FileIO::DoesFileExist(addressRealPath)) << "ipc file unexpectedly deleted: " << addressRealPath;
   }
   // Vampire falls out of scope and ipc file is cleaned up
   ASSERT_FALSE(FileIO::DoesFileExist(addressRealPath)) << "Vampire did not clean up ipc file: " << addressRealPath;
}

TEST_F(RifleVampireTests, ipcFilesCleanedOnFatal) {
   std::string target("ipc:///rifleVampireDeath");
   Rifle stick{target};
   Vampire vamp{target};
   std::string addressRealPath(target, target.find("ipc://") + 6);
   Death::SetupExitHandler();
   ASSERT_TRUE(stick.Aim());
   ASSERT_TRUE(vamp.PrepareToBeShot());
   ASSERT_TRUE(FileIO::DoesFileExist(addressRealPath));
   CHECK(false);
   ASSERT_FALSE(FileIO::DoesFileExist(addressRealPath));
}

/**
*
* The Rifle Vampire performance tests are NOT measuring correctly how much the throughput is and what the actual 
* time it took to process them. For that reason all the performance tests are disabled with #if 0
* 
* For the time being they are being left in case we want to revisit similar performance tests later for the new
* queues that we are doing.
*
* At that time these obsolete tests should be replaced and deleted
*/ 


#if 0

TEST_F(RifleVampireTests, RifleOwnsSocketOneRifleOneVampireIPCLargeSize) {
   if (geteuid() == 0) {
      std::string location = GetIpcLocation();
      int nVampires = 1;
      int nIOThreads = 1;
      int rifleHWM = 1000;
      int vampireHWM = 500;
      int dataSize = 56554;
      int nShotsPerVampire = 200000;
      int expectedSpeed = 1000;
      OneRifleNVampiresBenchmark(nVampires, nIOThreads, rifleHWM, vampireHWM, location, dataSize, nShotsPerVampire, expectedSpeed, kWaitTimeMs);
   }
}



TEST_F(RifleVampireTests, RifleOwnsSocketOneRifleOneVampireIPCSmallSize) {
   if (geteuid() == 0) {
      std::string location = GetIpcLocation();
      int nVampires = 1;
      int nIOThreads = 1;
      int rifleHWM = 120000;
      int vampireHWM = 30000;
      int dataSize = 100;
      int nShotsPerVampire = 1000000;
      int expectedSpeed = 50;
      OneRifleNVampiresBenchmark(nVampires, nIOThreads, rifleHWM, vampireHWM, location, dataSize, nShotsPerVampire, expectedSpeed, kWaitTimeMs);
   }

}




TEST_F(RifleVampireTests, SPSC) {
   int dataSize = 100;
   int howMany = 1000000;
   int expectedSpeed = 50;
   QueueSPSCBenchmark(dataSize, howMany, expectedSpeed);
}

TEST_F(RifleVampireTests, SPSCWithLargeTransfer) {
   int dataSize = 65554;
   int howMany = 200000;
   int expectedSpeed = 1000;
   QueueSPSCBenchmark(dataSize, howMany, expectedSpeed);
}



TEST_F(RifleVampireTests, OneRifleOneVampirePointers) {
   if (geteuid() == 0) {
      std::string location = GetIpcLocation();
      int nVampires = 1;
      int nIOThreads = 1;
      int rifleHWM = 120000;
      int vampireHWM = 30000;
      int dataSize = 100;
      int nShotsPerVampire = 1000000;
      int expectedSpeed = 50;
      OneRifleNVampiresStakeBenchmark(nVampires, nIOThreads, rifleHWM, vampireHWM, location, dataSize, nShotsPerVampire, expectedSpeed, kLongWaitTimeMs);
   }
}

TEST_F(RifleVampireTests, VampireOwnsSocketOneRifleOneVampireIPCSmallSize) {
   if (geteuid() == 0) {
      std::string location = GetIpcLocation();
      int nRifles = 1;
      int nIOThreads = 1;
      int rifleHWM = 30000;
      int vampireHWM = 120000;
      int dataSize = 100;
      int nShotsPerRifle = 100000;
      int expectedSpeed = 50;
      NRiflesOneVampireBenchmark(nRifles, nIOThreads, rifleHWM, vampireHWM,
                                 location, dataSize, nShotsPerRifle, expectedSpeed, kWaitTimeMs);
   }
}

TEST_F(RifleVampireTests, VampireOwnsSocketOneRifleOneVampireIPCSmallSizeZeroCopy) {
   if (geteuid() == 0) {
      std::string location = GetIpcLocation();
      int nRifles = 1;
      int nIOThreads = 1;
      int rifleHWM = 30000;
      int vampireHWM = 120000;
      int dataSize = 100;
      int nShotsPerRifle = 100000;
      int expectedSpeed = 50;
      NRiflesOneVampireBenchmarkZeroCopy(nRifles, nIOThreads, rifleHWM, vampireHWM,
                                         location, dataSize, nShotsPerRifle, expectedSpeed, kWaitTimeMs);
   }
}

TEST_F(RifleVampireTests, RifleOwnsSocketOneRifleTwoVampiresIPCSmallSize) {
   if (geteuid() == 0) {
      std::string location = GetIpcLocation();
      int nVampires = 2;
      int nIOThreads = 1;
      int rifleHWM = 120000;
      int vampireHWM = 30000;
      int dataSize = 100;
      int nShotsPerVampire = 1000000;
      int expectedSpeed = 50;
      OneRifleNVampiresBenchmark(nVampires, nIOThreads, rifleHWM, vampireHWM, location, dataSize, nShotsPerVampire, expectedSpeed, kWaitTimeMs);

   }

}

TEST_F(RifleVampireTests, OneRifleTwoVampiresPointers) {
   if (geteuid() == 0) {
      std::string location = GetIpcLocation();
      int nVampires = 2;
      int nIOThreads = 1;
      int rifleHWM = 120000;
      int vampireHWM = 30000;
      int dataSize = 100;
      int nShotsPerVampire = 1000000;
      int expectedSpeed = 50;
      OneRifleNVampiresStakeBenchmark(nVampires, nIOThreads, rifleHWM, vampireHWM, location, dataSize, nShotsPerVampire, expectedSpeed, kLongWaitTimeMs);

   }

}

TEST_F(RifleVampireTests, VampireOwnsSocketTwoRiflesOneVampireIPCSmallSize) {
   if (geteuid() == 0) {
      std::string location = GetIpcLocation();
      int nRifles = 2;
      int nIOThreads = 1;
      int rifleHWM = 30000;
      int vampireHWM = 120000;
      int dataSize = 100;
      int nShotsPerRifle = 100000;
      int expectedSpeed = 50;
      NRiflesOneVampireBenchmark(nRifles, nIOThreads, rifleHWM, vampireHWM,
                                 location, dataSize, nShotsPerRifle, expectedSpeed, kWaitTimeMs);
   }
}
TEST_F(RifleVampireTests, VampireOwnsSocketTwoRiflesOneVampireIPCSmallSizeZeroCopy) {
   if (geteuid() == 0) {
      std::string location = GetIpcLocation();
      int nRifles = 2;
      int nIOThreads = 1;
      int rifleHWM = 30000;
      int vampireHWM = 120000;
      int dataSize = 100;
      int nShotsPerRifle = 100000;
      int expectedSpeed = 50;
      NRiflesOneVampireBenchmarkZeroCopy(nRifles, nIOThreads, rifleHWM, vampireHWM,
                                         location, dataSize, nShotsPerRifle, expectedSpeed, kWaitTimeMs);
   }
}
TEST_F(RifleVampireTests, RifleOwnsSocketOneRifleThreeVampiresIPCSmallSize) {
   if (geteuid() == 0) {
      std::string location = GetIpcLocation();
      int nVampires = 3;
      int nIOThreads = 1;
      int rifleHWM = 120000;
      int vampireHWM = 30000;
      int dataSize = 100;
      int nShotsPerVampire = 1000000;
      int expectedSpeed = 50;
      OneRifleNVampiresBenchmark(nVampires, nIOThreads, rifleHWM, vampireHWM, location, dataSize, nShotsPerVampire, expectedSpeed, kWaitTimeMs);
   }

}

TEST_F(RifleVampireTests, OneRifleThreeVampiresPointers) {
   if (geteuid() == 0) {
      std::string location = GetIpcLocation();
      int nVampires = 3;
      int nIOThreads = 1;
      int rifleHWM = 120000;
      int vampireHWM = 30000;
      int dataSize = 100;
      int nShotsPerVampire = 1000000;
      int expectedSpeed = 50;
      OneRifleNVampiresStakeBenchmark(nVampires, nIOThreads, rifleHWM, vampireHWM, location, dataSize, nShotsPerVampire, expectedSpeed, kLongWaitTimeMs);
   }

}

TEST_F(RifleVampireTests, RifleOwnsSocketOneRifleFourVampiresIPCSmallSize) {
   if (geteuid() == 0) {
      std::string location = GetIpcLocation();
      int nVampires = 4;
      int nIOThreads = 1;
      int rifleHWM = 120000;
      int vampireHWM = 30000;
      int dataSize = 100;
      int nShotsPerVampire = 1000000;
      int expectedSpeed = 50;
      OneRifleNVampiresBenchmark(nVampires, nIOThreads, rifleHWM, vampireHWM, location, dataSize, nShotsPerVampire, expectedSpeed, kWaitTimeMs);
   }
}

TEST_F(RifleVampireTests, OneRifleFourVampiresPointers) {
   if (geteuid() == 0) {
      std::string location = GetIpcLocation();
      int nVampires = 4;
      int nIOThreads = 1;
      int rifleHWM = 120000;
      int vampireHWM = 30000;
      int dataSize = 100;
      int nShotsPerVampire = 1000000;
      int expectedSpeed = 50;
      OneRifleNVampiresStakeBenchmark(nVampires, nIOThreads, rifleHWM, vampireHWM, location, dataSize, nShotsPerVampire, expectedSpeed, kLongWaitTimeMs);
   }
}

TEST_F(RifleVampireTests, VampireOwnsSocketFiveRiflesOneVampireIPCSmallSize) {
   if (geteuid() == 0) {
      std::string location = GetIpcLocation();
      int nRifles = 5;
      int nIOThreads = 1;
      int rifleHWM = 30000;
      int vampireHWM = 120000;
      int dataSize = 100;
      int nShotsPerRifle = 100000;
      int expectedSpeed = 50;
      NRiflesOneVampireBenchmark(nRifles, nIOThreads, rifleHWM, vampireHWM,
                                 location, dataSize, nShotsPerRifle, expectedSpeed, kWaitTimeMs);
   }
}
TEST_F(RifleVampireTests, VampireOwnsSocketFiveRiflesOneVampireIPCSmallSizeZeroCopy) {
   if (geteuid() == 0) {
      std::string location = GetIpcLocation();
      int nRifles = 5;
      int nIOThreads = 1;
      int rifleHWM = 30000;
      int vampireHWM = 120000;
      int dataSize = 100;
      int nShotsPerRifle = 100000;
      int expectedSpeed = 50;
      NRiflesOneVampireBenchmarkZeroCopy(nRifles, nIOThreads, rifleHWM, vampireHWM,
                                         location, dataSize, nShotsPerRifle, expectedSpeed, kWaitTimeMs);
   }
}
TEST_F(RifleVampireTests, RifleOwnsSocketOneRifleOneVampireTCPSmallSize) {
   if (geteuid() == 0) {
      std::string location = GetTcpLocation();
      int nVampires = 1;
      int nIOThreads = 1;
      int rifleHWM = 120000;
      int vampireHWM = 30000;
      int dataSize = 100;
      int nShotsPerVampire = 3500000;
      int expectedSpeed = 50;
      OneRifleNVampiresBenchmark(nVampires, nIOThreads, rifleHWM, vampireHWM, location, dataSize, nShotsPerVampire, expectedSpeed, kWaitTimeMs);
   }

}

TEST_F(RifleVampireTests, VampireOwnsSocketOneRifleOneVampireTCPSmallSize) {
   if (geteuid() == 0) {
      std::string location = GetTcpLocation();
      int nRifles = 1;
      int nIOThreads = 1;
      int rifleHWM = 30000;
      int vampireHWM = 120000;
      int dataSize = 100;
      int nShotsPerRifle = 350000;
      int expectedSpeed = 50;
      NRiflesOneVampireBenchmark(nRifles, nIOThreads, rifleHWM, vampireHWM,
                                 location, dataSize, nShotsPerRifle, expectedSpeed, kWaitTimeMs);
   }
}
TEST_F(RifleVampireTests, VampireOwnsSocketOneRifleOneVampireTCPSmallSizeZeroCopy) {
   if (geteuid() == 0) {
      std::string location = GetTcpLocation();
      int nRifles = 1;
      int nIOThreads = 1;
      int rifleHWM = 30000;
      int vampireHWM = 120000;
      int dataSize = 100;
      int nShotsPerRifle = 350000;
      int expectedSpeed = 50;
      NRiflesOneVampireBenchmarkZeroCopy(nRifles, nIOThreads, rifleHWM, vampireHWM,
                                         location, dataSize, nShotsPerRifle, expectedSpeed, kWaitTimeMs);
   }
}
TEST_F(RifleVampireTests, RifleOwnsSocketOneRifleTwoVampiresTCPSmallSize) {
   if (geteuid() == 0) {
      std::string location = GetTcpLocation();
      int nVampires = 2;
      int nIOThreads = 1;
      int rifleHWM = 120000;
      int vampireHWM = 30000;
      int dataSize = 100;
      int nShotsPerVampire = 2000000;
      int expectedSpeed = 50;
      OneRifleNVampiresBenchmark(nVampires, nIOThreads, rifleHWM, vampireHWM, location, dataSize, nShotsPerVampire, expectedSpeed, kWaitTimeMs);

   }

}

TEST_F(RifleVampireTests, VampireOwnsSocketTwoRiflesOneVampireTCPSmallSize) {
   if (geteuid() == 0) {
      std::string location = GetTcpLocation();
      int nRifles = 2;
      int nIOThreads = 1;
      int rifleHWM = 30000;
      int vampireHWM = 120000;
      int dataSize = 100;
      int nShotsPerRifle = 350000;
      int expectedSpeed = 50;
      NRiflesOneVampireBenchmark(nRifles, nIOThreads, rifleHWM, vampireHWM,
                                 location, dataSize, nShotsPerRifle, expectedSpeed, kWaitTimeMs);
   }
}
TEST_F(RifleVampireTests, VampireOwnsSocketTwoRiflesOneVampireTCPSmallSizeZeroCopy) {
   if (geteuid() == 0) {
      std::string location = GetTcpLocation();
      int nRifles = 2;
      int nIOThreads = 1;
      int rifleHWM = 30000;
      int vampireHWM = 120000;
      int dataSize = 100;
      int nShotsPerRifle = 350000;
      int expectedSpeed = 50;
      NRiflesOneVampireBenchmarkZeroCopy(nRifles, nIOThreads, rifleHWM, vampireHWM,
                                         location, dataSize, nShotsPerRifle, expectedSpeed, kWaitTimeMs);
   }
}
TEST_F(RifleVampireTests, RifleOwnsSocketOneRifleThreeVampiresTCPSmallSize) {
   if (geteuid() == 0) {
      std::string location = GetTcpLocation();
      int nVampires = 3;
      int nIOThreads = 1;
      int rifleHWM = 120000;
      int vampireHWM = 30000;
      int dataSize = 100;
      int nShotsPerVampire = 1000000;
      int expectedSpeed = 50;
      OneRifleNVampiresBenchmark(nVampires, nIOThreads, rifleHWM, vampireHWM, location, dataSize, nShotsPerVampire, expectedSpeed, kWaitTimeMs);
   }

}

TEST_F(RifleVampireTests, RifleOwnsSocketOneRifleFourVampiresTCPSmallSize) {
   if (geteuid() == 0) {
      std::string location = GetTcpLocation();
      int nVampires = 4;
      int nIOThreads = 1;
      int rifleHWM = 120000;
      int vampireHWM = 30000;
      int dataSize = 100;
      int nShotsPerVampire = 1000000;
      int expectedSpeed = 50;
      OneRifleNVampiresBenchmark(nVampires, nIOThreads, rifleHWM, vampireHWM, location, dataSize, nShotsPerVampire, expectedSpeed, kWaitTimeMs);
   }
}

TEST_F(RifleVampireTests, VampireOwnsSocketFiveRiflesOneVampireTCPSmallSize) {
   if (geteuid() == 0) {
      std::string location = GetTcpLocation();
      int nRifles = 5;
      int nIOThreads = 1;
      int rifleHWM = 30000;
      int vampireHWM = 120000;
      int dataSize = 100;
      int nShotsPerRifle = 350000;
      int expectedSpeed = 50;
      NRiflesOneVampireBenchmark(nRifles, nIOThreads, rifleHWM, vampireHWM,
                                 location, dataSize, nShotsPerRifle, expectedSpeed, kWaitTimeMs);
   }
}
TEST_F(RifleVampireTests, VampireOwnsSocketFiveRiflesOneVampireTCPSmallSizeZeroCopy) {
   if (geteuid() == 0) {
      std::string location = GetTcpLocation();
      int nRifles = 5;
      int nIOThreads = 1;
      int rifleHWM = 30000;
      int vampireHWM = 120000;
      int dataSize = 100;
      int nShotsPerRifle = 350000;
      int expectedSpeed = 50;
      NRiflesOneVampireBenchmarkZeroCopy(nRifles, nIOThreads, rifleHWM, vampireHWM,
                                         location, dataSize, nShotsPerRifle, expectedSpeed, kWaitTimeMs);
   }
}
TEST_F(RifleVampireTests, RifleOwnsSocketOneRifleOneVampireIPCAvgSize) {
   if (geteuid() == 0) {
      std::string location = GetIpcLocation();
      int nVampires = 1;
      int nIOThreads = 1;
      int rifleHWM = 120000;
      int vampireHWM = 30000;
      int dataSize = 350;
      int nShotsPerVampire = 1000000;
      int expectedSpeed = 100;
      OneRifleNVampiresBenchmark(nVampires, nIOThreads, rifleHWM, vampireHWM, location, dataSize, nShotsPerVampire, expectedSpeed, kWaitTimeMs);
   }

}

TEST_F(RifleVampireTests, VampireOwnsSocketOneRifleOneVampireIPCAvgSize) {
   if (geteuid() == 0) {
      std::string location = GetIpcLocation();
      int nRifles = 1;
      int nIOThreads = 1;
      int rifleHWM = 30000;
      int vampireHWM = 120000;
      int dataSize = 350;
      int nShotsPerRifle = 100000;
      int expectedSpeed = 100;
      NRiflesOneVampireBenchmark(nRifles, nIOThreads, rifleHWM, vampireHWM,
                                 location, dataSize, nShotsPerRifle, expectedSpeed, kWaitTimeMs);
   }
}
TEST_F(RifleVampireTests, VampireOwnsSocketOneRifleOneVampireIPCAvgSizeZeroCopy) {
   if (geteuid() == 0) {
      std::string location = GetIpcLocation();
      int nRifles = 1;
      int nIOThreads = 1;
      int rifleHWM = 30000;
      int vampireHWM = 120000;
      int dataSize = 350;
      int nShotsPerRifle = 100000;
      int expectedSpeed = 100;
      NRiflesOneVampireBenchmarkZeroCopy(nRifles, nIOThreads, rifleHWM, vampireHWM,
                                         location, dataSize, nShotsPerRifle, expectedSpeed, kWaitTimeMs);
   }
}
TEST_F(RifleVampireTests, RifleOwnsSocketOneRifleTwoVampiresIPCAvgSize) {
   if (geteuid() == 0) {
      std::string location = GetIpcLocation();
      int nVampires = 2;
      int nIOThreads = 1;
      int rifleHWM = 120000;
      int vampireHWM = 30000;
      int dataSize = 350;
      int nShotsPerVampire = 1000000;
      int expectedSpeed = 100;
      OneRifleNVampiresBenchmark(nVampires, nIOThreads, rifleHWM, vampireHWM, location, dataSize, nShotsPerVampire, expectedSpeed, kWaitTimeMs);

   }

}

TEST_F(RifleVampireTests, VampireOwnsSocketTwoRiflesOneVampireIPCAvgSize) {
   if (geteuid() == 0) {
      std::string location = GetIpcLocation();
      int nRifles = 2;
      int nIOThreads = 1;
      int rifleHWM = 30000;
      int vampireHWM = 120000;
      int dataSize = 350;
      int nShotsPerRifle = 100000;
      int expectedSpeed = 100;
      NRiflesOneVampireBenchmark(nRifles, nIOThreads, rifleHWM, vampireHWM,
                                 location, dataSize, nShotsPerRifle, expectedSpeed, kWaitTimeMs);
   }
}
TEST_F(RifleVampireTests, VampireOwnsSocketTwoRiflesOneVampireIPCAvgSizeZeroCopy) {
   if (geteuid() == 0) {
      std::string location = GetIpcLocation();
      int nRifles = 2;
      int nIOThreads = 1;
      int rifleHWM = 30000;
      int vampireHWM = 120000;
      int dataSize = 350;
      int nShotsPerRifle = 100000;
      int expectedSpeed = 100;
      NRiflesOneVampireBenchmarkZeroCopy(nRifles, nIOThreads, rifleHWM, vampireHWM,
                                         location, dataSize, nShotsPerRifle, expectedSpeed, kWaitTimeMs);
   }
}
TEST_F(RifleVampireTests, RifleOwnsSocketOneRifleThreeVampiresIPCAvgSize) {
   if (geteuid() == 0) {
      std::string location = GetIpcLocation();
      int nVampires = 3;
      int nIOThreads = 1;
      int rifleHWM = 120000;
      int vampireHWM = 30000;
      int dataSize = 350;
      int nShotsPerVampire = 1000000;
      int expectedSpeed = 100;
      OneRifleNVampiresBenchmark(nVampires, nIOThreads, rifleHWM, vampireHWM, location, dataSize, nShotsPerVampire, expectedSpeed, kWaitTimeMs);
   }

}

TEST_F(RifleVampireTests, RifleOwnsSocketOneRifleFourVampiresIPCAvgSize) {
   if (geteuid() == 0) {
      std::string location = GetIpcLocation();
      int nVampires = 4;
      int nIOThreads = 1;
      int rifleHWM = 120000;
      int vampireHWM = 30000;
      int dataSize = 350;
      int nShotsPerVampire = 1000000;
      int expectedSpeed = 100;
      OneRifleNVampiresBenchmark(nVampires, nIOThreads, rifleHWM, vampireHWM, location, dataSize, nShotsPerVampire, expectedSpeed, kWaitTimeMs);
   }
}

 // kjellkod

TEST_F(RifleVampireTests, VampireOwnsSocketFiveRiflesOneVampireIPCAvgSize) {
   if (geteuid() == 0) {
      std::string location = GetIpcLocation();
      int nRifles = 5;
      int nIOThreads = 1;
      int rifleHWM = 30000;
      int vampireHWM = 120000;
      int dataSize = 350;
      int nShotsPerRifle = 100000;
      int expectedSpeed = 100;
      NRiflesOneVampireBenchmark(nRifles, nIOThreads, rifleHWM, vampireHWM,
                                 location, dataSize, nShotsPerRifle, expectedSpeed, kWaitTimeMs);
   }
}
TEST_F(RifleVampireTests, VampireOwnsSocketFiveRiflesOneVampireIPCAvgSizeZeroCopy) {
   if (geteuid() == 0) {
      std::string location = GetIpcLocation();
      int nRifles = 5;
      int nIOThreads = 1;
      int rifleHWM = 30000;
      int vampireHWM = 120000;
      int dataSize = 350;
      int nShotsPerRifle = 100000;
      int expectedSpeed = 100;
      NRiflesOneVampireBenchmarkZeroCopy(nRifles, nIOThreads, rifleHWM, vampireHWM,
                                         location, dataSize, nShotsPerRifle, expectedSpeed, kWaitTimeMs);
   }
}
TEST_F(RifleVampireTests, OneRifleOneVampirePointerAvgSize) {
   if (geteuid() == 0) {
      std::string location = GetIpcLocation();
      int nVampires = 1;
      int nIOThreads = 1;
      int rifleHWM = 120000;
      int vampireHWM = 30000;
      int dataSize = 350;
      int nShotsPerVampire = 1000000;
      int expectedSpeed = 100;
      OneRifleNVampiresStakeBenchmark(nVampires, nIOThreads, rifleHWM, vampireHWM, location, dataSize, nShotsPerVampire, expectedSpeed, kLongWaitTimeMs);
   }

}

TEST_F(RifleVampireTests, OneRifleTwoVampiresPointerAvgSize) {
   if (geteuid() == 0) {
      std::string location = GetIpcLocation();
      int nVampires = 2;
      int nIOThreads = 1;
      int rifleHWM = 120000;
      int vampireHWM = 30000;
      int dataSize = 350;
      int nShotsPerVampire = 1000000;
      int expectedSpeed = 100;
      OneRifleNVampiresStakeBenchmark(nVampires, nIOThreads, rifleHWM, vampireHWM, location, dataSize, nShotsPerVampire, expectedSpeed, kLongWaitTimeMs);

   }

}

TEST_F(RifleVampireTests, OneRifleThreeVampiresPointerAvgSize) {
   if (geteuid() == 0) {
      std::string location = GetIpcLocation();
      int nVampires = 3;
      int nIOThreads = 1;
      int rifleHWM = 120000;
      int vampireHWM = 30000;
      int dataSize = 350;
      int nShotsPerVampire = 1000000;
      int expectedSpeed = 100;
      OneRifleNVampiresStakeBenchmark(nVampires, nIOThreads, rifleHWM, vampireHWM, location, dataSize, nShotsPerVampire, expectedSpeed, kLongWaitTimeMs);
   }

}

TEST_F(RifleVampireTests, OneRifleFourVampiresPointerAvgSize) {
   if (geteuid() == 0) {
      std::string location = GetIpcLocation();
      int nVampires = 4;
      int nIOThreads = 1;
      int rifleHWM = 120000;
      int vampireHWM = 30000;
      int dataSize = 350;
      int nShotsPerVampire = 1000000;
      int expectedSpeed = 100;
      OneRifleNVampiresStakeBenchmark(nVampires, nIOThreads, rifleHWM, vampireHWM, location, dataSize, nShotsPerVampire, expectedSpeed, kLongWaitTimeMs);
   }
}

TEST_F(RifleVampireTests, RifleOwnsSocketOneRifleOneVampireTCPAvgSize) {
   if (geteuid() == 0) {
      std::string location = GetTcpLocation();
      int nVampires = 1;
      int nIOThreads = 1;
      int rifleHWM = 120000;
      int vampireHWM = 30000;
      int dataSize = 350;
      int nShotsPerVampire = 3500000;
      int expectedSpeed = 100;
      OneRifleNVampiresBenchmark(nVampires, nIOThreads, rifleHWM, vampireHWM, location, dataSize, nShotsPerVampire, expectedSpeed, kWaitTimeMs);
   }

}

TEST_F(RifleVampireTests, VampireOwnsSocketOneRifleOneVampireTCPAvgSize) {
   if (geteuid() == 0) {
      std::string location = GetTcpLocation();
      int nRifles = 1;
      int nIOThreads = 1;
      int rifleHWM = 30000;
      int vampireHWM = 120000;
      int dataSize = 350;
      int nShotsPerRifle = 350000;
      int expectedSpeed = 100;
      NRiflesOneVampireBenchmark(nRifles, nIOThreads, rifleHWM, vampireHWM,
                                 location, dataSize, nShotsPerRifle, expectedSpeed, kWaitTimeMs);
   }
}
TEST_F(RifleVampireTests, VampireOwnsSocketOneRifleOneVampireTCPAvgSizeZeroCopy) {
   if (geteuid() == 0) {
      std::string location = GetTcpLocation();
      int nRifles = 1;
      int nIOThreads = 1;
      int rifleHWM = 30000;
      int vampireHWM = 120000;
      int dataSize = 350;
      int nShotsPerRifle = 350000;
      int expectedSpeed = 100;
      NRiflesOneVampireBenchmarkZeroCopy(nRifles, nIOThreads, rifleHWM, vampireHWM,
                                         location, dataSize, nShotsPerRifle, expectedSpeed, kWaitTimeMs);
   }
}
TEST_F(RifleVampireTests, RifleOwnsSocketOneRifleTwoVampiresTCPAvgSize) {
   if (geteuid() == 0) {
      std::string location = GetTcpLocation();
      int nVampires = 2;
      int nIOThreads = 1;
      int rifleHWM = 120000;
      int vampireHWM = 30000;
      int dataSize = 350;
      int nShotsPerVampire = 2000000;
      int expectedSpeed = 100;
      OneRifleNVampiresBenchmark(nVampires, nIOThreads, rifleHWM, vampireHWM, location, dataSize, nShotsPerVampire, expectedSpeed, kWaitTimeMs);

   }

}

TEST_F(RifleVampireTests, VampireOwnsSocketTwoRiflesOneVampireTCPAvgSize) {
   if (geteuid() == 0) {
      std::string location = GetTcpLocation();
      int nRifles = 2;
      int nIOThreads = 1;
      int rifleHWM = 30000;
      int vampireHWM = 120000;
      int dataSize = 350;
      int nShotsPerRifle = 350000;
      int expectedSpeed = 100;
      NRiflesOneVampireBenchmark(nRifles, nIOThreads, rifleHWM, vampireHWM,
                                 location, dataSize, nShotsPerRifle, expectedSpeed, kWaitTimeMs);
   }
}
TEST_F(RifleVampireTests, VampireOwnsSocketTwoRiflesOneVampireTCPAvgSizeZeroCopy) {
   if (geteuid() == 0) {
      std::string location = GetTcpLocation();
      int nRifles = 2;
      int nIOThreads = 1;
      int rifleHWM = 30000;
      int vampireHWM = 120000;
      int dataSize = 350;
      int nShotsPerRifle = 350000;
      int expectedSpeed = 100;
      NRiflesOneVampireBenchmarkZeroCopy(nRifles, nIOThreads, rifleHWM, vampireHWM,
                                         location, dataSize, nShotsPerRifle, expectedSpeed, kWaitTimeMs);
   }
}
TEST_F(RifleVampireTests, RifleOwnsSocketOneRifleThreeVampiresTCPAvgSize) {
   if (geteuid() == 0) {
      std::string location = GetTcpLocation();
      int nVampires = 3;
      int nIOThreads = 1;
      int rifleHWM = 120000;
      int vampireHWM = 30000;
      int dataSize = 350;
      int nShotsPerVampire = 1000000;
      int expectedSpeed = 100;
      OneRifleNVampiresBenchmark(nVampires, nIOThreads, rifleHWM, vampireHWM, location, dataSize, nShotsPerVampire, expectedSpeed, kWaitTimeMs);
   }

}

TEST_F(RifleVampireTests, RifleOwnsSocketOneRifleFourVampiresTCPAvgSize) {
   if (geteuid() == 0) {
      std::string location = GetTcpLocation();
      int nVampires = 4;
      int nIOThreads = 1;
      int rifleHWM = 120000;
      int vampireHWM = 30000;
      int dataSize = 350;
      int nShotsPerVampire = 1000000;
      int expectedSpeed = 100;
      OneRifleNVampiresBenchmark(nVampires, nIOThreads, rifleHWM, vampireHWM, location, dataSize, nShotsPerVampire, expectedSpeed, kWaitTimeMs);
   }
}

TEST_F(RifleVampireTests, VampireOwnsSocketFiveRiflesOneVampireTCPAvgSize) {
   if (geteuid() == 0) {
      std::string location = GetTcpLocation();
      int nRifles = 5;
      int nIOThreads = 1;
      int rifleHWM = 30000;
      int vampireHWM = 120000;
      int dataSize = 350;
      int nShotsPerRifle = 350000;
      int expectedSpeed = 100;
      NRiflesOneVampireBenchmark(nRifles, nIOThreads, rifleHWM, vampireHWM,
                                 location, dataSize, nShotsPerRifle, expectedSpeed, kWaitTimeMs);
   }
}
TEST_F(RifleVampireTests, VampireOwnsSocketFiveRiflesOneVampireTCPAvgSizeZeroCopy) {
   if (geteuid() == 0) {
      std::string location = GetTcpLocation();
      int nRifles = 5;
      int nIOThreads = 1;
      int rifleHWM = 30000;
      int vampireHWM = 120000;
      int dataSize = 350;
      int nShotsPerRifle = 350000;
      int expectedSpeed = 100;
      NRiflesOneVampireBenchmarkZeroCopy(nRifles, nIOThreads, rifleHWM, vampireHWM,
                                         location, dataSize, nShotsPerRifle, expectedSpeed, kWaitTimeMs);
   }
}


TEST_F(RifleVampireTests, VampireOwnsSocketOneRifleOneVampireIPCLargeSize) {
   if (geteuid() == 0) {
      std::string location = GetIpcLocation();
      int nRifles = 1;
      int nIOThreads = 1;
      int rifleHWM = 500;
      int vampireHWM = 1000;
      int dataSize = 65554;
      int nShotsPerRifle = 10000;
      int expectedSpeed = 1000;
      NRiflesOneVampireBenchmark(nRifles, nIOThreads, rifleHWM, vampireHWM,
                                 location, dataSize, nShotsPerRifle, expectedSpeed, kWaitTimeMs);
   }
}
TEST_F(RifleVampireTests, VampireOwnsSocketOneRifleOneVampireIPCLargeSizeZeroCopy) {
   if (geteuid() == 0) {
      std::string location = GetIpcLocation();
      int nRifles = 1;
      int nIOThreads = 1;
      int rifleHWM = 500;
      int vampireHWM = 1000;
      int dataSize = 65554;
      int nShotsPerRifle = 10000;
      int expectedSpeed = 1000;
      NRiflesOneVampireBenchmarkZeroCopy(nRifles, nIOThreads, rifleHWM, vampireHWM,
                                         location, dataSize, nShotsPerRifle, expectedSpeed, kWaitTimeMs);
   }
}
TEST_F(RifleVampireTests, RifleOwnsSocketOneRifleTwoVampiresIPCLargeSize) {
   if (geteuid() == 0) {
      std::string location = GetIpcLocation();
      int nVampires = 2;
      int nIOThreads = 1;
      int rifleHWM = 1000;
      int vampireHWM = 500;
      int dataSize = 65554;
      int nShotsPerVampire = 200000;
      int expectedSpeed = 1000;
      OneRifleNVampiresBenchmark(nVampires, nIOThreads, rifleHWM, vampireHWM, location, dataSize, nShotsPerVampire, expectedSpeed, kWaitTimeMs);

   }

}

TEST_F(RifleVampireTests, VampireOwnsSocketTwoRiflesOneVampireIPCLargeSize) {
   if (geteuid() == 0) {
      std::string location = GetIpcLocation();
      int nRifles = 2;
      int nIOThreads = 1;
      int rifleHWM = 500;
      int vampireHWM = 1000;
      int dataSize = 65554;
      int nShotsPerRifle = 10000;
      int expectedSpeed = 1000;
      NRiflesOneVampireBenchmark(nRifles, nIOThreads, rifleHWM, vampireHWM,
                                 location, dataSize, nShotsPerRifle, expectedSpeed, kWaitTimeMs);
   }
}
TEST_F(RifleVampireTests, VampireOwnsSocketTwoRiflesOneVampireIPCLargeSizeZeroCopy) {
   if (geteuid() == 0) {
      std::string location = GetIpcLocation();
      int nRifles = 2;
      int nIOThreads = 1;
      int rifleHWM = 500;
      int vampireHWM = 1000;
      int dataSize = 65554;
      int nShotsPerRifle = 10000;
      int expectedSpeed = 1000;
      NRiflesOneVampireBenchmarkZeroCopy(nRifles, nIOThreads, rifleHWM, vampireHWM,
                                         location, dataSize, nShotsPerRifle, expectedSpeed, kWaitTimeMs);
   }
}
TEST_F(RifleVampireTests, RifleOwnsSocketOneRifleThreeVampiresIPCLargeSize) {
   if (geteuid() == 0) {
      std::string location = GetIpcLocation();
      int nVampires = 3;
      int nIOThreads = 1;
      int rifleHWM = 1000;
      int vampireHWM = 500;
      int dataSize = 65554;
      int nShotsPerVampire = 200000;
      int expectedSpeed = 1000;
      OneRifleNVampiresBenchmark(nVampires, nIOThreads, rifleHWM, vampireHWM, location, dataSize, nShotsPerVampire, expectedSpeed, kWaitTimeMs);
   }

}

TEST_F(RifleVampireTests, RifleOwnsSocketOneRifleFourVampiresIPCLargeSize) {
   if (geteuid() == 0) {
      std::string location = GetIpcLocation();
      int nVampires = 4;
      int nIOThreads = 1;
      int rifleHWM = 1000;
      int vampireHWM = 500;
      int dataSize = 65554;
      int nShotsPerVampire = 200000;
      int expectedSpeed = 1000;
      OneRifleNVampiresBenchmark(nVampires, nIOThreads, rifleHWM, vampireHWM, location, dataSize, nShotsPerVampire, expectedSpeed, kWaitTimeMs);
   }
}

TEST_F(RifleVampireTests, VampireOwnsSocketFiveRiflesOneVampireIPCLargeSize) {
   if (geteuid() == 0) {
      std::string location = GetIpcLocation();
      int nRifles = 5;
      int nIOThreads = 1;
      int rifleHWM = 500;
      int vampireHWM = 1000;
      int dataSize = 65554;
      int nShotsPerRifle = 10000;
      int expectedSpeed = 1000;
      NRiflesOneVampireBenchmark(nRifles, nIOThreads, rifleHWM, vampireHWM,
                                 location, dataSize, nShotsPerRifle, expectedSpeed, kWaitTimeMs);
   }
}
TEST_F(RifleVampireTests, VampireOwnsSocketFiveRiflesOneVampireIPCLargeSizeZeroCopy) {
   if (geteuid() == 0) {
      std::string location = GetIpcLocation();
      int nRifles = 5;
      int nIOThreads = 1;
      int rifleHWM = 500;
      int vampireHWM = 1000;
      int dataSize = 65554;
      int nShotsPerRifle = 10000;
      int expectedSpeed = 1000;
      NRiflesOneVampireBenchmarkZeroCopy(nRifles, nIOThreads, rifleHWM, vampireHWM,
                                         location, dataSize, nShotsPerRifle, expectedSpeed, kWaitTimeMs);
   }
}
TEST_F(RifleVampireTests, OneRifleOneVampirePointersLarge) {
   if (geteuid() == 0) {
      std::string location = GetIpcLocation();
      int nVampires = 1;
      int nIOThreads = 1;
      int rifleHWM = 1000;
      int vampireHWM = 500;
      int dataSize = 65554;
      int nShotsPerVampire = 200000;
      int expectedSpeed = 1000;
      OneRifleNVampiresStakeBenchmark(nVampires, nIOThreads, rifleHWM, vampireHWM, location, dataSize, nShotsPerVampire, expectedSpeed, kLongWaitTimeMs);
   }

}

TEST_F(RifleVampireTests, OneRifleTwoVampiresPointersLarge) {
   if (geteuid() == 0) {
      std::string location = GetIpcLocation();
      int nVampires = 2;
      int nIOThreads = 1;
      int rifleHWM = 1500;
      int vampireHWM = 500;
      int dataSize = 65554;
      int nShotsPerVampire = 200000;
      int expectedSpeed = 1000;
      OneRifleNVampiresStakeBenchmark(nVampires, nIOThreads, rifleHWM, vampireHWM, location, dataSize, nShotsPerVampire, expectedSpeed, kLongWaitTimeMs);

   }

}

TEST_F(RifleVampireTests, OneRifleThreeVampiresPointersLarge) {
   if (geteuid() == 0) {
      std::string location = GetIpcLocation();
      int nVampires = 3;
      int nIOThreads = 1;
      int rifleHWM = 1500;
      int vampireHWM = 500;
      int dataSize = 65554;
      int nShotsPerVampire = 200000;
      int expectedSpeed = 1000;
      OneRifleNVampiresStakeBenchmark(nVampires, nIOThreads, rifleHWM, vampireHWM, location, dataSize, nShotsPerVampire, expectedSpeed, kLongWaitTimeMs);
   }

}

TEST_F(RifleVampireTests, OneRifleFourVampiresPointersLarge) {
   if (geteuid() == 0) {
      std::string location = GetIpcLocation();
      int nVampires = 4;
      int nIOThreads = 1;
      int rifleHWM = 1500;
      int vampireHWM = 500;
      int dataSize = 65554;
      int nShotsPerVampire = 200000;
      int expectedSpeed = 1000;
      OneRifleNVampiresStakeBenchmark(nVampires, nIOThreads, rifleHWM, vampireHWM, location, dataSize, nShotsPerVampire, expectedSpeed, kLongWaitTimeMs);
   }
}

TEST_F(RifleVampireTests, RifleOwnsSocketOneRifleOneVampireTCPLargeSize) {
   if (geteuid() == 0) {
      std::string location = GetTcpLocation();
      int nVampires = 1;
      int nIOThreads = 1;
      int rifleHWM = 1500;
      int vampireHWM = 500;
      int dataSize = 65554;
      int nShotsPerVampire = 200000;
      int expectedSpeed = 1000;
      OneRifleNVampiresBenchmark(nVampires, nIOThreads, rifleHWM, vampireHWM, location, dataSize, nShotsPerVampire, expectedSpeed, kWaitTimeMs);
   }

}

TEST_F(RifleVampireTests, VampireOwnsSocketOneRifleOneVampireTCPLargeSize) {
   if (geteuid() == 0) {
      std::string location = GetTcpLocation();
      int nRifles = 1;
      int nIOThreads = 1;
      int rifleHWM = 500;
      int vampireHWM = 1000;
      int dataSize = 65554;
      int nShotsPerRifle = 10000;
      int expectedSpeed = 1000;
      NRiflesOneVampireBenchmark(nRifles, nIOThreads, rifleHWM, vampireHWM,
                                 location, dataSize, nShotsPerRifle, expectedSpeed, kWaitTimeMs);
   }
}
TEST_F(RifleVampireTests, VampireOwnsSocketOneRifleOneVampireTCPLargeSizeZeroCopy) {
   if (geteuid() == 0) {
      std::string location = GetTcpLocation();
      int nRifles = 1;
      int nIOThreads = 1;
      int rifleHWM = 500;
      int vampireHWM = 1000;
      int dataSize = 65554;
      int nShotsPerRifle = 10000;
      int expectedSpeed = 1000;
      NRiflesOneVampireBenchmarkZeroCopy(nRifles, nIOThreads, rifleHWM, vampireHWM,
                                         location, dataSize, nShotsPerRifle, expectedSpeed, kWaitTimeMs);
   }
}
TEST_F(RifleVampireTests, RifleOwnsSocketOneRifleTwoVampiresTCPLargeSize) {
   if (geteuid() == 0) {
      std::string location = GetTcpLocation();
      int nVampires = 2;
      int nIOThreads = 1;
      int rifleHWM = 1000;
      int vampireHWM = 500;
      int dataSize = 65554;
      int nShotsPerVampire = 200000;
      int expectedSpeed = 1000;
      OneRifleNVampiresBenchmark(nVampires, nIOThreads, rifleHWM, vampireHWM, location, dataSize, nShotsPerVampire, expectedSpeed, kWaitTimeMs);

   }

}

TEST_F(RifleVampireTests, VampireOwnsSocketTwoRiflesOneVampireTCPLargeSize) {
   if (geteuid() == 0) {
      std::string location = GetTcpLocation();
      int nRifles = 2;
      int nIOThreads = 1;
      int rifleHWM = 500;
      int vampireHWM = 1000;
      int dataSize = 65554;
      int nShotsPerRifle = 10000;
      int expectedSpeed = 1000;
      NRiflesOneVampireBenchmark(nRifles, nIOThreads, rifleHWM, vampireHWM,
                                 location, dataSize, nShotsPerRifle, expectedSpeed, kWaitTimeMs);
   }
}
TEST_F(RifleVampireTests, VampireOwnsSocketTwoRiflesOneVampireTCPLargeSizeZeroCopy) {
   if (geteuid() == 0) {
      std::string location = GetTcpLocation();
      int nRifles = 2;
      int nIOThreads = 1;
      int rifleHWM = 500;
      int vampireHWM = 1000;
      int dataSize = 65554;
      int nShotsPerRifle = 10000;
      int expectedSpeed = 1000;
      NRiflesOneVampireBenchmarkZeroCopy(nRifles, nIOThreads, rifleHWM, vampireHWM,
                                         location, dataSize, nShotsPerRifle, expectedSpeed, kWaitTimeMs);
   }
}
TEST_F(RifleVampireTests, RifleOwnsSocketOneRifleThreeVampiresTCPLargeSize) {
   if (geteuid() == 0) {
      std::string location = GetTcpLocation();
      int nVampires = 3;
      int nIOThreads = 1;
      int rifleHWM = 1000;
      int vampireHWM = 500;
      int dataSize = 65554;
      int nShotsPerVampire = 200000;
      int expectedSpeed = 1000;
      OneRifleNVampiresBenchmark(nVampires, nIOThreads, rifleHWM, vampireHWM, location, dataSize, nShotsPerVampire, expectedSpeed, kWaitTimeMs);
   }

}

TEST_F(RifleVampireTests, RifleOwnsSocketOneRifleFourVampiresTCPLargeSize) {
   if (geteuid() == 0) {
      std::string location = GetTcpLocation();
      int nVampires = 4;
      int nIOThreads = 1;
      int rifleHWM = 1000;
      int vampireHWM = 500;
      int dataSize = 65554;
      int nShotsPerVampire = 200000;
      int expectedSpeed = 1000;
      OneRifleNVampiresBenchmark(nVampires, nIOThreads, rifleHWM, vampireHWM, location, dataSize, nShotsPerVampire, expectedSpeed, kWaitTimeMs);
   }
}


TEST_F(RifleVampireTests, MPMC_OneToFour) {
   int dataSize = 65554;
   int howMany = 200000;
   int expectedSpeed = 1000;
   int numSenders = 1;
   int numReceivers = 4;
   QueueMPMCBenchmark(numSenders, numReceivers, dataSize, howMany, expectedSpeed);
}

TEST_F(RifleVampireTests, SPMC_OneToFour_OneMinute) {
   int dataSize = 65554;
   int howMany = std::numeric_limits<int>::max();
   int expectedSpeed = 1000;
   int numSenders = 1;
   int numReceivers = 4;
   QueueMPMCBenchmark_1Minute(numSenders, numReceivers, dataSize, howMany, expectedSpeed);
}


TEST_F(RifleVampireTests, MPMC_FourToFour) {
   int dataSize = 65554;
   int howMany = 200000;
   int expectedSpeed = 1000;
   int numSenders = 4;
   int numReceivers = 4;
   QueueMPMCBenchmark(numSenders, numReceivers, dataSize, howMany, expectedSpeed);
}


TEST_F(RifleVampireTests, VampireOwnsSocketFiveRiflesOneVampireTCPLargeSize) {
   if (geteuid() == 0) {
      std::string location = GetTcpLocation();
      int nRifles = 5;
      int nIOThreads = 1;
      int rifleHWM = 500;
      int vampireHWM = 1000;
      int dataSize = 65554;
      int nShotsPerRifle = 10000;
      int expectedSpeed = 1000;
      NRiflesOneVampireBenchmark(nRifles, nIOThreads, rifleHWM, vampireHWM,
                                 location, dataSize, nShotsPerRifle, expectedSpeed, kWaitTimeMs);
   }
}
TEST_F(RifleVampireTests, VampireOwnsSocketFiveRiflesOneVampireTCPLargeSizeZeroCopy) {
   if (geteuid() == 0) {
      std::string location = GetTcpLocation();
      int nRifles = 5;
      int nIOThreads = 1;
      int rifleHWM = 500;
      int vampireHWM = 1000;
      int dataSize = 65554;
      int nShotsPerRifle = 10000;
      int expectedSpeed = 1000;
      NRiflesOneVampireBenchmarkZeroCopy(nRifles, nIOThreads, rifleHWM, vampireHWM,
                                         location, dataSize, nShotsPerRifle, expectedSpeed, kWaitTimeMs);
   }
}

#endif // 0 --- Broken performance test

TEST_F(RifleVampireTests, StopPreparingAlreadyAndJustGo) {
   std::string location = GetIpcLocation();
   Vampire vampire(location);
   Rifle rifle(location);
   //preparing twice to make sure it doesn't break us.
   rifle.Aim();
   rifle.Aim();
   vampire.PrepareToBeShot();
   vampire.PrepareToBeShot();
   std::string bullet;
   EXPECT_FALSE(vampire.GetShot(bullet, 1));
   std::string msg("woo");
   EXPECT_TRUE(rifle.Fire(msg));
   EXPECT_TRUE(vampire.GetShot(bullet, 1));
}

TEST_F(RifleVampireTests, NoTargetToShoot) {
   std::string location = GetIpcLocation();
   Rifle rifle(location);
   rifle.SetOwnSocket(true);
   rifle.Aim();
   std::string msg("woo");
   EXPECT_FALSE(rifle.Fire(msg, 1));
   EXPECT_FALSE(rifle.FireStake(&msg, 1));
   std::vector<std::pair<void*, unsigned int> > bundle;
   bundle.push_back(make_pair(&msg, 0));
   EXPECT_FALSE(rifle.FireStakes(bundle, 1));
}

TEST_F(RifleVampireTests, ThisWillNeverWorkStopTrying) {
   std::string location = "blahblahblah";
   Vampire vampire(location);
   Rifle rifle(location);
   vampire.SetOwnSocket(false);
   rifle.SetOwnSocket(true);

   EXPECT_FALSE(rifle.Aim());
   EXPECT_FALSE(rifle.Aim());
   EXPECT_FALSE(vampire.PrepareToBeShot());
   EXPECT_FALSE(vampire.PrepareToBeShot());
   vampire.SetOwnSocket(true);
   rifle.SetOwnSocket(false);
   EXPECT_FALSE(rifle.Aim());
   EXPECT_FALSE(rifle.Aim());
   EXPECT_FALSE(vampire.PrepareToBeShot());
   EXPECT_FALSE(vampire.PrepareToBeShot());
}

TEST_F(RifleVampireTests, StopPreparingAlreadyAndJustGoPointer) {
   std::string location = GetIpcLocation();
   Vampire vampire(location);
   Rifle rifle(location);
   //preparing twice to make sure it doesn't break us.
   rifle.Aim();
   rifle.Aim();
   vampire.PrepareToBeShot();
   vampire.PrepareToBeShot();
   void* bullet;
   EXPECT_FALSE(vampire.GetStake(bullet, 1));
   std::string msg("woo");
   EXPECT_TRUE(rifle.FireStake(&msg, 1));
   EXPECT_TRUE(vampire.GetStake(bullet, 1));
   unsigned int hash(5);
   EXPECT_EQ(&msg, bullet);
   std::vector<std::pair<void*, unsigned int> > bundle, gotBundle;
   bundle.push_back(make_pair(&msg, hash));
   EXPECT_TRUE(rifle.FireStakes(bundle));
   EXPECT_TRUE(vampire.GetStakes(gotBundle));
   ASSERT_EQ(bundle.size(), gotBundle.size());
   auto it = bundle.begin();
   auto jt = gotBundle.begin();
   for (; it != bundle.end() && it != gotBundle.end() && !zctx_interrupted; it++, jt++) {
      EXPECT_EQ(it->first, jt->first);
      EXPECT_EQ(it->second, jt->second);
   }

}

TEST_F(RifleVampireTests, ShootBlank) {
   std::string location = GetIpcLocation();
   Vampire vampire(location);
   Rifle rifle(location);
   rifle.Aim();
   vampire.PrepareToBeShot();
   std::string msg;
   //should fail to shoot a blank, the rifle doesn't allow it.
   EXPECT_FALSE(rifle.Fire(msg, 100));
   std::string bullet;
   EXPECT_FALSE(vampire.GetShot(bullet, 1));


}

TEST_F(RifleVampireTests, ShootNULL) {
   std::string location = GetIpcLocation();
   Vampire vampire(location);
   Rifle rifle(location);
   rifle.Aim();
   vampire.PrepareToBeShot();
   //should fail to shoot a blank, the rifle doesn't allow it.
   EXPECT_FALSE(rifle.FireStake(NULL, 100));
   void* bullet;
   EXPECT_FALSE(vampire.GetStake(bullet, 1));
}

TEST_F(RifleVampireTests, ShootEmptyBundle) {
   std::string location = GetIpcLocation();
   Vampire vampire(location);
   Rifle rifle(location);
   rifle.Aim();
   vampire.PrepareToBeShot();
   std::vector<std::pair<void*, unsigned int> > bundle, gotBundle;
   EXPECT_FALSE(rifle.FireStakes(bundle));
   EXPECT_FALSE(vampire.GetStakes(gotBundle));
}

TEST_F(RifleVampireTests, ShotInTheDark) {
   Rifle rifle(GetIpcLocation());
   rifle.Aim();
   std::string msg("Fire!");
   //should fail without someone to shoot.
   EXPECT_FALSE(rifle.Fire(msg, 10));
}

TEST_F(RifleVampireTests, StakeInTheDark) {
   Rifle rifle(GetIpcLocation());
   rifle.Aim();
   std::string msg("Fire!");
   //should fail without someone to shoot.
   EXPECT_FALSE(rifle.FireStake(&msg, 1));
}

TEST_F(RifleVampireTests, StakesInTheDark) {
   Rifle rifle(GetIpcLocation());
   rifle.Aim();
   std::string msg("Fire!");
   std::vector<std::pair<void*, unsigned int> > bundle;
   bundle.push_back(make_pair(&msg, 0));
   EXPECT_FALSE(rifle.FireStakes(bundle, 1));
}

TEST_F(RifleVampireTests, LoadRifleAndThrowAway) {
   Rifle* rifle = new Rifle(GetIpcLocation());
   rifle->Aim();
   std::string msg("abc");
   EXPECT_FALSE(rifle->Fire(msg, 1));
   delete rifle;
}

TEST_F(RifleVampireTests, LoadStakendThrowAway) {
   Rifle* rifle = new Rifle(GetIpcLocation());
   rifle->Aim();
   std::string msg("abc");
   EXPECT_FALSE(rifle->FireStake(&msg, 1));
   delete rifle;
}

TEST_F(RifleVampireTests, LoadStaksendThrowAway) {
   Rifle* rifle = new Rifle(GetIpcLocation());
   rifle->Aim();
   std::string msg;
   std::vector<std::pair<void*, unsigned int> > bundle;
   bundle.push_back(make_pair(&msg, 0));
   EXPECT_FALSE(rifle->FireStakes(bundle, 1));
   delete rifle;
}

TEST_F(RifleVampireTests, CreateVampireAndShootHim) {
   Vampire* vampire = new Vampire(GetIpcLocation());
   vampire->SetHighWater(1);
   vampire->PrepareToBeShot();
   std::string bullet;
   EXPECT_FALSE(vampire->GetShot(bullet, 1));
   EXPECT_EQ(1, vampire->GetHighWater());
   delete vampire;

}

TEST_F(RifleVampireTests, CreateVampireAndStakeHim) {
   Vampire* vampire = new Vampire(GetIpcLocation());
   vampire->SetHighWater(1);
   vampire->PrepareToBeShot();
   void* bullet;
   EXPECT_FALSE(vampire->GetStake(bullet, 1));
   EXPECT_EQ(1, vampire->GetHighWater());
   delete vampire;

}

TEST_F(RifleVampireTests, CreateVampireAndStakesHim) {
   Vampire* vampire = new Vampire(GetIpcLocation());
   vampire->SetHighWater(1);
   vampire->PrepareToBeShot();
   std::vector<std::pair<void*, unsigned int> > bundle;
   EXPECT_FALSE(vampire->GetStakes(bundle, 1));
   EXPECT_EQ(1, vampire->GetHighWater());
   delete vampire;

}

TEST_F(RifleVampireTests, CreateVampireAndRifleAndSetIOThreads) {
   Vampire* vampire = new Vampire(GetIpcLocation());
   vampire->SetHighWater(1);
   vampire->SetIOThreads(1);
   vampire->PrepareToBeShot();
   std::string bullet;
   EXPECT_FALSE(vampire->GetShot(bullet, 1));
   EXPECT_EQ(1, vampire->GetHighWater());
   EXPECT_EQ(1, vampire->GetIOThreads());
   delete vampire;

   Rifle* rifle = new Rifle(GetIpcLocation());
   rifle->SetHighWater(1);
   rifle->SetIOThreads(1);
   rifle->Aim();
   std::string msg;
   //should fail with empty message.
   EXPECT_FALSE(rifle->Fire(msg, 100));
   EXPECT_EQ(1, rifle->GetHighWater());
   EXPECT_EQ(1, rifle->GetIOThreads());
   delete rifle;

}

/**
 * This should have failures because there is no one pulling messages off.
 * for some reason even with a HWM of 1 there is extra buffering going on that
 * allows you to send more then 1 message.
 */
TEST_F(RifleVampireTests, ShootVampireMoreThenHeCanHandle) {
   std::string location = GetIpcLocation();
   int hwm = 100;
   int shots = (hwm * 10);
   Vampire vampire(location);
   Rifle rifle(location);
   rifle.SetHighWater(hwm);
   vampire.SetHighWater(hwm);
   rifle.Aim();
   vampire.PrepareToBeShot();
   std::string msg("woo");
   int success = 0;
   int failed = 0;
   for (int i = 0; i <= shots && !zctx_interrupted; i++) {
      if (rifle.Fire(msg, 2)) {
         success++;
      } else {
         failed++;
      }
   }
   std::string bullet;
   int rSuccess = 0;
   int rFailed = 0;
   for (int i = 0; i <= shots && !zctx_interrupted; i++) {
      if (vampire.GetShot(bullet, 10)) {
         rSuccess++;
      } else {
         rFailed++;
      }
   }
   std::cout << "Unable to shoot " << failed << " bullet(s)" << std::endl;
   EXPECT_EQ(success, rSuccess);
   EXPECT_EQ(failed, rFailed);
}

TEST_F(RifleVampireTests, StakeVampireMoreThenHeCanHandle) {
   std::string location = GetIpcLocation();
   int hwm = 10;
   int shots = (hwm * 10);
   Vampire vampire(location);
   Rifle rifle(location);
   rifle.SetHighWater(hwm);
   vampire.SetHighWater(hwm);
   rifle.Aim();
   vampire.PrepareToBeShot();
   std::string msg("woo");
   int success = 0;
   int failed = 0;
   for (int i = 0; i <= shots && !zctx_interrupted; i++) {
      if (rifle.FireStake(&msg, 2)) {
         success++;
      } else {
         failed++;
      }
   }
   void* stake;
   int rSuccess = 0;
   int rFailed = 0;
   for (int i = 0; i <= shots && !zctx_interrupted; i++) {
      if (vampire.GetStake(stake, 10)) {
         rSuccess++;
      } else {
         rFailed++;
      }
   }
   std::cout << "Unable to shoot " << failed << " bullet(s)" << std::endl;
   EXPECT_EQ(success, rSuccess);
   EXPECT_EQ(failed, rFailed);
}

TEST_F(RifleVampireTests, StakesVampireMoreThenHeCanHandle) {
   std::string location = GetIpcLocation();
   int hwm = 10;
   int shots = (hwm * 10);
   Vampire vampire(location);
   Rifle rifle(location);
   rifle.SetHighWater(hwm);
   vampire.SetHighWater(hwm);
   rifle.Aim();
   vampire.PrepareToBeShot();
   std::string msg("woo");
   std::vector<std::pair<void*, unsigned int> > bundle, gotBundle;
   bundle.push_back(make_pair(&msg, 0));
   int success = 0;
   int failed = 0;
   for (int i = 0; i <= shots && !zctx_interrupted; i++) {
      if (rifle.FireStakes(bundle)) {
         success++;
      } else {
         failed++;
      }
   }
   int rSuccess = 0;
   int rFailed = 0;
   for (int i = 0; i <= shots && !zctx_interrupted; i++) {
      if (vampire.GetStakes(gotBundle, 10)) {
         rSuccess++;
      } else {
         rFailed++;
      }
   }
   std::cout << "Unable to shoot " << failed << " bullet(s)" << std::endl;
   EXPECT_EQ(success, rSuccess);
   EXPECT_EQ(failed, rFailed);
}

/**
 * Test Firing after socket has been shutdown.
 */
TEST_F(RifleVampireTests, RifleImplosionTests) {
   std::string location = GetIpcLocation();
   TestRifle rifle(location);
   //   Rifle rifle(location);
   EXPECT_TRUE(rifle.Aim());
   rifle.Destroy();
   std::string bullet("woo");
   EXPECT_FALSE(rifle.Fire(bullet, 10));
}

TEST_F(RifleVampireTests, RifleStakeImplosionTests) {
   std::string location = GetIpcLocation();
   TestRifle rifle(location);
   //   Rifle rifle(location);
   EXPECT_TRUE(rifle.Aim());
   rifle.Destroy();
   std::string bullet("woo");
   EXPECT_FALSE(rifle.FireStake(&bullet, 10));
}

TEST_F(RifleVampireTests, RifleStakesImplosionTests) {
   std::string location = GetIpcLocation();
   TestRifle rifle(location);
   //   Rifle rifle(location);
   EXPECT_TRUE(rifle.Aim());
   rifle.Destroy();
   std::string msg("woo");
   std::vector<std::pair<void*, unsigned int> > bundle;
   bundle.push_back(make_pair(&msg, 0));
   EXPECT_FALSE(rifle.FireStakes(bundle, 10));
}

/**
 * Test Killing vampire and trying to get shot.
 */
TEST_F(RifleVampireTests, VampireShootingTests) {
   std::string location = GetIpcLocation();
   TestVampire vampire(location);
   EXPECT_TRUE(vampire.PrepareToBeShot());
   vampire.Destroy();
   std::string bullet;
   EXPECT_FALSE(vampire.GetShot(bullet, 10));
}

TEST_F(RifleVampireTests, VampireStakingTests) {
   std::string location = GetIpcLocation();
   TestVampire vampire(location);
   EXPECT_TRUE(vampire.PrepareToBeShot());
   vampire.Destroy();
   void* stake;
   EXPECT_FALSE(vampire.GetStake(stake, 10));
}

TEST_F(RifleVampireTests, VampireBundleStakingTests) {
   std::string location = GetIpcLocation();
   TestVampire vampire(location);
   EXPECT_TRUE(vampire.PrepareToBeShot());
   vampire.Destroy();
   std::vector<std::pair<void*, unsigned int> > bundle;
   EXPECT_FALSE(vampire.GetStakes(bundle, 1));
}

/**
 * This should have failures because there is no one pulling messages off.
 * for some reason even with a HWM of 1 there is extra buffering going on that
 * allows you to send more then 1 message.
 * @param
 */
TEST_F(RifleVampireTests, BringDeadVampireBackToLife) {
   std::string location = GetIpcLocation();
   int shots = 10;
   int hwm = 100;
   TestVampire vampire(location);
   TestRifle rifle(location);
   rifle.SetHighWater(hwm);
   vampire.SetHighWater(hwm);
   rifle.Aim();
   vampire.PrepareToBeShot();
   std::string msg("woo");
   int success = 0;
   for (int i = 0; i <= shots && !zctx_interrupted; i++) {
      ASSERT_TRUE(rifle.Fire(msg));
      success++;
   }
   std::string bullet;
   for (int i = 0; i <= shots && !zctx_interrupted; i++) {
      EXPECT_TRUE(vampire.GetShot(bullet, 1));
   }

   rifle.Destroy();
   vampire.Destroy();

   rifle.Aim();
   vampire.PrepareToBeShot();


   for (int i = 0; i <= shots && !zctx_interrupted; i++) {
      ASSERT_TRUE(rifle.Fire(msg));
   }
   for (int i = 0; i <= shots && !zctx_interrupted; i++) {
      EXPECT_TRUE(vampire.GetShot(bullet, 1));
   }


}

/**
 * Test socket ownership. Only one things can own the socket.
 * @param
 */
TEST_F(RifleVampireTests, MultileRiflesSocketOwnerTest) {

   std::string msg("Woo");
   std::string location(GetIpcLocation());
   Vampire vampire(location);
   Vampire vampire2(location);
   EXPECT_TRUE(vampire.PrepareToBeShot());
   EXPECT_TRUE(vampire2.PrepareToBeShot());
   Rifle rifle1(location);
   rifle1.SetOwnSocket(true);
   Rifle rifle2(location);
   rifle2.SetOwnSocket(true);

   EXPECT_TRUE(rifle1.Aim());
   EXPECT_TRUE(rifle2.Aim());

   sleep(10);
   //We should fail to send because we no longer own the socket.
   EXPECT_FALSE(rifle1.Fire(msg, 10));

   EXPECT_TRUE(rifle2.Fire(msg, 500));
   EXPECT_TRUE(rifle2.Fire(msg, 500));

   std::string bullet;
   //messages should have been load balanced between them, so each should have
   //a single shot.
   EXPECT_TRUE(vampire.GetShot(bullet, 1));
   EXPECT_TRUE(vampire2.GetShot(bullet, 1));

}

