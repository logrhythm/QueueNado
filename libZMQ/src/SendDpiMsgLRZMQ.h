#pragma once

#include "Rifle.h"

class SendDpiMsgLRZMQ : public Rifle
{
public:
   explicit SendDpiMsgLRZMQ(const std::string& binding);
   SendDpiMsgLRZMQ(zctx_t* context, const std::string& binding);
   bool Initialize();
   bool SendData(const std::string& data);
   void SetQueueSize(const int size);
};
