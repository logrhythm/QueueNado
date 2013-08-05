/* 
 * File:   MockNtpConfigCommand.h
 * Author: kjell
 *
 * Created on August 1, 2013, 2:09 PM
 */

#pragma once
#include "NtpConfigCommand.h"
#include "CommandReply.pb.h"
#include "ProcessManager.h"

struct MockNtpConfigCommand : public NtpConfigCommand {

   MockNtpConfigCommand(const protoMsg::CommandRequest& request, ProcessManager* processManager)
   : NtpConfigCommand(request, processManager) {
   }

   ~MockNtpConfigCommand() {
   }
};
