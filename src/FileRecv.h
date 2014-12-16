#pragma once

#include <string>

struct _zctx_t;
typedef struct _zctx_t zctx_t;

class FileRecv {
public:

   enum Socket : std::int8_t { INVALID = -1, OK = 0 };
   enum Stream : std::int8_t { TIMEOUT = -2, INTERRUPT = -1, END_OF_STREAM = 0, CONTINUE = 1 };

   FileRecv();
   Socket SetLocation(const std::string& location);
   void SetTimeout(const int timeoutMs);
   Stream Receive();
   uint8_t* GetChunkData();
   size_t GetChunkSize();
   virtual ~FileRecv();

protected:
   void RequestChunks();
   void FreeChunk();
   void FreeData();
   
private:
   void* mDealer;
   zctx_t* mCtx;
   size_t mQueueLength;
   int mTimeoutMs;
   size_t mCredit;
   size_t mOffset;
   size_t mSize;
   uint8_t* mData;
   zframe_t *mChunk;
};