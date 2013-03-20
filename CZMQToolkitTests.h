/* 
 * File:   CZMQToolkitTests.h
 * Author: root
 *
 * Created on November 16, 2012, 10:39 AM
 */

#pragma once

#include "gtest/gtest.h"
#include <pthread.h>
#include <sys/time.h>
#include <iostream>
#include <sstream>
#include <string>
#include <set>
#include "ZeroMQTests.h"
#include "CZMQToolkit.h"

class CZMQToolkitTests : public /*::testing::Test*/ ZeroMQTests {
public:

   CZMQToolkitTests() : mContext(NULL), mRepSocket(NULL), mReqSocket(NULL), mReqSocket2(NULL) {
   }

   virtual ~CZMQToolkitTests() {
   }

protected:

   virtual void SetUp() {
      std::stringstream makeATarget;
      makeATarget << "ipc:///tmp/ipc.test" << boost::this_thread::get_id();
      mTarget = makeATarget.str();
      srandom((unsigned) time(NULL));
      zctx_interrupted = false;
      mContext = zctx_new();
      zctx_set_linger(mContext, 10); // linger for a millisecond on close
      zctx_set_hwm(mContext, 10); // HWM on internal thread communicaiton
      zctx_set_iothreads(mContext, 2);
      void* socket = zsocket_new(mContext, ZMQ_REP);
      if (socket != NULL) {
         zsocket_set_hwm(socket, 1);
         zsocket_set_linger(socket, 0);
         int connectRetries = 100;
         while ((zsocket_bind(socket, mTarget.c_str()) < 0) && connectRetries-- > 0) {
            zclock_sleep(100);
         }
         if (connectRetries > 0) {
            mRepSocket = socket;
            socket = zsocket_new(mContext, ZMQ_REQ);
            if (socket != NULL) {
               zsocket_set_hwm(socket, 1);
               zsocket_set_linger(socket, 0);
               int connectRetries = 100;
               while ((zsocket_connect(socket, mTarget.c_str()) < 0) && connectRetries-- > 0) {
                  zclock_sleep(100);
               }
               if (connectRetries > 0) {
                  mReqSocket = socket;
               }
            }
            socket = zsocket_new(mContext, ZMQ_REQ);
            if (socket != NULL) {
               zsocket_set_hwm(socket, 1);
               zsocket_set_linger(socket, 0);
               int connectRetries = 100;
               while ((zsocket_connect(socket, mTarget.c_str()) < 0) && connectRetries-- > 0) {
                  zclock_sleep(100);
               }
               if (connectRetries > 0) {
                  mReqSocket2 = socket;
               }
            }
         }
      }


   }

   virtual void TearDown() {
      raise(SIGTERM);
      if (mContext) {
         zctx_destroy(&mContext);
      }
      
   }

   std::string mTarget;
   uLong t_header;
   zctx_t* mContext;
   void* mRepSocket;
   void* mReqSocket;
   void* mReqSocket2;
};