#include "QosmosConfTest.h"

TEST_F(QosmosConfInfoTest, ConstructorTest) {
   QosmosConf someInfo;
   QosmosConf* pointerInfo = new QosmosConf;
   delete pointerInfo;
}

TEST_F(QosmosConfInfoTest, PopulateEmptyMessage) {
   QosmosConf someInfo;
   ASSERT_EQ(0, someInfo.qosmosprotocol_size());
   ProtocolFamilyMap familyMap;
   ProtocolEnabledMap enabledMap;
   ASSERT_TRUE(someInfo.GetMaps(familyMap, enabledMap));
   ASSERT_TRUE(familyMap.empty());
   ASSERT_TRUE(enabledMap.empty());

   familyMap = mFamilyMap;
   enabledMap = mEnabledMap;
   ASSERT_TRUE(someInfo.ReconcileWithMaps(familyMap, enabledMap));
   ASSERT_EQ(familyMap, mFamilyMap);
   ASSERT_EQ(enabledMap, mEnabledMap);
   familyMap.clear();
   enabledMap.clear();
   ASSERT_TRUE(someInfo.GetMaps(familyMap, enabledMap));
   ASSERT_EQ(familyMap, mFamilyMap);
   ASSERT_EQ(enabledMap, mEnabledMap);
   ASSERT_EQ(2, someInfo.qosmosprotocol_size());
   for (int i = 0; i < someInfo.qosmosprotocol_size(); i++) {
      const protoMsg::QosmosConf::Protocol& existingProtocol = someInfo.qosmosprotocol(i);

      std::string protocolName = existingProtocol.protocolname();
      ASSERT_EQ(familyMap[protocolName].first, existingProtocol.protocolfamily());
      ASSERT_EQ(familyMap[protocolName].second, existingProtocol.protocollongname());
      ASSERT_EQ(enabledMap[protocolName], existingProtocol.protocolenabled());

   }

}

TEST_F(QosmosConfInfoTest, PopulateSimilaressageOldConfigWins) {

   QosmosConf someInfo;
   ASSERT_EQ(0, someInfo.qosmosprotocol_size());

   mEnabledMap["fakeProto"] = false;
   ProtocolFamilyMap familyMap(mFamilyMap);
   ProtocolEnabledMap enabledMap(mEnabledMap);
   ASSERT_TRUE(someInfo.ReconcileWithMaps(familyMap, enabledMap));
   enabledMap["fakeProto"] = true;
   enabledMap["newProtocol"] = false;
   familyMap["newProtocol"] = FamilyLongProtocolPair("fakeFamily", "a third fake protocol");
   ASSERT_TRUE(someInfo.ReconcileWithMaps(familyMap, enabledMap));
   ASSERT_EQ(3, someInfo.qosmosprotocol_size());
   for (int i = 0; i < someInfo.qosmosprotocol_size(); i++) {
      const protoMsg::QosmosConf::Protocol& existingProtocol = someInfo.qosmosprotocol(i);

      std::string protocolName = existingProtocol.protocolname();
      if (protocolName.find("fakeProto") != std::string::npos) {
         ASSERT_EQ(enabledMap[protocolName], existingProtocol.protocolenabled());
         ASSERT_EQ(mEnabledMap[protocolName], existingProtocol.protocolenabled());
      }

   }

}

TEST_F(QosmosConfInfoTest, PopulateSimilaressageMissingProtocol) {

   QosmosConf someInfo;
   ASSERT_EQ(0, someInfo.qosmosprotocol_size());


   ProtocolFamilyMap familyMap(mFamilyMap);
   ProtocolEnabledMap enabledMap(mEnabledMap);
   enabledMap["newProtocol"] = false;
   familyMap["newProtocol"] = FamilyLongProtocolPair("fakeFamily", "a third fake protocol");
   ASSERT_TRUE(someInfo.ReconcileWithMaps(familyMap, enabledMap));
   ASSERT_EQ(3, someInfo.qosmosprotocol_size());
   ASSERT_TRUE(someInfo.ReconcileWithMaps(mFamilyMap, mEnabledMap));
   ASSERT_EQ(2, someInfo.qosmosprotocol_size());
   for (int i = 0; i < someInfo.qosmosprotocol_size(); i++) {
      const protoMsg::QosmosConf::Protocol& existingProtocol = someInfo.qosmosprotocol(i);

      std::string protocolName = existingProtocol.protocolname();
      ASSERT_NE("newProtocol", protocolName);

   }

}

TEST_F(QosmosConfInfoTest, BadPopulate) {
   QosmosConf someInfo;
   ASSERT_EQ(0, someInfo.qosmosprotocol_size());
   mFamilyMap["foo"] = FamilyLongProtocolPair("f00Family", "another f00 protocol"); //intentionally mismatched
   ProtocolFamilyMap familyMap(mFamilyMap);
   ProtocolEnabledMap enabledMap(mEnabledMap);

   ASSERT_FALSE(someInfo.ReconcileWithMaps(familyMap, enabledMap));
   ASSERT_EQ(familyMap, mFamilyMap);
   ASSERT_EQ(enabledMap, mEnabledMap);
}

TEST_F(QosmosConfInfoTest, BadPopulate2) {
   QosmosConf someInfo;
   ASSERT_EQ(0, someInfo.qosmosprotocol_size());
   mEnabledMap["foo"] = false; //intentionally mismatched
   ProtocolFamilyMap familyMap(mFamilyMap);
   ProtocolEnabledMap enabledMap(mEnabledMap);

   ASSERT_FALSE(someInfo.ReconcileWithMaps(familyMap, enabledMap));
   ASSERT_EQ(familyMap, mFamilyMap);
   ASSERT_EQ(enabledMap, mEnabledMap);
}

TEST_F(QosmosConfInfoTest, BadPopulate3) {
   QosmosConf someInfo;
   ASSERT_EQ(0, someInfo.qosmosprotocol_size());
   mEnabledMap["foo"] = false; //intentionally mismatched
   mFamilyMap["bar"] = FamilyLongProtocolPair("f00Family", "another f00 protocol"); //intentionally mismatched
   ProtocolFamilyMap familyMap(mFamilyMap);
   ProtocolEnabledMap enabledMap(mEnabledMap);

   ASSERT_FALSE(someInfo.ReconcileWithMaps(familyMap, enabledMap));
   ASSERT_EQ(familyMap, mFamilyMap);
   ASSERT_EQ(enabledMap, mEnabledMap);
}

TEST_F(QosmosConfInfoTest, CanBeDisabledTests) {
   QosmosConf someInfo;
   ProtocolFamilyMap familyMap;
   ProtocolEnabledMap enabledMap;

   familyMap = mFamilyMap;
   enabledMap = mEnabledMap;



   familyMap["base"] = FamilyLongProtocolPair("Base", "the base Qosmos");
   enabledMap["base"] = false; // this will be rejected
   familyMap["unknown"] = FamilyLongProtocolPair("Unknown", "Qosmos is clueless");
   enabledMap["unknown"] = false; // this will be rejected
   familyMap["malformed"] = FamilyLongProtocolPair("Malformed", "garbage flow");
   enabledMap["malformed"] = false; // this will be rejected
   familyMap["incomplete"] = FamilyLongProtocolPair("Incomplete", "Missing data");
   enabledMap["incomplete"] = false; // this will be rejected
   familyMap["ip"] = FamilyLongProtocolPair("IP", "the base IP protocol");
   enabledMap["ip"] = false; // this will be rejected
   familyMap["tcp"] = FamilyLongProtocolPair("TCP", "the base TCP protocol");
   enabledMap["tcp"] = false; // this will be rejected
   familyMap["udp"] = FamilyLongProtocolPair("UDP", "the base UDP protocol");
   enabledMap["udp"] = false; // this will be rejected

   ProtocolKey key = "base";
   ASSERT_FALSE(someInfo.CanBeDisabled(key));
   key = "unknown";
   ASSERT_FALSE(someInfo.CanBeDisabled(key));
   key = "malformed";
   ASSERT_FALSE(someInfo.CanBeDisabled(key));
   key = "incomplete";
   ASSERT_FALSE(someInfo.CanBeDisabled(key));
   key = "ip";
   ASSERT_FALSE(someInfo.CanBeDisabled(key));
   key = "tcp";
   ASSERT_FALSE(someInfo.CanBeDisabled(key));
   //key = "udp";
   //ASSERT_FALSE(someInfo.CanBeDisabled(key));

   ASSERT_TRUE(someInfo.ReconcileWithMaps(familyMap, enabledMap));
   ASSERT_TRUE(someInfo.GetMaps(familyMap, enabledMap));
   
   ASSERT_TRUE(enabledMap["base"]);
   ASSERT_TRUE(enabledMap["unknown"]);
   ASSERT_TRUE(enabledMap["malformed"]);
   ASSERT_TRUE(enabledMap["incomplete"]);
   ASSERT_TRUE(enabledMap["ip"]);
   ASSERT_TRUE(enabledMap["tcp"]);
   //ASSERT_TRUE(enabledMap["udp"]);

}