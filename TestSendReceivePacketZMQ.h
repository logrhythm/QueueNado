#pragma once

#include "SendPacketZMQ.h"
#include "ReceivePacketZMQ.h"
#include "ZeroMQTests.h"
#include "gtest/gtest.h"
#include <pthread.h>
#include <sys/time.h>
#include <iostream>
#include "boost/thread/locks.hpp"
#include "boost/thread/recursive_mutex.hpp"

class PacketReaderThreadConfig {
public:

   PacketReaderThreadConfig() :
   clientThread(NULL), serverThread(NULL), totalPackets(0), ready(NULL) {
   }
   std::string binding;
   SendPacketZMQ* clientThread;
   ReceivePacketZMQ * serverThread;
   int totalPackets;
   volatile bool* ready;
};

class PacketThreadConfig {
public:

   PacketThreadConfig() :
   thread(NULL), serverThread(NULL), totalPackets(0), ready(NULL),
   dataSize(0), bigData(NULL) {
   }
   std::string binding;
   SendPacketZMQ* thread;
   ReceivePacketZMQ * serverThread;
   int totalPackets;
   bool* ready;
   size_t dataSize;
   u_char* bigData;
};

class TestSendReceivePacketZMQ : public ZeroMQTests {
public:
   void DummyReaderThread(PacketReaderThreadConfig *config);
   void DummySenderThread(PacketThreadConfig *config);
   bool RunBenchmark(size_t packets, const std::string& binding, size_t expectedSpeed);

   virtual void SetUp() {
      srandom((unsigned) time(NULL));
      std::stringstream makeATarget;
      makeATarget << "ipc:///tmp/ipc.test" << boost::this_thread::get_id();
      mTarget = makeATarget.str();
      zctx_interrupted = false;
   }

   virtual void TearDown() {
   }
   std::string mTarget;
};