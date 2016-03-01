#include "NanoMsg.h"
#include <cstring>
#include <stdexcept>
/**
 * Constructor for copying a packet hash vector into a nanomsg buffer
 */
NanoMsg::NanoMsg(PacketHashVector& data) : sent(false) {
   auto data_size = data.size() * sizeof(std::pair<void*, unsigned int>);
   buffer = nn_allocmsg(data_size, 0);
   auto error = nn_errno ();
   if (error == ENOMEM) {
      throw std::runtime_error("not enough memory to allocate NanoMsg");
   }
   std::memcpy(buffer, &data, data_size);
}
/**
 * Constructor for copying a raw pointer into the nanomsg buffer
 */
NanoMsg::NanoMsg(void*& data) : sent(false) {
   auto data_size = sizeof(void*);
   buffer = nn_allocmsg(data_size, 0);
   auto error = nn_errno ();
   if (error == ENOMEM) {
      throw std::runtime_error("not enough memory to allocate NanoMsg");
   }
   std::memcpy(buffer, &data, data_size);
}
/**
 * Constructor for copying a string into the nanomsg buffer
 */
NanoMsg::NanoMsg(const std::string& data) : sent(false) {
   buffer = nn_allocmsg(data.size(), 0);
   auto error = nn_errno ();
   if (error == ENOMEM) {
      throw std::runtime_error("not enough memory to allocate NanoMsg");
   }
   std::memcpy(buffer, data.c_str(), data.size());
}
/**
 * Destructor to clean up the nanomsg allocated memory if the class has gone
 *  out of scope before being sent over the wire where it would normally
 *  be deallocated
 */
NanoMsg::~NanoMsg(){
   if (!sent) {
      nn_freemsg(buffer);
   }
}
