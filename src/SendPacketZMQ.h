#pragma once

#include "Rifle.h"

class SendPacketZMQ : public Rifle
{
public:
   explicit SendPacketZMQ(const std::string& binding);
   SendPacketZMQ(const std::string& binding,zctx_t* context);
   bool SendData(const std::string& data);
   bool SendData(void* data, const unsigned int timeout);
   void SetQueueSize(const int size);
   bool Initialize();
};
