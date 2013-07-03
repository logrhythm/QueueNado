#include "Shotgun.h"
#include "g2log.hpp"
#define _OPEN_SYS
#include <sys/stat.h>
#include "czmq.h"
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

      LOG(WARNING) << "bound socket rc:" << rc << " : location: " << location;
      LOG(WARNING) << zmq_strerror(zmq_errno());

      throw std::string("Failed to connect to bind socket");
   }
   setIpcFilePermissions(location);
}

/**
 * Set the file permisions on an IPC socket to 0777
 */
void Shotgun::setIpcFilePermissions(const std::string& location) {

   mode_t mode = S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP
           | S_IXGRP | S_IROTH | S_IWOTH | S_IXOTH;

   size_t ipcFound = location.find("ipc");
   if (ipcFound != std::string::npos) {
      size_t tmpFound = location.find("/tmp");
      if (tmpFound != std::string::npos) {
         std::string ipcFile = location.substr(tmpFound);
         LOG(INFO) << "Shotgun set ipc permissions: " << ipcFile;
         chmod(ipcFile.c_str(), mode);
      }
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

   zmsg_t* msg = zmsg_new();
   zmsg_add(msg, key);
   for (auto it = bullets.begin(); it != bullets.end(); it++) {
      zframe_t* body = zframe_new(it->c_str(), it->size());
      zmsg_add(msg, body);
   }

   if (zmsg_send(&msg, mGun) != 0) {
      if (msg) {
         zmsg_destroy(&msg);
      }
   }

}

/**
 * Cleanup our socket and context.
 */
Shotgun::~Shotgun() {
   zsocket_destroy(mCtx, mGun);
   zctx_destroy(&mCtx);
}

