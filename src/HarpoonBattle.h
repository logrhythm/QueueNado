/*
 * File:   HarpoonBattle.h
 * Author: Kjell Hedstrom
 *
 * Created on November 30, 2015
 */



#pragma once

#include "KrakenBattle.h"
#include "Kraken.h"
#include <Result.h>
#include <string>
#include <tuple>




/**
* The Harpoon Battle namespace provides necesassary functionality
* to parse out the data from incoming Kraken streams
* As the data comes in it will contain (if run through KrakenBattle)
* some KrakenBattle headers. Typically the data will be
* session_id   <STATE>   data
*
* What's interesting to receive back is all three of these but of course separated.
*/
namespace HarpoonBattle {

   enum class ReceivedType {
      Begin = KrakenBattle::SendType::Begin, // optional
      Data = KrakenBattle::SendType::Data,
      Done = KrakenBattle::SendType::Done,
      Error = KrakenBattle::SendType::Error,
      End = KrakenBattle::SendType::End
   };

   std::string EnumToString(const ReceivedType& type);
   ReceivedType StringToEnum(const std::string& type);

   enum ReceivedPartsIndex {IndexOfSession, IndexOfReceivedType, IndexOfChunk};
   using ReceivedParts = std::tuple<std::string, ReceivedType, Kraken::Chunks>;


   ReceivedParts ExtractToParts(const Kraken::Chunks& chunks);


} // HarpoonBattle