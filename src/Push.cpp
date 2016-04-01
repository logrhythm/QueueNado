#include "Push.h"
#include <g3log/g3log.hpp>
/**
 * Construct a blocking nano msg push class (no timeout)
 */
Push::Push(const std::string& location, const bool shouldConnect) {
   mProtocolHandler = std::move(std::unique_ptr<NanoProtocol>(
                                   new NanoProtocol(location)));
   mSocket = nn_socket(AF_SP, NN_PUSH);
   if (mSocket < 0) {
      throw std::runtime_error("could not open transport socket" +
                               mProtocolHandler->GetLocation() +
                               " because of error: " +
                               std::string(nn_strerror(errno)));
   }
   int connectResponse = {0};
   std::string connectionType("");
   if (shouldConnect) {
      connectResponse = nn_connect(mSocket,
                                   mProtocolHandler->GetLocation().c_str());
      connectionType = std::string("nn_connect");
   } else {
      connectResponse = nn_bind(mSocket,
                                mProtocolHandler->GetLocation().c_str());
      connectionType = std::string("nn_bind");
   }
   if (connectResponse < 0) {
      throw std::runtime_error("could not connect to endpoint: " +
                               mProtocolHandler->GetLocation() +
                               " because of error: " +
                               std::string(nn_strerror(errno)));
   }
   LOG(INFO) << connectionType
              << " socket at: " << mProtocolHandler->GetLocation();
}
/**
 * Construct non-blocking nano msg push class (timeout)
 */
Push::Push(const std::string& location,
           const int timeoutInMs,
           const bool shouldConnect) {
   if (timeoutInMs <= 0) {
      //throw std::runtime_error("bad timeout value: " + std::to_string(timeoutInMs));
      LOG(FATAL) << "bad timeout value: " << std::to_string(timeoutInMs);
   }
   mProtocolHandler = std::move(std::unique_ptr<NanoProtocol>(
                                   new NanoProtocol(location)));
   mSocket = nn_socket(AF_SP, NN_PUSH);
   if (mSocket < 0) {
      throw std::runtime_error("could not open transport socket" +
                               mProtocolHandler->GetLocation() +
                               " because of error: " +
                               std::string(nn_strerror(errno)));
   }
   nn_setsockopt(mSocket,
                 NN_SOL_SOCKET,
                 NN_SNDTIMEO,
                 &timeoutInMs,
                 sizeof(timeoutInMs));
   int connectResponse = {0};
   std::string connectionType("");
   if (shouldConnect) {
      connectResponse = nn_connect(mSocket,
                                   mProtocolHandler->GetLocation().c_str());
      connectionType = std::string("nn_connect");
   } else {
      connectResponse = nn_bind(mSocket,
                                mProtocolHandler->GetLocation().c_str());
      connectionType = std::string("nn_bind");
   }
   if (connectResponse < 0) {
      throw std::runtime_error("could not connect to endpoint: " +
                               mProtocolHandler->GetLocation() +
                               " because of error: " +
                               std::string(nn_strerror(errno)));
   }
   LOG(INFO) << connectionType
             << " socket at: " << mProtocolHandler->GetLocation()
             << " with timeout: " << timeoutInMs;
}
/**
 * Construct a specific push queue
 */
Push::Push(const std::string& location,
           const int timeoutInMs,
           const int bufferSize,
           const bool shouldConnect) {
   mProtocolHandler = std::move(std::unique_ptr<NanoProtocol>(
                                   new NanoProtocol(location)));
   mSocket = nn_socket(AF_SP, NN_PUSH);
   if (mSocket < 0) {
      throw std::runtime_error("could not open transport socket" +
                               mProtocolHandler->GetLocation() +
                               " because of error: " +
                               std::string(nn_strerror(errno)));
   }
   if (timeoutInMs > 0) {
      nn_setsockopt(mSocket,
                    NN_SOL_SOCKET,
                    NN_SNDTIMEO,
                    &timeoutInMs,
                    sizeof(timeoutInMs));
   } else {
      LOG(INFO) << " zero or negative timeout requested, blocking socket";
   }
   nn_setsockopt(mSocket,
                 NN_SOL_SOCKET,
                 NN_SNDBUF,
                 &bufferSize,
                 sizeof(bufferSize));
   int connectResponse = {0};
   std::string connectionType("");
   if (shouldConnect) {
      connectResponse = nn_connect(mSocket,
                                   mProtocolHandler->GetLocation().c_str());
      connectionType = std::string("nn_connect");
   } else {
      connectResponse = nn_bind(mSocket,
                                mProtocolHandler->GetLocation().c_str());
      connectionType = std::string("nn_bind");
   }
   if (connectResponse < 0) {
      throw std::runtime_error("could not connect to endpoint: " +
                               mProtocolHandler->GetLocation() +
                               " because of error: " +
                               std::string(nn_strerror(errno)));
   }
   LOG(INFO) << connectionType
             << " socket at: " << mProtocolHandler->GetLocation()
             << " with timeout: " << timeoutInMs;
}
/**
 * Return the location the socket is bound to
 * @return
 */
std::string Push::GetBinding() const {
   return mProtocolHandler->GetLocation();
}
/**
 * Fires a basic string
 */
void Push::Send(const std::string& data, bool dontWait) {
   NanoMsg msg(data);
   SendMessage(msg, dontWait);
}
/**
 * Fires a void pointer
 */
void Push::Send(void *data, bool dontWait) {
   //LOG(INFO) << "sending pointer";
   NanoMsg msg(data);
   SendMessage(msg, dontWait);
}
/**
 * Fires a packet hash vector
 */
void Push::Send(const std::vector<std::pair<void*, unsigned int>>& data,
                bool dontWait) {
   NanoMsg msg(data);
   //LOG(INFO) << "Sending vector with starting pointer: " << &data[0];
   //Send(const_cast<std::vector<std::pair<void*, unsigned int> >*>(&data));
   SendMessage(msg, dontWait);
}
/**
 * private function to send a message using zero copy api
 */
void Push::SendMessage(NanoMsg& msg, bool dontWait) {
   //LOG(INFO) << "sending message";
   auto num_bytes_sent = nn_send(mSocket,
                                 msg.GetBufferReference(),
                                 NN_MSG,
                                 dontWait? NN_DONTWAIT : 0);
   if (num_bytes_sent < 0) {
      auto error = nn_errno ();
      if (error == ETIMEDOUT) {
         throw std::runtime_error("timed out sending message");
      } else if (error == EFAULT || error == EBADF || error == ENOTSUP) {
         // really bad issue,
         // trying to send a nullptr,
         // bad socket,
         // or operation not supported
         LOG(FATAL) << "fatal nanomsg pull error: "
                    << std::string(nn_strerror(errno));
      } else if (error == EFSM || error == EAGAIN) {
         // failed for now, but you can try again
         throw std::runtime_error("try again");
      } else if (error == EINTR || error ==  ETERM) {
         // shutting down, we don't care stop trying to run
      }
   }
   // mark message as sent so it does not have to deallocate itself
   msg.SetSent();
}
/**
 * Destructor that closes socket
 */
Push::~Push() {
   nn_close(mSocket);
}
