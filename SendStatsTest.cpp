#include "SendStatsTest.h"
#include "MockSendStats.h"

TEST_F(SendStatsTest, ConstructAndInitialize) {
   SendStats* pSender = new SendStats();
   delete pSender;

   EXPECT_FALSE(mSender.SendStat("foo", (double) 1.0, true));
   EXPECT_FALSE(mSender.SendStat("foo", (float) 1.0, true));
   EXPECT_FALSE(mSender.SendStat("foo", (int) 1, true));
   EXPECT_FALSE(mSender.SendStat("foo", "name", (int) 1, true));
   EXPECT_FALSE(mSender.SendStat("foo", (unsigned int) 1, true));
   EXPECT_FALSE(mSender.SendStat("foo", (uint64_t) 1, true));
   EXPECT_FALSE(mSender.SendStat("foo", (int64_t) 1, true));
   EXPECT_FALSE(mSender.SendStat("foo", "name", (int64_t) 1, "columnName", true));

   EXPECT_TRUE(mSender.Initialize(mStatsIpcLocation));
   EXPECT_TRUE(mSender.SendStat("foo", (double) 1.0, true));
   EXPECT_TRUE(mSender.SendStat("foo", (float) 1.0, true));
   EXPECT_TRUE(mSender.SendStat("foo", (int) 1, true));
   EXPECT_TRUE(mSender.SendStat("foo", (unsigned int) 1, true));
   EXPECT_TRUE(mSender.SendStat("foo", (uint64_t) 1, true));
   EXPECT_TRUE(mSender.SendStat("foo", (int64_t) 1, true));
   EXPECT_TRUE(mSender.SendStat("foo", "name", (int) 1, true));
   EXPECT_TRUE(mSender.SendStat("foo", "name", (int) 1, "columnName", true));
}

TEST_F(SendStatsTest, CorrectTimeIncludedInStats) {
#ifdef LR_DEBUG
   MockSendStats fakeSender;
   fakeSender.mDummyTimeStamp = 1234;

   ASSERT_TRUE(fakeSender.Initialize(mStatsIpcLocation));

   protoMsg::Stats intStatNamedMessage(fakeSender.ConstructMessageToSend("foo", "name", (int64_t) 1));
   ASSERT_EQ("foo", intStatNamedMessage.key());
   ASSERT_EQ(1234, intStatNamedMessage.time());
   ASSERT_EQ("name", intStatNamedMessage.statname());
   ASSERT_TRUE(intStatNamedMessage.has_statname());
   ASSERT_TRUE(intStatNamedMessage.has_longvalue());
   ASSERT_EQ(1, intStatNamedMessage.longvalue());

   protoMsg::Stats int64Message(fakeSender.ConstructMessageToSend("foo", (int64_t) 1));
   ASSERT_EQ("foo", int64Message.key());
   ASSERT_EQ(1234, int64Message.time());
   ASSERT_FALSE(int64Message.has_statname());
   ASSERT_TRUE(int64Message.has_longvalue());
   ASSERT_EQ(1, int64Message.longvalue());
   
   protoMsg::Stats statMessageWithColumn(fakeSender.ConstructMessageToSend("foo", "name" , (int64_t) 1, "columnName"));
   ASSERT_EQ("foo", statMessageWithColumn.key());
   ASSERT_EQ(1234, statMessageWithColumn.time());
   ASSERT_EQ("name", statMessageWithColumn.statname());
   ASSERT_TRUE(statMessageWithColumn.has_longvalue());
   ASSERT_EQ(1, statMessageWithColumn.longvalue());
   ASSERT_EQ("columnName", statMessageWithColumn.columnname());
#endif
}

std::string SendStatsTest::GetIpcLocation() {
   int pid = getpid();
   std::string ipcLocation("ipc:///tmp/");
   ipcLocation.append("SendStatsTests");
   ipcLocation.append(boost::lexical_cast<std::string > (pid));
   ipcLocation.append(".ipc");
   return ipcLocation;
}

/**
 * Start a vampire thread and pull off the set number of messages.
 * @param numberOfMessages
 * @param location
 * @param expectedData
 * @param hwm
 * @param ioThreads
 */
void SendStatsTest::VampireThread(int numberOfMessages,
   std::string& location, std::string& expectedData, int hwm, int ioThreads) {
   Vampire vampire(location);
   vampire.SetHighWater(hwm);
   vampire.SetIOThreads(ioThreads);
   vampire.SetOwnSocket( true );
   ASSERT_TRUE(vampire.PrepareToBeShot());
   for (int i=0; i < numberOfMessages; i++) {
      std::string bullet;
      if (vampire.GetShot(bullet, 2000)) {
         EXPECT_EQ(bullet, expectedData);
      } else {
         //no shot in time try again
         i--;
      }
   }
   while (1) {
      boost::this_thread::sleep(boost::posix_time::seconds(1));
   }
}


TEST_F(SendStatsTest, VerifySendStatsDoubleMessage) {
#ifdef LR_DEBUG
   int nIOThreads = 1;
   int vampireHWM = 10;
   int nShotsPerVampire = 1;

   SendStats statSender;
   EXPECT_TRUE(statSender.Initialize(mStatsIpcLocation));
   EXPECT_TRUE( statSender.SendStat("double", (double) 1.0, true) );
   std::string expectedMsg;
   expectedMsg = statSender.GetStatMessage();

   boost::thread* aVampire = new boost::thread(&SendStatsTest::VampireThread,
         this, nShotsPerVampire, mStatsIpcLocation, expectedMsg, vampireHWM, nIOThreads);

   sleep( 1 );
   aVampire->interrupt();
   aVampire->join();
   delete aVampire;
#endif
}

TEST_F(SendStatsTest, VerifySendStatsFloatMessage) {
#ifdef LR_DEBUG
   int nIOThreads = 1;
   int vampireHWM = 10;
   int nShotsPerVampire = 1;

   SendStats statSender;
   EXPECT_TRUE(statSender.Initialize(mStatsIpcLocation));
   EXPECT_TRUE( statSender.SendStat("float", (float) 1.0, true) );
   std::string expectedMsg;
   expectedMsg = statSender.GetStatMessage();

   boost::thread* aVampire = new boost::thread(&SendStatsTest::VampireThread,
         this, nShotsPerVampire, mStatsIpcLocation, expectedMsg, vampireHWM, nIOThreads);

   sleep( 1 );
   aVampire->interrupt();
   aVampire->join();
   delete aVampire;
#endif
}

TEST_F(SendStatsTest, VerifySendStatsIntMessage) {
#ifdef LR_DEBUG
   int nIOThreads = 1;
   int vampireHWM = 10;
   int nShotsPerVampire = 1;

   SendStats statSender;
   EXPECT_TRUE(statSender.Initialize(mStatsIpcLocation));
   EXPECT_TRUE( statSender.SendStat("int", (int) 1, true) );
   std::string expectedMsg;
   expectedMsg = statSender.GetStatMessage();

   boost::thread* aVampire = new boost::thread(&SendStatsTest::VampireThread,
         this, nShotsPerVampire, mStatsIpcLocation, expectedMsg, vampireHWM, nIOThreads);

   sleep( 1 );
   aVampire->interrupt();
   aVampire->join();
   delete aVampire;
#endif
}

TEST_F(SendStatsTest, VerifySendStatsOptionNameIntMessage) {
#ifdef LR_DEBUG
   int nIOThreads = 1;
   int vampireHWM = 10;
   int nShotsPerVampire = 1;

   SendStats statSender;
   EXPECT_TRUE(statSender.Initialize(mStatsIpcLocation));
   EXPECT_TRUE( statSender.SendStat("Option Name int", (int) 1, true) );
   std::string expectedMsg;
   expectedMsg = statSender.GetStatMessage();

   boost::thread* aVampire = new boost::thread(&SendStatsTest::VampireThread,
         this, nShotsPerVampire, mStatsIpcLocation, expectedMsg, vampireHWM, nIOThreads);

   sleep( 1 );
   aVampire->interrupt();
   aVampire->join();
   delete aVampire;
#endif
}

TEST_F(SendStatsTest, VerifySendStatsUnsignedIntMessage) {
#ifdef LR_DEBUG
   int nIOThreads = 1;
   int vampireHWM = 10;
   int nShotsPerVampire = 1;

   SendStats statSender;
   EXPECT_TRUE(statSender.Initialize(mStatsIpcLocation));
   EXPECT_TRUE( statSender.SendStat("unsigned int", (unsigned int) 1, true) );
   std::string expectedMsg;
   expectedMsg = statSender.GetStatMessage();

   boost::thread* aVampire = new boost::thread(&SendStatsTest::VampireThread,
         this, nShotsPerVampire, mStatsIpcLocation, expectedMsg, vampireHWM, nIOThreads);

   sleep( 1 );
   aVampire->interrupt();
   aVampire->join();
   delete aVampire;
#endif
}

TEST_F(SendStatsTest, VerifySendStatsUint64Message) {
#ifdef LR_DEBUG
   int nIOThreads = 1;
   int vampireHWM = 10;
   int nShotsPerVampire = 1;

   SendStats statSender;
   EXPECT_TRUE(statSender.Initialize(mStatsIpcLocation));
   EXPECT_TRUE( statSender.SendStat("uint64_t", "name", (uint64_t) 1, "columnName", true) );
   std::string expectedMsg;
   expectedMsg = statSender.GetStatMessage();

   boost::thread* aVampire = new boost::thread(&SendStatsTest::VampireThread,
         this, nShotsPerVampire, mStatsIpcLocation, expectedMsg, vampireHWM, nIOThreads);

   sleep( 1 );
   aVampire->interrupt();
   aVampire->join();
   delete aVampire;
#endif
}

TEST_F(SendStatsTest, VerifySendStatsInt64Message) {
#ifdef LR_DEBUG
   int nIOThreads = 1;
   int vampireHWM = 10;
   int nShotsPerVampire = 1;

   SendStats statSender;
   EXPECT_TRUE(statSender.Initialize(mStatsIpcLocation));
   EXPECT_TRUE( statSender.SendStat("int64_t", (int64_t) 1, true) );
   std::string expectedMsg;
   expectedMsg = statSender.GetStatMessage();

   boost::thread* aVampire = new boost::thread(&SendStatsTest::VampireThread,
         this, nShotsPerVampire, mStatsIpcLocation, expectedMsg, vampireHWM, nIOThreads);

   sleep( 1 );
   aVampire->interrupt();
   aVampire->join();
   delete aVampire;
#endif
}

