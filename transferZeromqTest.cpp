#include "transferZeromqTest.h"
#include "g2log.hpp"
#include <unordered_set>
#include <unordered_map>
#include "ElasticSearch.h"
#include <thread>
#include "jansson.h"

void MesageSendThread(int numberOfMessages, int threadNumber, const std::string& binding,
        const std::string& baseMessage, zctx_t* context) {
   void* socket = zsocket_new(context, ZMQ_DEALER);
   std::string resultString;
   if (zsocket_connect(socket, binding.c_str()) == 0) {
      std::stringstream testStream;
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
            zmsg_t* message = zmsg_new();
            zmsg_addstr(message, testStream.str().c_str());
            zmsg_send(&message, socket);
            sent++;
            ids.insert(i);
            testStream.str("");
         }
         for (int i = 0; i < 1000 && recieved < numberOfMessages; i++) {
            zmsg_t* message = zmsg_recv(socket);
            resultString = zmsg_popstr(message);

            ASSERT_TRUE(resultString.find("\"ok\":true") != std::string::npos);
            ASSERT_TRUE(resultString.find(type) != std::string::npos);
            size_t idLocation = resultString.find("id\":\"") + 5;
            ASSERT_TRUE(idLocation != std::string::npos);
            int id = atoi(resultString.substr(idLocation, resultString.find("\"", idLocation)).c_str());
            //std::cout << resultString << std::endl;
            //std::cout << id << resultString.substr(idLocation,resultString.find("\"",idLocation)) << std::endl;
            ids.erase(id);
            zmsg_destroy(&message);
            recieved++;
         }
         ASSERT_TRUE(ids.empty());

      }

   } else {
      int err = zmq_errno();
      std::string error(zmq_strerror(err));
      std::cout << "Could not connect to " << binding << ":" << error << std::endl;
      zctx_destroy(&context);
      ASSERT_TRUE(false);
   }
}

void JSonPrettyPrint(json_t* Object, int indent) {
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

TEST_F(transferZeromqTest, FileSystemInfoTest) {
   RESTBuilder builder;
   BoomStick transport("tcp://127.0.0.1:9700");
   RESTSender sender(transport);
   
   ASSERT_TRUE(transport.Initialize());

   std::string resultString;
   sender.Send(builder.GetDiskInfo(), resultString);

   json_error_t error;
   std::string jsonReturnString = resultString.substr(resultString.find("{"));
   json_t* decodedMessage = json_loads(jsonReturnString.c_str(), 0, &error);
   ASSERT_TRUE(decodedMessage != NULL);
   //JSonPrettyPrint(decodedMessage, 0);

   DiskInformation diskInfo = sender.GetDiskInformation(resultString);
}

TEST_F(transferZeromqTest, GetListOfIndexes) {
   zctx_t* context = zctx_new();
   void* socket = zsocket_new(context, ZMQ_DEALER);
   if (zsocket_connect(socket, "tcp://127.0.0.1:9700") == 0) {
      zmsg_t* message = zmsg_new();
      zmsg_addstr(message, "GET|/_status/");
      zmsg_send(&message, socket);
      message = zmsg_recv(socket);
      std::string resultString = zmsg_popstr(message);
      //std::cout << resultString << std::endl;
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
      for (auto name : indexes) {
         std::cout << name << std::endl;
      }
   }
}

TEST_F(transferZeromqTest, singleIteration) {
   zctx_t* context = zctx_new();
   int iterations = 1;

   MesageSendThread(iterations, 1, "tcp://127.0.0.1:9700", "this is a single thread", context);

}

TEST_F(transferZeromqTest, multipleThreads) {
   zctx_t* context = zctx_new();
   int iterations = 10000;
   int threadcount = 10;
   SetExpectedTime(iterations*threadcount, 60, 1, 4000L);
   StartTimedSection();
   std::unordered_map<int, std::thread*> threads;
   for (int i = 0; i < threadcount; i++) {
      threads[i] = new std::thread(MesageSendThread, iterations, i, "tcp://127.0.0.1:9700",
              "this is a multiple-thread", context);
   }
   for (int i = 0; i < threadcount; i++) {
      threads[i]->join();
      delete threads[i];
   }
   EndTimedSection();
}

TEST_F(transferZeromqTest, SingleThreadSpeedTest) {
   zctx_t* context = zctx_new();
   int iterations = 50000;
   SetExpectedTime(iterations, 60, 1, 4000L);
   StartTimedSection();
   MesageSendThread(iterations, 1, "tcp://127.0.0.1:9700", "this is a single thread", context);
   EndTimedSection();
}

