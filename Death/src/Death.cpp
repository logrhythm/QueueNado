
#include <g2logworker.hpp>
#include <unistd.h>

#include "Death.h"

/**
 * Singleton Instance Method
 * @return 
 */
Death& Death::Instance() {
   static Death gInstance;

   return gInstance;
}

Death::Death() : mReceived(false), mMessage {""}, mEnableDefaultFatal(false)
{

}

/**
 * The most common use case is for ZMQ sockets to want their files cleared on 
 *   fatal exits
 * @param binding
 */
void Death::DeleteIpcFiles(const DeathCallbackArg& binding) {
   auto realPathStart = binding.find("ipc://");
   if (realPathStart != std::string::npos) {
      std::string realPath(binding,realPathStart+6); // start at the end of the matched string
      unlink(realPath.c_str());
   }
}

/**
 * In order to re-enable the default handler you must re-supply the worker 
 * @param loggerWorker
 */
void Death::EnableDefaultFatalCall(std::shared_ptr<g2LogWorker> loggerWorker) {
   Death::Instance().mEnableDefaultFatal = true;
   Death::Instance().mWorker = loggerWorker; // weak pointer constructed from shared
}
/// @param death message with any captured death details

void Death::Received(g2::internal::FatalMessage death) {
   Death::Instance().mReceived = true;
   Death::Instance().mMessage = death.message_;
   std::lock_guard<std::mutex> glock(Death::Instance().mListLock);
   for (const auto& deathFunction : Death::Instance().mShutdownFunctions) {
      (deathFunction.first)(deathFunction.second);
   }
   auto lockedWorkerPointer = Death::Instance().mWorker.lock();
   if (Death::Instance().mEnableDefaultFatal && nullptr != lockedWorkerPointer.get() ) {
      lockedWorkerPointer->fatal(death);
   }
}

/**
 * Register a DeathCallback into the set of functions that will be called
 * @return 
 */
void Death::RegisterDeathEvent(DeathCallbackType deathFunction, const DeathCallbackArg& deathArg) {
   std::lock_guard<std::mutex> glock(Death::Instance().mListLock);
   Death::Instance().mShutdownFunctions.push_back(std::make_pair(deathFunction,deathArg));
}

bool Death::WasKilled() {
   return Death::Instance().mReceived;
}

/// Please call this if you plan on doing DEATH tests. 

void Death::SetupExitHandler() {
   g2::internal::changeFatalInitHandlerForUnitTesting(Death::Received);
}

void Death::ClearExits() {
   Death::Instance().mReceived = false;
   Death::Instance().mMessage = "";
   std::lock_guard<std::mutex> glock(Death::Instance().mListLock);
   Death::Instance().mShutdownFunctions.clear();
}

 std::string Death::Message() {
    return Death::Instance().mMessage;
 }

