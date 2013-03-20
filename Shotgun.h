#pragma once

#include "czmq.h"
#include <stdlib.h>
#include <vector>
#include <string>

class Shotgun {
public:
   Shotgun();
   void Aim(const std::string& location);
   void Fire(const std::string& msg);
   void Fire(const std::vector<std::string>& bullets);
   virtual ~Shotgun();
private:
   void *mGun;
   zctx_t *mCtx;
};