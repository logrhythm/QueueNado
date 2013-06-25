#include "transferZeromqTest.h"
#include "g2logworker.hpp"
#include "g2log.hpp"
#include <unordered_set>
#include <unordered_map>
#include <thread>

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