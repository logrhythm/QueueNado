#include "Vampire.h"
#include <stdexcept>
#include <g3log/g3log.hpp>
/**
 * Construct a blocking vampire (no timeout)
 */
Vampire::Vampire(const std::string& location) {
   mProtocolHandler = std::move(std::unique_ptr<NanoProtocol>(
                                   new NanoProtocol(location)));
   mSocket = nn_socket(AF_SP, NN_PULL);
   if (mSocket < 0) {
      throw std::runtime_error("could not open transport socket");
   }
   auto connect_response = nn_connect(mSocket,
                                      mProtocolHandler->GetLocation().c_str());
   if (connect_response < 0) {
      throw std::runtime_error("could not connect to endpoint: " +
                               mProtocolHandler->GetLocation());
   }
   LOG(DEBUG) << "opened socket at: " << mProtocolHandler->GetLocation();
}
/**
 * Construct a non-blocking Vampire (timeout)
 */
Vampire::Vampire(const std::string& location, const int timeoutInMs) {
   mProtocolHandler = std::move(std::unique_ptr<NanoProtocol>(
                                   new NanoProtocol(location)));
   mSocket = nn_socket(AF_SP, NN_PULL);
   if (mSocket < 0) {
      throw std::runtime_error("could not open transport socket");
   }
   nn_setsockopt(mSocket,
                 NN_SOL_SOCKET,
                 NN_RCVTIMEO,
                 &timeoutInMs,
                 sizeof (timeoutInMs));
   auto connect_response = nn_connect(mSocket,
                                      mProtocolHandler->GetLocation().c_str());
   if (connect_response < 0) {
      throw std::runtime_error("could not connect to endpoint: " +
                               mProtocolHandler->GetLocation());
   }
   LOG(DEBUG) << "opened socket at: " << mProtocolHandler->GetLocation()
              << " with timeout: " << timeoutInMs;
}
/**
 * Return the socket location
 */
std::string Vampire::GetBinding() const {
   return mProtocolHandler->GetLocation();
}
/**
 * Receive a msg from the nanomsg queue, check for errors
 */
int Vampire::ReceiveMsg(void * buf) {
   auto num_bytes_received = nn_recv(mSocket, &buf, NN_MSG, 0);
   if (num_bytes_received < 0) {
      auto error = nn_errno ();
      if (error == ETIMEDOUT) {
         throw std::runtime_error("timed out receiving message");
      }
   }
   return num_bytes_received;
}
/**
 * Get a string sent by the rifle
 */
std::string Vampire::GetShot() {
   void * buf = {nullptr};
   auto num_bytes_received = ReceiveMsg(buf);
   std::string nanoString(static_cast<char*>(buf), num_bytes_received);
   nn_freemsg(buf);
   return nanoString;
}
/**
 * Gets a void pointer from the nanomsg queue
 */
void Vampire::GetStake(void*& stake) {
   ReceiveMsg(stake);
}
/*
 * Receives a vector of pairs from the nanomsg queue
 */
void Vampire::GetStakes(std::vector<std::pair<void*, unsigned int>>& stakes) {
   void * buf = {nullptr};
   auto num_bytes_received = ReceiveMsg(buf);
   auto vectorStart = reinterpret_cast<std::pair<void*, unsigned int>*> (buf);
   stakes.assign(vectorStart,
                 vectorStart
                 + num_bytes_received /  sizeof (std::pair<void*, unsigned int>));
   nn_freemsg(buf);
}
/**
 * Destructor that closes socket
 */
Vampire::~Vampire() {
   nn_close(mSocket);
}
