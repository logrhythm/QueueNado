#include <stdlib.h>
#include "DpiMsgLR.h"
#include "DpiMsgLRTest.h"
#include "g2log.hpp"
#include "czmq.h"
#include <pcap/pcap.h>
#include <google/protobuf/message.h>
#include <google/protobuf/generated_message_reflection.h>

using namespace networkMonitor;
using namespace google::protobuf;
using namespace std;


TEST_F(DpiMsgLRTests, EstimatePCapFileSize) {
   DpiMsgLR testMsg;
   
   testMsg.set_session("123456789012345678901234567890123456");
   testMsg.set_totalpackets(0);
   testMsg.set_srcbytes(0);
   testMsg.set_destbytes(0);
   
   EXPECT_EQ(sizeof(pcap_file_header),testMsg.TheoreticalPCapFileSize());
   testMsg.set_totalpackets(10);
   EXPECT_EQ(sizeof(pcap_file_header)+10*sizeof(pcap_pkthdr),testMsg.TheoreticalPCapFileSize());
   testMsg.set_srcbytes(100);
   EXPECT_EQ(sizeof(pcap_file_header)+10*sizeof(pcap_pkthdr)+100,testMsg.TheoreticalPCapFileSize());
   testMsg.set_destbytes(1000);
   EXPECT_EQ(sizeof(pcap_file_header)+10*sizeof(pcap_pkthdr)+100+1000,testMsg.TheoreticalPCapFileSize());
}
TEST_F(DpiMsgLRTests, EncodeJSonValgrind ) {
   DpiMsgLR testMsg;
   
   testMsg.set_session("ABC123");
   testMsg.add_uri_fullq_proto_http("1");
   testMsg.add_application_endq_proto_base("test");
   testMsg.set_application_id_endq_proto_base(1234);
   testMsg.set_destbytes(567);
   testMsg.set_destbytesdelta(567);
   testMsg.set_srcbytes(899);
   testMsg.set_srcbytesdelta(899);
   testMsg.set_totalpackets(88);
   testMsg.set_packetsdelta(88);
   testMsg.add_loginq_proto_aim("aLogin");
   testMsg.add_domainq_proto_smb("aDomain12345");
   testMsg.add_uriq_proto_http("not/this/one");
   testMsg.add_serverq_proto_http("thisname12345");
   testMsg.add_referer_serverq_proto_http("notThisOne");
   testMsg.add_methodq_proto_ftp("RUN");
   testMsg.add_methodq_proto_ftp("COMMAND");
   testMsg.add_methodq_proto_ftp("LONGLONGLONGLONG");
   testMsg.add_senderq_proto_smtp("test1_123456");
   testMsg.add_receiverq_proto_smtp("test2_123");
   testMsg.add_subjectq_proto_smtp("test3_12345");
   testMsg.add_versionq_proto_http("4.0");
   
   for ( int i = 0 ; i < 10000 && !zctx_interrupted; i++ ) {
      std::string updateAsString = testMsg.EncodeAsJSON();
   }
}
TEST_F(DpiMsgLRTests, SetTimeUpdated) {
   DpiMsgLR dm;
   
   dm.SetTimeUpdated(123456789);
   EXPECT_FALSE(dm.has_timeprevious());
   EXPECT_EQ(123456789,dm.timeupdated());
   dm.SetTimeUpdated(123);
   ASSERT_TRUE(dm.has_timeprevious());
   EXPECT_EQ(123456789,dm.timeprevious());
   EXPECT_EQ(123,dm.timeupdated());
}
TEST_F(DpiMsgLRTests, GetESIndexName) {
   DpiMsgLR dm;
   dm.set_session("TESTTEST");
   EXPECT_EQ("",dm.GetESIndexName());
   dm.set_timeupdated(123456789);
   EXPECT_EQ("network_1973_11_29",dm.GetESIndexName());
}

TEST_F(DpiMsgLRTests, GetESDocId) {
   DpiMsgLR dm;
   dm.set_session("TESTTEST");
   EXPECT_EQ("TESTTEST", dm.GetESDocId());
   dm.set_childflownumber(1234);
   EXPECT_EQ("TESTTEST_1234", dm.GetESDocId());
}

TEST_F(DpiMsgLRTests, ParsePathTest) {
   DpiMsgLR dm;

   EXPECT_EQ("", dm.GetClassificationPath());
   dm.add_applicationq_proto_base("test");
   EXPECT_EQ("/test", dm.GetClassificationPath());
   //   dm.add_applicationq_proto_base("test");
   //   EXPECT_EQ("/test", dm.GetClassificationPath());
   dm.add_applicationq_proto_base("bar");
   EXPECT_EQ("/test/bar", dm.GetClassificationPath());
   dm.add_applicationq_proto_base("test");
   EXPECT_EQ("/test/bar/test", dm.GetClassificationPath());
   //   dm.add_applicationq_proto_base("bar");
   //   EXPECT_EQ("/test/bar", dm.GetClassificationPath());
}

TEST_F(DpiMsgLRTests, setUuidSuccess) {
   // Set the Unique ID
   string uuid("01234567-89ab-cdef-0123456789abcdef");
   DpiMsgLR dm;
   dm.set_session(uuid);

   // Get the Unique ID from the object
   string rUuid(dm.session());

   // Expect the two values to match
   EXPECT_EQ(uuid, rUuid);
}

TEST_F(DpiMsgLRTests, ConstructDeconstruct) {

   DpiMsgLR* dm = new DpiMsgLR;
   delete dm;
}

/**
 * Test basic password scrubbing enabled.
 */
TEST_F(DpiMsgLRTests, enablePasswordScrubbing) {
   DpiMsgLR* dm = new DpiMsgLR;
   dm->EnablePasswordScrubbing(true);
   dm->set_session("01234567-89ab-cdef-0123456789abcdef");
   dm->add_passwordq_proto_ftp("wooo");
   IndexedFieldPairs pairs = dm->GetAllFieldsAsStrings();
   ASSERT_EQ(2, pairs.size());
   IndexedFieldPairs::const_iterator it;
   for (it = pairs.begin(); it != pairs.end(); ++it) {
      const std::string key((*it).second.first);
      const std::string value((*it).second.second);
      LOG(DEBUG) << "Key: " << key;
      LOG(DEBUG) << "Value: " << value;
      if (key.find("uuid") != std::string::npos) {
         ASSERT_EQ("01234567-89ab-cdef-0123456789abcdef", value);
      } else if (key.find("password") != std::string::npos) {
         ASSERT_EQ("********", value);
      }
   }

   delete dm;
}

TEST_F(DpiMsgLRTests, setIPV6SrcSuccess) {
   uint8_t ipv6Addr[16];

   ipv6Addr[0] = 0xfe;
   ipv6Addr[1] = 0x80;
   ipv6Addr[2] = 0x00;
   ipv6Addr[3] = 0x00;
   ipv6Addr[4] = 0x00;
   ipv6Addr[5] = 0x00;
   ipv6Addr[6] = 0x00;
   ipv6Addr[7] = 0x00;
   ipv6Addr[8] = 0x38;
   ipv6Addr[9] = 0xff;
   ipv6Addr[10] = 0x05;
   ipv6Addr[11] = 0xfa;
   ipv6Addr[12] = 0x6b;
   ipv6Addr[13] = 0x76;
   ipv6Addr[14] = 0x87;
   ipv6Addr[15] = 0x0c;

   DpiMsgLR dm;
   uint32_t outVal;
   vector<uint32_t> inpIp6Src;
   for (int i = 0, j = 0; i < 16 && j < 8 ; i = i + 2, ++j) {
      outVal = ipv6Addr[i];
      outVal = outVal << 8;
      outVal |= ipv6Addr[i + 1];
      inpIp6Src.push_back(outVal);
   }
   dm.SetSrcIP6(inpIp6Src);

   vector<uint32_t> rIPV6Src;
   rIPV6Src.push_back(0x00);
   rIPV6Src.push_back(0x00);
   rIPV6Src.push_back(0x00);
   rIPV6Src.push_back(0x00);
   rIPV6Src.push_back(0x00);
   rIPV6Src.push_back(0x00);
   rIPV6Src.push_back(0x00);
   rIPV6Src.push_back(0x00);

   dm.GetSrcIP6(rIPV6Src);

   // Expect the values in the two arrays to match
   for (int tInd = 0; tInd < IPV6_ADDR_SIZE; tInd++) {
      EXPECT_EQ(rIPV6Src[tInd], inpIp6Src[tInd]);
   }
}

TEST_F(DpiMsgLRTests, setIPV6DstSuccess) {
   uint8_t ipv6Addr[16];

   ipv6Addr[0] = 0xfe;
   ipv6Addr[1] = 0x80;
   ipv6Addr[2] = 0x00;
   ipv6Addr[3] = 0x00;
   ipv6Addr[4] = 0x00;
   ipv6Addr[5] = 0x00;
   ipv6Addr[6] = 0x00;
   ipv6Addr[7] = 0x00;
   ipv6Addr[8] = 0x38;
   ipv6Addr[9] = 0xff;
   ipv6Addr[10] = 0x05;
   ipv6Addr[11] = 0xfa;
   ipv6Addr[12] = 0x6b;
   ipv6Addr[13] = 0x76;
   ipv6Addr[14] = 0x87;
   ipv6Addr[15] = 0x0c;

   DpiMsgLR dm;
   uint32_t outVal;
   vector<uint32_t> inpIp6Dst;
   for (int i = 0, j = 0; i < 16 && j < 8 ; i = i + 2, ++j) {
      outVal = ipv6Addr[i];
      outVal = outVal << 8;
      outVal |= ipv6Addr[i + 1];
      inpIp6Dst.push_back(outVal);
   }
   dm.SetDstIP6(inpIp6Dst);

   vector<uint32_t> rIPV6Dst;
   rIPV6Dst.push_back(0x00);
   rIPV6Dst.push_back(0x00);
   rIPV6Dst.push_back(0x00);
   rIPV6Dst.push_back(0x00);
   rIPV6Dst.push_back(0x00);
   rIPV6Dst.push_back(0x00);
   rIPV6Dst.push_back(0x00);
   rIPV6Dst.push_back(0x00);

   dm.GetDstIP6(rIPV6Dst);

   // Expect the values in the two arrays to match
   for (int tInd = 0; tInd < IPV6_ADDR_SIZE; tInd++) {
      EXPECT_EQ(rIPV6Dst[tInd], inpIp6Dst[tInd]);
   }
}

TEST_F(DpiMsgLRTests, compareIpV6Addr) {
   vector<uint32_t> rIPV6Src;
   vector<uint32_t> rIPV6Dst, rIPV6Dst1;

   rIPV6Src.push_back(0x00);
   rIPV6Src.push_back(0x01);
   rIPV6Src.push_back(0x02);
   rIPV6Src.push_back(0x03);
   rIPV6Src.push_back(0x04);
   rIPV6Src.push_back(0x05);
   rIPV6Src.push_back(0x06);
   rIPV6Src.push_back(0x07);


   rIPV6Dst.push_back(0x00);
   rIPV6Dst.push_back(0x01);
   rIPV6Dst.push_back(0x02);
   rIPV6Dst.push_back(0x03);
   rIPV6Dst.push_back(0x04);
   rIPV6Dst.push_back(0x05);
   rIPV6Dst.push_back(0x06);
   rIPV6Dst.push_back(0x07);


   rIPV6Dst1.push_back(0x07);
   rIPV6Dst1.push_back(0x06);
   rIPV6Dst1.push_back(0x05);
   rIPV6Dst1.push_back(0x04);
   rIPV6Dst1.push_back(0x03);
   rIPV6Dst1.push_back(0x02);
   rIPV6Dst1.push_back(0x01);
   rIPV6Dst1.push_back(0x00);

   EXPECT_EQ(rIPV6Src.size(), rIPV6Dst.size());
   EXPECT_EQ(rIPV6Src.size(), rIPV6Dst1.size());
   bool is_equal = false;

   is_equal = std::equal(rIPV6Src.begin(), rIPV6Src.end(), rIPV6Dst.begin());
   EXPECT_TRUE(is_equal);

   is_equal = std::equal(rIPV6Src.begin(), rIPV6Src.end(), rIPV6Dst1.begin());
   EXPECT_FALSE(is_equal);
}

TEST_F(DpiMsgLRTests, setEthSrcSuccess) {
   // Set the Ethernet Source
   vector<unsigned char> ethSrc;
   ethSrc.push_back(0x00);
   ethSrc.push_back(0x50);
   ethSrc.push_back(0x56);
   ethSrc.push_back(0xBE);
   ethSrc.push_back(0x00);
   ethSrc.push_back(0x1C);
   DpiMsgLR dm;
   dm.SetEthSrc(ethSrc);

   // Get the Ethernet Source
   vector<unsigned char> rEthSrc;
   rEthSrc.push_back(0x00);
   rEthSrc.push_back(0x00);
   rEthSrc.push_back(0x00);
   rEthSrc.push_back(0x00);
   rEthSrc.push_back(0x00);
   rEthSrc.push_back(0x00);
   dm.GetEthSrc(rEthSrc);

   // Expect the values in the two arrays to match
   for (int tInd = 0; tInd < ETH_ADDR_SIZE; tInd++) {
      EXPECT_EQ(rEthSrc[tInd], ethSrc[tInd]);
   }
}

TEST_F(DpiMsgLRTests, setEthDstSuccess) {
   // Set the Ethernet Destination
   vector<unsigned char> ethDst;
   ethDst.push_back(0xF0);
   ethDst.push_back(0xF7);
   ethDst.push_back(0x55);
   ethDst.push_back(0xDC);
   ethDst.push_back(0xDC);
   ethDst.push_back(0xDC);

   DpiMsgLR dm;
   dm.SetEthDst(ethDst);

   // Get the Ethernet Destination
   vector<unsigned char> rEthDst;
   rEthDst.push_back(0x00);
   rEthDst.push_back(0x00);
   rEthDst.push_back(0x00);
   rEthDst.push_back(0x00);
   rEthDst.push_back(0x00);
   rEthDst.push_back(0x00);
   dm.GetEthDst(rEthDst);

   // Expect the values in the two arrays to match
   for (int tInd = 0; tInd < ETH_ADDR_SIZE; tInd++) {
      EXPECT_EQ(rEthDst[tInd], ethDst[tInd]);
   }
}

TEST_F(DpiMsgLRTests, setIpSrcSuccess) {
   // Set the IP Source Address
   uint32_t ipSrc = 3232235876; // 192.168.1.100
   DpiMsgLR dm;
   dm.set_srcip(ipSrc);

   // Get the IP Source Address
   uint32_t rIpSrc = dm.srcip();

   // Expect the two address to match
   EXPECT_EQ(ipSrc, rIpSrc);
}

TEST_F(DpiMsgLRTests, setIpDstSuccess) {
   // Set the IP Destination Address
   uint32_t ipDst = 167837813; // 10.1.0.75
   DpiMsgLR dm;
   dm.set_destip(ipDst);

   // Get the IP Destination Address
   uint32_t rIpDst = dm.destip();

   // Expect the two address to match
   EXPECT_EQ(ipDst, rIpDst);
}

TEST_F(DpiMsgLRTests, setPacketPathSuccess) {
   // Set the Packet Path
   std::string path("eth.ip.tcp.http");
   DpiMsgLR dm;
   dm.set_packetpath(path);

   // Get the Packet Path
   std::string rPath = dm.packetpath();

   // Expect the two strings to match
   EXPECT_EQ(path, rPath);
}

TEST_F(DpiMsgLRTests, ReadIPSrcDstSPortDPortProto) {
   char charbuffer[100];
   memset(charbuffer, 0, 100);
   charbuffer[16] = '9';
   charbuffer[17] = '0';
   charbuffer[18] = 'a';
   charbuffer[19] = 'f';
   charbuffer[20] = '5';
   charbuffer[21] = 'b';
   charbuffer[22] = '4';
   charbuffer[23] = '1';
#ifdef LR_DEBUG
   tDpiMessage.ReadIPSrcDstSPortDPortProto(charbuffer);
   EXPECT_EQ(0x90af, tDpiMessage.srcport());
   EXPECT_EQ(0x5b41, tDpiMessage.destport());
#endif
}

TEST_F(DpiMsgLRTests, GetSpecialIntegersAsStrings) {
#ifdef LR_DEBUG
   MockDpiMsgLR dm;

   dm.set_srcip(0x0a0b0c0d);
   EXPECT_EQ("13.12.11.10", dm.GetIpSrcString());
   dm.set_destip(0x01020304);
   EXPECT_EQ("4.3.2.1", dm.GetIpDstString());
   dm.set_srcmac(0x00000a0b0c0d0e0f);
   EXPECT_EQ("0f:0e:0d:0c:0b:0a", dm.GetEthSrcString());
   dm.set_destmac(0x0000010203040506);
   EXPECT_EQ("06:05:04:03:02:01", dm.GetEthDstString());
#endif
}

TEST_F(DpiMsgLRTests, FlowSessionCount) {
#ifdef LR_DEBUG
   MockDpiMsgLR dm;

   EXPECT_EQ(0, dm.GetFlowSessionCnt());
   dm.IncrFlowSessionCnt();
   EXPECT_EQ(1, dm.GetFlowSessionCnt());
   dm.DecrFlowSessionCnt();
   EXPECT_EQ(0, dm.GetFlowSessionCnt());
#endif
}

TEST_F(DpiMsgLRTests, GetUuidPair) {
   EXPECT_EQ("none", tDpiMessage.GetUuidPair().second);
   tDpiMessage.set_session("01234567-89ab-cdef-0123456789abcdef");
   EXPECT_EQ("UUID", tDpiMessage.GetUuidPair().first);
   EXPECT_EQ("01234567-89ab-cdef-0123456789abcdef",
           tDpiMessage.GetUuidPair().second);
}

TEST_F(DpiMsgLRTests, GetEthSrcPair) {
   EXPECT_EQ("00:00:00:00:00:00", tDpiMessage.GetEthSrcPair().second);
   tDpiMessage.set_srcmac(0x00000a0b0c0d0e0f);
   EXPECT_EQ("EthSrc", tDpiMessage.GetEthSrcPair().first);
   EXPECT_EQ("0f:0e:0d:0c:0b:0a", tDpiMessage.GetEthSrcPair().second);
}

TEST_F(DpiMsgLRTests, GetEthDstPair) {
   EXPECT_EQ("00:00:00:00:00:00", tDpiMessage.GetEthDstPair().second);
   tDpiMessage.set_destmac(0x0000010203040506);
   EXPECT_EQ("EthDst", tDpiMessage.GetEthDstPair().first);
   EXPECT_EQ("06:05:04:03:02:01", tDpiMessage.GetEthDstPair().second);
}

TEST_F(DpiMsgLRTests, GetIpSrcPair) {
   EXPECT_EQ("00.00.00.00", tDpiMessage.GetIpSrcPair().second);
   tDpiMessage.set_srcip(0x0a0b0c0d);

   EXPECT_EQ("IpSrc", tDpiMessage.GetIpSrcPair().first);
   EXPECT_EQ("13.12.11.10", tDpiMessage.GetIpSrcPair().second);

}

TEST_F(DpiMsgLRTests, GetIpDstPair) {
   EXPECT_EQ("00.00.00.00", tDpiMessage.GetIpDstPair().second);
   tDpiMessage.set_destip(0x01020304);

   EXPECT_EQ("IpDst", tDpiMessage.GetIpDstPair().first);
   EXPECT_EQ("4.3.2.1", tDpiMessage.GetIpDstPair().second);

}

TEST_F(DpiMsgLRTests, GetPacketPathPair) {
   EXPECT_EQ(EMPTY, tDpiMessage.GetPacketPathPair().second);
   string path = "foo.bar";
   tDpiMessage.set_packetpath(path);
   EXPECT_EQ("Path", tDpiMessage.GetPacketPathPair().first);
   EXPECT_EQ("foo.bar", tDpiMessage.GetPacketPathPair().second);

}

TEST_F(DpiMsgLRTests, GetSourcePortPair) {
   EXPECT_EQ("0", tDpiMessage.GetSourcePortPair().second);
   tDpiMessage.set_srcport(1234);
   EXPECT_EQ("SourcePort", tDpiMessage.GetSourcePortPair().first);
   EXPECT_EQ("1234", tDpiMessage.GetSourcePortPair().second);

}

TEST_F(DpiMsgLRTests, GetDestPortPair) {
   EXPECT_EQ("0", tDpiMessage.GetDestPortPair().second);
   tDpiMessage.set_destport(5678);
   EXPECT_EQ("DestPort", tDpiMessage.GetDestPortPair().first);
   EXPECT_EQ("5678", tDpiMessage.GetDestPortPair().second);
}

TEST_F(DpiMsgLRTests, GetStaticFieldPairs) {
   IndexedFieldPairs results;
   tDpiMessage.GetStaticFieldPairs(results);
   ASSERT_EQ(1, results.size());
   EXPECT_EQ("UUID", results[1].first);
   tDpiMessage.set_destmac(123);
   tDpiMessage.set_srcmac(123);
   tDpiMessage.set_destip(123);
   tDpiMessage.set_srcip(123);
   tDpiMessage.set_packetpath("foo");
   tDpiMessage.set_srcport(123);
   tDpiMessage.set_destport(123);
   tDpiMessage.set_flowcompleted(true);
   tDpiMessage.GetStaticFieldPairs(results);
   ASSERT_EQ(9, results.size());

   EXPECT_EQ("EthSrc", results[2].first);
   EXPECT_EQ("EthDst", results[3].first);
   EXPECT_EQ("IpSrc", results[4].first);
   EXPECT_EQ("IpDst", results[5].first);
   EXPECT_EQ("Path", results[6].first);
   EXPECT_EQ("SourcePort", results[8].first);
   EXPECT_EQ("DestPort", results[9].first);
   EXPECT_EQ("FlowCompleted", results[10].first);
}

// Some of the fields will be camelcase and some of the fields will be 
// lowercase-underscore separated
//
// See DpiMsgLR.h/cpp to see which ones are hard coded. The hard coded
// are CamelCase and the others will be with underscore_separation
TEST_F(DpiMsgLRTests, GetDynamicFieldPairs) {
   std::map<unsigned int, std::pair<std::string, std::string> > results;
   std::map<string, string> expecteds;
   tDpiMessage.GetDynamicFieldPairs(results);
   ASSERT_EQ(0, results.size());

   int chunk = 1234;
   string chunks = std::to_string(chunk);
   tDpiMessage.set_file_chunk_data_offsetq_proto_bittorrent(chunk);
   expecteds["file_chunk_data_offset"] = chunks;

   int filesize = 7890;
   string filesizes = std::to_string(filesize);
   tDpiMessage.set_filesizeq_proto_edonkey(filesize);
   expecteds["filesize"] = filesizes;

   string filename = "this is a file";
   tDpiMessage.add_attach_filenameq_proto_facebook_mail(filename);
   tDpiMessage.GetDynamicFieldPairs(results);
   expecteds["attach_filename"] = filename;

   ASSERT_EQ(3, results.size());
   for (auto i = results.begin(); i != results.end(); ++i) {
      string fieldName = i->second.first;
      string fieldValue = i->second.second;
      EXPECT_EQ(expecteds[fieldName], fieldValue) << "name/value: [" << fieldName << "]/[" << fieldValue << "]";
   }
}

TEST_F(DpiMsgLRTests, GetPossiblyRepeatedString) {
   DpiMsgLR dm;
   const Reflection* msgReflection = dm.GetReflection();
   IndexedFieldPairs tempFieldData;
   vector<const FieldDescriptor*> allFields;

   std::map<string, int> expectedint;
   std::map<string, string> expectedstring;
   int k = 0;
   int ldapProto = 20;
   int messageSize = 100;

   dm.set_message_idq_proto_ldap(ldapProto);
   dm.set_messagesize(messageSize);
   dm.set_flowcompleted(true);

   expectedint["message_idQ_PROTO_LDAP"] =  ldapProto;  // CPPTYPE_UINT32
   expectedint["MessageSize"] =  messageSize; // this is CPPTYPE_UINT64

   expectedstring["FlowCompleted"] =  "true";  // CPPTYPE_BOOL

   msgReflection->ListFields(dm, &allFields);
   for (auto j = allFields.begin(); j != allFields.end(); j++) {
      std::string key = (*j)->name();
      dm.PopulatePossiblyRepeatedField(msgReflection, j, key.c_str(), k, tempFieldData);
      k++;
   }

   for (const auto &field:tempFieldData) {
      string fieldName = field.second.first;
      string  fieldValue = field.second.second;
      if (fieldName == "FlowCompleted") {
         EXPECT_EQ(expectedstring[fieldName],fieldValue) << "name/value: [" << fieldName << "]/[" <<fieldValue << "]";    
      } else {
         EXPECT_EQ(std::to_string(expectedint[fieldName]),fieldValue) << "name/value: [" << fieldName << "]/[" <<fieldValue << "]";    
      }
   }
}

// Some of the fields will be camelcase and some of the fields will be 
// lowercase-underscore separated
//
// See DpiMsgLR.h/cpp to see which ones are hard coded. The hard coded
// are CamelCase and the others will be with underscore_separation
TEST_F(DpiMsgLRTests, GetAllFieldsAsStrings) {
#ifdef LR_DEBUG
   map<unsigned int, pair<string, string> > results;
   std::map<string, string> expecteds;
   MockDpiMsgLR dm;

   string SrcIPs = "13.12.11.10";
   dm.set_srcip(0x0a0b0c0d);
   expecteds["IpSrc"] = SrcIPs;

   string DestIPs = "4.3.2.1";
   dm.set_destip(0x01020304);
   expecteds["IpDst"] = DestIPs;

   string MacSources = "0f:0e:0d:0c:0b:0a";
   dm.set_srcmac(0x00000a0b0c0d0e0f);
   expecteds["EthSrc"] = MacSources;

   string MacDests = "06:05:04:03:02:01";
   dm.set_destmac(0x0000010203040506);
   expecteds["EthDst"] = MacDests;

   string uuid = "01234567-89ab-cdef-0123456789abcdef";
   dm.set_session(uuid);
   expecteds["UUID"] = uuid;

   string path = "foo.bar";
   dm.set_packetpath(path);
   expecteds["Path"] = path;

   int sport = 1234;
   dm.set_srcport(sport);
   expecteds["SourcePort"] = std::to_string(sport);

   int dport = 5678;
   dm.set_destport(dport);
   expecteds["DestPort"] = std::to_string(dport);

   int chunk = 1234;
   string chunks = std::to_string(chunk);
   dm.set_file_chunk_data_offsetq_proto_bittorrent(chunk);
   expecteds["file_chunk_data_offset"] = chunks;

   int filesize = 7890;
   string filesizes = std::to_string(filesize);
   dm.set_filesizeq_proto_edonkey(filesize);
   expecteds["filesize"] = filesizes;

   string filename = "this is a file";
   dm.add_attach_filenameq_proto_facebook_mail(filename);
   expecteds["attach_filename"] = filename;

   results = dm.GetAllFieldsAsStrings();
   ASSERT_EQ(11, results.size());

   for (auto i = results.begin(); i != results.end(); ++i) {
      string fieldName = i->second.first;
      string fieldValue = i->second.second;
      EXPECT_EQ(expecteds[fieldName], fieldValue) << "name/value: [" << fieldName << "]/[" << fieldValue << "]";    
   }
#endif
}

TEST_F(DpiMsgLRTests, MissingKeyFields) {
#ifdef LR_DEBUG
   map<unsigned int, pair<string, string> > results;
   MockDpiMsgLR dm;

   dm.set_session("01234567-89ab-cdef-0123456789abcdef");

   results = dm.GetAllFieldsAsStrings();

   ASSERT_EQ(1, results.size());
#endif
}

TEST_F(DpiMsgLRTests, serializationSuccess) {
   // Set up some local data to put in the DpiMsgLR
   string uuid("01234567-89ab-cdef-0123456789abcdef");
   vector<unsigned char> ethDst;
   ethDst.push_back(0xF0);
   ethDst.push_back(0xF0);
   ethDst.push_back(0x55);
   ethDst.push_back(0xDC);
   ethDst.push_back(0xA8);
   ethDst.push_back(0x7F);

   vector<unsigned char> ethSrc;
   ethSrc.push_back(0x00);
   ethSrc.push_back(0x50);
   ethSrc.push_back(0x50);
   ethSrc.push_back(0xBE);
   ethSrc.push_back(0x00);
   ethSrc.push_back(0x1C);
   uint32_t ipSrc = 3232235876; // 192.168.1.100
   uint32_t ipDst = 167837813; // 10.1.0.75
   std::string path("eth.ip.tcp.http");

   // Set the DPI Message member data
   DpiMsgLR dm;
   DpiMsgLR rdm;
   std::string dmVec;
   dm.set_session(uuid);
   dm.SetEthSrc(ethSrc);
   dm.SetEthDst(ethDst);
   dm.set_srcip(ipSrc);
   dm.set_destip(ipDst);
   dm.set_packetpath(path);
   vector<unsigned char> rEthDst;
   vector<unsigned char> rEthSrc;

   // Test the getBuffer and putBuffer methods 10000 times to see some
   // rough performance numbers

   for (int testCnt = 1; testCnt <= 10000; testCnt++) {

      dmVec.clear();
      dm.GetBuffer(dmVec);
      rdm.PutBuffer(dmVec);

      // Expect the uuid in the new object to match what was put in the original
      // object.
      string rUuid(rdm.session());
      ASSERT_EQ(uuid, rUuid);

      // Expect the IP Source Address in the new object to match what was put
      // in the original object.
      uint32_t rIpSrc = rdm.srcip();
      ASSERT_EQ(ipSrc, rIpSrc);

      // Expect the IP Destination Address in the new object to match what was
      // put in the original object.
      uint32_t rIpDst = rdm.destip();
      ASSERT_EQ(ipDst, rIpDst);

      // Expect the Packet Path in the new object to match what was put in the
      // original object.
      std::string rPath = rdm.packetpath();
      ASSERT_EQ(path, rPath);

      // Get the Ethernet Addresses from the new object
      rEthDst.clear();
      rEthSrc.clear();
      rdm.GetEthSrc(rEthSrc);
      rdm.GetEthDst(rEthDst);
      // Expect the values in the two arrays to match
      ASSERT_EQ(ethSrc.size(), rEthSrc.size());
      ASSERT_EQ(ethDst.size(), rEthDst.size());
      ASSERT_EQ(ETH_ADDR_SIZE, rEthDst.size());
      ASSERT_EQ(ETH_ADDR_SIZE, rEthSrc.size());
      for (int tInd = 0; tInd < ETH_ADDR_SIZE; tInd++) {
         ASSERT_EQ(rEthSrc[tInd], ethSrc[tInd]);
         ASSERT_EQ(rEthDst[tInd], ethDst[tInd]);
      }
   }
}

TEST_F(DpiMsgLRTests, SetStringFieldsByName) {
   DpiMsgLR dm;

   ASSERT_EQ(0, dm.application_endq_proto_base_size());
   if (dm.CanAddRepeatedString("application_endq_proto_base", "abc123")) {
      dm.add_application_endq_proto_base("abc123");
   }
   EXPECT_FALSE(dm.CanAddRepeatedString("application_endq_proto_base", "abc123"));
   ASSERT_EQ("abc123", dm.application_endq_proto_base(0));
   if (dm.CanAddRepeatedString("application_endq_proto_base", "def456")) {
      dm.add_application_endq_proto_base("def456");
   }
   ASSERT_EQ("def456", dm.application_endq_proto_base(1));
}

TEST_F(DpiMsgLRTests, SetStringFieldsByNameSerializes) {
   DpiMsgLR dm;
   std::string serializeData;
   dm.set_session("abc123");

   EXPECT_TRUE(dm.CanAddRepeatedString("application_endq_proto_base", "ghi789"));
   EXPECT_FALSE(dm.CanAddRepeatedString("application_endq_proto_base", "ghi789"));
   dm.add_application_endq_proto_base("ghi789");
   dm.add_application_endq_proto_base("abc123");
   dm.GetBuffer(serializeData);
   DpiMsgLR rdm;

   rdm.PutBuffer(serializeData);
   ASSERT_EQ("abc123", rdm.session());
   ASSERT_EQ("abc123", rdm.application_endq_proto_base(1));
}

TEST_F(DpiMsgLRTests, GetLastStringFieldsByName) {
   DpiMsgLR dm;
   std::string lastStr;
   std::string serializeData;
   ASSERT_FALSE(dm.GetLastStringByName("application_endq_proto_base", lastStr));
   ASSERT_EQ("", lastStr);
   dm.add_application_endq_proto_base("abc123");

   ASSERT_TRUE(dm.GetLastStringByName("application_endq_proto_base", lastStr));
   ASSERT_EQ("abc123", lastStr);
   EXPECT_TRUE(dm.CanAddRepeatedString("uuid", "def456"));
   dm.add_application_endq_proto_base("def456");
   lastStr.clear();
   ASSERT_TRUE(dm.GetLastStringByName("application_endq_proto_base", lastStr));
   ASSERT_EQ("def456", lastStr);
}

TEST_F(DpiMsgLRTests, GetLastApplicationFromProtoMultipleProtocols) {
   DpiMsgLR dm;
   dm.add_application_endq_proto_base("tcp");
   dm.add_application_endq_proto_base("http");
   dm.add_application_endq_proto_base("google");
   std::string lastStr = dm.GetLatestApplicationFromProto();
   ASSERT_EQ("google", lastStr);
}

#ifdef LR_DEBUG
TEST_F(DpiMsgLRTests, VerifyGetApplicationNameFromList) {
   MockDpiMsgLR dm;
   dm.add_application_endq_proto_base("google");
   dm.add_applicationq_proto_base("yahoo");
   auto listG = dm.application_endq_proto_base();
   auto listY = dm.applicationq_proto_base();
                
   auto appG = dm.GetApplicationNameFromList(listG);
   auto appY = dm.GetApplicationNameFromList(listY);
   ASSERT_EQ(appG, "google");   
   ASSERT_EQ(appY, "yahoo");   
}

TEST_F(DpiMsgLRTests, GetUnknownApplicationNameFromList) {
   MockDpiMsgLR dm;
   dm.add_application_endq_proto_base("tcp");
   dm.add_application_endq_proto_base("unknown");
   auto list = dm.application_endq_proto_base();
   
   auto app = dm.GetApplicationNameFromList(list);
   ASSERT_EQ(app, "unknown");   
}

TEST_F(DpiMsgLRTests, GetKnownApplicationNameFromList) {
   MockDpiMsgLR dm;
   dm.add_application_endq_proto_base("tcp");
   dm.add_application_endq_proto_base("gtalk");
   dm.add_application_endq_proto_base("unknown");
   auto list = dm.application_endq_proto_base();
   
   auto app = dm.GetApplicationNameFromList(list);
   ASSERT_EQ(app, "gtalk");   
   ASSERT_EQ(app, dm.GetLatestApplicationFromProto());
}

TEST_F(DpiMsgLRTests, GetXApplicationNameFromList) {
   std::vector<std::string> unknownInclusiveList {{"base"}, {"ip"}, {"udp"}, {"tcp"}};
   for(auto& protocol: unknownInclusiveList) {
      MockDpiMsgLR dm;
      dm.add_applicationq_proto_base(protocol);
      dm.add_applicationq_proto_base("jabber");
      dm.add_applicationq_proto_base("unknown");
      EXPECT_EQ("jabber", dm.GetLatestApplication());
   }
   
   for(auto& protocol: unknownInclusiveList) {
      MockDpiMsgLR dm;
      dm.add_applicationq_proto_base("jabber");
      dm.add_applicationq_proto_base(protocol);
      dm.add_applicationq_proto_base("unknown");
      EXPECT_EQ("unknown", dm.GetLatestApplication());
   }
}
#endif

TEST_F(DpiMsgLRTests, GetLastApplicationFromProtoSingleProtocol) {
   DpiMsgLR dm;
   dm.add_application_endq_proto_base("tcp");
   std::string lastStr;
   lastStr = dm.GetLatestApplicationFromProto();
   ASSERT_EQ("tcp", lastStr);
}

TEST_F(DpiMsgLRTests, GetLastApplicationFromProtoEmpty) {
   DpiMsgLR dm;
   std::string lastStr;
   lastStr = dm.GetLatestApplicationFromProto();
   ASSERT_EQ(EMPTY, lastStr);
}

TEST_F(DpiMsgLRTests, CountUpRepeats) {
   DpiMsgLR dm;

   EXPECT_EQ(0, dm.CountUpRepeats());
   dm.add_accept_encodingq_proto_http("test1");
   EXPECT_EQ(1, dm.CountUpRepeats());
   dm.add_accept_encodingq_proto_http("test2");
   EXPECT_EQ(2, dm.CountUpRepeats());
   dm.add_actionq_proto_adobe_update("test3");
   EXPECT_EQ(3, dm.CountUpRepeats());
}

TEST_F(DpiMsgLRTests, EmptyLongFields) {
   DpiMsgLR dm;
   dm.EmptyLongFields(0, 0);
   dm.add_application_endq_proto_base("dontDeleteMe");
   dm.add_applicationq_proto_base("meNeither");
   dm.add_familyq_proto_base("not me");
   dm.add_family_endq_proto_base("or me");
   dm.EmptyLongFields(0, 0);
   EXPECT_EQ(1, dm.application_endq_proto_base_size());
   EXPECT_EQ(1, dm.applicationq_proto_base_size());
   EXPECT_EQ(1, dm.familyq_proto_base_size());
   EXPECT_EQ(1, dm.family_endq_proto_base_size());
   dm.add_access_pointq_proto_gtp("test1");
   dm.add_access_pointq_proto_gtp("test2");
   dm.add_access_pointq_proto_gtp("test3");
   dm.EmptyLongFields(4, 0);
   EXPECT_EQ(3, dm.access_pointq_proto_gtp_size());
   dm.EmptyLongFields(3, 0);
   EXPECT_EQ(0, dm.access_pointq_proto_gtp_size());
   dm.add_devq_proto_base("test1");
   dm.EmptyLongFields(0, DpiMsgLRproto::kDevQPROTOBASEFieldNumber + 1);
   EXPECT_EQ(1, dm.devq_proto_base_size());
   dm.EmptyLongFields(0, DpiMsgLRproto::kDevQPROTOBASEFieldNumber);
   EXPECT_EQ(0, dm.devq_proto_base_size());
}

