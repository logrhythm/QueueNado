/* 
 * File:   MockConfNtpToReadFile.h
 * Author: kjell
 *
 * Created on August 1, 2013, 2:12 PM
 */

#pragma once
#include "ConfNtp.h"

struct MockConfNtpToReadFile : public ConfNtp {
   MockConfNtpToReadFile() : ConfNtp() {}
   std::string GetFileContent() LR_OVERRIDE {   return ConfNtp::GetFileContent();   }
};


