#pragma once

#include "Vampire.h"

class ReceiveDpiMsgLRZMQ: public Vampire {
public:
   explicit ReceiveDpiMsgLRZMQ(const std::string& binding);
   bool Initialize();
   bool ReceiveDataBlock(std::string& wound,const int timeout);
   void SetQueueSize(const int size);
protected:
};
