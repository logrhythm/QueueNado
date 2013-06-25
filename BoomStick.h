#pragma once
#include <string>
#include <czmq.h>
#include <zctx.h>
#include <zsocket.h>
#include <zmsg.h>
#include <zsocket.h>
#include <zframe.h>
#include <unordered_map>
#include "include/global.h"
class BoomStick {
public:
   explicit BoomStick(const std::string& binding);
   virtual ~BoomStick();
   
   LR_VIRTUAL bool Initialize();
   LR_VIRTUAL std::string Send(const std::string& command);
   LR_VIRTUAL bool SendAsync(const std::string& uuid, const std::string& command) {
      return false;
   }
   LR_VIRTUAL std::string GetAsyncReply(const std::string& uuid) {
      return {};
   }
private:
   std::unordered_map<std::string, std::string> pendingReplies;
   std::unordered_map<std::string, std::string> unreadReplies;
   std::string mBinding;
   void *mChamber;
   zctx_t *mCtx;
};
