#include "Pull.h"
#include <stdexcept>
#include <g3log/g3log.hpp>
/**
 * Construct a blocking nanomsg pull (no timeout)
 */
Pull::Pull(const std::string& location, const bool shouldConnect) {
   mProtocolHandler = std::move(std::unique_ptr<NanoProtocol>(
                                   new NanoProtocol(location)));
   mSocket = nn_socket(AF_SP, NN_PULL);
   if (mSocket < 0) {
      throw std::runtime_error("could not open transport socket" +
                               mProtocolHandler->GetLocation() +
                               " because of error: " +
                               std::string(nn_strerror(errno)));
   }
   int connectResponse = {0};
   std::string connectionType("");
   if (shouldConnect){
      connectResponse = nn_connect(mSocket,
                                   mProtocolHandler->GetLocation().c_str());
      connectionType = "nn_connect";
   } else{
      connectResponse = nn_bind(mSocket,
                                mProtocolHandler->GetLocation().c_str());
      connectionType = "nn_bind";
   }
   if (connectResponse < 0) {
      throw std::runtime_error("could not connect to endpoint: " +
                               mProtocolHandler->GetLocation() +
                               " because of error: " +
                               std::string(nn_strerror(errno)));
   }
   LOG(INFO) << connectionType
              << " socket at: " << mProtocolHandler->GetLocation();
}
/**
 * Construct a non-blocking nanomsg pull (timeout)
 */
Pull::Pull(const std::string& location,
                 const int timeoutInMs,
                 const bool shouldConnect) {
   mProtocolHandler = std::move(std::unique_ptr<NanoProtocol>(
                                   new NanoProtocol(location)));
   mSocket = nn_socket(AF_SP, NN_PULL);
   if (mSocket < 0) {
      throw std::runtime_error("could not open transport socket" +
                               mProtocolHandler->GetLocation() +
                               " because of error: " +
                               std::string(nn_strerror(errno)));
   }
   nn_setsockopt(mSocket,
                 NN_SOL_SOCKET,
                 NN_RCVTIMEO,
                 &timeoutInMs,
                 sizeof (timeoutInMs));
   int connectResponse = {0};
   std::string connectionType("");
   if (shouldConnect){
      connectResponse = nn_connect(mSocket,
                                   mProtocolHandler->GetLocation().c_str());
      connectionType = "nn_connect";
   } else{
      connectResponse = nn_bind(mSocket,
                                mProtocolHandler->GetLocation().c_str());
      connectionType = "nn_bind";
   }
   if (connectResponse < 0) {
      throw std::runtime_error("could not connect to endpoint: " +
                               mProtocolHandler->GetLocation() +
                               " because of error: " +
                               std::string(nn_strerror(errno)));
   }
   LOG(INFO) << connectionType
              << " socket at: " << mProtocolHandler->GetLocation()
              << " with timeout: " << timeoutInMs;
}
/**
 * Return the socket location
 */
std::string Pull::GetBinding() const {
   return mProtocolHandler->GetLocation();
}
/**
 * Receive a msg from the nanomsg queue, check for errors
 */
int Pull::ReceiveMsg(void *& buf) {
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
 * Get a string sent by the nano push
 */
std::string Pull::GetString() {
   void * buf = nullptr;
   auto num_bytes_received = ReceiveMsg(buf);
   std::string nanoString(static_cast<char*>(buf), num_bytes_received);
   nn_freemsg(buf);
   return nanoString;
}
/**
 * Gets a void pointer from the nanomsg queue
 */
int Pull::GetPointer(void*& pointer) {
   void * buf = nullptr;
   auto bytesReceived = ReceiveMsg(buf);
   auto castedBuf = reinterpret_cast<void**>(buf);
   pointer = *castedBuf;
   nn_freemsg(buf);
   return bytesReceived;
}
/**
 * Gets a vector of pointers from the nanomsg queue
 */
void Pull::GetVector(std::vector<std::pair<void*, unsigned int>>& vector) {
   void * buf = {nullptr};
   ReceiveMsg(buf);
   auto receivedVector = reinterpret_cast<std::vector<std::pair<void*, unsigned int>>*> (buf);
   vector = *receivedVector;
   nn_freemsg(buf);
}
/**
 * Destructor that closes socket
 */
Pull::~Pull() {
   nn_close(mSocket);
}
