
#include "ConfProcessorTests.h"
#include "Conf.h"
#include "MockConf.h"
#include "MockValidateConf.h"
#include "SyslogConfMsg.pb.h"
#include <g2log.hpp>
#include <functional>
#include <limits>

#ifdef LR_DEBUG
using namespace std;

namespace {
   Range gMax = {1, std::numeric_limits<uint32_t>::max()};
}
// Not set fields are NOT failure, they will just be ignored 
TEST_F(ConfProcessorTests, SyslogValidationBlankMsgWillSucceed) {
   MockConf conf;
   MockValidateConf validateConf;
   conf.mIgnoreSyslogConfValidation = false;
   
   EXPECT_EQ(validateConf.mValidConf, true);
   protoMsg::SyslogConf blank;
   EXPECT_EQ(blank.has_syslogenabled(), false);
   EXPECT_EQ(blank.has_syslogtcpenabled(), false);
   EXPECT_EQ(blank.has_sysloglogagentip(), false);
   EXPECT_EQ(blank.has_sysloglogagentport(), false);
   EXPECT_EQ(blank.has_syslogmaxlinelength(), false);
   EXPECT_EQ(blank.has_siemlogging(), false);
   EXPECT_EQ(blank.has_debugsiemlogging(), false);
   EXPECT_EQ(blank.has_scrubpasswords(), false);
   EXPECT_EQ(blank.has_reporteverything(), false);

   conf.updateFields(blank); // trigger Mocked ValidateBaseConf
   EXPECT_EQ(validateConf.mValidConf, true);   
}


  
   
/**
 * Sets all fields except for the given @param shouldFail. 
 * Setting the shouldField to a higher number than the number of fields
 * SHOULD make the validation to pass instead of fail
 */
namespace {
    const size_t gNumberOfFieldsLowerBound = 9;
    const size_t gNumberOfFieldsUpperBound = 9;
}
void SyslogValidateAllFieldsSetInvalidOnXLowerBound(const size_t shouldFail) {
   size_t index = 0;
   MockConf conf;
   MockValidateConf validateConf;
   conf.mIgnoreSyslogConfValidation = false;
   validateConf.mValidConf = false;
   
   protoMsg::SyslogConf msg; 
   
   const std::string validText = {"Hello Wold!"};
   const std::string invalidText(1001,'x');
   
   const std::string validBool = {"false"};
   const std::string invalidBool = {"1"};
   
   (index++ == shouldFail) ? msg.set_syslogenabled("0") : msg.set_syslogenabled("false");
   (index++ == shouldFail) ? msg.set_syslogtcpenabled("0") : msg.set_syslogtcpenabled("false");
   (index++ == shouldFail) ? msg.set_sysloglogagentip(invalidText) : msg.set_sysloglogagentip(validText); 
   (index++ == shouldFail) ? msg.set_sysloglogagentport("0"): msg.set_sysloglogagentport("1");
   (index++ == shouldFail) ? msg.set_syslogmaxlinelength("199") : msg.set_syslogmaxlinelength("200") ;
   (index++ == shouldFail) ? msg.set_siemlogging("0") : msg.set_siemlogging("false");
   (index++ == shouldFail) ? msg.set_debugsiemlogging("0") : msg.set_debugsiemlogging("false");
   (index++ == shouldFail) ? msg.set_scrubpasswords("0") : msg.set_scrubpasswords("false");
   (index++ == shouldFail) ? msg.set_reporteverything("0") : msg.set_reporteverything("false");
   
   // Test sanity check. Total number of used fields are :  32
   EXPECT_EQ(index, gNumberOfFieldsLowerBound) << "\t\t\t\t\t: Expected number of fields are " 
                                     << gNumberOfFieldsLowerBound << " unless you added more?";
   conf.updateFields(msg);

   if (shouldFail > gNumberOfFieldsLowerBound) {
      if(false == validateConf.mValidConf) {
         FAIL() << "\t\t\t\t\t: No fields should be invalid, 'shouldFail was: " << std::to_string(shouldFail);  
         return;
      }
      SUCCEED();
      return;
   }
   
   // We can only reach this if 'shouldFail' was less than number of fields
   // in this case me MUST have failed or else this test or Conf.cpp has changed
   //  (or is corrupt)
   if (true == validateConf.mValidConf) {
      FAIL() << "\t\t\t\t\t: One field should be invalid. 'shouldFail was: " << std::to_string(shouldFail);
      return;
   }
   
   SUCCEED();
}

void SyslogValidateAllFieldsSetInvalidOnXUpperBound(const size_t shouldFail) {
   size_t index = 0;
   MockConf conf;
   MockValidateConf validateConf;
   conf.mIgnoreSyslogConfValidation = false;
   
   protoMsg::SyslogConf msg; 
   
   const std::string validText = {"Hello Wold!"};
   const std::string invalidText(1001,'x');
   
   const std::string validBool = {"false"};
   const std::string invalidBool = {"1"};
   
   (index++ == shouldFail) ? msg.set_syslogenabled("1") : msg.set_syslogenabled("true");
   (index++ == shouldFail) ? msg.set_syslogtcpenabled("1") : msg.set_syslogtcpenabled("true");
   (index++ == shouldFail) ? msg.set_sysloglogagentip(invalidText) : msg.set_sysloglogagentip(validText); 
   (index++ == shouldFail) ? msg.set_sysloglogagentport("65536"): msg.set_sysloglogagentport("65535");
   (index++ == shouldFail) ? msg.set_syslogmaxlinelength("2001") : msg.set_syslogmaxlinelength("2000") ;
   (index++ == shouldFail) ? msg.set_siemlogging("1") : msg.set_siemlogging("true");
   (index++ == shouldFail) ? msg.set_debugsiemlogging("1") : msg.set_debugsiemlogging("true");
   (index++ == shouldFail) ? msg.set_scrubpasswords("1") : msg.set_scrubpasswords("true");
   (index++ == shouldFail) ? msg.set_reporteverything("1") : msg.set_reporteverything("true");
   
   // Test sanity check. Total number of used fields are :  32
   EXPECT_EQ(index, gNumberOfFieldsUpperBound) << "\t\t\t\t\t: Expected number of fields are " 
                                     << gNumberOfFieldsUpperBound << " unless you added more?";
   conf.updateFields(msg);

   if (shouldFail > gNumberOfFieldsUpperBound) {
      if(false == validateConf.mValidConf) {
         FAIL() << "\t\t\t\t\t: No fields should be invalid, 'shouldFail was: " << std::to_string(shouldFail);  
         return;
      }
      SUCCEED();
      return;
   }
   
   // We can only reach this if 'shouldFail' was less than number of fields
   // in this case me MUST have failed or else this test or Conf.cpp has changed
   //  (or is corrupt)
   if (true == validateConf.mValidConf) {
      FAIL() << "\t\t\t\t\t: One field should be invalid. 'shouldFail was: " << std::to_string(shouldFail);
      return;
   }
   
   SUCCEED();
}


TEST_F(ConfProcessorTests, SyslogConfValidationAllFieldsFailure) {
   for(size_t field = 0; field <  gNumberOfFieldsLowerBound; ++field) {
      SyslogValidateAllFieldsSetInvalidOnXLowerBound(field);
   }
   for(size_t field = 0; field <  gNumberOfFieldsUpperBound; ++field) {
      SyslogValidateAllFieldsSetInvalidOnXUpperBound(field);
   }
}

TEST_F(ConfProcessorTests, SyslogConfValidationAllFieldsSuccess) {
   SyslogValidateAllFieldsSetInvalidOnXLowerBound(gNumberOfFieldsLowerBound+1);
   SyslogValidateAllFieldsSetInvalidOnXUpperBound(gNumberOfFieldsUpperBound+1);


}

#endif //  LR_DEBUG

