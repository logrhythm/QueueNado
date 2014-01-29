
#include "ConfProcessorTests.h"
#include "Conf.h"
#include "ConfSlave.h"
#include "MockConf.h"
#include "MockConfMaster.h"
#include "BaseConfMsg.pb.h"
#include "ProtoDefaults.h"
#include <g2log.hpp>
#include <functional>
#include <limits>
#include <string>

#ifdef LR_DEBUG
using namespace std;

// Not set fields are NOT failure, they will just be ignored 

TEST_F(ConfProcessorTests, BaseConfValidationBlankMsgWillSucceed) {
   MockConfMaster master;
   MockConf conf;
   master.mIgnoreConfValidate = false;

   EXPECT_EQ(master.mValidConfValidation, true);
   protoMsg::BaseConf blank;
   EXPECT_EQ(blank.has_dpithreads(), false);
   EXPECT_TRUE(master.ValidateConfFieldValues(conf,blank,protoMsg::ConfType_Type_BASE));
}

// Erroneous fields WILL be cleared

TEST_F(ConfProcessorTests, BaseConfValidationErrorFieldsWillBeCleared) {
   MockConfMaster master;
   MockConf conf;
   master.mIgnoreConfValidate = false;
   EXPECT_EQ(master.mValidConfValidation, true);

   protoMsg::BaseConf right;
   right.set_dpithreads("2");
   EXPECT_TRUE(master.ValidateConfFieldValues(conf,right,protoMsg::ConfType_Type_BASE));

   // Verify that erroneous fields are cleared and ignored
   protoMsg::BaseConf wrong = conf.getProtoMsg();
   EXPECT_EQ(wrong.dpithreads(), "2");
   wrong.set_dpithreads("Hello World!");
   master.mValidConfValidation = true;
   EXPECT_FALSE(master.ValidateConfFieldValues(conf,wrong,protoMsg::ConfType_Type_BASE));
   std::string wrongString;
   wrongString = wrong.SerializeAsString();
   master.ProcessBaseConfigRequest(conf,wrongString);
   EXPECT_EQ(wrong.has_dpithreads(), true); // copies are not cleared

   wrong = conf.getProtoMsg();
   EXPECT_EQ(wrong.dpithreads(), "2");

   wrong.set_dpithreads("Hello World!");
   wrongString =wrong.SerializeAsString();
   master.ProcessBaseConfigRequest(conf,wrongString);
   EXPECT_EQ(wrong.has_dpithreads(), true); // copies are not cleared
   EXPECT_FALSE(master.ValidateConfFieldValues(conf,wrong,protoMsg::ConfType_Type_BASE));
}

/**
 * Sets all fields except for the given @param shouldFail. 
 * Setting the shouldField to a higher number than the number of fields
 * SHOULD make the validation to pass instead of fail
 */
namespace {
   const size_t gNumberOfFieldsLowerBound = 34;
   const size_t gNumberOfFieldsUpperBound = 34;
}

void ValidateAllFieldsSetInvalidOnXLowerBound(const size_t shouldFail) {
   size_t index = 0;
   MockConfMaster master;
   MockConf conf;
   master.mIgnoreConfValidate = false;
   master.mValidConfValidation = false;
   //conf.GetValidateConf().GetChecker().mValidCheck = false;

   protoMsg::BaseConf msg;

   const std::string validText = {"Hello Wold!"};
   const std::string invalidText(1001, 'x');

   const std::string validBool = {"false"};
   const std::string invalidBool = {"1"};

   (index++ == shouldFail) ? msg.set_dpithreads("0") : msg.set_dpithreads("1");
   (index++ == shouldFail) ? msg.set_pcapetimeout("0") : msg.set_pcapetimeout("1");
   (index++ == shouldFail) ? msg.set_pcapbuffersize("0") : msg.set_pcapbuffersize("1");
   (index++ == shouldFail) ? msg.set_pcapinterface(invalidText) : msg.set_pcapinterface(validText);
   (index++ == shouldFail) ? msg.set_dpihalfsessions("999999") : msg.set_dpihalfsessions("1000000");
   (index++ == shouldFail) ? msg.set_packetsendqueuesize("9") : msg.set_packetsendqueuesize("10");
   (index++ == shouldFail) ? msg.set_packetrecvqueuesize("9") : msg.set_packetrecvqueuesize("10");
   (index++ == shouldFail) ? msg.set_dpimsgsendqueuesize("9") : msg.set_dpimsgsendqueuesize("10");
   (index++ == shouldFail) ? msg.set_dpimsgrecvqueuesize("9") : msg.set_dpimsgrecvqueuesize("10");
   (index++ == shouldFail) ? msg.set_qosmosdebugmodeenabled(invalidBool) : msg.set_qosmosdebugmodeenabled(validBool);
   (index++ == shouldFail) ? msg.set_qosmos64bytepool("199999") : msg.set_qosmos64bytepool("200000");
   (index++ == shouldFail) ? msg.set_qosmos128bytepool("199999") : msg.set_qosmos128bytepool("200000");
   (index++ == shouldFail) ? msg.set_qosmos256bytepool("199999") : msg.set_qosmos256bytepool("200000");
   (index++ == shouldFail) ? msg.set_qosmos512bytepool("199999") : msg.set_qosmos512bytepool("200000");
   (index++ == shouldFail) ? msg.set_statsaccumulatorqueue(invalidText) : msg.set_statsaccumulatorqueue(validText);
   (index++ == shouldFail) ? msg.set_sendstatsqueue(invalidText) : msg.set_sendstatsqueue(validText);
   (index++ == shouldFail) ? msg.set_qosmosexpirepercallback("0") : msg.set_qosmosexpirepercallback("1");
   (index++ == shouldFail) ? msg.set_qosmostcpreassemblyenabled(invalidBool) : msg.set_qosmostcpreassemblyenabled(validBool);
   (index++ == shouldFail) ? msg.set_qosmosipdefragmentationenabled(invalidBool) : msg.set_qosmosipdefragmentationenabled(validBool);
   
   // next three are used in java only, cannot be changed through the UI
   (index++ == shouldFail) ? msg.set_dbclustername(invalidText) : msg.set_dbclustername(validText);
   (index++ == shouldFail) ? msg.set_dburl(invalidText) : msg.set_dburl(validText);
   (index++ == shouldFail) ? msg.set_statsaggregationqueue(invalidText) : msg.set_statsaggregationqueue(validText);

   (index++ == shouldFail) ? msg.set_commandqueue(invalidText) : msg.set_commandqueue(validText);
   (index++ == shouldFail) ? msg.set_enableintermediateflows(invalidBool) : msg.set_enableintermediateflows(validBool);
   (index++ == shouldFail) ? msg.set_enablepacketcapture(invalidBool) : msg.set_enablepacketcapture(validBool);

   (index++ == shouldFail) ? msg.set_capturefilelimit("999") : msg.set_capturefilelimit("1000");
   (index++ == shouldFail) ? msg.set_capturesizelimit("999") : msg.set_capturesizelimit("1000");
   (index++ == shouldFail) ? msg.set_capturememorylimit("999") : msg.set_capturememorylimit("1000");
   (index++ == shouldFail) ? msg.set_capturemaxpackets("999") : msg.set_capturemaxpackets("1000");
   (index++ == shouldFail) ? msg.set_captureindividualfilelimit("0") : msg.set_captureindividualfilelimit("1");

   (index++ == shouldFail) ? msg.set_syslogrecvqueuesize("9") : msg.set_syslogrecvqueuesize("10");
   (index++ == shouldFail) ? msg.set_syslogsendqueuesize("9") : msg.set_syslogsendqueuesize("10");
   
   (index++ == shouldFail) ? msg.set_pcaprecordstoclearpercycle("0") : msg.set_pcaprecordstoclearpercycle("1");
   (index++ == shouldFail) ? msg.set_flowreportinterval("59") : msg.set_flowreportinterval("60");
   
   // Test sanity check. Total number of used fields are :  34
   EXPECT_EQ(index, gNumberOfFieldsLowerBound) << "\t\t\t\t\t: Expected number of fields are "
           << gNumberOfFieldsLowerBound << " unless you added more?";

   if (shouldFail > gNumberOfFieldsLowerBound) {
      if (false == master.ValidateConfFieldValues(conf,msg,protoMsg::ConfType_Type_BASE)) {
         FAIL() << "\t\t\t\t\t: No fields should be invalid, 'shouldFail was: " << std::to_string(shouldFail);
         return;
      }
      SUCCEED();
      return;
   }

   SUCCEED();
}

void ValidateAllFieldsSetInvalidOnXUpperBound(const size_t shouldFail) {
   size_t index = 0;
   MockConfMaster master;
   MockConf conf;
   master.mIgnoreConfValidate = false;
   master.mValidConfValidation = false;
   conf.mOverrideGetPcapCaptureLocations = false;

   protoMsg::BaseConf msg;

   const std::string validText = {"Hello Wold!"};
   const std::string invalidText(1001, 'x');

   const std::string validBool = {"false"};
   const std::string invalidBool = {"1"};

   (index++ == shouldFail) ? msg.set_dpithreads("13") : msg.set_dpithreads("12");
   (index++ == shouldFail) ? msg.set_pcapetimeout("31") : msg.set_pcapetimeout("30");
   (index++ == shouldFail) ? msg.set_pcapbuffersize("1001") : msg.set_pcapbuffersize("1000");
   (index++ == shouldFail) ? msg.set_pcapinterface(invalidText) : msg.set_pcapinterface(validText);
   (index++ == shouldFail) ? msg.set_dpihalfsessions("20000001") : msg.set_dpihalfsessions("20000000");
   (index++ == shouldFail) ? msg.set_packetsendqueuesize("10001") : msg.set_packetsendqueuesize("10000");
   (index++ == shouldFail) ? msg.set_packetrecvqueuesize("10001") : msg.set_packetrecvqueuesize("10000");
   (index++ == shouldFail) ? msg.set_dpimsgsendqueuesize("100001") : msg.set_dpimsgsendqueuesize("100000");
   (index++ == shouldFail) ? msg.set_dpimsgrecvqueuesize("100001") : msg.set_dpimsgrecvqueuesize("100000");
   (index++ == shouldFail) ? msg.set_qosmosdebugmodeenabled(invalidBool) : msg.set_qosmosdebugmodeenabled(validBool);
   (index++ == shouldFail) ? msg.set_qosmos64bytepool("8000001") : msg.set_qosmos64bytepool("8000000");
   (index++ == shouldFail) ? msg.set_qosmos128bytepool("8000001") : msg.set_qosmos128bytepool("8000000");
   (index++ == shouldFail) ? msg.set_qosmos256bytepool("8000001") : msg.set_qosmos256bytepool("8000000");
   (index++ == shouldFail) ? msg.set_qosmos512bytepool("800001") : msg.set_qosmos512bytepool("800000");
   (index++ == shouldFail) ? msg.set_statsaccumulatorqueue(invalidText) : msg.set_statsaccumulatorqueue(validText);
   (index++ == shouldFail) ? msg.set_sendstatsqueue(invalidText) : msg.set_sendstatsqueue(validText);
   (index++ == shouldFail) ? msg.set_qosmosexpirepercallback("201") : msg.set_qosmosexpirepercallback("200");
   (index++ == shouldFail) ? msg.set_qosmostcpreassemblyenabled(invalidBool) : msg.set_qosmostcpreassemblyenabled(validBool);
   (index++ == shouldFail) ? msg.set_qosmosipdefragmentationenabled(invalidBool) : msg.set_qosmosipdefragmentationenabled(validBool);
   
   // next three are used in java only, cannot be changed through the UI
   (index++ == shouldFail) ? msg.set_dbclustername(invalidText) : msg.set_dbclustername(validText);
   (index++ == shouldFail) ? msg.set_dburl(invalidText) : msg.set_dburl(validText);
   (index++ == shouldFail) ? msg.set_statsaggregationqueue(invalidText) : msg.set_statsaggregationqueue(validText);

   (index++ == shouldFail) ? msg.set_commandqueue(invalidText) : msg.set_commandqueue(validText);
   (index++ == shouldFail) ? msg.set_enableintermediateflows(invalidBool) : msg.set_enableintermediateflows(validBool);
   (index++ == shouldFail) ? msg.set_enablepacketcapture(invalidBool) : msg.set_enablepacketcapture(validBool);

   (index++ == shouldFail) ? msg.set_capturefilelimit("2147483648") : msg.set_capturefilelimit("2147483647");

   ProtoDefaults getDefaults{conf.GetPcapCaptureLocations()};
   auto confDefaults = getDefaults.GetConfDefaults(protoMsg::ConfType_Type_BASE);
   auto rangePtr = getDefaults.GetRange(confDefaults, "captureSizeLimit"); // int
   auto captureSizeLimitTooMuch = std::to_string(1+ std::stoul(rangePtr->StringifyMax()));  
   (index++ == shouldFail) ? msg.set_capturesizelimit(captureSizeLimitTooMuch) : msg.set_capturesizelimit(rangePtr->StringifyMax());
   
   (index++ == shouldFail) ? msg.set_capturememorylimit("16001") : msg.set_capturememorylimit("16000");
   (index++ == shouldFail) ? msg.set_capturemaxpackets("2147483648") : msg.set_capturemaxpackets("2147483647");
   (index++ == shouldFail) ? msg.set_captureindividualfilelimit("2000001") : msg.set_captureindividualfilelimit("2000000");

   
   (index++ == shouldFail) ? msg.set_syslogrecvqueuesize("10001") : msg.set_syslogrecvqueuesize("10000");
   (index++ == shouldFail) ? msg.set_syslogsendqueuesize("10001") : msg.set_syslogsendqueuesize("10000");
     
   (index++ == shouldFail) ? msg.set_pcaprecordstoclearpercycle("20001") : msg.set_pcaprecordstoclearpercycle("20000");
   (index++ == shouldFail) ? msg.set_flowreportinterval("3601") : msg.set_flowreportinterval("3600");
   
   // Test sanity check. Total number of used fields are :  34
   EXPECT_EQ(index, gNumberOfFieldsUpperBound) << "\t\t\t\t\t: Expected number of fields are "
           << gNumberOfFieldsUpperBound << " unless you added more?";

   if (shouldFail > gNumberOfFieldsUpperBound) {
      if (false == master.ValidateConfFieldValues(conf,msg,protoMsg::ConfType_Type_BASE)) {
         FAIL() << "\t\t\t\t\t: No fields should be invalid, 'shouldFail was: " << std::to_string(shouldFail);
         return;
      }
      SUCCEED();
      return;
   }

   SUCCEED();
}

TEST_F(ConfProcessorTests, BaseConfValidationAllFieldsFailure) {
   for (size_t field = 0; field < gNumberOfFieldsLowerBound; ++field) {
      ValidateAllFieldsSetInvalidOnXLowerBound(field);
   }
   for (size_t field = 0; field < gNumberOfFieldsUpperBound; ++field) {
      ValidateAllFieldsSetInvalidOnXUpperBound(field);
   }
}

TEST_F(ConfProcessorTests, BaseConfValidationAllFieldsSuccess) {
   ValidateAllFieldsSetInvalidOnXLowerBound(gNumberOfFieldsLowerBound + 1);
   ValidateAllFieldsSetInvalidOnXUpperBound(gNumberOfFieldsUpperBound + 1);
}

TEST_F(ConfProcessorTests, BaseConfValidationInternalRepairBaseConf) {
   // using a real conf but without a real file so that it has bad values
   MockConfMaster master;
   MockConf conf("/tmp/I/am/not/here/woo.ls");
   master.mValidateEthFailCount = -1;
   master.mValidateEthFailCount = -1;
   conf.mOverridegetPCapInterface = false;
   auto check = master.InternallyRepairBaseConf(conf);
   EXPECT_EQ(check, true);
}

#endif //  LR_DEBUG
