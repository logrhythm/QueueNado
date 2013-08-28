
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
   
   EXPECT_ANY_THROW(conf.CheckNumber("")); // check for empty
   EXPECT_EQ(conf.mValidBaseConf, false);  
   conf.mValidBaseConf = true;
   
   EXPECT_ANY_THROW(conf.CheckNumber("Hello World!")); // check for empty
   EXPECT_EQ(conf.mValidBaseConf, false);  
   conf.mValidBaseConf = true;

   EXPECT_ANY_THROW(conf.CheckNumberForNegative("-123"));
   EXPECT_EQ(conf.mValidBaseConf, false);
   EXPECT_NO_THROW(conf.CheckNumberForNegative("123"));
   EXPECT_EQ(conf.mValidBaseConf, true);

   size_t value = std::numeric_limits<int32_t>::max();   
   EXPECT_ANY_THROW(conf.CheckNumberForSize(std::to_string(value+1)));
   EXPECT_EQ(conf.mValidBaseConf, false);  
   EXPECT_NO_THROW(conf.CheckNumberForSize(std::to_string(value)));
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
   EXPECT_ANY_THROW(conf.CheckString(text)); 
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
   EXPECT_ANY_THROW(conf.CheckBool("")); 
   EXPECT_EQ(conf.mValidBaseConf, false);  

   conf.mValidBaseConf = true;
   EXPECT_ANY_THROW(conf.CheckBool("Hello World!")); 
   EXPECT_EQ(conf.mValidBaseConf, false);  

   conf.mValidBaseConf = false;
   EXPECT_NO_THROW(conf.CheckBool("true")); 
   EXPECT_EQ(conf.mValidBaseConf, true);  

   conf.mValidBaseConf = false;
   EXPECT_NO_THROW(conf.CheckBool("false")); 
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
   msg.set_commandqueue(validText);            // commandQueue
   msg.set_enableintermediateflows(validBool); //enableIntermediateFlows
   msg.set_enablepacketcapture(validBool);     //enablePacketCapture
   msg.set_capturefilelimit(validNumber);       //captureFileLimit
   msg.set_capturesizelimit(validNumber);       //captureSizeLimit
   msg.set_capturememorylimit(validNumber);     //captureMemoryLimit
   msg.set_capturemaxpackets(validNumber);      //captureMaxPackets
   
   conf.updateFields(msg);
   EXPECT_EQ(conf.mValidBaseConf, true);   
}

/**
 * Sets all fields except for the given @param shouldFail. 
 * Setting the shouldField to a higher number than the number of fields
 * SHOULD make the validation to pass instead of fail
 */
namespace {
    const size_t gNumberOfFields = 26;
}
void ValidateAllFieldsSetInvalidOnX(const size_t shouldFail) {
   size_t index = 0;
   MockConf conf;
   conf.mIgnoreBaseConfValidation = false;
   conf.mValidBaseConf = false;
   
   protoMsg::BaseConf msg; 
   const std::string validNumber = {"10"};
   const std::string invalidNumber = std::to_string(std::numeric_limits<size_t>::max());
   
   const std::string validText = {"Hello Wold!"};
   const std::string invalidText(1001,'x');
   
   const std::string validBool = {"false"};
   const std::string invalidBool = {"1"};
   
   (index++ == shouldFail) ? msg.set_dpithreads(invalidNumber) : msg.set_dpithreads(validNumber);
   (index++ == shouldFail) ? msg.set_pcapetimeout(invalidNumber) : msg.set_pcapetimeout(validNumber);
   (index++ == shouldFail) ? msg.set_pcapbuffersize(invalidNumber): msg.set_pcapbuffersize(validNumber);
   (index++ == shouldFail) ? msg.set_pcapinterface(invalidText): msg.set_pcapinterface(validText);
   (index++ == shouldFail) ? msg.set_dpihalfsessions(invalidNumber) : msg.set_dpihalfsessions(validNumber) ;
   (index++ == shouldFail) ? msg.set_packetsendqueuesize(invalidNumber) : msg.set_packetsendqueuesize(validNumber);
   (index++ == shouldFail) ? msg.set_packetrecvqueuesize(invalidNumber) : msg.set_packetrecvqueuesize(validNumber);
   (index++ == shouldFail) ? msg.set_dpimsgsendqueuesize(invalidNumber) : msg.set_dpimsgsendqueuesize(validNumber);
   (index++ == shouldFail) ? msg.set_dpimsgrecvqueuesize(invalidNumber) : msg.set_dpimsgrecvqueuesize(validNumber);
   (index++ == shouldFail) ? msg.set_qosmosdebugmodeenabled(invalidBool) : msg.set_qosmosdebugmodeenabled(validBool);
   (index++ == shouldFail) ? msg.set_qosmos64bytepool(invalidNumber) : msg.set_qosmos64bytepool(validNumber);
   (index++ == shouldFail) ? msg.set_qosmos128bytepool(invalidNumber) : msg.set_qosmos128bytepool(validNumber);
   (index++ == shouldFail) ? msg.set_qosmos256bytepool(invalidNumber) :msg.set_qosmos256bytepool(validNumber);
   (index++ == shouldFail) ? msg.set_qosmos512bytepool(invalidNumber) : msg.set_qosmos512bytepool(validNumber);
   (index++ == shouldFail) ? msg.set_statsaccumulatorqueue(invalidText) : msg.set_statsaccumulatorqueue(validText);
   (index++ == shouldFail) ? msg.set_sendstatsqueue(invalidText) : msg.set_sendstatsqueue(validText);
   (index++ == shouldFail) ? msg.set_qosmosexpirepercallback(invalidNumber):msg.set_qosmosexpirepercallback(validNumber);
   (index++ == shouldFail) ? msg.set_qosmostcpreassemblyenabled(invalidBool) : msg.set_qosmostcpreassemblyenabled(validBool);
   (index++ == shouldFail) ? msg.set_qosmosipdefragmentationenabled(invalidBool) : msg.set_qosmosipdefragmentationenabled(validBool);
   (index++ == shouldFail) ? msg.set_commandqueue(invalidText) : msg.set_commandqueue(validText);
   (index++ == shouldFail) ? msg.set_enableintermediateflows(invalidBool) : msg.set_enableintermediateflows(validBool);
   (index++ == shouldFail) ? msg.set_enablepacketcapture(invalidBool) : msg.set_enablepacketcapture(validBool);
   (index++ == shouldFail) ? msg.set_capturefilelimit(invalidNumber) : msg.set_capturefilelimit(validNumber);
   (index++ == shouldFail) ? msg.set_capturesizelimit(invalidNumber) : msg.set_capturesizelimit(validNumber);
   (index++ == shouldFail) ? msg.set_capturememorylimit(invalidNumber) : msg.set_capturememorylimit(validNumber);
   (index++ == shouldFail) ? msg.set_capturemaxpackets(invalidNumber) : msg.set_capturemaxpackets(validNumber);
   
   
   // Test sanity check. Total number of used fields are :  26
   EXPECT_EQ(index, gNumberOfFields) << "\t\t\t\t\t: Expected number of fields are 26 unless you added more?";
   conf.updateFields(msg);
   
   
   if (shouldFail > gNumberOfFields) {
      if(false == conf.mValidBaseConf) {
         FAIL() << "\t\t\t\t\t: No fields should be invalid, 'shouldFail was: " << std::to_string(shouldFail);  
      }
      else {
         SUCCEED();
      }
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


#endif //  LR_DEBUG