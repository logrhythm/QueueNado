#pragma once
#include <string>
class BoomStick {
public:
   explicit BoomStick(const std::string& binding) {}
   virtual ~BoomStick(){}
   
   virtual bool Initialize() { return true; }
   virtual std::string Send(const std::string& command) {
      return "{\"ok\":true,\"acknowledged\":true}";
   }
};
