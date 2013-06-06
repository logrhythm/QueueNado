/* 
 * File:   QosmosProtocolCaptureTest.cpp
 * Author: John Gress
 * 
 * Created on November 26, 2012, 4:30 PM
 */

#include "QosmosProtocolCaptureTest.h"
#include "QosmosConf.h"

using namespace std;

TEST_F( QosmosProtocolCaptureTest, SingletonInstantiation ) {
   QosmosProtocolCapture& qpc1 = QosmosProtocolCapture::Instance();
   QosmosProtocolCapture& qpc2 = QosmosProtocolCapture::Instance();
   EXPECT_EQ( &qpc1, &qpc2 );
}

TEST_F( QosmosProtocolCaptureTest, AddingProtocolsAndFamilies ) {
   QosmosProtocolCapture& qpc = QosmosProtocolCapture::Instance();
   const char * protocolName = "ftp";
   const char * protocolLongName = "File Transfer Protocol";
   const char * familyName = "File Server";
   unsigned int enabled = 1;

   qpc.AddProtocolFamilyNames( protocolName, protocolLongName,
                               familyName, enabled, 1 );

   ProtocolFamilyMap pfm = qpc.GetProtocolFamilies();
   EXPECT_EQ( 1, pfm.size() );

   auto pfmIt = pfm.begin();
   EXPECT_EQ( protocolName, pfmIt->first );
   EXPECT_EQ( familyName, pfmIt->second.first );
   EXPECT_EQ( protocolLongName, pfmIt->second.second );

   ProtocolEnabledMap pem = qpc.GetProtocolEnabled();
   EXPECT_EQ( 1, pem.size() );
   auto pemIt = pem.begin();
   EXPECT_EQ( protocolName, pemIt->first );
   EXPECT_EQ( enabled, pemIt->second );
}

