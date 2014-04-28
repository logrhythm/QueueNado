
#include "zlib.h"
#include <time.h>
#include "boost/thread.hpp"

#include "ZeroMQTests.h"
#include "MockZeroMQ.h"

using namespace std;
int ZeroMQTests::gCurrentPacketSize(0);





TEST_F(ZeroMQTests, ZeroCopyPacketBroadcasterConstruct) {
#ifdef LR_DEBUG
   MockZeroMQPacket fastCopyQueue(1);
   ASSERT_FALSE(fastCopyQueue.ContextSet());
   ASSERT_FALSE(fastCopyQueue.SocketSet());
   ASSERT_TRUE(fastCopyQueue.Initialize());
   ASSERT_TRUE(fastCopyQueue.ContextSet());
   ASSERT_TRUE(fastCopyQueue.SocketSet());
   ASSERT_TRUE(fastCopyQueue.GetPointer(1) == NULL);
   ASSERT_FALSE(fastCopyQueue.SendClientReady());

   int foo;
   MockZeroMQPacket fastCopyQueueCopy(fastCopyQueue);
   ASSERT_TRUE(fastCopyQueueCopy.ContextSet());
   ASSERT_FALSE(fastCopyQueueCopy.SocketSet());
   ASSERT_TRUE(fastCopyQueueCopy.Initialize());
   ASSERT_TRUE(fastCopyQueueCopy.SocketSet());
   ASSERT_FALSE(fastCopyQueueCopy.SendPointer(&foo));
   ASSERT_FALSE(fastCopyQueueCopy.WaitForClient(1));

   EXPECT_EQ(1, fastCopyQueue.GetId());
   EXPECT_EQ(1, fastCopyQueueCopy.GetId());

   stringstream expectedBinding1;
   expectedBinding1 << "inproc://voidstar_" << getpid() << "_" << 1;
   EXPECT_EQ(expectedBinding1.str(), fastCopyQueue.GetBinding());
   EXPECT_EQ(expectedBinding1.str(), fastCopyQueueCopy.GetBinding());

   MockZeroMQPacket* pQueue = new MockZeroMQPacket(2);
   ASSERT_FALSE(pQueue->ContextSet());
   ASSERT_FALSE(pQueue->SocketSet());
   ASSERT_TRUE(pQueue->Initialize());
   ASSERT_TRUE(pQueue->ContextSet());
   ASSERT_TRUE(pQueue->SocketSet());
   MockZeroMQPacket* p2Queue = new MockZeroMQPacket(*pQueue);
   ASSERT_TRUE(p2Queue->ContextSet());
   ASSERT_FALSE(p2Queue->SocketSet());
   ASSERT_TRUE(p2Queue->Initialize());
   ASSERT_TRUE(p2Queue->SocketSet());

   EXPECT_EQ(2, pQueue->GetId());
   EXPECT_EQ(2, p2Queue->GetId());

   delete p2Queue;
   delete pQueue;

#endif
}

TEST_F(ZeroMQTests, ZeroCopyPacketBroadcasterConstructBadly) {
#ifdef LR_DEBUG
   MockZeroMQPacket* pFastCopyQueue = new MockZeroMQPacket(2);
   ASSERT_FALSE(pFastCopyQueue->ContextSet());
   MockZeroMQPacket* pFastCopyQueueCopy = new MockZeroMQPacket(pFastCopyQueue);
   ASSERT_FALSE(pFastCopyQueueCopy->ContextSet());
   ASSERT_FALSE(pFastCopyQueueCopy->SocketSet());
   ASSERT_FALSE(pFastCopyQueueCopy->Initialize());
   ASSERT_FALSE(pFastCopyQueueCopy->ContextSet());
   ASSERT_FALSE(pFastCopyQueueCopy->SocketSet());

   delete pFastCopyQueueCopy;
   delete pFastCopyQueue;
#endif
}

TEST_F(ZeroMQTests, ZeroCopyPacketBroadcasterGetHighWater) {
#ifdef LR_DEBUG
   MockZeroMQPacket fastCopyQueue(1);
   ASSERT_TRUE(fastCopyQueue.GetHighWater() > 0);
#endif
}

TEST_F(ZeroMQTests, ZeroCopyPacketBroadcasterGetContext) {
#ifdef LR_DEBUG
   MockZeroMQPacket fastCopyQueue(1);
   void* context;

   ASSERT_FALSE((context = fastCopyQueue.GetContext()) == NULL);
   ASSERT_FALSE(zmq_term(context) != 0);
#endif
}

TEST_F(ZeroMQTests, ZeroCopyPacketBroadcasterGetSocket) {
#ifdef LR_DEBUG
   MockZeroMQPacket fastCopyQueue(1);
   void* context;
   void* socket;

   ASSERT_TRUE((socket = fastCopyQueue.GetSocket(NULL)) == NULL);

   ASSERT_FALSE((context = fastCopyQueue.GetContext()) == NULL);
   ASSERT_FALSE((socket = fastCopyQueue.GetSocket(context)) == NULL);

   ASSERT_FALSE(zmq_close(socket) != 0);
   ASSERT_FALSE(zmq_term(context) != 0);
#endif
}

TEST_F(ZeroMQTests, ZeroCopyPacketBroadcasterServerSetup) {
#ifdef LR_DEBUG
   MockZeroMQPacket fastCopyQueue(1);
   void* context;
   ASSERT_FALSE((context = fastCopyQueue.GetContext()) == NULL);

   stringstream binding;
   binding << "inproc://foo_" << getpid();
   string emptyString;
   void* socket = NULL;

   fastCopyQueue.ServerSetup(binding.str(), socket);
   ASSERT_TRUE(socket == NULL);

   ASSERT_FALSE((socket = fastCopyQueue.GetSocket(context)) == NULL);
   fastCopyQueue.ServerSetup(emptyString, socket);
   ASSERT_TRUE(socket == NULL);

   ASSERT_FALSE((socket = fastCopyQueue.GetSocket(context)) == NULL);
   fastCopyQueue.ServerSetup(binding.str(), socket);
   ASSERT_TRUE(socket != NULL);

   ASSERT_FALSE(zmq_close(socket) != 0);
   ASSERT_FALSE(zmq_term(context) != 0);
#endif
}

TEST_F(ZeroMQTests, ZeroCopyPacketBroadcasterClientSetup) {
#ifdef LR_DEBUG
   MockZeroMQPacket fastCopyQueue(1);
   void* context;
   ASSERT_FALSE((context = fastCopyQueue.GetContext()) == NULL);

   stringstream binding;
   binding << "inproc://foo_" << getpid();
   string emptyString;
   void* socket = NULL;

   fastCopyQueue.ClientSetup(binding.str(), socket);
   ASSERT_TRUE(socket == NULL);

   ASSERT_FALSE((socket = fastCopyQueue.GetSocket(context)) == NULL);
   fastCopyQueue.ClientSetup(emptyString, socket);
   ASSERT_TRUE(socket == NULL);

// the new versions of ZMQ have fixed the inproc connection order issue making order arbitrary
//   ASSERT_FALSE((socket = fastCopyQueue.GetSocket(context)) == NULL);
//   fastCopyQueue.ClientSetup(binding.str(), socket);
//   ASSERT_TRUE(socket == NULL);

   void* serverSocket = NULL;
   ASSERT_FALSE((serverSocket = fastCopyQueue.GetSocket(context)) == NULL);
   fastCopyQueue.ServerSetup(binding.str(), serverSocket);
   ASSERT_TRUE(serverSocket != NULL);

   ASSERT_FALSE((socket = fastCopyQueue.GetSocket(context)) == NULL);
   fastCopyQueue.ClientSetup(binding.str(), socket);
   ASSERT_TRUE(socket != NULL);

   ASSERT_FALSE(zmq_close(socket) != 0);
   ASSERT_FALSE(zmq_close(serverSocket) != 0);
   ASSERT_FALSE(zmq_term(context) != 0);
#endif
}

TEST_F(ZeroMQTests, ZeroCopyPacketBroadcasterSendReadyFromClientToServer) {
#ifdef LR_DEBUG
   MockZeroMQPacket serverQueue(1);
   ASSERT_TRUE(serverQueue.Initialize());
   MockZeroMQPacket clientQueue(serverQueue);

   boost::thread clientThread = boost::thread(&ZeroMQTests::PacketBroadcasterReceiver, this, &clientQueue, (ctb_ppacket) NULL);
   ASSERT_TRUE(serverQueue.WaitForClient(20000));
   ASSERT_TRUE(clientQueue.SocketSet());
   clientThread.interrupt();
   clientThread.join();
#endif
}

TEST_F(ZeroMQTests, ZeroCopyPacketBroadcasterSendDataToReadyClient) {
#ifdef LR_DEBUG
   MockZeroMQPacket serverQueue(1);
   ASSERT_TRUE(serverQueue.Initialize());
   MockZeroMQPacket clientQueue(serverQueue);

   ctb_ppacket packet = (ctb_ppacket) malloc(sizeof (ctb_pkt));

   boost::thread clientThread = boost::thread(&ZeroMQTests::PacketBroadcasterReceiver, this, &clientQueue, packet);
   ASSERT_TRUE(serverQueue.WaitForClient(20000));
   ASSERT_TRUE(clientQueue.SocketSet());
   mPacketsToTest = 1;
   ASSERT_TRUE(serverQueue.SendPointer(packet));

   int timeSleeping(0);
   int sleepInterval(1000);
   clientThread.join();

   free(packet);
#endif
}

TEST_F(ZeroMQTests, ConcequencesOfPacketInitFailureOnPointerQueue) {
#ifdef LR_DEBUG
   MockZeroMQPacket mockServer(1);
   mockServer.Initialize();
   MockZeroMQPacket mockClient(mockServer);

   mockServer.mFailsToInitMsg = true;
   mockClient.mFailsToInitMsg = true;

   ASSERT_FALSE(mockServer.WaitForClient(1));
   ASSERT_TRUE(mockClient.GetPointer(1) == NULL);

   ASSERT_FALSE(mockClient.SendClientReady());

#endif
}

TEST_F(ZeroMQTests, ZeroCopyPacketBroadcasterSendDataToReadyClientPerformanceTest) {
   ZeroMQ<void*> serverQueue(1);
   ASSERT_TRUE(serverQueue.Initialize());
   ZeroMQ<void*> clientQueue(serverQueue);

   ctb_ppacket packet = (ctb_ppacket) malloc(sizeof (ctb_pkt));
   mPacketsToTest = PACKETS_TO_TEST << 2;
   boost::thread clientThread = boost::thread(
           &ZeroMQTests::PacketBroadcasterReceiver, this, &clientQueue, packet);
   ASSERT_TRUE(serverQueue.WaitForClient(20000));

   SetExpectedTime(mPacketsToTest, sizeof (ctb_pkt), 300, 500000L);

   int packetsSent(0);
   StartTimedSection();
   while (packetsSent++ < mPacketsToTest) {
      ASSERT_TRUE(serverQueue.SendPointer(packet));
   }
   clientThread.join();
   EndTimedSection();
   bool success = TimedSectionPassed();
   if (PACKETS_TO_TEST > 1000) {
      EXPECT_TRUE(success);
   }
   clientThread.interrupt();
   clientThread.join();
   free(packet);
}

TEST_F(ZeroMQTests, ConcequencesOfFailedSetSendHWMOnPointerQueue) {
#ifdef LR_DEBUG
   MockZeroMQPacket mockServer(1);
   mockServer.mFailsToSetSendHWM = true;

   EXPECT_FALSE(mockServer.Initialize());
   MockZeroMQPacket mockClient(mockServer);
   mockClient.mFailsToSetSendHWM = true;
   EXPECT_FALSE(mockClient.Initialize());

#endif
}

TEST_F(ZeroMQTests, ConcequencesOfFailedSetReceiveHWMOnPointerQueue) {
#ifdef LR_DEBUG

   MockZeroMQPacket mockServer(1);
   mockServer.mFailsToSetReceiveHWM = true;

   EXPECT_FALSE(mockServer.Initialize());
   MockZeroMQPacket mockClient(mockServer);
   mockClient.mFailsToSetReceiveHWM = true;
   EXPECT_FALSE(mockClient.Initialize());
#endif
}
