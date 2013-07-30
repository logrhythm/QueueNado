/* 
 * File:   RebootCommandTest.h
 * Author: vrecan
 *
 * Created on April 3, 2013, 10:51 AM
 */

#pragma once

#include "RebootCommandTest.h"
#include "Command.h"

class RebootCommandTest : public RebootCommand {
public:
   using RebootCommand::DoTheReboot;

   RebootCommandTest(const protoMsg::CommandRequest& request, ProcessManager* processManager) : RebootCommand(request, processManager) {
   }

   virtual ~RebootCommandTest() {
   }


};