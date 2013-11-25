/* 
 * File:   MockConf.h
 *
 * Created on December 26, 2012, 10:39 AM
 */

#pragma once
#include "ValidateConf.h"
#include "include/global.h"
#include "Range.h"

class MockValidateConf : public ValidateConf {
public:

   MockValidateConf() :

   mValidConf(true),
   mIgnoreSyslogConfValidation(true),
   mIgnoreBaseConfValidation(true) {
   }

   ~MockValidateConf() {
   }

   bool ValidateBaseConf(protoMsg::BaseConf& msg) LR_OVERRIDE {
      if (mIgnoreBaseConfValidation) {
         return true;
      }
      mValidConf = mValidationOfConf.ValidateBaseConf(msg);
      return mValidConf;
   }

   bool ValidateSyslogConf(protoMsg::SyslogConf& msg) LR_OVERRIDE {
      if (mIgnoreSyslogConfValidation) {
         return true;
      }
      mValidConf = mValidationOfConf.ValidateSyslogConf(msg);
      return mValidConf;
   }

   ValidateConf mValidationOfConf;
   bool mValidConf;
   bool mIgnoreSyslogConfValidation;
   bool mIgnoreBaseConfValidation;
};


