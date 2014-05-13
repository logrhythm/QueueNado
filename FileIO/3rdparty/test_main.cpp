#include <stdio.h>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <stdlib.h>
#include <time.h>
#include <thread>
#include <google/protobuf/stubs/common.h>


int main(int argc, char *argv[])
{

   // Start the g2log logger running
   srand(time(NULL));
   std::stringstream fileName;
   fileName << "UnitTest" << geteuid();
   ::testing::InitGoogleTest(&argc, argv);
   int retval = RUN_ALL_TESTS();
   std::this_thread::sleep_for(std::chrono::seconds(5));
   return retval;
}
