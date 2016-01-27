/* 
 * File:   CZMQToolkitTests.h
 * Author: root
 *
 * Created on November 16, 2012, 10:39 AM
 */

#pragma once

#include "gtest/gtest.h"
#include <sys/time.h>
#include <sstream>
#include <string>
#include "CZMQToolkit.h"
#include <czmq.h>
#include <boost/thread.hpp>

class CZMQToolkitTests : public ::testing::Test {
public:

   CZMQToolkitTests() : mContext(nullptr), mReplySocket(nullptr), mRequestSocket(nullptr) {
   }

   virtual ~CZMQToolkitTests() {
   }

protected:

   virtual void SetUp() {
      std::stringstream makeATarget;
      makeATarget << "ipc:///tmp/ipc.test" << boost::this_thread::get_id();
      mTarget = makeATarget.str();
      srandom((unsigned) time(nullptr));
      zctx_interrupted = false;
      if (mContext) {
         zctx_destroy(&mContext);
      }
      mContext = zctx_new();
      zctx_set_linger(mContext, 10); // linger for a millisecond on close
      zctx_set_rcvhwm(mContext, 10);
      zctx_set_sndhwm(mContext, 10); // HWM on internal thread communicaiton
      zctx_set_iothreads(mContext, 2);

      mReplySocket = zsocket_new(mContext, ZMQ_REP);
      ASSERT_NE(nullptr, mReplySocket);
      zsocket_set_hwm(mReplySocket, 1);
      zsocket_set_linger(mReplySocket, 0);
      int bindRetries = 100;
      while ((zsocket_bind(mReplySocket, mTarget.c_str()) < 0) && bindRetries-- > 0) {
         zclock_sleep(100);
      }
      ASSERT_LT(0, bindRetries);

      mRequestSocket = zsocket_new(mContext, ZMQ_REQ);
      ASSERT_NE(nullptr, mRequestSocket);
      zsocket_set_hwm(mRequestSocket, 1);
      zsocket_set_linger(mRequestSocket, 0);
      int connectRetries = 100;
      while ((zsocket_connect(mRequestSocket, mTarget.c_str()) < 0) && connectRetries-- > 0) {
         zclock_sleep(100);
      }
      ASSERT_LT(0, connectRetries);
   }

   virtual void TearDown() {
      raise(SIGTERM);
      ASSERT_EQ(0, zsocket_unbind(mReplySocket, mTarget.c_str()));
      mReplySocket = nullptr;
      ASSERT_EQ(0, zsocket_disconnect(mRequestSocket, mTarget.c_str()));
      mRequestSocket = nullptr;
   }

   std::string mTarget;
   zctx_t* mContext;
   void* mReplySocket;
   void* mRequestSocket;
};
