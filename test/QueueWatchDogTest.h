/* 
 * File:   QueueWatchDogTest.h
 * Author: Craig Cogdill
 * Date Created: August 18, 2015 1:14PM
 */

#pragma once
#include "gtest/gtest.h"
#include <memory>
#include <csignal>
#include <czmq.h>
#include <FileIO.h>


class QueueWatchDogTest : public ::testing::Test {
public:
   const std::string ipcPrefix = "ipc:///tmp/";
   const std::string notifierQueueName = "QueueWatchDogTest.ipc";
   const std::string notifierHandshakeQueueName = "QueueWatchDogTestHandshake.ipc";
   const std::string notifierQueue = ipcPrefix + notifierQueueName;
   const std::string notifierHandshakeQueue = ipcPrefix + notifierHandshakeQueueName;

   QueueWatchDogTest() {};
   virtual ~QueueWatchDogTest(){}

protected:
   virtual void SetUp() {
      zctx_interrupted = false;
      // ignore the return from this. It's just for cleanup in case anything before had crashed
      FileIO::RemoveFileAsRoot(notifierQueue);
      FileIO::RemoveFileAsRoot(notifierHandshakeQueue);

   };

   virtual void TearDown() {
      zctx_interrupted = false;
   };


};