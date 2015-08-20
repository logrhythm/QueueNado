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
   static std::unique_ptr<Listener>  CreateListener(const std::string& notificationQueue, const std::string& handshakeQueue, const std::string& program);

   virtual ~Listener();
   
   Listener(const Listener&) = delete;
   Listener& operator=(const Listener&) = delete;
   bool NotificationReceived();
   bool SendConfirmation();

 protected:
   bool MessageHasPayload(const std::vector<std::string>& shots);

 private:
   Listener() = delete;
   Listener(const std::string& notificationQueue, const std::string& handshakeQueue, const std::string& program);
   bool Initialize();
   void Reset();
   std::string ThreadID();
   std::unique_ptr<Rifle> CreateFeedbackShooter();


   const std::string mNotificationQueueName;
   const std::string mHandshakeQueueName;
   std::unique_ptr<Alien> mQueueReader;
   std::unique_ptr<Rifle> mHandshakeQueue;
   const std::string mProgramName;
   const unsigned int getShotTimeout = 0;
   const int kBlockForOneMinute = 60;
   const size_t kNumberOfMessages = 2;
};