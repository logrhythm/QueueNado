/* 
 * File:   DpiSyslogTest.cpp
 * Author: John Gress
 * 
 * Created on October 19, 2012
 */
#include <iostream>
#include "SendDpiMsgLRZMQ.h"
#include "RuleEngineTest.h"
#include "MockRuleEngine.h"
#include "MockConfSlave.h"
#include "DpiMsgLR.h"

using namespace networkMonitor;

TEST_F(RuleEngineTest, getSiemSyslogMessagesSplitDataTestWithDebug) {
#ifdef LR_DEBUG
    MockRuleEngine dm(conf, syslogName, syslogOption,
            syslogFacility, syslogPriority, true, 0);
    map<unsigned int, pair <string, string> > dataPairs;
    vector<string> messages;

    dm.mSiemMode = true;
    dm.mSiemDebugMode = false;
    tDpiMessage.set_uuid("550e8400-e29b-41d4-a716-446655440000");
    tDpiMessage.set_ethdst(123);
    tDpiMessage.set_ethsrc(124);
    tDpiMessage.set_ipdst(125);
    tDpiMessage.set_ipsrc(126);
    tDpiMessage.set_sourceport(127);
    tDpiMessage.set_destport(128);
    tDpiMessage.set_protoid(129);
    tDpiMessage.set_application_endq_proto_base("test");
    tDpiMessage.set_application_id_endq_proto_base(1234);
    tDpiMessage.set_sessionlen(567);
    tDpiMessage.set_packetcount(88);
    tDpiMessage.set_loginq_proto_aim("aLogin");
    tDpiMessage.set_domainq_proto_smb("aDomain");
    tDpiMessage.set_uri_fullq_proto_http("this/url.htm");
    tDpiMessage.set_uriq_proto_http("not/this/one");
    tDpiMessage.set_serverq_proto_http("thisname");
    tDpiMessage.set_referer_serverq_proto_http("notThisOne");
    tDpiMessage.set_methodq_proto_ftp("TEST|COMMAND|LONGLONGLONGLONG");
    tDpiMessage.set_senderq_proto_smtp("test1");
    tDpiMessage.set_receiverq_proto_smtp("test2");
    tDpiMessage.set_subjectq_proto_smtp("test3");
    tDpiMessage.set_versionq_proto_http("4.0");
    tDpiMessage.set_starttime(123);
    tDpiMessage.set_endtime(456);
    dm.SetMaxSize(512 + 8 + 36 + 4);
    messages = dm.GetSiemSyslogMessage(tDpiMessage);
    //   for (int i = 0; i < messages.size(); i++) {
    //      std::cout << messages[i] << std::endl;
    //   }
    ASSERT_EQ(1, messages.size());
    std::string expectedEvent = "EVT:001 550e8400-e29b-41d4-a716-446655440000:";
    std::string expectedHeader = " 126.0.0.0,125.0.0.0,127,128,7c:00:00:00:00:00,7b:00:00:00:00:00,129,test,567,88,123,456";
    std::string expectedHeaderNoCounts = " 126.0.0.0,125.0.0.0,127,128,7c:00:00:00:00:00,7b:00:00:00:00:00,129,test,,,123,456";
    std::string expected;
    expected = BuildExpectedHeaderForSiem(expectedEvent, expectedHeader, 0);
    expected += ",login=aLogin,domain=aDomain,dname=thisname,url=this/url.htm,command=TEST|COMMAND|LONGLONGLONGLONG,sender=test1,recipient=test2,subject=test3,version=4.0";
    EXPECT_EQ(expected, messages[0]);
    expected = "EVT:002 " + expectedHeader;
    expected += "";
    //EXPECT_EQ(expected,messages[1]);


    messages.clear();
    dm.SetMaxSize(108 + 8 + 36 + 4 - 5);
    messages = dm.GetSiemSyslogMessage(tDpiMessage);
    //   for (int i = 0; i < messages.size(); i++) {
    //      std::cout << messages[i] << std::endl;
    //   }
    ASSERT_EQ(11, messages.size());
    unsigned int index = 0;
    expected = BuildExpectedHeaderForSiem(expectedEvent, expectedHeader, index);
    expected += ",login=aLogin";
    EXPECT_EQ(expected, messages[index++]);
    expected = BuildExpectedHeaderForSiem(expectedEvent, expectedHeaderNoCounts, index);
    expected += ",domain=aDomain";
    EXPECT_EQ(expected, messages[index++]);
    expected = BuildExpectedHeaderForSiem(expectedEvent, expectedHeaderNoCounts, index);
    expected += ",dname=thisname";
    EXPECT_EQ(expected, messages[index++]);
    expected = BuildExpectedHeaderForSiem(expectedEvent, expectedHeaderNoCounts, index);
    expected += ",url=this/url.htm";
    EXPECT_EQ(expected, messages[index++]);
    expected = BuildExpectedHeaderForSiem(expectedEvent, expectedHeaderNoCounts, index);
    expected += ",command=TEST|COMMAND";
    EXPECT_EQ(expected, messages[index++]);
    expected = BuildExpectedHeaderForSiem(expectedEvent, expectedHeaderNoCounts, index);
    expected += ",command=LONGLONGLONG";
    EXPECT_EQ(expected, messages[index++]);
    expected = BuildExpectedHeaderForSiem(expectedEvent, expectedHeaderNoCounts, index);
    expected += ",command=LONG";
    EXPECT_EQ(expected, messages[index++]);
    expected = BuildExpectedHeaderForSiem(expectedEvent, expectedHeaderNoCounts, index);
    expected += ",sender=test1";
    EXPECT_EQ(expected, messages[index++]);
    expected = BuildExpectedHeaderForSiem(expectedEvent, expectedHeaderNoCounts, index);
    expected += ",recipient=test2";
    EXPECT_EQ(expected, messages[index++]);
    expected = BuildExpectedHeaderForSiem(expectedEvent, expectedHeaderNoCounts, index);
    expected += ",subject=test3";
    EXPECT_EQ(expected, messages[index++]);
    expected = BuildExpectedHeaderForSiem(expectedEvent, expectedHeaderNoCounts, index);
    expected += ",version=4.0";
    EXPECT_EQ(expected, messages[index++]);
#endif
}

TEST_F(RuleEngineTest, testConstructor) {
    conf.SetPath("resources/test.yaml");
    MockRuleEngine dpiSyslog(conf, syslogName, syslogOption,
            syslogFacility, syslogPriority, true, 0);
    dpiSyslog.StartupMessage();
    EXPECT_EQ(syslogName, sysLogOpenIdent);
    EXPECT_EQ(syslogOption, sysLogOpenOption);
    EXPECT_EQ(syslogFacility, sysLogOpenFacility);
    EXPECT_EQ(syslogPriority, sysLogOpenPriority);
}

TEST_F(RuleEngineTest, testNewDelete) {
    conf.SetPath("resources/test.yaml");
    MockRuleEngine * pDpiSyslog = new MockRuleEngine(conf, syslogName, syslogOption,
            syslogFacility, syslogPriority, true, 0);
    pDpiSyslog->StartupMessage();
    EXPECT_EQ(syslogName, sysLogOpenIdent);
    EXPECT_EQ(syslogOption, sysLogOpenOption);
    EXPECT_EQ(syslogFacility, sysLogOpenFacility);
    EXPECT_EQ(syslogPriority, sysLogOpenPriority);
    delete pDpiSyslog;
}

TEST_F(RuleEngineTest, testStartStop) {
    conf.SetPath("resources/test.yaml");
    MockRuleEngine dpiSyslog(conf, syslogName, syslogOption,
            syslogFacility, syslogPriority, true, 0);

    EXPECT_FALSE(dpiSyslog.isRunning());
    dpiSyslog.start();
    EXPECT_TRUE(dpiSyslog.isRunning());
    dpiSyslog.join();
}

TEST_F(RuleEngineTest, testMsgReceive) {
#if defined(LR_DEBUG)
    if (geteuid() != 0) {
        //   conf.SetPath("resources/test.yaml");
        //
        protoMsg::BaseConf confMsg;
        protoMsg::SyslogConf sysMsg;

        boost::this_thread::sleep(boost::posix_time::seconds(1));
        MockRuleEngine dpiSyslog(conf, syslogName, syslogOption,
                syslogFacility, syslogPriority, true, 0);
        dpiSyslog.mSiemMode = false;
        dpiSyslog.start();
        EXPECT_TRUE(dpiSyslog.isRunning());

        // Send a message to the thread and check the syslog output contains the
        // data send.
        std::string queueName = dpiSyslog.GetDpiRcvrQueue();
        queueName += "0";
        SendDpiMsgLRZMQ sendQueue(queueName);
        sendQueue.Initialize();

        DpiMsgLR msg;

        string testUuid("8a3461dc-4aaa-41d5-bf3f-f55037d5ed25");
        msg.set_uuid(testUuid.c_str());

        string testEthSrc("00:22:19:08:2c:00");
        vector<unsigned char> ethSrc;
        ethSrc.push_back(0x00);
        ethSrc.push_back(0x22);
        ethSrc.push_back(0x19);
        ethSrc.push_back(0x08);
        ethSrc.push_back(0x2c);
        ethSrc.push_back(0x00);
        msg.SetEthSrc(ethSrc);

        string testEthDst("f0:f7:55:dc:a8:00");

        vector<unsigned char> ethDst;
        ethDst.push_back(0xf0);
        ethDst.push_back(0xf7);
        ethDst.push_back(0x55);
        ethDst.push_back(0xdc);
        ethDst.push_back(0xa8);
        ethDst.push_back(0x00);
        msg.SetEthDst(ethDst);

        string testIpSrc = "10.1.10.50";
        uint32_t ipSrc = 0x320A010A; // 10.1.10.50, note: little endian
        msg.set_ipsrc(ipSrc);

        string testIpDst = "10.128.64.251";
        uint32_t ipDst = 0xFB40800A; // 10.128.64.251, note: little endian
        msg.set_ipdst(ipDst);

        string path("base.eth.ip.udp.ntp");
        msg.set_pktpath(path.c_str());

        string testIpSourcePort = "=12345"; // bogus, but easier to test
        msg.set_sourceport(12345);

        string testIpDestPort = "=54321"; // bogus, but easier to test
        msg.set_destport(54321);

        string dataToSend;
        msg.GetBuffer(dataToSend);
        sendQueue.SendData(dataToSend);

        // Allow DPI Syslog Thread time to process the message
        timespec timeToSleep;
        timeToSleep.tv_sec = 0;
        timeToSleep.tv_nsec = 200000000;
        nanosleep(&timeToSleep, NULL);

        //std::cout << "SyslogOutput: " << sysLogOutput << std::endl;
        // Did the data show up in the syslog output
        //      for ( int i = 0 ; i < sysLogOutput.size() ; i++) {
        //          std::cout << sysLogOutput[i] << std::endl;
        //      }
        ASSERT_EQ(1, sysLogOutput.size());
        EXPECT_NE(std::string::npos, sysLogOutput[0].find("EVT:003 "));
        EXPECT_NE(std::string::npos, sysLogOutput[0].find(testUuid));

        EXPECT_NE(std::string::npos, sysLogOutput[0].find(testEthSrc));

        EXPECT_NE(std::string::npos, sysLogOutput[0].find(testEthDst));

        EXPECT_NE(std::string::npos, sysLogOutput[0].find(testIpSrc));

        EXPECT_NE(std::string::npos, sysLogOutput[0].find(testIpDst));

        EXPECT_NE(std::string::npos, sysLogOutput[0].find(path));

        EXPECT_NE(std::string::npos, sysLogOutput[0].find(testIpSourcePort));

        EXPECT_NE(std::string::npos, sysLogOutput[0].find(testIpDestPort));


        dpiSyslog.join();
    }
#endif
}

TEST_F(RuleEngineTest, testMsgReceiveSiemMode) {
#if defined(LR_DEBUG)
    if (geteuid() != 0) {
        MockConfSlave myConfSlave;

        myConfSlave.SetPath("resources/test.yaml");
        myConfSlave.mConf.mSiemLogging = true;
        //myConfSlave.Start();
        protoMsg::BaseConf confMsg;
        protoMsg::SyslogConf sysMsg;
        sysMsg.set_siemlogging("true");
        Conf myConf = conf.GetConf();
        myConf.updateFields(sysMsg);
        myConf.sendConfigUpdate();
        boost::this_thread::sleep(boost::posix_time::seconds(1));
        MockRuleEngine dpiSyslog(myConfSlave, syslogName, syslogOption,
                syslogFacility, syslogPriority, true, 0);
        dpiSyslog.mSiemMode = true;
        dpiSyslog.mSiemDebugMode = false;
        dpiSyslog.start();
        EXPECT_TRUE(dpiSyslog.isRunning());

        // Send a message to the thread and check the syslog output contains the
        // data send.
        std::string queueName = dpiSyslog.GetDpiRcvrQueue();
        queueName += "0";
        SendDpiMsgLRZMQ sendQueue(queueName);
        sendQueue.Initialize();

        DpiMsgLR msg;

        string testUuid("8a3461dc-4aaa-41d5-bf3f-f55037d5ed25");
        msg.set_uuid(testUuid.c_str());

        string testEthSrc("00:22:19:08:2c:00");
        vector<unsigned char> ethSrc;
        ethSrc.push_back(0x00);
        ethSrc.push_back(0x22);
        ethSrc.push_back(0x19);
        ethSrc.push_back(0x08);
        ethSrc.push_back(0x2c);
        ethSrc.push_back(0x00);
        msg.SetEthSrc(ethSrc);

        string testEthDst("f0:f7:55:dc:a8:00");

        vector<unsigned char> ethDst;
        ethDst.push_back(0xf0);
        ethDst.push_back(0xf7);
        ethDst.push_back(0x55);
        ethDst.push_back(0xdc);
        ethDst.push_back(0xa8);
        ethDst.push_back(0x00);
        msg.SetEthDst(ethDst);

        string testIpSrc = "10.1.10.50";
        uint32_t ipSrc = 0x320A010A; // 10.1.10.50, note: little endian
        msg.set_ipsrc(ipSrc);

        string testIpDst = "10.128.64.251";
        uint32_t ipDst = 0xFB40800A; // 10.128.64.251, note: little endian
        msg.set_ipdst(ipDst);

        string path("base.eth.ip.udp.ntp");
        msg.set_pktpath(path.c_str());

        string testIpSourcePort = "=12345"; // bogus, but easier to test
        msg.set_sourceport(12345);

        string testIpDestPort = "=54321"; // bogus, but easier to test
        msg.set_destport(54321);
        msg.set_protoid(12);
        msg.set_application_id_endq_proto_base(13);
        msg.set_application_endq_proto_base("wrong|dummy");
        msg.set_sessionlen(12345);
        msg.set_packetcount(99);
        msg.set_loginq_proto_aim("aLogin");
        msg.set_domainq_proto_smb("aDomain");
        msg.set_uri_fullq_proto_http("this/url.htm");
        msg.set_uriq_proto_http("notitUrl");
        msg.set_serverq_proto_http("thisname");
        msg.set_referer_serverq_proto_http("notitServer");
        msg.set_methodq_proto_ftp("TEST|COMMAND");
        msg.set_senderq_proto_smtp("test1");
        msg.set_receiverq_proto_smtp("test2");
        msg.set_subjectq_proto_smtp("test3");
        msg.set_versionq_proto_http("4.0");
        msg.set_filenameq_proto_gnutella("aFilename");
        msg.set_filename_encodingq_proto_aim_transfer("notitFile");
        msg.set_directoryq_proto_smb("aPath");
        msg.set_starttime(123);
        msg.set_endtime(456);
        msg.set_sessionidq_proto_ymsg(2345);
        string dataToSend;
        msg.GetBuffer(dataToSend);
        sendQueue.SendData(dataToSend);

        // Allow DPI Syslog Thread time to process the message
        timespec timeToSleep;
        timeToSleep.tv_sec = 0;
        timeToSleep.tv_nsec = 200000000;
        nanosleep(&timeToSleep, NULL);

        //std::cout << "SyslogOutput: " << sysLogOutput << std::endl;
        // Did the data show up in the syslog output
        ASSERT_EQ(1, sysLogOutput.size());
        EXPECT_NE(std::string::npos, sysLogOutput[0].find("EVT:001 "));
        EXPECT_NE(std::string::npos, sysLogOutput[0].find(testUuid));
        EXPECT_NE(std::string::npos, sysLogOutput[0].find("10.1.10.50,10.128.64.251,12345,54321,00:22:19:08:2c:00,f0:f7:55:dc:a8:00,12,dummy,12345,99,123,456"));
        EXPECT_NE(std::string::npos, sysLogOutput[0].find("login=aLogin"));
        EXPECT_NE(std::string::npos, sysLogOutput[0].find("domain=aDomain"));
        EXPECT_NE(std::string::npos, sysLogOutput[0].find("url=this/url.htm"));
        EXPECT_NE(std::string::npos, sysLogOutput[0].find("dname=thisname"));
        EXPECT_NE(std::string::npos, sysLogOutput[0].find("command=TEST|COMMAND"));
        EXPECT_NE(std::string::npos, sysLogOutput[0].find("sender=test1"));
        EXPECT_NE(std::string::npos, sysLogOutput[0].find("recipient=test2"));
        EXPECT_NE(std::string::npos, sysLogOutput[0].find("subject=test3"));
        EXPECT_NE(std::string::npos, sysLogOutput[0].find("version=4.0"));
        EXPECT_NE(std::string::npos, sysLogOutput[0].find("objectname=aFilename"));
        EXPECT_EQ(std::string::npos, sysLogOutput[0].find("notit"));
        EXPECT_NE(std::string::npos, sysLogOutput[0].find("object=aPath"));
        EXPECT_NE(std::string::npos, sysLogOutput[0].find("session=2345"));

        dpiSyslog.join();
    }
#endif
}

TEST_F(RuleEngineTest, testMsgReceiveSiemModeDebug) {
#if defined(LR_DEBUG)
    if (geteuid() != 0) {
        MockConfSlave myConfSlave;

        myConfSlave.SetPath("resources/test.yaml");
        myConfSlave.mConf.mSiemLogging = true;
        myConfSlave.mConf.mSiemDebug = true;
        //myConfSlave.Start();
        protoMsg::BaseConf confMsg;
        protoMsg::SyslogConf sysMsg;
        sysMsg.set_siemlogging("true");
        Conf myConf = conf.GetConf();
        myConf.updateFields(sysMsg);
        myConf.sendConfigUpdate();
        boost::this_thread::sleep(boost::posix_time::seconds(1));
        MockRuleEngine dpiSyslog(myConfSlave, syslogName, syslogOption,
                syslogFacility, syslogPriority, true, 0);
        dpiSyslog.mSiemMode = true;

        dpiSyslog.mSiemDebugMode = true;
        dpiSyslog.start();
        EXPECT_TRUE(dpiSyslog.isRunning());

        // Send a message to the thread and check the syslog output contains the
        // data send.
        std::string queueName = dpiSyslog.GetDpiRcvrQueue();
        queueName += "0";
        SendDpiMsgLRZMQ sendQueue(queueName);
        sendQueue.Initialize();

        DpiMsgLR msg;

        string testUuid("8a3461dc-4aaa-41d5-bf3f-f55037d5ed25");
        msg.set_uuid(testUuid.c_str());

        string testEthSrc("00:22:19:08:2c:00");
        vector<unsigned char> ethSrc;
        ethSrc.push_back(0x00);
        ethSrc.push_back(0x22);
        ethSrc.push_back(0x19);
        ethSrc.push_back(0x08);
        ethSrc.push_back(0x2c);
        ethSrc.push_back(0x00);
        msg.SetEthSrc(ethSrc);

        string testEthDst("f0:f7:55:dc:a8:00");

        vector<unsigned char> ethDst;
        ethDst.push_back(0xf0);
        ethDst.push_back(0xf7);
        ethDst.push_back(0x55);
        ethDst.push_back(0xdc);
        ethDst.push_back(0xa8);
        ethDst.push_back(0x00);
        msg.SetEthDst(ethDst);

        string testIpSrc = "10.1.10.50";
        uint32_t ipSrc = 0x320A010A; // 10.1.10.50, note: little endian
        msg.set_ipsrc(ipSrc);

        string testIpDst = "10.128.64.251";
        uint32_t ipDst = 0xFB40800A; // 10.128.64.251, note: little endian
        msg.set_ipdst(ipDst);

        string path("base.eth.ip.udp.ntp");
        msg.set_pktpath(path.c_str());

        string testIpSourcePort = "=12345"; // bogus, but easier to test
        msg.set_sourceport(12345);

        string testIpDestPort = "=54321"; // bogus, but easier to test
        msg.set_destport(54321);
        msg.set_protoid(12);
        msg.set_application_id_endq_proto_base(13);
        msg.set_application_endq_proto_base("wrong|dummy");
        msg.set_sessionlen(12345);
        msg.set_packetcount(99);
        msg.set_loginq_proto_aim("aLogin");
        msg.set_domainq_proto_smb("aDomain");
        msg.set_uri_fullq_proto_http("this/url.htm");
        msg.set_uriq_proto_http("notitUrl");
        msg.set_serverq_proto_http("thisname");
        msg.set_referer_serverq_proto_http("notitServer");
        msg.set_methodq_proto_ftp("TEST|COMMAND");
        msg.set_senderq_proto_smtp("test1");
        msg.set_receiverq_proto_smtp("test2");
        msg.set_subjectq_proto_smtp("test3");
        msg.set_versionq_proto_http("4.0");
        msg.set_filenameq_proto_gnutella("aFilename");
        msg.set_filename_encodingq_proto_aim_transfer("notitFile");
        msg.set_directoryq_proto_smb("aPath");
        msg.set_starttime(123);
        msg.set_endtime(456);
        msg.set_sessionidq_proto_ymsg(2345);
        string dataToSend;
        msg.GetBuffer(dataToSend);
        sendQueue.SendData(dataToSend);

        // Allow DPI Syslog Thread time to process the message
        timespec timeToSleep;
        timeToSleep.tv_sec = 0;
        timeToSleep.tv_nsec = 200000000;
        nanosleep(&timeToSleep, NULL);

        //std::cout << "SyslogOutput: " << sysLogOutput << std::endl;
        // Did the data show up in the syslog output
        //      for ( int i = 0 ; i < sysLogOutput.size() ; i++) {
        //          std::cout << sysLogOutput[i] << std::endl;
        //      }
        ASSERT_EQ(2, sysLogOutput.size());
        std::string testUuidWithNumber = testUuid;
        testUuidWithNumber += ":00";
        EXPECT_NE(std::string::npos, sysLogOutput[0].find("EVT:001 "));
        EXPECT_NE(std::string::npos, sysLogOutput[0].find(testUuidWithNumber));
        EXPECT_NE(std::string::npos, sysLogOutput[0].find("10.1.10.50,10.128.64.251,12345,54321,00:22:19:08:2c:00,f0:f7:55:dc:a8:00,12,dummy,12345,99,123,456"));
        EXPECT_NE(std::string::npos, sysLogOutput[0].find("login=aLogin"));
        EXPECT_NE(std::string::npos, sysLogOutput[0].find("domain=aDomain"));
        EXPECT_NE(std::string::npos, sysLogOutput[0].find("url=this/url.htm"));
        EXPECT_NE(std::string::npos, sysLogOutput[0].find("dname=thisname"));
        EXPECT_NE(std::string::npos, sysLogOutput[0].find("command=TEST|COMMAND"));
        EXPECT_NE(std::string::npos, sysLogOutput[0].find("sender=test1"));
        EXPECT_NE(std::string::npos, sysLogOutput[0].find("recipient=test2"));
        EXPECT_NE(std::string::npos, sysLogOutput[0].find("subject=test3"));
        EXPECT_NE(std::string::npos, sysLogOutput[0].find("version=4.0"));
        EXPECT_NE(std::string::npos, sysLogOutput[0].find("objectname=aFilename"));
        EXPECT_EQ(std::string::npos, sysLogOutput[0].find("notit"));
        EXPECT_NE(std::string::npos, sysLogOutput[0].find("object=aPath"));
        EXPECT_NE(std::string::npos, sysLogOutput[0].find("session=2345"));

        testUuidWithNumber = testUuid;
        testUuidWithNumber += ":01";
        EXPECT_NE(std::string::npos, sysLogOutput[1].find("EVT:002 "));
        EXPECT_NE(std::string::npos, sysLogOutput[1].find(testUuidWithNumber));
        EXPECT_NE(std::string::npos, sysLogOutput[1].find("10.1.10.50,10.128.64.251,12345,54321,00:22:19:08:2c:00,f0:f7:55:dc:a8:00,12,dummy,12345,99,123,456"));
        EXPECT_EQ(std::string::npos, sysLogOutput[1].find("EndTime=456"));
        EXPECT_NE(std::string::npos, sysLogOutput[1].find("applicationEnd=wrong|dummy"));
        EXPECT_NE(std::string::npos, sysLogOutput[1].find("applicationIdEnd=13"));
        EXPECT_NE(std::string::npos, sysLogOutput[1].find("login=aLogin"));
        EXPECT_NE(std::string::npos, sysLogOutput[1].find("filenameEncoding=notitFile"));
        EXPECT_NE(std::string::npos, sysLogOutput[1].find("method=TEST|COMMAND"));
        EXPECT_NE(std::string::npos, sysLogOutput[1].find("filename=aFilename"));
        EXPECT_NE(std::string::npos, sysLogOutput[1].find("server=thisname"));
        EXPECT_NE(std::string::npos, sysLogOutput[1].find("refererServer=notitServer"));
        EXPECT_NE(std::string::npos, sysLogOutput[1].find("uri=notitUrl"));
        EXPECT_NE(std::string::npos, sysLogOutput[1].find("uriFull=this/url.htm"));
        EXPECT_NE(std::string::npos, sysLogOutput[1].find("version=4.0"));
        EXPECT_NE(std::string::npos, sysLogOutput[1].find("directory=aPath"));
        EXPECT_NE(std::string::npos, sysLogOutput[1].find("domain=aDomain"));
        EXPECT_NE(std::string::npos, sysLogOutput[1].find("sender=test1"));
        EXPECT_NE(std::string::npos, sysLogOutput[1].find("receiver=test2"));
        EXPECT_NE(std::string::npos, sysLogOutput[1].find("subject=test3"));
        EXPECT_NE(std::string::npos, sysLogOutput[1].find("sessionid=2345"));

        dpiSyslog.join();
    }
#endif
}

TEST_F(RuleEngineTest, getSyslogMessagesBufferTooSmall) {
    map<unsigned int, pair <string, string> > dataPairs;
    vector<string> messages;
    MockRuleEngine dm(conf, syslogName, syslogOption,
            syslogFacility, syslogPriority, true, 0);

    dataPairs[0] = make_pair("bad0", "one");
    dataPairs[1] = make_pair("test0", "me0");
    dataPairs[2] = make_pair("test1", "me1");
    dataPairs[3] = make_pair("test2", "me2");
    dataPairs[4] = make_pair("test3", "me3");
    dataPairs[5] = make_pair("test4", "me4");
    dataPairs[6] = make_pair("test5", "me5");
    dataPairs[7] = make_pair("bad2", "one2");
    dataPairs[10000] = make_pair("bad2", "one3");
    dm.SetMaxSize(11 * 5 + 8);
    ASSERT_FALSE(dm.GetSyslogMessages(dataPairs, messages, DYNAMIC_DATA_START));
}

TEST_F(RuleEngineTest, getSyslogMessages) {
#if defined(LR_DEBUG)
    MockRuleEngine dm(conf, syslogName, syslogOption,
            syslogFacility, syslogPriority, true, 0);
    map<unsigned int, pair<string, string> > results;

    tDpiMessage.set_ipsrc(0x0a0b0c0d);
    tDpiMessage.set_ipdst(0x01020304);
    tDpiMessage.set_ethsrc(0x00000a0b0c0d0e0f);
    tDpiMessage.set_ethdst(0x0000010203040506);
    tDpiMessage.set_uuid("01234567-89ab-cdef-0123456789abcdef");
    string path = "foo.bar";
    tDpiMessage.set_pktpath(path);
    tDpiMessage.set_sourceport(1234);
    tDpiMessage.set_destport(5678);

    tDpiMessage.set_filesizeq_proto_bittorrent(1212);
    tDpiMessage.set_filesizeq_proto_edonkey(12345678901234L);
    tDpiMessage.set_attach_filenameq_proto_facebook_mail("this is a file");

    results = tDpiMessage.GetAllFieldsAsStrings();
    ASSERT_EQ(11, results.size());
    vector<string> syslogMessages;
    map<unsigned int, pair<string, string> > formattedFieldData =
            tDpiMessage.GetAllFieldsAsStrings();
    dm.SetMaxSize(2048);
    ASSERT_TRUE(dm.GetSyslogMessages(formattedFieldData, syslogMessages, DYNAMIC_DATA_START));
    ASSERT_EQ(1, syslogMessages.size());
    EXPECT_EQ("EVT:003 UUID=01234567-89ab-cdef-0123456789abcdef, "
            "EthSrc=0f:0e:0d:0c:0b:0a, EthDst=06:05:04:03:02:01, "
            "IpSrc=13.12.11.10, IpDst=4.3.2.1, Path=foo.bar, "
            "SourcePort=1234, DestPort=5678, filesize=1212, "
            "filesize=12345678901234, attachFilename=this is a file",
            syslogMessages[0]);
#endif

}

TEST_F(RuleEngineTest, getSyslogMessagesBigMessage) {
#if defined(LR_DEBUG)
    MockRuleEngine dm(conf, syslogName, syslogOption,
            syslogFacility, syslogPriority, true, 0);
    map<unsigned int, pair <string, string> > dataPairs;
    vector<string> messages;

    dataPairs[1] = make_pair("0", "0");
    dataPairs[2] = make_pair("1", "1");
    dataPairs[3] = make_pair("2", "2");
    dataPairs[4] = make_pair("3", "3");
    dataPairs[5] = make_pair("4", "4");
    dataPairs[6] = make_pair("5", "5");
    dataPairs[8] = make_pair("6", "6");
    dataPairs[9] = make_pair("7", "7");
    dataPairs[18] = make_pair("a", "10");
    dataPairs[19] = make_pair("b", "123456789aaaaaaaaaaaaaaaa0|aaaaaaaaa|bbbbbbbb");
    dataPairs[20] = make_pair("c", "1");

    dm.SetMaxSize((7 * 5 + 3) + 8 + 8);

    ASSERT_TRUE(dm.GetSyslogMessages(dataPairs, messages, DYNAMIC_DATA_START));
    //   for (int i = 0 ; i < messages.size(); i++) {
    //   std::cout << messages[i] << std::endl;
    //   }
    ASSERT_EQ(3, messages.size());
    ASSERT_EQ("EVT:003 0=0, 1=1, 2=2, 3=3, 4=4, 5=5, 6=6, 7=7, a=10", messages[0]);
    ASSERT_EQ("EVT:003 0=0, b=123456789aaaaaaaaaaaaaaaa0|aaaaaaaaa", messages[1]);
    ASSERT_EQ("EVT:003 0=0, b=bbbbbbbb, c=1", messages[2]);
#endif
}

TEST_F(RuleEngineTest, getSyslogMessagesSplitDataTest) {
#if defined(LR_DEBUG)
    MockRuleEngine dm(conf, syslogName, syslogOption,
            syslogFacility, syslogPriority, true, 0);
    map<unsigned int, pair <string, string> > dataPairs;
    vector<string> messages;


    dataPairs[1] = make_pair("UUID", "57c4384a-15b7-44c0-9814-b2e95b23dd15");
    dataPairs[2] = make_pair("EthSrc", "f0:f7:55:dc:a8:7f");
    dataPairs[3] = make_pair("EthDst", "84:18:88:7b:db:04");
    dataPairs[4] = make_pair("IpSrc", "10.128.24.59");
    dataPairs[5] = make_pair("IpDst", "192.168.178.21");
    dataPairs[6] = make_pair("Path", "base.eth.ip.tcp|base.eth.ip.tcp.http");
    dataPairs[7] = make_pair("SourcePort", "52421");
    dataPairs[8] = make_pair("DestPort", "80");
    dataPairs[9] = make_pair("FlowCompleted", "true");
    dataPairs[10] = make_pair("application", "tcp|http");
    dataPairs[11] = make_pair("flowId", "49649");
    dataPairs[12] = make_pair("family", "Network Service|Web");
    dataPairs[13] = make_pair("applicationId", "67");
    dataPairs[14] = make_pair("session", "300");
    dataPairs[15] = make_pair("dev", "eth0");
    dataPairs[16] = make_pair("declassified", "67");
    dataPairs[17] = make_pair("applicationEnd", "tcp|http");
    dataPairs[18] = make_pair("familyEnd", "Network Service|Web");
    dataPairs[19] = make_pair("applicationIdEnd", "67");
    dataPairs[20] = make_pair("sessionLen", "74256");
    dataPairs[21] = make_pair("server", "192.168.178.21");
    dataPairs[22] = make_pair("referer", "http://192.168.178.21/frameset/upper/|http://192.168.178.21/frameset/");
    dataPairs[23] = make_pair("referer", "http://192.168.178.21/frameset/upper/|http://192.168.178.21/frameset/");
    dataPairs[24] = make_pair("refererServer", "192.168.178.21");
    dataPairs[25] = make_pair("uri", "/ha/status_json|/activity/query/?query=partition&query=config&query=threatLevel");
    dataPairs[26] = make_pair("uri", "uri=/ha/status_json|/activity/query/");
    dataPairs[27] = make_pair("uriFull", "/ha/status_json|/activity/query/?query=partition&query=config&query=threatLevel");
    dataPairs[28] = make_pair("userAgent", "Mozilla/4.0 (compatible; MSIE 7.0; Windows NT 6.1; WOW64; Trident/5.0; SLCC2; .NET CLR 2.0.50727; .NET CLR 3.5.30729; .NET CLR 3.0.30729; Media Center PC 6.0; .NET4.0C; .NET4.0E; MDDR)");
    dataPairs[29] = make_pair("mime", "text/html");
    dataPairs[30] = make_pair("method", "GET");
    dataPairs[31] = make_pair("version", "1.1");
    dataPairs[32] = make_pair("serverAgent", "Apache/2.2.14 (Ubuntu) mod_ssl/2.2.14 OpenSSL/0.9.8k mod_fastcgi/2.4.6");
    dataPairs[33] = make_pair("rttQ_PROTO_HTTP", "0.236118|0.720132|0.721577|0.726061|0.730292|0.712930|0.717017|0.689391|0.744431|0.725071|0.732237|0.719680|0.720685|0.731514|0.733816|0.763178|0.219743|0.719475|0.723441|0.734603|0.706938|0.692968|0.658081|0.731333|0.739257|0.737810|0.725861|0.731982|0.727546|0.721841|0.704078|0.709424|0.232954|0.764327|0.717029|0.724730|0.715977|0.705920|0.763877|0.728564|0.738548|0.727176|0.714079|0.714557|0.694057|0.726929|0.701185|0.727373|0.227517|0.699752|0.765659|0.732002|0.736440|0.720890|0.724230|0.719586|0.729707|0.712549|0.715832|0.723022|0.722323|0.722087|0.712564|0.714766|0.725085|0.722592|0.718138|0.732483|0.732279|0.758076|0.711826|0.730520|0.216783|0.718176|0.719815|0.711295|0.694291|0.771365|0.722658|0.707148|0.729696|0.717143|0.718489|0.714692|0.712587|0.736150|0.719579|0.705759|0.216820|0.760850|0.703886|0.710497|0.720210|0.712620|0.733677|0.716371|0.722511|0.712855|0.764938|0.702055|0.678128");
    dataPairs[34] = make_pair("header_nameQ_PROTO_HTTP", "headerName=x-requested-with|Accept-Language|Referer|Accept|Accept-Encoding|User-Agent|Host|Connection|Date|Server|Content-Length|Expires|Pragma|Cache-Control|Set-Cookie|Keep-Alive|Content-Type|Transfer-Encoding");
    dataPairs[35] = make_pair("header_valueQ_PROTO_HTTP", "XMLHttpRequest|en-us|http://192.168.178.21/frameset/upper/http://192.168.178.21/frameset/upper/|application/json, text/javascript, */*|gzip, deflate");

    dm.SetMaxSize(2048);
    ASSERT_TRUE(dm.GetSyslogMessages(dataPairs, messages, DYNAMIC_DATA_START));
    ASSERT_EQ(2, messages.size());

    // Try a different max size.
    messages.clear();
    dm.SetMaxSize(1024);
    ASSERT_TRUE(dm.GetSyslogMessages(dataPairs, messages, DYNAMIC_DATA_START));
    ASSERT_EQ(4, messages.size());

    // Try a different max size.
    messages.clear();
    dm.SetMaxSize(512 + 8);

    ASSERT_TRUE(dm.GetSyslogMessages(dataPairs, messages, DYNAMIC_DATA_START));
    //   for (int i = 0; i < messages.size(); i++) {
    //      std::cout << messages[i] << std::endl;
    //   }
    ASSERT_EQ(6, messages.size());
    string expected = "EVT:003 UUID=57c4384a-15b7-44c0-9814-b2e95b23dd15, EthSrc=f0:f7:55:dc:a8:7f, EthDst=84:18:88:7b:db:04, IpSrc=10.128.24.59, IpDst=192.168.178.21, Path=base.eth.ip.tcp|base.eth.ip.tcp.http, DestPort=80, FlowCompleted=true, application=tcp|http, flowId=49649, family=Network Service|Web, applicationId=67, session=300, dev=eth0, declassified=67, applicationEnd=tcp|http, familyEnd=Network Service|Web, applicationIdEnd=67, sessionLen=74256, server=192.168.178.21";
    EXPECT_EQ(expected, messages[0]);
    expected = "EVT:003 UUID=57c4384a-15b7-44c0-9814-b2e95b23dd15, referer=http://192.168.178.21/frameset/upper/|http://192.168.178.21/frameset/, referer=http://192.168.178.21/frameset/upper/|http://192.168.178.21/frameset/, refererServer=192.168.178.21, uri=/ha/status_json|/activity/query/?query=partition&query=config&query=threatLevel, uri=uri=/ha/status_json|/activity/query/, uriFull=/ha/status_json|/activity/query/?query=partition&query=config&query=threatLevel";
    EXPECT_EQ(expected, messages[1]);
    expected = "EVT:003 UUID=57c4384a-15b7-44c0-9814-b2e95b23dd15, userAgent=Mozilla/4.0 (compatible; MSIE 7.0; Windows NT 6.1; WOW64; Trident/5.0; SLCC2; .NET CLR 2.0.50727; .NET CLR 3.5.30729; .NET CLR 3.0.30729; Media Center PC 6.0; .NET4.0C; .NET4.0E; MDDR), mime=text/html, method=GET, version=1.1, serverAgent=Apache/2.2.14 (Ubuntu) mod_ssl/2.2.14 OpenSSL/0.9.8k mod_fastcgi/2.4.6";
    EXPECT_EQ(expected, messages[2]);
    expected = "EVT:003 UUID=57c4384a-15b7-44c0-9814-b2e95b23dd15, rttQ_PROTO_HTTP=0.236118|0.720132|0.721577|0.726061|0.730292|0.712930|0.717017|0.689391|0.744431|0.725071|0.732237|0.719680|0.720685|0.731514|0.733816|0.763178|0.219743|0.719475|0.723441|0.734603|0.706938|0.692968|0.658081|0.731333|0.739257|0.737810|0.725861|0.731982|0.727546|0.721841|0.704078|0.709424|0.232954|0.764327|0.717029|0.724730|0.715977|0.705920|0.763877|0.728564|0.738548|0.727176|0.714079|0.714557|0.694057|0.726929|0.701185|0.727373|0.227517|0.699752";
    EXPECT_EQ(expected, messages[3]);
    expected = "EVT:003 UUID=57c4384a-15b7-44c0-9814-b2e95b23dd15, rttQ_PROTO_HTTP=0.765659|0.732002|0.736440|0.720890|0.724230|0.719586|0.729707|0.712549|0.715832|0.723022|0.722323|0.722087|0.712564|0.714766|0.725085|0.722592|0.718138|0.732483|0.732279|0.758076|0.711826|0.730520|0.216783|0.718176|0.719815|0.711295|0.694291|0.771365|0.722658|0.707148|0.729696|0.717143|0.718489|0.714692|0.712587|0.736150|0.719579|0.705759|0.216820|0.760850|0.703886|0.710497|0.720210|0.712620|0.733677|0.716371|0.722511|0.712855|0.764938|0.702055";
    EXPECT_EQ(expected, messages[4]);
    expected = "EVT:003 UUID=57c4384a-15b7-44c0-9814-b2e95b23dd15, rttQ_PROTO_HTTP=0.678128, header_nameQ_PROTO_HTTP=headerName=x-requested-with|Accept-Language|Referer|Accept|Accept-Encoding|User-Agent|Host|Connection|Date|Server|Content-Length|Expires|Pragma|Cache-Control|Set-Cookie|Keep-Alive|Content-Type|Transfer-Encoding, header_valueQ_PROTO_HTTP=XMLHttpRequest|en-us|http://192.168.178.21/frameset/upper/http://192.168.178.21/frameset/upper/|application/json, text/javascript, */*|gzip, deflate";
    EXPECT_EQ(expected, messages[5]);

#endif
}

TEST_F(RuleEngineTest, getSiemSyslogMessagesSplitDataTest) {
#if defined(LR_DEBUG)
    MockRuleEngine dm(conf, syslogName, syslogOption,
            syslogFacility, syslogPriority, true, 0);
    map<unsigned int, pair <string, string> > dataPairs;
    vector<string> messages;

    dm.mSiemMode = true;
    tDpiMessage.set_uuid("550e8400-e29b-41d4-a716-446655440000");
    tDpiMessage.set_ethdst(123);
    tDpiMessage.set_ethsrc(124);
    tDpiMessage.set_ipdst(125);
    tDpiMessage.set_ipsrc(126);
    tDpiMessage.set_sourceport(127);
    tDpiMessage.set_destport(128);
    tDpiMessage.set_protoid(129);
    tDpiMessage.set_application_endq_proto_base("test");
    tDpiMessage.set_application_id_endq_proto_base(1234);
    tDpiMessage.set_sessionlen(567);
    tDpiMessage.set_packetcount(88);
    tDpiMessage.set_loginq_proto_aim("aLogin");
    tDpiMessage.set_domainq_proto_smb("aDomain");
    tDpiMessage.set_uri_fullq_proto_http("this/url.htm");
    tDpiMessage.set_uriq_proto_http("not/this/one");
    tDpiMessage.set_serverq_proto_http("thisname");
    tDpiMessage.set_referer_serverq_proto_http("notThisOne");
    tDpiMessage.set_methodq_proto_ftp("TEST|COMMAND|LONGLONGLONGLONG");
    tDpiMessage.set_senderq_proto_smtp("test1");
    tDpiMessage.set_receiverq_proto_smtp("test2");
    tDpiMessage.set_subjectq_proto_smtp("test3");
    tDpiMessage.set_versionq_proto_http("4.0");
    tDpiMessage.set_starttime(123);
    tDpiMessage.set_endtime(456);
    dm.SetMaxSize(512 + 8 + 36 + 4);
    messages = dm.GetSiemSyslogMessage(tDpiMessage);
    //   for (int i = 0; i < messages.size(); i++) {
    //      std::cout << messages[i] << std::endl;
    //   }
    ASSERT_EQ(1, messages.size());
    std::string expectedHeader = "EVT:001 550e8400-e29b-41d4-a716-446655440000:";
    std::string expectedHeader2 = " 126.0.0.0,125.0.0.0,127,128,7c:00:00:00:00:00,7b:00:00:00:00:00,129,test,567,88,123,456";
    std::string expectedHeaderNoCounts = " 126.0.0.0,125.0.0.0,127,128,7c:00:00:00:00:00,7b:00:00:00:00:00,129,test,,,123,456";
    std::string expected;

    expected = BuildExpectedHeaderForSiem(expectedHeader, expectedHeader2, 0);
    expected += ",login=aLogin,domain=aDomain,dname=thisname,url=this/url.htm,command=TEST|COMMAND|LONGLONGLONGLONG,sender=test1,recipient=test2,subject=test3,version=4.0";
    EXPECT_EQ(expected, messages[0]);
    messages.clear();
    dm.SetMaxSize(108 + 8 + 36 + 4 - 5);
    messages = dm.GetSiemSyslogMessage(tDpiMessage);
    //   for (int i = 0; i < messages.size(); i++) {
    //      std::cout << messages[i] << std::endl;
    //   }
    ASSERT_EQ(11, messages.size());
    unsigned int index = 0;

    expected = BuildExpectedHeaderForSiem(expectedHeader, expectedHeader2, index);
    expected += ",login=aLogin";
    EXPECT_EQ(expected, messages[index++]);
    expected = BuildExpectedHeaderForSiem(expectedHeader, expectedHeaderNoCounts, index);
    expected += ",domain=aDomain";
    EXPECT_EQ(expected, messages[index++]);
    expected = BuildExpectedHeaderForSiem(expectedHeader, expectedHeaderNoCounts, index);
    expected += ",dname=thisname";
    EXPECT_EQ(expected, messages[index++]);
    expected = BuildExpectedHeaderForSiem(expectedHeader, expectedHeaderNoCounts, index);
    expected += ",url=this/url.htm";
    EXPECT_EQ(expected, messages[index++]);
    expected = BuildExpectedHeaderForSiem(expectedHeader, expectedHeaderNoCounts, index);
    expected += ",command=TEST|COMMAND";
    EXPECT_EQ(expected, messages[index++]);
    expected = BuildExpectedHeaderForSiem(expectedHeader, expectedHeaderNoCounts, index);
    expected += ",command=LONGLONGLONG";
    EXPECT_EQ(expected, messages[index++]);
    expected = BuildExpectedHeaderForSiem(expectedHeader, expectedHeaderNoCounts, index);
    expected += ",command=LONG";
    EXPECT_EQ(expected, messages[index++]);
    expected = BuildExpectedHeaderForSiem(expectedHeader, expectedHeaderNoCounts, index);
    expected += ",sender=test1";
    EXPECT_EQ(expected, messages[index++]);
    expected = BuildExpectedHeaderForSiem(expectedHeader, expectedHeaderNoCounts, index);
    expected += ",recipient=test2";
    EXPECT_EQ(expected, messages[index++]);
    expected = BuildExpectedHeaderForSiem(expectedHeader, expectedHeaderNoCounts, index);
    expected += ",subject=test3";
    EXPECT_EQ(expected, messages[index++]);
    expected = BuildExpectedHeaderForSiem(expectedHeader, expectedHeaderNoCounts, index);
    expected += ",version=4.0";
    EXPECT_EQ(expected, messages[index++]);
#endif
}

TEST_F(RuleEngineTest, CrazyData) {
#if defined(LR_DEBUG)
    MockRuleEngine dm(conf, syslogName, syslogOption,
            syslogFacility, syslogPriority, true, 0);
    map<unsigned int, pair <string, string> > dataPairs;
    vector<string> messages;

    dataPairs[1] = make_pair("0", "0");
    dataPairs[2] = make_pair("1", "1");
    dataPairs[3] = make_pair("2", "2");
    dataPairs[4] = make_pair("3", "3");
    dataPairs[5] = make_pair("4", "4");
    dataPairs[6] = make_pair("5", "5");
    dataPairs[8] = make_pair("6", "6");
    dataPairs[9] = make_pair("7", "7");
    dataPairs[18] = make_pair("a", "10");
    dataPairs[19] = make_pair("b", "123456789aaaaaaaaaaaaaaaa0|aaaaaaaaa|bbbbbbbb1234567890123456789012345678900aaaa");
    dm.SetMaxSize((7 * 5 + 3) + 8 + 8);
    ASSERT_TRUE(dm.GetSyslogMessages(dataPairs, messages, DYNAMIC_DATA_START));
    //      for (int i = 0 ; i < messages.size(); i++) {
    //   std::cout << messages[i] << std::endl;
    //   }
    ASSERT_EQ(4, messages.size());
    ASSERT_EQ("EVT:003 0=0, 1=1, 2=2, 3=3, 4=4, 5=5, 6=6, 7=7, a=10", messages[0]);
    ASSERT_EQ("EVT:003 0=0, b=123456789aaaaaaaaaaaaaaaa0|aaaaaaaaa", messages[1]);
    ASSERT_EQ("EVT:003 0=0, b=bbbbbbbb1234567890123456789012345678900", messages[2]);
    ASSERT_EQ("EVT:003 0=0, b=aaaa", messages[3]);
#endif
}

TEST_F(RuleEngineTest, getSyslogMessagesLongMessage) {
#if defined(LR_DEBUG)
    MockRuleEngine dm(conf, syslogName, syslogOption,
            syslogFacility, syslogPriority, true, 0);
    map<unsigned int, pair <string, string> > dataPairs;
    vector<string> messages;

    dataPairs[1] = make_pair("0", "0");
    dataPairs[2] = make_pair("1", "1");
    dataPairs[3] = make_pair("2", "2");
    dataPairs[4] = make_pair("3", "3");
    dataPairs[5] = make_pair("4", "4");
    dataPairs[6] = make_pair("5", "5");
    dataPairs[8] = make_pair("6", "6");
    dataPairs[9] = make_pair("7", "7");
    dataPairs[18] = make_pair("a", "1");
    dataPairs[19] = make_pair("b", "1");
    dataPairs[20] = make_pair("c", "1");
    dataPairs[21] = make_pair("d", "1");
    dataPairs[22] = make_pair("e", "1");
    dataPairs[23] = make_pair("f", "1");
    dm.SetMaxSize((7 * 5 + 3) + 10 + 8);
    ASSERT_TRUE(dm.GetSyslogMessages(dataPairs, messages, DYNAMIC_DATA_START));

    ASSERT_EQ(2, messages.size());
    ASSERT_EQ("EVT:003 0=0, 1=1, 2=2, 3=3, 4=4, 5=5, 6=6, 7=7, a=1, b=1", messages[0]);
    ASSERT_EQ("EVT:003 0=0, c=1, d=1, e=1, f=1", messages[1]);
#endif
}

TEST_F(RuleEngineTest, getStaticInfo) {
#ifdef LR_DEBUG
    map<unsigned int, pair <string, string> > dataPairs;
    MockRuleEngine dm(conf, syslogName, syslogOption,
            syslogFacility, syslogPriority, true, 0);

    dataPairs[0] = make_pair("bad", "one");
    dataPairs[1] = make_pair("test", "me");
    dataPairs[2] = make_pair("test1", "me1");
    dataPairs[3] = make_pair("test2", "me2");
    dataPairs[4] = make_pair("test3", "me3");
    dataPairs[5] = make_pair("test4", "me4");
    dataPairs[6] = make_pair("test5", "me5");
    dataPairs[8] = make_pair("test6", "me6");
    dataPairs[9] = make_pair("test7", "me7");
    dataPairs[7] = make_pair("bad", "one2");
    dataPairs[10000] = make_pair("bad", "one3");

    ASSERT_EQ("test=me, test1=me1, test2=me2, test3=me3, test4=me4, test5=me5, test6=me6, test7=me7", dm.GetStaticInfo(dataPairs, DYNAMIC_DATA_START));
#endif
}

TEST_F(RuleEngineTest, getNextDataPair) {
#ifdef LR_DEBUG
    map<unsigned int, pair <string, string> > dataPairs;
    MockRuleEngine dm(conf, syslogName, syslogOption,
            syslogFacility, syslogPriority, true, 0);

    dataPairs[0] = make_pair("test", "me");
    map<unsigned int, pair <string, string> >::iterator i = dataPairs.begin();
    ASSERT_EQ(", test=me", dm.GetNextDataPair(i));
#endif

}

TEST_F(RuleEngineTest, GetSiemRequiredFieldPairs) {
#ifdef LR_DEBUG
    MockRuleEngine dm(conf, syslogName, syslogOption,
            syslogFacility, syslogPriority, true, 0);
    IndexedFieldPairs results;
    dm.GetSiemRequiredFieldPairs(tDpiMessage, results);
    ASSERT_EQ(13, results.size());
    EXPECT_EQ("sip", results[2].first);
    EXPECT_EQ("dip", results[3].first);
    EXPECT_EQ("sport", results[4].first);
    EXPECT_EQ("dport", results[5].first);
    EXPECT_EQ("smac", results[6].first);
    EXPECT_EQ("dmac", results[8].first);
    EXPECT_EQ("protnum", results[9].first);
    EXPECT_EQ("process", results[10].first);
    EXPECT_EQ("bytesin", results[11].first);
    EXPECT_EQ("packetsin", results[12].first);
    EXPECT_EQ("timestart", results[13].first);
    EXPECT_EQ("timeend", results[14].first);

    EXPECT_EQ("0.0.0.0", results[2].second);
    EXPECT_EQ("0.0.0.0", results[3].second);
    EXPECT_EQ("0", results[4].second);
    EXPECT_EQ("0", results[5].second);
    EXPECT_EQ("00:00:00:00:00:00", results[6].second);
    EXPECT_EQ("00:00:00:00:00:00", results[8].second);
    EXPECT_EQ("0", results[9].second);
    EXPECT_EQ("unknown", results[10].second);
    EXPECT_EQ("0", results[11].second);
    EXPECT_EQ("0", results[12].second);

    tDpiMessage.set_ethdst(123);
    tDpiMessage.set_ethsrc(124);
    tDpiMessage.set_ipdst(125);
    tDpiMessage.set_ipsrc(126);
    tDpiMessage.set_sourceport(127);
    tDpiMessage.set_destport(128);
    tDpiMessage.set_protoid(129);
    tDpiMessage.set_application_endq_proto_base("test");
    tDpiMessage.set_application_id_endq_proto_base(1234);
    tDpiMessage.set_sessionlen(567);
    tDpiMessage.set_packetcount(88);
    tDpiMessage.set_loginq_proto_0zz0("dontSeeMee");
    tDpiMessage.set_starttime(1234);
    tDpiMessage.set_endtime(5678);
    dm.GetSiemRequiredFieldPairs(tDpiMessage, results);
    ASSERT_EQ(13, results.size());
    EXPECT_EQ("sip", results[2].first);
    EXPECT_EQ("dip", results[3].first);
    EXPECT_EQ("sport", results[4].first);
    EXPECT_EQ("dport", results[5].first);
    EXPECT_EQ("smac", results[6].first);
    EXPECT_EQ("dmac", results[8].first);
    EXPECT_EQ("protnum", results[9].first);
    EXPECT_EQ("process", results[10].first);
    EXPECT_EQ("bytesin", results[11].first);
    EXPECT_EQ("packetsin", results[12].first);
    EXPECT_EQ("timestart", results[13].first);
    EXPECT_EQ("timeend", results[14].first);
    EXPECT_EQ("126.0.0.0", results[2].second);
    EXPECT_EQ("125.0.0.0", results[3].second);
    EXPECT_EQ("127", results[4].second);
    EXPECT_EQ("128", results[5].second);
    EXPECT_EQ("7c:00:00:00:00:00", results[6].second);
    EXPECT_EQ("7b:00:00:00:00:00", results[8].second);
    EXPECT_EQ("129", results[9].second);
    EXPECT_EQ("test", results[10].second);
    EXPECT_EQ("567", results[11].second);
    EXPECT_EQ("88", results[12].second);
    EXPECT_EQ("1234", results[13].second);
    EXPECT_EQ("5678", results[14].second);

#endif
}

TEST_F(RuleEngineTest, GetLoginField) {
#ifdef LR_DEBUG
    MockRuleEngine dm(conf, syslogName, syslogOption,
            syslogFacility, syslogPriority, true, 0);
    IndexedFieldPairs results;
    ASSERT_EQ(1, dm.GetLoginField(1, tDpiMessage, results));
    ASSERT_EQ(0, results.size());

    tDpiMessage.set_loginq_proto_0zz0("test1");
    ASSERT_EQ(2, dm.GetLoginField(1, tDpiMessage, results));
    ASSERT_EQ(1, results.size());
    EXPECT_EQ("login", results[1].first);
    EXPECT_EQ("test1", results[1].second);
    results.clear();
    tDpiMessage.Clear();

    tDpiMessage.set_loginq_proto_aim("test1");
    ASSERT_EQ(2, dm.GetLoginField(1, tDpiMessage, results));
    ASSERT_EQ(1, results.size());
    EXPECT_EQ("login", results[1].first);
    EXPECT_EQ("test1", results[1].second);
    results.clear();
    tDpiMessage.Clear();

    tDpiMessage.set_contact_loginq_proto_aim("test1");
    ASSERT_EQ(1, dm.GetLoginField(1, tDpiMessage, results));
    ASSERT_EQ(0, results.size());
    results.clear();
    tDpiMessage.Clear();

    tDpiMessage.set_proxy_loginq_proto_http("test1");
    ASSERT_EQ(1, dm.GetLoginField(1, tDpiMessage, results));
    ASSERT_EQ(0, results.size());
    results.clear();
    tDpiMessage.Clear();

    tDpiMessage.set_login_serverq_proto_imap("test1");
    ASSERT_EQ(1, dm.GetLoginField(1, tDpiMessage, results));
    ASSERT_EQ(0, results.size());
    results.clear();
    tDpiMessage.Clear();
#endif
}

TEST_F(RuleEngineTest, GetDomainField) {
#ifdef LR_DEBUG
    MockRuleEngine dm(conf, syslogName, syslogOption,
            syslogFacility, syslogPriority, true, 0);
    IndexedFieldPairs results;
    ASSERT_EQ(1, dm.GetDomainField(1, tDpiMessage, results));
    ASSERT_EQ(0, results.size());

    tDpiMessage.set_domainq_proto_smb("test1");
    ASSERT_EQ(2, dm.GetDomainField(1, tDpiMessage, results));
    ASSERT_EQ(1, results.size());
    EXPECT_EQ("domain", results[1].first);
    EXPECT_EQ("test1", results[1].second);
    results.clear();
    tDpiMessage.Clear();

    tDpiMessage.set_real_sender_domainq_proto_gmail("test1");
    ASSERT_EQ(2, dm.GetDomainField(1, tDpiMessage, results));
    ASSERT_EQ(1, results.size());
    EXPECT_EQ("domain", results[1].first);
    EXPECT_EQ("test1", results[1].second);
    results.clear();
    tDpiMessage.Clear();

    tDpiMessage.set_vtp_mgmt_domainq_proto_cdp("test1");
    ASSERT_EQ(2, dm.GetDomainField(1, tDpiMessage, results));
    ASSERT_EQ(1, results.size());
    EXPECT_EQ("domain", results[1].first);
    EXPECT_EQ("test1", results[1].second);
    results.clear();
    tDpiMessage.Clear();


#endif
}

TEST_F(RuleEngineTest, GetDestHostField) {
#ifdef LR_DEBUG
    MockRuleEngine dm(conf, syslogName, syslogOption,
            syslogFacility, syslogPriority, true, 0);
    IndexedFieldPairs results;
    ASSERT_EQ(1, dm.GetDestHostField(1, tDpiMessage, results));
    ASSERT_EQ(0, results.size());

    tDpiMessage.set_serverq_proto_http("test1");
    ASSERT_EQ(2, dm.GetDestHostField(1, tDpiMessage, results));
    ASSERT_EQ(1, results.size());
    EXPECT_EQ("dname", results[1].first);
    EXPECT_EQ("test1", results[1].second);
    results.clear();
    tDpiMessage.Clear();


    tDpiMessage.set_destination_hostq_proto_diameter("test1");
    ASSERT_EQ(2, dm.GetDestHostField(1, tDpiMessage, results));
    ASSERT_EQ(1, results.size());
    EXPECT_EQ("dname", results[1].first);
    EXPECT_EQ("test1", results[1].second);
    results.clear();
    tDpiMessage.Clear();

    tDpiMessage.set_origin_hostq_proto_diameter("test1");
    ASSERT_EQ(1, dm.GetDestHostField(1, tDpiMessage, results));
    ASSERT_EQ(0, results.size());
    results.clear();
    tDpiMessage.Clear();

    tDpiMessage.set_server_addrq_proto_ip("test1");
    ASSERT_EQ(1, dm.GetDestHostField(1, tDpiMessage, results));
    ASSERT_EQ(0, results.size());
    results.clear();
    tDpiMessage.Clear();
#endif
}

TEST_F(RuleEngineTest, GetUrlField) {
#ifdef LR_DEBUG
    MockRuleEngine dm(conf, syslogName, syslogOption,
            syslogFacility, syslogPriority, true, 0);
    IndexedFieldPairs results;
    ASSERT_EQ(1, dm.GetUrlField(1, tDpiMessage, results));
    ASSERT_EQ(0, results.size());

    tDpiMessage.set_uriq_proto_http("notit");
    tDpiMessage.set_uri_fullq_proto_http("test1");
    ASSERT_EQ(2, dm.GetUrlField(1, tDpiMessage, results));
    ASSERT_EQ(1, results.size());
    EXPECT_EQ("url", results[1].first);
    EXPECT_EQ("test1", results[1].second);
    results.clear();
    tDpiMessage.Clear();

    tDpiMessage.set_uri_pathq_proto_http("notit");
    tDpiMessage.set_uriq_proto_http("test1");
    tDpiMessage.set_destination_hostq_proto_diameter("test1");
    ASSERT_EQ(2, dm.GetUrlField(1, tDpiMessage, results));
    ASSERT_EQ(1, results.size());
    EXPECT_EQ("url", results[1].first);
    EXPECT_EQ("test1", results[1].second);
    results.clear();
    tDpiMessage.Clear();

    tDpiMessage.set_ad_urlq_proto_google_ads("notit");
    tDpiMessage.set_ad_url_fullq_proto_google_ads("test1");
    tDpiMessage.set_destination_hostq_proto_diameter("test1");
    ASSERT_EQ(2, dm.GetUrlField(1, tDpiMessage, results));
    ASSERT_EQ(1, results.size());
    EXPECT_EQ("url", results[1].first);
    EXPECT_EQ("test1", results[1].second);
    results.clear();
    tDpiMessage.Clear();

    tDpiMessage.set_ad_urlq_proto_google_ads("test1");
    tDpiMessage.set_ad_visible_urlq_proto_google_ads("notit");
    ASSERT_EQ(2, dm.GetUrlField(1, tDpiMessage, results));
    ASSERT_EQ(1, results.size());
    EXPECT_EQ("url", results[1].first);
    EXPECT_EQ("test1", results[1].second);
    results.clear();
    tDpiMessage.Clear();

    tDpiMessage.set_urilast64q_proto_rtsp("notit");
    ASSERT_EQ(1, dm.GetUrlField(1, tDpiMessage, results));
    ASSERT_EQ(0, results.size());
    results.clear();
    tDpiMessage.Clear();

    tDpiMessage.set_ad_visible_urlq_proto_google_ads("test1");
    ASSERT_EQ(2, dm.GetUrlField(1, tDpiMessage, results));
    ASSERT_EQ(1, results.size());
    EXPECT_EQ("url", results[1].first);
    EXPECT_EQ("test1", results[1].second);
    results.clear();
    tDpiMessage.Clear();
#endif
}

TEST_F(RuleEngineTest, GetCommandField) {
#ifdef LR_DEBUG
    MockRuleEngine dm(conf, syslogName, syslogOption,
            syslogFacility, syslogPriority, true, 0);
    IndexedFieldPairs results;
    ASSERT_EQ(1, dm.GetCommandField(1, tDpiMessage, results));
    ASSERT_EQ(0, results.size());

    tDpiMessage.set_command_stringq_proto_smb("test1");
    ASSERT_EQ(2, dm.GetCommandField(1, tDpiMessage, results));
    ASSERT_EQ(1, results.size());
    EXPECT_EQ("command", results[1].first);
    EXPECT_EQ("test1", results[1].second);
    results.clear();
    tDpiMessage.Clear();

    tDpiMessage.set_methodq_proto_ftp("test1");
    ASSERT_EQ(2, dm.GetCommandField(1, tDpiMessage, results));
    ASSERT_EQ(1, results.size());
    EXPECT_EQ("command", results[1].first);
    EXPECT_EQ("test1", results[1].second);
    results.clear();
    tDpiMessage.Clear();

    tDpiMessage.set_command_codeq_proto_diameter(1);
    ASSERT_EQ(1, dm.GetCommandField(1, tDpiMessage, results));
    ASSERT_EQ(0, results.size());
    results.clear();
    tDpiMessage.Clear();

    tDpiMessage.set_h245_methodq_proto_h225("notit");
    ASSERT_EQ(1, dm.GetCommandField(1, tDpiMessage, results));
    ASSERT_EQ(0, results.size());
    results.clear();
    tDpiMessage.Clear();

#endif
}

TEST_F(RuleEngineTest, GetSenderField) {
#ifdef LR_DEBUG
    MockRuleEngine dm(conf, syslogName, syslogOption,
            syslogFacility, syslogPriority, true, 0);
    IndexedFieldPairs results;
    ASSERT_EQ(1, dm.GetSenderField(1, tDpiMessage, results));
    ASSERT_EQ(0, results.size());

    tDpiMessage.set_senderq_proto_aim("test1");
    ASSERT_EQ(2, dm.GetSenderField(1, tDpiMessage, results));
    ASSERT_EQ(1, results.size());
    EXPECT_EQ("sender", results[1].first);
    EXPECT_EQ("test1", results[1].second);
    results.clear();
    tDpiMessage.Clear();

    tDpiMessage.set_msglist_sender_aliasq_proto_dimp("notit");
    ASSERT_EQ(1, dm.GetSenderField(1, tDpiMessage, results));
    ASSERT_EQ(0, results.size());
    results.clear();
    tDpiMessage.Clear();

#endif
}

TEST_F(RuleEngineTest, GetRecipientField) {
#ifdef LR_DEBUG
    MockRuleEngine dm(conf, syslogName, syslogOption,
            syslogFacility, syslogPriority, true, 0);
    IndexedFieldPairs results;
    ASSERT_EQ(1, dm.GetRecipientField(1, tDpiMessage, results));
    ASSERT_EQ(0, results.size());

    tDpiMessage.set_receiverq_proto_aim("test1");
    ASSERT_EQ(2, dm.GetRecipientField(1, tDpiMessage, results));
    ASSERT_EQ(1, results.size());
    EXPECT_EQ("recipient", results[1].first);
    EXPECT_EQ("test1", results[1].second);
    results.clear();
    tDpiMessage.Clear();

    tDpiMessage.set_msglist_receiver_aliasq_proto_dimp("notit");
    ASSERT_EQ(1, dm.GetSenderField(1, tDpiMessage, results));
    ASSERT_EQ(0, results.size());
    results.clear();
    tDpiMessage.Clear();

#endif
}

TEST_F(RuleEngineTest, GetSubjectField) {
#ifdef LR_DEBUG
    MockRuleEngine dm(conf, syslogName, syslogOption,
            syslogFacility, syslogPriority, true, 0);
    IndexedFieldPairs results;
    ASSERT_EQ(1, dm.GetSubjectField(1, tDpiMessage, results));
    ASSERT_EQ(0, results.size());

    tDpiMessage.set_subjectq_proto_smtp("test1");
    ASSERT_EQ(2, dm.GetSubjectField(1, tDpiMessage, results));
    ASSERT_EQ(1, results.size());
    EXPECT_EQ("subject", results[1].first);
    EXPECT_EQ("test1", results[1].second);
    results.clear();
    tDpiMessage.Clear();

    tDpiMessage.set_msglist_subjectq_proto_dimp("notit");
    ASSERT_EQ(1, dm.GetSubjectField(1, tDpiMessage, results));
    ASSERT_EQ(0, results.size());
    results.clear();
    tDpiMessage.Clear();

#endif
}

TEST_F(RuleEngineTest, GetVersionField) {
#ifdef LR_DEBUG
    MockRuleEngine dm(conf, syslogName, syslogOption,
            syslogFacility, syslogPriority, true, 0);
    IndexedFieldPairs results;
    ASSERT_EQ(1, dm.GetVersionField(1, tDpiMessage, results));
    ASSERT_EQ(0, results.size());

    tDpiMessage.set_versionq_proto_rpc(21);
    ASSERT_EQ(2, dm.GetVersionField(1, tDpiMessage, results));
    ASSERT_EQ(1, results.size());
    EXPECT_EQ("version", results[1].first);
    EXPECT_EQ("21", results[1].second);
    results.clear();
    tDpiMessage.Clear();
    tDpiMessage.set_versionq_proto_skype("test1");
    ASSERT_EQ(2, dm.GetVersionField(1, tDpiMessage, results));
    ASSERT_EQ(1, results.size());
    EXPECT_EQ("version", results[1].first);
    EXPECT_EQ("test1", results[1].second);
    results.clear();
    tDpiMessage.Clear();
    tDpiMessage.set_server_versionq_proto_teamspeak("test1");
    ASSERT_EQ(2, dm.GetVersionField(1, tDpiMessage, results));
    ASSERT_EQ(1, results.size());
    EXPECT_EQ("version", results[1].first);
    EXPECT_EQ("test1", results[1].second);
    results.clear();
    tDpiMessage.Clear();
    tDpiMessage.set_program_versionq_proto_rpc(99);
    ASSERT_EQ(1, dm.GetVersionField(1, tDpiMessage, results));
    ASSERT_EQ(0, results.size());
    results.clear();
    tDpiMessage.Clear();

#endif
}

TEST_F(RuleEngineTest, GetPathField) {
#ifdef LR_DEBUG
    MockRuleEngine dm(conf, syslogName, syslogOption,
            syslogFacility, syslogPriority, true, 0);
    IndexedFieldPairs results;
    ASSERT_EQ(1, dm.GetPathField(1, tDpiMessage, results));
    ASSERT_EQ(0, results.size());

    tDpiMessage.set_directoryq_proto_http("aPath");
    ASSERT_EQ(2, dm.GetPathField(1, tDpiMessage, results));
    ASSERT_EQ(1, results.size());
    EXPECT_EQ("object", results[1].first);
    EXPECT_EQ("aPath", results[1].second);
    results.clear();
    tDpiMessage.Clear();
    tDpiMessage.set_directoryq_proto_smb("test1");
    ASSERT_EQ(2, dm.GetPathField(1, tDpiMessage, results));
    ASSERT_EQ(1, results.size());
    EXPECT_EQ("object", results[1].first);
    EXPECT_EQ("test1", results[1].second);
    results.clear();


#endif
}

TEST_F(RuleEngineTest, GetFilenameField) {
#ifdef LR_DEBUG
    MockRuleEngine dm(conf, syslogName, syslogOption,
            syslogFacility, syslogPriority, true, 0);
    IndexedFieldPairs results;
    ASSERT_EQ(1, dm.GetFilenameField(1, tDpiMessage, results));
    ASSERT_EQ(0, results.size());

    tDpiMessage.set_filenameq_proto_winmx("aName");
    ASSERT_EQ(2, dm.GetFilenameField(1, tDpiMessage, results));
    ASSERT_EQ(1, results.size());
    EXPECT_EQ("objectname", results[1].first);
    EXPECT_EQ("aName", results[1].second);
    results.clear();
    tDpiMessage.Clear();
    tDpiMessage.set_attach_filenameq_proto_yandex_webmail("test1");
    ASSERT_EQ(2, dm.GetFilenameField(1, tDpiMessage, results));
    ASSERT_EQ(1, results.size());
    EXPECT_EQ("objectname", results[1].first);
    EXPECT_EQ("test1", results[1].second);
    results.clear();
    tDpiMessage.Clear();

    tDpiMessage.set_bootfilenameq_proto_dhcp("notit");
    ASSERT_EQ(1, dm.GetFilenameField(1, tDpiMessage, results));
    ASSERT_EQ(0, results.size());


#endif
}

TEST_F(RuleEngineTest, GetSessionField) {
#ifdef LR_DEBUG
    MockRuleEngine dm(conf, syslogName, syslogOption,
            syslogFacility, syslogPriority, true, 0);
    IndexedFieldPairs results;
    ASSERT_EQ(1, dm.GetSessionField(1, tDpiMessage, results));
    ASSERT_EQ(0, results.size());

    tDpiMessage.set_sessionidq_proto_ymsg(1234);
    ASSERT_EQ(2, dm.GetSessionField(1, tDpiMessage, results));
    ASSERT_EQ(1, results.size());
    EXPECT_EQ("session", results[1].first);
    EXPECT_EQ("1234", results[1].second);
    results.clear();
    tDpiMessage.Clear();
    tDpiMessage.set_session_idq_proto_ymail2("test1");
    ASSERT_EQ(2, dm.GetSessionField(1, tDpiMessage, results));
    ASSERT_EQ(1, results.size());
    EXPECT_EQ("session", results[1].first);
    EXPECT_EQ("test1", results[1].second);
    results.clear();
    tDpiMessage.Clear();
    tDpiMessage.set_session_durationq_proto_sip("test1");
    ASSERT_EQ(1, dm.GetSessionField(1, tDpiMessage, results));
    ASSERT_EQ(0, results.size());
    results.clear();
    tDpiMessage.Clear();
    tDpiMessage.set_end_sessionq_proto_rtp("test1");
    ASSERT_EQ(1, dm.GetSessionField(1, tDpiMessage, results));
    ASSERT_EQ(0, results.size());
    results.clear();
    tDpiMessage.Clear();
    tDpiMessage.set_sessionlen(1234);
    ASSERT_EQ(1, dm.GetSessionField(1, tDpiMessage, results));
    ASSERT_EQ(0, results.size());
    results.clear();
    tDpiMessage.Clear();
    tDpiMessage.set_session_packet_counterq_proto_base(1234);
    ASSERT_EQ(1, dm.GetSessionField(1, tDpiMessage, results));
    ASSERT_EQ(0, results.size());
    results.clear();
    tDpiMessage.Clear();
#endif
}
