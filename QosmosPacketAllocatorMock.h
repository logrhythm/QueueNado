#pragma once

#include "QosmosPacketAllocator.h"

class MockQosmosPacketAllocator : public QosmosPacketAllocator {
public:
   MockQosmosPacketAllocator() : mMallocpacketFail(false){};

   size_t NumberOfKnownHashes() {
      return mPacketPositions.size();
   }

   size_t NumberOfFreeHashes() {
      return mFreePositions.size();
   }

   size_t SizeOfgivenHashData(unsigned int hash) {
      unsigned int position(0);

      auto foundPosition(mPacketPositions.find(hash));
      if (foundPosition != mPacketPositions.end()) {
         return mPackets[foundPosition->second]->size();
      }

      return 0;

   }

   ctb_ppacket MallocpPacket() {
      if (mMallocpacketFail) {
         return NULL;
      }
      return QosmosPacketAllocator::MallocpPacket();
   }

   bool mMallocpacketFail;
protected:
   //	std::map<unsigned int, std::vector<unsigned int> > mPacketHashes;
   //	std::map<unsigned int, std::vector<std::string > > mPackets;
   //	std::map<unsigned int, boost::recursive_mutex* > mMutexes;
   //	boost::recursive_mutex mMutex;
private:
};
