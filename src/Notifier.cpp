/*
 * File: Notifier.cpp
 * Author: Craig Cogdill
 * Created: August 13, 2015 3:20PM
 */

#include "Notifier.h"
#include <g2log.hpp>
#include <mutex>
#include <memory>
#include <Shotgun.h>
#include <Vampire.h>
#include <StopWatch.h>

std::unique_ptr<Notifier>  Notifier::CreateNotifier(const std::string& notifierQueue, const std::string& handshakeQueue, const size_t handshakeCount) {

   auto notifier = std::unique_ptr<Notifier>(new Notifier(notifierQueue, handshakeQueue));
   if (notifier->Initialize(handshakeCount)) {
      return std::move(notifier);
   }
   std::unique_ptr<Notifier> deadNotifier;
   return deadNotifier;
}


/**
* @param notifierQueue to send notification on
* @param handshakeQueue to receive handshake confirmation on
*/
Notifier::Notifier(const std::string& notifierQueue, const std::string& handshakeQueue)
   : mNotifierQueueName(notifierQueue),
     mHandshakeQueueName(handshakeQueue) {};

Notifier::~Notifier() {
   Reset();
}

/*
 * Initialize the mutex-guarded Shotgun-Alien
 *    queue by "aiming" it at the Notifier
 *    ipc file
 *
 * @return bool, whether or not both queues are initialized
 */
bool Notifier::Initialize(const size_t handshakeCount) {
   std::lock_guard<std::mutex> guard(gLock);
   gHandshakeCount = handshakeCount;
   if (gHandshakeQueue.get() == nullptr) {
      gHandshakeQueue = std::move(CreateHandshakeQueue());
   }
   if (gQueue.get() == nullptr) {
      gQueue.reset(new Shotgun);
      try {
         gQueue->Aim(GetNotifierQueueName());
      } catch (std::exception& e) {
         LOG(WARNING) << "Caught exception: " << e.what();
         Reset();
      }
   }
   return (gQueue.get() != nullptr) && (gHandshakeQueue.get() != nullptr);
}

/*
*  Initialize a Vampire queue to get confirmation from
*     listeners that they received the notification
*
*  @return unique_ptr to Vampire handshake queue     
*/
std::unique_ptr<Vampire> Notifier::CreateHandshakeQueue() {
   const std::string handshakeQueueName = GetHandshakeQueueName();
   LOG(INFO) << "Creating Vampire with queue: " << handshakeQueueName;
   const auto size = 100;

   auto target = std::unique_ptr<Vampire>(new Vampire(handshakeQueueName));
   target->SetHighWater(size);
   target->SetOwnSocket(true);
   if (!target->PrepareToBeShot()) {
      std::unique_ptr<Vampire> miss;
      LOG(WARNING) << "Unable to bind socket: " << handshakeQueueName;
      return miss;
   }
   return target;
}

/*
 * Fire a message from the Shotgun to be
 *    read by the queue subscriber. Then wait for listeners
 *    to confirm they received the notification
 *
 * @param string to be sent to listeners
 * @return number of confirmed updates
 */
size_t Notifier::Notify(const std::string& message) {
   std::vector<std::string> bullets;
   bullets.push_back(message);
   return Notify(bullets);
}

/*
 * Fire a dummy message from the Shotgun to be
 *    read by the queue subscriber. Then wait for listeners
 *    to confirm they received the notification
 *
 * @return number of confirmed updates
 */
size_t Notifier::Notify() {
   return Notify(kNotifyMessage);
}

/*
 * Fire a message from the Shotgun to be
 *    read by the queue subscriber. Then wait for listeners
 *    to confirm they received the notification
 * 
 * @param vector of strings to be sent to the listeners             
 * @return number of confirmed updates
 */
size_t Notifier::Notify(const std::vector<std::string>& messages) {
   std::lock_guard<std::mutex> guard(gLock);
   std::vector<std::string> bullets;
   bullets.push_back("dummy");

   for (auto& msg : messages) {
      bullets.push_back(msg);
   }

   if (QueuesAreUnitialized()) {
      LOG(WARNING) << "Uninitialized notifier queues";
      return {0};
   }
   LOG(INFO) << "Notifier: Sending " << bullets.size() << " messages";
   gQueue->Fire(bullets);
   size_t confirmed = ReceiveConfirmation();
   LOG(INFO) << "Notifier received " << confirmed << " handshakes";
   return {confirmed};
}

/*
*  Receive confirmation from listener threads that
*     they have been notified successfully
*
*  @return size_t, number of confirmations received
*/
size_t Notifier::ReceiveConfirmation() {
   StopWatch waitCheck;
   size_t responses = 0;
   while (responses < gHandshakeCount && waitCheck.ElapsedSec() < gMaxTimeoutInSec) {
      std::string msg;
      if (gHandshakeQueue->GetShot(msg, gMaxTimeoutInSec)) {
         LOG(INFO) << "Received update confirmation from thread #"
                   << msg << ", response count #" << ++responses;
      }
   }

   const bool goodEnough = waitCheck.ElapsedSec() < gMaxTimeoutInSec || responses == gHandshakeCount;

   LOG_IF(WARNING, !goodEnough) << "Listener confirmation timed out after" << gMaxTimeoutInSec
                                << "seconds... " << responses << "/" << gHandshakeCount << " replied";
   return responses;
}

/*
 * Reset the Shotgun-Alien queue to nullptr
 */
void Notifier::Reset() {
   std::lock_guard<std::mutex> guard(gLock);
   gQueue.reset(nullptr);
   gHandshakeQueue.reset(nullptr);
}

/*
*  @return name of notifier ipc queue taken from ExternalConnections
*/
std::string Notifier::GetNotifierQueueName() {
   return mNotifierQueueName;
}

/*
*  @return name of handshake ipc queue taken from ExternalConnections
*/
std::string Notifier::GetHandshakeQueueName() {
   return mHandshakeQueueName;
}

/*
*  @return whether both of the queues are initialized
*/
bool Notifier::QueuesAreUnitialized() {
   return (gQueue.get() == nullptr || gHandshakeQueue.get() == nullptr); 
}
