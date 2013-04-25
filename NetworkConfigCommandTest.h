/* 
 * File:   NetworkConfigCommandTest.h
 * Author: jgress
 *
 * Created on April 25, 2013
 */

#pragma once

#include "NetworkConfigCommand.h"
#include "Command.h"

class NetworkConfigCommandTest : public NetworkConfigCommand {
public:

   NetworkConfigCommandTest(const protoMsg::CommandRequest& request, ProcessManager* processManager) : NetworkConfigCommand(request, processManager) {
   }
   

   virtual ~NetworkConfigCommandTest() {
   }


};
