#include "AttributeRenamerTest.h"
#include "MockAttributeRenamer.h"
#include <memory>

namespace {
   bool StringContains(const std::string& input, const std::string& pattern) {
      if (input.find(pattern) != std::string::npos) {
         return true;
      } else {
         std::cout << input << " does not contain " << pattern << std::endl;
      }
      return false;
   }
}
#ifdef LR_DEBUG
TEST_F(AttributeRenamerTest, ConstructAndInitialize) {
   MockAttributeRenamer renamer;
   EXPECT_FALSE(renamer.Initialize("/file/does/not/exist"));
   EXPECT_TRUE(renamer.Initialize("../conf/remapping.yaml"));
   EXPECT_TRUE(renamer.Initialize("../conf/remapping.yaml"));
   {
      std::unique_ptr<MockAttributeRenamer> renamerPtr(new MockAttributeRenamer);
      EXPECT_TRUE(renamerPtr->Initialize("../conf/remapping.yaml"));
   }
}

TEST_F(AttributeRenamerTest, BasicRenaming) {
   MockAttributeRenamer renamer;
   ASSERT_TRUE(renamer.Initialize("resources/remapping.yaml"));
   
   EXPECT_TRUE("logrhythm_ack_number" == renamer.GetNewName("ack_number","#"));
   EXPECT_TRUE("#notFound" == renamer.GetNewName("notFound","#"));
}
TEST_F(AttributeRenamerTest, ResetRenaming) {
   MockAttributeRenamer renamer;
   ASSERT_TRUE(renamer.Initialize("resources/remapping.yaml"));
   
   EXPECT_TRUE("logrhythm_ack_number" == renamer.GetNewName("ack_number","#"));
   EXPECT_TRUE("#notFound" == renamer.GetNewName("notFound","#"));
   ASSERT_FALSE(renamer.Initialize("/file/does/not/exist"));
   EXPECT_TRUE("#ack_number" == renamer.GetNewName("ack_number","#"));
   EXPECT_TRUE("#notFound" == renamer.GetNewName("notFound","#"));
}
TEST_F(AttributeRenamerTest, EmptyFields) {
   MockAttributeRenamer attributeRenamer;
   ASSERT_TRUE(attributeRenamer.Initialize("resources/remapping-empty.yaml"));
   EXPECT_TRUE("#ack_number" == attributeRenamer.GetNewName("ack_number","#"));
   EXPECT_TRUE("#VTP_mgmt_domain" == attributeRenamer.GetNewName("VTP_mgmt_domain","#"));
   
}

TEST_F(AttributeRenamerTest, CorruptYaml) {
   MockAttributeRenamer attributeRenamer;
   ASSERT_FALSE(attributeRenamer.Initialize("resources/remapping-corrupt.yaml"));
}
#endif
TEST_F(AttributeRenamerTest, Singleton) {
   AttributeRenamer& attributeRenamer = AttributeRenamer::Instance();
   AttributeRenamer& attributeRenamer2 = AttributeRenamer::Instance();
   ASSERT_EQ(&attributeRenamer,&attributeRenamer2);
   ASSERT_TRUE(attributeRenamer.Initialize("../conf/remapping.yaml"));
   EXPECT_FALSE(attributeRenamer.Initialize("../conf/remapping.yaml"));
   EXPECT_FALSE(attributeRenamer2.Initialize("../conf/remapping.yaml"));
}
