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
#include <string>

struct MockNtpConfigCommand : public NtpConfigCommand {

   MockNtpConfigCommand(const protoMsg::CommandRequest& request, ProcessManager& processManager)
   : NtpConfigCommand(request, processManager), throwCounter(0), willFakeThrow(false) {
   }

   ~MockNtpConfigCommand() {
   }

   LR_VIRTUAL void TriggerNtpdChange() LR_OVERRIDE {
      return NtpConfigCommand::TriggerNtpdChange();
   }

   protoMsg::ProcessReply IsServerAlive(const std::string& server) LR_OVERRIDE {
      if (oneServerAlive.empty()) {
         return NtpConfigCommand::IsServerAlive(server);
      }
      protoMsg::ProcessReply reply = NtpConfigCommand::IsServerAlive(server);
      if (oneServerAlive == server) {
         reply.set_success(true);
         reply.set_returncode(0);
      } else {
         reply.set_success(true); //the ping command always give true even if it does not reply correctly
         reply.set_returncode(-1);
      }

      return reply;
   }

   protoMsg::ProcessReply ForceTimeSync(const std::string & server) LR_OVERRIDE {
      if (oneServerAlive.empty()) {
         return NtpConfigCommand::ForceTimeSync(server);
      }

      protoMsg::ProcessReply reply = NtpConfigCommand::ForceTimeSync(server);
      if (oneServerAlive == server) {
         reply.set_success(true);
         reply.set_returncode(0);
      } else {
         reply.set_success(false);
         reply.set_returncode(-1);
      }

      return reply;
   }

   void ReportUnexpectedResult(protoMsg::ProcessReply& result,
           const std::string& command, const std::string& commandArgs,
           const std::string& report) LR_OVERRIDE {
      if (false == willFakeThrow) {
         NtpConfigCommand::ReportUnexpectedResult(result, command, commandArgs, report);
         return;
      }

      try {
         NtpConfigCommand::ReportUnexpectedResult(result, command, commandArgs, report);
      } catch (...) {
         throwCounter++;
         return;
      }
   }


   std::string oneServerAlive;
   size_t throwCounter;
   bool willFakeThrow;
};
