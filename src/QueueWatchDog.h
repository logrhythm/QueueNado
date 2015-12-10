/*
*  File: QueueWatchDog.h
*  Author: Craig Cogdill
*  Date Created: August 18, 2015 1:27PM
*/
#pragma once
#include <string>
#include <atomic>
#include <memory>
#include <future>
#include <iostream>

class QueueWatchDog {
public:
   QueueWatchDog(const std::string& caller,
                 const size_t secondsToCheckQueue,
                 const int signalToRaise,
                 const std::string& listenerNotificationQueue,
                 const std::string& listenerHandshakeQueue);
   virtual ~QueueWatchDog();

   QueueWatchDog(const QueueWatchDog&) = delete;
   QueueWatchDog& operator=(const QueueWatchDog&) = delete;

   virtual void QueueWatchDogThread(std::shared_ptr<std::atomic<bool>> keepRunning,
                                       const std::string caller,
                                       const size_t secondsToCheckQueue,
                                       const int singalFlag,
                                       const std::string listenerNotificationQueue,
                                       const std::string listenerHandshakeQueue);
private:
   std::shared_ptr<std::atomic<bool>> mKeepRunning;
   std::future<void> mExited;
};
