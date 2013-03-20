#include "CZMQToolkit.h"
#include "g2logworker.hpp"
#include "g2log.hpp"

/**
 * This function does nothing, is necessary to do a zero copy in ZMQ
 * @param data
 * @param hint
 */
void nullfree(void* data, void*hint) {

}

/**
 * Simple method to set all 4 variables needed to set our buffers and high water mark.
 * @param socket
 * @param size
 */
void CZMQToolkit::setHWMAndBuffer(void* socket, const int size) {

   //   zmq_setsockopt(socket, ZMQ_SNDHWM, &size,
   //      sizeof (size));
   //   zmq_setsockopt(socket, ZMQ_RCVHWM, &size,
   //      sizeof (size));
   zsocket_set_rcvhwm(socket, size);
   //      zsocket_set_rcvbuf(socket, size);
   zsocket_set_sndhwm(socket, size);
   //   zsocket_set_sndbuf(socket, size);
}

/**
 * Print the current HWM for the given socket.
 * @param socket
 * @param name
 */
void CZMQToolkit::PrintCurrentHighWater(void* socket, const std::string& name) {
   int rcvbuf = zsocket_rcvbuf(socket);
   int rcvhwm = zsocket_rcvhwm(socket);
   int sndbuf = zsocket_sndbuf(socket);
   int sndhwm = zsocket_sndhwm(socket);
   std::string tmpName = name;
   LOGF(DEBUG, "%s: rcvbuf:%d rcvhwm:%d sndbuf:%d sndhwm:%d", tmpName.c_str(), rcvbuf, rcvhwm, sndbuf, sndhwm);
}

/**
 * Validates a message, if invalid the caller should wind down and exit
 * 
 * @param message
 *   A pointer that was the result of a zmsg_recv
 * @return 
 *   If the message is valid
 */
bool CZMQToolkit::IsValidMessage(zmsg_t* message) {
   if (!message) {
      return false;
   } else if (zmsg_size(message) == ZMQ_KILL_MESSAGE) {
      return false;
   }
   return true;
}

/**
 * Send a message that will tell the receiver thread to shutdown
 * 
 * @param socket
 *   A czmq pipe
 */
void CZMQToolkit::SendShutdownMessage(void* socket) {
   if (!socket) {
      LOG(WARNING) << "Invalid socket!";
      return;
   }
   zmsg_t* message = zmsg_new();
   for (int i = 0; i < ZMQ_KILL_MESSAGE; i++) {
      zmsg_addmem(message, "", 0);
   }
   zmsg_send(&message, socket);
}

/**
 * Read a message from the socket that contains a size_t variable
 * 
 * Will destroy the message that it found, assumes last frame is the one of 
 *   interest
 * 
 * @param socket
 *   An open socket
 * @param value
 *   The return value
 * @return 
 *   False if an error occurred
 */
bool CZMQToolkit::GetSizeTFromSocket(void* socket, size_t& value) {
   if (!socket) {
      return false;
   }
   zmsg_t* numberReply = zmsg_recv(socket);

   if (!CZMQToolkit::IsValidMessage(numberReply)) {
      LOG(WARNING) << "Invalid Message!";
      return false;
   }
   zframe_t* frame = zmsg_last(numberReply);
   if (zframe_size(frame) != sizeof (size_t)) {
      zmsg_destroy(&numberReply);
      LOGF(WARNING, "Invalid Message, expected size:%lu :: actual size:%lu", sizeof (size_t), zframe_size(frame));
      return false;
   }
   value = *reinterpret_cast<size_t*> (zframe_data(frame));
   zmsg_destroy(&numberReply);

   return true;
}

/**
 * Send an empty message
 * 
 * Typically used to request data from another thread
 * 
 * @param socket
 *   An open socket
 * @return 
 *   If the call was successful
 */
bool CZMQToolkit::SendBlankMessage(void* socket) {
   if (!socket) {
      return false;
   }
   zmsg_t* blankRequest = zmsg_new();
   zmsg_addmem(blankRequest, "", 0);
   if (zmsg_send(&blankRequest, socket) != 0) {
      int err = zmq_errno();
      LOGF(WARNING, "Failed on send %s", zmq_strerror(err));

      return false;
   }
   return true;
}

/**
 * Read from a socket and push the message back to the sender
 * 
 * @param socket
 * @return 
 *   If everything worked
 */
bool CZMQToolkit::SocketFIFO(void* socket) {
   if (!socket) {
      return false;
   }
   zmsg_t* bullet = zmsg_recv(socket);

   if (!CZMQToolkit::IsValidMessage(bullet)) {
      LOG(WARNING) << "Invalid Message!";
      return false;
   }
   if (zmsg_size(bullet) != 1) {
      LOGF(WARNING, "%d frames in overflow", zmsg_size(bullet));
      zmsg_destroy(&bullet);
      return false;
   }
   if (zmsg_send(&bullet, socket) != 0) {
      int err = zmq_errno();
      LOGF(WARNING, "Failed on send %s", zmq_strerror(err));
      return false;
   }
   return true;
}

/**
 * Send a message that already exists to a socket
 * 
 * @param bullet
 *   A valid message
 * @param socket
 *   An open socket
 * @return 
 *   If the call succeeded. 
 */
bool CZMQToolkit::SendExistingMessage(zmsg_t*& message, void* socket) {

   if (!socket || !message) {
      return false;
   }
   if (zmsg_send(&message, socket) != 0) {
      int err = zmq_errno();
      LOGF(WARNING, "Failed on send %s", zmq_strerror(err));

      return false;
   }
   return true;
}

/**
 * Grab a message from a socket and throw it away
 * 
 * @param socket
 *   A valid socket
 * @return 
 *   if the call succeeded
 */
bool CZMQToolkit::PopAndDiscardMessage(void* socket) {
   if (!socket) {
      return false;
   }
   zmsg_t* tossMe = zmsg_recv(socket);

   if (!CZMQToolkit::IsValidMessage(tossMe)) {
      LOG(WARNING) << "Invalid Message!";
      return false;
   }
   zmsg_destroy(&tossMe);

   return true;
}

/**
 * Send a size_t to a socket
 * 
 * @param socket
 *   An open socket
 * @param size
 *   A value
 * @return 
 *   If the call completed
 */
bool CZMQToolkit::SendSizeTToSocket(void* socket, const size_t size) {
   if (!socket) {
      return false;
   }
   zmsg_t* numberRequest = zmsg_new();
   zmsg_addmem(numberRequest, &size, sizeof (size));
   if (zmsg_send(&numberRequest, socket) != 0) {
      int err = zmq_errno();
      LOGF(WARNING, "Failed on send %s", zmq_strerror(err));

      return false;
   }
   return true;

}

/**
 * Pass the message waiting on one socket to another
 * 
 * @param sourceSocket
 *   A socket with a message pending
 * @param destSocket
 *   A socket to sent to
 * @return 
 *   if the send was successful
 */
bool CZMQToolkit::PassMessageAlong(void* sourceSocket, void* destSocket) {
   if (!sourceSocket || !destSocket) {
      LOG(WARNING) << "Invalid Socket!";
      return false;
   }
   //   printf("r msg\n");
   zmsg_t* message = zmsg_recv(sourceSocket);
   //   printf("s msg\n");
   if (!CZMQToolkit::IsValidMessage(message)) {
      LOG(WARNING) << "Invalid Message!";
      return false;
   } else if (zmsg_send(&message, destSocket) != 0) {
      int err = zmq_errno();
      LOGF(WARNING, "Failed on send %s", zmq_strerror(err));
      return false;
   }
   //   printf("done \n");
   return true;
}

/**
 * Grabs a message from a socket, that is assumed to have a pointer to an 
 *   existing block of memory that is a std::string.  Take the data from
 *   that string and send it to the other socket
 * 
 * @param sourceSocket
 *   The socket with a message waiting
 * @param destSocket
 *   Where the data will be sent
 * @return 
 *   If the call succeeded 
 */
bool CZMQToolkit::SendStringContentsToSocket(void* sourceSocket, void* destSocket) {
   if (!sourceSocket || !destSocket) {
      LOG(WARNING) << "Invalid Socket!";
      return false;
   }
   zmsg_t* message = zmsg_recv(sourceSocket);
   if (!CZMQToolkit::IsValidMessage(message)) {
      LOG(WARNING) << "Invalid Message!";
      return false;
   }
   zframe_t* frame = zmsg_last(message);
   std::string* slug = *(reinterpret_cast<std::string**> (zframe_data(frame)));
   zmsg_destroy(&message);
   zmsg_t* shot = zmsg_new();
   zmsg_pushmem(shot, "", 0);
   frame = zframe_new_zero_copy(&(*slug)[0], slug->size(), nullfree, NULL);
   zmsg_add(shot, frame);
   if (zmsg_send(&shot, destSocket) != 0) {
      int err = zmq_errno();
      LOGF(WARNING, "Failed on send %s", zmq_strerror(err));
      return false;
   }
   return true;
}

/**
 * Read a multipart message from the source, send the first part to one
 *   destination, and the rest to another
 * 
 * @param sourceSocket
 *   A socket with a message awaiting
 * @param firstDestination
 *   Where a new message that pops from the source message will go
 * @param secondDestination
 *   Where the rest of the message will go
 * @return 
 *   If the call succeeded
 */
bool CZMQToolkit::ForkPartsOfMessageTwoDirections(void* sourceSocket,
   void* firstDestination, void* secondDestination) {
   if (!sourceSocket || !firstDestination || !secondDestination) {
      LOG(WARNING) << "Invalid socket!";
      return false;
   }
   zmsg_t* shot = zmsg_recv(sourceSocket);
   if (!CZMQToolkit::IsValidMessage(shot)) {
      LOG(WARNING) << "Invalid Message!";
      return false;
   }
   zframe_t* recoil = zmsg_pop(shot);
   zmsg_t* splatter = zmsg_new();
   zmsg_push(splatter, recoil);
   if (zmsg_send(&splatter, firstDestination) != 0) {
      int err = zmq_errno();
      LOGF(WARNING, "Failed on send ", zmq_strerror(err));
      return false;
   }
   if (zmsg_send(&shot, secondDestination) != 0) {
      int err = zmq_errno();
      LOGF(WARNING, "Failed on send ", zmq_strerror(err));
      return false;
   }
   return true;
}

/**
 * Pop the message and turn it into a new std::string*
 * 
 * @param message
 *   A message that contains some characters in it
 */
std::string* CZMQToolkit::GetStringFromMessage(zmsg_t*& message) {
   if (!message) {
      return NULL;
   }
   zframe_t* frame = zmsg_pop(message);
   if (!frame) {
      return NULL;
   }
   std::string* slug = new std::string;
   slug->insert(0, reinterpret_cast<const char*> (zframe_data(frame)), zframe_size(frame));
   zframe_destroy(&frame);
   return slug;
}

/**
 * Sends the given message on the socket containing the pointer slug along with
 *   a hash of the data
 * 
 * @param socket
 *   The target socket
 * @param message
 *   An existing and empty zmsg_t
 * @param slug
 *   A pointer to a string of data
 * @return 
 *   If the call succeeded
 */
bool CZMQToolkit::SendStringWithHash(void* socket, zmsg_t*& message,const  std::string* slug) {
   if (!socket || !message || !slug) {
      LOG(WARNING) << "Socket || message || slug!";
      return false;
   }
   uLong hash;
   CZMQToolkit::GetHash(slug, hash);
   zmsg_pushmem(message, &hash, sizeof (hash));
   zframe_t* msgFrame = zframe_new(&slug, sizeof (slug));
   zmsg_add(message, msgFrame);
   if (zmsg_send(&message, socket) != 0) {
      int err = zmq_errno();
      LOGF(WARNING, "Failed on send %s", zmq_strerror(err));
      return false;
   }
   return true;
}
