#include "ClientPipe.h"
#include "ServerPipe.h"

class MockClientPipe : public ClientPipe {
public:

   MockClientPipe(std::string programName) : ClientPipe(programName), mFakeChildFinished(false) {
   }

   LR_VIRTUAL int GetReplyTimeout() {
      return 1;
   }

   LR_VIRTUAL int GetSendReplyTimeout() {
      return 1;
   }

   LR_VIRTUAL int GetRunningCommandTimeout() {
      return 1;
   }

   LR_VIRTUAL bool GetStringFromPipeWithWait(std::string& resultString, const int waitInSeconds) {
      return ClientPipe::GetStringFromPipeWithWait(resultString, waitInSeconds);
   }

   LR_VIRTUAL bool GetStringFromUniquePipeWithWait(const CommandId& id, std::string& resultString, const int waitInSeconds) {
      return ClientPipe::GetStringFromUniquePipeWithWait(id, resultString, waitInSeconds);
   }

   LR_VIRTUAL bool MakeUniqueFifo(const CommandId& id) {
      return ClientPipe::MakeUniqueFifo(id);
   }

   LR_VIRTUAL int ReadOpenUniqueFifo(const CommandId& id) {
      return ClientPipe::ReadOpenUniqueFifo(id);
   }

   LR_VIRTUAL int WriteOpenUniqueFifo(const CommandId& id) {
      return ClientPipe::WriteOpenUniqueFifo(id);
   }

   LR_VIRTUAL std::string ConstructUniquePipeName(const CommandId& id) {
      return ClientPipe::ConstructUniquePipeName(id);
   }

   LR_VIRTUAL std::string GetTargetReceivePipe() {
      return ClientPipe::GetTargetReceivePipe();
   }

   LR_VIRTUAL std::string GetTargetSendPipe() {
      return ClientPipe::GetTargetSendPipe();
   }

   LR_VIRTUAL std::string GetUUID() {
      return ClientPipe::GetUUID();
   }

   LR_VIRTUAL int RunExecVE(const char* command, char** arguments, char** environment) {
      return ClientPipe::RunExecVE(command, arguments, environment);
   }

   LR_VIRTUAL bool IsChildLiving(pid_t child, int& returnCode) {
      if (mFakeChildFinished) {
         return false;
      }
      return ClientPipe::IsChildLiving(child, returnCode);
   }

   LR_VIRTUAL std::string ReadFromReadyPipe(int pipe) {
      return ClientPipe::ReadFromReadyPipe(pipe);
   }

   LR_VIRTUAL bool WaitForDataOnPipe(int pipe, const int waitInSeconds) {
      return ClientPipe::WaitForDataOnPipe(pipe, waitInSeconds);
   }
   
   LR_VIRTUAL bool DestroyUniqueFifo(const CommandId& id) {
      return ClientPipe::DestroyUniqueFifo(id);
   }

   void InsertDummyCommand(const CommandId& id, const CommandState& stat) {
      mRunningCommands[id] = stat;
      mRunningCommands[id].id = id;
   }
   bool mFakeChildFinished;
};

class MockServerPipe : public ServerPipe {
public:

   MockServerPipe(std::string programName) : ServerPipe(programName), mFakeChildFinished(false) {
   }

   LR_VIRTUAL int GetReplyTimeout() {
      return 1;
   }

   LR_VIRTUAL int GetSendReplyTimeout() {
      return 1;
   }

   LR_VIRTUAL int GetRunningCommandTimeout() {
      return 1;
   }

   LR_VIRTUAL bool GetStringFromPipeWithWait(std::string& resultString, const int waitInSeconds) {
      return ServerPipe::GetStringFromPipeWithWait(resultString, waitInSeconds);
   }

   LR_VIRTUAL bool GetStringFromUniquePipeWithWait(const CommandId& id, std::string& resultString, const int waitInSeconds) {
      return ServerPipe::GetStringFromUniquePipeWithWait(id, resultString, waitInSeconds);
   }

   LR_VIRTUAL bool MakeUniqueFifo(const CommandId& id) {
      return ServerPipe::MakeUniqueFifo(id);
   }

   LR_VIRTUAL int ReadOpenUniqueFifo(const CommandId& id) {
      return ServerPipe::ReadOpenUniqueFifo(id);
   }

   LR_VIRTUAL int WriteOpenUniqueFifo(const CommandId& id) {
      return ServerPipe::WriteOpenUniqueFifo(id);
   }

   LR_VIRTUAL std::string ConstructUniquePipeName(const CommandId& id) {
      return ServerPipe::ConstructUniquePipeName(id);
   }

   LR_VIRTUAL std::string GetTargetReceivePipe() {
      return ServerPipe::GetTargetReceivePipe();
   }

   LR_VIRTUAL std::string GetTargetSendPipe() {
      return ServerPipe::GetTargetSendPipe();
   }

   LR_VIRTUAL std::string GetUUID() {
      return ServerPipe::GetUUID();
   }

   LR_VIRTUAL int RunExecVE(const char* command, char** arguments, char** environment) {
      return ServerPipe::RunExecVE(command, arguments, environment);
   }

   LR_VIRTUAL bool IsChildLiving(pid_t child, int& returnCode) {
      if (mFakeChildFinished) {
         return false;
      }
      return ServerPipe::IsChildLiving(child, returnCode);
   }

   LR_VIRTUAL std::string ReadFromReadyPipe(int pipe) {
      return ServerPipe::ReadFromReadyPipe(pipe);
   }

   LR_VIRTUAL bool WaitForDataOnPipe(int pipe, const int waitInSeconds) {
      return ServerPipe::WaitForDataOnPipe(pipe, waitInSeconds);
   }

   void InsertDummyCommand(const CommandId& id, const CommandState& stat) {
      mRunningCommands[id] = stat;
      mRunningCommands[id].id = id;
   }
   bool mFakeChildFinished;
};