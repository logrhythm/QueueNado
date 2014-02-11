/* 
 * File:   UpgradeCommandTest.h
 * Author: vrecan
 *
 * Created on April 3, 2013, 10:51 AM
 */

#pragma once

#include "UpgradeCommand.h"
#include "Command.h"
#include "CommandFailedException.h"

class UpgradeCommandTest : public UpgradeCommand {
public:

   using UpgradeCommand::DecryptFile;
   using UpgradeCommand::RenameDecryptedFile;
   using UpgradeCommand::UntarFile;
   using UpgradeCommand::RunUpgradeScript;
   using UpgradeCommand::CleanUploadDir;

   UpgradeCommandTest(const protoMsg::CommandRequest& request, ProcessClient& processManager, bool noThrowForCreatePassPhraseFile = false) 
   : UpgradeCommand(request, processManager),  mNoThrowForCreatePassPhraseFile(noThrowForCreatePassPhraseFile){  }


   void CreatePassPhraseFile() LR_OVERRIDE {
      if (false == mNoThrowForCreatePassPhraseFile) {
        UpgradeCommand::CreatePassPhraseFile();
        return;
      }
      
      // Ignore any exceptions at this stage. ProcessManager is not involved
      // when we are writing to file so we are not interested in any failures 
      // from :  FileIO::WriteAsciiFileContent(file,content);
      try { 
         UpgradeCommand::CreatePassPhraseFile();
      } catch(CommandFailedException& e) {
         // ignored
         return;
      }
   }

   virtual ~UpgradeCommandTest() { }
   void SetBadPassPhraseFilePath() {
      mUploadDir = "/doesNot/Exist";
   }
   
   bool mNoThrowForCreatePassPhraseFile;
};
