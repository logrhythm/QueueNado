#include "BoomStick.h"
#include <iostream>

/**
 * Construct with a ZMQ socket binding
 * @param binding
 */
BoomStick::BoomStick(const std::string& binding) : mBinding(binding), mChamber(nullptr), mCtx(nullptr) {
}

/**
 * Deconstruct
 */
BoomStick::~BoomStick() {
   zctx_destroy(&mCtx);
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
   if (ctx == nullptr) {
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
   if (socket == nullptr) {
      return false;
   }
   return (zsocket_connect(socket, binding.c_str()) >= 0);
}
/**
 * set the binding to something else, de-initialize
 * @param binding
 */
void BoomStick::SetBinding(const std::string& binding) {
   if (mCtx != nullptr) {
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
   if (mCtx != nullptr) {
      return true;
   }
   mCtx = GetNewContext();
   if (mCtx  == nullptr) {
      return false;
   }
   mChamber = GetNewSocket(mCtx);
   if (mChamber == nullptr) {
      zctx_destroy(&mCtx);

      return false;
   }
   if (!ConnectToBinding(mChamber, mBinding)) {
      zctx_destroy(&mCtx);
      mChamber = nullptr;
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
   if (mCtx == nullptr || mChamber == nullptr) {
      return
      {
      };
   }
   //std::cout << "sent " << command << std::endl;
   zmsg_t* msg = zmsg_new();
   zmsg_addstr(msg, command.c_str());
   if (zmsg_send(&msg, mChamber) < 0) {
      return
      {
      };
   }
   msg = zmsg_recv(mChamber);
   if (!msg) {
      return
      {
      };
   }
   std::string returnString = zmsg_popstr(msg);
   zmsg_destroy(&msg);
   return returnString;
}