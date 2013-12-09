/* 
 * File:   ToolsStringFix.cpp
 * Author: kjell
 * 
 * Created on August 2, 2013, 3:18 PM
 */

#include "ToolsTestStringFix.h"
#include "StringFix.h"
#include "FileIO.h"
#include <g2log.hpp>
#include <string>
#include <vector>

TEST_F(ToolsTestStringFix, ReplaceFailed_WithEmptyFrom) {
   std::string content{"abcdefghijklmnopqrstuvxyz"};
   std::string copy = content;
   EXPECT_EQ(0, stringfix::replace(content, "", "Hello"));
   EXPECT_EQ(copy, content);
}

TEST_F(ToolsTestStringFix, ReplaceFailedWithNoMatchingContent) {
   std::string content{""};
   EXPECT_EQ(0, stringfix::replace(content, " ", "Hello"));
   EXPECT_EQ(content, "");
}

TEST_F(ToolsTestStringFix, ReplaceIntoEmty) {
   std::string content{"aaa"};
   EXPECT_EQ(1, stringfix::replace(content, "aaa", ""));
   EXPECT_EQ(content, "");
}

TEST_F(ToolsTestStringFix, ReplaceRepeatedly) {
   std::string content{"2:525MB:299GB:299GB:::lvm;"};
   ASSERT_EQ(6, stringfix::replace(content, ":", ": "));
   EXPECT_EQ(content, "2: 525MB: 299GB: 299GB: : : lvm;");
}

TEST_F(ToolsTestStringFix, ReplaceRepeatedlyAndRecursively) {
   std::string content{"2:525MB:299GB:299GB:::lvm;"};
   ASSERT_EQ(6, stringfix::replace(content, ":", "::"));
   EXPECT_EQ(content, "2::525MB::299GB::299GB::::::lvm;");
}

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

TEST_F(ToolsTestStringFix, SplitAdvancedNoSpaceInBetween) {
   const std::string numbers{"1,,2,,3,,4,,5,,6"};
   auto tokens = stringfix::split(",", numbers);
   ASSERT_EQ(tokens.size(), 6);
   size_t count = 1;
   for(auto t : tokens) {
      ASSERT_EQ(t, std::to_string(count++));
   }
}




TEST_F(ToolsTestStringFix, SplitAndExplodeShouldNotBeSimilar) {
   const std::string numbers{"1,,2,,3,,4,,5,,6"};
   const std::vector<std::string> expected{{"1"},{""},{"2"},{""},{"3"},{""},{"4"},{""},{"5"},{""},{"6"}};
   auto tokens = stringfix::explode(",", numbers);
   ASSERT_EQ(tokens.size(), expected.size());
   size_t index = 0;
   for(auto t : tokens) {
      ASSERT_EQ(t, expected[index++]);
   }
}


TEST_F(ToolsTestStringFix, EmptyStringAsKey__ExpectingZeroExplode) {
   const std::string greeting{"Hello World! Hola El Mundo!"};
   auto tokens = stringfix::explode("", greeting);
   ASSERT_EQ(tokens.size(), 1); 
   EXPECT_EQ(tokens[0], greeting); // same returned. no match
}




TEST_F(ToolsTestStringFix, TooBigStringAsKey__ExpectingZeroExplode) {
   const std::string greeting{"Hello World! Hola El Mundo!"};
   const std::string key = greeting+greeting;
   auto tokens = stringfix::explode(key, greeting);
   ASSERT_EQ(tokens.size(), 1); 
   EXPECT_EQ(tokens[0], greeting); // same returned. no match
}



TEST_F(ToolsTestStringFix, NoMatch__ExpectingZeroExplode) {
   const std::string greeting{"Hello World! Hola El Mundo!"};
   auto tokens = stringfix::explode("Goodbye World!", greeting);
   ASSERT_EQ(tokens.size(), 1); // same returned. no match
   EXPECT_EQ(tokens[0], greeting); // same returned. no match
}

TEST_F(ToolsTestStringFix, ExplodeWithManyMatches__ExpecingAlsoEdgeSubString) {
   const std::string machine{":a:::b:"};
   const std::vector<std::string> expected{{""}, {"a"},{""},{""},{"b"}};
   auto tokens = stringfix::explode(":", machine);
   
   ASSERT_EQ(tokens.size(), expected.size());
   size_t index = 0;
   for(auto t : tokens) {
      ASSERT_EQ(t, expected[index++]);
   }
}


TEST_F(ToolsTestStringFix, ExplodeWithCompleteMatch__Expecting__FullExplodeWithEmptyReturn) {
   const std::string greeting{"Hello World! Hola El Mundo!"};
   auto tokens = stringfix::explode(greeting, greeting);
   ASSERT_EQ(tokens.size(), 1);
   EXPECT_EQ(tokens[0], "");
}

TEST_F(ToolsTestStringFix, ExplodeWithOneCharacterMatch__ExpectingTwoReturns) {
   const std::string greeting{"Hello World! Hola El Mundo!"};
   auto tokens = stringfix::explode("!", greeting);
   ASSERT_EQ(tokens.size(), 2);
   EXPECT_EQ(tokens[0], "Hello World");
   EXPECT_EQ(tokens[1], " Hola El Mundo");
}


TEST_F(ToolsTestStringFix, RealWorldExample_DAS_Matching) {
   auto partedReading = FileIO::ReadAsciiFileContent("resources/parted.119.das.txt");
   ASSERT_FALSE(partedReading.HasFailed());
   std::string parted = partedReading.result;
   LOG(DEBUG) << "read parted information: \n" << parted;

   std::string zeroItem = {""};
   std::string firstItem = {"/dev/sda:299GB:scsi:512:512:msdos:DELL PERC H710;\n1:1049kB:525MB:524MB:ext4::boot;\n2:525MB:299GB:299GB:::lvm;"};
   std::string lastItem = {"/dev/mapper/vg_probe00-lv_root:211GB:dm:512:512:loop:Linux device-mapper (linear);\n1:0.00B:211GB:211GB:ext4::;"};
   std::vector<std::string> tokens = stringfix::explode("BYT;", parted);

   ASSERT_EQ(tokens.size(), 8);
   EXPECT_EQ(tokens[0], zeroItem);
   EXPECT_EQ(stringfix::trim(tokens[1], "\n \t"), firstItem);
   EXPECT_EQ(stringfix::trim(tokens[7], "\n \t"), lastItem);
}
