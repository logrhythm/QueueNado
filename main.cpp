#include <stdio.h>
#include "gtest/gtest.h"
#include "g2logworker.hpp"
#include "g2log.hpp"
int main(int argc, char *argv[])
{
   // Start the g2log logger running
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
   return retval;
}
