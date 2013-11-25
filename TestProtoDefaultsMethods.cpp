#include <tuple>
#include "ValidateConf.cpp"
#include "gtest/gtest.h"
#include "Range.h"

TEST(TestProtoDefaults, GetConfDefaultsOK) {
   protoDefaults::ConfDefaults defaults1 = protoDefaults::GetConfDefaults("BASE");
   EXPECT_TRUE(defaults1.size() > 0);
   EXPECT_TRUE(std::get<0>(defaults1[0]).compare("dpiThreads") == 0);
   EXPECT_TRUE((*std::get<2>(defaults1[0])).Validate("4")); //4 DPI Threads

   protoDefaults::ConfDefaults defaults2 = protoDefaults::GetConfDefaults("TEST");
   EXPECT_TRUE(defaults2.size() == 0);
}

TEST(TestProtoDefaults, GetRangeOK){
   protoDefaults::ConfDefaults confDefaults = protoDefaults::GetConfDefaults("BASE");
   EXPECT_TRUE(!(*protoDefaults::GetRange(confDefaults, "pcapRecordsToClearPerCycle")).Validate("test1")); //Int
   EXPECT_TRUE((*protoDefaults::GetRange(confDefaults, "pcapRecordsToClearPerCycle")).Validate("1")); //Int
   EXPECT_TRUE(!(*protoDefaults::GetRange(confDefaults, "pcapRecordsToClearPerCycle")).Validate("0")); //Int
   EXPECT_TRUE(protoDefaults::GetRange(confDefaults, "pcapRecordsToClearPerCycle").use_count() > 0);

   EXPECT_TRUE(!(*protoDefaults::GetRange(confDefaults, "qosmosTCPReAssemblyEnabled")).Validate("100")); //Bool
   EXPECT_TRUE((*protoDefaults::GetRange(confDefaults, "qosmosTCPReAssemblyEnabled")).Validate("true")); //Bool
   EXPECT_TRUE((*protoDefaults::GetRange(confDefaults, "qosmosTCPReAssemblyEnabled")).Validate("FALSE")); //Bool
   
   
   EXPECT_TRUE(!(*protoDefaults::GetRange(confDefaults, "dpiThreads")).Validate("13")); //Int
   EXPECT_TRUE(!(*protoDefaults::GetRange(confDefaults, "dpiThreads")).Validate("FALSE")); //Int
   
   EXPECT_TRUE((*protoDefaults::GetRange(confDefaults, "pcapInterface")).Validate("test")); //string
   EXPECT_TRUE(!(*protoDefaults::GetRange(confDefaults, "pcapInterface")).Validate("")); //string
   EXPECT_TRUE((*protoDefaults::GetRange(confDefaults, "pcapInterface")).Validate("eth0")); //string
   
   //Shouldn't exist:
   EXPECT_TRUE(protoDefaults::GetRange(confDefaults, "pcapInterface2").use_count() == 0);

}

TEST(TestProtoDefaults, GetConfParamDefaultOK){
   protoDefaults::ConfDefaults confDefaults = protoDefaults::GetConfDefaults("BASE");
   
   const std::string str("4");
   EXPECT_TRUE(protoDefaults::GetConfParamDefault(confDefaults, "dpiThreads").size() > 0);
   EXPECT_TRUE(str.compare(protoDefaults::GetConfParamDefault(confDefaults, "dpiThreads")) == 0);
   EXPECT_TRUE(protoDefaults::GetConfParamDefault(confDefaults, "dpiThreads2").size() == 0);
}

TEST(TestProtoDefaults, GetConfParamOK){
   protoDefaults::ConfDefaults confDefaults = protoDefaults::GetConfDefaults("BASE");
   
   EXPECT_TRUE((std::get<0>(protoDefaults::GetConfParam(confDefaults, "dpiThreads2"))).size() == 0);
   EXPECT_TRUE((std::get<1>(protoDefaults::GetConfParam(confDefaults, "dpiThreads2"))).size() == 0);
   EXPECT_TRUE(std::get<2>(protoDefaults::GetConfParam(confDefaults, "dpiThreads2")) == nullptr);
   
   std::string str("dpiThreads");
   EXPECT_TRUE((std::get<0>(protoDefaults::GetConfParam(confDefaults, str))).size() > 0);
   EXPECT_TRUE((std::get<1>(protoDefaults::GetConfParam(confDefaults, str))).size() > 0);
   EXPECT_TRUE(std::get<2>(protoDefaults::GetConfParam(confDefaults, str)) != nullptr);

   EXPECT_TRUE(str.compare(std::get<0>(protoDefaults::GetConfParam(confDefaults, str))) == 0);
   EXPECT_TRUE((std::get<1>(protoDefaults::GetConfParam(confDefaults, str))).compare("4") == 0);
   EXPECT_TRUE((*std::get<2>(protoDefaults::GetConfParam(confDefaults, str))).Validate("4")); //Int
   EXPECT_TRUE(!(*std::get<2>(protoDefaults::GetConfParam(confDefaults, str))).Validate("f")); //Int
   
}
