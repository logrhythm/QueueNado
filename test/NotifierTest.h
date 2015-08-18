#pragma once
#include <gtest/gtest.h>
#include <FileIO.h>

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
      FileIO::RemoveFileAsRoot(notifierIPCPath).HasSuccess();
      FileIO::RemoveFileAsRoot(handshakeIPCPath).HasSuccess();
   };

private:
   const std::string notifierIPCPath = "/tmp/RestartServicesQueue.ipc";
   const std::string handshakeIPCPath = "/tmp/RestartServicesHandshakeQueue.ipc";

};