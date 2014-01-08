#include "AttributeRenamerTest.h"
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

TEST_F(AttributeRenamerTest, ConstructAndInitialize) {
   AttributeRenamer renamer;
   EXPECT_FALSE(renamer.Initialize("/file/does/not/exist"));
   EXPECT_TRUE(renamer.Initialize("../conf/remapping.yaml"));
   EXPECT_TRUE(renamer.Initialize("../conf/remapping.yaml"));
   {
      std::unique_ptr<AttributeRenamer> renamerPtr(new AttributeRenamer);
      EXPECT_TRUE(renamerPtr->Initialize("../conf/remapping.yaml"));
   }
}

TEST_F(AttributeRenamerTest, BasicRenaming) {
   AttributeRenamer renamer;
   ASSERT_TRUE(renamer.Initialize("resources/remapping.yaml"));
   
   EXPECT_TRUE("logrhythm_ack_number" == renamer.GetNewName("ack_number"));
   EXPECT_TRUE("notFound" == renamer.GetNewName("notFound"));
}