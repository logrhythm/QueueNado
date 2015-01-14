#include <czmq.h>
#include <thread>
#include "HarpoonKrakenTests.h"
#include "MockKraken.h"
#include "Harpoon.h"
#include "Death.h"
#include <future>
#include <atomic>

namespace {

   std::atomic<bool> gBreaking{false};

   void SendHello(std::string address) {
   Kraken server;
   server.SetLocation(address);
   server.MaxWaitInMs(1000); // 1 second

   uint8_t h = 'h';
   uint8_t e = 'e';
   uint8_t l = 'l';
   uint8_t o = 'o';
   uint8_t end = '\0';   
   std::vector<uint8_t> hello{h,e,l,l,o,end};

   for (auto c : hello) {
      auto status = server.SendTidalWave({c});
      LOG(INFO) << "SendTidalWave returned: " << static_cast<int> (status);

      if (gBreaking.load()) {
         return;
      }
   }
   server.FinalBreach();
   return;
}


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

   for (uint8_t i = 0; i < 30; i++) {
      server.SendTidalWave({i});
   }

   return nullptr;
}

void* HarpoonKrakenTests::SendThreadSendThirtyTwoEnd(void* arg) {
   std::string address = *(reinterpret_cast<std::string*>(arg));
   MockKraken server;
   server.SetLocation(address);
   server.MaxWaitInMs(1000);

   for (uint8_t i = 0; i < 30; i++) {
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


TEST_F(HarpoonKrakenTests, SendThreadSendThirtyTwoEndWithIpc) {


   // IF THIS mIPC line is removed then we use
   // a real ipc file,.. but that doesn't work
   // mIpc = "ipc:///tmp/HarpoonKrakenTests.ipc"
   mIpc = "tcp://127.0.0.1:13123";
   auto done = std::async(std::launch::async, &SendHello, mIpc);

   Harpoon client;
   std::vector<uint8_t> p;
   client.MaxWaitInMs(1000); // on purpose not the same timeout
   
   Harpoon::Spear status = client.Aim(mIpc);
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
   gBreaking = true;
   done.wait();
   
   //Should now receive an empty chucnk to indicate end of stream:
   Harpoon::Battling res = client.Heave(p);  
   EXPECT_EQ(res, Harpoon::Battling::VICTORIOUS) << "result: " << static_cast<int>(res) << ", VICTORIOUS: " << static_cast<int>(Harpoon::Battling::VICTORIOUS);

   EXPECT_EQ(p.size(), 0);

}

