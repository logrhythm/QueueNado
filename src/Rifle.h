#pragma once
#include <vector>
#include <string>
#include <nanomsg/pipeline.h>
#include "NanoMsg.h"
class Rifle {
public:
   Rifle(const std::string& location);
   std::string GetBinding() const;
   void FireZeroCopy(NanoMsg& msg);
   virtual ~Rifle();
protected:
   void Destroy();
private:
   std::string mLocation;
   int mSocket;
};
