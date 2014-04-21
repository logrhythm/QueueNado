#include <stdio.h>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <g2logworker.hpp>
#include <g2log.hpp>
#include <stdlib.h>
#include <time.h>
#include <thread>
#include <google/protobuf/stubs/common.h>
#include "BaseConfMsg.pb.h"
#include "include/global.h"

std::string gProgramName;
std::unique_ptr<g2LogWorker> g2logger(nullptr);

int main(int argc, char *argv[])
{
   gProgramName = argv[0];
   global::SetProgramName(gProgramName);
   std::cout << gProgramName << std::endl;
   GOOGLE_PROTOBUF_VERIFY_VERSION;

   // Start the g2log logger running
   srand(time(NULL));
   std::stringstream fileName;
   fileName << "UnitTest" << geteuid();
   g2logger.reset(new g2LogWorker(fileName.str(), "/tmp/" ));
   g2::initializeLogging(g2logger.get());

   ::testing::InitGoogleTest(&argc, argv);
   int retval = RUN_ALL_TESTS();
   google::protobuf::ShutdownProtobufLibrary();
   g2::shutDownLogging();
   std::this_thread::sleep_for(std::chrono::seconds(5));
   return retval;
}
