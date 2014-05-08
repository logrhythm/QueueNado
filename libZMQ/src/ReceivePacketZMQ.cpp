#include "ReceivePacketZMQ.h"
using namespace std;

/**
 * Default constructor
 */
ReceivePacketZMQ::ReceivePacketZMQ(const std::string& binding) :
      Vampire(binding) {

}

/**
 * This must be called before initialize.
 * @param size
 */
void ReceivePacketZMQ::SetQueueSize(const int size) {
   return Vampire::SetHighWater(size);
}

bool ReceivePacketZMQ::Initialize() {
   return Vampire::PrepareToBeShot();
}

bool ReceivePacketZMQ::ReceiveDataBlockPointer(ctb_ppacket& dataReceived, const int milliseconds) {
   void* data;
   bool returnVal = Vampire::GetStake(data,milliseconds);
   if ( returnVal) {
      dataReceived = reinterpret_cast<ctb_ppacket>(data);
   } else { 
      dataReceived = NULL;
   }
   return returnVal;
}
bool ReceivePacketZMQ::ReceiveDataBlock(std::string& dataReceived, const int milliseconds) {
   return Vampire::GetShot(dataReceived,milliseconds);
}
