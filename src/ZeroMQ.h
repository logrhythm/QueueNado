#pragma once
#include <stdint.h>
#include <zmq.h>
#include <zlib.h>
#include <map>
#include <string>
#include "IComponentQueue.h"
#include "boost/thread.hpp"
#include "global.h"

template<class dataType>
class ZeroMQ : public IComponentQueue {
public:
   ZeroMQ();
   ZeroMQ(const ZeroMQ<dataType>& that);
   virtual ~ZeroMQ();

};

template<>
class ZeroMQ<void*> : public IComponentQueue {
public:
   explicit ZeroMQ(const unsigned int id);
   ZeroMQ(const ZeroMQ<void*>& that);
   ZeroMQ(const ZeroMQ<void*>* that);
   virtual ~ZeroMQ();

   bool Initialize();
   int GetHighWater();
   bool WaitForClient(int microseconds);
   bool SendClientReady();
   void* GetPointer(long timeout);
   bool SendPointer(void* packet);
protected:
   virtual void* GetContext();
   virtual void* GetSocket(void* context);
   virtual void ServerSetup(const std::string& binding, void*& socket);
   virtual void ClientSetup(const std::string& binding, void*& socket);
   virtual bool SetSendHWM(void *socket, int highWaterMark);
   virtual bool SetReceiveHWM(void *socket, int highWaterMark);
   virtual void CloseSocket();
   virtual void CloseContext();
   virtual bool InitializeMsg(zmq_msg_t& msg);

   unsigned int mId;
   std::string mBinding;
   const bool mOwnsContext;
   void* mContext;
   void* mSocket;
private:
   
   int PollForSendSocketReady(long timeout);
   int PollForReceiveSocketReady(long timeout);
};

#define ZeroMQ_HEADER_SIZE 0

