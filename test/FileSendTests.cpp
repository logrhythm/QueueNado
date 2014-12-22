#include <czmq.h>
#include <thread>
#include "FileSendTests.h"
#include "MockFileRecv.h"
#include "MockFileSend.h"
#include "FileSend.h"
#include "Death.h"

void* FileSendTests::RecvThreadNextChunkIdDie(void* arg) {
   std::string address = *(reinterpret_cast<std::string*>(arg));
   MockFileRecv client;
   client.SetLocation(address);
   client.SetTimeout(500);
   client.CallRequestChunks();
   return nullptr;
}

void* FileSendTests::RecvThreadNextChunkIdWait(void* arg) {
   std::string address = *(reinterpret_cast<std::string*>(arg));
   std::vector<uint8_t> p;
   MockFileRecv client;
   client.SetLocation(address);
   client.SetTimeout(500);
   client.Receive(p);
   return nullptr;
}

void* FileSendTests::RecvThreadGetThreeWait(void* arg) {
   std::string address = *(reinterpret_cast<std::string*>(arg));

   std::vector<uint8_t> p;
   MockFileRecv client;
   client.SetLocation(address);
   client.SetTimeout(500);
   client.Receive(p);
   client.Receive(p);
   client.Receive(p);
   client.Receive(p);
   return nullptr;
}

void* FileSendTests::RecvThreadGetFileDie(void* arg) {
   std::string address = *(reinterpret_cast<std::string*>(arg));
   
   std::vector<uint8_t> p;
   MockFileRecv client;
   client.SetTimeout(500);
   client.SetLocation(address);

   while(FileRecv::Stream::CONTINUE == client.Receive(p)){

   }
   return nullptr;
}

int FileSendTests::GetTcpPort() {
   int max = 15000;
   int min = 13000;
   return(rand()%(max-min))+min;
}

std::string FileSendTests::GetTcpLocation(int port) {
   std::string tcpLocation("tcp://127.0.0.1:");
   tcpLocation.append(std::to_string(port));
   return tcpLocation;
}

TEST_F(FileSendTests, FileSendSetLocation) {
   FileSend server;

   int port = GetTcpPort();
   std::string location = FileSendTests::GetTcpLocation(port);

   FileSend::Socket socket = server.SetLocation(location);
   EXPECT_EQ(socket, FileSend::Socket::OK);  
}

TEST_F(FileSendTests, SendDataGetNextChunkIdDieMethods) {
   //Client thread will send out request for 10 chunks and instantly die. Therefore, none of those
   // chunks will be received and a -2 status should be returned
   int port = GetTcpPort();
   std::string location = GetTcpLocation(port);

   zthread_new(FileSendTests::RecvThreadNextChunkIdDie, reinterpret_cast<void*>(&location));

   MockFileSend server;
   server.SetTimeout(500);

   FileSend::Socket socket = server.SetLocation(location);
   EXPECT_EQ(socket, FileSend::Socket::OK);    
   
   for(int i = 0; i < 10; i++){
      FileSend::Stream stream = server.CallNextChunkId(); 
      EXPECT_EQ(stream, FileSend::Stream::TIMEOUT);  
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
   server.SetTimeout(500);
   
   FileSend::Socket socket = server.SetLocation(location);
   EXPECT_EQ(socket, FileSend::Socket::OK);   
   
   for(int i = 0; i < 10; i++){
      FileSend::Stream stream = server.CallNextChunkId(); 
      EXPECT_EQ(stream, FileSend::Stream::CONTINUE);  
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
   server.SetTimeout(500);
   
   FileSend::Socket socket = server.SetLocation(location);
   EXPECT_EQ(socket, FileSend::Socket::OK);  

   std::vector<uint8_t> data = {10,20,30};
   FileSend::Stream stream = server.SendData(data);
   EXPECT_EQ(stream, FileSend::Stream::CONTINUE);
   stream = server.SendData(data);
   EXPECT_EQ(stream, FileSend::Stream::CONTINUE);
   stream = server.SendData(data);
   EXPECT_EQ(stream, FileSend::Stream::CONTINUE);

   for(int i = 0; i < 10; i++){
      stream = server.CallNextChunkId(); 
      EXPECT_EQ(stream, FileSend::Stream::CONTINUE);
   }

   stream = server.CallNextChunkId(); 
   EXPECT_EQ(stream, FileSend::Stream::TIMEOUT);   
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
   server.SetTimeout(500);
   
   FileSend::Socket socket = server.SetLocation(location);
   EXPECT_EQ(socket, FileSend::Socket::OK);  

   std::vector<uint8_t> data = {10,20,30};
   for (int i = 0; i < 30; i++){
      FileSend::Stream stream = server.SendData(data);
      EXPECT_EQ(stream, FileSend::Stream::CONTINUE); 
   }

   FileSend::Stream stream = server.SendFinal();
   EXPECT_EQ(stream, FileSend::Stream::CONTINUE); 
}