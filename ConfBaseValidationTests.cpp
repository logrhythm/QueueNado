
#include "ConfProcessorTests.h"
#include "Conf.h"
#include "MockConf.h"
#include "BaseConfMsg.pb.h"
#include <g2log.hpp>
#include <functional>
#include <limits>

#ifdef LR_DEBUG
using namespace std;


TEST_F(ConfProcessorTests, BaseConfValidationBlankMsgWillFail) {
   MockConf conf;
   EXPECT_EQ(conf.mValidBaseConf, true);
   protoMsg::BaseConf blank;
   EXPECT_EQ(blank.has_dpithreads(), false);
   conf.updateFields(blank); // trigger Mocked ValidateBaseConf
   EXPECT_EQ(conf.mValidBaseConf, false);   
}


TEST_F(ConfProcessorTests, BaseConfValidationNumbers) {
   MockConf conf;
   EXPECT_EQ(conf.mValidBaseConf, true);
   
   conf.CheckNumber(""); // check for empty
   EXPECT_EQ(conf.mValidBaseConf, false);  
   conf.mValidBaseConf = true;

   conf.CheckNumberForNegative("-123");
   EXPECT_EQ(conf.mValidBaseConf, false);
   conf.CheckNumberForNegative("123");
   EXPECT_EQ(conf.mValidBaseConf, true);

   size_t value = std::numeric_limits<int32_t>::max();   
   conf.CheckNumberForSize(std::to_string(value+1));
   EXPECT_EQ(conf.mValidBaseConf, false);  
   conf.CheckNumberForSize(std::to_string(value));
   EXPECT_EQ(conf.mValidBaseConf, true);  
   
   protoMsg::BaseConf msg;
   msg.set_dpithreads("10");
   conf.CheckNumber(msg.dpithreads()); 
   EXPECT_EQ(conf.mValidBaseConf, true);  
   
}

TEST_F(ConfProcessorTests, BaseConfValidationDpiThreadsInvalidNumberWillFail) {
   MockConf conf;
   EXPECT_EQ(conf.mValidBaseConf, true);
   protoMsg::BaseConf blank;
   blank.set_dpithreads("");
   EXPECT_EQ(blank.has_dpithreads(), true);
   conf.updateFields(blank); // trigger Mocked ValidateBaseConf
   EXPECT_EQ(conf.mValidBaseConf, false);   
   
   blank.set_dpithreads("Hello World!");
   EXPECT_EQ(blank.has_dpithreads(), true);
   conf.updateFields(blank); // trigger Mocked ValidateBaseConf
   EXPECT_EQ(conf.mValidBaseConf, false);  
   
   blank.set_dpithreads("-123");
   EXPECT_EQ(blank.has_dpithreads(), true);
   conf.updateFields(blank); // trigger Mocked ValidateBaseConf
   EXPECT_EQ(conf.mValidBaseConf, false);      
   
   blank.set_dpithreads("-123");
   EXPECT_EQ(blank.has_dpithreads(), true);
   conf.updateFields(blank); // trigger Mocked ValidateBaseConf
   EXPECT_EQ(conf.mValidBaseConf, false);      
   

}


//bool BaseConfValidateNumber(const std::string& dataField, std::function<bool()> hasData, std::function<void()> setData) {
//   std::string help = {"\t\t\t\t\t\tTesting of : "};
//   help.append(dataField);
//   MockConf conf;
//   protoMsg::BaseConf msg;
//
//   EXPECT_EQ(conf.mValidBaseConf, true); // before any tests are run
//   EXPECT_EQ(hasData(msg), false);
//   conf.updateFields(msg); 
//   EXPECT_EQ(conf.mValidBaseConf, false) << help;
//   
//   setData(msg, "10"); // msg.set_dpithreads("10");
//   EXPECT_EQ(hasData(msg), true);
//   conf.updateFields(msg); 
//   EXPECT_EQ(conf.mValidBaseConf, true);
//   
//   return true;
//}

TEST_F(ConfProcessorTests, BaseConfValidationpcapETimeoutInvalidNumberWillFail) {
//   auto hasDpi = [](protoMsg::BaseConf& msg) -> bool { return msg.has_dpithreads();};
//   auto setDpi = [](protoMsg::BaseConf& msg, const std::string& value) ->void { msg.set_dpithreads(value);};
//   BaseConfValidateNumber("dpithreads", hasDpi, setDpi);
   
   MockConf conf;
   EXPECT_EQ(conf.mValidBaseConf, true);
   protoMsg::BaseConf msg;
   msg.set_dpithreads("10"); // valid dpithreads
   
   msg.set_pcapetimeout("");
   EXPECT_EQ(msg.has_pcapetimeout(), true);
   conf.updateFields(msg); // trigger Mocked ValidateBaseConf
   EXPECT_EQ(conf.mValidBaseConf, false);   
   
   msg.set_pcapetimeout("Hello World!");
   EXPECT_EQ(msg.has_pcapetimeout(), true);
   conf.updateFields(msg); // trigger Mocked ValidateBaseConf
   EXPECT_EQ(conf.mValidBaseConf, false);  
   
   msg.set_pcapetimeout("-123");
   EXPECT_EQ(msg.has_pcapetimeout(), true);
   conf.updateFields(msg); // trigger Mocked ValidateBaseConf
   EXPECT_EQ(conf.mValidBaseConf, false);      
}


TEST_F(ConfProcessorTests, BaseConfValidationWithValidNumbersWillNotFail) {
   MockConf conf;
   protoMsg::BaseConf msg;
   EXPECT_EQ(conf.mValidBaseConf, true);
   conf.updateFields(msg);
   EXPECT_EQ(conf.mValidBaseConf, false);

   const std::string validNumber = {"10"};
   msg.set_dpithreads(validNumber); // valid dpithreads
   msg.set_pcapetimeout(validNumber);
   conf.updateFields(msg);
   EXPECT_EQ(conf.mValidBaseConf, true);   
}

#endif //  LR_DEBUG