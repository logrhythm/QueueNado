#include <fstream>

#include "QosmosPacketAllocatorTests.h"
using namespace std;

TEST_F(QsomosPacketAllocatorTests, Constructors) {
   QosmosPacketAllocator & testAllocator(QosmosPacketAllocator::Instance());

}


//TEST_F ( QsomosPacketAllocatorTests, ReturnPacketData_PopulatePacketData) {
//	string aString = "ABCDEF";
//	size_t stringSize = aString.size();
//	unsigned char* dataPointer(NULL);
//    ASSERT_EQ(0,t_allocator.NumberOfKnownHashes());
//	ASSERT_EQ(0,t_allocator.SizeOfgivenHashData((uLong)dataPointer));
//
//	t_allocator.PopulatePacketData(aString,dataPointer);
//	ASSERT_TRUE(dataPointer != NULL);
//    EXPECT_EQ(1,t_allocator.NumberOfKnownHashes());
//
//    t_allocator.ReturnPacketData(dataPointer,stringSize);
//    ASSERT_EQ(0,t_allocator.NumberOfKnownHashes());
//    ASSERT_EQ(1,t_allocator.NumberOfFreeHashes());
//    ASSERT_EQ(0,t_allocator.SizeOfgivenHashData((uLong)dataPointer));
//
//    //verify that memory is reused
//
//    aString = "abcdef";
//	t_allocator.PopulatePacketData(aString,dataPointer);
//	ASSERT_TRUE(dataPointer != NULL);
//    EXPECT_EQ(1,t_allocator.NumberOfKnownHashes());
//    ASSERT_EQ(0,t_allocator.NumberOfFreeHashes());
//    aString = "123456";
//
//    t_allocator.PopulatePacketData(aString,dataPointer);
//    ASSERT_TRUE(dataPointer != NULL);
//    EXPECT_EQ(2,t_allocator.NumberOfKnownHashes());
//
//    t_allocator.ReturnPacketData(dataPointer,stringSize);
//    ASSERT_EQ(1,t_allocator.NumberOfKnownHashes());
//    ASSERT_EQ(1,t_allocator.NumberOfFreeHashes());
//}

TEST_F(QsomosPacketAllocatorTests, Liars) {
   string aString = "ABCDEF";
   size_t stringSize = aString.size();
   ctb_ppacket dataPointer(NULL);
   ASSERT_EQ(0, t_allocator.NumberOfKnownHashes());

   t_allocator.ReturnPacketData(dataPointer);
   ASSERT_EQ(0, t_allocator.NumberOfKnownHashes());
   ASSERT_EQ(0, t_allocator.SizeOfgivenHashData(0));

}

TEST_F(QsomosPacketAllocatorTests, FailedMalloc) {
#ifdef LR_DEBUG
   ctb_ppacket packet(reinterpret_cast<ctb_pkt*> (1));
   uint8_t * rawPacket(NULL);
   struct pcap_pkthdr * phdr(NULL);
   EXPECT_EQ(0, t_allocator.PopulatePacketData(rawPacket, phdr, packet));
   EXPECT_EQ(NULL, packet);
   rawPacket = new uint8_t[123];
   EXPECT_EQ(0, t_allocator.PopulatePacketData(rawPacket, phdr, packet));
   EXPECT_EQ(NULL, packet);
   phdr = new pcap_pkthdr;
   phdr->len = 123;
   t_allocator.mMallocpacketFail = true;
   EXPECT_EQ(0, t_allocator.PopulatePacketData(rawPacket, phdr, packet));
   EXPECT_EQ(NULL, packet);
   delete [] rawPacket;
   delete phdr;
#endif
}

TEST_F(QsomosPacketAllocatorTests, TwoPacketConversions) {
   std::fstream inputFile;
   std::fstream outputFile;
   inputFile.open("resources/1000packets.tcpdump", fstream::in | fstream::binary);

   unsigned char* buffer = new unsigned char[65536];
   inputFile.read(reinterpret_cast<char*>(buffer), sizeof (pcap_file_header));
   if (!inputFile && inputFile.gcount() < sizeof (struct pcap_file_header)) {
      inputFile.close();
      delete [] buffer;
      ASSERT_TRUE(false);
   }
   struct pcap_file_header* fileHeader = reinterpret_cast<struct pcap_file_header*>(buffer);
   outputFile.open("resources/1000packets.tcpdump.copy",fstream::out | fstream::binary | fstream::trunc);
   outputFile.write(reinterpret_cast<char*>(buffer),sizeof(struct pcap_file_header));
   ctb_ppacket dataPointer(NULL);
   while (!inputFile.eof()) {
      struct pcap_pkthdr* header = static_cast<struct pcap_pkthdr*>(malloc(sizeof(struct pcap_pkthdr)));
      inputFile.read(reinterpret_cast<char*>(buffer), sizeof (struct pcap_pkthdr));
      if (!inputFile && inputFile.gcount() < sizeof (struct pcap_pkthdr)) {
         break;
      }
      memcpy(&header->ts.tv_sec,buffer,sizeof(header->ts.tv_sec));
      memcpy(&header->ts.tv_usec,buffer+sizeof(header->ts.tv_sec),sizeof(header->ts.tv_usec));
      header->caplen = buffer[sizeof(header->ts.tv_sec)+sizeof(header->ts.tv_sec)];
      header->len = header->caplen;
      size_t length = header->caplen;
      ASSERT_TRUE(length<=65536);
      inputFile.read(reinterpret_cast<char*>(buffer), length);
      if (!inputFile && inputFile.gcount() < length) {
         break;
      }
      unsigned char* rawData = buffer;
      
      t_allocator.PopulatePacketData(rawData, header, dataPointer);
      struct pcap_pkthdr* header2(NULL);
      unsigned char* raw2(NULL);
      ASSERT_TRUE(t_allocator.PopulatrePCapData(dataPointer, header2, raw2));
      outputFile.write(reinterpret_cast<char*>(header2),sizeof(struct pcap_file_header));
      outputFile.write(reinterpret_cast<char*>(raw2),header2->len);
      free(header2);
      free(raw2);
      free(dataPointer->data);
      free(dataPointer);
      free(header);
   }
   outputFile.close();
   inputFile.close();
   delete [] buffer;
}

TEST_F(QsomosPacketAllocatorTests, PacketTranslation) {
   std::string* packetAsString;
   ctb_ppacket testPacket = NULL;
   t_allocator.PopulateStringFromPacket(testPacket, packetAsString);
   std::string testPacketString;
   t_allocator.PopulatePacketFromString(testPacketString, testPacket);
   EXPECT_TRUE(testPacket == NULL);
   testPacket = reinterpret_cast<ctb_ppacket> (malloc(sizeof (ctb_pkt)));
   testPacket->timestamp.tv_sec = 12;
   testPacket->timestamp.tv_usec = 45;
   testPacket->data = reinterpret_cast<unsigned char*> (malloc(sizeof (unsigned char) *100));
   testPacket->len = 100;

   EXPECT_TRUE(packetAsString == NULL);
   t_allocator.PopulateStringFromPacket(testPacket, packetAsString);
   EXPECT_TRUE(packetAsString != NULL);
   EXPECT_TRUE(testPacket == NULL);
   EXPECT_EQ(100 + sizeof (timeval) + sizeof (unsigned int), packetAsString->size());
   t_allocator.PopulatePacketFromString(*packetAsString, testPacket);
   delete packetAsString;
   EXPECT_TRUE(testPacket != NULL);
   EXPECT_EQ(12, testPacket->timestamp.tv_sec);
   EXPECT_EQ(45, testPacket->timestamp.tv_usec);
   EXPECT_EQ(100, testPacket->len);
   free(testPacket->data);
   free(testPacket);


}