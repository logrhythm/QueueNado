/*
 * File:   QueueWatchDogTest.cpp
 * Author: Craig Cogdill
 * Date Created: August 18, 2015 1:14PM
 */
#include "QueueWatchDogTest.h"
#include "QueueWatchDog.h"
#include <Notifier.h>
#include <chrono>
#include <g3log/g3log.hpp>
#include <StopWatch.h>

namespace QueueWatchDogTestHelpers {
   const int DUMMY_CRASH_SIGNAL = 35;
   const size_t waitTillExitInSeconds = 1;
   const std::chrono::milliseconds duration(50);
   const std::string ipcPrefix = "ipc:///tmp/";
   const std::string notifierQueueName = "QueueWatchDogTest.ipc";
   const std::string notifierHandshakeQueueName = "QueueWatchDogTestHandshake.ipc";
   std::string notifierQueue = ipcPrefix + notifierQueueName;
   std::string notifierHandshakeQueue = ipcPrefix + notifierHandshakeQueueName;
   std::atomic<bool> system_saw_signal(false);
   const size_t numberOfListeners = 1;

   void UserSignalHandler(int signalValue) {
     LOG(INFO) << "Test caught signal " << signalValue;
     system_saw_signal.store(true);
   }

   void SleepTestMS(unsigned int milliseconds) {
      StopWatch timer;
      while (timer.ElapsedMs() < milliseconds) {
         std::this_thread::sleep_for(duration);
      }
   }

}

TEST_F(QueueWatchDogTest, CatchUserSignal_withNotifier) {
   std::signal(35, QueueWatchDogTestHelpers::UserSignalHandler);

   // Set up a notifier
   std::unique_ptr<Notifier> notifier = Notifier::CreateNotifier(QueueWatchDogTestHelpers::notifierQueue,
                                                                 QueueWatchDogTestHelpers::notifierHandshakeQueue,
                                                                 QueueWatchDogTestHelpers::numberOfListeners);

   // Verify that the user signal 35 has not been seen
   EXPECT_FALSE(QueueWatchDogTestHelpers::system_saw_signal.load());
   {
      // Spawn the WatchDog thread and give it a little time to spin up
      QueueWatchDog restartWD(__FUNCTION__,
                              QueueWatchDogTestHelpers::waitTillExitInSeconds,
                              QueueWatchDogTestHelpers::DUMMY_CRASH_SIGNAL,
                              QueueWatchDogTestHelpers::notifierQueue,
                              QueueWatchDogTestHelpers::notifierHandshakeQueue);
      QueueWatchDogTestHelpers::SleepTestMS(100);

      // Notify the WatchDog that it is time to raise the signal
      EXPECT_EQ(notifier->Notify(), QueueWatchDogTestHelpers::numberOfListeners);
   }
   // The WatchDog should have received the notification, raised
   //    the signal, and our signal catcher should have caught it
   //    and set the atomic<bool>
   EXPECT_TRUE(QueueWatchDogTestHelpers::system_saw_signal.load());
}
