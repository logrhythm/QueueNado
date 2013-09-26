#include "transferZeromqTest.h"
#include "g2log.hpp"
#include <unordered_set>
#include <unordered_map>
#include "ElasticSearch.h"
#include "MockBoomStick.h"
#include "MockRESTParser.h"
#include <thread>
#include <memory>
#include "jansson.h"

namespace {

   void MessageSend(int numberOfMessages, int threadNumber, const std::string& binding,
           const std::string& baseMessage, zctx_t* context, size_t& dataSent, size_t& recordsSent) {
      dataSent = 0;
      recordsSent = 0;
      void* socket = zsocket_new(context, ZMQ_DEALER);
      std::string resultString;
      if (zsocket_connect(socket, binding.c_str()) == 0) {
         std::ostringstream testStream;
         testStream << "tweet" << threadNumber;
         std::string type = testStream.str();
         testStream.str("");
         std::unordered_set<int> ids;
         int sent = 0;
         int recieved = 0;
         for (int j = 0; j < numberOfMessages / 1000; j++) {
            for (int i = 0; i < 1000 && sent < numberOfMessages; i++) {

               testStream << "POST|/twitter/" << type << "/" << i << "|{\"user\" : \"kimchy\", \"post_date\" : \"2009-11-15T"
                       << i / (60 * 60) % 24 << ":" << (i / 60) % 60 << ":" << i % 60 << "\", \"message\" : \"" << baseMessage << i
                       << "\"}";
               dataSent += testStream.str().size();
               zmsg_t* message = zmsg_new();
               zmsg_addstr(message, testStream.str().c_str());
               zmsg_send(&message, socket);
               sent++;
               ids.insert(i);
               testStream.str("");
            }
            for (int i = 0; i < 1000 && recieved < numberOfMessages; i++) {
               zmsg_t* message = zmsg_recv(socket);
               char* msgChar = zmsg_popstr(message);
               resultString = msgChar;
               free(msgChar);

               ASSERT_TRUE(resultString.find("\"ok\":true") != std::string::npos);
               ASSERT_TRUE(resultString.find(type) != std::string::npos);
               size_t idLocation = resultString.find("id\":\"") + 5;
               ASSERT_TRUE(idLocation != std::string::npos);
               size_t id = std::stoi(resultString.substr(idLocation, resultString.find("\"", idLocation)));
               //std::cout << resultString << std::endl;
               //std::cout << id << resultString.substr(idLocation,resultString.find("\"",idLocation)) << std::endl;
               ids.erase(id);
               zmsg_destroy(&message);
               recieved++;
               recordsSent++;
            }
            ASSERT_TRUE(ids.empty());

         }

      } else {
         int err = zmq_errno();
         std::string error(zmq_strerror(err));
         std::cout << "Could not connect to " << binding << ":" << error << std::endl;
         zctx_destroy(&context);
         FAIL();
      }
   }

   void MesageSendThread(int numberOfMessages, int threadNumber, const std::string& binding,
           const std::string& baseMessage, zctx_t* context) {
      size_t dataSent;
      size_t recordsSent;
      MessageSend(numberOfMessages, threadNumber, binding, baseMessage, context, dataSent, recordsSent);
   }

   void JSonPrettyPrint(json_t* Object, unsigned int indent) {
      const char* key;
      json_t* value;

      json_object_foreach(Object, key, value) {
         for (int i = 0; i < indent; i++) {
            std::cout << "   ";
         }
         std::cout << key << ":";
         if (json_is_object(value)) {
            std::cout << std::endl;
            JSonPrettyPrint(value, indent + 1);
         } else if (json_is_string(value)) {
            std::cout << json_string_value(value);
         } else if (json_is_integer(value)) {
            std::cout << json_integer_value(value);
         } else if (json_is_array(value)) {
            for (int i = 0; i < json_array_size(value); i++) {
               std::cout << std::endl;
               JSonPrettyPrint(json_array_get(value, i), indent + 1);
            }
         }
         std::cout << std::endl;
      }
   }

   DiskInformation GetDiskInfo() {
      RESTBuilder builder;
      BoomStick transport("tcp://127.0.0.1:9700");
      MockRESTParser sender(transport);

      transport.Initialize();
      std::string resultString;
      sender.Send(builder.GetDiskInfo(), resultString);

      return sender.GetDiskInformation(resultString);
   }

   void PrintDiskInfo() {
      BoomStick transport("tcp://127.0.0.1:9700");
      RESTParser sender(transport);
      auto diskInfo = GetDiskInfo();
      std::vector<std::string> clusterNames = sender.GetAllClusterNamesFromDiskInfo(diskInfo);
      for (const auto& hostName : clusterNames) {
         std::cout << hostName << " : " << std::endl;
         std::cout << sender.GetDiskReads(hostName, diskInfo) << " reads" << std::endl;
         std::cout << sender.GetDiskWrites(hostName, diskInfo) << " writes" << std::endl;
      }
   }
}

TEST_F(transferZeromqTest, FileSystemInfoTest) {
   PrintDiskInfo();
}

TEST_F(transferZeromqTest, GetListOfIndexes) {
   zctx_t* context = zctx_new();
   void* socket = zsocket_new(context, ZMQ_DEALER);
   if (zsocket_connect(socket, "tcp://127.0.0.1:9700") == 0) {
      zmsg_t* message = zmsg_new();
      zmsg_addstr(message, "GET|/_status/");
      zmsg_send(&message, socket);
      message = zmsg_recv(socket);
      char* msgChar = zmsg_popstr(message);
      std::string resultString = msgChar;
      free(msgChar);
      std::cout << resultString << std::endl;
      zmsg_destroy(&message);
      json_error_t error;
      std::string jsonReturnString = resultString.substr(resultString.find("{"));
      json_t* decodedMessage = json_loads(jsonReturnString.c_str(), 0, &error);
      ASSERT_TRUE(decodedMessage != NULL);
      //JSonPrettyPrint(decodedMessage, 0);
      const char* key;
      json_t* value;
      std::unordered_set<std::string> indexes;

      json_object_foreach(decodedMessage, key, value) {
         std::string keyString(key);
         if (keyString == "indices") {
            const char* indexName;
            json_t* indexInfo;

            json_object_foreach(value, indexName, indexInfo) {
               indexes.insert(indexName);
            }
         }
      }
      EXPECT_FALSE(indexes.empty());
      for (const auto& name : indexes) {
         std::cout << name << std::endl;
      }
   }
}

TEST_F(transferZeromqTest, singleIteration) {
   zctx_t* context = zctx_new();
   int iterations = 1;
   size_t dataSent;
   size_t recordsSent;
   MessageSend(iterations, 1, "tcp://127.0.0.1:9700", "this is a single thread", context, dataSent, recordsSent);

}

TEST_F(transferZeromqTest, multipleThreads) {
   zctx_t* context = zctx_new();
   int iterations = 10000;
   int threadcount = 10;
   SetExpectedTime(iterations*threadcount, 60, 1, 4000L);
   StartTimedSection();
   std::unordered_map<int, std::unique_ptr<std::thread> > threads;
   for (int i = 0; i < threadcount; i++) {
      size_t dataSent;
      size_t recordsSent;
      threads[i].reset(new std::thread(MesageSendThread, iterations, i, "tcp://127.0.0.1:9700",
              "this is a multiple-thread", context));
   }
   for (int i = 0; i < threadcount; i++) {
      threads[i]->join();
   }
   EndTimedSection();
}
#ifdef LR_DEBUG
TEST_F(transferZeromqTest, SingleThreadSpeedTest) {

   auto diskInfoPre = GetDiskInfo();
   zctx_t* context = zctx_new();
   int iterations = 50000;
   size_t dataSent;
   size_t recordsSent;
   std::string bigMessage;
   for (int i = 0; i < 10; i++) {
      bigMessage += "1234567890";
   }
   SetExpectedTime(iterations, bigMessage.size(), 10, 4000L);
   StartTimedSection();

   MessageSend(iterations, 1, "tcp://127.0.0.1:9700", bigMessage, context, dataSent, recordsSent);
   EndTimedSection();
   std::cout << recordsSent << " total records sent " << std::endl;
   std::cout << dataSent << " total bytes sent " << std::endl;
   auto diskInfoPost = GetDiskInfo();
   MockBoomStick transport("tcp://127.0.0.1:9700");
   RESTParser sender(transport);
   std::vector<std::string> clusterNames = sender.GetAllClusterNamesFromDiskInfo(diskInfoPre);
   for (const auto& hostName : clusterNames) {
      std::cout << hostName << " : " << std::endl;
      std::cout << (sender.GetDiskReads(hostName, diskInfoPost) - sender.GetDiskReads(hostName, diskInfoPre)) << " reads" << std::endl;
      std::cout << (sender.GetDiskWrites(hostName, diskInfoPost) - sender.GetDiskWrites(hostName, diskInfoPre)) << " writes" << std::endl;
      std::cout << (sender.GetDiskReadSizeInBytes(hostName, diskInfoPost) - sender.GetDiskReadSizeInBytes(hostName, diskInfoPre)) << " bytes read" << std::endl;
      std::cout << (sender.GetDiskWriteSizeInBytes(hostName, diskInfoPost) - sender.GetDiskWriteSizeInBytes(hostName, diskInfoPre)) << " bytes written" << std::endl;

   }
   iterations = 5000;
   diskInfoPre = GetDiskInfo();
   SetExpectedTime(iterations, bigMessage.size(), 10, 4000L);
   StartTimedSection();

   MessageSend(iterations, 1, "tcp://127.0.0.1:9700", bigMessage, context, dataSent, recordsSent);
   EndTimedSection();
   std::cout << recordsSent << " total records sent " << std::endl;
   std::cout << dataSent << " total bytes sent " << std::endl;
   diskInfoPost = GetDiskInfo();
   clusterNames = sender.GetAllClusterNamesFromDiskInfo(diskInfoPre);
   for (const auto& hostName : clusterNames) {
      std::cout << hostName << " : " << std::endl;
      std::cout << (sender.GetDiskReads(hostName, diskInfoPost) - sender.GetDiskReads(hostName, diskInfoPre)) << " reads" << std::endl;
      std::cout << (sender.GetDiskWrites(hostName, diskInfoPost) - sender.GetDiskWrites(hostName, diskInfoPre)) << " writes" << std::endl;
      std::cout << (sender.GetDiskReadSizeInBytes(hostName, diskInfoPost) - sender.GetDiskReadSizeInBytes(hostName, diskInfoPre)) << " bytes read" << std::endl;
      std::cout << (sender.GetDiskWriteSizeInBytes(hostName, diskInfoPost) - sender.GetDiskWriteSizeInBytes(hostName, diskInfoPre)) << " bytes written" << std::endl;

   }
   
}

TEST_F(transferZeromqTest, SingleThreadSpeedTestMedium) {

   auto diskInfoPre = GetDiskInfo();
   zctx_t* context = zctx_new();
   int iterations = 50000;
   size_t dataSent;
   size_t recordsSent;
   std::string bigMessage;
   for (int i = 0; i < 100; i++) {
      bigMessage += "1234567890";
   }
   SetExpectedTime(iterations, bigMessage.size(), 10, 4000L);
   StartTimedSection();

   MessageSend(iterations, 1, "tcp://127.0.0.1:9700", bigMessage, context, dataSent, recordsSent);
   EndTimedSection();
   std::cout << recordsSent << " total records sent " << std::endl;
   std::cout << dataSent << " total bytes sent " << std::endl;
   auto diskInfoPost = GetDiskInfo();
   MockBoomStick transport("tcp://127.0.0.1:9700");
   RESTParser sender(transport);
   std::vector<std::string> clusterNames = sender.GetAllClusterNamesFromDiskInfo(diskInfoPre);
   for (const auto& hostName : clusterNames) {
      std::cout << hostName << " : " << std::endl;
      std::cout << (sender.GetDiskReads(hostName, diskInfoPost) - sender.GetDiskReads(hostName, diskInfoPre)) << " reads" << std::endl;
      std::cout << (sender.GetDiskWrites(hostName, diskInfoPost) - sender.GetDiskWrites(hostName, diskInfoPre)) << " writes" << std::endl;
      std::cout << (sender.GetDiskReadSizeInBytes(hostName, diskInfoPost) - sender.GetDiskReadSizeInBytes(hostName, diskInfoPre)) << " bytes read" << std::endl;
      std::cout << (sender.GetDiskWriteSizeInBytes(hostName, diskInfoPost) - sender.GetDiskWriteSizeInBytes(hostName, diskInfoPre)) << " bytes written" << std::endl;

   }
   iterations = 5000;
   diskInfoPre = GetDiskInfo();
   SetExpectedTime(iterations, bigMessage.size(), 10, 4000L);
   StartTimedSection();

   MessageSend(iterations, 1, "tcp://127.0.0.1:9700", bigMessage, context, dataSent, recordsSent);
   EndTimedSection();
   std::cout << recordsSent << " total records sent " << std::endl;
   std::cout << dataSent << " total bytes sent " << std::endl;
   diskInfoPost = GetDiskInfo();
   clusterNames = sender.GetAllClusterNamesFromDiskInfo(diskInfoPre);
   for (const auto& hostName : clusterNames) {
      std::cout << hostName << " : " << std::endl;
      std::cout << (sender.GetDiskReads(hostName, diskInfoPost) - sender.GetDiskReads(hostName, diskInfoPre)) << " reads" << std::endl;
      std::cout << (sender.GetDiskWrites(hostName, diskInfoPost) - sender.GetDiskWrites(hostName, diskInfoPre)) << " writes" << std::endl;
      std::cout << (sender.GetDiskReadSizeInBytes(hostName, diskInfoPost) - sender.GetDiskReadSizeInBytes(hostName, diskInfoPre)) << " bytes read" << std::endl;
      std::cout << (sender.GetDiskWriteSizeInBytes(hostName, diskInfoPost) - sender.GetDiskWriteSizeInBytes(hostName, diskInfoPre)) << " bytes written" << std::endl;

   }
  

}

TEST_F(transferZeromqTest, SingleThreadSpeedTestBigData) {

   auto diskInfoPre = GetDiskInfo();
   zctx_t* context = zctx_new();
   int iterations = 50000;

   size_t dataSent;
   size_t recordsSent;
   std::string bigMessage;
   for (int i = 0; i < 1024; i++) {
      bigMessage += "1234567890";
   }
   SetExpectedTime(iterations, bigMessage.size(), 100, 4000L);
   StartTimedSection();
   MessageSend(iterations, 1, "tcp://127.0.0.1:9700", bigMessage, context, dataSent, recordsSent);
   EndTimedSection();
   std::cout << recordsSent << " total records sent " << std::endl;
   std::cout << dataSent << " total bytes sent " << std::endl;
   auto diskInfoPost = GetDiskInfo();
   MockBoomStick transport("tcp://127.0.0.1:9700");
   RESTParser sender(transport);
   std::vector<std::string> clusterNames = sender.GetAllClusterNamesFromDiskInfo(diskInfoPre);
   for (const auto& hostName : clusterNames) {
      std::cout << hostName << " : " << std::endl;
      std::cout << (sender.GetDiskReads(hostName, diskInfoPost) - sender.GetDiskReads(hostName, diskInfoPre)) << " reads" << std::endl;
      std::cout << (sender.GetDiskWrites(hostName, diskInfoPost) - sender.GetDiskWrites(hostName, diskInfoPre)) << " writes" << std::endl;
      std::cout << (sender.GetDiskReadSizeInBytes(hostName, diskInfoPost) - sender.GetDiskReadSizeInBytes(hostName, diskInfoPre)) << " bytes read" << std::endl;
      std::cout << (sender.GetDiskWriteSizeInBytes(hostName, diskInfoPost) - sender.GetDiskWriteSizeInBytes(hostName, diskInfoPre)) << " bytes written" << std::endl;

   }
   iterations = 5000;
   diskInfoPre = GetDiskInfo();
   SetExpectedTime(iterations, bigMessage.size(), 100, 4000L);
   StartTimedSection();
   MessageSend(iterations, 1, "tcp://127.0.0.1:9700", bigMessage, context, dataSent, recordsSent);
   EndTimedSection();
   std::cout << recordsSent << " total records sent " << std::endl;
   std::cout << dataSent << " total bytes sent " << std::endl;
   diskInfoPost = GetDiskInfo();
   clusterNames = sender.GetAllClusterNamesFromDiskInfo(diskInfoPre);
   for (const auto& hostName : clusterNames) {
      std::cout << hostName << " : " << std::endl;
      std::cout << (sender.GetDiskReads(hostName, diskInfoPost) - sender.GetDiskReads(hostName, diskInfoPre)) << " reads" << std::endl;
      std::cout << (sender.GetDiskWrites(hostName, diskInfoPost) - sender.GetDiskWrites(hostName, diskInfoPre)) << " writes" << std::endl;
      std::cout << (sender.GetDiskReadSizeInBytes(hostName, diskInfoPost) - sender.GetDiskReadSizeInBytes(hostName, diskInfoPre)) << " bytes read" << std::endl;
      std::cout << (sender.GetDiskWriteSizeInBytes(hostName, diskInfoPost) - sender.GetDiskWriteSizeInBytes(hostName, diskInfoPre)) << " bytes written" << std::endl;

   }
   
}
#endif