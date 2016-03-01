#include "NanoProtocol.h"
#include <stdio.h>
#include <FileIO.h>
#include <g3log/g3log.hpp>
NanoProtocol::NanoProtocol(const std::string& location) {
   if (location.find("ipc") != std::string::npos) {
      mProtocol = Protocol::IPC;
      mLocation = location.substr(location.find("ipc://") + 6);
      mOwner = !FileIO::DoesFileExist(mLocation);
      LOG_IF(DEBUG, mOwner) << "i own the location: " << location;
   } else if (location.find("tcp") != std::string::npos){
      mProtocol = Protocol::TCP;
      mOwner = false;
      mLocation = location.substr(location.find("tcp://") + 6);
      LOG_IF(DEBUG, mOwner) << "i own the location: " << location;
   }
}
NanoProtocol::~NanoProtocol() {
   if (mProtocol == Protocol::IPC && mOwner) {
      remove(mLocation.c_str());
      LOG(DEBUG) << "removed ipc file at: " << mLocation;
   }
}
std::string NanoProtocol::GetLocation() {
   std::string toReturn = {};
   if (mProtocol == Protocol::IPC) {
      toReturn = "ipc://" + mLocation;
   } else if (mProtocol == Protocol::TCP) {
      toReturn = "tcp://" + mLocation;
   }
   return toReturn;
}
