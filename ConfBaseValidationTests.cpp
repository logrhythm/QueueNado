
#include "ConfProcessorTests.h"
#include "Conf.h"
#include "MockConf.h"
#include "BaseConfMsg.pb.h"
#include <g2log.hpp>
#include <functional>
#include <limits>

#ifdef LR_DEBUG
using namespace std;

namespace {
   Range gMax = {1, std::numeric_limits<uint32_t>::max()};
}
// Not set fields are NOT failure, they will just be ignored 
TEST_F(ConfProcessorTests, BaseConfValidationBlankMsgWillSucceed) {
   MockConf conf;
   conf.mIgnoreBaseConfValidation = false;
   
   EXPECT_EQ(conf.mValidBaseConf, true);
   protoMsg::BaseConf blank;
   EXPECT_EQ(blank.has_dpithreads(), false);
   conf.updateFields(blank); // trigger Mocked ValidateBaseConf
   EXPECT_EQ(conf.mValidBaseConf, true);   
}

// Erronous fields WILL be cleared
TEST_F(ConfProcessorTests, BaseConfValidationErrorFieldsWillBeCleared) {
   MockConf conf;
   conf.mIgnoreBaseConfValidation = false;
   EXPECT_EQ(conf.mValidBaseConf, true);
   
   protoMsg::BaseConf right;
   right.set_dpithreads("2");
   conf.updateFields(right);
   EXPECT_EQ(conf.mValidBaseConf, true);
   
   
   
   // Verify that erronous fields are cleared and ignored
   protoMsg::BaseConf wrong = conf.getProtoMsg();
   EXPECT_EQ(wrong.dpithreads(), "2");
   wrong.set_dpithreads("Hello World!");
   conf.mValidBaseConf =  true;
   conf.updateFields(wrong);
   EXPECT_EQ(wrong.has_dpithreads(), true); // copies are not cleared
   EXPECT_EQ(conf.mValidBaseConf, false);
   
   wrong = conf.getProtoMsg();
   EXPECT_EQ(wrong.dpithreads(), "2"); 
      
   wrong.set_dpithreads("Hello World!");
   EXPECT_EQ(wrong.has_dpithreads(), true); 
   conf.ValidateBaseConf(wrong); // this should clear the field
   EXPECT_EQ(wrong.has_dpithreads(), false);
}


TEST_F(ConfProcessorTests, BaseConfValidationNumbers) {
   MockConf conf;
   conf.mIgnoreBaseConfValidation = false;
   EXPECT_EQ(conf.mValidBaseConf, true);
   
   EXPECT_NO_THROW(conf.CheckNumber("", gMax)); // check for empty
   EXPECT_EQ(conf.mValidBaseConf, false);  
   conf.mValidBaseConf = true;
   
   EXPECT_NO_THROW(conf.CheckNumber("Hello World!", gMax)); // check for empty
   EXPECT_EQ(conf.mValidBaseConf, false);  
   conf.mValidBaseConf = true;

   EXPECT_ANY_THROW(conf.CheckNumberForNegative("-123"));
   EXPECT_EQ(conf.mValidBaseConf, false);
   EXPECT_NO_THROW(conf.CheckNumberForNegative("123"));
   EXPECT_EQ(conf.mValidBaseConf, true);
  
   EXPECT_ANY_THROW(conf.CheckNumberForSize(std::to_string(gMax.higher+1), gMax));
   EXPECT_EQ(conf.mValidBaseConf, false);  
   EXPECT_NO_THROW(conf.CheckNumberForSize(std::to_string(gMax.higher), gMax));
   EXPECT_EQ(conf.mValidBaseConf, true);  
   
   protoMsg::BaseConf msg;
   msg.set_dpithreads("10");
   conf.CheckNumber(msg.dpithreads(), Range{1,12}); 
   EXPECT_EQ(conf.mValidBaseConf, true);   
}

TEST_F(ConfProcessorTests, BaseConfValidationText) {
   MockConf conf;
   conf.mIgnoreBaseConfValidation = false;
   EXPECT_EQ(conf.mValidBaseConf, true);
   

   conf.mValidBaseConf = false;
   EXPECT_NO_THROW(conf.CheckString("")); 
   EXPECT_EQ(conf.mValidBaseConf, true);  
   conf.mValidBaseConf = true;
   
   conf.mValidBaseConf = false;
   EXPECT_NO_THROW(conf.CheckString("Hello World!")); 
   EXPECT_EQ(conf.mValidBaseConf, true);  


   std::string text(1000,'x');
   conf.mValidBaseConf = false;
   EXPECT_NO_THROW(conf.CheckString(text)); 
   EXPECT_EQ(conf.mValidBaseConf, true);  
   conf.mValidBaseConf = false;
   EXPECT_NO_THROW(conf.CheckStringForSize(text)); 
   EXPECT_EQ(conf.mValidBaseConf, true); 
   
   // validate size failures
   conf.mValidBaseConf = true;
   text.append({"y"});
   EXPECT_NO_THROW(conf.CheckString(text)); 
   EXPECT_EQ(conf.mValidBaseConf, false);  
 
   conf.mValidBaseConf = true;
   EXPECT_ANY_THROW(conf.CheckStringForSize(text)); 
   EXPECT_EQ(conf.mValidBaseConf, false); 
}   


TEST_F(ConfProcessorTests, BaseConfValidationBool) {
   MockConf conf;
   conf.mIgnoreBaseConfValidation = false;
   EXPECT_EQ(conf.mValidBaseConf, true);

   conf.mValidBaseConf = true;
   EXPECT_NO_THROW(conf.CheckBool("")); 
   EXPECT_EQ(conf.mValidBaseConf, false);  

   conf.mValidBaseConf = true;
   EXPECT_NO_THROW(conf.CheckBool("Hello World!")); 
   EXPECT_EQ(conf.mValidBaseConf, false);  

   conf.mValidBaseConf = false;
   EXPECT_NO_THROW(conf.CheckBool("true")); 
   EXPECT_EQ(conf.mValidBaseConf, true);  

   conf.mValidBaseConf = false;
   EXPECT_NO_THROW(conf.CheckBool("false")); 
   EXPECT_EQ(conf.mValidBaseConf, true);  
}   

   
TEST_F(ConfProcessorTests, BaseConfValidationWithMaxMinRangesWillNotFail) {
   MockConf conf;
   conf.mIgnoreBaseConfValidation = false;
   protoMsg::BaseConf msgMax;
   protoMsg::BaseConf msgMin;
   EXPECT_EQ(conf.mValidBaseConf, true);

   // Nonsense update will not trigger a 'failed validation'
   conf.updateFields(msgMax);
   EXPECT_EQ(conf.mValidBaseConf, true);
   conf.updateFields(msgMin);
   EXPECT_EQ(conf.mValidBaseConf, true);

   
   const std::string validText = {"Hello Wold!"};
   const std::string validBool = {"false"};
   msgMax.set_dpithreads("12");     // dpithreads: 1-12
   msgMin.set_dpithreads("1");     // dpithreads: 1-12
   msgMax.set_pcapetimeout("30");   // pcapETimeout 1-30
   msgMin.set_pcapetimeout("1");   // pcapETimeout
   msgMax.set_pcapbuffersize("1000"); // pcapBufferSize 1-1000
   msgMin.set_pcapbuffersize("1"); // pcapBufferSize 1-1000
   msgMax.set_pcapinterface(validText);    // pcapBufferSize string
   msgMin.set_pcapinterface(validText);    // pcapBufferSize string

   msgMax.set_dpihalfsessions("20000000"); // dpiHalfSessions
   msgMin.set_dpihalfsessions("1000000"); // dpiHalfSessions
   msgMax.set_packetsendqueuesize("10000"); // packetSendQueueSize
   msgMin.set_packetsendqueuesize("10"); // packetSendQueueSize
   msgMax.set_packetrecvqueuesize("10000"); // packetRecvQueueSize
   msgMin.set_packetrecvqueuesize("10"); // packetRecvQueueSize
   msgMax.set_dpimsgsendqueuesize("100000"); // dpiMsgSendQueueSize
   msgMin.set_dpimsgsendqueuesize("10"); // dpiMsgSendQueueSize
   msgMax.set_dpimsgrecvqueuesize("100000"); // dpiMsgRecvQueueSize 
   msgMin.set_dpimsgrecvqueuesize("10"); // dpiMsgRecvQueueSize 
   msgMax.set_qosmosdebugmodeenabled(validBool); //qosmosdebugmodeenabled
   msgMin.set_qosmosdebugmodeenabled(validBool); //qosmosdebugmodeenabled

   msgMax.set_qosmos64bytepool("8000000");     // Qosmos Byte Bool 64Byte
   msgMin.set_qosmos64bytepool("200000");     // Qosmos Byte Bool 64Byte
   msgMax.set_qosmos128bytepool("8000000");     // Qosmos Byte Bool 128Byte
   msgMin.set_qosmos128bytepool("200000");     // Qosmos Byte Bool 128Byte
   msgMax.set_qosmos256bytepool("8000000");     // Qosmos Byte Bool 256Byte
   msgMin.set_qosmos256bytepool("200000");     // Qosmos Byte Bool 256Byte
   msgMax.set_qosmos512bytepool("800000"); // Qosmos Byte Bool 512Byte
   msgMin.set_qosmos512bytepool("200000"); // Qosmos Byte Bool 512Byte
   msgMax.set_statsaccumulatorqueue(validText);  // statsAccumulatorQueue
   msgMin.set_statsaccumulatorqueue(validText);  // statsAccumulatorQueue
   msgMax.set_sendstatsqueue(validText); //sendStatsQueue
   msgMin.set_sendstatsqueue(validText); //sendStatsQueue
   msgMax.set_qosmosexpirepercallback("200"); //qosmosExpirePerCallback 1-200
   msgMin.set_qosmosexpirepercallback("1"); //qosmosExpirePerCallback 1-200
   msgMax.set_qosmostcpreassemblyenabled(validBool); //qosmosTCPReAssemblyEnabled
   msgMin.set_qosmostcpreassemblyenabled(validBool); //qosmosTCPReAssemblyEnabled
   msgMax.set_qosmosipdefragmentationenabled(validBool);//qosmosIPDefragmentationEnabled
   msgMin.set_qosmosipdefragmentationenabled(validBool);//qosmosIPDefragmentationEnabled
   msgMax.set_dbclustername(validText); // dbCluserName
   msgMin.set_dbclustername(validText); // dbCluserName
   msgMax.set_dburl(validText);         // dbUrl
   msgMin.set_dburl(validText);         // dbUrl
   msgMax.set_statsaggregationqueue(validText); // statsAggregationQueue
   msgMin.set_statsaggregationqueue(validText); // statsAggregationQueue
   msgMax.set_commandqueue(validText);            // commandQueue
   msgMin.set_commandqueue(validText);            // commandQueue
   msgMax.set_enableintermediateflows(validBool); //enableIntermediateFlows
   msgMin.set_enableintermediateflows(validBool); //enableIntermediateFlows
   msgMax.set_enablepacketcapture(validBool);     //enablePacketCapture
   msgMin.set_enablepacketcapture(validBool);     //enablePacketCapture

   msgMax.set_capturefilelimit(std::to_string(std::numeric_limits<int32_t>::max()));       //captureFileLimit
   msgMin.set_capturefilelimit("1000000");       //captureFileLimit
   msgMax.set_capturesizelimit("1000000");       //captureSizeLimit
   msgMin.set_capturesizelimit("1000");       //captureSizeLimit
   msgMax.set_capturememorylimit("16000");     //captureMemoryLimit
   msgMin.set_capturememorylimit("1000");     //captureMemoryLimit
   msgMax.set_capturemaxpackets(std::to_string(std::numeric_limits<int32_t>::max()));      //captureMaxPackets
   msgMin.set_capturemaxpackets("1000000");      //captureMaxPackets
   
   conf.updateFields(msgMax);
   EXPECT_EQ(conf.mValidBaseConf, true);   

   conf.updateFields(msgMin);
   EXPECT_EQ(conf.mValidBaseConf, true);   
}

/**
 * Sets all fields except for the given @param shouldFail. 
 * Setting the shouldField to a higher number than the number of fields
 * SHOULD make the validation to pass instead of fail
 */
namespace {
    const size_t gNumberOfFields = 29;
}
void ValidateAllFieldsSetInvalidOnX(const size_t shouldFail) {
   size_t index = 0;
   MockConf conf;
   conf.mIgnoreBaseConfValidation = false;
   conf.mValidBaseConf = false;
   
   protoMsg::BaseConf msg; 
   
   const std::string validText = {"Hello Wold!"};
   const std::string invalidText(1001,'x');
   
   const std::string validBool = {"false"};
   const std::string invalidBool = {"1"};
   
   (index++ == shouldFail) ? msg.set_dpithreads("13") : msg.set_dpithreads("8");
   (index++ == shouldFail) ? msg.set_pcapetimeout("31") : msg.set_pcapetimeout("30");
   (index++ == shouldFail) ? msg.set_pcapbuffersize("1001"): msg.set_pcapbuffersize("1000");
   (index++ == shouldFail) ? msg.set_pcapinterface(invalidText): msg.set_pcapinterface(validText);
   (index++ == shouldFail) ? msg.set_dpihalfsessions("20000001") : msg.set_dpihalfsessions("20000000") ;
   (index++ == shouldFail) ? msg.set_packetsendqueuesize("10001") : msg.set_packetsendqueuesize("10000");
   (index++ == shouldFail) ? msg.set_packetrecvqueuesize("9") : msg.set_packetrecvqueuesize("10");
   (index++ == shouldFail) ? msg.set_dpimsgsendqueuesize("9") : msg.set_dpimsgsendqueuesize("100000");
   (index++ == shouldFail) ? msg.set_dpimsgrecvqueuesize("9") : msg.set_dpimsgrecvqueuesize("100000");
   (index++ == shouldFail) ? msg.set_qosmosdebugmodeenabled(invalidBool) : msg.set_qosmosdebugmodeenabled(validBool);
   (index++ == shouldFail) ? msg.set_qosmos64bytepool("199999") : msg.set_qosmos64bytepool("200000");
   (index++ == shouldFail) ? msg.set_qosmos128bytepool("8000001") : msg.set_qosmos128bytepool("8000000");
   (index++ == shouldFail) ? msg.set_qosmos256bytepool("8000001") :msg.set_qosmos256bytepool("8000000");
   (index++ == shouldFail) ? msg.set_qosmos512bytepool("800001") : msg.set_qosmos512bytepool("800000");
   (index++ == shouldFail) ? msg.set_statsaccumulatorqueue(invalidText) : msg.set_statsaccumulatorqueue(validText);
   (index++ == shouldFail) ? msg.set_sendstatsqueue(invalidText) : msg.set_sendstatsqueue(validText);
   (index++ == shouldFail) ? msg.set_qosmosexpirepercallback("201"):msg.set_qosmosexpirepercallback("200");
   (index++ == shouldFail) ? msg.set_qosmostcpreassemblyenabled(invalidBool) : msg.set_qosmostcpreassemblyenabled(validBool);
   (index++ == shouldFail) ? msg.set_qosmosipdefragmentationenabled(invalidBool) : msg.set_qosmosipdefragmentationenabled(validBool);
   // next three are used in jave only, cannot be changed through the UI
   (index++ == shouldFail) ? msg.set_dbclustername(invalidText) : msg.set_dbclustername(validText);
   (index++ == shouldFail) ? msg.set_dburl(invalidText) : msg.set_dburl(validText);
   (index++ == shouldFail) ? msg.set_statsaggregationqueue(invalidText) : msg.set_statsaggregationqueue(validText);
   
   (index++ == shouldFail) ? msg.set_commandqueue(invalidText) : msg.set_commandqueue(validText);
   (index++ == shouldFail) ? msg.set_enableintermediateflows(invalidBool) : msg.set_enableintermediateflows(validBool);
   (index++ == shouldFail) ? msg.set_enablepacketcapture(invalidBool) : msg.set_enablepacketcapture(validBool);
   (index++ == shouldFail) ? msg.set_capturefilelimit("999") : msg.set_capturefilelimit("1000000");
   (index++ == shouldFail) ? msg.set_capturesizelimit("999") : msg.set_capturesizelimit("1000");
   (index++ == shouldFail) ? msg.set_capturememorylimit("999") : msg.set_capturememorylimit("1000");
   (index++ == shouldFail) ? msg.set_capturemaxpackets("999999") : msg.set_capturemaxpackets("1000000");
   
   // Test sanity check. Total number of used fields are :  29
   EXPECT_EQ(index, gNumberOfFields) << "\t\t\t\t\t: Expected number of fields are 26 unless you added more?";
   conf.updateFields(msg);

   if (shouldFail > gNumberOfFields) {
      if(false == conf.mValidBaseConf) {
         FAIL() << "\t\t\t\t\t: No fields should be invalid, 'shouldFail was: " << std::to_string(shouldFail);  
         return;
      }
      SUCCEED();
      return;
   }
   
   // We can only reach this if 'shouldFail' was less than number of fields
   // in this case me MUST have failed or else this test or Conf.cpp has changed
   //  (or is corrupt)
   if (true == conf.mValidBaseConf) {
      FAIL() << "\t\t\t\t\t: One field should be invalid. 'shouldFail was: " << std::to_string(shouldFail);
      return;
   }
   
   SUCCEED();
}


TEST_F(ConfProcessorTests, BaseConfValidationAllFieldsFailure) {
   for(size_t field = 0; field <  gNumberOfFields; ++field) {
      ValidateAllFieldsSetInvalidOnX(field);
   }
}

TEST_F(ConfProcessorTests, BaseConfValidationAllFieldsSuccess) {
   ValidateAllFieldsSetInvalidOnX(gNumberOfFields+1);
}

TEST_F(ConfProcessorTests, BaseConfValidationInternalRepairBaseConf) {
   Conf conf; // using a real conf
   auto check = conf.InternallyRepairBaseConf();
   EXPECT_EQ(check, true);   
}




#endif //  LR_DEBUG