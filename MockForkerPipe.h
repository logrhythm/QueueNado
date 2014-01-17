#include "ForkerPipe.h"

class MockForkerPipe : public ForkerPipe {
public:

   MockForkerPipe(std::string programName, bool client = true) : ForkerPipe(programName, client) {
   }
   LR_VIRTUAL int GetReplyTimeout() { return 1;}
   LR_VIRTUAL int GetSendReplyTimeout(){ return 1;}
   LR_VIRTUAL int GetRunningCommandTimeout(){ return 1;}
};