/* 
 * File:   MockDpiMsgLR.h
 * Author: root
 *
 * Created on February 7, 2013, 8:51 AM
 */
#pragma once
#include "DpiMsgLR.h"
namespace networkMonitor {
class MockDpiMsgLR : public DpiMsgLR {
public:
	MockDpiMsgLR(){}
    virtual ~MockDpiMsgLR(){}
    uint32_t ConvertArrayToU32( const std::vector<uint8_t>& array ) {
    	return DpiMsgLR::ConvertArrayToU32(array);
    }
    uint64_t ConvertArrayToU64( const std::vector<uint8_t>& array , size_t size) {
    	return DpiMsgLR::ConvertArrayToU64(array, size);
    }
    std::string ConvertEtherValToString( uint64_t value )  {
    	return DpiMsgLR::ConvertEtherValToString(value);
    }
    std::string ConvertIpValToString( uint32_t value ) {
    	return DpiMsgLR::ConvertIpValToString(value);
    }
    std::map<unsigned int, std::pair<std::string,std::string> > GetAllFieldsAsStrings() {
    	return DpiMsgLR::GetAllFieldsAsStrings();
    }

    unsigned short ReadShortFromString(char *charbuffer, size_t& index) {
    	return DpiMsgLR::ReadShortFromString(charbuffer,index);
    }
    unsigned int ReadUIntFromString(char *charbuffer, size_t& index){
    	return DpiMsgLR::ReadUIntFromString(charbuffer,index);
    }
    std::pair<std::string, std::string> GetUuidPair() {
    	return DpiMsgLR::GetUuidPair();
    }
    std::pair<std::string, std::string> GetEthSrcPair(){
    	return DpiMsgLR::GetEthSrcPair();
    }
    std::pair<std::string, std::string> GetEthDstPair(){
    	return DpiMsgLR::GetEthDstPair();
    }
    std::pair<std::string, std::string> GetIpSrcPair(){
    	return DpiMsgLR::GetIpSrcPair();
    }
    std::pair<std::string, std::string> GetIpDstPair(){
    	return DpiMsgLR::GetIpDstPair();
    }
    std::pair<std::string, std::string> GetPktPathPair(){
    	return DpiMsgLR::GetPktPathPair();
    }
    std::pair<std::string, std::string> GetSourcePortPair() {
    	return DpiMsgLR::GetSourcePortPair();
    }
    std::pair<std::string, std::string> GetDestPortPair() {
    	return DpiMsgLR::GetDestPortPair();
    }
    void GetStaticFieldPairs(
  		  std::map<unsigned int, std::pair<std::string, std::string> >& formattedFieldData) {
    	DpiMsgLR::GetStaticFieldPairs(formattedFieldData);
    }
    void GetDynamicFieldPairs(
  		  std::map<unsigned int, std::pair<std::string, std::string> >& formattedFieldData) {
    	DpiMsgLR::GetDynamicFieldPairs(formattedFieldData);
    }
    void ReadIPSrcDstSPortDPortProto(char *charbuffer) {
    	DpiMsgLR::ReadIPSrcDstSPortDPortProto(charbuffer);
    }

};
}

