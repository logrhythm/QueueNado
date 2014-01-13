#pragma once

#ifdef LR_DEBUG
#include "MockConf.h"
#include <g2log.hpp>
class tempFileCreate {
public:

   tempFileCreate(MockConf& conf) : mConf(conf) {
      mTestDir << "/tmp/TooMuchPcap." << pthread_self() << "/";
   }
   
    
   tempFileCreate(MockConf& conf, const std::string& directory) : mConf(conf) {
      mTestDir << directory << "." << pthread_self() << "/";
   }

   ~tempFileCreate() {
      std::string makeADir = "rm -rf ";
      makeADir += mTestDir.str();
      LOG_IF(WARNING, -1 == system(makeADir.c_str())) << "Could not remove directory with command: " << makeADir;
   }

   bool Init() {
      mConf.mPCapCaptureLocations.clear();
      mConf.mPCapCaptureLocations.push_back(mTestDir.str());
      struct stat st;
      int success = 0;
      if(stat(mTestDir.str().c_str(),&st) != 0) {
         std::string makeADir = "mkdir -p ";
         makeADir += mTestDir.str();
         success =  system(makeADir.c_str());
         LOG_IF(WARNING, -1 == success) << "Could not create directory with command: " << makeADir;
      }
      return success == 0;
   }

   std::stringstream mTestDir;
   MockConf& mConf;
};

#endif
