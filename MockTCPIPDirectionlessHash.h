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
MockTCPIPDirectionlessHash() {}
~MockTCPIPDirectionlessHash() {}
uLong GetHash(const uint8_t* packet) {
   return TCPIPDirectionlessHash::GetHash(packet);
}
bool CompareEth(const  u_int8_t* first,const  u_int8_t* second) {
   return TCPIPDirectionlessHash::CompareEth(first,second);
}
bool CompareIp6(const  u_int8_t* first,const  u_int8_t* second) {
   return TCPIPDirectionlessHash::CompareIp6(first,second);
}
bool CompareArray(const  u_int8_t* first,const  u_int8_t* second, unsigned int length) {
   return TCPIPDirectionlessHash::CompareArray(first,second,length);
}
void GetSortedEthAddrs(const uint8_t* packet, bool& isIp, std::string& result) {
   return TCPIPDirectionlessHash::GetSortedEthAddrs(packet,isIp,result);
}
void GetSortedIPAddrs(const uint8_t* packet, std::string& result) {
   return TCPIPDirectionlessHash::GetSortedIPAddrs(packet,result);
}
bool CompareIp(const struct in_addr& first, const struct in_addr& second) {
   return TCPIPDirectionlessHash::CompareIp(first,second);
}
private:

};

