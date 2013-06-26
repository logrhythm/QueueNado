#include "BoomStick.h"
#include <iostream>
/**
 * Construct with a ZMQ socket binding
 * @param binding
 */
BoomStick::BoomStick(const std::string& binding) : mBinding(binding), mChamber(NULL), mCtx(NULL) {
}

/**
 * Deconstruct
 */
BoomStick::~BoomStick() {
   if (mCtx) {
      zctx_destroy(&mCtx);
   }
}
/**
 * Move constructor
 * @param other
 *   A BoomStick that is presumably setup already
 */
BoomStick::BoomStick(BoomStick&& other) {
   mBinding = other.mBinding;
   mChamber = other.mChamber;
   mCtx = other.mCtx;
//   other.mBinding.clear();  Allow it to be initialized again
   other.mChamber = NULL;
   other.mCtx = NULL;
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
      if (mCtx) {
         zctx_destroy(&mCtx);
      }
      mBinding = other.mBinding;
      mCtx = other.mCtx;
      mChamber = other.mChamber;
      other.mCtx = NULL;
      other.mChamber = NULL;
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
   if (!ctx) {
      return NULL;
   }
   return zsocket_new(ctx,ZMQ_DEALER);
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
   if (!socket) {
      return false;
   }
   return (zsocket_connect(socket, binding.c_str()) >= 0);
}
/**
 * Initialize the context, socket and connect to the bound address
 * @return 
 *   true when successful
 */
bool BoomStick::Initialize() {

   mCtx = GetNewContext();
   if (!mCtx) {
      return false;
   }
   mChamber = GetNewSocket(mCtx);
   if (!mChamber) {
      zctx_destroy(&mCtx);

      return false;
   }
   if (!ConnectToBinding(mChamber,mBinding)) {
      zctx_destroy(&mCtx);
      mChamber = NULL;
      return false;
   }
   return true;
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
   if (!mCtx || !mChamber) {
      return {};
   }
   //std::cout << "sent " << command << std::endl;
   zmsg_t* msg = zmsg_new();
   zmsg_addstr(msg,command.c_str());
   if ( zmsg_send(&msg,mChamber) < 0) {
      return {};
   }
   msg = zmsg_recv(mChamber);
   if (!msg) {
      return {};
   }
   std::string returnString = zmsg_popstr(msg);
   zmsg_destroy(&msg);
   return returnString;
}