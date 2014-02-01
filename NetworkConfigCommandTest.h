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
   using NetworkConfigCommand::InterfaceDown;
   using NetworkConfigCommand::InterfaceUp;
   using NetworkConfigCommand::IfcfgInterfaceAllowed;
   using NetworkConfigCommand::IfcfgFileExists;
   using NetworkConfigCommand::ResetIfcfgFile;
   using NetworkConfigCommand::BackupIfcfgFile;
   using NetworkConfigCommand::RestoreIfcfgFile;
   using NetworkConfigCommand::AddBootProto;
   using NetworkConfigCommand::AddIpAddr;
   using NetworkConfigCommand::AddNetmask;
   using NetworkConfigCommand::AddGateway;
   using NetworkConfigCommand::AddDnsServers;
   using NetworkConfigCommand::AddDns1;
   using NetworkConfigCommand::AddDns2;
   using NetworkConfigCommand::AddDomain;
   using NetworkConfigCommand::AddOnBoot;
   using NetworkConfigCommand::AddNmControlled;
   using NetworkConfigCommand::AddPeerDns;

   NetworkConfigCommandTest(const protoMsg::CommandRequest& request,
           ProcessManager& processManager) : mStatSuccess(true), NetworkConfigCommand(request, processManager) {
   }
   

   virtual ~NetworkConfigCommandTest() {}

   void setStatFail() {
      mStatSuccess = false;
   }
   void ManglemIfcfgFile() {
      mIfcfgFile = "/thisPath/Does/Not/exist";
   }
   std::string GetIfcfgFile() {
      return mIfcfgFile;
   }
   int stat(const char *path, struct stat *buf) {
      if (mStatSuccess) {
         return 0;
      } else {
         return -1;
      }
   }

private:
   bool mStatSuccess;
};
