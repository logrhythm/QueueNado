
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
   conf.mIgnoreBaseConfValidation = false;
   
   EXPECT_EQ(conf.mValidBaseConf, true);
   protoMsg::BaseConf blank;
   EXPECT_EQ(blank.has_dpithreads(), false);
   conf.updateFields(blank); // trigger Mocked ValidateBaseConf
   EXPECT_EQ(conf.mValidBaseConf, false);   
}


TEST_F(ConfProcessorTests, BaseConfValidationNumbers) {
   MockConf conf;
   conf.mIgnoreBaseConfValidation = false;
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

TEST_F(ConfProcessorTests, BaseConfValidationText) {
   MockConf conf;
   conf.mIgnoreBaseConfValidation = false;
   EXPECT_EQ(conf.mValidBaseConf, true);
   

   conf.mValidBaseConf = false;
   conf.CheckString(""); 
   EXPECT_EQ(conf.mValidBaseConf, true);  
   conf.mValidBaseConf = true;
   
   conf.mValidBaseConf = false;
   conf.CheckString("Hello World!"); 
   EXPECT_EQ(conf.mValidBaseConf, true);  


   std::string text(1000,'x');
   conf.mValidBaseConf = false;
   conf.CheckString(text); 
   EXPECT_EQ(conf.mValidBaseConf, true);  
   conf.mValidBaseConf = false;
   conf.CheckStringForSize(text); 
   EXPECT_EQ(conf.mValidBaseConf, true); 
   
   // validate size failures
   conf.mValidBaseConf = true;
   text.append({"y"});
   conf.CheckString(text); 
   EXPECT_EQ(conf.mValidBaseConf, false);  
 
   conf.mValidBaseConf = true;
   conf.CheckStringForSize(text); 
   EXPECT_EQ(conf.mValidBaseConf, false); 
}   


TEST_F(ConfProcessorTests, BaseConfValidationBool) {
   MockConf conf;
   conf.mIgnoreBaseConfValidation = false;
   EXPECT_EQ(conf.mValidBaseConf, true);

   conf.mValidBaseConf = true;
   conf.CheckBool(""); 
   EXPECT_EQ(conf.mValidBaseConf, false);  

   conf.mValidBaseConf = true;
   conf.CheckBool("Hello World!"); 
   EXPECT_EQ(conf.mValidBaseConf, false);  

   conf.mValidBaseConf = false;
   conf.CheckBool("true"); 
   EXPECT_EQ(conf.mValidBaseConf, true);  

   conf.mValidBaseConf = false;
   conf.CheckBool("false"); 
   EXPECT_EQ(conf.mValidBaseConf, true);  
}   

   
TEST_F(ConfProcessorTests, BaseConfValidationWithValidDataWillNotFail) {
   MockConf conf;
   conf.mIgnoreBaseConfValidation = false;
   protoMsg::BaseConf msg;
   EXPECT_EQ(conf.mValidBaseConf, true);
   conf.updateFields(msg);
   EXPECT_EQ(conf.mValidBaseConf, false);

   const std::string validNumber = {"10"};
   const std::string validText = {"Hello Wold!"};
   const std::string validBool = {"false"};
   msg.set_dpithreads(validNumber);     // dpithreads
   msg.set_pcapetimeout(validNumber);   // pcapETimeout
   msg.set_pcapbuffersize(validNumber); // pcapBufferSize
   msg.set_pcapinterface(validText);    // pcapBufferSize
   msg.set_dpihalfsessions(validNumber); // dpiHalfSessions
   msg.set_packetsendqueuesize(validNumber); // packetSendQueueSize
   msg.set_packetrecvqueuesize(validNumber); // packetRecvQueueSize
   msg.set_dpimsgsendqueuesize(validNumber); // dpiMsgSendQueueSize
   msg.set_dpimsgrecvqueuesize(validNumber); // dpiMsgRecvQueueSize 
   msg.set_qosmosdebugmodeenabled(validBool); //qosmosdebugmodeenabled
   msg.set_qosmos64bytepool(validNumber);     // Qosmos Byte Bool 64Byte
   msg.set_qosmos128bytepool(validNumber);     // Qosmos Byte Bool 128Byte
   msg.set_qosmos256bytepool(validNumber);     // Qosmos Byte Bool 256Byte
   msg.set_qosmos512bytepool(validNumber); // Qosmos Byte Bool 512Byte
   msg.set_statsaccumulatorqueue(validText);  // statsAccumulatorQueue
   msg.set_sendstatsqueue(validText); //sendStatsQueue
   msg.set_qosmosexpirepercallback(validNumber); //qosmosExpirePerCallback
   msg.set_qosmostcpreassemblyenabled(validBool); //qosmosTCPReAssemblyEnabled
   msg.set_qosmosipdefragmentationenabled(validBool);//qosmosIPDefragmentationEnabled
   
   // WARNING: The commented out fields does not seem to be used in the conf
   // SHOULD THEY BE REMOVED FROM THE BaseConfMsg.proto?
   // msg.set_dbclustername
   // msg.set_dburl
   // msg.set_dburl;
   // msg.set_statsaggregationqueue
   msg.set_commandqueue(validText);
   msg.set_enableintermediateflows(validBool);
   msg.set_enablepacketcapture(validBool);
   
   
   // START
   msg.set_capturefilelimit(validNumber);
   msg.set_capturesizelimit(validNumber);
   msg.set_capturememorylimit(validNumber);
   msg.set_capturemaxpackets(validNumber);
   
   conf.updateFields(msg);
   EXPECT_EQ(conf.mValidBaseConf, true);   
}


// below this old, failing ALWAYS since the total conf is evaluated
// the tests below should be removed and instead be replaced for 
// 1. the non failing test above
// 2. An looping approach where all fields are entered except one (
// 
//  this should make it less verbose and easier to modify as the conf changes
//  over time
// 
//    for(int i = 0; i < fields; ++i) }{ doAllExcept(i)
//    void doAllExcept(int number) {
//    if( check++ != number) xxx_setdpithreads(...);
//    ....
//    conf.updateFields(...);
//    EXPECT_EQ(conf.mValidBaseConf, false)....
//     
//
TEST_F(ConfProcessorTests, BaseConfValidationDpiThreadsInvalidNumberWillFail) {
   MockConf conf;
   conf.mIgnoreBaseConfValidation = false;
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
   conf.mIgnoreBaseConfValidation = false;
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



#endif //  LR_DEBUG