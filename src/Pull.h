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
   std::string GetString();
   void GetVector(std::vector<std::pair<void*, unsigned int>>& vector);
   int GetPointer(void*& pointer);
   virtual ~Pull();
private:
   int ReceiveMsg(void *& buf);
   std::unique_ptr<NanoProtocol> mProtocolHandler;
   int mSocket;
};
