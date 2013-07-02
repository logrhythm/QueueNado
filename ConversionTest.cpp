#include "ConversionTest.h"


TEST_F(ConversionTest, convertArray32) {
#ifdef LR_DEBUG
   Conversion mockDpi;
   std::vector<unsigned char> array;
   array.push_back(0x1);
   array.push_back(0x2);
   array.push_back(0x4);
   array.push_back(0x8);
   EXPECT_EQ(0x08040201, mockDpi.ConvertArrayToU32(array));
#endif
}
TEST_F(ConversionTest, convertArray64One) {
#ifdef LR_DEBUG
   Conversion mockDpi;
   std::vector<unsigned char> array;
   array.push_back(0x1);
   array.push_back(0x2);
   array.push_back(0x4);
   array.push_back(0x8);
   array.push_back(0x10);
   array.push_back(0x20);
   array.push_back(0x40);
   array.push_back(0x80);

   EXPECT_EQ(0x0000000000000001, mockDpi.ConvertArrayToU64(array, 1));
#endif
}

TEST_F(ConversionTest, convertArray64Zero) {
#ifdef LR_DEBUG
   Conversion mockDpi;
   std::vector<unsigned char> array;
   array.push_back(0x1);
   array.push_back(0x2);
   array.push_back(0x4);
   array.push_back(0x8);
   array.push_back(0x10);
   array.push_back(0x20);
   array.push_back(0x40);
   array.push_back(0x80);

   EXPECT_EQ(0x0000000000000000, mockDpi.ConvertArrayToU64(array, 0));
#endif
}

TEST_F(ConversionTest, convertIpToString) {
#ifdef LR_DEBUG
   Conversion mockDpi;
   std::vector<unsigned char> array;
   array.push_back(0x1);
   array.push_back(0x2);
   array.push_back(0x4);
   array.push_back(0x8);

   uint32_t value = mockDpi.ConvertArrayToU32(array);
   EXPECT_EQ("1.2.4.8", mockDpi.ConvertIpValToString(value));

#endif
}

TEST_F(ConversionTest, convertEtherToString) {
#ifdef LR_DEBUG
   Conversion mockDpi;
   std::vector<unsigned char> array;
   array.push_back(0x1);
   array.push_back(0x2);
   array.push_back(0x4);
   array.push_back(0x8);
   array.push_back(0x10);
   array.push_back(0x20);
   array.push_back(0x40);
   array.push_back(0x80);

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
   std::vector<unsigned char> array;
   array.push_back(0x1);
   array.push_back(0x2);
   array.push_back(0x4);
   array.push_back(0x8);
   array.push_back(0x10);
   array.push_back(0x20);
   array.push_back(0x40);
   array.push_back(0x80);

   EXPECT_EQ(0x8040201008040201, mockDpi.ConvertArrayToU64(array, 8));
#endif
}
