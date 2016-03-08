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
   Push(const std::string&,
        const int timeoutInMs,
        const int bufferSize,
        const bool shouldConnect=true);
   std::string GetBinding() const;
   void Send(const std::string& data, bool dontWait=false);
   void Send(void * data, bool dontWait=false);
   void Send(const std::vector<std::pair<void*, unsigned int>>& data,
             bool dontWait=false);
   virtual ~Push();
   Push& operator=(const Push&) = delete;
   Push(const Push&) = delete;
   Push() = delete;
private:
   void SendMessage(NanoMsg& msg, bool dontWait=false);
   std::unique_ptr<NanoProtocol> mProtocolHandler;
   int mSocket;
};
