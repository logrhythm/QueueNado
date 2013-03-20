#include "Shotgun.h"
#include "g2logworker.hpp"
#include "g2log.hpp"

/**
 * Shotgun class is a ZeroMQ Publisher.
 */
Shotgun::Shotgun() {
   mCtx = zctx_new();
   assert(mCtx);
   mGun = zsocket_new(mCtx, ZMQ_PUB);
}

/**
 * Where to fire our messages.
 * @param location
 */
void Shotgun::Aim(const std::string& location) {
   zsocket_set_sndhwm(mGun, 32 * 1024);
   int rc = zsocket_bind(mGun, location.c_str());
   if (rc == -1) {
      
      LOGF(WARNING, "bound socket rc:%d : location: %s\n %s", rc, location.c_str(),zmq_strerror(zmq_errno()));
      throw std::string("Failed to connect to bind socket");
   }
}

/**
 * Fire our shotgun, hopefully we hit something.
 * @param msg
 */
void Shotgun::Fire(const std::string& bullet) {
   std::vector<std::string> bullets;
   bullets.push_back(bullet);
   Fire(bullets);
}
/**
 * Fire our shotgun, hopefully we hit something.
 * @param msg
 */
void Shotgun::Fire(const std::vector<std::string>& bullets) {
   zframe_t* key = zframe_new("key", 0);
   
   int rc;
   zmsg_t* msg = zmsg_new();
   zmsg_add(msg, key);
   for (std::vector<std::string>::const_iterator it = bullets.begin() ;
           it != bullets.end() ; it++) {
      zframe_t* body = zframe_new(it->c_str(), it->size());
      zmsg_add(msg, body);
   }
   
   zmsg_send(&msg, mGun);
   zmsg_destroy(&msg);
}
/**
 * Cleanup our socket and context.
 */
Shotgun::~Shotgun() {
   zsocket_destroy(mCtx, mGun);
   zctx_destroy(&mCtx);
}

