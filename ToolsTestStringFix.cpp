/* 
 * File:   ToolsStringFix.cpp
 * Author: kjell
 * 
 * Created on August 2, 2013, 3:18 PM
 */

#include "ToolsTestStringFix.h"
#include "StringFix.h"

TEST_F(ToolsTestStringFix, NoTrim) {
   const std::string noSpaceAtEnd{"abcde efgh"};
   auto notTrimmed = stringfix::trim(noSpaceAtEnd);
   ASSERT_EQ(notTrimmed, noSpaceAtEnd);
}

TEST_F(ToolsTestStringFix, TrimEnds) {
   const std::string spaceAtEnd{"\n\t abcde efgh\n\t "};
   const std::string noSpaceAtEnd{"abcde efgh"};
   auto trimmed = stringfix::trim(noSpaceAtEnd, {"\n\t "});
   
   ASSERT_NE(trimmed, spaceAtEnd);
   ASSERT_EQ(trimmed, noSpaceAtEnd);
}


TEST_F(ToolsTestStringFix, NoSplit) {
   const std::string numbers{"1,2,3,4,5,6"};
   auto tokens = stringfix::split("", numbers);
   ASSERT_EQ(tokens.size(), 1);
   ASSERT_EQ(tokens[0], numbers);
}

TEST_F(ToolsTestStringFix, Split) {
   const std::string numbers{"1,2,3,4,5,6"};
   auto tokens = stringfix::split(",", numbers);
   ASSERT_EQ(tokens.size(), 6);
   size_t count = 1;
   for(auto t : tokens) {
      ASSERT_EQ(t, std::to_string(count++));
   }
}

TEST_F(ToolsTestStringFix, SplitAdvanced) {
   const std::string numbers{"1 2,3 4,5 6"};
   auto tokens = stringfix::split(", ", numbers);
   ASSERT_EQ(tokens.size(), 6);
   size_t count = 1;
   for(auto t : tokens) {
      ASSERT_EQ(t, std::to_string(count++));
   }
}

TEST_F(ToolsTestStringFix, ToLower){
   const std::string lower("test");
   EXPECT_TRUE(lower.compare(stringfix::to_lower("test")) == 0);
   EXPECT_TRUE(lower.compare(stringfix::to_lower("TEST")) == 0);
   EXPECT_TRUE(lower.compare(stringfix::to_lower("Test")) == 0);
   EXPECT_TRUE(lower.compare(stringfix::to_lower("teST")) == 0);
   EXPECT_TRUE(lower.compare(stringfix::to_lower("tEst")) == 0);
   EXPECT_TRUE(!lower.compare(stringfix::to_lower("tests")) == 0);
   EXPECT_TRUE(!lower.compare(stringfix::to_lower("Tests")) == 0);
   EXPECT_TRUE(!lower.compare(stringfix::to_lower("aTest")) == 0);
}

TEST_F(ToolsTestStringFix, ToUpper){
   const std::string upper("TEST");
   EXPECT_TRUE(upper.compare(stringfix::to_upper("test")) == 0);
   EXPECT_TRUE(upper.compare(stringfix::to_upper("TEST")) == 0);
   EXPECT_TRUE(upper.compare(stringfix::to_upper("Test")) == 0);
   EXPECT_TRUE(upper.compare(stringfix::to_upper("teST")) == 0);
   EXPECT_TRUE(upper.compare(stringfix::to_upper("tEst")) == 0);
   EXPECT_TRUE(!upper.compare(stringfix::to_upper("tests")) == 0);
   EXPECT_TRUE(!upper.compare(stringfix::to_upper("Tests")) == 0);
   EXPECT_TRUE(!upper.compare(stringfix::to_upper("aTest")) == 0);
}

TEST_F(ToolsTestStringFix, ToLowerNums){
   const std::string upper("123test456");
   EXPECT_TRUE(upper.compare(stringfix::to_lower("123test456")) == 0);
   EXPECT_TRUE(upper.compare(stringfix::to_lower("123TEST456")) == 0);
   EXPECT_TRUE(upper.compare(stringfix::to_lower("123teSt456")) == 0);
   EXPECT_TRUE(!upper.compare(stringfix::to_lower("123te3St456")) == 0);
   EXPECT_TRUE(!upper.compare(stringfix::to_lower("test")) == 0);
}

TEST_F(ToolsTestStringFix, ToLowerSpecialChars){
   const std::string upper("<>123te!st45&6*");
   EXPECT_TRUE(upper.compare(stringfix::to_lower("<>123te!st45&6*")) == 0);
   EXPECT_TRUE(upper.compare(stringfix::to_lower("<>123TE!ST45&6*")) == 0);
   EXPECT_TRUE(upper.compare(stringfix::to_lower("<>123te!St45&6*")) == 0);
   EXPECT_TRUE(!upper.compare(stringfix::to_lower("123teSt456")) == 0);
}

TEST_F(ToolsTestStringFix, ToUpperNums){
   const std::string upper("123TEST456");
   EXPECT_TRUE(upper.compare(stringfix::to_upper("123TEST456")) == 0);
   EXPECT_TRUE(upper.compare(stringfix::to_upper("123test456")) == 0);
   EXPECT_TRUE(upper.compare(stringfix::to_upper("123teSt456")) == 0);
   EXPECT_TRUE(!upper.compare(stringfix::to_upper("123te3St456")) == 0);
   EXPECT_TRUE(!upper.compare(stringfix::to_upper("test")) == 0);
}

TEST_F(ToolsTestStringFix, ToUpperSpecialChars){
   const std::string upper("<>123TE!ST45&6*");
   EXPECT_TRUE(upper.compare(stringfix::to_upper("<>123te!st45&6*")) == 0);
   EXPECT_TRUE(upper.compare(stringfix::to_upper("<>123TE!ST45&6*")) == 0);
   EXPECT_TRUE(upper.compare(stringfix::to_upper("<>123te!St45&6*")) == 0);
   EXPECT_TRUE(!upper.compare(stringfix::to_upper("123teSt456")) == 0);
}

TEST_F(ToolsTestStringFix, EmptyString){
   const std::string empty("");
   EXPECT_TRUE(empty.compare(stringfix::to_upper("")) == 0);
   EXPECT_TRUE(empty.compare(stringfix::to_upper("t")) != 0);
   EXPECT_TRUE(empty.compare(stringfix::to_lower("")) == 0);
   EXPECT_TRUE(empty.compare(stringfix::to_lower("t")) != 0);
}

TEST_F(ToolsTestStringFix, NumbersOnlyString){
   const std::string num("456");
   EXPECT_TRUE(num.compare(stringfix::to_upper("456")) == 0);
   EXPECT_TRUE(num.compare(stringfix::to_upper("456t")) != 0);
   EXPECT_TRUE(num.compare(stringfix::to_upper("")) != 0);
   EXPECT_TRUE(num.compare(stringfix::to_lower("456")) == 0);
   EXPECT_TRUE(num.compare(stringfix::to_lower("456t")) != 0);
   EXPECT_TRUE(num.compare(stringfix::to_lower("")) != 0);
}