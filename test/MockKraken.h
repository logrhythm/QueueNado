#pragma once
#include "Kraken.h"

class MockKraken : public Kraken {
public:

   MockKraken() : Kraken() {
   }

   virtual ~MockKraken() {
   }

   Kraken::Battling CallNextChunkId() {
      return NextChunkId();
   }

};

