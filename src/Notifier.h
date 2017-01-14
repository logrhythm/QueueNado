/*
 * File: Notifier.h
 * Author: Craig Cogdill
 * Created: August 13, 2015 3:20PM
 */

#pragma once
#include <mutex>
#include <memory>
#include <vector>
#include <atomic>
class Shotgun;
class Vampire;

class Notifier {
 public:
   static std::unique_ptr<Notifier>  CreateNotifier(const std::string& notifierQueue, const std::string& handshakeQueue, const size_t handshakeCount, const size_t maxTimeoutInSec = 60, const int maxGetShotTimeoutInMs = 10);
   size_t Notify(const std::vector<std::string>& messages);
   size_t Notify(const std::string& message);
   size_t Notify();
   virtual ~Notifier();
   std::string ReceiveData();
   void SetNotificationMessage(const std::string& msg);

 protected:
   size_t ReceiveConfirmation();
   std::unique_ptr<Vampire> CreateHandshakeQueue();
   void Reset();

 private:
   Notifier() = delete;
   Notifier(const std::string& notifierQueue, const std::string& handshakeQueue, const size_t maxTimeoutInSec, const int maxGetShotTimeoutInMs);

   bool Initialize(const size_t handshakeCount);

   std::string GetNotifierQueueName();
   std::string GetHandshakeQueueName();
   bool QueuesAreUnitialized();
   const std::string mNotifierQueueName;
   const std::string mHandshakeQueueName;
   std::mutex mLock;
   std::unique_ptr<Shotgun> mQueue;
   std::unique_ptr<Vampire> mHandshakeQueue;
   size_t gHandshakeCount = 0;
   const size_t mMaxTimeoutInSec;
   const int mGetShotTimeoutInMs;
   std::string mNotifyMessage = "notify";
};
