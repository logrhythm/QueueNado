/*
 * File:   KrakenIntegrationHelper.h
 * Author: Kjell Hedstrom
 *
 * Created on November 30, 2015
 */

#pragma once

#include "Kraken.h"
#include <vector>
#include <string>

namespace KrakenIntegrationHelper {
   Kraken::Chunks GetRandomData(const size_t sizeOfData);
   std::string vectorToString(const std::vector<uint8_t>& vec);
   std::string vectorToString(const std::vector<uint8_t>& vec, size_t stopper);
}