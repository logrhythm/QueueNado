#pragma once
#include "Kraken.h"

class MockKraken : public Kraken {
public:

   MockKraken() : Kraken() {
   }

   virtual ~MockKraken() {
   }

   Kraken::Battling CallNextChunkId() {
      return Kraken::NextChunkId();
   }

   Kraken::Battling CallPollTimeout(int timeoutMs) {
      return Kraken::PollTimeout(timeoutMs);
   }

};

