
#include "BoomStickTest.h"
#include "MockSkelleton.h"
#include "MockBoomStick.h"
#include <unordered_set>
#include <memory>
#include <future>
#include <unordered_map>
#ifdef LR_DEBUG
namespace {

   void runIterations(BoomStick& stick, int iterations) {
      std::stringstream sS;
      for (int i = 0; i < iterations; i++) {
         sS << "request " << i;
         std::string reply = stick.Send(sS.str());
         sS << " reply";
         if (sS.str() != reply) {
            FAIL();
         }
         sS.str("");
      }
   }

   void runAsync(BoomStick& stick, int iterations) {
      std::stringstream sS;
      std::stringstream uuid;
      std::vector<MessageIdentifier> sentMessages;
      for (int i = 0; i < iterations; i++) {
         sS << "request " << i;
         uuid << i;
         MessageIdentifier id = make_pair(uuid.str(), time(NULL));
         if (!stick.SendAsync(id, sS.str())) {
            FAIL();
         }
         sentMessages.push_back(id);
         sS.str("");
         uuid.str("");
      }
      int i = 0;
      for (auto id : sentMessages) {
         std::string reply;
         sS << "request " << i++ << " reply";
         if (!stick.GetAsyncReply(id, 10, reply)) {
            FAIL();
         }
         if (reply != sS.str()) {
            FAIL();
         }
         sS.str("");
      }
   }

   void Shooter(int threadId, int repitions, const std::string& address) {
      BoomStick stick{address};
      ASSERT_TRUE(stick.Initialize());
      runIterations(stick, 100);
   }

   void AsyncShooter(int threadId, int repitions, const std::string& address) {
      BoomStick stick{address};
      ASSERT_TRUE(stick.Initialize());
      runAsync(stick, 100);
   }
}



TEST_F(BoomStickTest, Construct) {
   BoomStick stick{mAddress};
   std::unique_ptr<BoomStick> pStick(new BoomStick(mAddress));
}

TEST_F(BoomStickTest, SimpleSend) {
   BoomStick stick{mAddress};
   MockSkelleton target{mAddress};

   ASSERT_TRUE(target.Initialize());
   ASSERT_TRUE(stick.Initialize());

   target.BeginListenAndRepeat();

   std::string reply = stick.Send("foo");
   EXPECT_EQ("foo reply", reply);

   target.EndListendAndRepeat();
}

TEST_F(BoomStickTest, SimpleSendNoListener) {
   BoomStick stick{mAddress};
   ASSERT_TRUE(stick.Initialize());

   std::string reply = stick.Send("foo");
   EXPECT_EQ("", reply);

}

TEST_F(BoomStickTest, AsyncSendMultipleTimesOnOneIdFails) {
   BoomStick stick{mAddress};
   MockSkelleton target{mAddress};

   ASSERT_TRUE(target.Initialize());
   ASSERT_TRUE(stick.Initialize());

   target.BeginListenAndRepeat();
   MessageIdentifier id;
   id.first = "uuid1";
   id.second = time(NULL);
   ASSERT_TRUE(stick.SendAsync(id, "foo1"));
   ASSERT_FALSE(stick.SendAsync(id, "foo2"));

   target.EndListendAndRepeat();

}

TEST_F(BoomStickTest, SimpleSendAfterSeveralUnMatchedSends) {
   BoomStick stick{mAddress};
   MockSkelleton target{mAddress};

   ASSERT_TRUE(target.Initialize());
   ASSERT_TRUE(stick.Initialize());

   target.BeginListenAndRepeat();
   MessageIdentifier id;
   id.first = "uuid1";
   id.second = time(NULL);
   ASSERT_TRUE(stick.SendAsync(id, "foo1"));
   id.first = "uuid2";
   ASSERT_TRUE(stick.SendAsync(id, "foo2"));
   id.first = "uuid3";
   ASSERT_TRUE(stick.SendAsync(id, "foo3"));
   std::string reply = stick.Send("foo");
   EXPECT_EQ("foo reply", reply);

   target.EndListendAndRepeat();
}

TEST_F(BoomStickTest, SingleTargetSingleShooterSync) {
   BoomStick stick{mAddress};
   MockSkelleton target{mAddress};

   ASSERT_TRUE(target.Initialize());
   ASSERT_TRUE(stick.Initialize());

   target.BeginListenAndRepeat();

   runIterations(stick, 100);

   target.EndListendAndRepeat();

}

TEST_F(BoomStickTest, AsyncDoesnWorkIfUninit) {
   BoomStick stick{mAddress};
   std::string reply;
   MessageIdentifier id;
   id.first = "foo";
   id.second = time(NULL);
   ASSERT_FALSE(stick.SendAsync(id, "bar"));
   ASSERT_FALSE(stick.GetAsyncReply(id, 0, reply));
}

TEST_F(BoomStickTest, GetAsyncReplyTwice) {
   BoomStick stick{mAddress};
   MockSkelleton target{mAddress};
   std::string reply;

   ASSERT_TRUE(target.Initialize());
   ASSERT_TRUE(stick.Initialize());

   target.BeginListenAndRepeat();
   MessageIdentifier id;
   id.first = "foo";
   id.second = time(NULL);
   ASSERT_TRUE(stick.SendAsync(id, "foo"));
   ASSERT_TRUE(stick.GetAsyncReply(id, 10, reply));
   ASSERT_FALSE(stick.GetAsyncReply(id, 10,  reply));

   target.EndListendAndRepeat();
}

TEST_F(BoomStickTest, GetAsyncNoListener) {
   BoomStick stick{mAddress};
   std::string reply;

   ASSERT_TRUE(stick.Initialize());

   MessageIdentifier id;
   id.first = "foo";
   id.second = time(NULL);
   ASSERT_TRUE(stick.SendAsync(id, "foo"));
   ASSERT_FALSE(stick.GetAsyncReply(id, 0, reply));

}

TEST_F(BoomStickTest, CleanupStaleStuff) {
   MockBoomStick exposedStick{mAddress};
   MockSkelleton target{mAddress};

   ASSERT_TRUE(target.Initialize());
   ASSERT_TRUE(exposedStick.Initialize());

   target.BeginListenAndRepeat();
   MessageIdentifier id;
   id.first = "foo";
   id.second = time(NULL)-(5 * MINUTES_TO_SECONDS);
   MessageIdentifier id2;
   id2.first = "bar";
   id2.second = time(NULL);
   std::string reply;

   ASSERT_TRUE(exposedStick.SendAsync(id, "foo"));
   while (reply.empty()) { // Loop until we find the reply for id while searching for id2
      ASSERT_TRUE(exposedStick.SendAsync(id2, "foo"));
      EXPECT_TRUE(exposedStick.FindPendingId(id));
      EXPECT_TRUE(exposedStick.FindPendingId(id2));
      EXPECT_TRUE(exposedStick.GetAsyncReply(id2, 10, reply));
      reply = exposedStick.GetCachedReply(id);
   }
   EXPECT_TRUE(exposedStick.FindPendingId(id));
   EXPECT_FALSE(exposedStick.FindPendingId(id2));
   exposedStick.ForceGC();
   exposedStick.CleanOldPendingData();

   EXPECT_FALSE(exposedStick.FindPendingId(id));
   // We never read the AsyncReply, it was purged in the cleanup
   EXPECT_FALSE(exposedStick.GetAsyncReply(id, 0, reply));

   target.EndListendAndRepeat();
}

TEST_F(BoomStickTest, SingleTargetSingleShooterAsync) {
   BoomStick stick{mAddress};
   MockSkelleton target{mAddress};

   ASSERT_TRUE(target.Initialize());
   ASSERT_TRUE(stick.Initialize());

   target.BeginListenAndRepeat();

   runAsync(stick, 100);

   target.EndListendAndRepeat();

}

TEST_F(BoomStickTest, SingleTargetMultipleShooterSync) {

   MockSkelleton target{mAddress};

   ASSERT_TRUE(target.Initialize());
   std::unordered_set<std::shared_ptr<std::thread> > threads;
   target.BeginListenAndRepeat();
   for (int i = 0; i < 10; i++) {
      threads.insert(std::make_shared<std::thread>(Shooter, i, 100, mAddress));
   }
   for (auto thread : threads) {
      thread->join();
   }
   target.EndListendAndRepeat();

}

TEST_F(BoomStickTest, SingleTargetMultipleShooterAsync) {

   MockSkelleton target{mAddress};

   ASSERT_TRUE(target.Initialize());
   std::unordered_set<std::shared_ptr<std::thread> > threads;
   target.BeginListenAndRepeat();
   for (int i = 0; i < 10; i++) {
      threads.insert(std::make_shared<std::thread>(AsyncShooter, i, 100, mAddress));
   }
   for (auto thread : threads) {
      thread->join();
   }
   target.EndListendAndRepeat();

}

TEST_F(BoomStickTest, InitializeFailsOnBadAddress) {
   BoomStick failure{"abc123"};

   EXPECT_FALSE(failure.Initialize());

   EXPECT_EQ("", failure.Send("command"));
}

TEST_F(BoomStickTest, InitializeFailsNoContext) {
   MockBoomStick failure{mAddress};

   failure.mFailsGetNewContext = true;
   EXPECT_FALSE(failure.Initialize());

   EXPECT_EQ("", failure.Send("command"));
}

TEST_F(BoomStickTest, InitializeFailsNoSocket) {
   MockBoomStick failure{mAddress};

   failure.mFailseGetNewSocket = true;
   EXPECT_FALSE(failure.Initialize());

   EXPECT_EQ("", failure.Send("command"));
}

TEST_F(BoomStickTest, InitializeFailsNoConnect) {
   MockBoomStick failure{mAddress};

   failure.mFailsConnect = true;
   EXPECT_FALSE(failure.Initialize());

   EXPECT_EQ("", failure.Send("command"));
}

TEST_F(BoomStickTest, MoveConstructor) {
   BoomStick firstObject{mAddress};
   MockSkelleton target{mAddress};

   ASSERT_TRUE(target.Initialize());
   ASSERT_TRUE(firstObject.Initialize());

   target.BeginListenAndRepeat();
   runIterations(firstObject, 100);

   BoomStick secondObject(std::move(firstObject));

   runIterations(secondObject, 100);

   target.EndListendAndRepeat();
}

TEST_F(BoomStickTest, MoveAssignment) {
   BoomStick firstObject{mAddress};
   MockSkelleton target{mAddress};

   ASSERT_TRUE(target.Initialize());
   ASSERT_TRUE(firstObject.Initialize());

   target.BeginListenAndRepeat();
   runIterations(firstObject, 100);

   BoomStick secondObject{"abc123"};
   secondObject = std::move(firstObject);
   runIterations(secondObject, 100);

   target.EndListendAndRepeat();
}

TEST_F(BoomStickTest, Swap) {
   BoomStick firstObject{mAddress};
   MockSkelleton target{mAddress};

   ASSERT_TRUE(target.Initialize());
   ASSERT_TRUE(firstObject.Initialize());

   target.BeginListenAndRepeat();
   runIterations(firstObject, 100);

   BoomStick secondObject{"abc123"};
   secondObject.Swap(firstObject);
   ASSERT_EQ("", firstObject.Send("test"));
   runIterations(secondObject, 100);

   target.EndListendAndRepeat();
}

TEST_F(BoomStickTest, DontSearchSocketForever) {
   MockBoomStick stick{mAddress};
   MockSkelleton target{mAddress};

   ASSERT_TRUE(target.Initialize());
   ASSERT_TRUE(stick.Initialize());
   
   MessageIdentifier id;
   id.first = "foo";
   id.second = time(NULL)-(5 * MINUTES_TO_SECONDS);
   ASSERT_TRUE(stick.SendAsync(id, "foo"));
   
   std::promise<bool> promisedFinished;
   auto futureResult = promisedFinished.get_future();
   std::thread([](std::promise<bool>& finished, MockBoomStick & stick, MessageIdentifier& id) {
      std::string reply;
              EXPECT_FALSE(stick.GetAsyncReply(id, 10, reply));
              finished.set_value(true);
   }, std::ref(promisedFinished), std::ref(stick), std::ref(id)).detach();

   EXPECT_TRUE(futureResult.wait_for(std::chrono::milliseconds(10+2)) != std::future_status::timeout);

   
}

#else 

TEST_F(BoomStickTest, emptyTest) {
   EXPECT_TRUE(true);
}
#endif