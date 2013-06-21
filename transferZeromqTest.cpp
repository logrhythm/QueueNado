#include "transferZeromqTest.h"
#include "g2logworker.hpp"
#include "g2log.hpp"
void MesageSendThread(int numberOfMessages, const std::string& binding, 
        const std::string& baseMessage, zctx_t* context) {
   void* socket = zsocket_new(context,ZMQ_DEALER);
   std::string resultString;
   if (zsocket_connect(socket,binding.c_str()) == 0) {
      std::stringstream testStream;
      for (int i = 0 ; i < numberOfMessages; i++) {
         testStream << "POST|/twitter/tweet/" << i << "|{\"user\" : \"kimchy\", \"post_date\" : \"2009-11-15T" 
                 << i/(60*60)%24 << ":" << (i/60)%60 << ":" << i % 60 << "\", \"message\" : \"" << baseMessage << i
                 << "\"}";
         zmsg_t* message = zmsg_new();
         zmsg_addstr(message,testStream.str().c_str());
         zmsg_send(&message,socket);
         message = zmsg_recv(socket);
         testStream.str("");
         resultString = zmsg_popstr(message);

         ASSERT_TRUE(resultString.find("\"ok\":true") != std::string::npos);

         zmsg_destroy(&message);
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

   MesageSendThread(iterations, "tcp://127.0.0.1:9700", "this is a single thread", context);

}
TEST_F(transferZeromqTest, SingleThreadSpeedTest) {
   zctx_t* context = zctx_new();
   int iterations = 50000;
   SetExpectedTime(iterations, 60, 5000, 500000L);
   StartTimedSection();
   MesageSendThread(iterations, "tcp://127.0.0.1:9700", "this is a single thread", context);
   EndTimedSection();
}