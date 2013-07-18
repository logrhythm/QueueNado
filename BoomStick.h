#pragma once
#include <string>

#include <unordered_map>
#include <unordered_set>
#include "include/global.h"
struct _zctx_t;
typedef struct _zctx_t zctx_t;
typedef std::pair<std::string, time_t> MessageIdentifier;

class BoomStick {
public:
   explicit BoomStick(const std::string& binding);
   BoomStick(BoomStick&& other);
   virtual ~BoomStick();

   BoomStick& operator=(BoomStick&& other);
   LR_VIRTUAL bool Initialize();
   LR_VIRTUAL std::string Send(const std::string& command);
   LR_VIRTUAL bool SendAsync(const MessageIdentifier& uuid, const std::string& command);
   LR_VIRTUAL bool GetAsyncReply(const MessageIdentifier& uuid, const unsigned int msToWait, std::string& reply);
   void Swap(BoomStick& other);
   void SetBinding(const std::string& binding);
   bool FindPendingId(const MessageIdentifier& id) const;
   zctx_t* GetContext();
protected:
   LR_VIRTUAL zctx_t* GetNewContext();
   LR_VIRTUAL void* GetNewSocket(zctx_t* ctx);
   LR_VIRTUAL bool ConnectToBinding(void* socket, const std::string& binding);
   LR_VIRTUAL std::string HashMessageId(const MessageIdentifier& messageid) const;
   LR_VIRTUAL bool FindPendingHash(const std::string& hash) const;
   LR_VIRTUAL void CleanOldPendingData();
   LR_VIRTUAL bool GetReplyFromSocket(const std::string& messageHash, const unsigned int msToWait, std::string& reply);
   bool GetReplyFromCache(const std::string& messageHash, std::string& reply);
   bool CheckForMessagePending(const std::string& messageHash, const unsigned int msToWait, std::string& reply);
   bool ReadFromReadySocket(std::string& foundId, std::string& foundReply);
   std::unordered_map<std::string, std::string> mUnreadReplies;
   time_t mLastGCTime;
private:
   std::unordered_map<std::string, MessageIdentifier> mPendingReplies;
   std::string mBinding;
   void *mChamber;
   zctx_t *mCtx;

};
