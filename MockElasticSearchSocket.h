#pragma once
#include "ElasticSearchSocket.h"
#include "MockBoomStick.h"
#include "include/global.h"
class BoomStick;
#ifdef LR_DEBUG

class FastMockElasticSearchSocket : public ElasticSearchSocket {
public:
   FastMockElasticSearchSocket(BoomStick& transport, const bool asynchronous) : ElasticSearchSocket::ElasticSearchSocket(transport, asynchronous) {
   }

   int GetSocketWaitTimeInMs(const std::map<std::string, std::string>& inFlightCommands) const LR_OVERRIDE {
      return 0;
   }
protected:

};

class MockElasticSearchSocket : public ElasticSearchSocket {
public:

   explicit MockElasticSearchSocket(bool async) : mMyTransport(""), ElasticSearchSocket(mMyTransport, async), mReplySent(false) {

   }

   MockElasticSearchSocket(BoomStick& transport, bool async) : mMyTransport(""), ElasticSearchSocket(transport, async), mReplySent(false) {

   }

   virtual ~MockElasticSearchSocket() {

   }

   void ZSocketSend(void* socket, const std::string& uuid, const std::string& reply) {
      ElasticSearchSocket::ZSocketSend(socket, uuid, reply);
      mReplySent = true;
   }
   MockBoomStick mMyTransport;
   bool mReplySent;
};
#endif