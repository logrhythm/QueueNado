#pragma once

#include "DiskPacketCapture.h"
#include <string>

class MockDiskPacketCapture : public DiskPacketCapture {
public:

   MockDiskPacketCapture() : DiskPacketCapture() {
   }

   virtual ~MockDiskPacketCapture() {
   }

   std::vector<std::pair<struct pcap_pkthdr*, uint8_t*> >* GetRunningPackets(const std::string& uuid) {
      return DiskPacketCapture::GetRunningPackets(uuid);
   }

   void RemoveOldestPCapFile(Conf& conf) {
      DiskPacketCapture::RemoveOldestPCapFile(conf);
   }

   void RemoveFromRunningPackets(const std::string& uuid) {
      DiskPacketCapture::RemoveFromRunningPackets(uuid);
   }

   bool TooMuchPCap(Conf& conf) {
      return DiskPacketCapture::TooMuchPCap(conf);
   }

   std::string BuildFilename(Conf& conf, const std::string& uuid,
           const std::string& appName, const std::time_t time) {
      return DiskPacketCapture::BuildFilename(conf, uuid, appName, time);
   }
};

