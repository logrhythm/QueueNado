/* 
 * File:   Result.h
 * Author: kjell
 *
 * Created on April 21
 */
#pragma once
#include <string>


/**
 * Example Usage: 
 * return Result<bool> success{true};
 * or in case of a failure
 * return Result<bool> failure{false, error};
 */
template<typename T> struct Result {
   const T result;
   const std::string error;

   /**
    * Result of an operation. 
    * @param output whatever the expected output would be
    * @param err error message to the client, default is empty which means successful operation
    */
   Result(T output, const std::string& err = {""})
   : result(output), error(err) {
   }

   /** @return status whether or not the Result contains a failure*/
   bool HasFailed() {
      return (!error.empty());
   }
};