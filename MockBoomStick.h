#pragma once
#include "BoomStick.h"
#ifdef LR_DEBUG

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

   std::string Send(const std::string& command) override {
      if (mReturnString.empty()) {
         return BoomStick::Send(command);
      } else {
         return mReturnString;
      }
   }

   bool SendAsync(const MessageIdentifier& uuid, const std::string& command) {
      if (mReturnString.empty()) {
         return BoomStick::SendAsync(uuid,command);
      } else {
         return true;
      }
   }

   bool GetAsyncReply(const MessageIdentifier& uuid, const unsigned int msToWait, std::string& reply) {
      if (mReturnString.empty()) {
         return BoomStick::GetAsyncReply(uuid,msToWait,reply);
      } else {
         reply = mReturnString;
         return true;
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
    * Peek into the replies for a given element
    * @param messageid
    * @return Empty if it isn't there
    */
   std::string GetCachedReply(const MessageIdentifier& messageid) {
      std::string messageHash = HashMessageId(messageid);
      if (mUnreadReplies.find(messageHash) != mUnreadReplies.end()) {
         return mUnreadReplies[messageHash];
      }
      return
      {
      };
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
};
#endif