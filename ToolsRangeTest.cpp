/* 
 * File:   ToolsRangeTest.cpp
 * Author: kjell
 *
 * Created on August 15, 2013, 3:41 PM
 */

#include "gtest/gtest.h"
#include "Range.h"


TEST(TestRange, RangeConstructionOK) {
   EXPECT_TRUE(!(Range{-2, -1}).error);
   EXPECT_TRUE(!(Range{-1, 1}).error);
   EXPECT_TRUE(!(Range{-1,0}).error);
   EXPECT_TRUE(!(Range{0,0}).error);
   EXPECT_TRUE(!(Range{1,1}).error);
}

TEST(TestRange, RangeConstructionNotOK) {
   EXPECT_TRUE((Range{-1, -2}).error);
   EXPECT_TRUE((Range{1, 0}).error);
   EXPECT_TRUE((Range{2,-1}).error);
}

TEST(TestRange, RangeConstructionNotOKAndNeitherWillRangeCheckBe) {
   EXPECT_FALSE((Range{-1, -2}).WithinRange(-2));
   EXPECT_FALSE((Range{1, -1}).WithinRange(0));
   EXPECT_FALSE((Range{4, 1}).WithinRange(2));
}

TEST(TestRange, WithinRange) {
   EXPECT_TRUE((Range{-2, -1}).WithinRange(-1));
   EXPECT_TRUE((Range{-2, -1}).WithinRange(-2));
   EXPECT_TRUE((Range{-2, 1}).WithinRange(0));
   EXPECT_TRUE((Range{0, 1}).WithinRange(0));
   EXPECT_TRUE((Range{-1, 2}).WithinRange(0));
}

TEST(TestRange, OutsideRange) {
   EXPECT_FALSE((Range{-2, -1}).WithinRange(0));
   EXPECT_FALSE((Range{-2, -1}).WithinRange(-3));
   EXPECT_FALSE((Range{-2, 1}).WithinRange(2));
   EXPECT_FALSE((Range{0, 1}).WithinRange(-1));
   EXPECT_FALSE((Range{-1, 2}).WithinRange(3));
}
