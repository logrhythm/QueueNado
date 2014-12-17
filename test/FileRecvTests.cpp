#include <czmq.h>
#include <thread>
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
   FileRecv::Socket status = client.SetLocation(location);

   EXPECT_EQ(status, FileRecv::Socket::OK);   
}

TEST_F(FileRecvTests, SendDataGetNextChunkIdMethods) {
   //Server will receive data requests from client, but will not respond to them.
   //  Client therefore will timeout:
   int port = GetTcpPort();
   std::string location = GetTcpLocation(port);
   zthread_new(FileRecvTests::SendThreadNextChunkIdDie, reinterpret_cast<void*>(&location));

   FileRecv client;
   client.SetTimeout(1000);

   FileRecv::Socket status = client.SetLocation(location);
   EXPECT_EQ(status, FileRecv::Socket::OK);   
   
   FileRecv::DataPacket p = FileRecv::DataPacketFactory();
   FileRecv::Stream res = client.Receive(p);

   EXPECT_EQ(res, FileRecv::Stream::TIMEOUT);
   EXPECT_EQ(p->size, 0);
   EXPECT_EQ(p->data, nullptr);
}

TEST_F(FileRecvTests, SendDataOneChunkReceivedMethods) {
   //Server will receive data requests from client and only send one chunk before quitting.
   //  Client therefore will timeout:
   int port = GetTcpPort();
   std::string location = GetTcpLocation(port);
   zthread_new(FileRecvTests::SendThreadSendOneDie, reinterpret_cast<void*>(&location));

   FileRecv client;
   client.SetTimeout(1000);
   
   FileRecv::Socket status = client.SetLocation(location);
   EXPECT_EQ(status, FileRecv::Socket::OK); 

   FileRecv::DataPacket p = FileRecv::DataPacketFactory();
   FileRecv::Stream res = client.Receive(p);

   EXPECT_EQ(res, FileRecv::Stream::CONTINUE);
   EXPECT_EQ(p->size, 3);
   EXPECT_EQ(p->data[0], 10);
   EXPECT_EQ(p->data[1], 20);
   EXPECT_EQ(p->data[2], 30);

   res = client.Receive(p);  
   EXPECT_EQ(res, FileRecv::Stream::TIMEOUT);
}

TEST_F(FileRecvTests, SendThirtyDataChunksReceivedMethods) {

   int port = GetTcpPort();
   std::string location = GetTcpLocation(port);
   zthread_new(FileRecvTests::SendThreadSendThirtyDie, reinterpret_cast<void*>(&location));

   FileRecv client;
   FileRecv::DataPacket p = FileRecv::DataPacketFactory();
   
   client.SetTimeout(1000);
   FileRecv::Socket status = client.SetLocation(location);
   EXPECT_EQ(status, FileRecv::Socket::OK); 

   for (int i = 0; i < 30; ++i){
      FileRecv::Stream res = client.Receive(p); 

      EXPECT_EQ(res, FileRecv::Stream::CONTINUE); 
      EXPECT_EQ(p->data[0], i);
      EXPECT_EQ(p->size, 1);
   }
   
   FileRecv::Stream res = client.Receive(p);  
   EXPECT_EQ(res, FileRecv::Stream::TIMEOUT);
   EXPECT_EQ(p->data, nullptr);
   EXPECT_EQ(p->size, 0);
}

TEST_F(FileRecvTests, SendThirtyTwoDataChunksReceivedMethods) {

   int port = GetTcpPort();
   std::string location = GetTcpLocation(port);
   zthread_new(FileRecvTests::SendThreadSendThirtyTwoEnd, reinterpret_cast<void*>(&location));

   FileRecv client;
   FileRecv::DataPacket p = FileRecv::DataPacketFactory();
   client.SetTimeout(1000);
   
   FileRecv::Socket status = client.SetLocation(location);
   EXPECT_EQ(status, FileRecv::Socket::OK); 

   for (int i = 0; i < 30; ++i){
      FileRecv::Stream res = client.Receive(p); 

      EXPECT_EQ(res, FileRecv::Stream::CONTINUE); 
      EXPECT_EQ(p->data[0], i);
      EXPECT_EQ(p->size, 1);
   }
   
   //Should now receive an empty chucnk to indicate end of stream:
   FileRecv::Stream res = client.Receive(p);  
   EXPECT_EQ(res, FileRecv::Stream::END_OF_STREAM);
   EXPECT_EQ(p->data, nullptr);
   EXPECT_EQ(p->size, 0);
}

