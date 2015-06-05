#pragma once
#include "BoomStick.h"
#ifdef QN_DEBUG

class MockBoomStick : public BoomStick {
public:

   explicit MockBoomStick(const std::string& binding) : BoomStick(binding), mFailsInit(false),
   mFailsGetNewContext(false),
   mFailseGetNewSocket(false),
   mFailsConnect(false),
   mReturnSocketEmpty(false) {
   }

   virtual ~MockBoomStick() {
   }

   bool Initialize() {
      if (mFailsInit) {
         return false;
      }
      return BoomStick::Initialize();
   }

   std::string Send(const std::string& command) QN_OVERRIDE {
      if (mReturnString.empty()) {
         return BoomStick::Send(command);
      } else {
         return mReturnString;
      }
   }

   bool SendAsync(const std::string& uuid, const std::string& command) QN_OVERRIDE {
      if (mReturnString.empty()) {
         return BoomStick::SendAsync(uuid,command);
      } else {
         return true;
      }
   }
   bool GetAsyncReply(const std::string& uuid, const unsigned int msToWait, std::string& reply) QN_OVERRIDE {
      if (mReturnString.empty()) {
         return BoomStick::GetAsyncReply(uuid,msToWait,reply);
      } else {
         reply = mReturnString;
         return !mReturnSocketEmpty;
      }
   }

   zctx_t* GetNewContext() {
      if (mFailsGetNewContext) {
         return nullptr;
      }
      return BoomStick::GetNewContext();
   }

   void* GetNewSocket(zctx_t* ctx) {
      if (mFailseGetNewSocket) {
         return nullptr;
      }
      return BoomStick::GetNewSocket(ctx);
   }

   bool ConnectToBinding(void* socket, const std::string& binding) {
      if (mFailsConnect) {
         return false;
      }
      return BoomStick::ConnectToBinding(socket, binding);
   }

   void CleanOldPendingData() {
      return BoomStick::CleanOldPendingData();
   }

   /**
    * Force Garbage collection
    */
   void ForceGC() {
      mLastGCTime = 0;
   }
   bool mFailsInit;
   bool mFailsGetNewContext;
   bool mFailseGetNewSocket;
   bool mFailsConnect;
   std::string mReturnString;
   bool mReturnSocketEmpty;
};
#endif