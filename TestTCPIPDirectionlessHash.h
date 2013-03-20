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
protected:

   virtual void SetUp() {

   };

   virtual void TearDown() {

   };

private:

};

