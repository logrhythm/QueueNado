/* 
 * File:   CZMQToolkit.h
 * Author: root
 *
 * Created on November 16, 2012, 8:34 AM
 */

#pragma once
#include <stdint.h>
#include <zmq.h>
#include <zlib.h>
#include <czmq.h>
#include <zframe.h>
#include <map>
#include <string>
#include <set>
#include "boost/thread.hpp"
#include "include/global.h"
#ifndef ZMQ_KILL_MESSAGE
#define ZMQ_KILL_MESSAGE 4
#endif

class CZMQToolkit {
public:

   /**
    * Get the CRC32 hash for the given data
    *
    * @param $first
    *   A reference to a data message string
    * @param $second
    *   A CRC of the data
    */
   static inline void GetHash(const std::string* data, uLong& hash) {
      hash = crc32(0L, NULL, 0);
      hash = crc32(hash, reinterpret_cast<const unsigned char *> (&((*data)[0])),
              data->size());
   }
   static void FreeString(std::string *data, void *hint);
   static void setHWMAndBuffer(void* socket, const int size);
   static void PrintCurrentHighWater(void* socket, const std::string& name);
   static bool GetSizeTFromSocket(void* socket, size_t& value);
   static bool SendBlankMessage(void* socket);
   static bool SocketFIFO(void* socket);
   static bool SendExistingMessage(zmsg_t*& bullet, void* socket);
   static bool PopAndDiscardMessage(void* socket);
   static bool SendSizeTToSocket(void* socket, const size_t size);
   static bool PassMessageAlong(void* sourceSocket, void* destSocket);
   static bool SendStringContentsToSocket(void* sourceSocket, void* destSocket);
   static bool IsValidMessage(zmsg_t* message);
   static void SendShutdownMessage(void* socket);
   static std::string* GetStringFromMessage(zmsg_t*& message);
   static bool SendStringWithHash(void* socket, zmsg_t*& message,
           const std::string* slug);
   static bool ForkPartsOfMessageTwoDirections(void* sourceSocket,
           void* firstDestination, void* secondDestination);
};

