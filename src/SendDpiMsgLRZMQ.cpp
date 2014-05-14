#include "SendDpiMsgLRZMQ.h"
SendDpiMsgLRZMQ::SendDpiMsgLRZMQ(const std::string& binding) :
      Rifle(binding) {
}

bool SendDpiMsgLRZMQ::SendData(const std::string& data) {
   return Rifle::Fire(data);
}

bool SendDpiMsgLRZMQ::Initialize() {
   return Rifle::Aim();
}
/**
 * * This must be called before initialize.
 * @param size
 */
void SendDpiMsgLRZMQ::SetQueueSize(const int size) {
   return Rifle::SetHighWater(size);
}
