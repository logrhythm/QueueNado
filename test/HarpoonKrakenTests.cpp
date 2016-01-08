#include <czmq.h>
#include <thread>
#include "HarpoonKrakenTests.h"
#include "MockKraken.h"
#include "MockHarpoon.h"
#include "Harpoon.h"
#include "Death.h"
#include <chrono>
#include <future>
#include <atomic>

void* HarpoonKrakenTests::SendHello(void* arg) {
   std::string address = *(reinterpret_cast<std::string*>(arg));
   Kraken server;
   server.SetLocation(address);
   server.MaxWaitInMs(1000); // 1 second

   uint8_t h = 'h';
   uint8_t e = 'e';
   uint8_t l = 'l';
   uint8_t o = 'o';
   uint8_t end = '\0';
   std::vector<uint8_t> hello{h, e, l, l, o, end};

   for (auto c : hello) {
      auto status = server.SendTidalWave({c});
      LOG(INFO) << "SendTidalWave returned: " << static_cast<int> (status);
   }
   server.FinalBreach();
   return nullptr;
}


namespace {
   struct HelloAbort {
      std::string address;
      std::atomic<size_t>  abortAt;
      explicit HelloAbort(std::string addressQ)
         : address(addressQ)
         , abortAt(0)
      {}
   };
} // anonymous
void* HarpoonKrakenTests::SendHelloExpectCancel(void* arg) {
   using AbortNotifyPtr = std::shared_ptr<HelloAbort>;
   std::shared_ptr<HelloAbort> helloAbort = *(reinterpret_cast<AbortNotifyPtr*>(arg));


   Kraken server;
   server.SetLocation(helloAbort->address);
   server.MaxWaitInMs(1000); // 1 second

   uint8_t h = 'h';
   uint8_t e = 'e';
   uint8_t l = 'l';
   uint8_t o = 'o';
   uint8_t end = '\0';
   std::vector<uint8_t> hello{h, e, l, l, o, end};

   signed int counter = -1; // 1 off 
   auto status = Kraken::Battling::CONTINUE;
   bool cancelReceived = {false};
   for (auto c : hello) {
      status = server.SendTidalWave({c});
      if (status == Kraken::Battling::CANCEL) {
         cancelReceived = true;
         helloAbort->abortAt.store(counter);
         LOG(INFO) << "Received cancel at count: " << counter;
         break;
      }
      LOG(INFO) << "SendTidalWave/Cancel returned: " << static_cast<int> (status);
      ++counter;
   }
   EXPECT_TRUE(cancelReceived);
   server.FinalBreach();
   return nullptr;
}



void* HarpoonKrakenTests::SendSmallChunks(void* arg) {
   std::string address = *(reinterpret_cast<std::string*>(arg));
   Kraken server;
   server.ChangeDefaultMaxChunkSizeInBytes(2);
   server.SetLocation(address);
   server.MaxWaitInMs(1000); // 1 second

   uint8_t h = 'h';
   uint8_t e = 'e';
   uint8_t l = 'l';
   uint8_t o = 'o';
   uint8_t end = '\0';
   std::vector<uint8_t> hello{h, e, l, l, o, end};
   std::vector<uint8_t> hell{h, e, l, l, end};
   std::vector<uint8_t> hEnd{h, end};
   std::vector<uint8_t> hV{h};

   auto status = server.SendTidalWave(hello);
   LOG(INFO) << "SendTidalWave returned: " << static_cast<int> (status);

   status = server.SendTidalWave(hell);
   LOG(INFO) << "SendTidalWave returned: " << static_cast<int> (status);

   status = server.SendTidalWave(hEnd);
   LOG(INFO) << "SendTidalWave returned: " << static_cast<int> (status);

   status = server.SendTidalWave(hV);
   LOG(INFO) << "SendTidalWave returned: " << static_cast<int> (status);

   server.FinalBreach();
   return nullptr;
}

void* HarpoonKrakenTests::SendThreadNextChunkIdDie(void* arg) {
   std::string address = *(reinterpret_cast<std::string*>(arg));
   MockKraken server;
   server.SetLocation(address);
   server.MaxWaitInMs(1000);
   server.CallNextChunkId();
   return nullptr;
}

void* HarpoonKrakenTests::SendThreadSendOneDie(void* arg) {
   std::string address = *(reinterpret_cast<std::string*>(arg));
   std::vector<uint8_t> v = {10, 20, 30};

   MockKraken server;
   server.SetLocation(address);
   server.MaxWaitInMs(1000);
   server.SendTidalWave(v);

   return nullptr;
}

void* HarpoonKrakenTests::SendThreadSendThirtyDie(void* arg) {
   std::string address = *(reinterpret_cast<std::string*>(arg));
   MockKraken server;
   server.SetLocation(address);
   server.MaxWaitInMs(1000);

   for (uint8_t i = 0; i < 30; ++i) {
      server.SendTidalWave({i});
   }

   return nullptr;
}

void* HarpoonKrakenTests::SendThreadSendThirtyTwoEnd(void* arg) {
   std::string address = *(reinterpret_cast<std::string*>(arg));
   MockKraken server;
   server.SetLocation(address);
   server.MaxWaitInMs(1000);

   for (uint8_t i = 0; i < 30; ++i) {
      server.SendTidalWave({i});
   }
   server.FinalBreach();

   return nullptr;
}




int HarpoonKrakenTests::GetTcpPort() {
   int max = 15000;
   int min = 13000;
   return (rand() % (max - min)) + min;
}

std::string HarpoonKrakenTests::GetTcpLocation(int port) {
   std::string tcpLocation("tcp://127.0.0.1:");
   tcpLocation.append(std::to_string(port));
   return tcpLocation;
}


TEST_F(HarpoonKrakenTests, HarpoonEnumToString) {
   Harpoon harpoon;
   const auto kTimeout = harpoon.EnumToString(Harpoon::Battling::TIMEOUT);
   const auto kInterrupt = harpoon.EnumToString(Harpoon::Battling::INTERRUPT);
   const auto kVictorious = harpoon.EnumToString(Harpoon::Battling::VICTORIOUS);
   const auto kContinue = harpoon.EnumToString(Harpoon::Battling::CONTINUE);
   const auto kCancel = harpoon.EnumToString(Harpoon::Battling::CANCEL);

   EXPECT_TRUE(kTimeout == "<TIMEOUT>");
   EXPECT_TRUE(kInterrupt == "<INTERRUPT>");
   EXPECT_TRUE(kVictorious == "<VICTORIOUS>");
   EXPECT_TRUE(kContinue == "<CONTINUE>");
   EXPECT_TRUE(kCancel == "<CANCEL>");
}




TEST_F(HarpoonKrakenTests, HarpoonSetGoodLocation) {
   Harpoon client;

   int port = GetTcpPort();
   std::string location = HarpoonKrakenTests::GetTcpLocation(port);
   Harpoon::Spear status = client.Aim(location);

   EXPECT_EQ(status, Harpoon::Spear::IMPALED);
}

TEST_F(HarpoonKrakenTests, HarpoonSetBadLocation) {
   Harpoon client;

   std::string location("bad location");
   Harpoon::Spear status = client.Aim(location);

   EXPECT_EQ(Harpoon::Spear::MISS, status);
}

TEST_F(HarpoonKrakenTests, PollTimeoutReturnsTimeout) {
   using namespace std::chrono;

   MockHarpoon client;

   int port = GetTcpPort();
   std::string location = HarpoonKrakenTests::GetTcpLocation(port);
   Harpoon::Spear status = client.Aim(location);
   EXPECT_EQ(status, Harpoon::Spear::IMPALED);

   for (int timeoutMs = 1; timeoutMs < 50; timeoutMs += 5) {
      steady_clock::time_point pollStartMs = steady_clock::now();
      Harpoon::Battling Battling = client.CallPollTimeout(timeoutMs);
      int pollElapsedMs = duration_cast<milliseconds>(steady_clock::now() - pollStartMs).count();
      EXPECT_EQ(Battling, Harpoon::Battling::TIMEOUT);
      EXPECT_LE(timeoutMs, pollElapsedMs);
      EXPECT_GE(timeoutMs, pollElapsedMs - 1);
   }
}

TEST_F(HarpoonKrakenTests, SendTidalWaveGetNextChunkIdMethods) {
   //Server will receive data requests from client, but will not respond to them.
   //  Client therefore will timeout:
   int port = GetTcpPort();
   std::string location = GetTcpLocation(port);
   auto done = std::async(std::launch::async, &SendThreadNextChunkIdDie, &location);

   Harpoon client;
   client.MaxWaitInMs(1000);

   Harpoon::Spear status = client.Aim(location);
   EXPECT_EQ(status, Harpoon::Spear::IMPALED);

   std::vector<uint8_t> p;
   Harpoon::Battling res = client.Heave(p);

   EXPECT_EQ(res, Harpoon::Battling::TIMEOUT);
   EXPECT_EQ(p.size(), 0);
   done.wait();
}

TEST_F(HarpoonKrakenTests, SendTidalWaveOneChunkReceivedMethods) {
   //Server will receive data requests from client and only send one chunk before quitting.
   //  Client therefore will timeout:
   int port = GetTcpPort();
   std::string location = GetTcpLocation(port);
   auto done = std::async(std::launch::async, &SendThreadSendOneDie, &location);

   Harpoon client;
   client.MaxWaitInMs(1000);

   Harpoon::Spear status = client.Aim(location);
   EXPECT_EQ(status, Harpoon::Spear::IMPALED);

   std::vector<uint8_t> p;
   Harpoon::Battling res = client.Heave(p);

   EXPECT_EQ(res, Harpoon::Battling::CONTINUE);
   ASSERT_EQ(p.size(), 3);
   EXPECT_EQ(p[0], 10);
   EXPECT_EQ(p[1], 20);
   EXPECT_EQ(p[2], 30);

   res = client.Heave(p);
   EXPECT_EQ(res, Harpoon::Battling::TIMEOUT);
   done.wait();
}

TEST_F(HarpoonKrakenTests, SendThirtyDataChunksReceivedMethods) {

   int port = GetTcpPort();
   std::string location = GetTcpLocation(port);
   auto done = std::async(std::launch::async, &SendThreadSendThirtyDie, &location);

   Harpoon client;
   std::vector<uint8_t> p;

   client.MaxWaitInMs(1000);
   Harpoon::Spear status = client.Aim(location);
   EXPECT_EQ(status, Harpoon::Spear::IMPALED);

   for (int i = 0; i < 30; ++i) {
      Harpoon::Battling res = client.Heave(p);

      EXPECT_EQ(res, Harpoon::Battling::CONTINUE);
      ASSERT_EQ(p.size(), 1);
      EXPECT_EQ(p[0], i);
   }

   Harpoon::Battling res = client.Heave(p);
   EXPECT_EQ(res, Harpoon::Battling::TIMEOUT);
   EXPECT_EQ(p.size(), 0);
   done.wait();
}

TEST_F(HarpoonKrakenTests, SendThirtyTwoDataChunksReceivedMethods) {

   int port = GetTcpPort();
   std::string location = GetTcpLocation(port);
   auto done = std::async(std::launch::async, &SendThreadSendThirtyTwoEnd, &location);

   Harpoon client;
   std::vector<uint8_t> p;
   client.MaxWaitInMs(1000);

   Harpoon::Spear status = client.Aim(location);
   EXPECT_EQ(status, Harpoon::Spear::IMPALED);

   for (int i = 0; i < 30; ++i) {
      Harpoon::Battling res = client.Heave(p);

      EXPECT_EQ(res, Harpoon::Battling::CONTINUE);
      ASSERT_EQ(p.size(), 1);
      EXPECT_EQ(p[0], i);
   }

   //Should now receive an empty chucnk to indicate end of stream:
   Harpoon::Battling res = client.Heave(p);
   EXPECT_EQ(res, Harpoon::Battling::VICTORIOUS);
   EXPECT_EQ(p.size(), 0);
   done.wait();
}


TEST_F(HarpoonKrakenTests, SendThreadSendHello) {

   int port = GetTcpPort();
   std::string location = GetTcpLocation(port);
   auto done = std::async(std::launch::async, &SendHello, &location);

   Harpoon client;
   std::vector<uint8_t> p;
   client.MaxWaitInMs(1000); // on purpose not the same timeout

   Harpoon::Spear status = client.Aim(location);
   EXPECT_EQ(status, Harpoon::Spear::IMPALED) << "1st: " << static_cast<int>(status) << ", IMPALED: " << static_cast<int>(Harpoon::Spear::IMPALED);

   uint8_t h = 'h';
   uint8_t e = 'e';
   uint8_t l = 'l';
   uint8_t o = 'o';
   uint8_t end = '\0';
   const std::vector<uint8_t> expected{h, e, l, l, o, end};

   for (const auto c : expected) {
      std::vector<uint8_t> p;
      Harpoon::Battling res = client.Heave(p);
      EXPECT_EQ(res, Harpoon::Battling::CONTINUE);
      ASSERT_EQ(p.size(), 1);
      EXPECT_EQ(p[0], c);
   }

   //Should now receive an empty chucnk to indicate end of stream:
   Harpoon::Battling res = client.Heave(p);
   EXPECT_EQ(res, Harpoon::Battling::VICTORIOUS) << "result: " << static_cast<int>(res) << ", VICTORIOUS: " << static_cast<int>(Harpoon::Battling::VICTORIOUS);

   EXPECT_EQ(p.size(), 0);
   done.wait();
}

TEST_F(HarpoonKrakenTests, ReceiveAbortDuringHello) {

   const int port = GetTcpPort();
   const std::string location = GetTcpLocation(port);
   auto helloAbort = std::make_shared<HelloAbort>(location);
   auto done = std::async(std::launch::async, &SendHelloExpectCancel, &helloAbort);

   Harpoon client;
   std::vector<uint8_t> p;
   client.MaxWaitInMs(1000); // on purpose not the same timeout

   Harpoon::Spear status = client.Aim(location);
   EXPECT_EQ(status, Harpoon::Spear::IMPALED) << "1st: " << static_cast<int>(status) << ", IMPALED: " << static_cast<int>(Harpoon::Spear::IMPALED);

   uint8_t h = 'h';
   uint8_t e = 'e';
   uint8_t l = 'l';
   uint8_t o = 'o';
   uint8_t end = '\0';

   const std::vector<uint8_t> expected{h, e, l, l, o, end};
   const size_t abortAt = 2;
   size_t counter = 0;
   bool cancelSet = {false};
   for (const auto c : expected) {
      std::vector<uint8_t> p;
      auto res = Harpoon::Battling::CONTINUE;

      if (abortAt == counter) {
         LOG(INFO) << "Calling cancel at count:" << counter << std::endl;
         res = client.Cancel();
         cancelSet = true;
         break;
      } else if (counter < abortAt){
         res = client.Heave(p);
         EXPECT_EQ(res, Harpoon::Battling::CONTINUE);
         ASSERT_EQ(p.size(), 1);
         EXPECT_EQ(p[0], c);
      }
      ++counter;
   }
   EXPECT_EQ(counter, 2);
   EXPECT_TRUE(cancelSet);

   //Should now receive an empty chunk to indicate end of stream:
   // technically this was "victorious by escape" ;)
   Harpoon::Battling res = client.Heave(p);
   EXPECT_EQ(res, Harpoon::Battling::VICTORIOUS) << "result: " << static_cast<int>(res) << ", VICTORIOUS: " << static_cast<int>(Harpoon::Battling::VICTORIOUS);

   EXPECT_EQ(p.size(), 0);
   done.wait();
   EXPECT_EQ(abortAt, helloAbort->abortAt.load());
}



TEST_F(HarpoonKrakenTests, SendInSmallChunks) {

   int port = GetTcpPort();
   std::string location = GetTcpLocation(port);
   auto done = std::async(std::launch::async, &SendSmallChunks, &location);

   Harpoon client;
   std::vector<uint8_t> p;
   client.MaxWaitInMs(1000);

   uint8_t h = 'h';
   uint8_t e = 'e';
   uint8_t l = 'l';
   uint8_t o = 'o';
   uint8_t end = '\0';

   Harpoon::Spear status = client.Aim(location);
   EXPECT_EQ(status, Harpoon::Spear::IMPALED);

   Harpoon::Battling res = client.Heave(p);
   EXPECT_EQ(res, Harpoon::Battling::CONTINUE);
   ASSERT_EQ(p.size(), 2);
   EXPECT_EQ(p[0], h);
   EXPECT_EQ(p[1], e);

   res = client.Heave(p);
   EXPECT_EQ(res, Harpoon::Battling::CONTINUE);
   ASSERT_EQ(p.size(), 2);
   EXPECT_EQ(p[0], l);
   EXPECT_EQ(p[1], l);

   res = client.Heave(p);
   EXPECT_EQ(res, Harpoon::Battling::CONTINUE);
   ASSERT_EQ(p.size(), 2);
   EXPECT_EQ(p[0], o);
   EXPECT_EQ(p[1], end);

   res = client.Heave(p);
   EXPECT_EQ(res, Harpoon::Battling::CONTINUE);
   ASSERT_EQ(p.size(), 2);
   EXPECT_EQ(p[0], h);
   EXPECT_EQ(p[1], e);

   res = client.Heave(p);
   EXPECT_EQ(res, Harpoon::Battling::CONTINUE);
   ASSERT_EQ(p.size(), 2);
   EXPECT_EQ(p[0], l);
   EXPECT_EQ(p[1], l);

   res = client.Heave(p);
   EXPECT_EQ(res, Harpoon::Battling::CONTINUE);
   ASSERT_EQ(p.size(), 1);
   EXPECT_EQ(p[0], end);

   res = client.Heave(p);
   EXPECT_EQ(res, Harpoon::Battling::CONTINUE);
   ASSERT_EQ(p.size(), 2);
   EXPECT_EQ(p[0], h);
   EXPECT_EQ(p[1], end);

   res = client.Heave(p);
   EXPECT_EQ(res, Harpoon::Battling::CONTINUE);
   ASSERT_EQ(p.size(), 1);
   EXPECT_EQ(p[0], h);

   //Should now receive an empty chucnk to indicate end of stream:
   res = client.Heave(p);
   EXPECT_EQ(res, Harpoon::Battling::VICTORIOUS);
   EXPECT_EQ(p.size(), 0);
   done.wait();
}

