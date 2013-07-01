/* 
 * File:   MockDpiMsgLR.h
 * Author: root
 *
 * Created on February 7, 2013, 8:51 AM
 */
#pragma once
#include "DpiMsgLR.h"
#ifdef LR_DEBUG
namespace networkMonitor {

   class MockDpiMsgLR : public DpiMsgLR {
   public:

      MockDpiMsgLR() : mRepeats(-1) {
      }

      virtual ~MockDpiMsgLR() {
      }


      std::map<unsigned int, std::pair<std::string, std::string> > GetAllFieldsAsStrings() const override {
         return DpiMsgLR::GetAllFieldsAsStrings();
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
#endif
