#pragma once
#include "FileSend.h"

class MockFileSend : public FileSend {
public:

   MockFileSend() : FileSend() {
   }

   virtual ~MockFileSend() {
   }

   FileSend::Stream CallNextChunkId() {
      return NextChunkId();
   }

};

