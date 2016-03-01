#pragma once
#include <vector>
#include <string>
#include <nanomsg/pipeline.h>
#include "NanoMsg.h"
#include "NanoProtocol.h"
#include <memory>
class Rifle {
public:
   explicit Rifle(const std::string& location);
   Rifle(const std::string&, const int timeoutInMs);
   std::string GetBinding() const;
   void Fire(const std::string& data);
   void Fire(void * data);
   //void FireUserPointer(void* data, int size);
   /* void FireStake(void* data); */
   /* void FireStakes(const std::vector<std::pair<void*, unsigned int>>& data); */
   virtual ~Rifle();
private:
   void FireZeroCopy(NanoMsg& msg);
   std::unique_ptr<NanoProtocol> mProtocolHandler;
   int mSocket;
};
