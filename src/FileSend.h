#pragma once

#include <string>
#include <vector>

struct _zctx_t;
typedef struct _zctx_t zctx_t;

class FileSend {
public:

   enum class Socket : std::int8_t { INVALID = -1, OK = 0 };
   enum class Stream : std::int8_t { TIMEOUT = -2, INTERRUPT = -1, CONTINUE = 0 };

   FileSend();
   Socket SetLocation(const std::string& location);
   void SetTimeout(const int timeout);
   Stream SendFinal();
   Stream SendData(const std::vector<uint8_t>& data);
   virtual ~FileSend();
    
protected:
   Stream SendRawData(const uint8_t*, int size);
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