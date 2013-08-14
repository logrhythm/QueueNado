/* 
 * File:   MockConfNtp.h
 * Author: kjell
 *
 * Created on August 1, 2013, 2:15 PM
 */

#pragma once
#include "ConfNtp.h"
#include <string>

struct MockConfNtp : public ConfNtp {
   std::string mContent;

   explicit MockConfNtp(const std::string path) 
   : ConfNtp(path) {
      ReadNtpConfFromFile();
   }
   
protected:
   std::string GetFileContent() LR_OVERRIDE {
      mContent = ConfNtp::GetFileContent();
      return mContent;
   }

   // No write!
   void WriteFileContent(const std::string& content) LR_OVERRIDE { 
      mContent = content; 
   }
};



