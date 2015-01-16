/*
 * File:   Harpoon.h
 * Author: Ryan Kophs
 *
 * Created on December, 2014
 */

#include <czmq.h>
#include <g2log.hpp>
#include "Kraken.h"


/// Constructing the server/Kraken that is about to be connected/impaled by the client/Harpoon
Kraken::Kraken():
mLocation(""), 
mQueueLength(1), //Number of allowed messages in queue
mMaxChunkSize(10485760), //10MB
mNextChunk(nullptr), 
mIdentity(nullptr), 
mTimeoutMs(300000), //5 Minutes
mChunk(nullptr) {
   mCtx = zctx_new();
   CHECK(mCtx);
   mRouter = zsocket_new(mCtx, ZMQ_ROUTER);
   CHECK(mRouter);
}

/// Set location of the queue (TCP location)
Kraken::Spear Kraken::SetLocation(const std::string& location){
   mLocation = location;
   zsocket_set_hwm(mRouter, mQueueLength * 2);

   int result = zsocket_bind(mRouter, mLocation.c_str());
   return result ? Kraken::Spear::IMPALED : Kraken::Spear::MISS;
}

/// Set the amount of time in MS the server should wait for client ACKs
void Kraken::MaxWaitInMs(const int timeoutMs){
   mTimeoutMs = timeoutMs;
}

void Kraken::ChangeDefaultMaxChunkSizeInBytes(const size_t bytes){
   mMaxChunkSize = bytes;
}

//Free the chunk of data struct used by ZMQ in ACKs from the client
void Kraken::FreeOldRequests(){
   if(mIdentity != nullptr){
      zframe_destroy(&mIdentity);
      mIdentity = nullptr;
   }
   if(mNextChunk != nullptr){
      delete [] mNextChunk;
      mNextChunk = nullptr;
   }
}

//Free the chunk of data struct used by ZMQ
void Kraken::FreeChunk(){
   if(mChunk != nullptr){
      zframe_destroy(&mChunk);
      mChunk = nullptr;
   }
}

/// Internally used to get an ACK from the client asking for another chunk.
/// We use this so that the sender does not send more data to the client than what the
/// client can consume and therefore overloading the queue.
Kraken::Battling Kraken::NextChunkId(){

   FreeChunk();
   FreeOldRequests();

   //Poll to see if anything is available on the pipeline:
   if(zsocket_poll(mRouter, mTimeoutMs)){

      // First frame is the identity of the client
      mIdentity = zframe_recv (mRouter);
      if (!mIdentity) {
         return Kraken::Battling::INTERRUPT;
      }

   } else {
      return Kraken::Battling::TIMEOUT;
   }

   //Poll to see if anything is available on the pipeline:
   if(zsocket_poll(mRouter, mTimeoutMs)){

      // Second frame is next chunk requested of the file
      mNextChunk = zstr_recv (mRouter);
      if(!mNextChunk){
         return Kraken::Battling::INTERRUPT;
      }

      return Kraken::Battling::CONTINUE;

   } 
   
   return Kraken::Battling::TIMEOUT;
}

/// Send data to client
Kraken::Battling Kraken::SendTidalWave(const std::vector<uint8_t>& dataToSend){
   size_t size = dataToSend.size();
   if(size == 0){
      return Kraken::Battling::CONTINUE;   
   }

   const uint8_t* data = dataToSend.data();
   Kraken::Battling status = Kraken::Battling::CONTINUE;

   for(size_t i = 0; i < size; i += mMaxChunkSize){
      size_t chunkSize = std::min(size - i, mMaxChunkSize);

      status = SendRawData(&data[i], chunkSize);
      if (Kraken::Battling::CONTINUE != status) {
         return status;
      }
   }

   return status;
}

/// Signals the end of the Battling. This HAS TO BE CALLED by the Client
/// when transfer is finished.
Kraken::Battling Kraken::FinalBreach(){
   auto complete = SendRawData(nullptr, 0);
   
   //Clean out any previous packets in the channel before initializing
   while(zsocket_poll(mRouter, 1)){
      FreeOldRequests();
      mIdentity = zframe_recv(mRouter);
   }
   return complete;
}

/// Internal call to send a data array to the client.
Kraken::Battling Kraken::SendRawData(const uint8_t* data, int size) {
   
   FreeChunk();
   FreeOldRequests();

   const auto next = NextChunkId();
   if(Kraken::Battling::CONTINUE != next){
      return next;
   }

   mChunk = zframe_new(data, size);

   // Send chunk to client
   zframe_send (&mIdentity, mRouter, ZFRAME_REUSE + ZFRAME_MORE);
   zframe_send (&mChunk, mRouter, 0);

   return Kraken::Battling::CONTINUE;

}


/// Destruction of the Kraken and zmq socket and memory cleanup
Kraken::~Kraken(){
   zsocket_unbind(mRouter, mLocation.c_str());
   zsocket_destroy(mCtx, mRouter);
   zctx_destroy(&mCtx);
   mCtx = nullptr;
   FreeOldRequests();
   FreeChunk();
}
