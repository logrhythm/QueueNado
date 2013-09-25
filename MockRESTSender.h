#pragma once

#include "RESTSender.h"

class MockRESTSender : public RESTSender {
public:
   explicit MockRESTSender(BoomStick& transport) : RESTSender(transport) {}
   virtual ~MockRESTSender(){}
   bool Send(const std::string& command, std::string& reply) {
      return RESTSender::Send(command,reply);
   }
   
};
