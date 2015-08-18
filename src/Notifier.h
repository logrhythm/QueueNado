/*
 * File: Notifier.h
 * Author: Craig Cogdill
 * Created: August 13, 2015 3:20PM
 */

#pragma once
//#include "include/global.h"
#include <mutex>
#include <memory>

class Shotgun;
class Vampire;

class Notifier {
   public:
      Notifier() = delete;
      Notifier(const std::string& notifierQueue, const std::string& handshakeQueue)
         : mNotifierQueueName(notifierQueue),
           mHandshakeQueueName(handshakeQueue) {};
      virtual ~Notifier() { Reset(); };

       bool Initialize(const size_t handshakeCount);
       size_t Notify();
   
   protected:
       size_t ReceiveConfirmation();
       std::unique_ptr<Vampire> CreateHandshakeQueue();
       void Reset();

   private:
      std::string GetNotifierQueueName();
      std::string GetHandshakeQueueName();
      bool QueuesAreUnitialized();
      const std::string mNotifierQueueName;
      const std::string mHandshakeQueueName;
      std::mutex gLock;
      std::unique_ptr<Shotgun> gQueue;
      std::unique_ptr<Vampire> gHandshakeQueue;
      size_t gHandshakeCount = 0;
      const size_t gMaxTimeoutInSec = 60;

};
