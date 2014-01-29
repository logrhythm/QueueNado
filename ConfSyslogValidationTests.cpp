
#include "ConfProcessorTests.h"
#include "Conf.h"
#include "MockConf.h"
#include "MockConfMaster.h"
#include "SyslogConfMsg.pb.h"
#include <g2log.hpp>
#include <functional>
#include <limits>

#ifdef LR_DEBUG
using namespace std;

// Not set fields are NOT failure, they will just be ignored 

TEST_F(ConfProcessorTests, SyslogValidationBlankMsgWillSucceed) {
   MockConfMaster master;
   MockConf conf;
   master.mIgnoreConfValidate = false;
   EXPECT_TRUE(master.mValidConfValidation);
   
   protoMsg::SyslogConf blank;
   EXPECT_EQ(blank.has_syslogenabled(), false);
   EXPECT_EQ(blank.has_syslogtcpenabled(), false);
   EXPECT_EQ(blank.has_sysloglogagentip(), false);
   EXPECT_EQ(blank.has_sysloglogagentport(), false);
   EXPECT_EQ(blank.has_syslogmaxlinelength(), false);
   EXPECT_EQ(blank.has_siemlogging(), false);
   EXPECT_EQ(blank.has_debugsiemlogging(), false);
   EXPECT_EQ(blank.has_scrubpasswords(), false);

   EXPECT_TRUE(master.ValidateConfFieldValues(conf,blank,protoMsg::ConfType_Type_SYSLOG)); // trigger Mocked ValidateConfFieldValues
}

/**
 * Sets all fields except for the given @param shouldFail. 
 * Setting the shouldField to a higher number than the number of fields
 * SHOULD make the validation to pass instead of fail
 */
namespace {
   const size_t gNumberOfFieldsLowerBound = 8;
   const size_t gNumberOfFieldsUpperBound = 8;
}

void SyslogValidateAllFieldsSetInvalidOnXLowerBound(const size_t shouldFail) {
   size_t index = 0;
   MockConfMaster master;
   MockConf conf;
   master.mIgnoreConfValidate = false;
   master.mValidConfValidation = false;

   protoMsg::SyslogConf msg;

   const std::string validText = {"Hello Wold!"};
   const std::string invalidText(1001, 'x');

   const std::string validBool = {"false"};
   const std::string invalidBool = {"1"};

   (index++ == shouldFail) ? msg.set_syslogenabled("0") : msg.set_syslogenabled("false");
   (index++ == shouldFail) ? msg.set_syslogtcpenabled("0") : msg.set_syslogtcpenabled("false");
   (index++ == shouldFail) ? msg.set_sysloglogagentip(invalidText) : msg.set_sysloglogagentip(validText);
   (index++ == shouldFail) ? msg.set_sysloglogagentport("0") : msg.set_sysloglogagentport("1");
   (index++ == shouldFail) ? msg.set_syslogmaxlinelength("199") : msg.set_syslogmaxlinelength("200");
   (index++ == shouldFail) ? msg.set_siemlogging("0") : msg.set_siemlogging("false");
   (index++ == shouldFail) ? msg.set_debugsiemlogging("0") : msg.set_debugsiemlogging("false");
   (index++ == shouldFail) ? msg.set_scrubpasswords("0") : msg.set_scrubpasswords("false");

   // Test sanity check. Total number of used fields are :  32
   EXPECT_EQ(index, gNumberOfFieldsLowerBound) << "\t\t\t\t\t: Expected number of fields are "
           << gNumberOfFieldsLowerBound << " unless you added more?";

   if (shouldFail > gNumberOfFieldsLowerBound) {
      if (false == master.ValidateConfFieldValues(conf,msg,protoMsg::ConfType_Type_SYSLOG)) {
         FAIL() << "\t\t\t\t\t: No fields should be invalid, 'shouldFail was: " << std::to_string(shouldFail);
         return;
      }
      SUCCEED();
      return;
   }

   SUCCEED();
}

void SyslogValidateAllFieldsSetInvalidOnXUpperBound(const size_t shouldFail) {
   size_t index = 0;
   MockConfMaster master;
   MockConf conf;
   master.mIgnoreConfValidate = false;

   protoMsg::SyslogConf msg;

   const std::string validText = {"Hello Wold!"};
   const std::string invalidText(1001, 'x');

   const std::string validBool = {"false"};
   const std::string invalidBool = {"1"};

   (index++ == shouldFail) ? msg.set_syslogenabled("1") : msg.set_syslogenabled("true");
   (index++ == shouldFail) ? msg.set_syslogtcpenabled("1") : msg.set_syslogtcpenabled("true");
   (index++ == shouldFail) ? msg.set_sysloglogagentip(invalidText) : msg.set_sysloglogagentip(validText);
   (index++ == shouldFail) ? msg.set_sysloglogagentport("65536") : msg.set_sysloglogagentport("65535");
   (index++ == shouldFail) ? msg.set_syslogmaxlinelength("2001") : msg.set_syslogmaxlinelength("2000");
   (index++ == shouldFail) ? msg.set_siemlogging("1") : msg.set_siemlogging("true");
   (index++ == shouldFail) ? msg.set_debugsiemlogging("1") : msg.set_debugsiemlogging("true");
   (index++ == shouldFail) ? msg.set_scrubpasswords("1") : msg.set_scrubpasswords("true");

   // Test sanity check. Total number of used fields are :  32
   EXPECT_EQ(index, gNumberOfFieldsUpperBound) << "\t\t\t\t\t: Expected number of fields are "
           << gNumberOfFieldsUpperBound << " unless you added more?";

   if (shouldFail > gNumberOfFieldsUpperBound) {
      if (false == master.ValidateConfFieldValues(conf,msg,protoMsg::ConfType_Type_SYSLOG)) {
         FAIL() << "\t\t\t\t\t: No fields should be invalid, 'shouldFail was: " << std::to_string(shouldFail);
         return;
      }
      SUCCEED();
      return;
   }


   SUCCEED();
}

TEST_F(ConfProcessorTests, SyslogConfValidationAllFieldsFailure) {
   for (size_t field = 0; field < gNumberOfFieldsLowerBound; ++field) {
      SyslogValidateAllFieldsSetInvalidOnXLowerBound(field);
   }
   for (size_t field = 0; field < gNumberOfFieldsUpperBound; ++field) {
      SyslogValidateAllFieldsSetInvalidOnXUpperBound(field);
   }
}

TEST_F(ConfProcessorTests, SyslogConfValidationAllFieldsSuccess) {
   SyslogValidateAllFieldsSetInvalidOnXLowerBound(gNumberOfFieldsLowerBound + 1);
   SyslogValidateAllFieldsSetInvalidOnXUpperBound(gNumberOfFieldsUpperBound + 1);
}

#endif //  LR_DEBUG

