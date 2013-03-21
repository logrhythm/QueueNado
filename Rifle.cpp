#include "Rifle.h"
#include "g2logworker.hpp"
#include "g2log.hpp"

/**
 * Construct our Rifle which is a push in our ZMQ push pull.
 */
Rifle::Rifle(const std::string& location) :
mContext(NULL),
mChamber(NULL),
mOwnSocket(true),
mLocation(location),
mHwm(500),
mLinger(10),
mIOThredCount(1) {
}

/**
 * Return thet location we are going to be shot.
 * @return 
 */
std::string Rifle::GetBinding() const {
   return mLocation;
}

/**
 * Get our high water mark.
 * @return 
 */
int Rifle::GetHighWater() {
   return mHwm;
}

/**
 * Set our highwatermark. This must be called before Aim.
 * @param hwm
 */
void Rifle::SetHighWater(const int hwm) {
   mHwm = hwm;
}

/**
 * Set IO thread count. This must be called before Aim.
 * @param count
 */
void Rifle::SetIOThreads(const int count) {
   mIOThredCount = count;
}

/**
 * Get IO thread count;
 * @param count
 */
int Rifle::GetIOThreads() {
   return mIOThredCount;
}

/**
 * Set if we own the socket or not, if we do bind to it.
 * @param own
 */
void Rifle::SetOwnSocket(const bool own) {
   mOwnSocket = own;
}

/**
 * Get value for owning the socket.
 * @return bool
 */
bool Rifle::GetOwnSocket() {
   return mOwnSocket;
}

/**
 * Set the location we want to shoot at.
 * @param location
 * @return 
 */
bool Rifle::Aim() {
   if (mChamber) {
      return true;
   }
   if (!mContext) {
      mContext = zctx_new();
      zctx_set_hwm(mContext, GetHighWater()); // HWM on internal thread communication
      //zctx_set_linger(mContext, mLinger); // linger for a millisecond on close
      zctx_set_iothreads(mContext, mIOThredCount);
   }
   if (!mChamber) {
      mChamber = zsocket_new(mContext, ZMQ_PUSH);
      CZMQToolkit::setHWMAndBuffer(mChamber, GetHighWater());
      if (GetOwnSocket()) {
         int result = zsocket_bind(mChamber, mLocation.c_str());

         if (result < 0) {
            printf("Can't bind : %d\n", result);
            zsocket_destroy(mContext, mChamber);
            mChamber = NULL;
            return false;
         }
      } else {
         int result = zsocket_connect(mChamber, mLocation.c_str());
         if (result < 0) {
            printf("Can't connect : %d\n", result);
            zsocket_destroy(mContext, mChamber);
            mChamber = NULL;
            return false;
         }
      }
      //CZMQToolkit::PrintCurrentHighWater(mChamber, "Rifle: chamber");
   }
   return ((mContext != NULL) && (mChamber != NULL));

}

/**
 * Shoot a buillet / message to the Vampires / pull.
 *
 * @param bullet
 *
 * @param waitToFire in milliseconds
 *
 * @return 
 */
bool Rifle::Fire(const std::string& bullet,const int waitToFire) {
   //LOG(DEBUG) << "RifleFire";
   if (!mChamber) {
      return false;
   }
   if (bullet.empty()) {
      LOG(WARNING) << "Tried to send empty packet";
      return false;
   }
   zmq_pollitem_t items [] = {
      { mChamber, 0, ZMQ_POLLOUT, 0}
   };

   if (zmq_poll(items, 1, waitToFire) > 0) {
      if (items[0].revents & ZMQ_POLLOUT) {
         zmsg_t* message = zmsg_new();
         zmsg_addmem(message, &(bullet[0]), bullet.size());
         return CZMQToolkit::SendExistingMessage(message, mChamber);
      } else {
         LOGF(WARNING, "Error on Zmq socket send: %s", zmq_strerror(zmq_errno()));
         return false;
      }
   } else {
      return false;
   }
}
bool Rifle::FireZeroCopy( std::string* zero, const size_t size, void (*FreeFunction)(void*,void*), const int waitToFire ){
   if (!mChamber) {
      return false;
   }
   if (size == 0) {
      LOG(WARNING) << "Tried to send empty packet";
      return false;
   }
   zmq_pollitem_t items [] = {
      { mChamber, 0, ZMQ_POLLOUT, 0}
   };

   if (zmq_poll(items, 1, waitToFire) > 0) {
      if (items[0].revents & ZMQ_POLLOUT) {
         zmsg_t* message = zmsg_new();
         zframe_t* frame = zframe_new_zero_copy(&((*zero)[0]), size, FreeFunction, zero);
         zmsg_add(message, frame);
         return CZMQToolkit::SendExistingMessage(message, mChamber);
      } else {
         LOGF(WARNING, "Error on Zmq socket send: %s", zmq_strerror(zmq_errno()));
         return false;
      }
   } else {
      return false;
   }
}
/**
 * Shoot a pointer / message to the Vampires / pull.
 * @param stake
 * @return 
 */
bool Rifle::FireStake(const void* stake, const int waitToFire) {
   //LOG(DEBUG) << "RifleFireStake";
   if (!mChamber) {
      return false;
   }
   if (stake == NULL) {
      LOG(WARNING) << "Tried to send empty packet";
      return false;
   }
   zmq_pollitem_t items [] = {
      { mChamber, 0, ZMQ_POLLOUT, 0}
   };

   if (zmq_poll(items, 1, waitToFire) > 0) {
      if (items[0].revents & ZMQ_POLLOUT) {
         zmsg_t* message = zmsg_new();
         zmsg_addmem(message, &(stake), sizeof (void*));
         return CZMQToolkit::SendExistingMessage(message, mChamber);
      } else {

         LOGF(WARNING, "Error in zmq_pollout in %s: %s", GetBinding().c_str(), zmq_strerror(zmq_errno()));
         return false;
      }
   } else {
      //LOGF(WARNING, "timeout in zmq_pollout in %s", GetBinding().c_str());
      return false;
   }
}

/**
 * Shoot a vector of pointers and some sort of hash message to the Vampires / pull.
 * @param stakes
 *   A vector of pairs, first being a pointer that the sender gives ownership
 * of, and a has of the data associated with the pointer
 * @return 
 *   false if something went wrong
 */
bool Rifle::FireStakes(const std::vector<std::pair<void*, unsigned int> >
        & stakes, const int waitToFire) {
   //LOG(DEBUG) << "RifleFireStakes";
   if (!mChamber) {
      return false;
   }
   if (stakes.empty()) {
      LOG(WARNING) << "Tried to send nothing";
      return false;
   }
   zmq_pollitem_t items [] = {
      { mChamber, 0, ZMQ_POLLOUT, 0}
   };

   if (zmq_poll(items, 1, waitToFire) > 0) {
      if (items[0].revents & ZMQ_POLLOUT) {
         zmsg_t* message = zmsg_new();
         zmsg_addmem(message, &(stakes[0]),
                 stakes.size() * (sizeof (std::pair<void*, unsigned int>)));
         return CZMQToolkit::SendExistingMessage(message, mChamber);
      } else {
         LOGF(WARNING, "Error in zmq_pollout %s", GetBinding().c_str());
         return false;
      }
   } else {
      LOGF(WARNING, "timeout in zmq_pollout %s", GetBinding().c_str());
      return false;
   }
}

/**
 * Destroy the gun.
 */
void Rifle::Destroy() {
   if (mContext != NULL) {
      //LOG(DEBUG) << "Rifle: destroying context";
      zsocket_destroy(mContext, mChamber);
      zctx_destroy(&mContext);
      //zclock_sleep(mLinger * 2);
      mChamber = NULL;
      mContext = NULL;
   }
}

Rifle::~Rifle() {
   Destroy();
}
