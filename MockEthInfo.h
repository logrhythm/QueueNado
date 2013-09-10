#pragma once
#include "EthInfo.h"

class MockEthInfo : public EthInfo {
public:
      
   MockEthInfo() : EthInfo(), mFakeInitialize(false), mFakeInitializeFailure(false),
   mFakeGetIFAddrs(false), mifaddr(NULL), mGetIFAddrsRetVal(-1),mFakeIsValid (false),
   mFakeIsValidValue(true){
      
   }
   virtual ~MockEthInfo() {}
   bool Initialize() {
      if (mFakeInitialize) {
         if (mFakeInitializeFailure) {
            return false;
         }
         mAvaliableInterfaces = mFakeInterfaceNames;
         return true;
      }
      return EthInfo::Initialize();
   }
   void FreeIFAddrs(struct ifaddrs *) {
      
   }
   int GetIFAddrs(struct ifaddrs **ifaddr) {
      if (mFakeGetIFAddrs) {
         if (NULL == mifaddr) {
            *ifaddr = NULL;
            return mGetIFAddrsRetVal;
         }
         *ifaddr = mifaddr;
         return mGetIFAddrsRetVal;
      }
      return EthInfo::GetIFAddrs(ifaddr);
   }
   bool IsValid() {
      if (mFakeIsValid) {
         return mFakeIsValidValue;
      }
      return EthInfo::IsValid();
   }
   bool mFakeInitialize;
   bool mFakeInitializeFailure;
   bool mFakeGetIFAddrs;
   struct ifaddrs *mifaddr;
   int mGetIFAddrsRetVal;
   std::unordered_set<std::string> mFakeInterfaceNames;
   bool mFakeIsValid;
   bool mFakeIsValidValue;
};