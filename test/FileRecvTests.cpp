#include <czmq.h>
#include <thread>
#include <boost/thread.hpp>
#include "FileRecvTests.h"
#include "MockFileSend.h"
#include "FileRecv.h"
#include "Death.h"

void* FileRecvTests::SendThreadNextChunkIdDie(void* arg) {
   std::string address = *(reinterpret_cast<std::string*>(arg));
   MockFileSend server;
   server.SetLocation(address);
   server.SetTimeout(1000);
   server.CallNextChunkId();
   return nullptr;
}

void* FileRecvTests::SendThreadSendOneDie(void* arg) {
   std::string address = *(reinterpret_cast<std::string*>(arg));
   uint8_t first[] = {10,20,30};

   MockFileSend server;
   server.SetLocation(address);
   server.SetTimeout(1000);
   server.SendData(first, 3);

   return nullptr;
}

void* FileRecvTests::SendThreadSendThirtyDie(void* arg) {
   std::string address = *(reinterpret_cast<std::string*>(arg));
   MockFileSend server;
   server.SetLocation(address);
   server.SetTimeout(1000);

   for (uint8_t i = 0; i < 30; i++) {
      server.SendData(&i, 1);
   }

   return nullptr;
}

void* FileRecvTests::SendThreadSendThirtyTwoEnd(void* arg) {
   std::string address = *(reinterpret_cast<std::string*>(arg));
   MockFileSend server;
   server.SetLocation(address);
   server.SetTimeout(1000);

   for (uint8_t i = 0; i < 30; i++) {
      server.SendData(&i, 1);
   }
   server.SendData(nullptr, 0);

   return nullptr;
}

int FileRecvTests::GetTcpPort() {
   int max = 15000;
   int min = 13000;
   return(rand()%(max-min))+min;
}

std::string FileRecvTests::GetTcpLocation(int port) {
   std::string tcpLocation("tcp://127.0.0.1:");
   tcpLocation.append(std::to_string(port));
   return tcpLocation;
}

std::string FileRecvTests::GetIpcLocation() {
   int pid = getpid();
   std::string ipcLocation("ipc:///tmp/");
   ipcLocation.append("FileRecvTests");
   ipcLocation.append(std::to_string(pid));
   ipcLocation.append(".ipc");
   return ipcLocation;
}

std::string FileRecvTests::GetInprocLocation() {
   int pid = getpid();
   std::string inprocLocation("inproc://FileRecvTests");
   inprocLocation.append(std::to_string(pid));
   return inprocLocation;
}

TEST_F(FileRecvTests, FileRecvSetLocation) {
   FileRecv client;

   int port = GetTcpPort();
   std::string location = FileRecvTests::GetTcpLocation(port);
   int status = client.SetLocation(location);

   EXPECT_EQ(status, 0);   
}

TEST_F(FileRecvTests, SendDataGetNextChunkIdMethods) {
   //Server will receive data requests from client, but will not respond to them.
   //  Client therefore will timeout:
   int port = GetTcpPort();
   std::string location = GetTcpLocation(port);
   zthread_new(FileRecvTests::SendThreadNextChunkIdDie, reinterpret_cast<void*>(&location));

   FileRecv client;
   client.SetTimeout(1000);
   int status = client.SetLocation(location);
   EXPECT_EQ(status, 0);
   
   status = client.Monitor();
   size_t size = client.GetChunkSize();
   uint8_t* data = client.GetChunkData();

   EXPECT_EQ(status, -2);
   EXPECT_EQ(size, 0);
   EXPECT_EQ(data, nullptr);
}

TEST_F(FileRecvTests, SendDataOneChunkReceivedMethods) {
   //Server will receive data requests from client and only send one chunk before quitting.
   //  Client therefore will timeout:
   int port = GetTcpPort();
   std::string location = GetTcpLocation(port);
   zthread_new(FileRecvTests::SendThreadSendOneDie, reinterpret_cast<void*>(&location));

   FileRecv client;
   client.SetTimeout(1000);
   int status = client.SetLocation(location);
   EXPECT_EQ(status, 0);
   
   status = client.Monitor();
   size_t size = client.GetChunkSize();
   uint8_t* data = client.GetChunkData();

   EXPECT_EQ(status, 3);
   EXPECT_EQ(size, 3);
   EXPECT_EQ(data[0], 10);
   EXPECT_EQ(data[1], 20);
   EXPECT_EQ(data[2], 30);

   status = client.Monitor(); 
   EXPECT_EQ(status, -2);
}

TEST_F(FileRecvTests, SendThirtyDataChunksReceivedMethods) {

   int port = GetTcpPort();
   std::string location = GetTcpLocation(port);
   zthread_new(FileRecvTests::SendThreadSendThirtyDie, reinterpret_cast<void*>(&location));

   FileRecv client;
   client.SetTimeout(1000);
   int status = client.SetLocation(location);
   EXPECT_EQ(status, 0);

   for (int i = 0; i < 30; ++i){
      status = client.Monitor(); 
      size_t size = client.GetChunkSize();
      uint8_t* data = client.GetChunkData();

      EXPECT_EQ(status, 1); 
      EXPECT_EQ(data[0], i);
      EXPECT_EQ(size, 1);
   }
   
   status = client.Monitor(); 
   EXPECT_EQ(status, -2);
}

TEST_F(FileRecvTests, SendThirtyTwoDataChunksReceivedMethods) {

   int port = GetTcpPort();
   std::string location = GetTcpLocation(port);
   zthread_new(FileRecvTests::SendThreadSendThirtyTwoEnd, reinterpret_cast<void*>(&location));

   FileRecv client;
   client.SetTimeout(1000);
   int status = client.SetLocation(location);
   EXPECT_EQ(status, 0);

   for (int i = 0; i < 30; ++i){
      status = client.Monitor(); 
      size_t size = client.GetChunkSize();
      uint8_t* data = client.GetChunkData();

      EXPECT_EQ(status, 1); 
      EXPECT_EQ(data[0], i);
      EXPECT_EQ(size, 1);
   }
   
   //Should now receive an empty chucnk to indicate end of stream:
   status = client.Monitor(); 
   uint8_t* data = client.GetChunkData();
   size_t size = client.GetChunkSize();

   EXPECT_EQ(status, 0);
   EXPECT_EQ(size, 0);
   EXPECT_EQ(data, nullptr);
}

