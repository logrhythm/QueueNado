/* 
 * File:   CZMQToolkit.h
 * Author: root
 *
 * Created on November 16, 2012, 8:34 AM
 */

#pragma once
#include <string>
#include <zlib.h>

struct _zmsg_t;
typedef struct _zmsg_t zmsg_t;

class CZMQToolkit {
public:

   static void setHWMAndBuffer(void* socket, const int size);
   static void PrintCurrentHighWater(void* socket, const std::string& name);
   static bool SendExistingMessage(zmsg_t*& bullet, void* socket);
};

