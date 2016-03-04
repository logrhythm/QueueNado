#pragma once
#include <functional>
#include <memory>
#include <nanomsg/pipeline.h>
#include <nanomsg/nn.h>
#include <string>
#include <vector>
#include "NanoProtocol.h"
class Pull {
public:
   Pull(const std::string& location, const bool shouldConnect=false);
   Pull(const std::string& location,
        const int timeoutInMs,
        const bool shouldConnect=false);
   std::string GetBinding() const;
   std::string GetString(bool dontWait=false);
   void GetVector(std::vector<std::pair<void*, unsigned int>>& vector,
                  bool dontWait=false);
   int GetPointer(void*& pointer, bool dontWait=false);
   virtual ~Pull();
   Pull& operator=(const Pull&) = delete;
   Pull(const Pull&) = delete;
   Pull() = delete;
private:
   int ReceiveMsg(void *& buf, bool dontWait);
   std::unique_ptr<NanoProtocol> mProtocolHandler;
   int mSocket;
};
