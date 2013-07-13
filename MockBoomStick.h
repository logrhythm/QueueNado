#pragma once
#include "BoomStick.h"


class MockBoomStick : public BoomStick {
public:

   explicit MockBoomStick(const std::string& binding) : BoomStick(binding), mFailsInit(false),
   mFailsGetNewContext(false),
   mFailseGetNewSocket(false),
   mFailsConnect(false) {
   }

   virtual ~MockBoomStick() {
   }

   bool Initialize() {
      if (mFailsInit) {
         return false;
      }
      return BoomStick::Initialize();
   }

   std::string Send(const std::string& command) {
      if (mReturnString.empty()) {
         return BoomStick::Send(command);
      } else {
         return mReturnString;
      }
   }

   bool SendAsync(const std::string& uuid, const std::string& command) {
      return false;
   }

   std::string GetAsyncReply(const std::string& uuid) {
      return
      {
      };
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


   bool mFailsInit;
   bool mFailsGetNewContext;
   bool mFailseGetNewSocket;
   bool mFailsConnect;
   std::string mReturnString;
};