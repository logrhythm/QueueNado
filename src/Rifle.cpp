#include "Rifle.h"
#include <g3log/g3log.hpp>
/**
 * Construct a blocking rifle (no timeout)
 */
Rifle::Rifle(const std::string& location) {
   mProtocolHandler = std::move(std::unique_ptr<NanoProtocol>(
                                   new NanoProtocol(location)));
   mSocket = nn_socket(AF_SP, NN_PUSH);
   if (mSocket < 0) {
      throw std::runtime_error("could not open transport socket");
   }
   auto connect_response = nn_bind(mSocket,
                                   mProtocolHandler->GetLocation().c_str());
   if (connect_response < 0) {
      throw std::runtime_error("could not connect to endpoint: " +
                               mProtocolHandler->GetLocation());
   }
   LOG(DEBUG) << "opened socket at: " << mProtocolHandler->GetLocation();
}
/**
 * Construct non-blocking rifle (timeout)
 */
Rifle::Rifle(const std::string& location, const int timeoutInMs) {
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
   auto connect_response = nn_bind(mSocket,
                                   mProtocolHandler->GetLocation().c_str());
   if (connect_response < 0) {
      throw std::runtime_error("could not connect to endpoint: " +
                               mProtocolHandler->GetLocation());
   }
   LOG(DEBUG) << "opened socket at: " << mProtocolHandler->GetLocation();
}
/**
 * Return the location the socket is bound to
 * @return
 */
std::string Rifle::GetBinding() const {
   return mProtocolHandler->GetLocation();
}
// /**
//  * Fires a void pointer
//  */
// void Rifle::FireStake(void* data) {
//    NanoMsg msg(data);
//    FireZeroCopy(msg);
// }
// /**
//  * Fires a packet hash vector
//  */
// void Rifle::FireStakes(const std::vector<std::pair<void*, unsigned int>>& data) {
//    NanoMsg msg(data);
//    FireZeroCopy(msg);
// }
/**
 * Fires a basic string
 */
void Rifle::Fire(const std::string& data) {
   NanoMsg msg(data);
   FireZeroCopy(msg);
}
void Rifle::Fire(void *data) {
   NanoMsg msg(data);
   FireZeroCopy(msg);
}
/**
 * Fires a pointer controlled by the user, ie no zero copy
 */
// void Rifle::FireUserPointer(void * data,
//                             int size) {
//    auto num_bytes_sent = nn_send(mSocket,
//                                  &data,
//                                  size,
//                                  0);
//    if (num_bytes_sent < 0) {
//       throw std::runtime_error("send failed");
//    }
// }
/**
 * Fire a message using zero copy api
 */
void Rifle::FireZeroCopy(NanoMsg& msg) {
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
Rifle::~Rifle() {
   nn_close(mSocket);
}
