/*
 * File:   KrakenIntegrationHelper.h
 * Author: Kjell Hedstrom
 *
 * Created on November 30, 2015
 */

#include "KrakenIntegrationHelper.h"
#include <algorithm>
#include <iterator>
 
namespace KrakenIntegrationHelper {
   Kraken::Chunks GetRandomData(const size_t sizeOfData) {
      Kraken::Chunks someData;
      for (size_t j = 0; j < sizeOfData; j++) {
         someData.push_back(rand() % 255);
      }
      return someData;
   }


   std::string vectorToString(const std::vector<uint8_t>& vec) {
      std::string data;
      std::copy(vec.begin(), vec.end(), std::back_inserter(data));
      return data;
   }

   std::string vectorToString(const std::vector<uint8_t>& vec, size_t stopper) {
      std::string data;
      stopper = std::min(stopper, vec.size());
      std::copy(vec.begin(), vec.begin() + stopper, std::back_inserter(data));
      return data;
   }
} // KrakenIntegrationHelper