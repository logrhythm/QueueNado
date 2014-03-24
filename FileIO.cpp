/* 
 * File:   FileIO.cpp
 * Author: kjell/weberr13
 * 
 * Created on August 15, 2013, 2:28 PM
 */

#include "FileIO.h"
#include <fstream>
/**
 * We should NOT use any LOGGING in this file.
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
namespace FileIO {

      /**
    * Reads content of Ascii file
    * @param pathToFile to read
    * @return Result<std::string> all the content of the file, and/or an error string 
    *         if something went wrong 
    */
   Result<std::string> ReadAsciiFileContent(const std::string& pathToFile) {
      std::ifstream in(pathToFile, std::ios::in);
      if (!in) {
         std::string error{"Cannot read-open file: "};
         error.append(pathToFile);
         return Result<std::string>{
            {}, error
         };
      }

      std::string contents;
      in.seekg(0, std::ios::end);
      auto end = in.tellg();

      // Attempt to read it the fastest way possible
      if (-1 != end) {
         contents.resize(end);
         in.seekg(0, std::ios::beg);
         in.read(&contents[0], contents.size());
         in.close();
         return Result<std::string>{contents};
      }
      // Could not calculate with ifstream::tellg(). Is it a RAM file? 
      // Fallback solution to slower iteratator approach
      contents.assign((std::istreambuf_iterator<char>(in)),
              (std::istreambuf_iterator<char>()));
      in.close();
      return Result<std::string>{contents};
   }

   /**
    * A generic write function that supports a variety of modes
    * @param pathToFile
    * @param content
    * @param mode, std::ios::app or std::ios::trunc
    * @return 
    */
   Result<bool> WriteFileContentInternal(const std::string& pathToFile, const std::string& content, std::ios_base::openmode mode) {
      std::ofstream out(pathToFile, std::ios::out | mode);
      if (!out) {
         std::string error{"Cannot write-open file: "};
         error.append(pathToFile);
         return Result<bool>{false, error};
      }

      out << content;
      out.close();
      return Result<bool>{true};
   }

      /**
    * Write ascii content to file
    * @param pathToFile to write
    * @param content to write to file
    * @return Result<bool> result if operation whent OK, if it did not the Result<bool>::error string 
    *         contains the error message
    */
   Result<bool> WriteAsciiFileContent(const std::string& pathToFile, const std::string& content) {

      return WriteFileContentInternal(pathToFile, content, std::ios::trunc);
   }

      /**
    * Write ascii content to the end of a file
    * @param pathToFile to write
    * @param content to write to file
    * @return Result<bool> result if operation whent OK, if it did not the Result<bool>::error string 
    *         contains the error message
    */
   Result<bool> AppendWriteAsciiFileContent(const std::string& pathToFile, const std::string& content) {
      return WriteFileContentInternal(pathToFile, content, std::ios::app);
   }

   /**
    * Use stat to determine the presence of a file
    * @param pathToFile
    * @return if the stat command succeeded (meaning that there is something at that filename)
    */
   bool DoesFileExist(const std::string& pathToFile) {
      struct stat fileInfo;

      return (stat(pathToFile.c_str(), &fileInfo) == 0);
   }

   struct passwd* GetDpiPasswd() {
      // Get the uid for dpi user
      static const std::string username("dpi");
      struct passwd* pwd = (struct passwd *) calloc(1, sizeof (struct passwd));
      if (pwd == NULL) {
         // Failed to allocate struct passwd for getpwnam_r.
         exit(1);
      }
      size_t buffer_len = sysconf(_SC_GETPW_R_SIZE_MAX) * sizeof (char);
      char *buffer = (char *) malloc(buffer_len);
      if (buffer == NULL) {
         //Failed to allocate buffer for getpwnam_r.
         exit(2);
      }
      getpwnam_r(username.c_str(), pwd, buffer, buffer_len, &pwd);
      if (pwd == NULL) {
         //getpwnam_r failed to find requested entry.
         exit(3);
      }
      free(buffer);
      return pwd;
   }

   /*
    * When running as root, change the file system access to user dpi.
    */
   void SetDpiFileSystemAccess() {
      struct passwd* pwd = GetDpiPasswd();

      // Set file system access to dpi for owner and group. Only on Linux.
      setfsuid(pwd->pw_uid);
      setfsgid(pwd->pw_gid);

      free(pwd);

   }

   Result<bool> RemoveFileAsRoot(const std::string& filename) {
      
      setfsuid(0);
      setfsgid(0);
      int rc = unlink(filename.c_str());
      FileIO::SetDpiFileSystemAccess();
      if (rc == -1) {
         return Result<bool>{false,"Unable to unlink file"};
      } 
      return Result<bool>{true};
   }

}
