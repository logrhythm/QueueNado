#pragma once

#include <string>

struct _zctx_t;
typedef struct _zctx_t zctx_t;

typedef struct DataChunk {
   uint8_t* data = nullptr;
   size_t size = 0;
   void Reset(){
      if(data != nullptr){
         free(data);
      }
      data = nullptr;
      size = 0;
   }
   ~DataChunk(){
      Reset();
   }
} DataChunk;

class FileRecv {
public: 

   typedef std::unique_ptr<DataChunk, void(*)(DataChunk*)> DataPacket;
   enum Socket : std::int8_t { INVALID = -1, OK = 0 };
   enum Stream : std::int8_t { TIMEOUT = -2, INTERRUPT = -1, END_OF_STREAM = 0, CONTINUE = 1 };

   FileRecv();

   static DataPacket DataPacketFactory();

   Socket SetLocation(const std::string& location);
   void SetTimeout(const int timeoutMs);
   Stream Receive(DataPacket& chunk);
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