/* 
 * File:   MockTCPIPDirectionlessHash.h
 * Author: root
 *
 * Created on January 22, 2013, 3:14 PM
 */
#pragma once

#include "TCPIPDirectionlessHash.h"

class MockTCPIPDirectionlessHash : public TCPIPDirectionlessHash {
public:

   MockTCPIPDirectionlessHash(): mRealSupportedFunction (true), mEthSupported(true){
   }

   ~MockTCPIPDirectionlessHash() {
   }

   uLong GetHash(const uint8_t* packet, const unsigned int length) LR_OVERRIDE {
      return TCPIPDirectionlessHash::GetHash(packet, length);
   }

   bool CompareEth(const u_int8_t* first, const u_int8_t* second) LR_OVERRIDE {
      return TCPIPDirectionlessHash::CompareEth(first, second);
   }

   bool CompareIp6(const u_int8_t* first, const u_int8_t* second) LR_OVERRIDE {
      return TCPIPDirectionlessHash::CompareIp6(first, second);
   }

   bool CompareArray(const u_int8_t* first, const u_int8_t* second, unsigned int length) LR_OVERRIDE {
      return TCPIPDirectionlessHash::CompareArray(first, second, length);
   }

   void GetSortedEthAddrs(struct ether_header *eth, bool& isIp, std::vector<uint8_t>& result) LR_OVERRIDE {
      return TCPIPDirectionlessHash::GetSortedEthAddrs(eth, isIp, result);
   }

   void GetSortedIPAddrs(struct ether_header *eth, std::vector<uint8_t>& result) LR_OVERRIDE {
      return TCPIPDirectionlessHash::GetSortedIPAddrs(eth, result);
   }

   bool CompareIp(const struct in_addr& first, const struct in_addr& second) LR_OVERRIDE {
      return TCPIPDirectionlessHash::CompareIp(first, second);
   }
   LR_VIRTUAL bool IsEthTypeSupported(const struct ether_header* eth) {
      if (mRealSupportedFunction) {
         return TCPIPDirectionlessHash::IsEthTypeSupported(eth);
      }
      return mEthSupported;
   }
   bool mRealSupportedFunction;
   bool mEthSupported;
private:

};

