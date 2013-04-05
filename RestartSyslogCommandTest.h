/* 
 * File:   UpgradeCommandTest.h
 * Author: vrecan
 *
 * Created on April 3, 2013, 10:51 AM
 */

#pragma once

#include "RestartSyslogCommand.h"
#include "Command.h"

class RestartSyslogCommandTest : public RestartSyslogCommand {
public:
   using RestartSyslogCommand::Restart;

   RestartSyslogCommandTest(const protoMsg::CommandRequest& request, ProcessManager* processManager) : RestartSyslogCommand(request, processManager) {
   }

   virtual ~RestartSyslogCommandTest() {
   }


};