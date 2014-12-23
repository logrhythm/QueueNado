#pragma once
#include "Vampire.h"
#include <type_traits>

class ReceivePacketZMQ : public Vampire {
public:
   explicit ReceivePacketZMQ(const std::string& binding);
   bool Initialize();
   bool ReceiveDataBlock(std::string& dataReceived, const int nSecs);

   template<typename T>
   bool ReceiveDataBlockPointer(T& dataReceived, const int milliseconds) {
      static_assert(std::is_pointer<T>::value, "T must be of pointer type");

      void *data = nullptr;
      bool returnVal = Vampire::GetStake(data, milliseconds);
      if ( returnVal) {
         dataReceived = reinterpret_cast<T>(data);
      } else {
         dataReceived = nullptr;
      }
      return returnVal;
   }
   void SetQueueSize(const int size);
protected:
};
