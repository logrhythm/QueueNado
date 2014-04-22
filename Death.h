#pragma once

#include <string>
#include <g2log.hpp>
#include <mutex>
#include <vector>
#include <functional>

/**
 * By calling @ref UseDeathHandler all CHECK, LOG(FATAL) or fatal signals will be caught by g2log
 *  but will instead of exiting the test/application call @ref Death::Received
 *
 * The reason for using this instead of Google's gtest DEATH framework is that the DEATH framework
 *  will do popen/fork which clashes with our use of the MotherForker.
 */
class Death {
public:
   typedef std::string DeathCallbackArg;
   typedef void (*DeathCallbackType)(const DeathCallbackArg& arg);

   static Death& Instance();
   static void ClearExits();
   static bool WasKilled();
   static void SetupExitHandler();
   static std::string Message();
   static void RegisterDeathEvent(DeathCallbackType deathFunction, const DeathCallbackArg& deathArg);
   static void EnableDefaultFatalCall(std::shared_ptr<g2LogWorker> loggerWorker);
   static void DeleteIpcFiles(const DeathCallbackArg& binding);
private:
   Death();
   Death(Death&) = delete;
   Death& operator=(Death&) = delete;
   static void Received(g2::internal::FatalMessage death);
   
   bool mReceived;
   std::string mMessage;
   std::mutex mListLock;
   std::vector<std::pair<DeathCallbackType,DeathCallbackArg> > mShutdownFunctions;
   bool mEnableDefaultFatal;
   std::weak_ptr<g2LogWorker> mWorker;
};

/** Makes sure that any Death tests will be cleaned up at test exit
 * Please add this to your test
 *
 * TEST_F(Something, Something) {
 * RaiiDeathCleanup cleanup;
 * ... // test logic
 * }
 */
struct RaiiDeathCleanup {

   ~RaiiDeathCleanup() {
      Death::ClearExits();
   }
};

