#pragma once

#include "LicenseClient.h"

class MockLicenseClient : public LicenseClient {
public:
   MockLicenseClient() : mRealInitialize(true) {}
   ~MockLicenseClient() {}
   bool Initialize() {
      if (mRealInitialize) {
         return LicenseClient::Initialize();
      }
      return false;
   }
   bool mRealInitialize;
};