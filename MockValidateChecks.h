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

struct MockValidateChecks : public ValidateChecks {

    MockValidateChecks() :

    mValidCheck(true){
    }

    ~MockValidateChecks(){ 
    }

    bool CheckNumber(const std::string& number, const Range& range) LR_OVERRIDE {
        return ValidateChecks::CheckNumber(number, range);
    }

    void CheckNumberForNegative(const std::string& number) LR_OVERRIDE {
        try {
            ValidateChecks::CheckNumberForNegative(number);
        } catch (std::exception e) {
            LOG(DEBUG) << e.what();
            mValidCheck = false;
            throw;
        }
        mValidCheck = true;
    }

    void CheckNumberForSize(const std::string& number, const Range& range) LR_OVERRIDE {
        try {
            ValidateChecks::CheckNumberForSize(number, range);
        } catch (std::exception e) {
            LOG(DEBUG) << e.what();
            mValidCheck = false;
            throw;
        }
        mValidCheck = true;
    }

    bool CheckString(const std::string& text) LR_OVERRIDE {
        return ValidateChecks::CheckString(text);
    }

    void CheckStringForSize(const std::string& text) LR_OVERRIDE {
        try {
            ValidateChecks::CheckStringForSize(text);
        } catch (std::exception e) {
            LOG(DEBUG) << e.what();
            mValidCheck = false;
            throw;
        }
        mValidCheck = true;
    }

    bool CheckBool(const std::string& text) LR_OVERRIDE {
        mValidCheck = ValidateChecks::CheckBool(text);
        return mValidCheck;
    }
    
    bool mValidCheck;
};


