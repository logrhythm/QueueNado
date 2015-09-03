#include <boost/thread.hpp>
#define _OPEN_SYS
#include <sys/stat.h>

#include "Vampire.h"
#include "czmq.h"
#include "g2log.hpp"
#include "Death.h"


/**
 * Construct our Vampire which is a pull in our ZMQ push pull.
 */
Vampire::Vampire(const std::string& location) :
mLocation(location),
mHwm(250),
mBody(NULL),
mContext(NULL),
mLinger(10),
mIOThredCount(1),
mOwnSocket(false) {
}

/**
 * Return thet location we are going to be shot.
 * @return 
 */
std::string Vampire::GetBinding() const {
   return mLocation;
}

/**
 * Get our high water mark.
 * @return 
 */
int Vampire::GetHighWater() {
   return mHwm;
}

/**
 * Set our highwatermark. This must be called before PrepareToBeShot.
 * @param hwm
 */
void Vampire::SetHighWater(const int hwm) {
   mHwm = hwm;
}

/**
 * Set IO thread count. This must be called before PrepareToBeShot.
 * @param count
 */
void Vampire::SetIOThreads(const int count) {
   mIOThredCount = count;
}

/**
 * Set if we own the socket or not, if we do bind to it.
 * @param own
 */
void Vampire::SetOwnSocket(const bool own) {
   mOwnSocket = own;
}

/**
 * Get value for owning the socket.
 * @return bool
 */
bool Vampire::GetOwnSocket() {
   return mOwnSocket;
}

/**
 * Get IO thread count;
 * @param count
 */
int Vampire::GetIOThreads() {
   return mIOThredCount;
}

/**
 * Set the location we are going to be shot at.
 * @param location
 * @return 
 */
bool Vampire::PrepareToBeShot() {
   if (mBody) {
      return true;
   }
   if (!mContext) {
      mContext = zctx_new();
      zctx_set_sndhwm(mContext, GetHighWater());
      zctx_set_rcvhwm(mContext, GetHighWater());// HWM on internal thread communication
      //zctx_set_linger(mContext, mLinger); // linger for a millisecond on close
      zctx_set_iothreads(mContext, GetIOThreads());
   }
   if (!mBody) {
      mBody = zsocket_new(mContext, ZMQ_PULL);
      CZMQToolkit::setHWMAndBuffer(mBody, GetHighWater());
      if (GetOwnSocket()) {
         int result = zsocket_bind(mBody, mLocation.c_str());

         if (result < 0) {
            zsocket_destroy(mContext, mBody);
            mBody = NULL;
            LOG(WARNING) << "Vampire Can't bind : " << result;
            return false;
         }
         setIpcFilePermissions();
         Death::Instance().RegisterDeathEvent(&Death::DeleteIpcFiles, mLocation);
      } else {
         int result = zsocket_connect(mBody, mLocation.c_str());
         if (result < 0) {
            zsocket_destroy(mContext, mBody);
            mBody = NULL;
            LOG(WARNING) << "Vampire Can't connect : " << result;
            return false;
         }
      }
      CZMQToolkit::PrintCurrentHighWater(mBody, "Vampire: body");
   }
   return ((mContext != NULL) && (mBody != NULL));

}

/**
 * Set the file permisions on an IPC socket to 0777
 */
void Vampire::setIpcFilePermissions() {

   mode_t mode = S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP
      | S_IXGRP | S_IROTH | S_IWOTH | S_IXOTH;

   size_t ipcFound = mLocation.find("ipc");
   if (ipcFound != std::string::npos) {
      size_t tmpFound = mLocation.find("/tmp");
      if (tmpFound != std::string::npos) {
         std::string ipcFile = mLocation.substr(tmpFound);
         LOG(INFO) << "Vampire set ipc permissions: " << ipcFile;
         chmod(ipcFile.c_str(), mode);
      }
   }
}

/**
 * Get shot by the rifle.
 * @param bullet
 * @return 
 */
bool Vampire::GetShot(std::string& wound, const int timeout) {
   if (!mBody) {
      LOG(WARNING) << "Socket uninitialized!";
      boost::this_thread::sleep(boost::posix_time::seconds(1));
      return false;
   }
   bool success = false;
   zmsg_t* message = NULL;
   zmq_pollitem_t items [] = {
      { mBody, 0, ZMQ_POLLIN, 0}
   };
   int pollResult = zmq_poll(items, 1, timeout);
   if (pollResult > 0) {
      if (items[0].revents & ZMQ_POLLIN) {
         message = zmsg_recv(mBody);
         if (message && zmsg_size(message) == 1) {
            zframe_t* frame = zmsg_last(message);
            wound.clear();
            wound.append(reinterpret_cast<char*> (zframe_data(frame)), zframe_size(frame));
            success = true;
         } else {
            if (!message) {
               LOG(INFO) << "received null message, time for shutdown.";
            } else {
               LOG(WARNING) << "Received invalid sized message of size: " << zmsg_size(message);
            }
         }
      } else {
         LOG(WARNING) << "Error in zmq_pollin " << GetBinding();
      }

   } else if (pollResult < 0) {
      LOG(WARNING) << "Error on zmq socket receiving " << GetBinding() << ": " << zmq_strerror(zmq_errno());
   } else {
      //socket timed out
   }
   if (message) {
      zmsg_destroy(&message);
   }
   return success;
}

/**
 * Get a pointer from the rifle
 * @param stake
 *   A void* that the caller needs to already know how to cast
 * @return 
 *   If something was found
 */
bool Vampire::GetStake(void*& stake, const int timeout) {
   if (!mBody) {
      LOG(WARNING) << "Socket uninitialized!";
      boost::this_thread::sleep(boost::posix_time::seconds(1));
      return false;
   }
   bool success = false;
   zmsg_t* message = NULL;
   if (zsocket_poll(mBody, timeout)) {
      message = zmsg_recv(mBody);
      if (message && (zmsg_size(message) == 1)) {
         zframe_t* frame = zmsg_pop(message);
         if (frame && zframe_size(frame) != sizeof (void*)) {
            LOG(WARNING) << "Received non-pointer message.";
         } else if(frame) {
            stake = *reinterpret_cast<void**> (zframe_data(frame));
            success = true;
         }
         //always delete frame if it exists
         if (frame) {
            zframe_destroy(&frame);
         }
      } else if (message) {
         LOG(WARNING) << "Received an invalid message";
      }
   }
   if (message) {
      zmsg_destroy(&message);
   }
   if (!success) {
      stake = NULL;
   }
   return success;
}

/**
 * Get a pointer from the rifle
 * @param stake
 *   A void* that the caller needs to already know how to cast
 * @return 
 *   If something was found
 */
bool Vampire::GetStakeNoWait(void*& stake) {
   return GetStake(stake, 0);
}

/**
 * Get a collection of pointers from the rifle
 * @param stakes
 *   A vector of pairs, first being a pointer that the sender gives ownership
 * of, and a has of the data associated with the pointer
 * @return 
 *   If something was found
 */
bool Vampire::GetStakes(std::vector<std::pair<void*, unsigned int> >& stakes,
   const int timeout) {
   if (!mBody) {
      LOG(WARNING) << "Socket uninitialized!";
      boost::this_thread::sleep(boost::posix_time::seconds(1));
      return false;
   }
   bool success = false;
   zmsg_t* message = NULL;
   if (zsocket_poll(mBody, timeout)) {
      message = zmsg_recv(mBody);
      if (message && zmsg_size(message) == 1) {
         zframe_t* frame = zmsg_pop(message);
         if (frame && zframe_size(frame) < (sizeof (std::pair<void*, unsigned int>))) {
            LOG(WARNING) << "Received non-pointer message.";
         } else if (frame) {
            stakes.clear();
            stakes.assign(reinterpret_cast<std::pair<void*, unsigned int>*> (zframe_data(frame)),
               reinterpret_cast<std::pair<void*, unsigned int>*> (zframe_data(frame))
               + (zframe_size(frame) / sizeof (std::pair<void*, unsigned int>)));
            success = true;
         }
         //always delete frame if it exists
         if (frame) {
            zframe_destroy(&frame);
         }
      } else if (!message || (zmsg_size(message) != 1)) {
         LOG(WARNING) << "Received invalid message.";
      }
   }
   if (message) {
      zmsg_destroy(&message);
   }
   if (!success) {
      stakes.clear();
   }
   return success;
}

/**
 * Stake our vampire.
 * @return 
 */
void Vampire::Destroy() {
   if (mContext != NULL) {
      //LOG(DEBUG) << "Vampire: destroying context";
      zsocket_destroy(mContext, mBody);
      zctx_destroy(&mContext);
      //zclock_sleep(mLinger * 2);
      mContext = NULL;
      mBody = NULL;
   }
}

/**
 * Kill our vampire... preferably with a stake to the heart.
 */
Vampire::~Vampire() {
   Destroy();
}
