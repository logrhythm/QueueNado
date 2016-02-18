#include "Vampire.h"
#include <stdexcept>
#include <g3log/g3log.hpp>
/**
 * Construct our Vampire which is a nanomsg pull
 */
Vampire::Vampire(const std::string& location) :
   mLocation(location) {
   mSocket = nn_socket(AF_SP, NN_PULL);
   if (mSocket < 0) {
      throw std::runtime_error("could not open transport socket");
   }
   auto connect_response = nn_connect(mSocket, location.c_str());
   if (connect_response < 0) {
      throw std::runtime_error("could not connect to endpoint: " + location);
   }
}

/**
 * Return thet location we are going to be shot.
 * @return
 */
std::string Vampire::GetBinding() const {
   return mLocation;
}
/**
 * Get shot by the rifle.
 */
std::string Vampire::GetShot() {
   void * buf;
   auto num_bytes_received = nn_recv(mSocket, &buf, NN_MSG, 0);
   std::string nanoString(static_cast<char*>(buf), num_bytes_received);
   nn_freemsg(buf);
   return nanoString;
}

Vampire::~Vampire() {
   nn_close(mSocket);
}
