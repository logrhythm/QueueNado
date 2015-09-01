#pragma once
#include <memory>
#include <vector>
#include <atomic>
#include <mutex>
#include <StopWatch.h>
#include <thread>
#include <g2log.hpp>
#include <czmq.h>
#include <gtest/gtest.h>

namespace {
const size_t kMaxWaitTimeInSec = 15;


struct TestThreadData {
   std::mutex sharing;
   std::shared_ptr<std::atomic<bool>> keepRunning;
   std::shared_ptr<std::atomic<bool>> received;
   std::shared_ptr<std::atomic<bool>> hasStarted;
   std::shared_ptr<std::atomic<bool>> hasExited;
   std::shared_ptr<std::atomic<bool>> timeToNotify;
   std::shared_ptr<std::vector<std::string>> messages;
   const size_t kExpectedFeedback;
   const std::string kName;

   TestThreadData(const std::string& name, size_t expectedFeedback = 0) : keepRunning(new std::atomic<bool>()),
      received(new std::atomic<bool>()),
      hasStarted(new std::atomic<bool>()),
      hasExited(new std::atomic<bool>()),
      timeToNotify(new std::atomic<bool>()),
      messages(new std::vector<std::string>()),
      kExpectedFeedback(expectedFeedback),
      kName(name) {
      reset();
   }

   TestThreadData(const TestThreadData& copy)
      : keepRunning(copy.keepRunning),
        received(copy.received),
        hasStarted(copy.hasStarted),
        hasExited(copy.hasExited),
        timeToNotify(copy.timeToNotify),
        messages(copy.messages),
        kExpectedFeedback(copy.kExpectedFeedback),
        kName(copy.kName) {}

   ~TestThreadData() = default;

   void reset() {
      keepRunning->store(true);
      received->store(false);
      hasStarted->store(false);
      hasExited->store(false);
      timeToNotify->store(false);
      messages.reset(new std::vector<std::string>());
   }
   std::string print() {
      std::ostringstream out;
      out << kName << " received: " << received->load()
          << "   keepRunning:  " << keepRunning->load()
          << "   hasStarted:   " << hasStarted->load()
          << "   hasExited:    " << received->load()
          << "   timeToNotify: " << timeToNotify->load();

      return out.str();
   }
};

void NotifyParentThatChildHasStarted(TestThreadData& threadData) {
   threadData.hasStarted->store(true);
}

bool MaxTimeoutHasOccurred(StopWatch& timeSinceStart) {
   return (timeSinceStart.ElapsedSec() >= kMaxWaitTimeInSec);
}

void UpdateThreadDataAfterReceivingMessage(TestThreadData& threadData) {
  threadData.received->store(true);
}

void UpdateThreadDataAfterReceivingMessage(TestThreadData& threadData, std::vector<std::string> messages) {
   std::lock_guard<std::mutex> lock(threadData.sharing);
   threadData.messages->clear();
   *(threadData.messages) = messages;
   threadData.received->store(true);
 }

bool SleepUntilCondition(std::vector<std::shared_ptr<std::atomic<bool>>> conditions) {
   StopWatch timer;

   auto allTrue = [&] {
      for (auto& check : conditions) {
         if (check->load() == false) {
            return false;
         }
      }
      return true;
   };

   while (!allTrue() && !MaxTimeoutHasOccurred(timer)) {
      std::this_thread::sleep_for(std::chrono::milliseconds(50));
   }

   if (MaxTimeoutHasOccurred(timer)) {
      ADD_FAILURE() << "SleepUntilCondition timed out after " << kMaxWaitTimeInSec << " seconds.";
      return false;
   }
   return true;
}

bool SleepUntilConditionIsFalse(std::vector<std::shared_ptr<std::atomic<bool>>> conditions) {
   StopWatch timer;

   auto allFalse = [&] {
      for (auto& check : conditions) {
         if (check->load() == true) {
            return false;
         }
      }
      return true;
   };

   while (!allFalse() && !MaxTimeoutHasOccurred(timer)) {
      std::this_thread::sleep_for(std::chrono::milliseconds(50));
   }

   if (MaxTimeoutHasOccurred(timer)) {
      ADD_FAILURE() << "SleepUntilConditionIsFalse timed out after " << kMaxWaitTimeInSec << " seconds.";
      return false;
   }
   return true;
}

std::vector<std::string>& GetMessages(TestThreadData& threadData) {
   std::lock_guard<std::mutex> lock(threadData.sharing);
   return *threadData.messages;
 }


void SendShutdownSignalToThread(TestThreadData& threadData) {
   threadData.keepRunning->store(false);
}

std::shared_ptr<std::atomic<bool>> ParentDecidesWhenToExit(TestThreadData& threadData) {
   return threadData.keepRunning;
}

void ShutdownThreads(std::vector<TestThreadData> threads) {
   for (TestThreadData& threadData : threads) {
      SendShutdownSignalToThread(threadData);
   }
}

bool ThreadIsShutdown(TestThreadData& threadData) {
   return threadData.hasExited->load();
}

void FireOffANotification(TestThreadData& threadData) {
   threadData.timeToNotify->store(true);
}

bool TimeToSendANotification(TestThreadData& threadData) {
   return (threadData.timeToNotify->load());
}

bool ParentHasNotSentExitSignal (TestThreadData& threadData) {
   return (threadData.keepRunning->load());
}

void ResetNotifyFlag(TestThreadData& threadData) {
   threadData.timeToNotify->store(false);
}
}
