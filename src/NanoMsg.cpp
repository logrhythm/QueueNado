#include "NanoMsg.h"
#include <cstring>
#include <stdexcept>
#include <g3log/g3log.hpp>
/**
 * Constructor for copying a packet hash vector into a nanomsg mBuffer
 */
NanoMsg::NanoMsg(PacketHashVector& data) : mSent(false) {
   auto bufferSize = data.size() * sizeof(std::pair<void*, unsigned int>);
   const int defaultAllocationPolicy = {0};
   mBuffer = nn_allocmsg(bufferSize, defaultAllocationPolicy);
   auto error = nn_errno ();
   if (error == ENOMEM) {
      throw std::runtime_error("not enough memory to allocate NanoMsg: " +
                               std::string(nn_strerror(errno)));
   }
   std::memcpy(mBuffer, &data[0], bufferSize);
}
NanoMsg::NanoMsg(void*& data) : mSent(false) {
   const int defaultAllocationPolicy = {0};
   auto bufferSize = sizeof(void*);
   mBuffer = nn_allocmsg(bufferSize, defaultAllocationPolicy);
   auto error = nn_errno ();
   if (error == ENOMEM) {
      throw std::runtime_error("not enough memory to allocate NanoMsg: " +
                               std::string(nn_strerror(errno)));
   }
   std::memcpy(mBuffer, &data, bufferSize);
}
/**
 * Constructor for copying a string into the nanomsg mBuffer
 */
NanoMsg::NanoMsg(const std::string& data) : mSent(false) {
   const int defaultAllocationPolicy = {0};
   mBuffer = nn_allocmsg(data.size(), defaultAllocationPolicy);
   auto error = nn_errno ();
   if (error == ENOMEM) {
      throw std::runtime_error("not enough memory to allocate NanoMsg: " +
                               std::string(nn_strerror(errno)));
   }
   std::memcpy(mBuffer, data.c_str(), data.size());
}
/**
 * Destructor to clean up the nanomsg allocated memory if the class has gone
 *  out of scope before being mSent over the wire where it would normally
 *  be deallocated
 */
NanoMsg::~NanoMsg(){
   if (!mSent) {
      nn_freemsg(mBuffer);
   }
}
