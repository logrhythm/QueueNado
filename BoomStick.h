#pragma once
#include <string>

#include <unordered_map>
#include "include/global.h"
struct _zctx_t;
typedef struct _zctx_t zctx_t;
class BoomStick {
public:
   explicit BoomStick(const std::string& binding);
   BoomStick(BoomStick&& other);
   virtual ~BoomStick();
   
   BoomStick& operator=(BoomStick&& other);
   LR_VIRTUAL bool Initialize();
   LR_VIRTUAL std::string Send(const std::string& command);
   LR_VIRTUAL bool SendAsync(const std::string& uuid, const std::string& command) {
      return false;
   }
   LR_VIRTUAL std::string GetAsyncReply(const std::string& uuid) {
      return {};
   }
   void Swap(BoomStick& other);
   void SetBinding(const std::string& binding);
protected:
   LR_VIRTUAL zctx_t* GetNewContext();
   LR_VIRTUAL void* GetNewSocket(zctx_t* ctx);
   LR_VIRTUAL bool ConnectToBinding(void* socket, const std::string& binding);
private:
   std::unordered_map<std::string, std::string> pendingReplies;
   std::unordered_map<std::string, std::string> unreadReplies;
   std::string mBinding;
   void *mChamber;
   zctx_t *mCtx;
};
