#include "SendPacketZMQ.h"

SendPacketZMQ::SendPacketZMQ(const std::string& binding) :
Rifle(binding) {

}

bool SendPacketZMQ::SendData(void* data, const unsigned int timeout) {
   return Rifle::FireStake(data,timeout);
}

bool SendPacketZMQ::SendData(const std::string& data) {
   return Rifle::Fire(data);
}

/**
 * This must be called before initialize.
 * @param size
 */
void SendPacketZMQ::SetQueueSize(const int size) {
   return Rifle::SetHighWater(size);
}

bool SendPacketZMQ::Initialize() {
   return Rifle::Aim();
}
