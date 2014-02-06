/* 
 * File:   UpgradeCommandTest.h
 * Author: vrecan
 *
 * Created on April 3, 2013, 10:51 AM
 */

#pragma once

#include "UpgradeCommand.h"
#include "Command.h"

class UpgradeCommandTest : public UpgradeCommand {
public:
   using UpgradeCommand::CreatePassPhraseFile;
   using UpgradeCommand::DecryptFile;
   using UpgradeCommand::RenameDecryptedFile;
   using UpgradeCommand::UntarFile;
   using UpgradeCommand::RunUpgradeScript;
   using UpgradeCommand::CleanUploadDir;

   UpgradeCommandTest(const protoMsg::CommandRequest& request, ProcessClient& processManager) 
   : UpgradeCommand(request, processManager) {  }
   
   virtual ~UpgradeCommandTest() { }
   void SetBadPassPhraseFilePath() {
      mUploadDir = "/doesNot/Exist";
   }
};