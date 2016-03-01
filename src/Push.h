#pragma once
#include <vector>
#include <string>
#include <nanomsg/pipeline.h>
#include "NanoMsg.h"
#include "NanoProtocol.h"
#include <memory>
class Push {
public:
   Push(const std::string& location, const bool shouldConnect=true);
   Push(const std::string&,
        const int timeoutInMs,
        const bool shouldConnect=true);
   std::string GetBinding() const;
   void Send(const std::string& data);
   void Send(void * data);
   virtual ~Push();
private:
   void SendMessage(NanoMsg& msg);
   std::unique_ptr<NanoProtocol> mProtocolHandler;
   int mSocket;
};
