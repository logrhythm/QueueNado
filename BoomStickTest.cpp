
#include "BoomStickTest.h"
#include "MockSkelleton.h"
#include "MockBoomStick.h"
#include <unordered_set>
#include <memory>
#ifdef LR_DEBUG
namespace {

   void runIterations(BoomStick& stick,int iterations) {
      std::stringstream sS;
      for (int i = 0; i < iterations; i++) {
         sS << "request " << i;
         std::string reply = stick.Send(sS.str());
         sS << " reply";
         if(sS.str()!= reply) {
            FAIL();
         }
         sS.str("");
      }
   }
}

TEST_F(BoomStickTest, Construct) {
   BoomStick stick{mAddress};
   std::unique_ptr<BoomStick> pStick(new BoomStick(mAddress));
}

TEST_F(BoomStickTest, SingleTargetSingleShooterSync) {
   BoomStick stick{mAddress};
   MockSkelleton target{mAddress};

   ASSERT_TRUE(target.Initialize());
   ASSERT_TRUE(stick.Initialize());

   target.BeginListenAndRepeat();
   
   runIterations(stick,100);

   target.EndListendAndRepeat();

}

void Shooter(int threadId, int repitions, const std::string& address) {
   BoomStick stick{address};
   ASSERT_TRUE(stick.Initialize());
   runIterations(stick,100);
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
   runIterations(firstObject,100);

   BoomStick secondObject(std::move(firstObject));

   runIterations(secondObject,100);

   target.EndListendAndRepeat();
}

TEST_F(BoomStickTest, MoveAssignment) {
   BoomStick firstObject{mAddress};
   MockSkelleton target{mAddress};

   ASSERT_TRUE(target.Initialize());
   ASSERT_TRUE(firstObject.Initialize());

   target.BeginListenAndRepeat();
   runIterations(firstObject,100);

   BoomStick secondObject{"abc123"};
   secondObject = std::move(firstObject);
   runIterations(secondObject,100);

   target.EndListendAndRepeat();
}

TEST_F(BoomStickTest, Swap) {
   BoomStick firstObject{mAddress};
   MockSkelleton target{mAddress};

   ASSERT_TRUE(target.Initialize());
   ASSERT_TRUE(firstObject.Initialize());

   target.BeginListenAndRepeat();
   runIterations(firstObject,100);

   BoomStick secondObject{"abc123"};
   secondObject.Swap(firstObject);
   ASSERT_EQ("", firstObject.Send("test"));
   runIterations(secondObject,100);

   target.EndListendAndRepeat();
}
#endif