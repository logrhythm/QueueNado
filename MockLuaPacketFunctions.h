

#include "LuaPacketFunctions.h"
#include "DiskPacketCapture.h"
#include "include/global.h"

class MockLuaPacketFunctions : public LuaPacketFunctions {
public:

   MockLuaPacketFunctions()
   : LuaPacketFunctions()
   , mOverrideStartPacketCapture(false)
   , mOverridePacketCapturePossible(false) {
   }

   void StartPacketCapture(Conf& conf) LR_OVERRIDE {
      if (mOverrideStartPacketCapture) {
         if (!gPacketCapture) {
            gPacketCapture = new DiskPacketCapture(conf);
         }
         gPacketCapturePossible.store(mOverridePacketCapturePossible);
      } else {
         LuaPacketFunctions::StartPacketCapture(conf);
      }
   }

   bool mOverrideStartPacketCapture;
   bool mOverridePacketCapturePossible;
};
