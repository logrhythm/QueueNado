#include "Rifle.h"
#include <g3log/g3log.hpp>
/**
 * Construct our Rifle which is a nanomsg push
 */
Rifle::Rifle(const std::string& location) :
   mLocation(location) {
   mSocket = nn_socket(AF_SP, NN_PUSH);
   if (mSocket < 0) {
      throw std::runtime_error("could not open transport socket");
   }
   auto connect_response = nn_bind(mSocket, location.c_str());
   if (connect_response < 0) {
      throw std::runtime_error("could not connect to endpoint: " + location);
   }
}

/**
 * Return the location the socket is bound to
 * @return
 */
std::string Rifle::GetBinding() const {
   return mLocation;
}


/**
 * Fire a string using zero copy api
 */
void Rifle::FireZeroCopy(NanoMsg& msg) {
   //LOG(INFO) << "about to send data";
   auto num_bytes_sent = nn_send(mSocket,
                                &msg.buffer,
                                NN_MSG,
                                0);
   if (num_bytes_sent < 0) {
      throw std::runtime_error("send failed");
   }
   msg.sent = true;
}

Rifle::~Rifle() {
   nn_close(mSocket);
}
