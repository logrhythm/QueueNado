
#include "BoomStickTest.h"
#include "MockSkelleton.h"
#include "MockBoomStick.h"
#include <unordered_set>
#ifdef LR_DEBUG

TEST_F(BoomStickTest, Construct) {
   BoomStick stick(mAddress);
   BoomStick* pStick = new BoomStick(mAddress);

   delete pStick;

}

TEST_F(BoomStickTest, SingleTargetSingleShooterSync) {
   BoomStick stick(mAddress);
   MockSkelleton target(mAddress);

   ASSERT_TRUE(target.Initialize());
   ASSERT_TRUE(stick.Initialize());

   target.BeginListenAndRepeat();
   std::stringstream sS;

   for (int i = 0; i < 100; i++) {
      sS << "request " << i;
      std::string reply = stick.Send(sS.str());
      sS << " reply";
      ASSERT_EQ(sS.str(), reply);
      sS.str("");
   }

   target.EndListendAndRepeat();

}

void Shooter(int threadId, int repitions, const std::string& address) {
   BoomStick stick(address);
   ASSERT_TRUE(stick.Initialize());
   std::stringstream sS;

   for (int i = 0; i < repitions; i++) {
      sS << "request " << threadId << " " << i;
      std::string reply = stick.Send(sS.str());
      sS << " reply";
      ASSERT_EQ(sS.str(), reply);
      sS.str("");
   }
}

TEST_F(BoomStickTest, SingleTargetMultipleShooterSync) {

   MockSkelleton target(mAddress);

   ASSERT_TRUE(target.Initialize());
   std::unordered_set<std::thread*> threads;
   target.BeginListenAndRepeat();
   for (int i = 0; i < 10; i++) {
      threads.insert(new std::thread(Shooter, i, 100, mAddress));
   }
   for (auto thread : threads) {
      thread->join();
      delete thread;
   }
   target.EndListendAndRepeat();

}

TEST_F(BoomStickTest, InitializeFailsOnBadAddress) {
   BoomStick failure("abc123");

   EXPECT_FALSE(failure.Initialize());

   EXPECT_EQ("", failure.Send("command"));
}

TEST_F(BoomStickTest, InitializeFailsNoContext) {
   MockBoomStick failure(mAddress);

   failure.mFailsGetNewContext = true;
   EXPECT_FALSE(failure.Initialize());

   EXPECT_EQ("", failure.Send("command"));
}

TEST_F(BoomStickTest, InitializeFailsNoSocket) {
   MockBoomStick failure(mAddress);

   failure.mFailseGetNewSocket = true;
   EXPECT_FALSE(failure.Initialize());

   EXPECT_EQ("", failure.Send("command"));
}

TEST_F(BoomStickTest, InitializeFailsNoConnect) {
   MockBoomStick failure(mAddress);

   failure.mFailsConnect = true;
   EXPECT_FALSE(failure.Initialize());

   EXPECT_EQ("", failure.Send("command"));
}

TEST_F(BoomStickTest, MoveConstructor) {
   BoomStick firstObject(mAddress);
   MockSkelleton target(mAddress);

   ASSERT_TRUE(target.Initialize());
   ASSERT_TRUE(firstObject.Initialize());

   target.BeginListenAndRepeat();
   std::stringstream sS;

   for (int i = 0; i < 100; i++) {
      sS << "request " << i;
      std::string reply = firstObject.Send(sS.str());
      sS << " reply";
      ASSERT_EQ(sS.str(), reply);
      sS.str("");
   }

   BoomStick secondObject(std::move(firstObject));

   for (int i = 0; i < 100; i++) {
      sS << "request " << i;
      std::string reply = secondObject.Send(sS.str());
      sS << " reply";
      ASSERT_EQ(sS.str(), reply);
      sS.str("");
   }

   target.EndListendAndRepeat();
}
TEST_F(BoomStickTest, MoveAssignment) {
   BoomStick firstObject(mAddress);
   MockSkelleton target(mAddress);

   ASSERT_TRUE(target.Initialize());
   ASSERT_TRUE(firstObject.Initialize());

   target.BeginListenAndRepeat();
   std::stringstream sS;

   for (int i = 0; i < 100; i++) {
      sS << "request " << i;
      std::string reply = firstObject.Send(sS.str());
      sS << " reply";
      ASSERT_EQ(sS.str(), reply);
      sS.str("");
   }

   BoomStick secondObject("abc123");
   secondObject = std::move(firstObject);

   for (int i = 0; i < 100; i++) {
      sS << "request " << i;
      std::string reply = secondObject.Send(sS.str());
      sS << " reply";
      ASSERT_EQ(sS.str(), reply);
      sS.str("");
   }

   target.EndListendAndRepeat();
}
TEST_F(BoomStickTest, Swap) {
   BoomStick firstObject(mAddress);
   MockSkelleton target(mAddress);

   ASSERT_TRUE(target.Initialize());
   ASSERT_TRUE(firstObject.Initialize());

   target.BeginListenAndRepeat();
   std::stringstream sS;

   for (int i = 0; i < 100; i++) {
      sS << "request " << i;
      std::string reply = firstObject.Send(sS.str());
      sS << " reply";
      ASSERT_EQ(sS.str(), reply);
      sS.str("");
   }

   BoomStick secondObject("abc123");
   secondObject.Swap(firstObject);
   ASSERT_EQ("",firstObject.Send("test"));
   for (int i = 0; i < 100; i++) {
      sS << "request " << i;
      std::string reply = secondObject.Send(sS.str());
      sS << " reply";
      ASSERT_EQ(sS.str(), reply);
      sS.str("");
   }
   
   target.EndListendAndRepeat();
}
#endif