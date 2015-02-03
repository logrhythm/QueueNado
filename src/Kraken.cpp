/*
 * File:   Harpoon.h
 * Author: Ryan Kophs
 *
 * Created on December, 2014
 */

#include <czmq.h>
#include <g2log.hpp>

#include "Kraken.h"
#include <chrono>


/// Constructing the server/Kraken that is about to be connected/impaled by the client/Harpoon
Kraken::Kraken():
mLocation(""), 
mQueueLength(10),
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

//Wait for input on the queue
// NOTE: zsocket_poll returns true only when the ZMQ_POLLIN is returned by zmq_poll. If false is
// returned it does not automatically mean a timeout occurred waiting for input. So std::chrono is
// used to determine when the poll has truly timed out.
Kraken::Battling Kraken::PollTimeout(){
   using namespace std::chrono;

   steady_clock::time_point pollStartMs = steady_clock::now();
   while (!zsocket_poll(mRouter, 1)){
      int pollElapsedMs = duration_cast<milliseconds>(steady_clock::now() - pollStartMs).count();
      if (pollElapsedMs >= mTimeoutMs){
         return Kraken::Battling::TIMEOUT;
      }
   }
   return Kraken::Battling::CONTINUE;
}

/// Internally used to get an ACK from the client asking for another chunk.
/// We use this so that the sender does not send more data to the client than what the
/// client can consume and therefore overloading the queue.
Kraken::Battling Kraken::NextChunkId(){

   FreeChunk();
   FreeOldRequests();

   //Poll to see if anything is available on the pipeline:
   if(Kraken::Battling::CONTINUE == PollTimeout()){

      // First frame is the identity of the client
      mIdentity = zframe_recv (mRouter);
      if(!mIdentity){
         return Kraken::Battling::INTERRUPT;
      }

   } else {
      return Kraken::Battling::TIMEOUT;
   }

   //Poll to see if anything is available on the pipeline:
   if(Kraken::Battling::CONTINUE == PollTimeout()){

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
   return SendRawData(dataToSend.data(), dataToSend.size());
}

/// Signals the end of the Battling. This HAS TO BE CALLED by the Client
/// when transfer is finished.
Kraken::Battling Kraken::FinalBreach(){
   return SendRawData(nullptr, 0);
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
   FreeOldRequests();
   FreeChunk();
}
