#pragma once
#include "FileSend.h"

class MockFileSend : public FileSend {
public:

   MockFileSend() : FileSend() {
   }

   virtual ~MockFileSend() {
   }

   int CallNextChunkId() {
      return NextChunkId();
   }

};

