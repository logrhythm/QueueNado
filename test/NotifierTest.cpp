#include "NotifierTestHelper.h"
#include "NotifierTest.h"
#include <FileIO.h>
#include <Shotgun.h>
#include <Rifle.h>
#include <Alien.h>
#include <Vampire.h>
#include "Listener.h"
#include "Notifier.h"
#include <StopWatch.h>

namespace {

const std::string notifierQueue = "ipc:///tmp/RestartServicesQueue.ipc";
const std::string notifierQueuePath = "/tmp/RestartServicesQueue.ipc";
const std::string handshakeQueue = "ipc:///tmp/RestartServicesHandshakeQueue.ipc";
const std::string handshakeQueuePath = "/tmp/RestartServicesHandshakeQueue.ipc";

void* ReceiverThread(void* args) {
   TestThreadData* tmpRawPtr = reinterpret_cast<TestThreadData*>(args);
   CHECK(tmpRawPtr != nullptr);
   TestThreadData threadData(*tmpRawPtr);
   Listener listener(notifierQueue, handshakeQueue, "TestThread");
   std::shared_ptr<void*> raiiExitFlag(nullptr, [threadData](void*) {
      threadData.hasExited->store(true);
      LOG(INFO) << "exiting thread: " << threadData.hasExited->load();
   });

   EXPECT_TRUE(listener.Initialize());
   NotifyParentThatChildHasStarted(threadData);
   StopWatch timer;

   bool confirmed = false;
   while (ParentHasNotSentExitSignal(threadData)) {
      if (MaxTimeoutHasOccurred(timer)) {
         break;
      }
      if (listener.NotificationReceived()) {
         UpdateThreadDataAfterReceivingMessage(threadData);
         if (threadData.kExpectedFeedback > 0) {
            confirmed = listener.SendConfirmation();
            if (!confirmed) {
               ADD_FAILURE() << "Failed to send feedback, thread #" << std::this_thread::get_id();
            }
         }
      }
   }
   const bool timeout = MaxTimeoutHasOccurred(timer);
   LOG(INFO) << "Exiting loop. Parent stated: keep running: " << threadData.keepRunning->load() << ", timout: " << timeout;
   if (timeout) {
      ADD_FAILURE() << "Receiver thread has timed out after " << kMaxWaitTimeInSec << " seconds.";
   }
   LOG(INFO) << threadData.print();
   return nullptr;
}

void* SenderThread(void* args) {
   Notifier notifier(notifierQueue, handshakeQueue);
   TestThreadData* tmpRawPtr = reinterpret_cast<TestThreadData*>(args);
   CHECK(tmpRawPtr != nullptr);
   TestThreadData threadData(*tmpRawPtr);
   std::shared_ptr<void*> raiiExitFlag(nullptr, [threadData](void*) { threadData.hasExited->store(true); });
   EXPECT_TRUE(notifier.Initialize(threadData.kExpectedFeedback));
   NotifyParentThatChildHasStarted(threadData);

   while (ParentHasNotSentExitSignal(threadData)) {
      if (TimeToSendANotification(threadData)) {
         EXPECT_EQ(notifier.Notify(), threadData.kExpectedFeedback);
         ResetNotifyFlag(threadData);
      }
   }
   LOG(INFO) << threadData.print();
   return nullptr;
}
} //namespace

TEST_F(NotifierTest, InitializationCreatesIPC) {
   EXPECT_FALSE(FileIO::DoesFileExist(notifierQueuePath));
   EXPECT_FALSE(FileIO::DoesFileExist(handshakeQueuePath));
   Notifier notifier(notifierQueue, handshakeQueue);
   EXPECT_TRUE(notifier.Initialize(0));
   EXPECT_TRUE(FileIO::DoesFileExist(notifierQueuePath));
   EXPECT_TRUE(FileIO::DoesFileExist(handshakeQueuePath));
}

TEST_F(NotifierTest, 1Message1Receiver_noPingBack) {
   TestThreadData senderData("sender");
   TestThreadData receiver1ThreadData("receiver");
   EXPECT_FALSE(FileIO::DoesFileExist(notifierQueuePath));
   EXPECT_FALSE(FileIO::DoesFileExist(handshakeQueuePath));

   // Spawn Sender and wait for it to start up
   zthread_new(&SenderThread, reinterpret_cast<TestThreadData*>(&senderData));
   EXPECT_TRUE(SleepUntilCondition({{senderData.hasStarted}}));
   EXPECT_TRUE(FileIO::DoesFileExist(notifierQueuePath));
   // Spawn Receiver and wait for it to start up
   zthread_new(&ReceiverThread, reinterpret_cast<TestThreadData*>(&receiver1ThreadData));
   EXPECT_TRUE(SleepUntilCondition({{receiver1ThreadData.hasStarted}}));

   // First Exchange
   FireOffANotification(senderData);
   EXPECT_TRUE(SleepUntilCondition({{receiver1ThreadData.received}}));
   EXPECT_TRUE(receiver1ThreadData.received->load());

   // Shutdown everything
   ShutdownThreads({{senderData}, {receiver1ThreadData}});
   EXPECT_TRUE(SleepUntilCondition({{senderData.hasExited}, {receiver1ThreadData.hasExited}}));
   EXPECT_TRUE(ThreadIsShutdown(receiver1ThreadData));
   EXPECT_TRUE(ThreadIsShutdown(senderData));
}

TEST_F(NotifierTest, 1Message2ReceiversUsingSenderandReceiverThreads) {
   TestThreadData senderData("sender");
   TestThreadData receiver1ThreadData("receiver1");
   TestThreadData receiver2ThreadData("receiver2");
   EXPECT_FALSE(FileIO::DoesFileExist(notifierQueuePath));
   EXPECT_FALSE(FileIO::DoesFileExist(handshakeQueuePath));

   // Spawn sender and wait for it to start up
   zthread_new(&SenderThread, reinterpret_cast<TestThreadData*>(&senderData));
   EXPECT_TRUE(SleepUntilCondition({{senderData.hasStarted}}));
   EXPECT_TRUE(FileIO::DoesFileExist(notifierQueuePath));
   EXPECT_TRUE(FileIO::DoesFileExist(handshakeQueuePath));

   // Spawn Receivers and wait for them to start up
   zthread_new(&ReceiverThread, reinterpret_cast<TestThreadData*>(&receiver1ThreadData));
   zthread_new(&ReceiverThread, reinterpret_cast<TestThreadData*>(&receiver2ThreadData));
   EXPECT_TRUE(SleepUntilCondition({{receiver1ThreadData.hasStarted}, {receiver2ThreadData.hasStarted}}));

   // First Exchange
   FireOffANotification(senderData);
   EXPECT_TRUE(SleepUntilCondition({{receiver1ThreadData.received}, {receiver2ThreadData.received}}));

   // Shutdown all the threads
   //    and verify they are dead
   ShutdownThreads({{senderData}, {receiver1ThreadData}, {receiver2ThreadData}});
   EXPECT_TRUE(SleepUntilCondition({{senderData.hasExited}, {receiver1ThreadData.hasExited}, receiver2ThreadData.hasExited}));
   EXPECT_TRUE(ThreadIsShutdown(receiver1ThreadData));
   EXPECT_TRUE(ThreadIsShutdown(receiver2ThreadData));
   EXPECT_TRUE(ThreadIsShutdown(senderData));
}

TEST_F(NotifierTest, 2Messages2Receivers_RestartReceivers) {
   const size_t expectedFeedback = 2;
   const size_t sendFeedback = 1;
   TestThreadData senderData{"sender", expectedFeedback};
   TestThreadData receiver1ThreadData{"receiver1", sendFeedback};
   TestThreadData receiver2ThreadData{"receiver2", sendFeedback};
   EXPECT_FALSE(FileIO::DoesFileExist(notifierQueuePath));
   EXPECT_FALSE(FileIO::DoesFileExist(handshakeQueuePath));
 
   // Spawn Sender and wait for it to start up
   zthread_new(&SenderThread, reinterpret_cast<TestThreadData*>(&senderData));
   EXPECT_TRUE(SleepUntilCondition({{senderData.hasStarted}}));
   EXPECT_TRUE(FileIO::DoesFileExist(notifierQueuePath));
   EXPECT_TRUE(FileIO::DoesFileExist(handshakeQueuePath));

   // Spawn 2 Listeners and wait for them to start up
   zthread_new(&ReceiverThread, reinterpret_cast<TestThreadData*>(&receiver1ThreadData));
   zthread_new(&ReceiverThread, reinterpret_cast<TestThreadData*>(&receiver2ThreadData));
   EXPECT_TRUE(SleepUntilCondition({{receiver1ThreadData.hasStarted}, {receiver2ThreadData.hasStarted}}));

   // First Exchange
   FireOffANotification(senderData);
   EXPECT_TRUE(SleepUntilCondition({{receiver1ThreadData.received}, {receiver2ThreadData.received}}));
   EXPECT_TRUE(receiver1ThreadData.received->load());
   EXPECT_TRUE(receiver2ThreadData.received->load());

   // Shutdown the receiver threads
   ShutdownThreads({{receiver1ThreadData}, {receiver2ThreadData}});
   EXPECT_TRUE(SleepUntilCondition({{receiver1ThreadData.hasExited}, {receiver2ThreadData.hasExited}}));
   ShutdownThreads({{receiver1ThreadData}, {receiver2ThreadData}});
   EXPECT_TRUE(SleepUntilCondition({{receiver1ThreadData.hasExited}, {receiver2ThreadData.hasExited}}));
   EXPECT_TRUE(ThreadIsShutdown(receiver1ThreadData));
   EXPECT_TRUE(ThreadIsShutdown(receiver2ThreadData));
   EXPECT_FALSE(ThreadIsShutdown(senderData));
   receiver1ThreadData.reset();
   receiver2ThreadData.reset();

   // Respawn the Receivers
   zthread_new(&ReceiverThread, reinterpret_cast<TestThreadData*>(&receiver1ThreadData));
   zthread_new(&ReceiverThread, reinterpret_cast<TestThreadData*>(&receiver2ThreadData));
   EXPECT_TRUE(SleepUntilCondition({{receiver1ThreadData.hasStarted}, {receiver2ThreadData.hasStarted}}));

   // Second Exchange
   FireOffANotification(senderData);
   EXPECT_TRUE(SleepUntilCondition({{receiver1ThreadData.received}, {receiver2ThreadData.received}}));
   EXPECT_TRUE(receiver1ThreadData.received->load());
   EXPECT_TRUE(receiver2ThreadData.received->load());

   // Shutdown everything
   ShutdownThreads({{senderData}, {receiver1ThreadData}, {receiver2ThreadData}});
   EXPECT_TRUE(SleepUntilCondition({{senderData.hasExited}}));
   EXPECT_TRUE(SleepUntilCondition({{senderData.hasExited}}));
   EXPECT_TRUE(SleepUntilCondition({{receiver2ThreadData.hasExited}}));
   EXPECT_TRUE(ThreadIsShutdown(receiver1ThreadData));
   EXPECT_TRUE(ThreadIsShutdown(receiver2ThreadData));
   EXPECT_TRUE(ThreadIsShutdown(senderData));
}
