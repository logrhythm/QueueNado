/* 
 * Author: Robert Weber
 *
 * Created on November 19, 2012, 11:58 AM
 */
#pragma once

#include "gtest/gtest.h"
#include "QosmosConf.h"

class QosmosConfInfoTest : public ::testing::Test {
public:

   QosmosConfInfoTest() {
   };
protected:

   virtual void SetUp() {
      std::string fakeProto = "fakeProto";

      mFamilyMap[fakeProto] = FamilyLongProtocolPair("fakeFamily", "a fake protocol");
      mEnabledMap[fakeProto] = false;
      fakeProto = "booProto2";

      mFamilyMap[fakeProto] = FamilyLongProtocolPair("fakeFamily", "another fake protocol");
      mEnabledMap[fakeProto] = true;
   };

   virtual void TearDown() {
   };

   ProtocolFamilyMap mFamilyMap;
   ProtocolEnabledMap mEnabledMap;
};

