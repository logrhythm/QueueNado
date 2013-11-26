/* 
 * File:   ToolsRangeTest.cpp
 * Author: Ryan Kophs
 *
 * Created on August 15, 2013, 3:41 PM
 */

#include "gtest/gtest.h"
#include "Range.h"

TEST(TestRange, CheckNumberForNegative) {
   RangeInt range(1, 2);
   EXPECT_NO_THROW(range.CheckNumberForNegative("1"));
   EXPECT_NO_THROW(range.CheckNumberForNegative("0"));
   EXPECT_ANY_THROW(range.CheckNumberForNegative("-1"));

   RangeDouble range2(1, 2);
   EXPECT_NO_THROW(range2.CheckNumberForNegative("1"));
   EXPECT_NO_THROW(range2.CheckNumberForNegative("0"));
   EXPECT_ANY_THROW(range2.CheckNumberForNegative("-1"));
}

TEST(TestRange, RangeIntConstructionOK) {
   EXPECT_TRUE(!(RangeInt{-2, -1}).error);
   EXPECT_TRUE(!(RangeInt{-2, -2}).error);
   EXPECT_TRUE((RangeInt{-1, -2}).error);
   EXPECT_TRUE(!(RangeInt{0, 0}).error);
   EXPECT_TRUE(!(RangeInt{1, 2}).error);
   EXPECT_TRUE((RangeInt{2, 1}).error);
   EXPECT_TRUE(!(RangeInt{1, 1}).error);
}

TEST(TestRange, RangeIntValidateOK) {
   RangeInt r1(2, 0); //Should set error flag;
   EXPECT_TRUE(!(r1.Validate("-1")));
   EXPECT_TRUE(!(r1.Validate("0")));
   EXPECT_TRUE(!(r1.Validate("1")));
   EXPECT_TRUE(!(r1.Validate("2")));
   EXPECT_TRUE(!(r1.Validate("3")));

   RangeInt r2(0, 2);
   EXPECT_TRUE(!(r2.Validate("-1")));
   EXPECT_TRUE(r2.Validate("0"));
   EXPECT_TRUE(r2.Validate("1"));
   EXPECT_TRUE(r2.Validate("2"));
   EXPECT_TRUE(!(r2.Validate("3")));

   RangeInt r3(0, 0);
   EXPECT_TRUE(!(r3.Validate("-1")));
   EXPECT_TRUE(r3.Validate("0"));
   EXPECT_TRUE(!(r3.Validate("1")));
}

TEST(TestRange, RangeDoubleConstructorOK) {
   EXPECT_TRUE(!(RangeDouble{-2.0, -1.0}).error);
   EXPECT_TRUE(!(RangeDouble{-2.0, -2.0}).error);
   EXPECT_TRUE((RangeDouble{-1.0, -2.0}).error); //should cause error
   EXPECT_TRUE(!(RangeDouble{0.0, 0.0}).error);
   EXPECT_TRUE(!(RangeDouble{1.0, 2.0}).error);
   EXPECT_TRUE((RangeDouble{2.0, 1.0}).error);  //should cause error
   EXPECT_TRUE(!(RangeDouble{1.0, 1.0}).error);

   EXPECT_TRUE(!(RangeDouble{-2, -1}).error);
   EXPECT_TRUE(!(RangeDouble{-2, -2}).error);
   EXPECT_TRUE((RangeDouble{-1, -2}).error); //should cause error
   EXPECT_TRUE(!(RangeDouble{0, 0}).error);
   EXPECT_TRUE(!(RangeDouble{1, 2}).error);
   EXPECT_TRUE((RangeDouble{2, 1}).error);   //Should cause error
   EXPECT_TRUE(!(RangeDouble{1, 1}).error);
}

TEST(TestRange, RangeDoubleValidateOK) {
   RangeDouble r1(2.0, 0.0); //Should set error flag;
   EXPECT_TRUE(!(r1.Validate("-1.0")));
   EXPECT_TRUE(!(r1.Validate("0.0")));
   EXPECT_TRUE(!(r1.Validate("1.0")));
   EXPECT_TRUE(!(r1.Validate("2.0")));
   EXPECT_TRUE(!(r1.Validate("3.0")));

   RangeDouble r2(0.0, 2.0);
   EXPECT_TRUE(!(r2.Validate("-1.0")));
   EXPECT_TRUE(r2.Validate("0.0"));
   EXPECT_TRUE(r2.Validate("1.0"));
   EXPECT_TRUE(r2.Validate("2.0"));
   EXPECT_TRUE(!(r2.Validate("3.0")));

   RangeDouble r3(0.0, 0.0);
   EXPECT_TRUE(!(r3.Validate("-1.0")));
   EXPECT_TRUE(r3.Validate("0.0"));
   EXPECT_TRUE(!(r3.Validate("1.0")));

   RangeDouble r4(0.5, 1.5);
   EXPECT_TRUE(!(r4.Validate("0.0")));
   EXPECT_TRUE(!(r4.Validate("0.49999999")));
   EXPECT_TRUE(r4.Validate("0.5"));
   EXPECT_TRUE(r4.Validate("1.0"));
   EXPECT_TRUE(r4.Validate("1.5"));
   EXPECT_TRUE(!(r4.Validate("1.50000001")));
   EXPECT_TRUE(!(r4.Validate("2.0")));
}

TEST(TestRange, RangeBoolConstructorOK) {
   EXPECT_TRUE(!(RangeBool{}).error);
   EXPECT_TRUE((RangeBool{}).False.compare("false") == 0);
   EXPECT_TRUE((RangeBool{}).True.compare("true") == 0);
}

TEST(TestRange, RangeBoolValidateOK){
   EXPECT_TRUE((RangeBool{}).Validate("false"));
   EXPECT_TRUE((RangeBool{}).Validate("False"));
   EXPECT_TRUE((RangeBool{}).Validate("fAlse"));
   EXPECT_TRUE((RangeBool{}).Validate("FALSE"));
   EXPECT_TRUE(!(RangeBool{}).Validate("falsed")); //Shouldn't be valid
   EXPECT_TRUE((RangeBool{}).Validate("true"));
   EXPECT_TRUE((RangeBool{}).Validate("TRUE"));
   EXPECT_TRUE((RangeBool{}).Validate("True"));
   EXPECT_TRUE(!(RangeBool{}).Validate("truthy")); //Shouldn't be valid
   EXPECT_TRUE(!(RangeBool{}).Validate(" trutHY")); //Shouldn't be valid
   EXPECT_TRUE(!(RangeBool{}).Validate("atrUThy")); //Shouldn't be valid
   EXPECT_TRUE(!(RangeBool{}).Validate("atruTh")); //Shouldn't be valid
}

TEST(TestRange, RangeStringConstructorOK){
   EXPECT_TRUE((RangeString{-2,-1}).error);
   EXPECT_TRUE((RangeString{-1,-2}).error);
   EXPECT_TRUE((RangeString{-2,-2}).error);
   EXPECT_TRUE((RangeString{-1,-1}).error);
   EXPECT_TRUE((RangeString{-1,0}).error);
   EXPECT_TRUE(!(RangeString{0,-1}).error); //-1 indicates INFINITY on upper bound
   EXPECT_TRUE(!(RangeString{0,0}).error); //empty string
   EXPECT_TRUE((RangeString{1,0}).error);
   EXPECT_TRUE(!(RangeString{0,1}).error);
   EXPECT_TRUE(!(RangeString{1,1}).error);
   EXPECT_TRUE((RangeString{2,1}).error);
   EXPECT_TRUE(!(RangeString{1,2}).error);
   EXPECT_TRUE(!(RangeString{2,-1}).error);
   
   const int max = (RangeString{0, -1}).maxStrSize;
   
   EXPECT_TRUE(max == (RangeString{0, -1}).higher);
   EXPECT_TRUE(max == (RangeString{0, max}).higher);
   EXPECT_TRUE(max == (RangeString{0, max+1}).higher);
   EXPECT_TRUE(max > (RangeString{0, max-1}).higher);
}

TEST(TestRange, RangeStringValidateOK){
   EXPECT_TRUE(!(RangeString{-2, -1}).Validate("test string"));      //Error set
   EXPECT_TRUE(!(RangeString{-2, -1}).Validate(""));                 //Error set
   
   const int max = (RangeString{0, -1}).maxStrSize;
   std::string test;
   for(int i = 0; i < max; i++){
      test.append("A");
   }
   EXPECT_TRUE((RangeString{0, -1}).Validate(""));
   EXPECT_TRUE((RangeString{0, -1}).Validate("test string"));
   EXPECT_TRUE((RangeString{0, -1}).Validate(test));
   test.append("A");
   EXPECT_TRUE(!(RangeString{0, -1}).Validate(test));
   test.append("A");
   EXPECT_TRUE(!(RangeString{0, -1}).Validate(test));
   
   EXPECT_TRUE((RangeString{5,5}).Validate("Hello"));
   EXPECT_TRUE((RangeString{4,5}).Validate("Hello"));
   EXPECT_TRUE((RangeString{5,6}).Validate("Hello"));
   EXPECT_TRUE(!(RangeString{4,4}).Validate("Hello"));
   EXPECT_TRUE(!(RangeString{0,4}).Validate("Hello"));
   EXPECT_TRUE(!(RangeString{6,10}).Validate("Hello"));
   EXPECT_TRUE((RangeString{0,4}).Validate(""));
   EXPECT_TRUE((RangeString{0,4}).Validate("Hi"));
   EXPECT_TRUE((RangeString{0,4}).Validate("High"));
}