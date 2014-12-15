#pragma once

#include <cstdlib>
#include <cstdint>
#include <string>

struct _zctx_t;
typedef struct _zctx_t zctx_t;

class FileSend {
public:
   explicit FileSend();
   int SetLocation(const std::string& location);
   void SetTimeout(const int timeout);
   int SendData(uint8_t* data, size_t size);
   virtual ~FileSend();
    
protected:
   int NextChunkId(); 
   void FreeOldRequests();
   void FreeChunk();

private:
   void * mRouter;
   zctx_t * mCtx;
   std::string mLocation;
   size_t mQueueLength;
   char * mNextChunk;
   size_t mNextChunkId;
   zframe_t * mIdentity;
   int mTimeout;
   zframe_t * mChunk;
};