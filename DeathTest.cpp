#include <thread>
#include <future>

#include "DeathTest.h"
#include "Death.h"

extern std::unique_ptr<g2LogWorker> g2logger;
bool DeathTest::ranEcho(false);
std::vector<Death::DeathCallbackArg> DeathTest::stringsEchoed;
int DeathTest::ranTimes(0);

void DeathTest::EchoTheString(const std::string& theString) {
   stringsEchoed.push_back(theString);
   ranEcho = true;
}

void DeathTest::RaceTest(const Death::DeathCallbackArg& theString) {
   if (!ranEcho) {
      std::this_thread::sleep_for(std::chrono::nanoseconds(rand() % 16000));
      ranEcho = !ranEcho;
      stringsEchoed.push_back(theString);
      
   }
   ranTimes++;
}

TEST(DeathTest, VerifySingleton) {
   Death & instance1(Death::Instance());
   Death & instance2(Death::Instance());

   ASSERT_EQ(&instance1, &instance2);
}

TEST(DeathTest, VerifyReceiveCheck) {
   RaiiDeathCleanup cleanup;
   Death::Instance().SetupExitHandler();

   EXPECT_FALSE(Death::Instance().WasKilled());
   CHECK(false);
   EXPECT_TRUE(Death::Instance().WasKilled());
}

TEST(DeathTest, VerifyReceiveSignal) {
   RaiiDeathCleanup cleanup;
   Death::Instance().SetupExitHandler();

   EXPECT_FALSE(Death::Instance().WasKilled());
   raise(SIGSEGV);
   EXPECT_TRUE(Death::Instance().WasKilled());
}

TEST(DeathTest, ResetWorks) {
   RaiiDeathCleanup cleanup;
   Death::Instance().SetupExitHandler();

   EXPECT_FALSE(Death::Instance().WasKilled());
   CHECK(false);
   EXPECT_TRUE(Death::Instance().WasKilled());
   Death::Instance().ClearExits();
   EXPECT_FALSE(Death::Instance().WasKilled());
}

TEST(DeathTest, RegisterSomething) {
   RaiiDeathCleanup cleanup;
   Death::Instance().SetupExitHandler();
   Death::Instance().RegisterDeathEvent(&DeathTest::EchoTheString, "test");
   EXPECT_FALSE(DeathTest::ranEcho);
   EXPECT_TRUE(DeathTest::stringsEchoed.empty());
   CHECK(false);
   EXPECT_TRUE(DeathTest::ranEcho);
   EXPECT_FALSE(DeathTest::stringsEchoed.empty());
   EXPECT_EQ("test", DeathTest::stringsEchoed[0]);
}

TEST(DeathTest, ThreadSafeTest) {
   DeathTest::ranEcho = false;
   DeathTest::ranTimes = 0;
   DeathTest::stringsEchoed.clear();
   RaiiDeathCleanup cleanup;
   Death::Instance().SetupExitHandler();
   
   EXPECT_FALSE(DeathTest::ranEcho);
   EXPECT_TRUE(DeathTest::stringsEchoed.empty());
   
   auto ManyThreads = [&]() {
      Death::Instance().RegisterDeathEvent(&DeathTest::RaceTest, "race");
   };
   std::vector<std::future<void>> waitingPromises;
   for (int i = 0; i < 10000; i++) {
      waitingPromises.push_back(std::async(std::launch::async, ManyThreads));
   }

   CHECK(false);
   for (auto& waitFor : waitingPromises) {
      waitFor.get();
   }
   EXPECT_TRUE(DeathTest::ranEcho);
   EXPECT_FALSE(DeathTest::stringsEchoed.empty());
   EXPECT_EQ(10000, DeathTest::ranTimes);
   EXPECT_EQ(1,DeathTest::stringsEchoed.size());
   EXPECT_EQ("race", DeathTest::stringsEchoed[0]);
}
//TEST(DeathTest, ReEnableFatalExit) {

TEST(DeathTest, DISABLED_ReEnableFatalExit) {
   RaiiDeathCleanup cleanup;
   Death::Instance().SetupExitHandler();

   EXPECT_FALSE(Death::Instance().WasKilled());
   Death::Instance().EnableDefaultFatalCall(g2logger.get());
   CHECK(false);
   std::this_thread::sleep_for(std::chrono::seconds(2));
}