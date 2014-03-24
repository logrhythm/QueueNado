
#include "TestTCPIPDirectionlessHash.h"

TEST_F(TestTCPIPDirectionlessHash, IpTypeIdentified) {

   bool isIp;
   std::vector<u_int8_t> result;

   std::vector<u_int8_t> first;
   std::vector<u_int8_t> second;
   for (int i = 0; i < ETH_ALEN; ++i) {
      first.push_back(i);
      second.push_back(ETH_ALEN - i);
   }

   memcpy(eth->ether_shost, &first[0], ETH_ALEN);
   memcpy(eth->ether_dhost, &second[0], ETH_ALEN);
   eth->ether_type = htons(ETHERTYPE_PUP);
   generator.GetSortedEthAddrs(eth, isIp, result);
   EXPECT_FALSE(isIp);
   eth->ether_type = htons(ETHERTYPE_SPRITE);
   generator.GetSortedEthAddrs(eth, isIp, result);
   EXPECT_FALSE(isIp);
   eth->ether_type = htons(ETHERTYPE_ARP);
   generator.GetSortedEthAddrs(eth, isIp, result);
   EXPECT_FALSE(isIp);
   eth->ether_type = htons(ETHERTYPE_REVARP);
   generator.GetSortedEthAddrs(eth, isIp, result);
   EXPECT_FALSE(isIp);
   eth->ether_type = htons(ETHERTYPE_AT);
   generator.GetSortedEthAddrs(eth, isIp, result);
   EXPECT_FALSE(isIp);
   eth->ether_type = htons(ETHERTYPE_AARP);
   generator.GetSortedEthAddrs(eth, isIp, result);
   EXPECT_FALSE(isIp);
   eth->ether_type = htons(ETHERTYPE_VLAN);
   generator.GetSortedEthAddrs(eth, isIp, result);
   EXPECT_FALSE(isIp);
   eth->ether_type = htons(ETHERTYPE_IPX);
   generator.GetSortedEthAddrs(eth, isIp, result);
   EXPECT_FALSE(isIp);
   eth->ether_type = htons(ETHERTYPE_IPV6);
   generator.GetSortedEthAddrs(eth, isIp, result);
   EXPECT_TRUE(isIp);
   eth->ether_type = htons(ETHERTYPE_LOOPBACK);
   generator.GetSortedEthAddrs(eth, isIp, result);
   EXPECT_FALSE(isIp);
   //#define	ETHERTYPE_PUP		0x0200          /* Xerox PUP */
   //#define ETHERTYPE_SPRITE	0x0500		/* Sprite */
   //#define	ETHERTYPE_IP		0x0800		/* IP */
   //#define	ETHERTYPE_ARP		0x0806		/* Address resolution */
   //#define	ETHERTYPE_REVARP	0x8035		/* Reverse ARP */
   //#define ETHERTYPE_AT		0x809B		/* AppleTalk protocol */
   //#define ETHERTYPE_AARP		0x80F3		/* AppleTalk ARP */
   //#define	ETHERTYPE_VLAN		0x8100		/* IEEE 802.1Q VLAN tagging */
   //#define ETHERTYPE_IPX		0x8137		/* IPX */
   //#define	ETHERTYPE_IPV6		0x86dd		/* IP protocol version 6 */
   //#define ETHERTYPE_LOOPBACK	0x9000		/* used to test interfaces */


   eth->ether_type = htons(ETHERTYPE_IP);
   generator.GetSortedEthAddrs(eth, isIp, result);
   EXPECT_TRUE(isIp);
}

TEST_F(TestTCPIPDirectionlessHash, EtherSorted) {

   bool isIp;
   std::vector<u_int8_t> result;

   memcpy(eth->ether_shost, &first[0], ETH_ALEN);
   memcpy(eth->ether_dhost, &second[0], ETH_ALEN);

   eth->ether_type = htons(ETHERTYPE_IP);
   generator.GetSortedEthAddrs(eth, isIp, result);
   EXPECT_EQ(ETH_ALEN * 2, result.size());
   EXPECT_EQ(second[0], result[0]);
   EXPECT_EQ(first[0], result[ETH_ALEN]);
   result.clear();
   memcpy(eth->ether_dhost, &first[0], ETH_ALEN);
   memcpy(eth->ether_shost, &second[0], ETH_ALEN);
   generator.GetSortedEthAddrs(eth, isIp, result);
   EXPECT_EQ(ETH_ALEN * 2, result.size());
   EXPECT_EQ(second[0], result[0]);
   EXPECT_EQ(first[0], result[ETH_ALEN]);
}

TEST_F(TestTCPIPDirectionlessHash, TestCompareEth) {
   for (int i = 0; i < ETH_ALEN; ++i) {
      first[i] = 0;
      second[i] = 0;
   }
   for (int i = 0; i < ETH_ALEN; ++i) {
      EXPECT_TRUE(generator.CompareEth(&first[0], &second[0]));
      first[i] = i + 2;
      for (int j = i + 1; j < ETH_ALEN; ++j) {
         second[j] = i + 1;
      }
      EXPECT_TRUE(generator.CompareEth(&first[0], &second[0]));
      second[i] = i + 3;
      EXPECT_FALSE(generator.CompareEth(&first[0], &second[0]));
      first[i] = i + 3;
      for (int j = i + 1; j < ETH_ALEN; ++j) {
         first[j] = i + 1;
      }
      EXPECT_TRUE(generator.CompareEth(&first[0], &second[0]));
   }

}

TEST_F(TestTCPIPDirectionlessHash, TestCompareArray) {
   unsigned int length = 12;
   for (int i = 0; i < length; ++i) {
      first[i] = 0;
      second[i] = 0;
   }
   for (int i = 0; i < length; ++i) {
      EXPECT_TRUE(generator.CompareArray(&first[0], &second[0], length));
      first[i] = i + 2;
      for (int j = i + 1; j < length; ++j) {
         second[j] = i + 1;
      }
      EXPECT_TRUE(generator.CompareArray(&first[0], &second[0], length));
      second[i] = i + 3;
      EXPECT_FALSE(generator.CompareArray(&first[0], &second[0], length));
      first[i] = i + 3;
      for (int j = i + 1; j < length; ++j) {
         first[j] = i + 1;
      }
      EXPECT_TRUE(generator.CompareArray(&first[0], &second[0], length));
   }

}

TEST_F(TestTCPIPDirectionlessHash, TestCompareIp6) {
   for (int i = 0; i < IP6_ALEN; ++i) {
      first[i] = 0;
      second[i] = 0;
   }
   for (int i = 0; i < IP6_ALEN; ++i) {
      EXPECT_TRUE(generator.CompareIp6(&first[0], &second[0]));
      first[i] = i + 2;
      for (int j = i + 1; j < IP6_ALEN; ++j) {
         second[j] = i + 1;
      }
      EXPECT_TRUE(generator.CompareIp6(&first[0], &second[0]));
      second[i] = i + 3;
      EXPECT_FALSE(generator.CompareIp6(&first[0], &second[0]));
      first[i] = i + 3;
      for (int j = i + 1; j < IP6_ALEN; ++j) {
         first[j] = i + 1;
      }
      EXPECT_TRUE(generator.CompareIp6(&first[0], &second[0]));
   }

}
TEST_F(TestTCPIPDirectionlessHash, TestCompareIP) {

   for (int i = 0; i < sizeof (in_addr); ++i) {
      first[i] = 0;
      second[i] = 0;
   }
   for (int i = 0; i < sizeof (in_addr); ++i) {
      struct in_addr firstA;
      struct in_addr secondA;
      memcpy(&(firstA.s_addr), &first[0], sizeof (firstA.s_addr));
      memcpy(&(secondA.s_addr), &second[0], sizeof (secondA.s_addr));
      EXPECT_TRUE(generator.CompareIp(firstA, secondA));
      first[sizeof (in_addr) - i - 1] = i + 2;
      for (int j = sizeof (in_addr) - i - 1 - 1; j > 0; --j) {
         second[j] = i + 1;
      }
      memcpy(&(firstA.s_addr), &first[0], sizeof (firstA.s_addr));
      memcpy(&(secondA.s_addr), &second[0], sizeof (secondA.s_addr));
      EXPECT_TRUE(generator.CompareIp(firstA, secondA));
      second[sizeof (in_addr) - i - 1] = i + 3;
      memcpy(&(firstA.s_addr), &first[0], sizeof (firstA.s_addr));
      memcpy(&(secondA.s_addr), &second[0], sizeof (secondA.s_addr));
      EXPECT_FALSE(generator.CompareIp(firstA, secondA));
      first[sizeof (in_addr) - i - 1] = i + 3;
      for (int j = sizeof (in_addr) - i - 1 - 1; j > 0; --j) {
         first[j] = i + 1;
      }
      memcpy(&(firstA.s_addr), &first[0], sizeof (firstA.s_addr));
      memcpy(&(secondA.s_addr), &second[0], sizeof (secondA.s_addr));
      EXPECT_TRUE(generator.CompareIp(firstA, secondA));
   }

}

TEST_F(TestTCPIPDirectionlessHash, IpSorted) {

   std::vector<u_int8_t> result;
   struct IPHeader* ipHeader;
   ipHeader = (struct IPHeader*) (packet + sizeof (ether_header));
   memcpy(&ipHeader->ip_src.s_addr, &first[0], sizeof (in_addr));
   memcpy(&ipHeader->ip_dst.s_addr, &second[0], sizeof (in_addr));
   generator.GetSortedIPAddrs(reinterpret_cast<struct ether_header *>(packet), result);
   ASSERT_EQ(sizeof (in_addr) * 2, result.size());
   EXPECT_EQ(second[sizeof (in_addr) - 1], result[0]);
   EXPECT_EQ(first[sizeof (in_addr) - 1], result[sizeof (in_addr)]);
   result.clear();
   memcpy(&ipHeader->ip_dst.s_addr, &first[0], sizeof (in_addr));
   memcpy(&ipHeader->ip_src.s_addr, &second[0], sizeof (in_addr));
   generator.GetSortedIPAddrs(reinterpret_cast<struct ether_header *>(packet), result);
   ASSERT_EQ(sizeof (in_addr) * 2, result.size());
   EXPECT_EQ(second[sizeof (in_addr) - 1], result[0]);
   EXPECT_EQ(first[sizeof (in_addr) - 1], result[sizeof (in_addr)]);
}

TEST_F(TestTCPIPDirectionlessHash, HashEthPacket) {

   eth->ether_type = htons(ETHERTYPE_ARP);
   EXPECT_EQ(EthOnlyHash, generator.GetHash(packet,1024));
   eth->ether_type = htons(ETHERTYPE_PUP);
   EXPECT_EQ(EthOnlyHash, generator.GetHash(packet,1024));
   eth->ether_type = htons(ETHERTYPE_SPRITE);
   EXPECT_EQ(EthOnlyHash, generator.GetHash(packet,1024));
   eth->ether_type = htons(ETHERTYPE_REVARP);
   EXPECT_EQ(EthOnlyHash, generator.GetHash(packet,1024));
   eth->ether_type = htons(ETHERTYPE_AT);
   EXPECT_EQ(EthOnlyHash, generator.GetHash(packet,1024));
   eth->ether_type = htons(ETHERTYPE_AARP);
   EXPECT_EQ(EthOnlyHash, generator.GetHash(packet,1024));
   eth->ether_type = htons(ETHERTYPE_IPX);
   EXPECT_EQ(EthOnlyHash, generator.GetHash(packet,1024));
   eth->ether_type = htons(ETHERTYPE_LOOPBACK);
   EXPECT_EQ(EthOnlyHash, generator.GetHash(packet,1024));
   
   memcpy(eth->ether_dhost, &second[0], ETH_ALEN);
   memcpy(eth->ether_shost, &first[0], ETH_ALEN);
   EXPECT_EQ(EthOnlyHash, generator.GetHash(packet,1024));  
}

TEST_F(TestTCPIPDirectionlessHash, UnsupportedTypesReturn0) {

   generator.mRealSupportedFunction = false;
   generator.mEthSupported = false;
   EXPECT_EQ(0, generator.GetHash(packet,1024));
   
}

TEST_F(TestTCPIPDirectionlessHash, StripdoubleVlans) {
   
   eth->ether_type = htons(0x9100);
   struct ether_header* stripped = generator.StripVLANHeaders(eth);
   EXPECT_EQ(reinterpret_cast<uint8_t*>(eth)+8,reinterpret_cast<uint8_t*>(stripped));
   generator.GetEthHeader(packet,stripped);
   EXPECT_EQ(reinterpret_cast<uint8_t*>(eth)+8,reinterpret_cast<uint8_t*>(stripped));
   eth->ether_type = htons(0x88A8);  
   stripped = generator.StripVLANHeaders(eth);
   EXPECT_EQ(reinterpret_cast<uint8_t*>(eth)+8,reinterpret_cast<uint8_t*>(stripped));
   generator.GetEthHeader(packet,stripped);
   EXPECT_EQ(reinterpret_cast<uint8_t*>(eth)+8,reinterpret_cast<uint8_t*>(stripped));
   eth->ether_type = htons(ETHERTYPE_VLAN);  
   stripped = generator.StripVLANHeaders(eth);
   EXPECT_EQ(reinterpret_cast<uint8_t*>(eth)+4,reinterpret_cast<uint8_t*>(stripped));
   generator.GetEthHeader(packet,stripped);
   EXPECT_EQ(reinterpret_cast<uint8_t*>(eth)+4,reinterpret_cast<uint8_t*>(stripped));

}
TEST_F(TestTCPIPDirectionlessHash, HashIPv4Packet) {

   eth->ether_type = htons(ETHERTYPE_IP);
   
   EXPECT_EQ(IPv4Hash, generator.GetHash(packet,1024));
   memcpy(&ipHeader->ip_dst.s_addr, &first[0], sizeof (in_addr));
   memcpy(&ipHeader->ip_src.s_addr, &second[0], sizeof (in_addr));
   EXPECT_EQ(IPv4Hash, generator.GetHash(packet,1024));
   memcpy(eth->ether_dhost, &third[0], ETH_ALEN);
   memcpy(eth->ether_shost, &fourth[0], ETH_ALEN);
   EXPECT_EQ(IPv4Hash, generator.GetHash(packet,1024));
   
}
TEST_F(TestTCPIPDirectionlessHash, HashIPv6Packet) {
   ipHeader->ip_vhl = (6 << 4);
   eth->ether_type = htons(ETHERTYPE_IP);
   memcpy(&ip6Header->ip_dst, &first[0], IP6_ALEN);
   memcpy(&ip6Header->ip_src, &second[0], IP6_ALEN);
   EXPECT_EQ(IPv6Hash, generator.GetHash(packet,1024));
   memcpy(&ip6Header->ip_dst, &second[0], IP6_ALEN);
   memcpy(&ip6Header->ip_src, &first[0], IP6_ALEN);
   EXPECT_EQ(IPv6Hash, generator.GetHash(packet,1024));   
   memcpy(eth->ether_dhost, &second[0], ETH_ALEN);
   memcpy(eth->ether_shost, &first[0], ETH_ALEN);
   EXPECT_EQ(IPv6Hash, generator.GetHash(packet,1024));   
   
   memcpy(eth->ether_dhost, &third[0], ETH_ALEN);
   memcpy(eth->ether_shost, &fourth[0], ETH_ALEN);
   EXPECT_EQ(IPv6Hash2, generator.GetHash(packet,1024));
   memcpy(eth->ether_dhost, &third[0], ETH_ALEN);
   memcpy(eth->ether_shost, &fourth[0], ETH_ALEN);
   EXPECT_EQ(IPv6Hash2, generator.GetHash(packet,1024));
   memset(&ip6Header->ip_dst, 'a',1);
   EXPECT_EQ(IPv6Hash3, generator.GetHash(packet,1024));
}