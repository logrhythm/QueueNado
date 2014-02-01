/* 
 * File:   MockRestartSyslogCommand.h
 * Author: kjell
 *
 * Created on October 24, 2013, 11:01 AM
 */

#pragma once
#include "RestartSyslogCommand.h"
#include "Command.h"
#include "CommandReply.pb.h"
#include "CommandRequest.pb.h"
#include "SyslogConfMsg.pb.h"
#include "ProcessManager.h"
#include "MockRestartSyslogCommand.h"
#include <string>

struct MockRestartSyslogCommand : public RestartSyslogCommand {

   MockRestartSyslogCommand(const protoMsg::CommandRequest& request, ProcessManager& processManager)
   : RestartSyslogCommand(request, processManager) {

      MockRestartSyslogCommand::mSyslogMsg.ParseFromString(request.stringargone());
   }

   MockRestartSyslogCommand(const protoMsg::CommandRequest& request) : RestartSyslogCommand(request) {

      MockRestartSyslogCommand::mSyslogMsg.ParseFromString(request.stringargone());
   }

   virtual ~MockRestartSyslogCommand() {
   }

   Conf GetFreshConf(const Conf& conf) {
      return conf;
   }

   bool ShouldGetFreshConf() {
      return false;
   }
   static std::shared_ptr<Command> Construct(const protoMsg::CommandRequest& request) {
      std::shared_ptr<Command> command(new MockRestartSyslogCommand(request));
      return command;
   }

   protoMsg::CommandReply Execute(const Conf& conf) LR_OVERRIDE {
      MockRestartSyslogCommand::mReceivedExecute = true;
      protoMsg::CommandReply reply;
      reply.set_success("true");
      reply.set_result("executed MockRestartSyslogCommand");
      return reply;
   }

   // Hack to enable these outside of the object instance
   static protoMsg::SyslogConf mSyslogMsg;
   static bool mReceivedExecute;
};

protoMsg::SyslogConf MockRestartSyslogCommand::mSyslogMsg;
bool MockRestartSyslogCommand::mReceivedExecute = false;
