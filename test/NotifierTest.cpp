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
   const std::string messageToSend = "Craig is cool";
   const std::vector<std::string> vectorToSend = {"Craig", "is", "cool"};
   enum SendMessageType {NONE = 0, MSG = 1, VECTOR = 2};
   SendMessageType SEND_MSG_TYPE = NONE;


   void HandleNotification(std::unique_ptr<Listener> const &listener, TestThreadData& threadData) {
      bool confirmed = false;
      std::vector<std::string> messagesReceived = listener->GetMessages();
      if (threadData.kExpectedFeedback > 0) {
         confirmed = listener->SendConfirmation();
         if (!confirmed) {
            ADD_FAILURE() << "Failed to send feedback, thread #" << std::this_thread::get_id();
         }
      }
      if (!messagesReceived.empty()) {
         UpdateThreadDataAfterReceivingMessage(threadData, messagesReceived);
      } else {
         UpdateThreadDataAfterReceivingMessage(threadData);
      }
   }

   void* ReceiverThread(void* args) {
      TestThreadData* tmpRawPtr = reinterpret_cast<TestThreadData*>(args);
      CHECK(tmpRawPtr != nullptr);
      TestThreadData threadData(*tmpRawPtr);
      std::shared_ptr<void*> raiiExitFlag(nullptr, [threadData](void*) {
         threadData.hasExited->store(true);
         LOG(INFO) << "exiting thread: " << threadData.hasExited->load();
      });

      auto listener = Listener::CreateListener(notifierQueue, handshakeQueue, "TestThread");
      if (listener.get() == nullptr) {
         ADD_FAILURE() << "Failed to create listener, thread #" << std::this_thread::get_id();
         return nullptr;
      }

      NotifyParentThatChildHasStarted(threadData);
      StopWatch timer;

      while (ParentHasNotSentExitSignal(threadData) && !MaxTimeoutHasOccurred(timer)) {
         if (listener->NotificationReceived()) {
            HandleNotification(listener, threadData);
         }
      }
      const bool timeout = MaxTimeoutHasOccurred(timer);
      LOG(INFO) << "Exiting loop. Parent stated: keep running: " << threadData.keepRunning->load() << ", timeout: " << timeout;
      if (timeout) {
         ADD_FAILURE() << "Receiver thread has timed out after " << kMaxWaitTimeInSec << " seconds.";
      }
      LOG(INFO) << threadData.print();
      return nullptr;
   }

   void* SenderThread(void* args) {
      TestThreadData* tmpRawPtr = reinterpret_cast<TestThreadData*>(args);
      CHECK(tmpRawPtr != nullptr);
      TestThreadData threadData(*tmpRawPtr);
      auto notifier = Notifier::CreateNotifier(notifierQueue, handshakeQueue, threadData.kExpectedFeedback);
      std::shared_ptr<void*> raiiExitFlag(nullptr, [threadData](void*) { threadData.hasExited->store(true); });
      const bool initialized = notifier.get() != nullptr;
      if (false == initialized) {
         ADD_FAILURE() << "Failed to initialize the Notifier";
         return nullptr;
      }
      NotifyParentThatChildHasStarted(threadData);

      while (ParentHasNotSentExitSignal(threadData)) {
         if (TimeToSendANotification(threadData)) {
            switch(SEND_MSG_TYPE) {
               case NONE: 
                  EXPECT_EQ(notifier->Notify(), threadData.kExpectedFeedback);
                  break;
               case MSG:
                  EXPECT_EQ(notifier->Notify(messageToSend), threadData.kExpectedFeedback);
                  break;
               case VECTOR:
                  EXPECT_EQ(notifier->Notify(vectorToSend), threadData.kExpectedFeedback);
                  break;
            }
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
   const size_t ignoredHandshakes = 0;
   auto notifier = Notifier::CreateNotifier(notifierQueue, handshakeQueue, ignoredHandshakes);
   EXPECT_NE(notifier.get(), nullptr);
   EXPECT_TRUE(FileIO::DoesFileExist(notifierQueuePath));
   EXPECT_TRUE(FileIO::DoesFileExist(handshakeQueuePath));
}

TEST_F(NotifierTest, 1Message1Receiver_NoResponse_WithMessage) {
   SEND_MSG_TYPE = MSG;
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
   EXPECT_EQ(receiver1ThreadData.messages->size(), 1);
   ASSERT_EQ(receiver1ThreadData.messages->at(0), messageToSend);

   // Shutdown everything
   ShutdownThreads({{senderData}, {receiver1ThreadData}});
   EXPECT_TRUE(SleepUntilCondition({{senderData.hasExited}, {receiver1ThreadData.hasExited}}));
   EXPECT_TRUE(ThreadIsShutdown(receiver1ThreadData));
   EXPECT_TRUE(ThreadIsShutdown(senderData));
}

TEST_F(NotifierTest, 1Message1Receiver_NoResponse_NoMessage) {
   SEND_MSG_TYPE = NONE;
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

   // Shutdown everything
   ShutdownThreads({{senderData}, {receiver1ThreadData}});
   EXPECT_TRUE(SleepUntilCondition({{senderData.hasExited}, {receiver1ThreadData.hasExited}}));
   EXPECT_TRUE(ThreadIsShutdown(receiver1ThreadData));
   EXPECT_TRUE(ThreadIsShutdown(senderData));
}

TEST_F(NotifierTest, 1Message1Receiver_NoResponse_VectorMessage) {
   SEND_MSG_TYPE = VECTOR;
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
   ASSERT_EQ(receiver1ThreadData.messages->size(), vectorToSend.size());
   for (unsigned int idx = 0; idx < receiver1ThreadData.messages->size(); idx++) {
      EXPECT_EQ(vectorToSend.at(idx), receiver1ThreadData.messages->at(idx));
   }

   // Shutdown everything
   ShutdownThreads({{senderData}, {receiver1ThreadData}});
   EXPECT_TRUE(SleepUntilCondition({{senderData.hasExited}, {receiver1ThreadData.hasExited}}));
   EXPECT_TRUE(ThreadIsShutdown(receiver1ThreadData));
   EXPECT_TRUE(ThreadIsShutdown(senderData));
}

TEST_F(NotifierTest, 1Message2ReceiversUsingSenderandReceiverThreads_NoMessage) {
   SEND_MSG_TYPE = NONE;
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

TEST_F(NotifierTest, 1Message2ReceiversUsingSenderandReceiverThreads_SingleMessage) {
   SEND_MSG_TYPE = MSG;
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
   EXPECT_EQ(1, receiver1ThreadData.messages->size());
   ASSERT_EQ(messageToSend, receiver1ThreadData.messages->at(0));

   // Shutdown all the threads
   //    and verify they are dead
   ShutdownThreads({{senderData}, {receiver1ThreadData}, {receiver2ThreadData}});
   EXPECT_TRUE(SleepUntilCondition({{senderData.hasExited}, {receiver1ThreadData.hasExited}, receiver2ThreadData.hasExited}));
   EXPECT_TRUE(ThreadIsShutdown(receiver1ThreadData));
   EXPECT_TRUE(ThreadIsShutdown(receiver2ThreadData));
   EXPECT_TRUE(ThreadIsShutdown(senderData));
}

TEST_F(NotifierTest, 1Message2ReceiversUsingSenderandReceiverThreads_VectorMessage) {
   SEND_MSG_TYPE = VECTOR;
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
   ASSERT_EQ(receiver1ThreadData.messages->size(), vectorToSend.size());
   for (unsigned int idx = 0; idx < receiver1ThreadData.messages->size(); idx++) {
      EXPECT_EQ(vectorToSend.at(idx), receiver1ThreadData.messages->at(idx));
   }

   // Shutdown all the threads
   //    and verify they are dead
   ShutdownThreads({{senderData}, {receiver1ThreadData}, {receiver2ThreadData}});
   EXPECT_TRUE(SleepUntilCondition({{senderData.hasExited}, {receiver1ThreadData.hasExited}, receiver2ThreadData.hasExited}));
   EXPECT_TRUE(ThreadIsShutdown(receiver1ThreadData));
   EXPECT_TRUE(ThreadIsShutdown(receiver2ThreadData));
   EXPECT_TRUE(ThreadIsShutdown(senderData));
}

TEST_F(NotifierTest, 2Messages2Receivers_RestartReceivers_NoMessage) {
   SEND_MSG_TYPE = NONE;
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

TEST_F(NotifierTest, 2Messages2Receivers_RestartReceivers_SingleMessage) {
   SEND_MSG_TYPE = MSG;
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
   EXPECT_EQ(1, receiver1ThreadData.messages->size());
   EXPECT_EQ(1, receiver2ThreadData.messages->size());
   ASSERT_EQ(messageToSend, receiver1ThreadData.messages->at(0));
   ASSERT_EQ(messageToSend, receiver2ThreadData.messages->at(0));

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
   EXPECT_EQ(1, receiver1ThreadData.messages->size());
   EXPECT_EQ(1, receiver2ThreadData.messages->size());
   ASSERT_EQ(messageToSend, receiver1ThreadData.messages->at(0));
   ASSERT_EQ(messageToSend, receiver2ThreadData.messages->at(0));

   // Shutdown everything
   ShutdownThreads({{senderData}, {receiver1ThreadData}, {receiver2ThreadData}});
   EXPECT_TRUE(SleepUntilCondition({{senderData.hasExited}}));
   EXPECT_TRUE(SleepUntilCondition({{senderData.hasExited}}));
   EXPECT_TRUE(SleepUntilCondition({{receiver2ThreadData.hasExited}}));
   EXPECT_TRUE(ThreadIsShutdown(receiver1ThreadData));
   EXPECT_TRUE(ThreadIsShutdown(receiver2ThreadData));
   EXPECT_TRUE(ThreadIsShutdown(senderData));
}

TEST_F(NotifierTest, 2Messages2Receivers_RestartReceivers_VectorMessage) {
   SEND_MSG_TYPE = VECTOR;
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
   ASSERT_EQ(receiver1ThreadData.messages->size(), vectorToSend.size());
   ASSERT_EQ(receiver2ThreadData.messages->size(), vectorToSend.size());
   for (unsigned int idx = 0; idx < vectorToSend.size(); idx++) {
      EXPECT_EQ(vectorToSend.at(idx), receiver1ThreadData.messages->at(idx));
      EXPECT_EQ(vectorToSend.at(idx), receiver2ThreadData.messages->at(idx));
   }

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
   ASSERT_EQ(receiver1ThreadData.messages->size(), vectorToSend.size());
   ASSERT_EQ(receiver2ThreadData.messages->size(), vectorToSend.size());
   for (unsigned int idx = 0; idx < vectorToSend.size(); idx++) {
      EXPECT_EQ(vectorToSend.at(idx), receiver1ThreadData.messages->at(idx));
      EXPECT_EQ(vectorToSend.at(idx), receiver2ThreadData.messages->at(idx));
   }

   // Shutdown everything
   ShutdownThreads({{senderData}, {receiver1ThreadData}, {receiver2ThreadData}});
   EXPECT_TRUE(SleepUntilCondition({{senderData.hasExited}}));
   EXPECT_TRUE(SleepUntilCondition({{senderData.hasExited}}));
   EXPECT_TRUE(SleepUntilCondition({{receiver2ThreadData.hasExited}}));
   EXPECT_TRUE(ThreadIsShutdown(receiver1ThreadData));
   EXPECT_TRUE(ThreadIsShutdown(receiver2ThreadData));
   EXPECT_TRUE(ThreadIsShutdown(senderData));
}