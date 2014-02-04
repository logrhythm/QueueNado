#include "ForkerPipe.h"

class MockForkerPipe : public ForkerPipe {
public:

   MockForkerPipe(std::string programName, bool client = true) : ForkerPipe(programName, client), mFakeChildFinished(false) {
   }
   LR_VIRTUAL int GetReplyTimeout() { return 1;}
   LR_VIRTUAL int GetSendReplyTimeout(){ return 1;}
   LR_VIRTUAL int GetRunningCommandTimeout(){ return 1;}
   LR_VIRTUAL bool GetStringFromPipeWithWait(std::string& resultString,const int waitInSeconds) {
      return ForkerPipe::GetStringFromPipeWithWait(resultString,waitInSeconds);
   }
   LR_VIRTUAL bool GetStringFromUniquePipeWithWait(const CommandId& id,std::string& resultString,const int waitInSeconds){
      return ForkerPipe::GetStringFromUniquePipeWithWait(id,resultString,waitInSeconds);
   }
   LR_VIRTUAL bool MakeUniqueFifo(const CommandId& id) {
      return ForkerPipe::MakeUniqueFifo(id);
   }
   LR_VIRTUAL bool DestroyUniqueFifo(const CommandId& id) {
      return ForkerPipe::DestroyUniqueFifo(id);
   }
   LR_VIRTUAL int ReadOpenUniqueFifo(const CommandId& id) {
      return ForkerPipe::ReadOpenUniqueFifo(id);
   }
   LR_VIRTUAL int WriteOpenUniqueFifo(const CommandId& id) {
      return ForkerPipe::WriteOpenUniqueFifo( id);
   }
   LR_VIRTUAL std::string ConstructUniquePipeName(const CommandId& id){
      return ForkerPipe::ConstructUniquePipeName(id);
   }
   
   LR_VIRTUAL std::string GetTargetReceivePipe(){
      return ForkerPipe::GetTargetReceivePipe();
   }
   LR_VIRTUAL std::string GetTargetSendPipe(){
      return ForkerPipe::GetTargetSendPipe();
   }
   LR_VIRTUAL std::string GetUUID(){
      return ForkerPipe::GetUUID();
   }
   LR_VIRTUAL int RunExecVE(const char* command, char** arguments, char** environment){
      return ForkerPipe::RunExecVE(command, arguments, environment);
   }
   LR_VIRTUAL bool IsChildLiving ( pid_t child, int& returnCode ){
      if (mFakeChildFinished) {
         return false;
      }
      return ForkerPipe::IsChildLiving (child,returnCode );
   }
   LR_VIRTUAL std::string ReadFromReadyPipe(int pipe){
      return ForkerPipe::ReadFromReadyPipe(pipe);
   }
   LR_VIRTUAL bool WaitForDataOnPipe(int pipe, const int waitInSeconds){
      return ForkerPipe::WaitForDataOnPipe(pipe, waitInSeconds);
   }
   void InsertDummyCommand(const CommandId& id, const CommandState& stat) {
      mRunningCommands[id] = stat;
   }
   bool mFakeChildFinished;
};