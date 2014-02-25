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
   EXPECT_FALSE(ethInfo.Initialize()); // aborts before bond0 is added
   EXPECT_FALSE(ethInfo.IsValid());
   EXPECT_TRUE(ethInfo.GetAvaliableInterfaces().empty());  
   ethInfo.mifaddr = reinterpret_cast<struct ifaddrs*>(malloc(sizeof(struct ifaddrs)));
   ethInfo.mifaddr->ifa_next = NULL;
   
   std::string fakeName("eth1");
   ethInfo.mifaddr->ifa_name = reinterpret_cast<char*>(malloc(fakeName.size()));
   strncpy(ethInfo.mifaddr->ifa_name,fakeName.c_str(),fakeName.size());
   EXPECT_TRUE(ethInfo.Initialize());
   EXPECT_TRUE(ethInfo.IsValid());
   EXPECT_EQ(2,ethInfo.GetAvaliableInterfaces().size());
   EXPECT_TRUE(ethInfo.GetAvaliableInterfaces().count("eth1") != 0);
   ethInfo.mifaddr->ifa_next = reinterpret_cast<struct ifaddrs*>(malloc(sizeof(struct ifaddrs)));
   ASSERT_FALSE(NULL == ethInfo.mifaddr->ifa_next);
   ethInfo.mifaddr->ifa_next->ifa_next = NULL;
   
   std::string fakeName2("eth2");
   ethInfo.mifaddr->ifa_next->ifa_name = reinterpret_cast<char*>(malloc(fakeName2.size()));
   ASSERT_FALSE(NULL == ethInfo.mifaddr->ifa_name);
   strncpy(ethInfo.mifaddr->ifa_next->ifa_name,fakeName2.c_str(),fakeName2.size());
   EXPECT_TRUE(ethInfo.Initialize());
   EXPECT_EQ(3,ethInfo.GetAvaliableInterfaces().size());
   free(ethInfo.mifaddr->ifa_next->ifa_name);
   free(ethInfo.mifaddr->ifa_name);
   free(ethInfo.mifaddr->ifa_next);
   free(ethInfo.mifaddr);
}

TEST_F(EthInfoTest, GetIFAddrsBond0) {
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
   
   std::string fakeName("bond0");
   ethInfo.mifaddr->ifa_name = reinterpret_cast<char*>(malloc(fakeName.size()));
   strncpy(ethInfo.mifaddr->ifa_name,fakeName.c_str(),fakeName.size());
   EXPECT_TRUE(ethInfo.Initialize());
   EXPECT_TRUE(ethInfo.IsValid());
   EXPECT_EQ(1,ethInfo.GetAvaliableInterfaces().size());
   EXPECT_EQ("bond0",*(ethInfo.GetAvaliableInterfaces().begin()));
  
   free(ethInfo.mifaddr->ifa_name);
   free(ethInfo.mifaddr);
}


// It will ALWAYS have at least bond0
TEST_F(EthInfoTest, SystemGetIFAddrsBond0) {
   EthInfo ethInfo;
   EXPECT_TRUE(ethInfo.Initialize());
   EXPECT_TRUE(ethInfo.IsValid());
   EXPECT_FALSE(ethInfo.GetAvaliableInterfaces().empty());
 
   std::string bondName("bond0");
   EXPECT_TRUE(ethInfo.GetAvaliableInterfaces().count(bondName) != 0);
   }


#else 
TEST_F(EthInfoTest, NullTest) {
   EXPECT_TRUE(true);
}
#endif