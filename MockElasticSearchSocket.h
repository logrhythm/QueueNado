#pragma once
#include "ElasticSearchSocket.h"
#include "MockBoomStick.h"
#include "include/global.h"
class BoomStick;
#ifdef LR_DEBUG
class MockElasticSearchSocket : public ElasticSearchSocket {
public:

   explicit MockElasticSearchSocket(bool async) : mMyTransport(""), ElasticSearchSocket(mMyTransport,async), mReplySent(false) {

   }
   MockElasticSearchSocket(BoomStick& transport, bool async) : mMyTransport(""), ElasticSearchSocket(transport,async),  mReplySent(false) {

   }
   virtual ~MockElasticSearchSocket() {

   }

   void ZSocketSend(void* socket, const std::string& reply) {
      ElasticSearchSocket::ZSocketSend(socket,reply);
      mReplySent = true;
   }
   MockBoomStick mMyTransport;
   bool mReplySent;
};
#endif