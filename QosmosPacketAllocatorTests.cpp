#include "QosmosPacketAllocatorTests.h"
using namespace std;
#include <fstream>

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
   t_allocator.PopulatePacketData(rawPacket, phdr, packet);
   EXPECT_EQ(NULL, packet);
   rawPacket = new uint8_t[123];
   t_allocator.PopulatePacketData(rawPacket, phdr, packet);
   EXPECT_EQ(NULL, packet);
   phdr = new pcap_pkthdr;
   phdr->len = 123;
   phdr->caplen = 123;
   t_allocator.mMallocpacketFail = true;
   t_allocator.PopulatePacketData(rawPacket, phdr, packet);
   EXPECT_EQ(NULL, packet);
   delete [] rawPacket;
   delete phdr;
#endif
}

TEST_F(QsomosPacketAllocatorTests, TwoPacketConversions) {
   if (geteuid() == 0) {
      std::fstream inputFile;
      std::fstream outputFile;
      inputFile.open("resources/1000packets.tcpdump", fstream::in | fstream::binary);

      unsigned char* buffer = new unsigned char[65536];
      inputFile.read(reinterpret_cast<char*> (buffer), sizeof (pcap_file_header));
      if (!inputFile && inputFile.gcount() < sizeof (struct pcap_file_header)) {
         inputFile.close();
         delete [] buffer;
         ASSERT_TRUE(false);
      }
      struct pcap_file_header* fileHeader = reinterpret_cast<struct pcap_file_header*> (buffer);
      outputFile.open("resources/1000packets.tcpdump.copy", fstream::out | fstream::binary | fstream::trunc);
      struct pcap_file_header* ourFileHeader = t_allocator.GetPcapHdr();
      outputFile.write(reinterpret_cast<char*> (ourFileHeader), sizeof (struct pcap_file_header));
      free(ourFileHeader);
      ctb_ppacket dataPointer(NULL);
      char* zeros = new char[64];
      memset(zeros, 0, 64);
      while (!inputFile.eof()) {
         struct pcap_pkthdr* header = static_cast<struct pcap_pkthdr*> (malloc(sizeof (struct pcap_pkthdr)));
         inputFile.read(reinterpret_cast<char*> (buffer), sizeof (struct pcap_pkthdr) - 8);
         if (!inputFile && inputFile.gcount() < sizeof (struct pcap_pkthdr) - 8) {
            free(header);
            break;
         }
         memset(&header->ts.tv_sec, 0, 8);
         memset(&header->ts.tv_usec, 0, 8);
         memcpy(&header->ts.tv_sec, buffer, 4);
         memcpy(&header->ts.tv_usec, buffer + 4, 4);
         memcpy(&header->caplen, &buffer[sizeof (header->ts.tv_sec) + sizeof (header->ts.tv_sec) - 8], sizeof (header->caplen));
         memcpy(&header->len, &buffer[sizeof (header->ts.tv_sec) + sizeof (header->ts.tv_sec) - 8 + sizeof (header->caplen)], sizeof (header->len));
         size_t length = header->caplen;
         ASSERT_FALSE(header->caplen >= 65536);
         ASSERT_FALSE(header->caplen != header->len);

         inputFile.read(reinterpret_cast<char*> (buffer), length);
         ASSERT_FALSE(!inputFile && inputFile.gcount() < length);
         unsigned char* rawData = buffer;

         t_allocator.PopulatePacketData(rawData, header, dataPointer);
         ASSERT_FALSE(dataPointer == NULL);
         struct pcap_pkthdr * header2(NULL);
         unsigned char* raw2(NULL);
         ASSERT_TRUE(t_allocator.PopulatrePCapData(dataPointer, header2, raw2));
         char* rawheader2(NULL);
         t_allocator.GetHeaderForDiskWriting(header2, rawheader2);
         ASSERT_TRUE(rawheader2 != NULL);
         outputFile.write(reinterpret_cast<char*> (rawheader2), sizeof (struct pcap_pkthdr) - 8);
         delete [] rawheader2;
         outputFile.write(reinterpret_cast<char*> (raw2), header2->len);
         free(header2);
         free(raw2);
         free(dataPointer->data);
         free(dataPointer);
         free(header);
      }
      delete []zeros;
      outputFile.close();
      inputFile.close();
      delete [] buffer;
      ASSERT_TRUE(dataPointer != NULL);
      int returnVal = system("/usr/bin/diff resources/1000packets.tcpdump resources/1000packets.tcpdump.copy");
      ASSERT_EQ(0, WEXITSTATUS(returnVal));
      unlink("resources/1000packets.tcpdump.copy");
   }
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
