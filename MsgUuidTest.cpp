#include <string>
#include "MsgUuidTest.h"
#include "MockMsgUuid.h"
using namespace networkMonitor;
using namespace std;


TEST_F( MsgUuidTests, MsgUuidSingletonInstance )
{

	MsgUuid& mu1 = MsgUuid::Instance();
	MsgUuid& mu2 = MsgUuid::Instance();

   EXPECT_EQ( &mu1, &mu2 );
}

TEST_F( MsgUuidTests, MsgUuidGetMsgUuid )
{

	MsgUuid& mu1 = MsgUuid::Instance();

   string su = mu1.GetMsgUuid();

   // UUID string is 36 bytes long and has the format:
   // xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx where x is any hexadecimal digit
   // and y is one of 8, 9, A, or B. 
   EXPECT_EQ( 36, su.size() );

   EXPECT_EQ( su[14], '4' );

   EXPECT_TRUE(    su[19] == '8' 
                || su[19] == '9'
                || su[19] == 'a'
                || su[19] == 'b'
                || su[19] == 'A'
                || su[19] == 'B' );
}
TEST_F( MsgUuidTests, DifferentSeeds) {
   MockMsgUuid uuid1;
   MockMsgUuid uuid2;
   
   std::string str1 = uuid1.GetMsgUuid();
   std::string str2 = uuid2.GetMsgUuid();
   
   ASSERT_NE(str1,str2);
}

