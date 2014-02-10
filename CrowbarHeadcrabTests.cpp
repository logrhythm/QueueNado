#include "CrowbarHeadcrabTests.h"
#include "QosmosFileCallbacks.h"
#include <czmq.h>
#include <boost/thread.hpp>

TEST_F(CrowbarHeadcrabTests, CrowbarBrokenSocket) {
   Crowbar firstCrowbar(mTarget);
   
   EXPECT_FALSE(firstCrowbar.Swing("foo"));
   ASSERT_TRUE(firstCrowbar.Wield());
   EXPECT_TRUE(firstCrowbar.Swing("foo"));
   EXPECT_FALSE(firstCrowbar.Swing("foo"));

}
TEST_F(CrowbarHeadcrabTests, SimpleConstructAndWieldCrowbar) {

   Crowbar firstStack(mTarget);
   ASSERT_TRUE(NULL == firstStack.GetContext());
   Crowbar* firstHeap = new Crowbar(mTarget);
   delete firstHeap;
   ASSERT_TRUE(firstStack.Wield());
   ASSERT_TRUE(NULL != firstStack.GetContext());
}

TEST_F(CrowbarHeadcrabTests, SimpleConstructAndPrepHeadcrab) {

   Headcrab firstStack(mTarget);
   EXPECT_EQ(mTarget, firstStack.GetBinding());
   EXPECT_EQ(NULL, firstStack.GetContext());
   Headcrab* firstHeap = new Headcrab(mTarget);
   delete firstHeap;

   ASSERT_TRUE(firstStack.ComeToLife());
   ASSERT_TRUE(NULL != firstStack.GetContext());

}

TEST_F(CrowbarHeadcrabTests, ConstructCrowbarinResponseToHeadcrab) {

   Headcrab target(mTarget);
   EXPECT_EQ(mTarget, target.GetBinding());
   EXPECT_EQ(NULL, target.GetContext());

   ASSERT_TRUE(target.ComeToLife());
   ASSERT_TRUE(NULL != target.GetContext());

   Crowbar shooter(target);

   ASSERT_TRUE(shooter.Wield());
   ASSERT_EQ(target.GetContext(), shooter.GetContext());

   Headcrab* targetHeap = new Headcrab(mTarget);
   ASSERT_TRUE(targetHeap->ComeToLife());

   Crowbar* shooterHeap = new Crowbar(*targetHeap);
   delete shooterHeap;
   delete targetHeap;
}

TEST_F(CrowbarHeadcrabTests, ConstructCrowbarwithContextandBinding) {

   Headcrab target(mTarget);
   EXPECT_EQ(mTarget, target.GetBinding());
   EXPECT_EQ(NULL, target.GetContext());

   ASSERT_TRUE(target.ComeToLife());
   ASSERT_TRUE(NULL != target.GetContext());

   Crowbar shooter(target.GetBinding(), target.GetContext());

   ASSERT_TRUE(shooter.Wield());
   ASSERT_EQ(target.GetContext(), shooter.GetContext());

   Headcrab* targetHeap = new Headcrab(mTarget);
   ASSERT_TRUE(targetHeap->ComeToLife());

   Crowbar* shooterHeap = new Crowbar(targetHeap->GetBinding(), targetHeap->GetContext());
   delete shooterHeap;
   delete targetHeap;
}

TEST_F(CrowbarHeadcrabTests, SmashAHeadcrab) {

   Headcrab target(mTarget);
   EXPECT_EQ(mTarget, target.GetBinding());
   EXPECT_EQ(NULL, target.GetContext());

   ASSERT_TRUE(target.ComeToLife());
   ASSERT_TRUE(NULL != target.GetContext());

   Crowbar shooter(target);

   ASSERT_TRUE(shooter.Wield());
   ASSERT_EQ(target.GetContext(), shooter.GetContext());

   std::string expected("abc123");
   std::string bullet = expected;
   std::string wound;
   ASSERT_FALSE(target.GetHitWait(wound, 1000));
   ASSERT_TRUE(shooter.Swing(bullet));
   ASSERT_TRUE(target.GetHitWait(wound, 1000));
   ASSERT_TRUE(target.SendSplatter(wound));
   ASSERT_TRUE(shooter.WaitForKill(bullet, 1000));

   ASSERT_EQ(expected, wound);
   ASSERT_EQ(expected, bullet);
   ASSERT_FALSE(bullet.empty());
   zclock_sleep(100);

}

TEST_F(CrowbarHeadcrabTests, SmashAHeadcrabBlocking) {

   Headcrab target(mTarget);
   EXPECT_EQ(mTarget, target.GetBinding());
   EXPECT_EQ(NULL, target.GetContext());

   ASSERT_TRUE(target.ComeToLife());
   ASSERT_TRUE(NULL != target.GetContext());

   Crowbar shooter(target);

   ASSERT_TRUE(shooter.Wield());
   ASSERT_EQ(target.GetContext(), shooter.GetContext());

   std::string expected("abc123");
   std::string bullet = expected;
   std::string wound;

   ASSERT_TRUE(shooter.Swing(bullet));
   ASSERT_TRUE(target.GetHitBlock(wound));
   ASSERT_TRUE(target.SendSplatter(wound));

   ASSERT_TRUE(shooter.BlockForKill(bullet));
   ASSERT_EQ(expected, wound);
   ASSERT_EQ(expected, bullet);
   ASSERT_FALSE(bullet.empty());
   zclock_sleep(100);

}

TEST_F(CrowbarHeadcrabTests, SmashAHeadcrabMulti) {

   Headcrab target(mTarget);
   EXPECT_EQ(mTarget, target.GetBinding());
   EXPECT_EQ(NULL, target.GetContext());

   ASSERT_TRUE(target.ComeToLife());
   ASSERT_TRUE(NULL != target.GetContext());

   Crowbar shooter(target);

   ASSERT_TRUE(shooter.Wield());
   ASSERT_EQ(target.GetContext(), shooter.GetContext());

   std::string expected("abc123");
   std::vector<std::string> data;
   std::string bullet = expected;
   data.push_back(bullet);
   bullet.append("again");
   data.push_back(bullet);
   std::string wound;
   std::vector<std::string> wounds;
   ASSERT_FALSE(target.GetHitWait(wounds, 1000));
   ASSERT_TRUE(shooter.Flurry(data));
   ASSERT_TRUE(target.GetHitWait(wounds, 1000));
   ASSERT_EQ(2, wounds.size());
   ASSERT_EQ(expected, wounds[0]);
   std::string expected2 = expected;
   expected2.append("again");
   ASSERT_EQ(expected2, wounds[1]);
   ASSERT_TRUE(target.SendSplatter(wounds));
   data.clear();
   ASSERT_TRUE(shooter.WaitForKill(data, 1000));
   ASSERT_EQ(2, data.size());

   ASSERT_EQ(expected, data[0]);

   ASSERT_EQ(expected2, data[1]);
   zclock_sleep(100);

}

TEST_F(CrowbarHeadcrabTests, SmashAHeadcrabBlockingMulti) {

   Headcrab target(mTarget);
   EXPECT_EQ(mTarget, target.GetBinding());
   EXPECT_EQ(NULL, target.GetContext());

   ASSERT_TRUE(target.ComeToLife());
   ASSERT_TRUE(NULL != target.GetContext());

   Crowbar shooter(target);

   ASSERT_TRUE(shooter.Wield());
   ASSERT_EQ(target.GetContext(), shooter.GetContext());

   std::string expected("abc123");
   std::vector<std::string> data;
   std::string bullet = expected;
   data.push_back(bullet);
   bullet.append("again");
   data.push_back(bullet);
   std::string wound;
   std::vector<std::string> wounds;
   ASSERT_TRUE(shooter.Flurry(data));
   ASSERT_TRUE(target.GetHitBlock(wounds));
   ASSERT_EQ(2, wounds.size());
   ASSERT_EQ(expected, wounds[0]);
   std::string expected2 = expected;
   expected2.append("again");
   ASSERT_EQ(expected2, wounds[1]);
   ASSERT_TRUE(target.SendSplatter(wounds));
   data.clear();
   ASSERT_TRUE(shooter.BlockForKill(data));
   ASSERT_EQ(2, data.size());

   ASSERT_EQ(expected, data[0]);
   ASSERT_EQ(expected2, data[1]);
   zclock_sleep(100);

}

TEST_F(CrowbarHeadcrabTests, SmashAHeadcrabInProc) {
   mTarget = "inproc://headcrabkiller";
   Headcrab target(mTarget);
   EXPECT_EQ(mTarget, target.GetBinding());
   EXPECT_EQ(NULL, target.GetContext());

   ASSERT_TRUE(target.ComeToLife());
   ASSERT_TRUE(NULL != target.GetContext());

   Crowbar shooter(target);

   ASSERT_TRUE(shooter.Wield());
   ASSERT_EQ(target.GetContext(), shooter.GetContext());

   std::string expected("abc123");
   std::string bullet = expected;
   std::string wound;
   std::vector<std::string> wounds;

   ASSERT_TRUE(shooter.Swing(bullet));
   ASSERT_TRUE(target.GetHitBlock(wounds));
   ASSERT_TRUE(target.SendSplatter(wounds));
   ASSERT_FALSE(wounds.empty());
   std::vector<std::string> data;
   ASSERT_TRUE(shooter.BlockForKill(data));
   ASSERT_FALSE(data.empty());
   bullet = data[0];
   wound = wounds[0];
   ASSERT_EQ(expected, wound);
   ASSERT_EQ(expected, bullet);
   ASSERT_FALSE(bullet.empty());
   zclock_sleep(100);

}

TEST_F(CrowbarHeadcrabTests, SmashAHeadcrabTCP) {
   mTarget = "tcp://127.0.0.1:12345";
   Headcrab target(mTarget);
   EXPECT_EQ(mTarget, target.GetBinding());
   EXPECT_EQ(NULL, target.GetContext());

   ASSERT_TRUE(target.ComeToLife());
   ASSERT_TRUE(NULL != target.GetContext());

   Crowbar shooter(target);

   ASSERT_TRUE(shooter.Wield());
   ASSERT_EQ(target.GetContext(), shooter.GetContext());

   std::string expected("abc123");
   std::string bullet = expected;
   std::string wound;
   std::vector<std::string> wounds;
   ASSERT_TRUE(shooter.Swing(bullet));
   ASSERT_TRUE(target.GetHitBlock(wounds));
   ASSERT_TRUE(target.SendSplatter(wounds));
   ASSERT_FALSE(wounds.empty());
   std::vector<std::string> data;
   ASSERT_TRUE(shooter.BlockForKill(data));
   ASSERT_FALSE(data.empty());
   bullet = data[0];
   wound = wounds[0];
   ASSERT_EQ(expected, wound);
   ASSERT_EQ(expected, bullet);
   ASSERT_FALSE(bullet.empty());
   zclock_sleep(100);


}

void CrowbarHeadcrabTests::Sender(std::string& baseData, int numberOfHits, std::string& binding) {
   Crowbar shooter(binding);
   assert(shooter.Wield());
   for (int i = 0; i < numberOfHits; i++) {
      assert(shooter.Swing(baseData));
      std::string killData;

      std::vector<std::string> data;
      ASSERT_TRUE(shooter.BlockForKill(data));
      ASSERT_FALSE(data.empty());
      killData = data[0];

      assert(killData == baseData);
   }
   while (1) {
      boost::this_thread::sleep(boost::posix_time::seconds(1));
   }
}

TEST_F(CrowbarHeadcrabTests, CrowbarSpeedTest) {

   Headcrab target(mTarget);
   EXPECT_EQ(mTarget, target.GetBinding());
   EXPECT_EQ(NULL, target.GetContext());

   ASSERT_TRUE(target.ComeToLife());
   ASSERT_TRUE(NULL != target.GetContext());

   std::string sentData(500, 'a');
   int numberOfHits = PACKETS_TO_TEST;
   boost::thread theSender(&CrowbarHeadcrabTests::Sender, this,
           sentData, numberOfHits, mTarget);
   SetExpectedTime(numberOfHits, sentData.size() * sizeof (char), 20, 10000L);
   StartTimedSection();
   for (int i = 0; i < numberOfHits; i++) {
      std::string foundData;
      std::vector<std::string> allFoundData;
      ASSERT_TRUE(target.GetHitBlock(allFoundData));
      ASSERT_FALSE(allFoundData.empty());
      ASSERT_TRUE(allFoundData[0] == sentData);
      ASSERT_TRUE(target.SendSplatter(allFoundData));
   }
   EndTimedSection();
   ASSERT_TRUE(TimedSectionPassed());
   theSender.interrupt();
   theSender.join();

}
