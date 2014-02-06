#include <tuple>
#include "gtest/gtest.h"
#include "Range.h"
#include "PcapDiskUsage.h"
#include "Conf.h"
#include "include/global.h"
#include "ConfSlave.h"
#include "Conf.h"
#include "ProtoDefaults.h"
#include "MockProtoDefaults.h"
#include "PcapDiskUsage.h"

TEST(TestProtoDefaults, GetConfDefaultsOK) {
   ProtoDefaults getDefaults{{}};
   auto defaults1 = getDefaults.GetConfDefaults(protoMsg::ConfType_Type_BASE);
   EXPECT_TRUE(defaults1.size() > 0);
   ASSERT_TRUE(defaults1.find("dpiThreads") != defaults1.end());
   
   auto element = defaults1.find("dpiThreads")->second;
   EXPECT_TRUE((*std::get<ProtoDefaults::indexRange>(element)).Validate("4")); //4 DPI Threads
}

TEST(TestProtoDefaults, GetRangeOK){
   auto conf = ConfSlave::Instance().GetConf();
   auto pcapLocations = conf.GetPcapCaptureLocations();
   ProtoDefaults getDefaults{pcapLocations};
   
   auto confDefaults = getDefaults.GetConfDefaults(protoMsg::ConfType_Type_BASE);
   EXPECT_TRUE(!(*getDefaults.GetRange(confDefaults, "pcapRecordsToClearPerCycle")).Validate("test1")); //Int
   EXPECT_TRUE((*getDefaults.GetRange(confDefaults, "pcapRecordsToClearPerCycle")).Validate("1")); //Int
   EXPECT_TRUE(!(*getDefaults.GetRange(confDefaults, "pcapRecordsToClearPerCycle")).Validate("0")); //Int
   EXPECT_TRUE(getDefaults.GetRange(confDefaults, "pcapRecordsToClearPerCycle").use_count() > 0);

   EXPECT_TRUE(!(*getDefaults.GetRange(confDefaults, "qosmosTCPReAssemblyEnabled")).Validate("100")); //Bool
   EXPECT_TRUE((*getDefaults.GetRange(confDefaults, "qosmosTCPReAssemblyEnabled")).Validate("true")); //Bool
   EXPECT_TRUE((*getDefaults.GetRange(confDefaults, "qosmosTCPReAssemblyEnabled")).Validate("FALSE")); //Bool
   
   
   EXPECT_TRUE(!(*getDefaults.GetRange(confDefaults, "dpiThreads")).Validate("13")); //Int
   EXPECT_TRUE(!(*getDefaults.GetRange(confDefaults, "dpiThreads")).Validate("FALSE")); //Int
   
   EXPECT_TRUE((*getDefaults.GetRange(confDefaults, "pcapInterface")).Validate("test")); //string
   EXPECT_TRUE(!(*getDefaults.GetRange(confDefaults, "pcapInterface")).Validate("")); //string
   EXPECT_TRUE((*getDefaults.GetRange(confDefaults, "pcapInterface")).Validate("eth0")); //string
   
   //Shouldn't exist:
   EXPECT_TRUE(getDefaults.GetRange(confDefaults, "pcapInterface2").use_count() == 0);
}

TEST(TestProtoDefaults, GetConfParamDefaultOK){
   auto conf = ConfSlave::Instance().GetConf();
   auto pcapLocations = conf.GetPcapCaptureLocations();
   ProtoDefaults getDefaults{pcapLocations};
   
   auto confDefaults = getDefaults.GetConfDefaults(protoMsg::ConfType_Type_BASE);
   
   const std::string str("4");
   EXPECT_TRUE(getDefaults.GetConfParamDefault(confDefaults, "dpiThreads").size() > 0);
   EXPECT_TRUE(str.compare(getDefaults.GetConfParamDefault(confDefaults, "dpiThreads")) == 0);
   EXPECT_TRUE(getDefaults.GetConfParamDefault(confDefaults, "dpiThreads2").size() == 0);
}

TEST(TestProtoDefaults, GetConfParamOK){
   auto conf = ConfSlave::Instance().GetConf();
   auto pcapLocations = conf.GetPcapCaptureLocations();
   ProtoDefaults getDefaults{pcapLocations};
   
   auto confDefaults = getDefaults.GetConfDefaults(protoMsg::ConfType_Type_BASE);
      
   EXPECT_TRUE((std::get<ProtoDefaults::indexDefault>(getDefaults.GetConfParam(confDefaults, "dpiThreads2"))).size() == 0);
   EXPECT_TRUE(std::get<ProtoDefaults::indexRange>(getDefaults.GetConfParam(confDefaults, "dpiThreads2")) == nullptr);
   EXPECT_TRUE((std::get<ProtoDefaults::indexType>(getDefaults.GetConfParam(confDefaults, "dpiThreads2"))).size() == 0);
   
   std::string str("dpiThreads");
   EXPECT_TRUE((std::get<ProtoDefaults::indexDefault>(getDefaults.GetConfParam(confDefaults, str))).size() > 0);
   EXPECT_TRUE(std::get<ProtoDefaults::indexRange>(getDefaults.GetConfParam(confDefaults, str)) != nullptr);
   EXPECT_TRUE((std::get<ProtoDefaults::indexType>(getDefaults.GetConfParam(confDefaults, str))).size() > 0);

   EXPECT_TRUE((std::get<ProtoDefaults::indexDefault>(getDefaults.GetConfParam(confDefaults, str))).compare("4") == 0);
   EXPECT_TRUE((*std::get<ProtoDefaults::indexRange>(getDefaults.GetConfParam(confDefaults, str))).Validate("4")); //Int
   EXPECT_FALSE((*std::get<ProtoDefaults::indexRange>(getDefaults.GetConfParam(confDefaults, str))).Validate("f")); //Int
}



TEST(TestProtoDefaults, VerifyPcapStorageSizeLimit__Default){
   MockProtoDefaults failedDefault{{"dont_exist"}}; // Failed pcap disk reading will trigger the default
   EXPECT_FALSE(failedDefault.WasPcapDiskReadSuccessful());
   
   auto baseDefaults = failedDefault.GetConfDefaults(protoMsg::ConfType_Type_BASE);
   EXPECT_FALSE(failedDefault.WasPcapDiskReadSuccessful());

   auto pcapSizeLimitDefault = failedDefault.GetConfParam(baseDefaults, "captureSizeLimit");
   std::string defaultValue = std::get<ProtoDefaults::indexDefault>(pcapSizeLimitDefault);
   EXPECT_EQ(defaultValue, "80000");
   
   auto  rangePtr = std::get<ProtoDefaults::indexRange>(pcapSizeLimitDefault);
   EXPECT_EQ(rangePtr->StringifyMin(), "1000");
   EXPECT_EQ(rangePtr->StringifyMax(), "1000000");
   
   std::string pcapSizeLimitType = std::get<ProtoDefaults::indexType>(pcapSizeLimitDefault);
   EXPECT_EQ(pcapSizeLimitType, "num_int");   
}


TEST(TestProtoDefaults, FailedReadingWillTriggerNewReading){
   MockProtoDefaults successfulDefault{{"/usr/local/probe"}}; // Any directory would work really
   
   auto baseDefaults = successfulDefault.GetConfDefaults(protoMsg::ConfType_Type_BASE);
   EXPECT_TRUE(successfulDefault.WasPcapDiskReadSuccessful());

   auto pcapSizeLimitDefault = successfulDefault.GetConfParam(baseDefaults, "captureSizeLimit");
   
   auto  rangePtr = std::get<ProtoDefaults::indexRange>(pcapSizeLimitDefault);
   EXPECT_EQ(rangePtr->StringifyMin(), "1000");
   EXPECT_NE(rangePtr->StringifyMax(), "1000000");
   
   PcapDiskUsage pcapDisk {{"/usr/local/probe"}};
   auto totalUsage = pcapDisk.GetTotalDiskUsage(MemorySize::MB);
   const int64_t pcapLimitInPercentUnits{80};
   auto maxPcapStorageLimit = (pcapLimitInPercentUnits * totalUsage.total) / 100;
   EXPECT_EQ(rangePtr->StringifyMax(), std::to_string(maxPcapStorageLimit));
}