#pragma once

#include <string>

struct _zctx_t;
typedef struct _zctx_t zctx_t;

class FileSend {
public:

   enum Socket : std::int8_t { INVALID = -1, OK = 0 };
   enum Stream : std::int8_t { TIMEOUT = -2, INTERRUPT = -1, CONTINUE = 0 };

   FileSend();
   Socket SetLocation(const std::string& location);
   void SetTimeout(const int timeout);
   Stream SendData(uint8_t* data, size_t size);
   virtual ~FileSend();
    
protected:
   Stream NextChunkId(); 
   void FreeOldRequests();
   void FreeChunk();

private:
   void* mRouter;
   zctx_t* mCtx;
   std::string mLocation;
   size_t mQueueLength;
   char* mNextChunk;
   zframe_t* mIdentity;
   int mTimeoutMs;
   zframe_t* mChunk;
};