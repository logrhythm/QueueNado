#pragma once
#include <dpi/libafc.h>
#include <dpi/libctl.h>
#include <dpi/protodef.h>
#include <dpi/libdata.h>
#include <dpi/modules/uaction.h>
#include <dpi/modules/uevent.h>
#include <dpi/modules/uevent_hooks.h>
#include <dpi/modules/uapplication.h>
//#include "Zombie.h"
#include "Vampire.h"

class ReceivePacketZMQ : public Vampire {
public:
   explicit ReceivePacketZMQ(const std::string& binding);
   bool Initialize();
   bool ReceiveDataBlock(std::string& dataReceived, const int nSecs);
   bool ReceiveDataBlockPointer(ctb_ppacket& dataReceived, const int milliseconds);
   void SetQueueSize(const int size);
protected:
};
