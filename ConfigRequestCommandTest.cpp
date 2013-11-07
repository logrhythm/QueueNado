#include "ConfigRequestCommandTest.h"
#include "ConfigRequestCommand.h"
#include "MockConfigRequestCommand.h"
#include "ConfigDefaultsRequest.pb.h"
#include "ConfigDefaults.pb.h"
#include "include/global.h"
#include <boost/lexical_cast.hpp>

TEST_F(ConfigRequestCommandTest, DoesItCompileAndLink) {
   protoMsg::CommandRequest request;
   MockConfigRequestCommand mockDummy(request, nullptr);
   GMockConfigRequestCommand gmockDummy(request, nullptr);
   SUCCEED();
}


TEST_F(ConfigRequestCommandTest, JustSendingAndReceivingProtoFiles) {
#ifdef LR_DEBUG
   mockConf.mCommandQueue = "tcp://127.0.0.1:";
   mockConf.mCommandQueue += boost::lexical_cast<std::string>(rand() % 1000 + 20000);
   MockProcessManagerCommand testProcessor(mockConf);
   EXPECT_TRUE(testProcessor.Initialize());
#endif
}