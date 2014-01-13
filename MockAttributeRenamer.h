#pragma once

#include "AttributeRenamerTest.h"

class MockAttributeRenamer : public AttributeRenamer {
public:
      
   MockAttributeRenamer() : AttributeRenamer() {}
   bool Initialize(const std::string& pathToRenameYaml) {
      bool success;
      ProtectedInitialize(pathToRenameYaml,success);
      return success;
   }
};

