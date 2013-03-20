#pragma once

#include "czmq.h"
#include <stdlib.h>
#include <vector>
#include <string>

class Alien {
public:
   Alien();
   void PrepareToBeShot(const std::string& location);
   std::vector<std::string> GetShot();
   void GetShot(const unsigned int timeout, std::vector<std::string>& bullets);
   virtual ~Alien();
    
private:
   void *mBody;
   zctx_t *mCtx;
};