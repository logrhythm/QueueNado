#pragma once
#include "FileRecv.h"

class MockFileRecv : public FileRecv {
public:

   MockFileRecv() : FileRecv() {
   }

   virtual ~MockFileRecv() {
   }

   void CallRequestChunks() {
      void RequestChunks();
   }

};

