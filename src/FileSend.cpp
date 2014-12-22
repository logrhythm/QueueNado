#include <czmq.h>
#include <g2log.hpp>

#include "FileSend.h"

FileSend::FileSend():
mLocation(""), 
mQueueLength(10),
mNextChunk(nullptr), 
mIdentity(nullptr), 
mTimeoutMs(30000), //5 Minutes
mChunk(nullptr) {
   mCtx = zctx_new();
   CHECK(mCtx);
   mRouter = zsocket_new(mCtx, ZMQ_ROUTER);
   CHECK(mRouter);
}

FileSend::Socket FileSend::SetLocation(const std::string& location){
   mLocation = location;
   zsocket_set_hwm(mRouter, mQueueLength * 2);

   int result = zsocket_bind(mRouter, mLocation.c_str());
   return result ? FileSend::Socket::OK : FileSend::Socket::INVALID;
}

void FileSend::SetTimeout(const int timeoutMs){
   mTimeoutMs = timeoutMs;
}

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

void FileSend::FreeChunk(){
   if(mChunk != nullptr){
      zframe_destroy(&mChunk);
      mChunk = nullptr;
   }
}

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

FileSend::Stream FileSend::SendData(const std::vector<uint8_t>& dataToSend){
   return SendRawData(dataToSend.data(), dataToSend.size());
}

FileSend::Stream FileSend::SendFinal(){
   return SendRawData(nullptr, 0);
}

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