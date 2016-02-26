#pragma once
#include <functional>
#include <memory>
#include <nanomsg/pipeline.h>
#include <nanomsg/nn.h>
#include <string>
#include <vector>
#include "NanoProtocol.h"
class Vampire {
public:
   explicit Vampire(const std::string& location);
   Vampire(const std::string& location, const int timeoutInMs);
   std::string GetBinding() const;
   std::string GetShot();
   void GetStake(void*& stake);
   void GetStakes(std::vector<std::pair<void*, unsigned int>>& stakes);
   virtual ~Vampire();
private:
   int ReceiveMsg(void * buf);
   std::unique_ptr<NanoProtocol> mProtocolHandler;
   int mSocket;
};
