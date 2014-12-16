#include <memory>
#include <czmq.h>
#include <g2log.hpp>

#include "FileRecv.h"

FileRecv::FileRecv(): 
mQueueLength(10),
mTimeoutMs(30000), //5 minutes
mOffset(0),
mSize(0),
mData(nullptr),
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
   while (mCredit) {
      zstr_sendf (mDealer, "%ld", mOffset);
      mOffset += 1;
      mCredit--;
   }   
}

FileRecv::Stream FileRecv::Receive(){
    
   //Erase any previous data from the last Monitor()
   FreeChunk();
   FreeData();

   RequestChunks();

   //Poll to see if anything is available on the pipeline:
   if(zsocket_poll(mDealer, mTimeoutMs)){

      mChunk = zframe_recv (mDealer);
      if(!mChunk) {
         //Interrupt or end of stream
         return FileRecv::Stream::INTERRUPT;
      }

      mSize = zframe_size (mChunk);
      if(mSize <= 0){
         //End of Stream
         return FileRecv::Stream::END_OF_STREAM;
      }
        
      mData = reinterpret_cast<uint8_t*>(malloc(mSize));  
      memcpy(mData, reinterpret_cast<void*>(zframe_data(mChunk)), mSize);

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

void FileRecv::FreeData(){
   if(mData != nullptr){
      free(mData);  // delete [] mData;
      mData = nullptr;
   }
}

uint8_t* FileRecv::GetChunkData(){
   return mData;
}

size_t FileRecv::GetChunkSize(){
   return mSize;
}

FileRecv::~FileRecv(){
   FreeChunk();
   FreeData();
   zsocket_destroy(mCtx, mDealer);
   zctx_destroy(&mCtx);
}