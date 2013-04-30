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
   using NetworkConfigCommand::IfcfgFileExists;
   using NetworkConfigCommand::ResetIfcfgFile;
   using NetworkConfigCommand::AddBootProto;
   using NetworkConfigCommand::AddIpAddr;
   using NetworkConfigCommand::AddNetMask;
   using NetworkConfigCommand::AddGateway;
   using NetworkConfigCommand::AddDnsServers;
   using NetworkConfigCommand::AddDns1;
   using NetworkConfigCommand::AddDns2;
   using NetworkConfigCommand::AddDomain;

   NetworkConfigCommandTest(const protoMsg::CommandRequest& request, ProcessManager* processManager) : NetworkConfigCommand(request, processManager) {
   }
   

   virtual ~NetworkConfigCommandTest() {
   }


};
