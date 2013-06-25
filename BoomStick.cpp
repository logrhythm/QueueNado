#include "BoomStick.h"
#include <iostream>
BoomStick::BoomStick(const std::string& binding) : mBinding(binding), mChamber(NULL), mCtx(NULL) {
}

BoomStick::~BoomStick() {
}

bool BoomStick::Initialize() {

   mCtx = zctx_new();
   if (!mCtx) {
      return false;
   }
   mChamber = zsocket_new(mCtx, ZMQ_DEALER);
   if (!mChamber) {
      zctx_destroy(&mCtx);

      return false;
   }
   if (zsocket_connect(mChamber, mBinding.c_str()) < 0) {
      zctx_destroy(&mCtx);
      mChamber = NULL;
      return false;
   }
   return true;
}

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