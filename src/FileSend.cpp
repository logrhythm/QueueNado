#include <czmq.h>
#include <g2log.hpp>

#include "FileSend.h"

FileSend::FileSend():
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
FileSend::Socket FileSend::SetLocation(const std::string& location){
   mLocation = location;
   zsocket_set_hwm(mRouter, mQueueLength * 2);

   int result = zsocket_bind(mRouter, mLocation.c_str());
   return result ? FileSend::Socket::OK : FileSend::Socket::INVALID;
}

/// Set the amount of time in MS the server should wait for client ACKs
void FileSend::SetTimeout(const int timeoutMs){
   mTimeoutMs = timeoutMs;
}

//Free the chunk of data struct used by ZMQ in ACKs from the client
void FileSend::FreeOldRequests(){
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
void FileSend::FreeChunk(){
   if(mChunk != nullptr){
      zframe_destroy(&mChunk);
      mChunk = nullptr;
   }
}

/// Internally used to get an ACK from the client asking for another chunk.
/// We use this so that the sender does not send more data to the client than what the
/// client can consume and therefore overloading the queue.
FileSend::Stream FileSend::NextChunkId(){

   FreeChunk();
   FreeOldRequests();

   //Poll to see if anything is available on the pipeline:
   if(zsocket_poll(mRouter, mTimeoutMs)){

      // First frame is the identity of the client
      mIdentity = zframe_recv (mRouter);
      if (!mIdentity) {
         return FileSend::Stream::INTERRUPT;
      }

   } else {
      return FileSend::Stream::TIMEOUT;
   }

   //Poll to see if anything is available on the pipeline:
   if(zsocket_poll(mRouter, mTimeoutMs)){

      // Second frame is next chunk requested of the file
      mNextChunk = zstr_recv (mRouter);
      if(!mNextChunk){
         return FileSend::Stream::INTERRUPT;
      }

      return FileSend::Stream::CONTINUE;

   } 
   
   return FileSend::Stream::TIMEOUT;
}

/// Send data to client
FileSend::Stream FileSend::SendData(const std::vector<uint8_t>& dataToSend){
   return SendRawData(dataToSend.data(), dataToSend.size());
}

/// Signals the end of the stream. This HAS TO BE CALLED by the Client
/// when transfer is finished.
FileSend::Stream FileSend::SendFinal(){
   return SendRawData(nullptr, 0);
}

/// Internal call to send a data array to the client.
FileSend::Stream FileSend::SendRawData(const uint8_t* data, int size) {
   FreeChunk();
   FreeOldRequests();

   const auto next = NextChunkId();
   if(FileSend::Stream::CONTINUE != next){
      return next;
   }

   mChunk = zframe_new(data, size);

   // Send chunk to client
   zframe_send (&mIdentity, mRouter, ZFRAME_REUSE + ZFRAME_MORE);
   zframe_send (&mChunk, mRouter, 0);

   return FileSend::Stream::CONTINUE;

}

FileSend::~FileSend(){
   zsocket_unbind(mRouter, mLocation.c_str());
   zsocket_destroy(mCtx, mRouter);
   zctx_destroy(&mCtx);
   FreeOldRequests();
   FreeChunk();
}