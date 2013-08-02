

#include "ToolsMakeUnique.h"
#include "MakeUniquePtr.h"

TEST_F(ToolsTest, MakeUniqueSimpleStruct) {
   bool alive = false;
   auto simplePtr = std2::make_unique<Simple>(alive);
   ASSERT_TRUE(alive);
   simplePtr.reset();
   ASSERT_FALSE(alive);
}

TEST_F(ToolsTest, ScopedMakeUniqueSimpleStruct) {
   bool alive = false;
   {
      auto simplePtr = std2::make_unique<Simple>(alive);
      ASSERT_TRUE(alive);
   }
   ASSERT_FALSE(alive);
}

TEST_F(ToolsTest, ArrayOfIntMakeUnique_ExpectingCompile) {
   auto simpleArrayPtr = std2::make_unique<int[]>(1, 2, 3);
   ASSERT_EQ(simpleArrayPtr[0], 1);
   ASSERT_EQ(simpleArrayPtr[1], 2);
   ASSERT_EQ(simpleArrayPtr[2], 3);
}

TEST_F(ToolsTest, ArrayMakeUnique) {
   bool alive1 = false;
   bool alive2 = false;
   {
      auto simpleArrayPtr = std2::make_unique<Simple[]>(alive1, alive2);
      ASSERT_TRUE(alive1);
      ASSERT_TRUE(alive2);
   }
   ASSERT_FALSE(alive1);
   ASSERT_FALSE(alive2);
}