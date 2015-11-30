/*
 * File:   KrakenBattle.cpp
 * Author: Kjell Hedstrom
 *
 * Created on November 24, 2015
 */

#include "KrakenBattle.h"
#include <algorithm>
#include <iterator>
#include <g3log/g3log.hpp>

namespace {
   const std::string emptyUUID = {"00000000-0000-0000-0000-000000000000"};
   const std::vector<uint8_t> emptyData = {};
}



namespace KrakenBattle {

   /**
   * formats the data to send to the Harpoon according to the specification for
   * data transfers
   * @param uuid
   * @param type one of Data, Done, Error, End
   * @param data to send (optional, used for SendType::Data)
   * @param error message (optional, used for SendType::Error)
   * @return merged data uuid<SendType>optional:data/optional:error_msg
   *
   * Ref: KrakenBattle.h for detailed information regarding the sending
   */
   Kraken::Chunks  MergeData(const std::string& uuid, const KrakenBattle::SendType& type, const Kraken::Chunks& data, const std::string& error_msg) {
      const std::string sendType = EnumToString(type);
      const std::string& uuidToSend = (SendType::End == type ? emptyUUID : uuid);
      std::vector<uint8_t> merged;
      merged.reserve(uuidToSend.size() + sendType.size() + data.size() + error_msg.size());
      // uuid, type, possible:data, possible:error
      std::copy(uuidToSend.begin(), uuidToSend.end(), std::back_inserter(merged));
      std::copy(sendType.begin(), sendType.end(), std::back_inserter(merged));

      if (SendType::Data == type) {
         std::copy(data.begin(), data.end(), std::back_inserter(merged));
      } else if (SendType::Error == type) {
         std::copy(error_msg.begin(), error_msg.end(), std::back_inserter(merged));
      }
      return merged;
   }


   /**
   *  Send  Chunks over Kraken to a Harpoon
   *  If chunks to send is more than the @ref Kraken::MaxChunkSizeInBytes() then
   *  it will split up the sending into several separate sends (@ref Kraken::SendTidalWave)
   * @param kraken to send over
   * @param uuid
   * @param sendState
   * @param chunk to send (optional content, for SendType::Data)
   * @param error to send (optional content, for SendType::Error)
   *
   * Ref: KrakenBattle.h for detailed information regarding the sending
   */
   Kraken::Battling SendChunks(Kraken* kraken, const std::string& uuid, const KrakenBattle::SendType& sendState, const Kraken::Chunks& chunk, const std::string& error) {
      const size_t kSplitSize = kraken->MaxChunkSizeInBytes();
      const auto kTotalToSend = MergeData(uuid, sendState, chunk, error);
      const size_t kTotalSize = kTotalToSend.size();
      const Kraken::Chunks kHeader = MergeData(uuid, sendState, {}, {}); // {}: ignored
      const size_t kSplitSizeAdjusted = kSplitSize - kHeader.size();
      CHECK(kSplitSize > kHeader.size());

      auto result = Kraken::Battling::CONTINUE;

      if (kTotalSize <= kSplitSize) {
         result = kraken->SendTidalWave(kTotalToSend);
      } else {

         std::vector<uint8_t> toSend;
         toSend.reserve(kSplitSize);
         toSend.assign(kTotalToSend.begin(), kTotalToSend.begin() + kSplitSize);

         CHECK(toSend.size() == kSplitSize);
         result = kraken->SendTidalWave(toSend);

         for (size_t i = kSplitSize; i <= kTotalSize; i += kSplitSizeAdjusted) {
            if (result != Kraken::Battling::CONTINUE) {
               LOG(WARNING) << "Sending UUID: " << uuid << ", #split break: " << i << ", kTotalSize: " << kTotalSize
                            << ", kSplitSizeAdjusted: " << kSplitSizeAdjusted << ", result: " << static_cast<int>(result);
               break;
            }

            toSend.assign(kHeader.begin(), kHeader.end());
            const size_t kLeftSize = kTotalSize - i;
            const size_t kChunkSize = std::min(kLeftSize, kSplitSizeAdjusted);
            std::copy(kTotalToSend.begin() + i,
                      kTotalToSend.begin() + i + kChunkSize,
                      std::back_inserter(toSend));
            LOG(INFO) << "Sending UUID: " << uuid << ", #split: " << i << ", toSend size: " << toSend.size();
            result = kraken->SendTidalWave(toSend);
         }
      }
      return result;
   }



   /**
   * Forward  chunks to the client. This can be repeatedly called until an error
   * occurrs (interrupt, timeout) or all is transmitted
   * @param kraken to send the harpoon/client
   * @param uuid to for unique identification
   * @param chunk to send to the client should have valid data on SendState::Data
   * @param sendState (End, Done, Data, Error)
   * @param error should have meaningful content on  SendState::Error
   *
   * Ref: KrakenBattle.h for detailed information regarding the sending
   */
   KrakenBattle::ProgressType  ForwardChunksToClient(Kraken* kraken, const std::string& uuid, const Kraken::Chunks& chunk,
         const KrakenBattle::SendType& sendState, const std::string& error) {
      auto sendingResult = SendChunks(kraken, uuid, sendState, chunk, error);
      bool result = (Kraken::Battling::CONTINUE == sendingResult);
      LOG_IF(WARNING, (!result)) << "Failed to send 'SendTidalWave'" << ", uuid: " << uuid
                                 << ", sendState: " << KrakenBattle::EnumToString(sendState)
                                 << ", sending result: " << kraken->EnumToString(sendingResult);


      if (SendType::End == sendState) {
         sendingResult = kraken->FinalBreach();
         result = (Kraken::Battling::CONTINUE == sendingResult);
         LOG_IF(WARNING, (!result)) << "Failed to send 'FinalBreach'" << ", uuid: " << uuid
                                    << ", sendState: " << KrakenBattle::EnumToString(sendState)
                                    << ", sending result: " << kraken->EnumToString(sendingResult);
      }

      auto status = KrakenBattle::ProgressType::Stop;
      if (result) {
         status = KrakenBattle::ProgressType::Continue;
      }
      return status;
   }




   std::string EnumToString(const KrakenBattle::SendType& type) {
      std::string textType = "<ERROR>";
      switch (type) {
         case SendType::Data : textType = "<DATA>"; break;
         case SendType::Done : textType = "<DONE>"; break;
         case SendType::Error : textType = "<ERROR>"; break;
         case SendType::End : textType = "<END>"; break;
         default:
            LOG(FATAL) << "Unknown SendType: " << static_cast<int>(type);
      }
      return textType;
   }

   std::string EnumToString(const KrakenBattle::ProgressType& type) {
      std::string textType = "<ERROR>";
      switch (type) {
         case ProgressType::Stop : textType = "ProgressType::Stop"; break;
         case ProgressType::Continue : textType = "ProgressType::Continue"; break;
         default:
            LOG(FATAL) << "Unknown ProgressType: " << static_cast<int>(type);
      }
      return textType;
   }
} // namespace KrakenBattle
