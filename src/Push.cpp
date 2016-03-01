#include "Push.h"
#include <g3log/g3log.hpp>
/**
 * Construct a blocking rifle (no timeout)
 */
Push::Push(const std::string& location, const bool shouldConnect) {
   mProtocolHandler = std::move(std::unique_ptr<NanoProtocol>(
                                   new NanoProtocol(location)));
   mSocket = nn_socket(AF_SP, NN_PUSH);
   if (mSocket < 0) {
      throw std::runtime_error("could not open transport socket");
   }
   int connectResponse = {0};
   std::string connectionType("");
   if (shouldConnect){
      connectResponse = nn_connect(mSocket,
                                   mProtocolHandler->GetLocation().c_str());
      connectionType = std::string("nn_connect");
   } else{
      connectResponse = nn_bind(mSocket,
                                mProtocolHandler->GetLocation().c_str());
      connectionType = std::string("nn_bind");
   }
   if (connectResponse < 0) {
      throw std::runtime_error("could not connect to endpoint: " +
                               mProtocolHandler->GetLocation());
   }
   LOG(DEBUG) << connectionType
              << " socket at: " << mProtocolHandler->GetLocation();
}
/**
 * Construct non-blocking rifle (timeout)
 */
Push::Push(const std::string& location,
             const int timeoutInMs,
             const bool shouldConnect) {
   mProtocolHandler = std::move(std::unique_ptr<NanoProtocol>(
                                   new NanoProtocol(location)));
   mSocket = nn_socket(AF_SP, NN_PUSH);
   if (mSocket < 0) {
      throw std::runtime_error("could not open transport socket");
   }
   nn_setsockopt(mSocket,
                  NN_SOL_SOCKET,
                  NN_SNDTIMEO,
                  &timeoutInMs,
                  sizeof(timeoutInMs));
   int connectResponse = {0};
   std::string connectionType("");
   if (shouldConnect){
      connectResponse = nn_connect(mSocket,
                                   mProtocolHandler->GetLocation().c_str());
      connectionType = std::string("nn_connect");
   } else{
      connectResponse = nn_bind(mSocket,
                                mProtocolHandler->GetLocation().c_str());
      connectionType = std::string("nn_bind");
   }
   if (connectResponse < 0) {
      throw std::runtime_error("could not connect to endpoint: " +
                               mProtocolHandler->GetLocation());
   }
   LOG(DEBUG) << connectionType
              << " socket at: " << mProtocolHandler->GetLocation();
}
/**
 * Return the location the socket is bound to
 * @return
 */
std::string Push::GetBinding() const {
   return mProtocolHandler->GetLocation();
}
/**
 * Fires a basic string
 */
void Push::Send(const std::string& data) {
   NanoMsg msg(data);
   SendMessage(msg);
}
/**
 * Fires a void pointer
 */
void Push::Send(void *data) {
   NanoMsg msg(data);
   SendMessage(msg);
}
void Push::SendMessage(NanoMsg& msg) {
   auto num_bytes_sent = nn_send(mSocket,
                                &msg.buffer,
                                NN_MSG,
                                0);
   if (num_bytes_sent < 0) {
      throw std::runtime_error("send failed");
   }
   // mark message as sent so it does not have to deallocate itself
   msg.sent = {true};
}
/**
 * Destructor that closes socket
 */
Push::~Push() {
   nn_close(mSocket);
}
