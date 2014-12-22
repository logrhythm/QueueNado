#include <czmq.h>
#include <g2log.hpp>
#include <algorithm>

#include "FileRecv.h"

FileRecv::FileRecv(): 
mQueueLength(10),
mTimeoutMs(30000), //5 minutes
mOffset(0),
mChunk(nullptr) {
   mCtx = zctx_new();
   CHECK(mCtx);
   mDealer = zsocket_new(mCtx, ZMQ_DEALER);
   CHECK(mDealer);
   mCredit = mQueueLength;
}

FileRecv::Socket FileRecv::SetLocation(const std::string& location){
   int result = zsocket_connect(mDealer, location.c_str());
   return (0 == result) ? FileRecv::Socket::OK : FileRecv::Socket::INVALID;
}

void FileRecv::SetTimeout(const int timeoutMs){
   mTimeoutMs = timeoutMs;
}


void FileRecv::RequestChunks(){
   // Send enough data requests to fill pipeline:
   while (mCredit && !zctx_interrupted) {
      zstr_sendf (mDealer, "%ld", mOffset);
      mOffset++;
      mCredit--;
   }   
}

FileRecv::Stream FileRecv::Receive(std::vector<uint8_t>& data){
   //Erase any previous data from the last Monitor()
   FreeChunk();
   RequestChunks();
   static const std::vector<uint8_t> emptyOnError;

   //Poll to see if anything is available on the pipeline:
   if(zsocket_poll(mDealer, mTimeoutMs)){

      mChunk = zframe_recv (mDealer);
      if(!mChunk) {
         data = emptyOnError;
         //Interrupt or end of stream
         return FileRecv::Stream::INTERRUPT;
      }


      //chunk->size = zframe_size (mChunk);
      int size = zframe_size (mChunk);
      data.resize(size);
      if(size <= 0){
         //End of Stream
         return FileRecv::Stream::END_OF_STREAM;
      }

      uint8_t* raw = reinterpret_cast<uint8_t*>(zframe_data(mChunk));
      std::copy(raw, raw + size, data.begin());

      mCredit++;
      return FileRecv::Stream::CONTINUE;

   } 

   data = emptyOnError;
   return FileRecv::Stream::TIMEOUT;   
}

void FileRecv::FreeChunk(){
   if(mChunk != nullptr){
      zframe_destroy(&mChunk);
      mChunk = nullptr;
   }
}

FileRecv::~FileRecv(){
   FreeChunk();
   zsocket_destroy(mCtx, mDealer);
   zctx_destroy(&mCtx);
}