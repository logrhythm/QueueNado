
#pragma once
#include "ConfigRequestCommand.h"
#include "include/global.h"
#include "gmock/gmock.h"


class MockConfigRequestCommand : public ConfigRequestCommand {
public:
   MockConfigRequestCommand(){}
   virtual ~MockConfigRequestCommand(){}
};

class GMockConfigRequestCommand : public MockConfigRequestCommand {
   public:
      GMockConfigRequestCommand() = default;
      virtual ~GMockConfigRequestCommand() = default;
};
