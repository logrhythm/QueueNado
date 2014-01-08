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
   mIgnoreConfValidation(true){}
   virtual ~MockValidateConf(){}

   
   bool ValidateConfFieldValues(::google::protobuf::Message& msg, const std::string &type) LR_OVERRIDE {
      if (mIgnoreConfValidation) {
         return true;
      }
      mValidConf = mValidationOfConf.ValidateConfFieldValues(msg, type);
      return mValidConf;
   }

   ValidateConf mValidationOfConf;
   bool mValidConf;
   bool mIgnoreConfValidation;
};


