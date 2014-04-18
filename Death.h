#pragma once

#include <string>
#include <g2log.hpp>

/**
 * By calling @ref UseDeathHandler all CHECK, LOG(FATAL) or fatal signals will be caught by g2log
 *  but will instead of exiting the test/application call @ref Death::Received
 *
 * The reason for using this instead of Google's gtest DEATH framework is that the DEATH framework
 *  will do popen/fork which clashes with our use of the MotherForker.
 */
struct Death {
   static bool mReceived;
   static std::string mMessage;

   static void Received(g2::internal::FatalMessage death);
   static bool WasKilled();

   static void SetupExitHandler();
   static void ClearExits();
};

/** Makes sure that any Death tests will be cleaned up at test exit
 * Please add this to your test
 *
 * TEST_F(Something, Something) {
 * RaiiDeathCleanup cleanup;
 * ... // test logic
 * }
 */
struct RaiiDeathCleanup {

   ~RaiiDeathCleanup() {
      Death::ClearExits();
   }
};

