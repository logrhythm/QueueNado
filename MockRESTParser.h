#pragma once

#include "RESTParser.h"

class MockRESTParser : public RESTParser {
public:
   explicit MockRESTParser(BoomStick& transport) : RESTParser(transport) {}
   virtual ~MockRESTParser(){}
   bool Send(const std::string& command, std::string& reply) {
      return RESTParser::Send(command,reply);
   }
   
};
