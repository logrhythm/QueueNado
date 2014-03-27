/* 
 * File:   FileIO.h
 * Author: kjell/weberr13
 *
 * Created on August 15, 2013, 2:28 PM
 */

#pragma once
#include <string>
#include <fstream>

#include <sys/fsuid.h>
#include <unistd.h>
#include <pwd.h>
namespace FileIO {   
   template<typename T> struct Result {
      const T result;
      const std::string error;
      /**
       * Result of a FileIO operation. 
       * @param output whatever the expected output would be
       * @param err error message to the client, default is empty which means successful operation
       */
      Result(T output, const std::string& err = {""}) 
      : result(output), error(err){}      
      
      /** @return status whether or not the Result contains a failure*/
      bool HasFailed() { return (!error.empty());}
   };
   
   Result<std::string> ReadAsciiFileContent(const std::string& pathToFile); 
   Result<bool> WriteAsciiFileContent(const std::string& pathToFile, const std::string& content);
   Result<bool> AppendWriteAsciiFileContent(const std::string& pathToFile, const std::string& content);
   Result<bool> WriteFileContentInternal(const std::string& pathToFile, const std::string& content, std::ios_base::openmode mode);
   bool DoesFileExist(const std::string& pathToFile);
   Result<bool> RemoveFileAsRoot(const std::string& filename);
   struct passwd* GetUserFromPasswordFile(const std::string& username);
   void SetUserFileSystemAccess(const std::string& username);
}



