/*
 * File:   HarpoonBattle.h
 * Author: Kjell Hedstrom
 *
 * Created on November 30, 2015
 */


#include "HarpoonBattle.h"
#include <g3log/g3log.hpp>
#include <algorithm>
#include <iterator>

namespace HarpoonBattle {
   std::string EnumToString(const ReceivedType& type) {
      std::string textType = "<ERROR>";
      switch (type) {
         case ReceivedType::Data : textType = "<DATA>"; break;
         case ReceivedType::Done : textType = "<DONE>"; break;
         case ReceivedType::Error : textType = "<ERROR>"; break;
         case ReceivedType::End : textType = "<END>"; break;
         default:
            LOG(FATAL) << "Unknown ReceivedType: " << static_cast<int>(type);
      }
      return textType;
   }

   ReceivedType StringToEnum(const std::string& type) {
      if (type == "<DATA>") {
         return ReceivedType::Data;
      } else if (type == "<DONE>") {
         return ReceivedType::Done;
      } else if (type == "<ERROR>") {
         return ReceivedType::Error;
      } else if (type == "<END>") {
         return ReceivedType::End;
      }

      LOG(WARNING) << "Could not transform string to HarpoonBattle::ReceivedType. input was: " << type;
      return ReceivedType::Error;
   }


   /** Recived Chunks should conform to the following format
   * if they are being sent through the KrakenBattle
   * UUIID<STATUS>DATA
   *
   * Where STATUS is one of
   * '<DATA>'
   * '<DONE>'
   * '<END>'
   * '<ERROR>'
   *
   * If STATUS is '<ERROR>' then the 'DATA' part of the chunk
   * will instead be an error message instead of raw data
   *
   * @returned data will be in tuple format accordint to the
   * typedef @HarpoonBattle::ReceivedParts
   */
   ReceivedParts ExtractToParts(const Kraken::Chunks& chunks) {

      // Extract session
      const Kraken::Chunks noChunks = {};
      auto uuidEnd = std::find(chunks.begin(), chunks.end(), '<');
      if (uuidEnd == chunks.end() || uuidEnd == chunks.begin()) {
         LOG(WARNING) << "received chunks does not conform to Kraken-Harpoon communication protocol";
         return std::make_tuple(ReceivedType::Error, noChunks, "no session");
      }

      std::string session;
      std::copy(chunks.begin(), uuidEnd, std::back_inserter(session));
      

      // Extract type
      auto typeEnd = std::find(chunks.begin(), chunks.end(), '>');
      if (typeEnd == chunks.end()) {
         LOG(WARNING) << "received chunks does not conform to Kraken-Harpoon communication protocol";
         return std::make_tuple(ReceivedType::Error, noChunks, session);
      }
      typeEnd = typeEnd+1; // this is possibly chunks.end()
      std::string rawType;
      std::copy(uuidEnd, typeEnd, std::back_inserter(rawType));
      auto type = StringToEnum(rawType);

      // End, Done or Error received
      if (type != ReceivedType::Data) {
        return std::make_tuple(type, noChunks, session);
      }


      // Extract raw data
      Kraken::Chunks data;
      const size_t distance = std::distance(typeEnd, chunks.end());
      data.reserve(distance);
      LOG(INFO) << "distance: " << distance;
      std::copy(typeEnd, chunks.end(), std::back_inserter(data));

      //auto type = HarpoonBattle::StringToEnum(rawType);
      return std::make_tuple(type, data, session);
   }

} // HarpoonBattle