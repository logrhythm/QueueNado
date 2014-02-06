/* 
 * File:   MockProtoDefaults.h
 * Author: kjell
 *
 * Created on February 6, 2014, 10:22 AM
 */

#pragma once
#include "ProtoDefaults.h"
#include <utility>
#include <string>
#include <memory>
#include "Range.h"
#include "ConfTypeMsg.pb.h"

class MockProtoDefaults : public ProtoDefaults {
public:

   explicit MockProtoDefaults(const std::vector<std::string>& pcapLocations)
   : ProtoDefaults(pcapLocations) {

      ProtoDefaults::gPcapDiskReadSuccess = false; // trigger a new read
   }

   ~MockProtoDefaults() {
      // RAII: Force the next usage of ProtoDefaults to reset so it does no longer
      // have any of these previous settings.
      ProtoDefaults::gPcapDiskReadSuccess = false;
   }

   void UpdateCache() {
      ProtoDefaults::UpdateCache();
   }

   bool WasPcapDiskReadSuccessful() {
      return  ProtoDefaults::gPcapDiskReadSuccess;
   }
};




