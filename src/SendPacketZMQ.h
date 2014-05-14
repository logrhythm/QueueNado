#pragma once

#include "Rifle.h"
#include <dpi/libafc.h>
#include <dpi/libctl.h>
#include <dpi/protodef.h>
#include <dpi/libdata.h>
#include <dpi/modules/uaction.h>
#include <dpi/modules/uevent.h>
#include <dpi/modules/uevent_hooks.h>
#include <dpi/modules/uapplication.h>

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
