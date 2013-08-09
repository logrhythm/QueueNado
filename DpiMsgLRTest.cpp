#include <stdlib.h>
#include "DpiMsgLR.h"
#include "DpiMsgLRTest.h"
#include "g2log.hpp"

using namespace networkMonitor;
using namespace std;

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
   dm.set_sessionid(uuid);

   // Get the Unique ID from the object
   string rUuid(dm.sessionid());

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
   dm->set_sessionid("01234567-89ab-cdef-0123456789abcdef");
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
   dm.set_ipsource(ipSrc);

   // Get the IP Source Address
   uint32_t rIpSrc = dm.ipsource();

   // Expect the two address to match
   EXPECT_EQ(ipSrc, rIpSrc);
}

TEST_F(DpiMsgLRTests, setIpDstSuccess) {
   // Set the IP Destination Address
   uint32_t ipDst = 167837813; // 10.1.0.75
   DpiMsgLR dm;
   dm.set_ipdest(ipDst);

   // Get the IP Destination Address
   uint32_t rIpDst = dm.ipdest();

   // Expect the two address to match
   EXPECT_EQ(ipDst, rIpDst);
}

TEST_F(DpiMsgLRTests, setPktPathSuccess) {
   // Set the Packet Path
   std::string path("eth.ip.tcp.http");
   DpiMsgLR dm;
   dm.set_pktpath(path);

   // Get the Packet Path
   std::string rPath = dm.pktpath();

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
   EXPECT_EQ(0x90af, tDpiMessage.portsource());
   EXPECT_EQ(0x5b41, tDpiMessage.portdest());
#endif
}

TEST_F(DpiMsgLRTests, GetSpecialIntegersAsStrings) {
#ifdef LR_DEBUG
   MockDpiMsgLR dm;

   dm.set_ipsource(0x0a0b0c0d);
   EXPECT_EQ("13.12.11.10", dm.GetIpSrcString());
   dm.set_ipdest(0x01020304);
   EXPECT_EQ("4.3.2.1", dm.GetIpDstString());
   dm.set_ethsource(0x00000a0b0c0d0e0f);
   EXPECT_EQ("0f:0e:0d:0c:0b:0a", dm.GetEthSrcString());
   dm.set_ethdest(0x0000010203040506);
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
   EXPECT_EQ("", tDpiMessage.GetUuidPair().second);
   tDpiMessage.set_sessionid("01234567-89ab-cdef-0123456789abcdef");
   EXPECT_EQ("UUID", tDpiMessage.GetUuidPair().first);
   EXPECT_EQ("01234567-89ab-cdef-0123456789abcdef",
           tDpiMessage.GetUuidPair().second);
}

TEST_F(DpiMsgLRTests, GetEthSrcPair) {
   EXPECT_EQ("00:00:00:00:00:00", tDpiMessage.GetEthSrcPair().second);
   tDpiMessage.set_ethsource(0x00000a0b0c0d0e0f);
   EXPECT_EQ("EthSrc", tDpiMessage.GetEthSrcPair().first);
   EXPECT_EQ("0f:0e:0d:0c:0b:0a", tDpiMessage.GetEthSrcPair().second);
}

TEST_F(DpiMsgLRTests, GetEthDstPair) {
   EXPECT_EQ("00:00:00:00:00:00", tDpiMessage.GetEthDstPair().second);
   tDpiMessage.set_ethdest(0x0000010203040506);
   EXPECT_EQ("EthDst", tDpiMessage.GetEthDstPair().first);
   EXPECT_EQ("06:05:04:03:02:01", tDpiMessage.GetEthDstPair().second);
}

TEST_F(DpiMsgLRTests, GetIpSrcPair) {
   EXPECT_EQ("00.00.00.00", tDpiMessage.GetIpSrcPair().second);
   tDpiMessage.set_ipsource(0x0a0b0c0d);

   EXPECT_EQ("IpSrc", tDpiMessage.GetIpSrcPair().first);
   EXPECT_EQ("13.12.11.10", tDpiMessage.GetIpSrcPair().second);

}

TEST_F(DpiMsgLRTests, GetIpDstPair) {
   EXPECT_EQ("00.00.00.00", tDpiMessage.GetIpDstPair().second);
   tDpiMessage.set_ipdest(0x01020304);

   EXPECT_EQ("IpDst", tDpiMessage.GetIpDstPair().first);
   EXPECT_EQ("4.3.2.1", tDpiMessage.GetIpDstPair().second);

}

TEST_F(DpiMsgLRTests, GetPktPathPair) {
   EXPECT_EQ(EMPTY, tDpiMessage.GetPktPathPair().second);
   string path = "foo.bar";
   tDpiMessage.set_pktpath(path);
   EXPECT_EQ("Path", tDpiMessage.GetPktPathPair().first);
   EXPECT_EQ("foo.bar", tDpiMessage.GetPktPathPair().second);

}

TEST_F(DpiMsgLRTests, GetSourcePortPair) {
   EXPECT_EQ("0", tDpiMessage.GetSourcePortPair().second);
   tDpiMessage.set_portsource(1234);
   EXPECT_EQ("SourcePort", tDpiMessage.GetSourcePortPair().first);
   EXPECT_EQ("1234", tDpiMessage.GetSourcePortPair().second);

}

TEST_F(DpiMsgLRTests, GetDestPortPair) {
   EXPECT_EQ("0", tDpiMessage.GetDestPortPair().second);
   tDpiMessage.set_portdest(5678);
   EXPECT_EQ("DestPort", tDpiMessage.GetDestPortPair().first);
   EXPECT_EQ("5678", tDpiMessage.GetDestPortPair().second);
}

TEST_F(DpiMsgLRTests, GetStaticFieldPairs) {
   IndexedFieldPairs results;
   tDpiMessage.GetStaticFieldPairs(results);
   ASSERT_EQ(1, results.size());
   EXPECT_EQ("UUID", results[1].first);
   tDpiMessage.set_ethdest(123);
   tDpiMessage.set_ethsource(123);
   tDpiMessage.set_ipdest(123);
   tDpiMessage.set_ipsource(123);
   tDpiMessage.set_pktpath("foo");
   tDpiMessage.set_portsource(123);
   tDpiMessage.set_portdest(123);
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

TEST_F(DpiMsgLRTests, GetDynamicFieldPairs) {
   std::map<unsigned int, std::pair<std::string, std::string> > results;
   std::map<string, string> expecteds;
   tDpiMessage.GetDynamicFieldPairs(results);
   ASSERT_EQ(0, results.size());

   int chunk = 1234;
   string chunks = std::to_string(chunk);
   tDpiMessage.set_file_chunk_data_offsetq_proto_bittorrent(chunk);
   expecteds["fileChunkDataOffset"] = chunks;

   int filesize = 7890;
   string filesizes = std::to_string(filesize);
   tDpiMessage.set_filesizeq_proto_edonkey(filesize);
   expecteds["filesize"] = filesizes;

   string filename = "this is a file";
   tDpiMessage.add_attach_filenameq_proto_facebook_mail(filename);
   tDpiMessage.GetDynamicFieldPairs(results);
   expecteds["attachFilename"] = filename;

   ASSERT_EQ(3, results.size());
   for (auto i = results.begin(); i != results.end(); ++i) {
      string fieldName = i->second.first;
      string fieldValue = i->second.second;
      EXPECT_EQ(expecteds[fieldName], fieldValue);
   }
}

TEST_F(DpiMsgLRTests, GetAllFieldsAsStrings) {
#ifdef LR_DEBUG
   map<unsigned int, pair<string, string> > results;
   std::map<string, string> expecteds;
   MockDpiMsgLR dm;

   string ipsources = "13.12.11.10";
   dm.set_ipsource(0x0a0b0c0d);
   expecteds["IpSrc"] = ipsources;

   string ipdests = "4.3.2.1";
   dm.set_ipdest(0x01020304);
   expecteds["IpDst"] = ipdests;

   string ethsources = "0f:0e:0d:0c:0b:0a";
   dm.set_ethsource(0x00000a0b0c0d0e0f);
   expecteds["EthSrc"] = ethsources;

   string ethdests = "06:05:04:03:02:01";
   dm.set_ethdest(0x0000010203040506);
   expecteds["EthDst"] = ethdests;

   string uuid = "01234567-89ab-cdef-0123456789abcdef";
   dm.set_sessionid(uuid);
   expecteds["UUID"] = uuid;

   string path = "foo.bar";
   dm.set_pktpath(path);
   expecteds["Path"] = path;

   int sport = 1234;
   dm.set_portsource(sport);
   expecteds["SourcePort"] = std::to_string(sport);

   int dport = 5678;
   dm.set_portdest(dport);
   expecteds["DestPort"] = std::to_string(dport);

   int chunk = 1234;
   string chunks = std::to_string(chunk);
   dm.set_file_chunk_data_offsetq_proto_bittorrent(chunk);
   expecteds["fileChunkDataOffset"] = chunks;

   int filesize = 7890;
   string filesizes = std::to_string(filesize);
   dm.set_filesizeq_proto_edonkey(filesize);
   expecteds["filesize"] = filesizes;

   string filename = "this is a file";
   dm.add_attach_filenameq_proto_facebook_mail(filename);
   expecteds["attachFilename"] = filename;

   results = dm.GetAllFieldsAsStrings();
   ASSERT_EQ(11, results.size());

   for (auto i = results.begin(); i != results.end(); ++i) {
      string fieldName = i->second.first;
      string fieldValue = i->second.second;
      EXPECT_EQ(expecteds[fieldName], fieldValue);
   }
#endif
}

TEST_F(DpiMsgLRTests, MissingKeyFields) {
#ifdef LR_DEBUG
   map<unsigned int, pair<string, string> > results;
   MockDpiMsgLR dm;

   dm.set_sessionid("01234567-89ab-cdef-0123456789abcdef");

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
   dm.set_sessionid(uuid);
   dm.SetEthSrc(ethSrc);
   dm.SetEthDst(ethDst);
   dm.set_ipsource(ipSrc);
   dm.set_ipdest(ipDst);
   dm.set_pktpath(path);
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
      string rUuid(rdm.sessionid());
      ASSERT_EQ(uuid, rUuid);

      // Expect the IP Source Address in the new object to match what was put
      // in the original object.
      uint32_t rIpSrc = rdm.ipsource();
      ASSERT_EQ(ipSrc, rIpSrc);

      // Expect the IP Destination Address in the new object to match what was
      // put in the original object.
      uint32_t rIpDst = rdm.ipdest();
      ASSERT_EQ(ipDst, rIpDst);

      // Expect the Packet Path in the new object to match what was put in the
      // original object.
      std::string rPath = rdm.pktpath();
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
   dm.set_sessionid("abc123");

   EXPECT_TRUE(dm.CanAddRepeatedString("application_endq_proto_base", "ghi789"));
   EXPECT_FALSE(dm.CanAddRepeatedString("application_endq_proto_base", "ghi789"));
   dm.add_application_endq_proto_base("ghi789");
   dm.add_application_endq_proto_base("abc123");
   dm.GetBuffer(serializeData);
   DpiMsgLR rdm;

   rdm.PutBuffer(serializeData);
   ASSERT_EQ("abc123", rdm.sessionid());
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
   std::string lastStr;
   lastStr = dm.GetLatestApplicationFromProto();
   ASSERT_EQ("google", lastStr);

}

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

