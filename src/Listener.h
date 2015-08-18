/*
 * File: Listener.h
 * Author: Craig Cogdill
 * Created: August 14, 2015 1:08PM
 */

#pragma once
#include <vector>
#include <string>
#include <memory>

class Alien;
class Rifle;

class Listener {
   public:
      Listener() = delete;
      Listener(const std::string& notificationQueue, const std::string& handshakeQueue, const std::string& program);
      virtual ~Listener() { Reset(); };
      Listener(const Listener&) = delete;
      Listener& operator=(const Listener&) = delete;
      bool Initialize();
      void Reset();
      bool NotificationReceived();
      bool SendConfirmation();
      std::unique_ptr<Rifle> CreateFeedbackShooter();
      std::string ThreadID();
   protected:
      bool MessageHasPayload(const std::vector<std::string>& shots);
   private:
      const std::string mNotificationQueueName;
      const std::string mHandshakeQueueName;
      std::unique_ptr<Alien> mQueueReader;
      std::unique_ptr<Rifle> mHandshakeQueue;
      const std::string mProgramName;
      const unsigned int getShotTimeout = 0;
      const int kBlockForOneMinute = 60;
      const size_t kNumberOfMessages = 2;
};