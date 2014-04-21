#include "Death.h"
#include <g2logworker.hpp>

/**
 * Singleton Instance Method
 * @return 
 */
Death& Death::Instance() {
   static Death gInstance;

   return gInstance;
}

Death::Death() : mReceived(false), mMessage {
   ""
}, mEnableDefaultFatal(false), mWorker(nullptr)
{

}

/**
 * In order to re-enable the default handler you must re-supply the worker 
 * @param loggerWorker
 */
void Death::EnableDefaultFatalCall(g2LogWorker* loggerWorker) {
   Instance().mEnableDefaultFatal = true;
   Instance().mWorker = loggerWorker;
}
/// @param death message with any captured death details

void Death::Received(g2::internal::FatalMessage death) {
   Death& instance = Instance();
   instance.mReceived = true;
   instance.mMessage = death.message_;
   std::lock_guard<std::mutex> glock(instance.mListLock);
   for (const auto& deathFunction : instance.mShutdownFunctions) {
      (deathFunction.first)(deathFunction.second);
   }
   if (instance.mEnableDefaultFatal && nullptr != instance.mWorker ) {
      instance.mWorker->fatal(death);
   }
}

/**
 * Register a DeathCallback into the set of functions that will be called
 * @return 
 */
void Death::RegisterDeathEvent(DeathCallbackType deathFunction, const DeathCallbackArg& deathArg) {
   Death& instance = Instance();
   std::lock_guard<std::mutex> glock(instance.mListLock);
   instance.mShutdownFunctions.push_back(std::make_pair(deathFunction,deathArg));
}

bool Death::WasKilled() {
   Death& instance = Instance();
   return instance.mReceived;
}

/// Please call this if you plan on doing DEATH tests. 

void Death::SetupExitHandler() {
   g2::internal::changeFatalInitHandlerForUnitTesting(Death::Received);
}

void Death::ClearExits() {
   Death& instance = Instance();
   instance.mReceived = false;
   instance.mMessage = "";
   std::lock_guard<std::mutex> glock(instance.mListLock);
   instance.mShutdownFunctions.clear();
}

 std::string Death::Message() {
    return Instance().mMessage;
 }

