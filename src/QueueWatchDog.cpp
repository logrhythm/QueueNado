/*
*  File: QueueWatchDog.cpp
*  Author: Craig Cogdill
*  Date Created: August 18, 2015 1:29PM
*/
#include <StopWatch.h>
#include <future>
#include <iostream>
#include <chrono>
#include "QueueWatchDog.h"
#include <Listener.h>
#include <g3log/g3log.hpp>
#include <czmq.h>
/*
* The thread to be spawned in the constructor. Should check the given notification
*   queue every "secondsToCheck" seconds. If it receives the notify message,
*   it should raise the system signal passed into it, and then send
*   the notifier a confirmation message
*
* @param atomic<bool> whether or not to continuing checking the queue
* @param string caller, the process that read from the queue (Logger or Reader)
* @param size_t secondsToCheckQueue, how often to check the queue so that we aren't
*                                    constantly polling
* @param string listenerNotificationQueue, queue to read from
* @param string listenerHandshakeQueue, queue to send confirmation to
*/
void QueueWatchDog::QueueWatchDogThread(std::shared_ptr<std::atomic<bool>> keepRunning,
                                        const std::string caller,
                                        const size_t secondsToCheckQueue,
                                        const int signalFlag,
                                        const std::string listenerNotificationQueue,
                                        const std::string listenerHandshakeQueue) {

  StopWatch timer;
  std::unique_ptr<Listener> listener = Listener::CreateListener(listenerNotificationQueue,
                                                                listenerHandshakeQueue,
                                                                caller);

  bool notificationReceived = false;
  StopWatch queueTimer;
  while(keepRunning->load() && !zctx_interrupted && !notificationReceived) {
    if (queueTimer.ElapsedSec() < secondsToCheckQueue) {
      std::this_thread::sleep_for(std::chrono::seconds(1));
    } else {
      queueTimer.Restart();
      if (listener->NotificationReceived()) {
        notificationReceived = true;
        bool confirmed = listener->SendConfirmation();
        if (!confirmed) {
          LOG(FATAL) << "The " << caller << " listener failed to send a confirmation";
        }
        raise(signalFlag);
      }
    }
  }
}

/**
 * Constructor for a queue watchdog. It spins up a detached thread
 * that will raise a system signal if notified to do so by the parent
 * thread.
 *
 * @param name of the caller
 * @param how often to check the queue, in seconds
 * @param the signal the watchdog should raise
 * @param the queue that it will poll
 * @param the queue that it will confirm back to the parent
 */
 QueueWatchDog::QueueWatchDog(const std::string& caller, const size_t secondsToCheckQueue, const int signalToRaise,
                              const std::string& listenerNotificationQueue, const std::string& listenerHandshakeQueue)
  : mKeepRunning(new std::atomic<bool>(true))
  , mExited(std::async(std::launch::async,
                       &QueueWatchDog::QueueWatchDogThread,
                       this,
                       mKeepRunning,
                       caller,
                       secondsToCheckQueue,
                       signalToRaise,
                       listenerNotificationQueue,
                       listenerHandshakeQueue)) {
  }

/**
 * Destructor. Signals the detached thread that it should exit, then wait for the thread to join
 */
QueueWatchDog::~QueueWatchDog() {
  mKeepRunning->store(false);
  mExited.wait();
}
