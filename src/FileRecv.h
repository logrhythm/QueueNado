#pragma once

#include <string>

struct _zctx_t;
typedef struct _zctx_t zctx_t;

class FileRecv {
public: 

   enum class Socket : std::int8_t { INVALID = -1, OK = 0 };
   enum class Stream : std::int8_t { TIMEOUT = -2, INTERRUPT = -1, END_OF_STREAM = 0, CONTINUE = 1 };

   FileRecv();

   Socket SetLocation(const std::string& location);
   void SetTimeout(const int timeoutMs);
   Stream Receive(std::vector<uint8_t>& data);
   virtual ~FileRecv();

protected:
   void RequestChunks();
   void FreeChunk();
   
private:
   void* mDealer;
   zctx_t* mCtx;
   size_t mQueueLength;
   int mTimeoutMs;
   size_t mCredit;
   size_t mOffset;
   zframe_t *mChunk;
};