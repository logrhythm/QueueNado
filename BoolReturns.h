#pragma once
class BoolReturns {
public:
   BoolReturns() : callsToReturnTrue(0), callsToReturnFalse(0){}
   bool ReturnTrue() {
      callsToReturnTrue++;
      return true;
   }
   bool ReturnFalse() {
      callsToReturnFalse++;
      return false;
   }
   int callsToReturnTrue;
   int callsToReturnFalse;
};
