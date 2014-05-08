#pragma once
#include <stdint.h>
#include <zmq.h>
#include <zlib.h>
#include <map>
#include <string>
#include "IComponentQueue.h"
#include "boost/thread.hpp"
#include "include/global.h"

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
   LR_VIRTUAL void* GetContext();
   LR_VIRTUAL void* GetSocket(void* context);
   LR_VIRTUAL void ServerSetup(const std::string& binding, void*& socket);
   LR_VIRTUAL void ClientSetup(const std::string& binding, void*& socket);
   LR_VIRTUAL bool SetSendHWM(void *socket, int highWaterMark);
   LR_VIRTUAL bool SetReceiveHWM(void *socket, int highWaterMark);
   LR_VIRTUAL void CloseSocket();
   LR_VIRTUAL void CloseContext();
   LR_VIRTUAL bool InitializeMsg(zmq_msg_t& msg);

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

