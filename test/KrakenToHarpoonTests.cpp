#include <czmq.h>
#include <thread>
#include "KrakenToHarpoonTests.h"
#include "MockHarpoon.h"
#include "MockKraken.h"
#include "Kraken.h"
#include "Death.h"
#include <chrono>
#include <future>
#include <atomic>

void* KrakenToHarpoonTests::RecvThreadNextChunkIdDie(void* arg) {
   std::string address = *(reinterpret_cast<std::string*>(arg));
   MockHarpoon client;
   client.Aim(address);
   client.MaxWaitInMs(500);
   client.CallRequestChunks();
   return nullptr;
}

void* KrakenToHarpoonTests::RecvThreadNextChunkIdWait(void* arg) {
   std::string address = *(reinterpret_cast<std::string*>(arg));
   std::vector<uint8_t> p;
   MockHarpoon client;
   client.Aim(address);
   client.MaxWaitInMs(500);
   client.Heave(p);
   return nullptr;
}

void* KrakenToHarpoonTests::RecvThreadGetThreeWait(void* arg) {
   std::string address = *(reinterpret_cast<std::string*>(arg));

   std::vector<uint8_t> p;
   MockHarpoon client;
   client.Aim(address);
   client.MaxWaitInMs(500);
   client.Heave(p);
   client.Heave(p);
   client.Heave(p);
   client.Heave(p);
   return nullptr;
}

void* KrakenToHarpoonTests::RecvThreadGetFileDie(void* arg) {
   std::string address = *(reinterpret_cast<std::string*>(arg));
   
   std::vector<uint8_t> p;
   MockHarpoon client;
   client.MaxWaitInMs(500);
   client.Aim(address);

   while(Harpoon::Battling::CONTINUE == client.Heave(p)){

   }
   return nullptr;
}

int KrakenToHarpoonTests::GetTcpPort() {
   int max = 15000;
   int min = 13000;
   return(rand()%(max-min))+min;
}

std::string KrakenToHarpoonTests::GetTcpLocation(int port) {
   std::string tcpLocation("tcp://127.0.0.1:");
   tcpLocation.append(std::to_string(port));
   return tcpLocation;
}


TEST_F(KrakenToHarpoonTests, KrakenEnumToString) {
   Kraken kraken;
   const auto kTimeout = kraken.EnumToString(Kraken::Battling::TIMEOUT);
   const auto kInterrupt = kraken.EnumToString(Kraken::Battling::INTERRUPT);
   const auto kContinue = kraken.EnumToString(Kraken::Battling::CONTINUE);
   const auto kCancel = kraken.EnumToString(Kraken::Battling::CANCEL);

   EXPECT_TRUE(kTimeout == "<TIMEOUT>");
   EXPECT_TRUE(kInterrupt == "<INTERRUPT>");
   EXPECT_TRUE(kContinue == "<CONTINUE>");
   EXPECT_TRUE(kCancel == "<CANCEL>");
}




TEST_F(KrakenToHarpoonTests, KrakenSetLocation) {
   Kraken server;

   int port = GetTcpPort();
   std::string location = KrakenToHarpoonTests::GetTcpLocation(port);

   Kraken::Spear spear = server.SetLocation(location);
   EXPECT_EQ(spear, Kraken::Spear::IMPALED);  
}

TEST_F(KrakenToHarpoonTests, Default_MaxChunkSize) {
   Kraken kraken;
   EXPECT_EQ(10 * 1024 * 1024, kraken.MaxChunkSizeInBytes());
   kraken.ChangeDefaultMaxChunkSizeInBytes(100);
   EXPECT_EQ(100, kraken.MaxChunkSizeInBytes());
}


TEST_F(KrakenToHarpoonTests, SendTidalWaveGetNextChunkIdDieMethods) {
   //Client thread will send out request for 10 chunks and instantly die. Therefore, none of those
   // chunks will be received and a -2 status should be returned
   int port = GetTcpPort();
   std::string location = GetTcpLocation(port);

   auto done = std::async(std::launch::async, &RecvThreadNextChunkIdDie, &location);

   MockKraken server;
   server.MaxWaitInMs(500);

   Kraken::Spear spear = server.SetLocation(location);
   EXPECT_EQ(spear, Kraken::Spear::IMPALED);    
   
   for(int i = 0; i < 10; i++){
      Kraken::Battling Battling = server.CallNextChunkId(); 
      EXPECT_EQ(Battling, Kraken::Battling::TIMEOUT);  
   }
   done.wait();
}

TEST_F(KrakenToHarpoonTests, GetNextChunkIdWaitMethods) {
   //Client thread will send out request for 10 chunks and wait for resulting data.
   // Server should receive each of those chunks.
   // Server will not send back data, so client will time out and die.

   int port = GetTcpPort();
   std::string location = GetTcpLocation(port);

   MockKraken server;
   server.MaxWaitInMs(500);
   Kraken::Spear spear = server.SetLocation(location);
   EXPECT_EQ(spear, Kraken::Spear::IMPALED);   

   auto done = std::async(std::launch::async, &RecvThreadNextChunkIdWait, &location);
   
   for(int i = 0; i < 1; i++){
      Kraken::Battling Battling = server.CallNextChunkId(); 
      EXPECT_EQ(Battling, Kraken::Battling::CONTINUE);  
   }

   Kraken::Battling Battling = server.CallNextChunkId(); 
   EXPECT_EQ(Battling, Kraken::Battling::TIMEOUT);  
   done.wait();
}

TEST_F(KrakenToHarpoonTests, SendTidalWaveGetResultingWaitsMethods) {
   //Client thread will send out request for 10 chunks and wait for resulting data.
   // Server should send 3 pieces of data.
   // Server then should see 10 requests in pipeline (3 - 13)
   // Server will then not send back data, so client will time out and die.

   int port = GetTcpPort();
   std::string location = GetTcpLocation(port);

   auto done = std::async(std::launch::async, &RecvThreadGetThreeWait, &location);

   MockKraken server;
   server.MaxWaitInMs(500);
   
   Kraken::Spear spear = server.SetLocation(location);
   EXPECT_EQ(spear, Kraken::Spear::IMPALED);  

   std::vector<uint8_t> data = {10,20,30};
   Kraken::Battling Battling = server.SendTidalWave(data);
   EXPECT_EQ(Battling, Kraken::Battling::CONTINUE);
   Battling = server.SendTidalWave(data);
   EXPECT_EQ(Battling, Kraken::Battling::CONTINUE);
   Battling = server.SendTidalWave(data);
   EXPECT_EQ(Battling, Kraken::Battling::CONTINUE);

   for(int i = 0; i < 1; i++){
      Battling = server.CallNextChunkId(); 
      EXPECT_EQ(Battling, Kraken::Battling::CONTINUE);
   }

   Battling = server.CallNextChunkId(); 
   EXPECT_EQ(Battling, Kraken::Battling::TIMEOUT);   
   done.wait();
}

TEST_F(KrakenToHarpoonTests, SendEntireFileMethods2) {
   //Client thread will send out request for 10 chunks and wait for resulting data.
   // Server should send 3 pieces of data.
   // Server then should see 10 requests in pipeline (3 - 13)
   // Server will then not send back data, so client will time out and die.

   int port = GetTcpPort();
   std::string location = GetTcpLocation(port);

   auto done = std::async(std::launch::async, &RecvThreadGetFileDie, &location);

   MockKraken server;
   server.MaxWaitInMs(500);
   
   Kraken::Spear spear = server.SetLocation(location);
   EXPECT_EQ(spear, Kraken::Spear::IMPALED);  

   std::vector<uint8_t> data = {10,20,30};
   for (int i = 0; i < 30; i++){
      Kraken::Battling Battling = server.SendTidalWave(data);
      EXPECT_EQ(Battling, Kraken::Battling::CONTINUE); 
   }

   Kraken::Battling Battling = server.FinalBreach();
   EXPECT_EQ(Battling, Kraken::Battling::CONTINUE); 
}


TEST_F(KrakenToHarpoonTests, KrakenSetBadLocation) {

   std::string location("bad location");

   MockKraken server;
   server.MaxWaitInMs(500);
   
   Kraken::Spear spear = server.SetLocation(location);
   EXPECT_EQ(Kraken::Spear::MISS, spear);  
}

