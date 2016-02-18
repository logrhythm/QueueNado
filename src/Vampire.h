#pragma once
#include <string>
#include <vector>
#include <nanomsg/pipeline.h>
#include <nanomsg/nn.h>
class Vampire {
public:
   explicit Vampire(const std::string& location);
   std::string GetBinding() const;
   std::string GetShot();
   virtual ~Vampire();
private:
   std::string mLocation;
   int mSocket;
};
