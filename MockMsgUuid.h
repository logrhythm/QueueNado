#pragma once
#include "MsgUuid.h"

class MockMsgUuid : public networkMonitor::MsgUuid {
public: 
   MockMsgUuid() : networkMonitor::MsgUuid() {}
   ~MockMsgUuid() {}
};
