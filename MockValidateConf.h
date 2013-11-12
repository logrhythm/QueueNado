/* 
 * File:   MockConf.h
 *
 * Created on December 26, 2012, 10:39 AM
 */

#pragma once
#include "ValidateConf.h"
#include "include/global.h"
#include "Range.h"
#include <g2log.hpp>
#include <functional>
#include <exception>

class MockValidateConf : public ValidateConf {
public:

    MockValidateConf() :

    mValidConf(true),
    mIgnoreSyslogConfValidation(true),
    mIgnoreBaseConfValidation(true){   
    }


    ~MockValidateConf(){ 
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

//    bool CheckNumber(const std::string& number, const Range& range) LR_OVERRIDE {
//        return ValidateChecks::CheckNumber(number, range);
//    }
//
//    void CheckNumberForNegative(const std::string& number) LR_OVERRIDE {
//        try {
//            ValidateChecks::CheckNumberForNegative(number);
//        } catch (std::exception e) {
//            LOG(DEBUG) << e.what();
//            mValidConf = false;
//            throw;
//        }
//        mValidConf = true;
//    }
//
//    void CheckNumberForSize(const std::string& number, const Range& range) LR_OVERRIDE {
//        try {
//            ValidateChecks::CheckNumberForSize(number, range);
//        } catch (std::exception e) {
//            LOG(DEBUG) << e.what();
//            mValidConf = false;
//            throw;
//        }
//        mValidConf = true;
//    }
//
//    bool CheckString(const std::string& text) LR_OVERRIDE {
//        return ValidateChecks::CheckString(text);
//    }
//
//    void CheckStringForSize(const std::string& text) LR_OVERRIDE {
//        try {
//            ValidateChecks::CheckStringForSize(text);
//        } catch (std::exception e) {
//            LOG(DEBUG) << e.what();
//            mValidConf = false;
//            throw;
//        }
//        mValidConf = true;
//    }
//
//    bool CheckBool(const std::string& text) LR_OVERRIDE {
//        mValidConf = ValidateChecks::CheckBool(text);
//        return mValidConf;
//    }

    ValidateConf mValidationOfConf;
    bool mValidConf;
    bool mIgnoreSyslogConfValidation;
    bool mIgnoreBaseConfValidation;
};


