#include <czmq.h>
#include <thread>
#include <boost/thread.hpp>
#include <boost/lexical_cast.hpp>
#include "FileSendTests.h"
#include "MockFileRecv.h"
#include "MockFileSend.h"
#include "FileSend.h"
#include "Death.h"

void* FileSendTests::RecvThreadNextChunkIdDie(void* arg) {
   std::string address = *(reinterpret_cast<std::string*>(arg));
   MockFileRecv *client = new MockFileRecv();
   client->SetLocation(address);
   if (client->SetLocation(address) < 0){
      std::cout << "Location unable to set" << std::endl;
      delete client;
      return nullptr;
   }
   client->SetTimeout(500);
   client->CallRequestChunks();
   delete client;
   return nullptr;
}

void* FileSendTests::RecvThreadNextChunkIdWait(void* arg) {
   std::string address = *(reinterpret_cast<std::string*>(arg));
   MockFileRecv *client = new MockFileRecv();
   client->SetLocation(address);
   if (client->SetLocation(address) < 0){
      std::cout << "Location unable to set" << std::endl;
      delete client;
      return nullptr;
   }
   client->SetTimeout(500);
   client->Monitor();
   delete client;
   return nullptr;
}

void* FileSendTests::RecvThreadGetThreeWait(void* arg) {
   std::string address = *(reinterpret_cast<std::string*>(arg));

   MockFileRecv *client = new MockFileRecv();
   client->SetLocation(address);
   if (client->SetLocation(address) < 0){
      std::cout << "Location unable to set" << std::endl;
      delete client;
      return nullptr;
   }
   client->SetTimeout(500);
   client->Monitor();
   client->Monitor();
   client->Monitor();
   client->Monitor();

   delete client;
   return nullptr;
}

void* FileSendTests::RecvThreadGetFileDie(void* arg) {
   std::string address = *(reinterpret_cast<std::string*>(arg));
   
   MockFileRecv *client = new MockFileRecv();
   if (client->SetLocation(address) < 0){
      std::cout << "Location unable to set" << std::endl;
      delete client;
      return nullptr;
   }
   client->SetTimeout(500);

   while(client->Monitor() > 0){

   }
   delete client;
   return nullptr;
}

int FileSendTests::GetTcpPort() {
   int max = 15000;
   int min = 13000;
   return(rand()%(max-min))+min;
}

std::string FileSendTests::GetTcpLocation(int port) {
   std::string tcpLocation("tcp://127.0.0.1:");
   tcpLocation.append(boost::lexical_cast<std::string > (port));
   return tcpLocation;
}

std::string FileSendTests::GetIpcLocation() {
   int pid = getpid();
   std::string ipcLocation("ipc:///tmp/");
   ipcLocation.append("FileSendTests");
   ipcLocation.append(boost::lexical_cast<std::string > (pid));
   ipcLocation.append(".ipc");
   return ipcLocation;
}

std::string FileSendTests::GetInprocLocation() {
   int pid = getpid();
   std::string inprocLocation("inproc://FileSendTests");
   inprocLocation.append(boost::lexical_cast<std::string > (pid));
   return inprocLocation;
}

TEST_F(FileSendTests, FileSendSetLocation) {
   FileSend server;

   int port = GetTcpPort();
   std::string location = FileSendTests::GetTcpLocation(port);
   int status = server.SetLocation(location);

   EXPECT_EQ(status, port);   
}

TEST_F(FileSendTests, SendDataGetNextChunkIdDieMethods) {
   //Client thread will send out request for 10 chunks and instantly die. Therefore, none of those
   // chunks will be received and a -2 status should be returned
   int port = GetTcpPort();
   std::string location = GetTcpLocation(port);

   zthread_new(FileSendTests::RecvThreadNextChunkIdDie, reinterpret_cast<void*>(&location));

   MockFileSend server;
   int status = server.SetLocation(location);
   server.SetTimeout(500);
   EXPECT_EQ(status, port);
   
   for(int i = 0; i < 10; i++){
      int nextChunk = server.CallNextChunkId(); 
      EXPECT_EQ(nextChunk, -2);  
   }
}

TEST_F(FileSendTests, SendDataGetNextChunkIdWaitMethods) {
   //Client thread will send out request for 10 chunks and wait for resulting data.
   // Server should receive each of those chunks.
   // Server will not send back data, so client will time out and die.

   int port = GetTcpPort();
   std::string location = GetTcpLocation(port);

   zthread_new(FileSendTests::RecvThreadNextChunkIdWait, reinterpret_cast<void*>(&location));

   MockFileSend server;
   int status = server.SetLocation(location);
   server.SetTimeout(500);
   EXPECT_EQ(status, port);
   
   for(int i = 0; i < 10; i++){
      int nextChunk = server.CallNextChunkId(); 
      EXPECT_TRUE(nextChunk >= 0);  
   }
}

TEST_F(FileSendTests, SendDataGetResultingWaitsMethods) {
   //Client thread will send out request for 10 chunks and wait for resulting data.
   // Server should send 3 pieces of data.
   // Server then should see 10 requests in pipeline (3 - 13)
   // Server will then not send back data, so client will time out and die.

   int port = GetTcpPort();
   std::string location = GetTcpLocation(port);

   zthread_new(FileSendTests::RecvThreadGetThreeWait, reinterpret_cast<void*>(&location));

   MockFileSend server;
   int status = server.SetLocation(location);
   server.SetTimeout(500);
   EXPECT_EQ(status, port);

   uint8_t data[] = {10,20,30};
   status = server.SendData(data, 3);
   EXPECT_EQ(status, 0);
   status = server.SendData(data, 3);
   EXPECT_EQ(status, 0);
   status = server.SendData(data, 3);
   EXPECT_EQ(status, 0);

   for(int i = 0; i < 10; i++){
      int nextChunk = server.CallNextChunkId(); 
      EXPECT_TRUE(nextChunk >= 0);  
   }

   int nextChunk = server.CallNextChunkId(); 
   EXPECT_EQ(nextChunk, -2);  
}

TEST_F(FileSendTests, SendEntireFileMethods2) {
   //Client thread will send out request for 10 chunks and wait for resulting data.
   // Server should send 3 pieces of data.
   // Server then should see 10 requests in pipeline (3 - 13)
   // Server will then not send back data, so client will time out and die.

   int port = GetTcpPort();
   std::string location = GetTcpLocation(port);

   zthread_new(FileSendTests::RecvThreadGetFileDie, reinterpret_cast<void*>(&location));

   MockFileSend server;
   int status = server.SetLocation(location);
   server.SetTimeout(500);
   EXPECT_EQ(status, port);

   uint8_t data[] = {10,20,30};
   for (int i = 0; i < 30; i++){
      int status = server.SendData(data, 3);
      EXPECT_EQ(status, 0);
   }

   status = server.SendData(nullptr, 0);
   EXPECT_EQ(status, 0);
}