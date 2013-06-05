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

      MockDpiMsgLR() : mRepeats(-1) {
      }

      virtual ~MockDpiMsgLR() {
      }

      uint32_t ConvertArrayToU32(const std::vector<uint8_t>& array) const override {
         return DpiMsgLR::ConvertArrayToU32(array);
      }

      uint64_t ConvertArrayToU64(const std::vector<uint8_t>& array, size_t size) const override {
         return DpiMsgLR::ConvertArrayToU64(array, size);
      }

      std::string ConvertEtherValToString(uint64_t value) const override {
         return DpiMsgLR::ConvertEtherValToString(value);
      }

      std::string ConvertIpValToString(uint32_t value) const override {
         return DpiMsgLR::ConvertIpValToString(value);
      }

      std::map<unsigned int, std::pair<std::string, std::string> > GetAllFieldsAsStrings() const override {
         return DpiMsgLR::GetAllFieldsAsStrings();
      }

      unsigned short ReadShortFromString(const char *charbuffer, size_t& index) override {
         return DpiMsgLR::ReadShortFromString(charbuffer, index);
      }

      unsigned int ReadUIntFromString(const char *charbuffer, size_t& index) override {
         return DpiMsgLR::ReadUIntFromString(charbuffer, index);
      }

      std::pair<std::string, std::string> GetUuidPair() const override {
         return DpiMsgLR::GetUuidPair();
      }

      std::pair<std::string, std::string> GetEthSrcPair() const override {
         return DpiMsgLR::GetEthSrcPair();
      }

      std::pair<std::string, std::string> GetEthDstPair() const override {
         return DpiMsgLR::GetEthDstPair();
      }

      std::pair<std::string, std::string> GetIpSrcPair() const override {
         return DpiMsgLR::GetIpSrcPair();
      }

      std::pair<std::string, std::string> GetIpDstPair() const override {
         return DpiMsgLR::GetIpDstPair();
      }

      std::pair<std::string, std::string> GetPktPathPair() const override {
         return DpiMsgLR::GetPktPathPair();
      }

      std::pair<std::string, std::string> GetSourcePortPair() const override {
         return DpiMsgLR::GetSourcePortPair();
      }

      std::pair<std::string, std::string> GetDestPortPair() const override {
         return DpiMsgLR::GetDestPortPair();
      }

      void GetStaticFieldPairs(
              std::map<unsigned int, std::pair<std::string, std::string> >& formattedFieldData) const override {
         DpiMsgLR::GetStaticFieldPairs(formattedFieldData);
      }

      void GetDynamicFieldPairs(
              std::map<unsigned int, std::pair<std::string, std::string> >& formattedFieldData) const override {
         DpiMsgLR::GetDynamicFieldPairs(formattedFieldData);
      }

      void ReadIPSrcDstSPortDPortProto(const char *charbuffer) override {
         DpiMsgLR::ReadIPSrcDstSPortDPortProto(charbuffer);
      }

      int CountUpRepeats() override {
         if (mRepeats >= 0) {
            return mRepeats;
         } else {
            return DpiMsgLR::CountUpRepeats();
         }
      }
      int mRepeats;

   };
}

