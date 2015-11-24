/*
 * File:   KrakenBattle.h
 * Author: Kjell Hedstrom
 *
 * Created on November 24, 2015
 */

 #include "Kraken.h"
#include <vector>
#include <string>


#pragma once

namespace KrakenBattle {
   /**
   * Collection of static functions for forwarding data from Kraken to Harpoon receiver
   * (the www-UI)
   * Below the following format is used
   * uuid:       734a83c7-9435-4605-b1f9-4724c81faf21  (random uuid)
   * empty_uuid: 00000000-0000-0000-0000-000000000000
   * <DATA>, <END>, <ERROR>: Just like it seems, strings that look exactly like that
   * data: pcap chunks, raw bytes
   * error: error message, a string
   *
   * The data format is as follows
   * uuid<DATA>data: continously sending pcap chunks
   * uuid<ERROR>error: stop sending for one UUID, reason for error is given
   * uuid<DONE>: done with sending for one UUID, completed without error
   * empty_uuid<END>: done with sending for ALL UUIDs.
   * 
   *
   * 
   * Detailed explanation
   * Harpoon Kraken enables data streaming from a server to a client. ( pub:sub :: 1:1 communication).
   * Kraken: Server that sends the data
   * =======
   * SendTidalWave() : Send a data chunk to the client (harpoon). Blocks until there is space available in the queue. Returns TIMEOUT, INTERRUPT, CONTINUE status to indicate the status of the underlying queue.
   * FinalBreach() : Call to client (harpoon) to indicate the end of a stream.
   * 
   * Harpoon: Client that receives the data
   * =========
   * Aim() : Set location of the queue (tcp)
   * Heave() : Request data and wait for the data to be returned. Returns TIMEOUT, INTERRUPT, VICTORIOUS, CONTINUE to indicate status of the stream. VICTORIOUS means the stream has completed.
   */
   enum class SendType {Data, Done, Error, End};
   enum class ProgressType{Continue, Stop};

   std::vector<uint8_t>  MergeData(const std::string& uuid, const KrakenBattle::SendType& type, const Kraken::Chunks& optional_data, const std::string& optional_error_msg);
   KrakenBattle::ProgressType  SendChunks(Kraken* kraken, const std::string& uuid, const Kraken::Chunks& chunk, const KrakenBattle::SendType& type, const std::string& error);
   KrakenBattle::ProgressType  ForwardChunksToClient(Kraken* kraken, const std::string& uuid,const Kraken::Chunks& chunk, const KrakenBattle::SendType& sendState, const std::string& error);
   std::string EnumToString(const KrakenBattle::SendType& type);
   std::string EnumToString(const KrakenBattle::ProgressType& type);
} // KrakenBattle