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

/**
 * Construct with a ZMQ socket binding
 * @param binding
 *   The binding is stored, but Initialize must be used to connect to it.
 */
BoomStick::BoomStick(const std::string& binding) : mLastGCTime(time(NULL)), mBinding(binding), mChamber(nullptr), mCtx(nullptr) {
}

/**
 * Deconstruct
 *   This destroys the context and any associated sockets
 */
BoomStick::~BoomStick() {
   zctx_destroy(&mCtx);
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
      zctx_destroy(&mCtx);
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
   return zctx_new();
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

/**
 * Does the current Id exist in the list of pending replies
 * @param id
 * @return 
 */
bool BoomStick::FindPendingId(const MessageIdentifier& id) const {
   std::string messageHash = HashMessageId(id);
   return FindPendingHash(messageHash);
}

/**
 * Does the current hashed id is in the list of pending replies
 * @param hash
 * @return 
 */
bool BoomStick::FindPendingHash(const std::string& hash) const {
   return mPendingReplies.find(hash) != mPendingReplies.end();
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
   MessageIdentifier id = std::make_pair("synchronous", time(NULL));

   if (!SendAsync(id, command)) {
      return
      {
      };
   }
   std::string returnString;
   if (!GetAsyncReply(id, 100, returnString)) {
      return
      {
      };
   }
   return returnString;
}

/**
 * Hash the contents of the MessageIdentifier type into something that can be used
 *   in hashmaps
 * @param messageid
 *   The message id
 * @return 
 *   A string that is a simple hash of the contents of the id
 */
std::string BoomStick::HashMessageId(const MessageIdentifier& messageid) const {
   std::stringstream messageHash;
   messageHash << messageid.first << messageid.second;
   return messageHash.str();
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
bool BoomStick::SendAsync(const MessageIdentifier& uuid, const std::string& command) {
   if (nullptr == mCtx || nullptr == mChamber) {
      return false;
   }
   std::string messageHash = HashMessageId(uuid);
   if (FindPendingHash(messageHash)) {
      //LOG(DEBUG) << "Attempted to re-send " << messageHash;
      return false;
   }
   zmsg_t* msg = zmsg_new();
   if (zmsg_addmem(msg, messageHash.c_str(), messageHash.size()) < 0) {
      LOG(WARNING) << "queue error " << zmq_strerror(zmq_errno());
      zmsg_destroy(&msg);
      return false;
   }
   if (zmsg_addmem(msg, command.c_str(), command.size()) < 0) {
      LOG(WARNING) << "queue error " << zmq_strerror(zmq_errno());
      zmsg_destroy(&msg);
      return false;
   }
   if (zmsg_send(&msg, mChamber) < 0) {
      LOG(WARNING) << "queue error " << zmq_strerror(zmq_errno());
      return false;
   }

   mPendingReplies[messageHash] = uuid;
   return true;
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
      mPendingReplies.erase(messageHash);
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
   if (!zsocket_poll(mChamber, msToWait)) {
      reply = "socket timed out";
      LOG(DEBUG) << "Failed to find any new messages on socket while looking for a message";
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
   zmsg_t* msg = zmsg_recv(mChamber);
   if (!msg) {
      foundReply = zmq_strerror(zmq_errno());
      zmsg_destroy(&msg);
      return false;
   }
   if (zmsg_size(msg) != 2) {
      foundReply = "Malformed reply, expecting 2 parts";
      zmsg_destroy(&msg);
      return false;
   }
   char* msgChar;
   msgChar = zmsg_popstr(msg);
   foundId = msgChar;
   free(msgChar);
   msgChar = zmsg_popstr(msg);
   foundReply = msgChar;
   free(msgChar);
   zmsg_destroy(&msg);
   return true;
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
bool BoomStick::GetAsyncReply(const MessageIdentifier& uuid, const unsigned int msToWait, std::string& reply) {
   if (nullptr == mCtx || nullptr == mChamber) {
      reply = "No socket";
      CleanOldPendingData();
      return false;
   }
   std::string messageHash = HashMessageId(uuid);
   if (!FindPendingHash(messageHash)) {
      LOG(WARNING) << "Tried to get a reply for a message more than once";
      reply = "ID is not pending";
      CleanOldPendingData();
      return false;
   }
   bool found = GetReplyFromCache(messageHash, reply);
   if (!found) {
      found = GetReplyFromSocket(messageHash, msToWait, reply);
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
bool BoomStick::GetReplyFromSocket(const std::string& messageHash, const unsigned int msToWait, std::string& reply) {
   bool found = false;
   reply = "Timed out searching for reply";
   while (!found && CheckForMessagePending(messageHash, msToWait, reply)) {
      std::string foundId;
      if (!ReadFromReadySocket(foundId, reply)) {
         break;
      }
      if (messageHash == foundId) {
         found = true;
         mPendingReplies.erase(messageHash);
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
   std::vector<std::string> hashesToRemove;
   for (auto pendingSend : mPendingReplies) {
      if (pendingSend.second.second <= now - 5 * MINUTES_TO_SECONDS) {
         hashesToRemove.push_back(pendingSend.first);
      }
   }
   int deleteUnread = 0;
   for (auto hash : hashesToRemove) {
      mPendingReplies.erase(hash);
      if (mUnreadReplies.find(hash) != mUnreadReplies.end()) {
         deleteUnread++;
         mUnreadReplies.erase(hash);
      }
   }
   LOG_IF(INFO, (deleteUnread > 0)) << "Deleted " << deleteUnread << " unread replies that exceed the 5 minute timeout";
}

/**
 * Cleanup unread replies that don't exist in pending.
 */
void BoomStick::CleanUnreadReplies() {
   std::vector<std::string> hashesToRemove;
   //check all unread replies and remove any that don't exist in pending.
   for (auto unreadReply : mUnreadReplies) {
      if (mPendingReplies.find(unreadReply.first) == mPendingReplies.end()) {
         LOG(WARNING) << "Found unmatched reply to unknown hash " << unreadReply.first;
         hashesToRemove.push_back(unreadReply.first);
      }
   }

   int count = 0;
   for (auto hash : hashesToRemove) {
      count++;
      mUnreadReplies.erase(hash);
   }
   LOG_IF(INFO, (count > 0)) << "Deleted " << count << " replies that no longer exist in pending";
}