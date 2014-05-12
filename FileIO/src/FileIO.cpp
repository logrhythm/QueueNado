/* 
 * File:   FileIO.cpp
 * Author: kjell/weberr13
 * 
 * https://github.com/weberr13/FileIO
 * Created on August 15, 2013, 2:28 PM
 */



#include "FileIO.h"
#include <mutex>
#include <sstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctime>
#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <cstring>

namespace FileIO {
   std::mutex mPermissionsMutex;

   
   
   /**
    * Reads the stats for the path to see if it is a mountpoint
    * @param pathToDirectory that may be a mountpoint
    * @param returns Result<true> if it is a mountpoint
    */
   Result<bool> IsMountPoint(const std::string& pathToDirectory) { 
      struct stat info;
      if (0 != stat(pathToDirectory.c_str(), &info)) {
         return Result<bool>{false, {"Cannot stat read location: " + pathToDirectory}};
      }
      
      if (!S_ISDIR(info.st_mode)) {
         return Result<bool>{false, {"Directory + [" + pathToDirectory + "] does not exist"}};
      }       
      
      std::string parentPath = pathToDirectory;
      if('/' != parentPath.back()){
         parentPath.append("/..");
      } else {
         parentPath.append("..");   
      }
     
      struct stat parent; 
      if (0 != stat(parentPath.c_str(), &parent)) {
         return Result<bool>{false, {"Cannot stat read parent location: [" + parentPath +"]"}};
      }
      
      //  st_dev: device number. st_ino: inode number
      const bool isMountPoint = (info.st_dev != parent.st_dev) || (info.st_dev == parent.st_dev && info.st_ino == parent.st_ino);
      if (!isMountPoint) {
         return Result<bool>{false, {"[" + parentPath + "] is not a mountpoint"}};
      }
      
      return Result<bool>{true};
   }

   
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

   /**
    * Use stat to determine the presence of a directory
    * @param pathToFile
    * @return if the stat command succeeded (meaning that there is a directory that directory name)
    */
   bool DoesDirectoryExist(const std::string& pathToDirectory) {
      struct stat directoryInfo;
      if (0 != stat(pathToDirectory.c_str(), &directoryInfo)) {
         return false;
      }
      bool isDirectory = S_ISDIR(directoryInfo.st_mode);
      return isDirectory;
   }

   
   
/**
    * Iterate through the directory. Remove any file found,  save any found directory
    * Any attempt to delete files from "/"or "/root" will be ignored.
    * 
    * 
    * @param location to delete files from
    * @param return by reference number of files deleted
    * @param return by reference any found directory
    * @return whether or not all the operations were successful
    */
   Result<bool> CleanDirectoryOfFileContents(const std::string& location
           , size_t& filesRemoved, std::vector<std::string>& foundDirectories) {
      if (("/" == location) || ("/root" == location) || ("/root/" == location)) {
         return Result<bool>{false, {"Not allowed to remove directory: " + location}};
      }

      if (location.empty() || !FileIO::DoesDirectoryExist(location)) {
         return Result<bool>{false, {"Directory does not exist. False location was: " + location}};
      }

      FileIO::DirectoryReader reader(location);
      if (reader.Valid().HasFailed()) {
         return Result<bool>{false, {"Failed to read directory: " + location + ". Error: " + reader.Valid().error}};;
      }

      FileIO::DirectoryReader::Entry entry;
      size_t failures{0};
      filesRemoved = 0;
      std::string lastError;
      do {
         entry = reader.Next();
         if (FileIO::FileType::Directory == entry.first) {
            std::string dirPath{location};
            if ('/' != location.back()) {
               dirPath.append("/");
            }
            dirPath.append(entry.second);
            foundDirectories.push_back(dirPath);
         } else if (FileIO::FileType::File == entry.first) {
            const std::string pathToFile{location + "/" + entry.second};
            bool removedFile = (0 == unlink(pathToFile.c_str()));
            if (removedFile) {
               filesRemoved++;
            } else {
               ++failures;
               lastError = {"Last Error for file: " + pathToFile + ", errno: " };
               lastError.append(std::strerror(errno));
            }
         }
         // FileIO::FileSystem::Unknown is ignored
      } while (entry.first != FileIO::FileType::End);

      std::string report;
      if (failures > 0) {
         report = {"#" + std::to_string(failures) + " number of failed removals. " + lastError};
      }
      return Result<bool>{(0 == failures), report};
   }
   
   
   

/**
    * Remove directories at the given paths
    * @return whether or not all the operations were successful
    */
   Result<bool> RemoveEmptyDirectories(const std::vector<std::string>& fullPathDirectories) {
      std::ostringstream error;
      error << "Failed to remove given directories : ";
      bool success = true;
      for (const auto& directory : fullPathDirectories) {
         if (FileIO::DoesDirectoryExist(directory)) { // invalids are ignored. 
            int removed = rmdir(directory.c_str());
            if (0 != removed) {
               success = false;
               error << "\n" << directory << error << ", error: " << std::strerror(errno);               
            }
         }
      }
      
      if (!success) {
         return Result<bool>{false, error.str()};
      }
      
      return Result<bool>{true};
   }
   
   
   Result<bool> ChangeFileOrDirOwnershipToUser(const std::string& path, const std::string& username) {

      std::lock_guard<std::mutex> lock(mPermissionsMutex);
      auto previuousuid = setfsuid(-1);
      auto previuousgid = setfsgid(-1);
      setfsuid(0);
      setfsgid(0);
      struct passwd* pwd = GetUserFromPasswordFile(username);
      auto returnVal = chown(path.c_str(), pwd->pw_uid, pwd->pw_gid);
      free(pwd);

      if (returnVal < 0) {
         std::string error{"Cannot chown dir/file: "};
         error.append(path);
         error.append(" error number: ");
         error.append(std::to_string(errno));
         error.append(" current fs permissions are for uid: ");
         error.append(std::to_string(setfsuid(-1)));
         setfsuid(previuousuid);
         setfsgid(previuousgid);
         return Result<bool>{false, error};
      }
      setfsuid(previuousuid);
      setfsgid(previuousgid);
      return Result<bool>{true};
   }

   struct passwd* GetUserFromPasswordFile(const std::string& username) {
      // Get the uid for dpi user
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
    * When running as root, change the file system access to a user.
    */
   void SetUserFileSystemAccess(const std::string& username) {
      struct passwd* pwd = GetUserFromPasswordFile(username);

      setfsuid(pwd->pw_uid);
      setfsgid(pwd->pw_gid);

      free(pwd);

   }

   Result<bool> RemoveFileAsRoot(const std::string& filename) {

      std::lock_guard<std::mutex> lock(mPermissionsMutex);
      auto previuousuid = setfsuid(-1);
      auto previuousgid = setfsgid(-1);
      setfsuid(0);
      setfsgid(0);
      int rc = unlink(filename.c_str());
      setfsuid(previuousuid);
      setfsgid(previuousgid);

      if (rc == -1) {
         return Result<bool>{false, "Unable to unlink file"};
      }
      return Result<bool>{true};
   }

   /**
    * Helper lambda to instantiate the const Result<bool> AFTER
    * the opendir call.
    */
   namespace {
      /// @return the success of the opendir operation
      auto DirectoryInit = [](DIR** directory, const std::string pathToDirectory) -> Result<bool> {
         *directory = opendir(pathToDirectory.c_str());
          std::string error{""};

         if (nullptr == *directory) {
            std::string error{std::strerror(errno)};
            return Result<bool>{false, error};
         }
         return Result<bool>(true);
      };
   } // anonymous helper

   
   DirectoryReader::DirectoryReader(const std::string& pathToDirectory)
   : mDirectory {nullptr }
   , mValid{DirectoryInit(&mDirectory, pathToDirectory)}
   {}

   DirectoryReader::~DirectoryReader() {
      closedir(mDirectory);
   }

   /**
    * Finds the next entry in the directory or returns it. 
    * The type of the entry is returned. The type corresponds to dirent.h d_type
    * 
    * The only filesystem types supported are file and directory, all other will be classified as 
    * TypeFound::Unknown
    * 
    * The directories "." and ".." are skipped
    * 
    * @return pair of name of entry and type. If end is reached "TypeFound::End" is returned.
    *
    */
   DirectoryReader::Entry DirectoryReader::Next() {
      static const std::string Ignore1{"."};
      static const std::string Ignore2{".."};

      Entry entry = std::make_pair(FileType::Unknown, "");
      bool found = false;
      while (!found && (entry.first != FileType::End)) {
         //auto ignoredError = readdir64_r(mDirectory, &mEntry, &mResult); // readdir_r is reentrant 

         found = true; // abort immediately unless we hit "." or ".."

         if (nullptr == mResult) {
            entry = std::make_pair(FileType::End, "");
         } else if (static_cast<unsigned char> (FileType::Directory) == mEntry.d_type) {
            std::string name{mEntry.d_name};
            if ((Ignore1 == name) || (Ignore2 == name)) {
               found = false;
            } else {
               entry = std::make_pair(FileType::Directory, std::move(name));
            }
         } else if (static_cast<unsigned char> (FileType::File) == mEntry.d_type) {
            entry = std::make_pair(FileType::File, mEntry.d_name);
         } else {
            // Default case. Unless continue was called it will always exit here
            entry = std::make_pair(FileType::Unknown, "");
         }
      }
      return entry;
   }
   /** Resets the position of the directory stream to the beginning of the directory */
   void DirectoryReader::Reset() {
      rewinddir(mDirectory);
   }
} // namespace FileIO

