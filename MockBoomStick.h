#pragma once
#include "BoomStick.h"
#ifdef LR_DEBUG
class MockBoomStick : public BoomStick {
public:

   explicit MockBoomStick(const std::string& binding) : BoomStick(binding) {
   }

   virtual ~MockBoomStick() {
   }

   bool Initialize() override {
      return true;
   }

   std::string Send(const std::string& command) override {
      if (mReturnString.empty()) {
         return BoomStick::Send(command);
      } else {
         return mReturnString;
      }
   }
   std::string mReturnString;
};
#endif

