/* 
 * File:   FileSystemWalker.h
 * Author: kjell
 *
 * Created on April 21, 2014
 */

#pragma once
#include <string>
#include <functional>
#include <fts.h>
#include "Result.h"

/*
 * FileSystemWalker is used in a similar way to the c-libraries "ftw, nftw, nftw64" but is thread-safe. 
 */
class FileSystemWalker {
public:
   FileSystemWalker(const std::string& startPath, std::function<int(FTSENT*, int ftstype_flag) > ftsHandler);
   bool IsValid() const;
   Result<int> Action();

   FileSystemWalker() = delete;
   FileSystemWalker(const FileSystemWalker&) = delete;
   FileSystemWalker& operator=(const FileSystemWalker&) = delete;   
private:
   std::function<int(FTSENT*, int) > mFtsHandler;
   const std::string mStartPath;
};

