#include "EthInfoTest.h"

#include <sys/types.h>
#include <ifaddrs.h>
#include <memory>


#ifdef LR_DEBUG
#include "MockEthInfo.h"
TEST_F(EthInfoTest, ConstructAndInitialize) {
   EthInfo ethInfo;
   EXPECT_FALSE(ethInfo.IsValid());
   std::unique_ptr<EthInfo> pEthInfo(new EthInfo);
   EXPECT_TRUE(ethInfo.GetAvaliableInterfaces().empty());
   EXPECT_TRUE(ethInfo.Initialize());
   EXPECT_TRUE(ethInfo.IsValid());
   EXPECT_TRUE(pEthInfo->Initialize());
   EXPECT_FALSE(ethInfo.GetAvaliableInterfaces().empty());
}

TEST_F(EthInfoTest, GetIFAddrs) {
   MockEthInfo ethInfo;
   ethInfo.mFakeGetIFAddrs = true;
   EXPECT_FALSE(ethInfo.Initialize());
   EXPECT_FALSE(ethInfo.IsValid());
   EXPECT_TRUE(ethInfo.GetAvaliableInterfaces().empty());
   ethInfo.mGetIFAddrsRetVal = 0;
   EXPECT_FALSE(ethInfo.Initialize());
   EXPECT_FALSE(ethInfo.IsValid());
   EXPECT_TRUE(ethInfo.GetAvaliableInterfaces().empty());  
   ethInfo.mifaddr = reinterpret_cast<struct ifaddrs*>(malloc(sizeof(struct ifaddrs)));
   ethInfo.mifaddr->ifa_next = NULL;
   
   std::string fakeName("eth1");
   ethInfo.mifaddr->ifa_name = reinterpret_cast<char*>(malloc(fakeName.size()));
   strncpy(ethInfo.mifaddr->ifa_name,fakeName.c_str(),fakeName.size());
   EXPECT_TRUE(ethInfo.Initialize());
   EXPECT_TRUE(ethInfo.IsValid());
   EXPECT_EQ(1,ethInfo.GetAvaliableInterfaces().size());
   EXPECT_EQ("eth1",*(ethInfo.GetAvaliableInterfaces().begin()));
   ethInfo.mifaddr->ifa_next = reinterpret_cast<struct ifaddrs*>(malloc(sizeof(struct ifaddrs)));
   ASSERT_FALSE(NULL == ethInfo.mifaddr->ifa_next);
   ethInfo.mifaddr->ifa_next->ifa_next = NULL;
   
   std::string fakeName2("eth2");
   ethInfo.mifaddr->ifa_next->ifa_name = reinterpret_cast<char*>(malloc(fakeName2.size()));
   ASSERT_FALSE(NULL == ethInfo.mifaddr->ifa_name);
   strncpy(ethInfo.mifaddr->ifa_next->ifa_name,fakeName2.c_str(),fakeName2.size());
   EXPECT_TRUE(ethInfo.Initialize());
   EXPECT_EQ(2,ethInfo.GetAvaliableInterfaces().size());
   free(ethInfo.mifaddr->ifa_next->ifa_name);
   free(ethInfo.mifaddr->ifa_name);
   free(ethInfo.mifaddr->ifa_next);
   free(ethInfo.mifaddr);
}

#else 
TEST_F(EthInfoTest, NullTest) {
   EXPECT_TRUE(true);
}
#endif