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
#include <Result.h>

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

   /**
    * RAII delete the string after it has been sent through the Rifle/Vampire queue back to the 
    * Notifier
    */
   void ZeroCopyDelete(void*, void* data) {
      std::string* theString = reinterpret_cast<std::string*>(data);
      delete theString;  
   }

   void *SenderThreadForReceiveData(void* args) {
      TestThreadData* tmpRawPtr = reinterpret_cast<TestThreadData*>(args);
      CHECK(tmpRawPtr != nullptr);
      TestThreadData senderThreadData(*tmpRawPtr);
      std::shared_ptr<void*> raiiExitFlag(nullptr, [senderThreadData](void*) { senderThreadData.hasExited->store(true); });
      Rifle testRifle(handshakeQueue);
      NotifyParentThatChildHasStarted(senderThreadData);
      testRifle.SetOwnSocket(false);
      testRifle.SetHighWater(100);
      EXPECT_TRUE(testRifle.Aim());
      EXPECT_TRUE(testRifle.Fire(std::string("test string")));
      std::this_thread::sleep_for(std::chrono::seconds(1));
      return nullptr;
   }

   void* ReceiverThreadForReceiveData(void* args) {
      TestThreadData* tmpRawPtr = reinterpret_cast<TestThreadData*>(args);
      CHECK(tmpRawPtr != nullptr);
      TestThreadData receiverThreadData(*tmpRawPtr);
      std::string toReceive = "test string";
      size_t notifierTimeoutInSec = 1;
      int getShotTimeoutInMs = 1;
      auto notifier = Notifier::CreateNotifier(notifierQueue, handshakeQueue, 0, notifierTimeoutInSec, getShotTimeoutInMs);
      std::shared_ptr<void*> raiiExitFlag(nullptr, [receiverThreadData](void*) { receiverThreadData.hasExited->store(true); });
      const bool initialized = notifier.get() != nullptr;
      if (false == initialized) {
         ADD_FAILURE() << "Failed to initialize the Notifier";
         return nullptr;
      }
      auto received = std::string("");
      NotifyParentThatChildHasStarted(receiverThreadData);
      while (ParentHasNotSentExitSignal(receiverThreadData)) {
         received = notifier->ReceiveData();
         if (!received.empty()) {
            EXPECT_EQ(received, toReceive);
            break;
         }
      }
      EXPECT_FALSE(received.empty());
      return nullptr;
   }

   void CheckReturnedVector(const std::vector<TestThreadData>& threadDataVector) {
      for (unsigned int idx = 0; idx < vectorToSend.size(); idx++) {
         for (auto& threadData : threadDataVector) {
            EXPECT_EQ(vectorToSend.at(idx), threadData.messages->at(idx));
         }
      }
   }

   void SpawnSender_WaitForStartup(TestThreadData& senderData) {
      zthread_new(&SenderThread, reinterpret_cast<TestThreadData*>(&senderData));
      EXPECT_TRUE(SleepUntilCondition({{senderData.hasStarted}}));
      EXPECT_TRUE(FileIO::DoesFileExist(notifierQueuePath));
      EXPECT_TRUE(FileIO::DoesFileExist(handshakeQueuePath));
   }

   void SpawnReceiveDataSender_WaitForStartup(TestThreadData& senderData) {
      EXPECT_TRUE(FileIO::DoesFileExist(notifierQueuePath));
      EXPECT_TRUE(FileIO::DoesFileExist(handshakeQueuePath));
      zthread_new(&SenderThreadForReceiveData, reinterpret_cast<TestThreadData*>(&senderData));
      EXPECT_TRUE(SleepUntilCondition({{senderData.hasStarted}}));
   }

   void SpawnReceiveDataReceiver_WaitForStartup(TestThreadData& receiverData) {
      zthread_new(&ReceiverThreadForReceiveData, reinterpret_cast<TestThreadData*>(&receiverData));
      EXPECT_TRUE(SleepUntilCondition({{receiverData.hasStarted}}));
      EXPECT_TRUE(FileIO::DoesFileExist(notifierQueuePath));
      EXPECT_TRUE(FileIO::DoesFileExist(handshakeQueuePath));
   }

   void SpawnReceiver_WaitForStartup(TestThreadData& receiverData) {
      zthread_new(&ReceiverThread, reinterpret_cast<TestThreadData*>(&receiverData));
      EXPECT_TRUE(SleepUntilCondition({{receiverData.hasStarted}}));
   }
   
   void ExchangeNotification(TestThreadData& senderData, TestThreadData& receiverData) {
      FireOffANotification(senderData);
      EXPECT_TRUE(SleepUntilCondition({{receiverData.received}}));
   }

   void ExchangeNotification2Receivers(TestThreadData& senderData,
                                       TestThreadData& receiver1ThreadData,
                                       TestThreadData& receiver2ThreadData) {
      FireOffANotification(senderData);
      EXPECT_TRUE(SleepUntilCondition({{receiver1ThreadData.received}, {receiver2ThreadData.received}}));
   }
   
   void VerifySingleMessage(TestThreadData& receiverData) {
      ASSERT_EQ(receiverData.messages->size(), 1);
      EXPECT_EQ(receiverData.messages->at(0), messageToSend);
   }

   void Shutdown(std::vector<TestThreadData> threads) {
      ShutdownThreads(threads);
      for (auto& thread : threads) {
         EXPECT_TRUE(SleepUntilCondition({{thread.hasExited}}));
         EXPECT_TRUE(ThreadIsShutdown(thread));
      }
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

   SpawnSender_WaitForStartup(senderData);

   SpawnReceiver_WaitForStartup(receiver1ThreadData);

   ExchangeNotification(senderData, receiver1ThreadData);

   VerifySingleMessage(receiver1ThreadData);


   Shutdown({senderData, receiver1ThreadData});
}

TEST_F(NotifierTest, 1Message1Receiver_NoResponse_NoMessage) {
   SEND_MSG_TYPE = NONE;
   TestThreadData senderData("sender");
   TestThreadData receiver1ThreadData("receiver");
   EXPECT_FALSE(FileIO::DoesFileExist(notifierQueuePath));
   EXPECT_FALSE(FileIO::DoesFileExist(handshakeQueuePath));

   SpawnSender_WaitForStartup(senderData);

   SpawnReceiver_WaitForStartup(receiver1ThreadData);

   ExchangeNotification(senderData, receiver1ThreadData);

   Shutdown({senderData, receiver1ThreadData});
}

TEST_F(NotifierTest, 1Message1Receiver_NoResponse_VectorMessage) {
   SEND_MSG_TYPE = VECTOR;
   TestThreadData senderData("sender");
   TestThreadData receiver1ThreadData("receiver");
   EXPECT_FALSE(FileIO::DoesFileExist(notifierQueuePath));
   EXPECT_FALSE(FileIO::DoesFileExist(handshakeQueuePath));

   SpawnSender_WaitForStartup(senderData);

   SpawnReceiver_WaitForStartup(receiver1ThreadData);

   ExchangeNotification(senderData, receiver1ThreadData);
   CheckReturnedVector({receiver1ThreadData});

   Shutdown({senderData, receiver1ThreadData});
}

TEST_F(NotifierTest, 1Message2Receivers_ExpectFeedback_NoMessage) {
   SEND_MSG_TYPE = NONE;
   TestThreadData senderData("sender");
   TestThreadData receiver1ThreadData("receiver1");
   TestThreadData receiver2ThreadData("receiver2");
   EXPECT_FALSE(FileIO::DoesFileExist(notifierQueuePath));
   EXPECT_FALSE(FileIO::DoesFileExist(handshakeQueuePath));

   SpawnSender_WaitForStartup(senderData);

   SpawnReceiver_WaitForStartup(receiver1ThreadData);
   SpawnReceiver_WaitForStartup(receiver2ThreadData);

   ExchangeNotification2Receivers(senderData, receiver1ThreadData, receiver2ThreadData);

   Shutdown({senderData, receiver2ThreadData, receiver1ThreadData});
}

TEST_F(NotifierTest, 1Message2Receivers_ExpectFeedback_SingleMessage) {
   SEND_MSG_TYPE = MSG;
   TestThreadData senderData("sender");
   TestThreadData receiver1ThreadData("receiver1");
   TestThreadData receiver2ThreadData("receiver2");
   EXPECT_FALSE(FileIO::DoesFileExist(notifierQueuePath));
   EXPECT_FALSE(FileIO::DoesFileExist(handshakeQueuePath));

   SpawnSender_WaitForStartup(senderData);

   // Spawn Receivers and wait for them to start up
   SpawnReceiver_WaitForStartup(receiver1ThreadData);
   SpawnReceiver_WaitForStartup(receiver2ThreadData);

   // First Exchange
   ExchangeNotification2Receivers(senderData, receiver1ThreadData, receiver2ThreadData);
   VerifySingleMessage(receiver1ThreadData);
   VerifySingleMessage(receiver2ThreadData);

   Shutdown({senderData, receiver1ThreadData, receiver2ThreadData});
}

TEST_F(NotifierTest, 1Message2Receivers_ExpectFeedback_VectorMessage) {
   SEND_MSG_TYPE = VECTOR;
   TestThreadData senderData("sender");
   TestThreadData receiver1ThreadData("receiver1");
   TestThreadData receiver2ThreadData("receiver2");
   EXPECT_FALSE(FileIO::DoesFileExist(notifierQueuePath));
   EXPECT_FALSE(FileIO::DoesFileExist(handshakeQueuePath));

   SpawnSender_WaitForStartup(senderData);

   // Spawn Receivers and wait for them to start up
   SpawnReceiver_WaitForStartup(receiver1ThreadData);
   SpawnReceiver_WaitForStartup(receiver2ThreadData);

   // First Exchange
   ExchangeNotification2Receivers(senderData, receiver1ThreadData, receiver2ThreadData);
   CheckReturnedVector({{receiver1ThreadData}});

   Shutdown({senderData, receiver1ThreadData, receiver2ThreadData});
}

TEST_F(NotifierTest, 2Messages2Receivers_RestartReceivers_ExpectFeedback_NoMessage) {
   SEND_MSG_TYPE = NONE;
   const size_t expectedFeedback = 2;
   const size_t sendFeedback = 1;
   TestThreadData senderData{"sender", expectedFeedback};
   TestThreadData receiver1ThreadData{"receiver1", sendFeedback};
   TestThreadData receiver2ThreadData{"receiver2", sendFeedback};
   EXPECT_FALSE(FileIO::DoesFileExist(notifierQueuePath));
   EXPECT_FALSE(FileIO::DoesFileExist(handshakeQueuePath));

   SpawnSender_WaitForStartup(senderData);

   SpawnReceiver_WaitForStartup(receiver1ThreadData);
   SpawnReceiver_WaitForStartup(receiver2ThreadData);

   // First Exchange
   ExchangeNotification2Receivers(senderData, receiver2ThreadData, receiver2ThreadData);

   // Shutdown the receiver threads
   Shutdown({receiver1ThreadData, receiver2ThreadData});
   EXPECT_FALSE(ThreadIsShutdown(senderData));
   receiver1ThreadData.reset();
   receiver2ThreadData.reset();

   // Respawn the Receivers
   SpawnReceiver_WaitForStartup(receiver1ThreadData);
   SpawnReceiver_WaitForStartup(receiver2ThreadData);

   // Second Exchange
   ExchangeNotification2Receivers(senderData, receiver2ThreadData, receiver2ThreadData);

   // Shutdown everything
   Shutdown({senderData, receiver1ThreadData, receiver2ThreadData});
}

TEST_F(NotifierTest, 2Messages2Receivers_RestartReceivers_ExpectFeedback_SingleMessage) {
   SEND_MSG_TYPE = MSG;
   const size_t expectedFeedback = 2;
   const size_t sendFeedback = 1;
   TestThreadData senderData{"sender", expectedFeedback};
   TestThreadData receiver1ThreadData{"receiver1", sendFeedback};
   TestThreadData receiver2ThreadData{"receiver2", sendFeedback};
   EXPECT_FALSE(FileIO::DoesFileExist(notifierQueuePath));
   EXPECT_FALSE(FileIO::DoesFileExist(handshakeQueuePath));

   SpawnSender_WaitForStartup(senderData);

   SpawnReceiver_WaitForStartup(receiver1ThreadData);
   SpawnReceiver_WaitForStartup(receiver2ThreadData);

   // First Exchange
   ExchangeNotification2Receivers(senderData, receiver2ThreadData, receiver2ThreadData);
   VerifySingleMessage(receiver1ThreadData);
   VerifySingleMessage(receiver2ThreadData);

   // Shutdown the receiver threads
   Shutdown({receiver1ThreadData, receiver2ThreadData});
   EXPECT_FALSE(ThreadIsShutdown(senderData));
   receiver1ThreadData.reset();
   receiver2ThreadData.reset();

   // Respawn the Receivers
   SpawnReceiver_WaitForStartup(receiver1ThreadData);
   SpawnReceiver_WaitForStartup(receiver2ThreadData);

   // Second Exchange
   ExchangeNotification2Receivers(senderData, receiver2ThreadData, receiver2ThreadData);
   VerifySingleMessage(receiver1ThreadData);
   VerifySingleMessage(receiver2ThreadData);

   // Shutdown everything
   Shutdown({senderData, receiver1ThreadData, receiver2ThreadData});
}

TEST_F(NotifierTest, 2Messages2Receivers_RestartReceivers_ExpectFeedback_VectorMessage) {
   SEND_MSG_TYPE = VECTOR;
   const size_t expectedFeedback = 2;
   const size_t sendFeedback = 1;
   TestThreadData senderData{"sender", expectedFeedback};
   TestThreadData receiver1ThreadData{"receiver1", sendFeedback};
   TestThreadData receiver2ThreadData{"receiver2", sendFeedback};
   EXPECT_FALSE(FileIO::DoesFileExist(notifierQueuePath));
   EXPECT_FALSE(FileIO::DoesFileExist(handshakeQueuePath));

   SpawnSender_WaitForStartup(senderData);

   SpawnReceiver_WaitForStartup(receiver1ThreadData);
   SpawnReceiver_WaitForStartup(receiver2ThreadData);

   // First Exchange
   ExchangeNotification2Receivers(senderData, receiver2ThreadData, receiver2ThreadData);
   CheckReturnedVector({receiver1ThreadData, receiver2ThreadData});

   // Shutdown the receiver threads
   Shutdown({receiver1ThreadData, receiver2ThreadData});
   EXPECT_FALSE(ThreadIsShutdown(senderData));
   receiver1ThreadData.reset();
   receiver2ThreadData.reset();

   // Respawn the Receivers
   SpawnReceiver_WaitForStartup(receiver1ThreadData);
   SpawnReceiver_WaitForStartup(receiver2ThreadData);

   // Second Exchange
   ExchangeNotification2Receivers(senderData, receiver2ThreadData, receiver2ThreadData);
   CheckReturnedVector({receiver1ThreadData, receiver2ThreadData});

   // Shutdown everything
   Shutdown({senderData, receiver1ThreadData, receiver2ThreadData});
}

TEST_F(NotifierTest, 1Message1Receiver_ReceiveData) {
   TestThreadData senderData("sender");
   TestThreadData receiver1ThreadData("receiver");
   EXPECT_FALSE(FileIO::DoesFileExist(notifierQueuePath));
   EXPECT_FALSE(FileIO::DoesFileExist(handshakeQueuePath));
   // spawn thread to create notifier and poll the ReceiveData function 
   SpawnReceiveDataReceiver_WaitForStartup(receiver1ThreadData);
   // spawn thread to create rifle and fire shot
   SpawnReceiveDataSender_WaitForStartup(senderData);

   Shutdown({senderData, receiver1ThreadData});
}
