#pragma once
#include <nanomsg/nn.h>
#include <string>
/*
 * class for encapsulating nanomsg zero copy allocation/deletion
 */
class NanoProtocol {
public:
   enum class Protocol {TCP,IPC};
   NanoProtocol(const std::string& location);
   ~NanoProtocol();
   NanoProtocol& operator=(const NanoProtocol&) = delete;
   NanoProtocol(const NanoProtocol&) = delete;
   NanoProtocol() = delete;
   std::string GetLocation();
private:
   bool mOwner;
   Protocol mProtocol;
   std::string mLocation;
};
