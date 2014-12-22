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
   std::vector<uint8_t> v = {10,20,30};

   MockFileSend server;
   server.SetLocation(address);
   server.SetTimeout(1000);
   server.SendData(v);

   return nullptr;
}

void* FileRecvTests::SendThreadSendThirtyDie(void* arg) {
   std::string address = *(reinterpret_cast<std::string*>(arg));
   MockFileSend server;
   server.SetLocation(address);
   server.SetTimeout(1000);

   for (uint8_t i = 0; i < 30; i++) {
      server.SendData({i});
   }

   return nullptr;
}

void* FileRecvTests::SendThreadSendThirtyTwoEnd(void* arg) {
   std::string address = *(reinterpret_cast<std::string*>(arg));
   MockFileSend server;
   server.SetLocation(address);
   server.SetTimeout(1000);

   for (uint8_t i = 0; i < 30; i++) {
      server.SendData({i});
   }
   server.SendFinal();

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
   
   std::vector<uint8_t> p;
   FileRecv::Stream res = client.Receive(p);

   EXPECT_EQ(res, FileRecv::Stream::TIMEOUT);
   EXPECT_EQ(p.size(), 0);
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

   std::vector<uint8_t> p;
   FileRecv::Stream res = client.Receive(p);

   EXPECT_EQ(res, FileRecv::Stream::CONTINUE);
   ASSERT_EQ(p.size(), 3);
   EXPECT_EQ(p[0], 10);
   EXPECT_EQ(p[1], 20);
   EXPECT_EQ(p[2], 30);

   res = client.Receive(p);  
   EXPECT_EQ(res, FileRecv::Stream::TIMEOUT);
}

TEST_F(FileRecvTests, SendThirtyDataChunksReceivedMethods) {

   int port = GetTcpPort();
   std::string location = GetTcpLocation(port);
   zthread_new(FileRecvTests::SendThreadSendThirtyDie, reinterpret_cast<void*>(&location));

   FileRecv client;
   std::vector<uint8_t> p;
   
   client.SetTimeout(1000);
   FileRecv::Socket status = client.SetLocation(location);
   EXPECT_EQ(status, FileRecv::Socket::OK); 

   for (int i = 0; i < 30; ++i){
      FileRecv::Stream res = client.Receive(p); 

      EXPECT_EQ(res, FileRecv::Stream::CONTINUE); 
      ASSERT_EQ(p.size(), 1);
      EXPECT_EQ(p[0], i);
   }
   
   FileRecv::Stream res = client.Receive(p);  
   EXPECT_EQ(res, FileRecv::Stream::TIMEOUT);
   EXPECT_EQ(p.size(), 0);
}

TEST_F(FileRecvTests, SendThirtyTwoDataChunksReceivedMethods) {

   int port = GetTcpPort();
   std::string location = GetTcpLocation(port);
   zthread_new(FileRecvTests::SendThreadSendThirtyTwoEnd, reinterpret_cast<void*>(&location));

   FileRecv client;
   std::vector<uint8_t> p;
   client.SetTimeout(1000);
   
   FileRecv::Socket status = client.SetLocation(location);
   EXPECT_EQ(status, FileRecv::Socket::OK); 

   for (int i = 0; i < 30; ++i){
      FileRecv::Stream res = client.Receive(p); 

      EXPECT_EQ(res, FileRecv::Stream::CONTINUE); 
      ASSERT_EQ(p.size(), 1);
      EXPECT_EQ(p[0], i);
   }
   
   //Should now receive an empty chucnk to indicate end of stream:
   FileRecv::Stream res = client.Receive(p);  
   EXPECT_EQ(res, FileRecv::Stream::END_OF_STREAM);
   EXPECT_EQ(p.size(), 0);
}

