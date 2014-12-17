#include <memory>
#include <czmq.h>
#include <g2log.hpp>

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

FileRecv::DataPacket FileRecv::DataPacketFactory(){
   return DataPacket(new DataChunk, [&](DataChunk* chunk){ delete chunk; });
};

FileRecv::Socket FileRecv::SetLocation(const std::string& location){
   int result = zsocket_connect(mDealer, location.c_str());
   return (0 == result) ? FileRecv::Socket::OK : FileRecv::Socket::INVALID;
}

void FileRecv::SetTimeout(const int timeoutMs){
   mTimeoutMs = timeoutMs;
}


void FileRecv::RequestChunks(){
   // Send enough data requests to fill pipeline:
   while (mCredit) {
      zstr_sendf (mDealer, "%ld", mOffset);
      mOffset++;
      mCredit--;
   }   
}

FileRecv::Stream FileRecv::Receive(DataPacket& chunk){
    
   //Erase any previous data from the last Monitor()
   FreeChunk();
   chunk->Reset();

   RequestChunks();

   //Poll to see if anything is available on the pipeline:
   if(zsocket_poll(mDealer, mTimeoutMs)){

      mChunk = zframe_recv (mDealer);
      if(!mChunk) {
         //Interrupt or end of stream
         return FileRecv::Stream::INTERRUPT;
      }

      chunk->size = zframe_size (mChunk);
      if(chunk->size <= 0){
         //End of Stream
         return FileRecv::Stream::END_OF_STREAM;
      }

      chunk->data = reinterpret_cast<uint8_t*>(malloc(chunk->size));
      memcpy(chunk->data, reinterpret_cast<void*>(zframe_data(mChunk)), chunk->size);

      mCredit++;
      return FileRecv::Stream::CONTINUE;

   } else {
      //timeout
      return FileRecv::Stream::TIMEOUT;
   }
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