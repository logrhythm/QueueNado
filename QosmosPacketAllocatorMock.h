#pragma once

#include "QosmosPacketAllocator.h"

class MockQosmosPacketAllocator : public QosmosPacketAllocator {
public:
   MockQosmosPacketAllocator() : mMallocpacketFail(false){};

   ctb_ppacket MallocpPacket() {
      if (mMallocpacketFail) {
         return NULL;
      }
      return QosmosPacketAllocator::MallocpPacket();
   }

   bool mMallocpacketFail;
protected:
private:
};
