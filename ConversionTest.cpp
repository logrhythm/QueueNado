#include "ConversionTest.h"
#include "include/global.h"

TEST_F(ConversionTest, convertNBytesStringsFailures) {
   Conversion converter;
   
   EXPECT_EQ(0,converter.ReadBytesFromNBytesString("nonumbers"));
   EXPECT_EQ(0,converter.ReadKBytesFromNBytesString("nonumbers"));
   EXPECT_EQ(0,converter.ReadMBytesFromNBytesString("nonumbers"));
   EXPECT_EQ(0,converter.ReadGBytesFromNBytesString("nonumbers"));
   EXPECT_EQ(0,converter.ReadBytesFromNBytesString("123NoBytes"));
   EXPECT_EQ(0,converter.ReadKBytesFromNBytesString("123NoBytes"));
   EXPECT_EQ(0,converter.ReadMBytesFromNBytesString("123NoBytes"));
   EXPECT_EQ(0,converter.ReadGBytesFromNBytesString("123NoBytes"));
}
TEST_F(ConversionTest, convertNBytesStrings) {
   Conversion converter;
   
   EXPECT_EQ(123,converter.ReadBytesFromNBytesString("123"));
   EXPECT_EQ(123<<B_TO_KB_SHIFT,converter.ReadBytesFromNBytesString("123kB"));
   EXPECT_EQ(123<<B_TO_MB_SHIFT,converter.ReadBytesFromNBytesString("123MB")); 
   EXPECT_TRUE(123L<<B_TO_GB_SHIFT==converter.ReadBytesFromNBytesString("123GB"));
   
   EXPECT_EQ(1,converter.ReadKBytesFromNBytesString("123"));
   EXPECT_EQ(0,converter.ReadKBytesFromNBytesString("0"));
   EXPECT_EQ(123,converter.ReadKBytesFromNBytesString("123kB"));
   EXPECT_TRUE(123L<<KB_TO_MB_SHIFT==converter.ReadKBytesFromNBytesString("123MB")); 
   EXPECT_TRUE(123L<<KB_TO_GB_SHIFT==converter.ReadKBytesFromNBytesString("123GB"));
   EXPECT_TRUE(123L<<KB_TO_TB_SHIFT==converter.ReadKBytesFromNBytesString("123TB"));
   
   EXPECT_EQ(1,converter.ReadMBytesFromNBytesString("123"));
   EXPECT_EQ(1,converter.ReadMBytesFromNBytesString("123kB"));
   EXPECT_EQ(0,converter.ReadMBytesFromNBytesString("0"));
   EXPECT_EQ(0,converter.ReadMBytesFromNBytesString("0kB"));
   EXPECT_EQ(123,converter.ReadMBytesFromNBytesString("123MB")); 
   EXPECT_EQ(123L<<MB_TO_GB_SHIFT,converter.ReadMBytesFromNBytesString("123GB"));
   EXPECT_EQ(123L<<MB_TO_TB_SHIFT,converter.ReadMBytesFromNBytesString("123TB"));
   
   EXPECT_EQ(1,converter.ReadGBytesFromNBytesString("123"));
   EXPECT_EQ(1,converter.ReadGBytesFromNBytesString("123kB"));
   EXPECT_EQ(1,converter.ReadGBytesFromNBytesString("123MB")); 
   EXPECT_EQ(0,converter.ReadGBytesFromNBytesString("0"));
   EXPECT_EQ(0,converter.ReadGBytesFromNBytesString("0kB"));
   EXPECT_EQ(0,converter.ReadGBytesFromNBytesString("0MB")); 
   EXPECT_EQ(123L,converter.ReadGBytesFromNBytesString("123GB"));
   EXPECT_EQ(123L<<GB_TO_TB_SHIFT,converter.ReadGBytesFromNBytesString("123TB"));
}
TEST_F(ConversionTest, convertNBytesStringsWithDecimals) {
   Conversion converter;
   
   EXPECT_EQ((12L<<B_TO_GB_SHIFT)+300L*(1L<<KB_TO_GB_SHIFT),converter.ReadBytesFromNBytesString("12.3GB"));
   
   EXPECT_EQ((12L<<KB_TO_GB_SHIFT)+300L*(1L<<MB_TO_GB_SHIFT),converter.ReadKBytesFromNBytesString("12.3GB"));
   
   EXPECT_EQ((12L<<KB_TO_TB_SHIFT)+300L*(1L<<KB_TO_GB_SHIFT),converter.ReadKBytesFromNBytesString("12.3TB"));
   
   EXPECT_EQ((12L<<MB_TO_TB_SHIFT)+300L*(1L<<MB_TO_GB_SHIFT),converter.ReadMBytesFromNBytesString("12.3TB"));
   
   EXPECT_EQ((12L<<GB_TO_TB_SHIFT)+300L*(1L),converter.ReadGBytesFromNBytesString("12.3TB"));
}
TEST_F(ConversionTest, convertArray32) {
#ifdef LR_DEBUG
   Conversion mockDpi;
   std::vector<unsigned char> array={0x1,0x2,0x4,0x8};

   EXPECT_EQ(0x08040201, mockDpi.ConvertArrayToU32(array));
#endif
}
TEST_F(ConversionTest, convertArray64One) {
#ifdef LR_DEBUG
   Conversion mockDpi;
   std::vector<unsigned char> array =
   {0x1,0x2,0x4,0x8,0x10,0x20,0x40,0x80};

   EXPECT_EQ(0x0000000000000001, mockDpi.ConvertArrayToU64(array, 1));
#endif
}

TEST_F(ConversionTest, convertArray64Zero) {
#ifdef LR_DEBUG
   Conversion mockDpi;
   std::vector<unsigned char> array =
   {0x1,0x2,0x4,0x8,0x10,0x20,0x40,0x80};

   EXPECT_EQ(0x0000000000000000, mockDpi.ConvertArrayToU64(array, 0));
#endif
}

TEST_F(ConversionTest, convertIpToString) {
#ifdef LR_DEBUG
   Conversion mockDpi;
   std::vector<unsigned char> array={0x1,0x2,0x4,0x8};

   uint32_t value = mockDpi.ConvertArrayToU32(array);
   EXPECT_EQ("1.2.4.8", mockDpi.ConvertIpValToString(value));

#endif
}

TEST_F(ConversionTest, convertEtherToString) {
#ifdef LR_DEBUG
   Conversion mockDpi;
   std::vector<unsigned char> array =
   {0x1,0x2,0x4,0x8,0x10,0x20,0x40,0x80};

   uint64_t value = mockDpi.ConvertArrayToU64(array, 6);
   EXPECT_EQ("01:02:04:08:10:20", mockDpi.ConvertEtherValToString(value));
#endif
}

TEST_F(ConversionTest, ReadShortFromString) {
#ifdef LR_DEBUG
   char charbuffer[100];
   memset(charbuffer, 0, 100);
   charbuffer[5] = '9';
   charbuffer[6] = '0';
   charbuffer[7] = 'a';
   charbuffer[8] = 'f';

   size_t index = 5;
   Conversion dm;
   EXPECT_EQ(0x90af, dm.ReadShortFromString(charbuffer, index));
   EXPECT_EQ(9, index);
#endif
}



TEST_F(ConversionTest, ReadUintFromString) {
   char charbuffer[100];
   memset(charbuffer, 0, 100);
   charbuffer[5] = '9';
   charbuffer[6] = '0';
   charbuffer[7] = 'a';
   charbuffer[8] = 'f';
   charbuffer[9] = '2';
   charbuffer[10] = '3';
   charbuffer[11] = 'b';
   charbuffer[12] = 'c';

   size_t index = 5;
#ifdef LR_DEBUG
   Conversion dm;
   EXPECT_EQ(0x90af23bc, dm.ReadUIntFromString(charbuffer, index));
   EXPECT_EQ(13, index);
#endif
}

TEST_F(ConversionTest, convertArray64) {
#ifdef LR_DEBUG
   Conversion mockDpi;
   std::vector<unsigned char> array =
   {0x1,0x2,0x4,0x8,0x10,0x20,0x40,0x80};

   EXPECT_EQ(0x8040201008040201, mockDpi.ConvertArrayToU64(array, 8));
#endif
}
