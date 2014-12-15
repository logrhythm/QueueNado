#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <string>
#include <vector>

struct _zctx_t;
typedef struct _zctx_t zctx_t;

class FileRecv {
public:
   explicit FileRecv();
   int SetLocation(const std::string& location);
   void SetTimeout(const int timeout);
   size_t Monitor();
   uint8_t* GetChunkData();
   size_t GetChunkSize();
   virtual ~FileRecv();

protected:
   void RequestChunks();
   void FreeChunk();
   void FreeData();
   
private:
   void *mDealer;
   zctx_t *mCtx;
   size_t mQueueLength;
   int mTimeout;
   size_t mCredit;
   size_t mOffset;
   size_t mSize;
   uint8_t *mData;
   zframe_t *mChunk;
};