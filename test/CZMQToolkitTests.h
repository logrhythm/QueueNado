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

   CZMQToolkitTests() : mContext(nullptr), mRepSocket(nullptr), mReqSocket(nullptr) {
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

      mRepSocket = zsocket_new(mContext, ZMQ_REP);
      ASSERT_NE(nullptr, mRepSocket);
      zsocket_set_hwm(mRepSocket, 1);
      zsocket_set_linger(mRepSocket, 0);
      int bindRetries = 100;
      while ((zsocket_bind(mRepSocket, mTarget.c_str()) < 0) && bindRetries-- > 0) {
         zclock_sleep(100);
      }
      ASSERT_LT(0, bindRetries);

      mReqSocket = zsocket_new(mContext, ZMQ_REQ);
      ASSERT_NE(nullptr, mReqSocket);
      zsocket_set_hwm(mReqSocket, 1);
      zsocket_set_linger(mReqSocket, 0);
      int connectRetries = 100;
      while ((zsocket_connect(mReqSocket, mTarget.c_str()) < 0) && connectRetries-- > 0) {
         zclock_sleep(100);
      }
      ASSERT_LT(0, connectRetries);
   }

   virtual void TearDown() {
      raise(SIGTERM);
      ASSERT_EQ(0, zsocket_unbind(mRepSocket, mTarget.c_str()));
      mRepSocket = nullptr;
      ASSERT_EQ(0, zsocket_disconnect(mReqSocket, mTarget.c_str()));
      mReqSocket = nullptr;
   }

   std::string mTarget;
   zctx_t* mContext;
   void* mRepSocket;
   void* mReqSocket;
};
