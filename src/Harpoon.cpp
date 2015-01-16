/*
 * File:   Harpoon.h
 * Author: Ryan Kophs
 *
 * Created on December, 2014
 */

#include <czmq.h>
#include <g2log.hpp>
#include <algorithm>
#include "Harpoon.h"


/// Creates the client that is to connect to the server/Kraken
Harpoon::Harpoon(): 
mQueueLength(1), //Number of allowed messages in queue
mTimeoutMs(300000), //5 minutes
mOffset(0),
mChunk(nullptr) {
   mCtx = zctx_new();
   CHECK(mCtx);
   mDealer = zsocket_new(mCtx, ZMQ_DEALER);
   CHECK(mDealer);
   mCredit = mQueueLength;
}

/// Set location of the queue (TCP location)
Harpoon::Spear Harpoon::Aim(const std::string& location){
   int result = zsocket_connect(mDealer, location.c_str());
   return (0 == result) ? Harpoon::Spear::IMPALED : Harpoon::Spear::MISS;
}

/// Set the amount of time in MS the client should wait for new data.
void Harpoon::MaxWaitInMs(const int timeoutMs){
   mTimeoutMs = timeoutMs;
}

/// Send out ACKSs to the Server that request new chunks. The server will only fill up the 
/// queue with a number of responses equal to the number of ACKs in the queue in order
/// to ensure the queue doesn't get overloaded. Max around of chunks is equal to mCredit
/// default to 10 max.
void Harpoon::RequestChunks(){
   // Send enough data requests to fill pipeline:
   while (mCredit && !zctx_interrupted) {
      zstr_sendf (mDealer, "%ld", mOffset);
      mOffset++;
      mCredit--;
   }   
}

/// Block until timeout or if there is new data to be received.
Harpoon::Battling Harpoon::Heave(std::vector<uint8_t>& data){
   //Erase any previous data from the last Monitor()
   FreeChunk();
   RequestChunks();
   static const std::vector<uint8_t> emptyOnError;

   //Poll to see if anything is available on the pipeline:
   if(zsocket_poll(mDealer, mTimeoutMs)){

      mChunk = zframe_recv (mDealer);
      if(!mChunk) {
         data = emptyOnError;
         return Harpoon::Battling::INTERRUPT;
      }

      int size = zframe_size (mChunk);
      data.resize(size);
      if(size <= 0){
         return Harpoon::Battling::VICTORIOUS;
      }

      uint8_t* raw = reinterpret_cast<uint8_t*>(zframe_data(mChunk));
      std::copy(raw, raw + size, data.begin());

      mCredit++;
      return Harpoon::Battling::CONTINUE;

   } 

   data = emptyOnError;
   return Harpoon::Battling::TIMEOUT;   
}

///Free the chunk of data struct used by ZMQ
void Harpoon::FreeChunk(){
   if(mChunk != nullptr){
      zframe_destroy(&mChunk);
      mChunk = nullptr;
   }
}


/// Destruction and frees of internal zmq memory 
Harpoon::~Harpoon(){
   FreeChunk();
   zsocket_destroy(mCtx, mDealer);
   zctx_destroy(&mCtx);
}