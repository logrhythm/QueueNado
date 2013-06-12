#include <stdio.h>
#include "gtest/gtest.h"
#include "g2logworker.hpp"
#include "g2log.hpp"
#include <stdlib.h>
#include <time.h>
#include "BaseConfMsg.pb.h"
#include <thread>
int main(int argc, char *argv[])
{
   GOOGLE_PROTOBUF_VERIFY_VERSION;

   // Start the g2log logger running
   srand(time(NULL));
   std::stringstream fileName;
   fileName << "UnitTest" << geteuid();
   g2LogWorker g2logger(fileName.str(), "/tmp/" );
   g2::initializeLogging(&g2logger);

   ::testing::InitGoogleTest(&argc, argv);
   int retval = RUN_ALL_TESTS();
//   if (geteuid() == 0 ) {
//      seteuid(500);
//      int ignoreVal = RUN_ALL_TESTS();
//      seteuid(0);
//   }
   google::protobuf::ShutdownProtobufLibrary();
   g2::shutDownLogging();
   std::this_thread::sleep_for(std::chrono::seconds(2));
   return retval;
}
