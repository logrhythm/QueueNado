#pragma once
#include "ZeroMQ.h"



class MockZeroMQPacket: public ZeroMQ<void*> {
public:
   explicit MockZeroMQPacket(unsigned int id) :
         ZeroMQ<void*>::ZeroMQ(id), mFailsToSetSendHWM(false),mFailsToSetReceiveHWM(false),mFailsToInitMsg(false) {
   }
   MockZeroMQPacket(const MockZeroMQPacket& that) :
         ZeroMQ<void*>::ZeroMQ(that), mFailsToSetSendHWM(false),mFailsToSetReceiveHWM(false),mFailsToInitMsg(false) {
   }
   MockZeroMQPacket(const MockZeroMQPacket* that) :
         ZeroMQ<void*>::ZeroMQ(that), mFailsToSetSendHWM(false),mFailsToSetReceiveHWM(false),mFailsToInitMsg(false) {
   }
   std::string GetBinding() {
      return mBinding;
   }
   unsigned int GetId() {
      return mId;
   }
   bool OwnsContext() {
      return mOwnsContext;
   }
   bool ContextSet() {
      return mContext != NULL;
   }
   bool SocketSet() {
      return mSocket != NULL;
   }
   void* GetContext() {
      return ZeroMQ<void*>::GetContext();
   }
   void* GetSocket(void* context) {
      return ZeroMQ<void*>::GetSocket(context);
   }
   void ServerSetup(const std::string& binding, void*& socket) {
      ZeroMQ<void*>::ServerSetup(binding, socket);
   }
   void ClientSetup(const std::string& binding, void*& socket) {
      ZeroMQ<void*>::ClientSetup(binding, socket);
   }
   bool InitializeMsg(zmq_msg_t& msg) {
      if (mFailsToInitMsg) {
         return false;
      }
      return ZeroMQ<void*>::InitializeMsg(msg);
   }
   bool SetSendHWM(void *socket, int highWaterMark) {
      if (mFailsToSetSendHWM) {
         return false;
      }
      return ZeroMQ<void*>::SetSendHWM(socket,highWaterMark);
   }
   bool SetReceiveHWM(void *socket, int highWaterMark) {
      if (mFailsToSetReceiveHWM) {
         return false;
      }
      return ZeroMQ<void*>::SetReceiveHWM(socket,highWaterMark);
   }
   bool mFailsToSetSendHWM;
   bool mFailsToSetReceiveHWM;
   bool mFailsToInitMsg;
};
