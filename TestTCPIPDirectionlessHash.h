/* 
 * File:   TestTCPIPDirectionlessHash.h
 * Author: root
 *
 * Created on January 22, 2013, 3:05 PM
 */
#pragma once

#include "gtest/gtest.h"
#include "MockTCPIPDirectionlessHash.h"

class TestTCPIPDirectionlessHash : public ::testing::Test {
public:

   TestTCPIPDirectionlessHash() :  packet(NULL) {

      packet = new uint8_t[1024];
      memset(packet, 0, 1024);
      for (int i = 0; i < IP6_ALEN; ++i) {
         first.push_back(i);
         second.push_back(IP6_ALEN - i);
         third.push_back(i+1);
         fourth.push_back(IP6_ALEN - i + 1);
      }
      struct IPHeader* ipHeader;
      ipHeader = (struct IPHeader*) (packet + sizeof (ether_header));
      ipHeader->ip_vhl = (4 << 4);
   };

   ~TestTCPIPDirectionlessHash() {

      if (packet) {
         delete[] packet;
      }
   }

   uint8_t* packet;
   std::vector<u_int8_t> first;
   std::vector<u_int8_t> second;
   std::vector<u_int8_t> third;
   std::vector<u_int8_t> fourth;
protected:

   virtual void SetUp() {
      eth = (struct ether_header *) packet;
      ipHeader = (struct IPHeader*) (packet + sizeof (ether_header));
      ip6Header = (struct IP6Header*) ipHeader;
      memcpy(eth->ether_shost, &first[0], ETH_ALEN);
      memcpy(eth->ether_dhost, &second[0], ETH_ALEN);
      memcpy(&ipHeader->ip_src.s_addr, &first[0], sizeof (in_addr));
      memcpy(&ipHeader->ip_dst.s_addr, &second[0], sizeof (in_addr));
   };

   virtual void TearDown() {

   };
   const uint32_t EthOnlyHash = 1157675110;
   const uint32_t IPv4Hash = 2922752215;
   const uint32_t IPv6Hash = 46847162;
   const uint32_t IPv6Hash2 = 1849378923;
   const uint32_t IPv6Hash3 = 2043957461;
   MockTCPIPDirectionlessHash generator;
   struct ether_header *eth;
   struct IPHeader* ipHeader;
   struct IP6Header* ip6Header;
private:

};

