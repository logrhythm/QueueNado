#include "BoomStick.h"
#include "g2log.hpp"
#include <czmq.h>
#include <zctx.h>
#include <zsocket.h>
#include <zmsg.h>
#include <zsocket.h>
#include <zframe.h>
#include <iostream>
#include <time.h>
#include <vector>
#include "boost/uuid/uuid_io.hpp"

/**
 * Construct with a ZMQ socket binding
 * @param binding
 *   The binding is stored, but Initialize must be used to connect to it.
 */
BoomStick::BoomStick(const std::string& binding) : mLastGCTime(time(NULL)),
mBinding(binding), mChamber(nullptr), mCtx(nullptr), mRan(), m_uuidGen(mRan) {
   mRan.seed(boost::uuids::detail::seed_rng()());
}

/**
 * Deconstruct
 *   This destroys the context and any associated sockets
 */
BoomStick::~BoomStick() {
   if (mCtx != nullptr) {
      zctx_destroy(&mCtx);
   }
   if (!mPendingReplies.empty()) {
      LOG(WARNING) << "Pending replies never emptied " << mPendingReplies.size();
      for (auto reply : mPendingReplies) {
         LOG(WARNING) << reply.first;
      }
   }
   if (!mUnreadReplies.empty()) {
      LOG(WARNING) << "mUnreadReplies replies never emptied " << mUnreadReplies.size();
      for (auto reply : mUnreadReplies) {
         LOG(WARNING) << reply.first;
      }
   }
}

/**
 * Get the context (can be safely shared between threads)
 * @return 
 */
zctx_t* BoomStick::GetContext() {
   return mCtx;
}

/**
 * Swap internals
 * @param other
 */
void BoomStick::Swap(BoomStick& other) {
   mBinding = other.mBinding;
   mChamber = other.mChamber;
   mCtx = other.mCtx;
   mLastGCTime = other.mLastGCTime;
   mRan = other.mRan;
   m_uuidGen = other.m_uuidGen;
   //   other.mBinding.clear();  Allow it to be initialized again
   other.mChamber = nullptr;
   other.mCtx = nullptr;
}

/**
 * Move constructor
 * @param other
 *   A BoomStick that is presumably setup already
 */
BoomStick::BoomStick(BoomStick&& other) {
   Swap(other);
}

/**
 * Move assignment operator
 * @param other
 *   A Boomstick that is presumably setup already
 * @return 
 *   A reference to this
 */
BoomStick& BoomStick::operator=(BoomStick&& other) {
   if (this != &other) {
      if (nullptr != mCtx) {
         zctx_destroy(&mCtx);
      }
      Swap(other);
   }
   return *this;
}

/**
 * Get a brand new ZMQ context
 * @return 
 *   A pointer to the context
 */
zctx_t* BoomStick::GetNewContext() {
   zctx_t* context = zctx_new();
   return context;
}

/**
 * Open a new socket (DEALER) on the given context
 * @param ctx
 *   An existing context
 * @return 
 *   A pointer to a socket
 */
void* BoomStick::GetNewSocket(zctx_t* ctx) {
   if (nullptr == ctx) {
      return nullptr;
   }
   return zsocket_new(ctx, ZMQ_DEALER);
}

std::string BoomStick::GetUuid() {
   boost::uuids::uuid u1 = m_uuidGen();

   std::stringstream ss;
   ss << u1;

   return ss.str();
}

/**
 * Connect the given socket to the given binding
 * @param socket
 *   An existing socket
 * @param binding
 *   A string binding 
 * @return 
 *   If connection was successful
 */
bool BoomStick::ConnectToBinding(void* socket, const std::string& binding) {
   if (nullptr == socket) {
      return false;
   }
   return (zsocket_connect(socket, binding.c_str()) >= 0);
}

/**
 * set the binding to something else, de-initialize
 * @param binding
 */
void BoomStick::SetBinding(const std::string& binding) {
   if (nullptr != mCtx) {
      zctx_destroy(&mCtx);
      mCtx = nullptr;
      mChamber = nullptr;
   }
   mBinding = binding;
}

/**
 * Initialize the context, socket and connect to the bound address
 * @return 
 *   true when successful
 */
bool BoomStick::Initialize() {
   if (nullptr != mCtx) {
      return true;
   }
   mCtx = GetNewContext();
   if (nullptr == mCtx) {
      LOG(WARNING) << "queue error " << zmq_strerror(zmq_errno());
      return false;
   }
   mChamber = GetNewSocket(mCtx);
   // The memory in this pointer is managed by the context and should not be deleted
   if (nullptr == mChamber) {
      LOG(WARNING) << "queue error " << zmq_strerror(zmq_errno());
      zctx_destroy(&mCtx);
      mCtx = nullptr;
      return false;
   }
   if (!ConnectToBinding(mChamber, mBinding)) {

      zctx_destroy(&mCtx);
      mChamber = nullptr;
      mCtx = nullptr;
      return false;
   }
   return true;
}

bool BoomStick::FindPendingUuid(const std::string& uuid) const {
   return mPendingReplies.find(uuid) != mPendingReplies.end();
}

/**
 * A Synchronous send with a blocking receive.
 * 
 * @param command
 *   A string to send
 * @return 
 *   The reply received
 */
std::string BoomStick::Send(const std::string& command) {
   const std::string uuid = GetUuid();

   if (!SendAsync(uuid, command)) {
      return
      {
      };
   }
   std::string returnString;
   if (!GetAsyncReply(uuid, 30000, returnString)) {
      return
      {
      };
   }
   return returnString;
}

/**
 * Send a message, but leave the reply on the socket
 * @param uuid
 *   A unique identifier for this send
 * @param command
 *   The string that will be sent
 * @return 
 *   If the send was successful
 */
bool BoomStick::SendAsync(const std::string& uuid, const std::string& command) {
   if (nullptr == mCtx || nullptr == mChamber) {
      return false;
   }
   bool success = true;
   if (FindPendingUuid(uuid)) {
      //LOG(DEBUG) << "Attempted to re-send " << messageHash;
      return false;
   }
   zmsg_t* msg = zmsg_new();
   if (zmsg_addmem(msg, uuid.c_str(), uuid.size()) < 0) {
      LOG(WARNING) << "queue error " << zmq_strerror(zmq_errno());
   }
   if (zmsg_addmem(msg, command.c_str(), command.size()) < 0) {
      LOG(WARNING) << "queue error " << zmq_strerror(zmq_errno());
   }
   zmq_pollitem_t items[1];
   items[0].socket = mChamber;
   items[0].events = ZMQ_POLLOUT;
   int rc = zmq_poll(items, 1, 0);
   if (rc < 0) {
      LOG(WARNING) << "Queue error, cannot poll for status";
   } else if (rc == 1) {
      if ((items[0].revents & ZMQ_POLLOUT) != ZMQ_POLLOUT) {
         LOG(WARNING) << "Queue error, cannot send messages the queue is full";
      } else if (zmsg_send(&msg, mChamber) == 0) {
         success = true;
         mPendingReplies[uuid] = std::time(NULL);
      } else {
         LOG(WARNING) << "queue error " << zmq_strerror(zmq_errno());
      }
   } else {
      LOG(WARNING) << "Queue error, timeout waiting for queue to be ready";
   }

   if (msg) {
      zmsg_destroy(&msg);
   }

   return success;
}

/**
 * Attempt to grab the reply from the previously read messages
 * 
 * @param messageHash
 * @param reply
 * @return 
 */
bool BoomStick::GetReplyFromCache(const std::string& messageHash, std::string& reply) {
   if (mUnreadReplies.find(messageHash) != mUnreadReplies.end()) {
      reply = mUnreadReplies[messageHash];
      mUnreadReplies.erase(messageHash);
      if (mPendingReplies.find(messageHash) != mPendingReplies.end()) {
         mPendingReplies.erase(messageHash);
      } else {
         LOG(WARNING) << "Found reply in cache, but it was never pending" << messageHash;
      }
      return true;
   }
   return false;
}

/**
 * Poll the socket, fail after timeout and log
 * @param messageHash
 * @return 
 */
bool BoomStick::CheckForMessagePending(const std::string& messageHash, const unsigned int msToWait, std::string& reply) {
   if (!mChamber) {
      LOG(WARNING) << "Invalid socket";
      return false;
   }
   if (!zsocket_poll(mChamber, msToWait)) {
      reply = "socket timed out";
      return false;
   }
   return true;
}

/**
 * 
 * @param foundId
 * @param foundReply
 * @return 
 */
bool BoomStick::ReadFromReadySocket(std::string& foundId, std::string& foundReply) {
   if (!mChamber) {
      LOG(WARNING) << "Invalid socket";
      return false;
   }
   bool success = false;
   zmsg_t* msg = zmsg_recv(mChamber);
   if (!msg) {
      foundReply = zmq_strerror(zmq_errno());
   } else if (zmsg_size(msg) == 2) {
      char* msgChar;
      msgChar = zmsg_popstr(msg);
      foundId = msgChar;
      free(msgChar);
      msgChar = zmsg_popstr(msg);
      foundReply = msgChar;
      free(msgChar);
      success = true;
   } else {
      foundReply = "Malformed reply, expecting 2 parts";
   }

   if (msg) {
      zmsg_destroy(&msg);
   }
   return success;
}

/**
 * Pull from the socket till the requested reply is found
 * 
 * @param uuid
 *   An id of a message that has previously been sent
 * @param reply
 *   The reply 
 * @return 
 *   if the pull was successful.  Can timeout and return false.  Error info will 
 * be in the reply return. 
 */
bool BoomStick::GetAsyncReply(const std::string& uuid, const unsigned int msToWait, std::string& reply) {
   if (nullptr == mCtx || nullptr == mChamber) {
      LOG(WARNING) << "Invalid socket";
      reply = "No socket";
      CleanOldPendingData();
      return false;
   }

   bool found = GetReplyFromCache(uuid, reply);
   if (!found) {
      found = GetReplyFromSocket(uuid, msToWait, reply);
   }
   CleanOldPendingData();
   return found;
}

/**
 * Check the socket for a specific reply, also fill the cache when other replies 
 *   are seen
 * @param messageHash
 * @param reply
 *   Either the reply, or when an error occurs an error message
 * @return 
 *   If the message was found
 */
bool BoomStick::GetReplyFromSocket(const std::string& uuid, const unsigned int msToWait, std::string& reply) {
   bool found = false;
   reply = "Timed out searching for reply";
   while (!found && CheckForMessagePending(uuid, msToWait, reply)) {
      std::string foundId;
      if (!ReadFromReadySocket(foundId, reply)) {
         break;
      }
      if (uuid == foundId) {
         found = true;
         mPendingReplies.erase(uuid);
      } else {
         mUnreadReplies[foundId] = reply;
      }
   }
   return found;
}

/**
 * Clean up pending sends/replies that are older than 5 minutes 
 */
void BoomStick::CleanOldPendingData() {
   const auto unreadSize = mUnreadReplies.size();
   const auto pendingSize = mPendingReplies.size();
   LOG_IF(WARNING, (pendingSize >= 500)) << "sizes mUnreadReplies: " << unreadSize << " mPendingReplies: " << pendingSize;
   CleanUnreadReplies();
   CleanPendingReplies();
}

/**
 * Cleanup pending replies that have exceeded our timeout.
 */
void BoomStick::CleanPendingReplies() {
   time_t now = time(NULL);
   if (now < mLastGCTime + 5 * MINUTES_TO_SECONDS) {
      return;
   }
   mLastGCTime = now;
   std::vector<std::string> uuidsToRemove;
   for (auto pendingSend : mPendingReplies) {
      if (pendingSend.second <= now - 5 * MINUTES_TO_SECONDS) {
         uuidsToRemove.push_back(pendingSend.first);
      }
   }
   int deleteUnread = 0;
   for (auto uuid : uuidsToRemove) {
      mPendingReplies.erase(uuid);
      if (mUnreadReplies.find(uuid) != mUnreadReplies.end()) {
         deleteUnread++;
         mUnreadReplies.erase(uuid);
      }
   }
   LOG_IF(INFO, (deleteUnread > 0)) << "Deleted " << deleteUnread << " unread replies that exceed the 5 minute timeout";
   if (mPendingReplies.size() == 0) {
      std::unordered_map<std::string, time_t> temp;
      mPendingReplies.swap(temp);
   }
}

/**
 * Cleanup unread replies that don't exist in pending.
 */
void BoomStick::CleanUnreadReplies() {
   std::vector<std::string> uuidsToRemove;
   //check all unread replies and remove any that don't exist in pending.
   for (auto unreadReply : mUnreadReplies) {
      if (mUnreadReplies.find(unreadReply.first) == mUnreadReplies.end()) {
         LOG(WARNING) << "Found unmatched reply to unknown UUID " << unreadReply.first;
         uuidsToRemove.push_back(unreadReply.first);
      }
   }

   int count = 0;
   for (auto hash : uuidsToRemove) {
      count++;
      mUnreadReplies.erase(hash);
   }
   LOG_IF(INFO, (count > 0)) << "Deleted " << count << " replies that no longer exist in pending";

   if (mUnreadReplies.size() == 0) {
      std::unordered_map<std::string, std::string> temp;
      mUnreadReplies.swap(temp);
   }
}
