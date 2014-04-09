#include <memory>
#include <atomic>
#include <thread>
#include <future>

#include "MockLicenseClient.h"
#include "LicenseClientTest.h"
#include "License.pb.h"
#include "LicenseRequest.pb.h"
#include "LicenseReply.pb.h"
#include "include/global.h"
#include "Headcrab.h"


#ifdef LR_DEBUG
TEST_F(LicenseClientTest, InitializeFailed) {
   MockLicenseClient client;
   client.mRealInitialize = false;
   std::atomic<bool> shutdown(false);
   std::promise<bool> promisedFinished;
   auto futureResult = promisedFinished.get_future();
   std::thread([](std::promise<bool>& finished,std::atomic<bool>& shutdown) {
      Headcrab listener("ipc:///tmp/licenseServer.ipc");
      listener.ComeToLife();
      protoMsg::LicenseRequest request;
      protoMsg::LicenseReply reply;
      protoMsg::RawLicense license; 
      license.set_bandwidthinmbps(1234);
      license.set_pcapfiles(1);
      license.set_pcapstorageinmb(2);
      license.set_maxindexesindays(3);
      license.set_expiretime(4);
      license.set_expiredurationindays(5);
      license.set_installationtime(6);
      reply.add_encodedlicenses(license.SerializeAsString());
      while (!shutdown) {
         std::string requestData;
         if (listener.GetHitWait(requestData,1000) ) {
            if (request.ParseFromString(requestData) && request.has_type() && 
                    (request.type() == protoMsg::LicenseRequest_RequestType_AGGREGATE)) {
              listener.SendSplatter(reply.SerializeAsString());
            } else {
               listener.SendSplatter("");
            }
         }
      }
      finished.set_value(true);
   }, std::ref(promisedFinished), std::ref(shutdown)).detach();
   
   protoMsg::RawLicense aLicense;
   EXPECT_FALSE(client.GetAggregateLicense(aLicense));
   shutdown = true;
   EXPECT_TRUE(futureResult.wait_for(std::chrono::milliseconds(1000 + 2)) != std::future_status::timeout);
}
#else
#endif

TEST_F(LicenseClientTest, constructor) {
   LicenseClient client;
   std::unique_ptr<LicenseClient> clientPtr(new LicenseClient);
}

TEST_F(LicenseClientTest, InitializeNoServer) {
   LicenseClient client;
   
   EXPECT_TRUE(client.Initialize());  // ZMQ is "nice" to us, you don't have to have a listener to connect
}

TEST_F(LicenseClientTest, InitializeListeningServer) {
   LicenseClient client;
   
   Headcrab listener("ipc:///tmp/licenseServer.ipc");
   ASSERT_TRUE(listener.ComeToLife());
   EXPECT_TRUE(client.Initialize());
}

TEST_F(LicenseClientTest, ServerThatReturnsGarbage) {
   LicenseClient client;
   std::atomic<bool> shutdown(false);
   std::promise<bool> promisedFinished;
   auto futureResult = promisedFinished.get_future();
   std::thread([](std::promise<bool>& finished,std::atomic<bool>& shutdown) {
      Headcrab listener("ipc:///tmp/licenseServer.ipc");
      listener.ComeToLife();
      while (!shutdown) {
         std::string request;
         if (listener.GetHitWait(request,1000) ) {
            listener.SendSplatter("abc123");
         }
      }
      finished.set_value(true);
   }, std::ref(promisedFinished), std::ref(shutdown)).detach();

   EXPECT_TRUE(client.Initialize());
   protoMsg::RawLicense aLicense;
   EXPECT_FALSE(client.GetAggregateLicense(aLicense));
   shutdown = true;
   EXPECT_TRUE(futureResult.wait_for(std::chrono::milliseconds(1000 + 2)) != std::future_status::timeout);
}

TEST_F(LicenseClientTest, ServerThatReturnsAnEmptyAggregateLicense) {
   LicenseClient client;
   std::atomic<bool> shutdown(false);
   std::promise<bool> promisedFinished;
   auto futureResult = promisedFinished.get_future();
   std::thread([](std::promise<bool>& finished,std::atomic<bool>& shutdown) {
      Headcrab listener("ipc:///tmp/licenseServer.ipc");
      listener.ComeToLife();
      protoMsg::LicenseRequest request;
      protoMsg::LicenseReply reply;
      while (!shutdown) {
         std::string requestData;
         if (listener.GetHitWait(requestData,1000) ) {
            if (request.ParseFromString(requestData) && request.has_type() && 
                    (request.type() == protoMsg::LicenseRequest_RequestType_AGGREGATE)) {
              listener.SendSplatter(reply.SerializeAsString());
            } else {
               listener.SendSplatter("");
            }
         }
      }
      finished.set_value(true);
   }, std::ref(promisedFinished), std::ref(shutdown)).detach();

   EXPECT_TRUE(client.Initialize());
   protoMsg::RawLicense aLicense;
   EXPECT_FALSE(client.GetAggregateLicense(aLicense));
   shutdown = true;
   EXPECT_TRUE(futureResult.wait_for(std::chrono::milliseconds(1000 + 2)) != std::future_status::timeout);
}

TEST_F(LicenseClientTest, ServerThatReturnsAnAggregateLicense) {
   LicenseClient client;
   std::atomic<bool> shutdown(false);
   std::promise<bool> promisedFinished;
   auto futureResult = promisedFinished.get_future();
   std::thread([](std::promise<bool>& finished,std::atomic<bool>& shutdown) {
      Headcrab listener("ipc:///tmp/licenseServer.ipc");
      listener.ComeToLife();
      protoMsg::LicenseRequest request;
      protoMsg::LicenseReply reply;
      protoMsg::RawLicense license; 
      license.set_bandwidthinmbps(1234);
      license.set_pcapfiles(1);
      license.set_pcapstorageinmb(2);
      license.set_maxindexesindays(3);
      license.set_expiretime(4);
      license.set_expiredurationindays(5);
      license.set_installationtime(6);
      reply.add_encodedlicenses(license.SerializeAsString());
      while (!shutdown) {
         std::string requestData;
         if (listener.GetHitWait(requestData,1000) ) {
            if (request.ParseFromString(requestData) && request.has_type() && 
                    (request.type() == protoMsg::LicenseRequest_RequestType_AGGREGATE)) {
              listener.SendSplatter(reply.SerializeAsString());
            } else {
               listener.SendSplatter("");
            }
         }
      }
      finished.set_value(true);
   }, std::ref(promisedFinished), std::ref(shutdown)).detach();

   EXPECT_TRUE(client.Initialize());
   protoMsg::RawLicense aLicense;
   EXPECT_TRUE(client.GetAggregateLicense(aLicense));
   shutdown = true;
   EXPECT_TRUE(futureResult.wait_for(std::chrono::milliseconds(1000 + 2)) != std::future_status::timeout);
}