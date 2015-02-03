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

namespace {
   std::atomic<bool> gSendSmallChunks{false};
}
void* HarpoonKrakenTests::SendHello(void* arg) {
   std::string address = *(reinterpret_cast<std::string*>(arg));
   Kraken server;
   server.SetLocation(address);
   server.MaxWaitInMs(1000); // 1 second

   gSendSmallChunks = true;

   uint8_t h = 'h';
   uint8_t e = 'e';
   uint8_t l = 'l';
   uint8_t o = 'o';
   uint8_t end = '\0';   
   std::vector<uint8_t> hello{h,e,l,l,o,end};

   for (auto c : hello) {
      auto status = server.SendTidalWave({c});
      LOG(INFO) << "SendTidalWave returned: " << static_cast<int> (status);
   }
   server.FinalBreach();
   return nullptr;
}

void* HarpoonKrakenTests::SendSmallChunks(void* arg) {
   std::string address = *(reinterpret_cast<std::string*>(arg));
   Kraken server;
   server.ChangeDefaultMaxChunkSizeInBytes(2);
   server.SetLocation(address);
   server.MaxWaitInMs(1000); // 1 second

   gSendSmallChunks = true;

   uint8_t h = 'h';
   uint8_t e = 'e';
   uint8_t l = 'l';
   uint8_t o = 'o';
   uint8_t end = '\0';   
   std::vector<uint8_t> hello{h,e,l,l,o,end};
   std::vector<uint8_t> hell{h,e,l,l,end};
   std::vector<uint8_t> hEnd{h,end};
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
   std::vector<uint8_t> v = {10,20,30};

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
   return(rand()%(max-min))+min;
}

std::string HarpoonKrakenTests::GetTcpLocation(int port) {
   std::string tcpLocation("tcp://127.0.0.1:");
   tcpLocation.append(std::to_string(port));
   return tcpLocation;
}

TEST_F(HarpoonKrakenTests, HarpoonSetLocation) {
   Harpoon client;

   int port = GetTcpPort();
   std::string location = HarpoonKrakenTests::GetTcpLocation(port);
   Harpoon::Spear status = client.Aim(location);

   EXPECT_EQ(status, Harpoon::Spear::IMPALED);   
}

TEST_F(HarpoonKrakenTests, PollTimeoutReturnsTimeout) {
   using namespace std::chrono;

   MockHarpoon client;

   int port = GetTcpPort();
   std::string location = HarpoonKrakenTests::GetTcpLocation(port);
   Harpoon::Spear status = client.Aim(location);

   EXPECT_EQ(status, Harpoon::Spear::IMPALED);

   int kWaitMs(21);
   client.MaxWaitInMs(kWaitMs);

   steady_clock::time_point pollStartMs = steady_clock::now();
   Harpoon::Battling Battling = client.CallPollTimeout();
   int pollElapsedMs = duration_cast<milliseconds>(steady_clock::now() - pollStartMs).count();
   EXPECT_EQ(Battling, Harpoon::Battling::TIMEOUT);
   EXPECT_EQ(kWaitMs, pollElapsedMs);
}

TEST_F(HarpoonKrakenTests, SendTidalWaveGetNextChunkIdMethods) {
   //Server will receive data requests from client, but will not respond to them.
   //  Client therefore will timeout:
   int port = GetTcpPort();
   std::string location = GetTcpLocation(port);
   zthread_new(HarpoonKrakenTests::SendThreadNextChunkIdDie, reinterpret_cast<void*>(&location));

   Harpoon client;
   client.MaxWaitInMs(1000);

   Harpoon::Spear status = client.Aim(location);
   EXPECT_EQ(status, Harpoon::Spear::IMPALED);   
   
   std::vector<uint8_t> p;
   Harpoon::Battling res = client.Heave(p);

   EXPECT_EQ(res, Harpoon::Battling::TIMEOUT);
   EXPECT_EQ(p.size(), 0);
}

TEST_F(HarpoonKrakenTests, SendTidalWaveOneChunkReceivedMethods) {
   //Server will receive data requests from client and only send one chunk before quitting.
   //  Client therefore will timeout:
   int port = GetTcpPort();
   std::string location = GetTcpLocation(port);
   zthread_new(HarpoonKrakenTests::SendThreadSendOneDie, reinterpret_cast<void*>(&location));

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
}

TEST_F(HarpoonKrakenTests, SendThirtyDataChunksReceivedMethods) {

   int port = GetTcpPort();
   std::string location = GetTcpLocation(port);
   zthread_new(HarpoonKrakenTests::SendThreadSendThirtyDie, reinterpret_cast<void*>(&location));

   Harpoon client;
   std::vector<uint8_t> p;
   
   client.MaxWaitInMs(1000);
   Harpoon::Spear status = client.Aim(location);
   EXPECT_EQ(status, Harpoon::Spear::IMPALED); 

   for (int i = 0; i < 30; ++i){
      Harpoon::Battling res = client.Heave(p); 

      EXPECT_EQ(res, Harpoon::Battling::CONTINUE); 
      ASSERT_EQ(p.size(), 1);
      EXPECT_EQ(p[0], i);
   }
   
   Harpoon::Battling res = client.Heave(p);  
   EXPECT_EQ(res, Harpoon::Battling::TIMEOUT);
   EXPECT_EQ(p.size(), 0);
}

TEST_F(HarpoonKrakenTests, SendThirtyTwoDataChunksReceivedMethods) {

   int port = GetTcpPort();
   std::string location = GetTcpLocation(port);
   zthread_new(HarpoonKrakenTests::SendThreadSendThirtyTwoEnd, reinterpret_cast<void*>(&location));

   Harpoon client;
   std::vector<uint8_t> p;
   client.MaxWaitInMs(1000);
   
   Harpoon::Spear status = client.Aim(location);
   EXPECT_EQ(status, Harpoon::Spear::IMPALED); 

   for (int i = 0; i < 30; ++i){
      Harpoon::Battling res = client.Heave(p); 

      EXPECT_EQ(res, Harpoon::Battling::CONTINUE); 
      ASSERT_EQ(p.size(), 1);
      EXPECT_EQ(p[0], i);
   }
   
   //Should now receive an empty chucnk to indicate end of stream:
   Harpoon::Battling res = client.Heave(p);  
   EXPECT_EQ(res, Harpoon::Battling::VICTORIOUS);
   EXPECT_EQ(p.size(), 0);
}


TEST_F(HarpoonKrakenTests, SendThreadSendHello) {

   gSendSmallChunks = false;

   int port = GetTcpPort();
   std::string location = GetTcpLocation(port);
   zthread_new(HarpoonKrakenTests::SendHello, reinterpret_cast<void*>(&location));


   Harpoon client;
   std::vector<uint8_t> p;
   client.MaxWaitInMs(1000); // on purpose not the same timeout

   while (!gSendSmallChunks) { }
   
   Harpoon::Spear status = client.Aim(location);
   EXPECT_EQ(status, Harpoon::Spear::IMPALED) << "1st: " << static_cast<int>(status) << ", IMPALED: " << static_cast<int>(Harpoon::Spear::IMPALED);

   uint8_t h = 'h';
   uint8_t e = 'e';
   uint8_t l = 'l';
   uint8_t o = 'o';
   uint8_t end = '\0';
   const std::vector<uint8_t> expected{h,e,l,l,o,end};
   
   for (const auto c: expected){
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

}

TEST_F(HarpoonKrakenTests, SendInSmallChunks) {

   gSendSmallChunks = false;
   int port = GetTcpPort();
   std::string location = GetTcpLocation(port);
   zthread_new(HarpoonKrakenTests::SendSmallChunks, reinterpret_cast<void*>(&location));

   Harpoon client;
   std::vector<uint8_t> p;
   client.MaxWaitInMs(1000);

   while (!gSendSmallChunks) { }

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
}

