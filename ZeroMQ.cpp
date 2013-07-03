#include "ZeroMQ.h"
#include <iostream>
#include "boost/thread.hpp"
#include "CZMQToolkit.h"
#include "g2logworker.hpp"
#include "g2log.hpp"

/*
 * An overload of the normal free call that would clear message data
 */
void null_free(void* data, void* hint) {

}
using namespace std;
using namespace boost;

/**
 * Default constructor for a zero copy queue of void* pointers
 *
 * Provide an ID which will be used to construct a unique inproc queue
 *
 * @param $first
 *   An integer id number for the process that will own this queue, such as
 * the child thread number.
 */
ZeroMQ<void*>::ZeroMQ(const unsigned int id) :
IComponentQueue::IComponentQueue(), mId(id), mOwnsContext(
true), mContext(NULL), mSocket(NULL) {
   stringstream bindingStream;
   bindingStream << "inproc://voidstar_" << getpid() << "_" << mId;
   mBinding = bindingStream.str();

}

/**
 * Copy constructor
 *
 * Inherits access to the context used by the source Queue as well as internal settings
 */
ZeroMQ<void*>::ZeroMQ(const ZeroMQ<void*>& that) :
IComponentQueue::IComponentQueue(), mId(that.mId), mBinding(
that.mBinding), mOwnsContext(false), mContext(that.mContext), mSocket(
NULL) {

}

/**
 * Pointer copy constructor
 *
 * Inherits access to the context used by the source Queue as well as internal settings
 */
ZeroMQ<void*>::ZeroMQ(const ZeroMQ<void*>* that) :
IComponentQueue::IComponentQueue(), mId(that->mId), mBinding(
that->mBinding), mOwnsContext(false), mContext(that->mContext), mSocket(
NULL) {

}

/**
 * Deconstructor
 *
 * Will destroy the context if it was the creator
 */
ZeroMQ<void*>::~ZeroMQ() {
   //LOG(DEBUG) << "deconstructor ZeroMQ<void*> ";
   boost::recursive_mutex::scoped_lock lock(mMutex);
   CloseSocket();
   CloseContext();
}

/**
 * Clean up the socket
 */
void ZeroMQ<void*>::CloseSocket() {
   if (mSocket != NULL) {
      if (zmq_close(mSocket) != 0) {
         LOG(WARNING) << "failed to terminate socket " << mBinding;
         exit(1);
      }
      mSocket = NULL;
   }
}

/**
 * Clean up the context
 */
void ZeroMQ<void*>::CloseContext() {
   if (mContext != NULL && mOwnsContext) {
      if (zmq_term(mContext) != 0) {
         LOG(WARNING) << "failed to terminate context " << mBinding;
         exit(1);
         //TODO do something here
      }
      mContext = NULL;
   }
}

/**
 * Setup client/server sockets and a context
 *
 * @return
 *   false is something goes wrong, if a client has no context
 */
bool ZeroMQ<void*>::Initialize() {
   if (mOwnsContext) {
      mContext = GetContext();
      mSocket = GetSocket(mContext);
      ServerSetup(mBinding, mSocket);
   } else {
      mSocket = GetSocket(mContext);
      ClientSetup(mBinding, mSocket);
   }

   return mSocket != NULL;

}

/**
 * Abstraction of the ZMQ context initializer
 *
 * @return
 *   NULL on failure, if successful an open context
 */
void* ZeroMQ<void*>::GetContext() {
   return zmq_init(1);
}

/**
 * Get the high water mark for the queue
 *
 * @return
 *   the number of messages that will be queued before calls begin to block
 */
int ZeroMQ<void*>::GetHighWater() {
   return 2048;
}

/**
 * Get the PAIR socket for inter thread communication
 *
 * @param $first
 *   An open context
 * @param $second
 *   A string describing a inproc socket binding
 *
 * @return
 *   a void* that is NULL if something went wrong
 */
void* ZeroMQ<void*>::GetSocket(void* context) {
   void* socket = NULL;
   if (context != NULL) {
      socket = zmq_socket(context, ZMQ_PAIR); //TODO switch to PUSH if there are issues in the library with PAIR
      if (!SetSendHWM(socket, GetHighWater()) || !SetReceiveHWM(socket, GetHighWater())) {
         zmq_close(socket);
         socket = NULL;
      } else {
         CZMQToolkit::PrintCurrentHighWater(socket, "ZMQ");
      }  
   }
   return socket;
}

/**
 * Get a socket ready to receive connections
 *
 * @param $first
 *   In/Out parameter that will be closed and set to NULL on a failure
 */
void ZeroMQ<void*>::ServerSetup(const std::string& binding,
        void*& socket) {
   if (socket == NULL) {
      return;
   }
   if (zmq_bind(socket, binding.c_str()) != 0) {
      zmq_close(socket);
      socket = NULL;
   }
}

/**
 * Connect to an existing socket
 *
 * @param $first
 *   In/Out parameter that will be closed and set to NULL on a failure
 */
void ZeroMQ<void*>::ClientSetup(const std::string& binding,
        void*& socket) {
   if (socket == NULL) {
      return;
   }
   if (zmq_connect(socket, binding.c_str()) != 0) {
      zmq_close(socket);
      socket = NULL;
   }

}

/**
 * Wait for the client to start up
 *
 * @param $first
 *   How long in microseconds to sleep, total
 *
 * @return
 *   If the client ever responded that it was ready
 */
bool ZeroMQ<void*>::WaitForClient(int microseconds) {
   if (!mOwnsContext) {
      return false;
   }
   zmq_msg_t msg;
   if (!InitializeMsg(msg)) {
      return false;
   }
   int bytesReceived(-1);
   if (PollForReceiveSocketReady(-1) < 0) {
      return false;
   }
   bytesReceived = zmq_recvmsg(mSocket, &msg, ZMQ_DONTWAIT);
   zmq_msg_close(&msg);
   return (bytesReceived >= 0);
}

/**
 * Polls to see if the send socket is ready
 * 
 * @param Timeout
 *   -1 for unlimited
 * @return 
 *   >=0 on success
 */
int ZeroMQ<void*>::PollForSendSocketReady(long timeout) {
   if (mSocket == NULL) {
      return -1;
   }
   zmq_pollitem_t pollForSendSocketReady;
   pollForSendSocketReady.socket = mSocket;
   pollForSendSocketReady.events = ZMQ_POLLOUT;
   return zmq_poll(&pollForSendSocketReady, 1, timeout);
}

/**
 * Polls to see if the socket has a message to receive
 * 
 * @param Timeout
 *   -1 for unlimited
 * @return 
 *   >=0 on success
 */
int ZeroMQ<void*>::PollForReceiveSocketReady(long timeout) {

   if (mSocket == NULL) {
      return -1;
   }
   zmq_pollitem_t pollForReceiveSocketReady;
   pollForReceiveSocketReady.socket = mSocket;
   pollForReceiveSocketReady.events = ZMQ_POLLIN;
   return zmq_poll(&pollForReceiveSocketReady, 1, timeout);
}

/**
 * Send ready signal to server
 *
 * @return
 *   if the ready message was successfully queued
 */
bool ZeroMQ<void*>::SendClientReady() {
   if (mOwnsContext) {
      return false;
   }
   if (!mSocket) {
      if (!Initialize()) {
         return false;
      }
   }
   zmq_msg_t msg;
   if (!InitializeMsg(msg)) {
      return false;
   }
   if (PollForSendSocketReady(-1) < 0) {
      return false;
   }
   bool result = (zmq_sendmsg(mSocket, &msg, ZMQ_DONTWAIT) >= 0);
   zmq_msg_close(&msg);
   return result;
}

/**
 * Get a void* pointer from the queue, if there is one
 *
 * @param timeout
 *   Timeout in ms
 * @return
 *   NULL if there isn't one to find
 */
void* ZeroMQ<void*>::GetPointer(long timeout) {
   if (mOwnsContext) {
      return NULL;
   }
   zmq_msg_t msg;
   if (!InitializeMsg(msg)) {
      return NULL;
   }


   if ((zmq_recvmsg(mSocket, &msg, ZMQ_DONTWAIT)) >= 0) {
      void * result = *(void**) zmq_msg_data(&msg);
      zmq_msg_close(&msg);
      return result;
   } else if (zmq_errno() == EAGAIN) {
      if (PollForReceiveSocketReady(timeout) < 0) {
         return NULL;
      }
      if ((zmq_recvmsg(mSocket, &msg, ZMQ_DONTWAIT)) >= 0) {
         void * result = *(void**) zmq_msg_data(&msg);
         zmq_msg_close(&msg);
         return result;
      }
   }

   return NULL;
}

/**
 * Send a void* pointer to the other thread
 *
 * @return
 *   If the send was successful
 */
bool ZeroMQ<void*>::SendPointer(void* packet) {
   if (!mOwnsContext) {
      return false;
   }
   zmq_msg_t msg;
   zmq_msg_init_size(&msg, sizeof (void*));
   memcpy(zmq_msg_data(&msg), &packet, sizeof (void*));
   // Benchmarks indicate that the overhead of using a poll here, slows things down
   //   if ( PollForSendSocketReady(-1)  < 0 ) {
   //      return false;
   //   }
   bool result = (zmq_sendmsg(mSocket, &msg, /*ZMQ_DONTWAIT*/0) > 0);
   zmq_msg_close(&msg);
   return result;
}

/**
 * Set the Receive side high water mark for a socket
 *
 * @param $first
 *   a non-null socket pointer
 * @param $second
 *   An integer number of messages
 *
 * @return
 *   if the call was successful
 */
bool ZeroMQ<void*>::SetReceiveHWM(void *socket, int highWaterMark) {
   return (zmq_setsockopt(socket, ZMQ_RCVHWM, &highWaterMark,
           sizeof (highWaterMark)) == 0);
}

/**
 * Set the send side high water mark for a socket
 *
 * @param $first
 *   a non-null socket pointer
 * @param $second
 *   An integer number of messages
 *
 * @return
 *   if the call was successful
 */
bool ZeroMQ<void*>::SetSendHWM(void *socket, int highWaterMark) {
   return (zmq_setsockopt(socket, ZMQ_SNDHWM, &highWaterMark,
           sizeof (highWaterMark)) == 0);
}

/**
 * Initialize a message object for use in ZMQ
 *
 * @param $first
 *   An uninitailzed object, re-initialization breaks the API
 *
 * @return
 *   If things were successful
 */
bool ZeroMQ<void*>::InitializeMsg(zmq_msg_t& msg) {
   return (zmq_msg_init(&msg) == 0);
}
