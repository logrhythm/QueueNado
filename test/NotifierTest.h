#pragma once
#include <gtest/gtest.h>
#include <FileIO.h>
#include <czmq.h>

class NotifierTest : public ::testing::Test {
public:

   NotifierTest() {
   };

   ~NotifierTest() {
   }

protected:

   virtual void SetUp() {
   };

   virtual void TearDown() {
      FileIO::RemoveFileAsRoot(notifierIPCPath);
      FileIO::RemoveFileAsRoot(handshakeIPCPath);
      zctx_interrupted = false;
   };

private:
   const std::string notifierIPCPath = "/tmp/RestartServicesQueue.ipc";
   const std::string handshakeIPCPath = "/tmp/RestartServicesHandshakeQueue.ipc";

};