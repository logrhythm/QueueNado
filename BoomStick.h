#pragma once
#include <string>

#include <unordered_map>
#include <unordered_set>
#include "include/global.h"
#include "boost/uuid/uuid.hpp"
#include "boost/uuid/uuid_generators.hpp"
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
   LR_VIRTUAL bool SendAsync(const std::string& uuid, const std::string& command);
   LR_VIRTUAL bool GetAsyncReply(const std::string& uuid, const unsigned int msToWait, std::string& reply);
   std::string GetUuid();
   void Swap(BoomStick& other);
   void SetBinding(const std::string& binding);
   zctx_t* GetContext();
protected:
   LR_VIRTUAL zctx_t* GetNewContext();
   LR_VIRTUAL void* GetNewSocket(zctx_t* ctx);
   LR_VIRTUAL bool ConnectToBinding(void* socket, const std::string& binding);
   LR_VIRTUAL bool FindPendingUuid(const std::string& uuid) const;
   LR_VIRTUAL void CleanOldPendingData();
   LR_VIRTUAL void CleanPendingReplies();
   LR_VIRTUAL void CleanUnreadReplies();
   LR_VIRTUAL bool GetReplyFromSocket(const std::string& uuid, const unsigned int msToWait, std::string& reply);
   bool GetReplyFromCache(const std::string& uuid, std::string& reply);
   bool CheckForMessagePending(const std::string& messageHash, const unsigned int msToWait, std::string& reply);
   bool ReadFromReadySocket(std::string& foundId, std::string& foundReply);
   std::unordered_map<std::string, std::string> mUnreadReplies;
   time_t mLastGCTime;
private:
   std::unordered_map<std::string, time_t> mPendingReplies;
   std::string mBinding;
   void *mChamber;
   zctx_t *mCtx;
   boost::mt19937 mRan;
   boost::uuids::basic_random_generator<boost::mt19937> m_uuidGen;
};
