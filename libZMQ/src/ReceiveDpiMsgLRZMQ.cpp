#include "ReceiveDpiMsgLRZMQ.h"
using namespace std;

/**
 * Default constructor
 */
ReceiveDpiMsgLRZMQ::ReceiveDpiMsgLRZMQ(const std::string& binding) :
      Vampire(binding) {

}
bool ReceiveDpiMsgLRZMQ::Initialize() {
   return Vampire::PrepareToBeShot();
}
bool ReceiveDpiMsgLRZMQ::ReceiveDataBlock(std::string& data,const int timeout) {
   return Vampire::GetShot(data,timeout);
}

/**
 * This must be called before initialize.
 * @param size
 */
void ReceiveDpiMsgLRZMQ::SetQueueSize(const int size) {
   return Vampire::SetHighWater(size);
}
