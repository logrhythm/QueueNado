#pragma once

#include "gtest/gtest.h"
#include <pthread.h>
#include <sys/time.h>
#include <iostream>
#include <sstream>
#include <string>
#include "QosmosCodeGenerator.h"
#include "QosmosCodeGeneratorConfigParser.h"
#define CALLBACK_FUNCTION_LINES 15

class QosmosCodeGeneratorTests : public ::testing::Test {
public:

   QosmosCodeGeneratorTests() :
   t_functionName("name"),
   t_qosmosPath_set("Q_PROTO_BASE"),
   t_qosmosVar_set("Q_UINT64"),
   t_secondComment("this is a comment with the correct tag for parsing QOSMOS:") {
   }

   virtual ~QosmosCodeGeneratorTests() {
   }

protected:

   virtual void SetUp() {
      srandom((unsigned) time(NULL));

      t_secondComment += t_qosmosPath_set;
      t_secondComment += ",";
      t_secondComment += t_qosmosVar_set;
      t_testStream.str("");
   }

   virtual void TearDown() {
   }

   bool parensMatch(std::string str) {
      int parens = 0;
      int curly = 0;
      int brackets = 0;

      for (auto i = str.begin(); i != str.end(); i++) {
         switch (*i) {
            case '{':
               curly++;
               break;
            case '}':
               curly--;
               break;
            case '(':
               parens++;
               break;
            case ')':
               parens--;
               break;
            case '[':
               brackets++;
               break;
            case ']':
               brackets--;
               break;
         }
      }
      return (brackets == 0 && parens == 0 && curly == 0);
   }
   std::string getDpiMsgLRLine();
   std::string getCheckForNullLine();
   std::string getCheckForEventName(const std::string& variableName);
   std::string getCheckForHiddenVariable(const std::string& variableName);
   std::string getCheckForCharBuffer(size_t size);
   std::string getCheckForConversion(const std::string& variableName);
   std::string getCheckForAssignment(const std::string& variableName);
   std::string getCheckForRepeatedAssignment(const std::string& variableName);
   std::string getCheckForAssignmentDeref(const std::string& variableName);
   std::string getCheckForAddHook(const std::string& functionName, const std::string& qosmosPath, const std::string& qosmosName);
   std::string getCheckForRemoveHook(const std::string& functionName, const std::string& qosmosPath, const std::string& qosmosName);
   std::string getCheckForAddHookDefine();
   std::string getCheckForRemoveHookDefine();

   std::stringstream t_testStream;
   std::string t_functionName;
   std::string t_qosmosPath_set;
   std::string t_qosmosVar_set;
   std::string t_secondComment;
   std::string t_qosmosPath;
   std::string t_qosmosVar;
   std::string t_hiddenType;
   QosmosCodeGeneratorConfigParser t_parser;
};

class MockQosmosCodeGeneratorConfigParser : public QosmosCodeGeneratorConfigParser {
public:

   std::string ConstructEventName(const std::string& value) {
      return QosmosCodeGeneratorConfigParser::ConstructEventName(value);
   }

};

class MockQosmosCodeGenerator : public QosmosCodeGenerator {
public:

   explicit MockQosmosCodeGenerator(QosmosCodeGeneratorConfigParser& configuration) :
   QosmosCodeGenerator(configuration) {
   }

   virtual std::string GetBodyWithConverterToString(const std::string& functionName) {
      return QosmosCodeGenerator::GetBodyWithConverterToString(functionName);
   }

   virtual std::string GetEventFunction(const std::string& variableName, const std::string& functionBody) {
      return QosmosCodeGenerator::GetEventFunction(variableName, functionBody);
   }

   virtual std::string GetBodyWithConverterToString(size_t bufferSize, const std::string& hiddenType, const std::string& converter, const std::string& finalVariableName) {
      return QosmosCodeGenerator::GetBodyWithConverterToString(bufferSize, hiddenType, converter, finalVariableName);
   }

   virtual std::string GetBodyWithConverterToStringDeref(const std::string& hiddenType, const std::string& finalVariableName) {
      return QosmosCodeGenerator::GetBodyWithConverterToStringDeref(hiddenType, finalVariableName);
   }

   virtual std::string GetEventFunction(const std::string& variableName) {
      return QosmosCodeGenerator::GetEventFunction(variableName);
   }

   virtual void AddHandleSafe(std::stringstream& stream) {
      QosmosCodeGenerator::AddHandleSafe(stream);
   }

   virtual void AddDpiMsgVariableFrom(std::stringstream& stream) {
      QosmosCodeGenerator::AddDpiMsgVariableFrom(stream);
   }

   virtual void AddOpenCurl(std::stringstream& stream) {
      QosmosCodeGenerator::AddOpenCurl(stream);
   }

   virtual void AddCloseCurl(std::stringstream& stream) {
      QosmosCodeGenerator::AddCloseCurl(stream);
   }

   virtual void CloseAllCurl(std::stringstream& stream) {
      QosmosCodeGenerator::CloseAllCurl(stream);
   }
   std::string GetBodyWithConverterToRepeatedString(size_t bufferSize,const std::string& hiddenType, const std::string& converter,const std::string& finalVariableName) {
      return QosmosCodeGenerator::GetBodyWithConverterToRepeatedString(bufferSize, hiddenType, converter, finalVariableName);
   }
};
