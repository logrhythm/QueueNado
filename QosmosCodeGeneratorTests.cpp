#include "QosmosCodeGeneratorTests.h"
#include "QosmosCodeGenerator.h"
#include "QosmosCodeGeneratorConfigParser.h"
#include "boost/regex.hpp"
#include <time.h>

using namespace std;
using namespace boost;

TEST_F(QosmosCodeGeneratorTests, construct) {
   QosmosCodeGeneratorConfigParser parser;
   QosmosCodeGeneratorConfigParser parserB(parser);

   QosmosCodeGenerator generator(parser);
   QosmosCodeGenerator generatorB(generator);

   QosmosCodeGeneratorConfigParser* pParser = new QosmosCodeGeneratorConfigParser;
   delete pParser;
}

TEST_F(QosmosCodeGeneratorTests, generatePathEvent) {
   QosmosCodeGeneratorConfigParser parser;

   parser.addStringValue("path");
   vector<string> eventFunctionNames = parser.getAllEventFunctionNames();

   ASSERT_EQ(1, eventFunctionNames.size());
   ASSERT_EQ("path", eventFunctionNames[0]);
}

TEST_F(QosmosCodeGeneratorTests, generateEventCall) {
   MockQosmosCodeGeneratorConfigParser parser;

   MockQosmosCodeGenerator generator(parser);

   string functionName = "foo";

   parser.addStringValue(functionName);
   stringstream dummyBody;
   dummyBody << "boooooo";
   dummyBody << endl;

   string function = generator.GetEventFunction("foo", dummyBody.str());

   ASSERT_TRUE(parensMatch(function));

   stringstream ss(function);
   string line;
   vector<string> lines;
   while (getline(ss, line)) {
      lines.push_back(line);
   }

   ASSERT_TRUE(lines.size() > 7);

   regex r(getCheckForEventName(functionName));
   EXPECT_TRUE(regex_search(lines[0], r));

   r = "[[:space:]]*+\\{[[:space:]]*+";
   EXPECT_TRUE(regex_search(lines[1], r));

   r = getCheckForNullLine();
   EXPECT_TRUE(regex_search(lines[2], r));

   r = "[[:space:]]*+\\{[[:space:]]*+";
   EXPECT_TRUE(regex_search(lines[3], r));

   r = getDpiMsgLRLine();
   EXPECT_TRUE(regex_search(lines[4], r));
   r = "boooooo";
   EXPECT_TRUE(regex_search(lines[5], r));

   r = "[[:space:]]*+\\}[[:space:]]*+";
   EXPECT_TRUE(regex_search(*(lines.end() - 2), r));
   r = "[[:space:]]*+\\}[[:space:]]*+";
   EXPECT_TRUE(regex_search(*(lines.end() - 1), r));

   //cout << function;
}

TEST_F(QosmosCodeGeneratorTests, generateAllEventCallOne) {
   MockQosmosCodeGeneratorConfigParser parser;

   MockQosmosCodeGenerator generator(parser);

   string functionName = "foo";

   ASSERT_FALSE(parser.addStringBodyInfo(functionName, 0, "boooooo", "data_clep_path2str"));
   parser.addStringValue(functionName);

   ASSERT_TRUE(parser.addStringBodyInfo(functionName, 0, "boooooo", "data_clep_path2str"));
   string function = generator.GetAllEventFunctions();

   ASSERT_TRUE(parensMatch(function));

   stringstream ss(function);
   string line;
   vector<string> lines;
   while (getline(ss, line)) {
      lines.push_back(line);
   }
   //cout << function;
   ASSERT_EQ(CALLBACK_FUNCTION_LINES, lines.size());

   regex r(getCheckForEventName(functionName));
   EXPECT_TRUE(regex_search(lines[0], r));

   r = "[[:space:]]*+\\{[[:space:]]*+";
   EXPECT_TRUE(regex_search(lines[1], r));

   r = getCheckForNullLine();
   EXPECT_TRUE(regex_search(lines[2], r));

   r = "[[:space:]]*+\\{[[:space:]]*+";
   EXPECT_TRUE(regex_search(lines[3], r));

   r = getDpiMsgLRLine();
   EXPECT_TRUE(regex_search(lines[4], r));
   r = "boooooo";
   EXPECT_TRUE(regex_search(lines[5], r));
   //cout << lines[5];

   r = "[[:space:]]*+\\}[[:space:]]*+";
   EXPECT_TRUE(regex_search(*(lines.end() - 2), r));
   r = "[[:space:]]*+\\}[[:space:]]*+";
   EXPECT_TRUE(regex_search(*(lines.end() - 1), r));


}

TEST_F(QosmosCodeGeneratorTests, generateAllEventCallMultiple) {
   MockQosmosCodeGeneratorConfigParser parser;

   MockQosmosCodeGenerator generator(parser);
   int totalEvents = 100;
   for (int i = 0; i < totalEvents; i++) {
      stringstream functionNameStream;
      functionNameStream << "name" << i;
      stringstream functionBodyStream;
      functionBodyStream << "body" << i;
      ASSERT_FALSE(parser.addStringBodyInfo(functionNameStream.str(), 0, functionBodyStream.str(), "data_clep_path2str"));
      parser.addStringValue(functionNameStream.str());
      ASSERT_TRUE(parser.addStringBodyInfo(functionNameStream.str(), 0, functionBodyStream.str(), "data_clep_path2str"));

   }

   string function = generator.GetAllEventFunctions();

   ASSERT_TRUE(parensMatch(function));

   stringstream ss(function);
   string line;
   vector<string> lines;
   while (getline(ss, line)) {
      lines.push_back(line);
   }
   //cout << function;
   ASSERT_EQ(CALLBACK_FUNCTION_LINES * totalEvents, lines.size());
   for (int i = 0; i < totalEvents; i++) {
      stringstream functionNameStream;
      functionNameStream << "name" << i;
      stringstream functionBodyStream;
      functionBodyStream << "body" << i;

      regex r(getCheckForEventName(functionNameStream.str()));
      ASSERT_TRUE(regex_search(lines[0 + CALLBACK_FUNCTION_LINES * i], r));

      r = "[[:space:]]*+\\{[[:space:]]*+";
      ASSERT_TRUE(regex_search(lines[1 + CALLBACK_FUNCTION_LINES * i], r));

      r = getCheckForNullLine();
      ASSERT_TRUE(regex_search(lines[2 + CALLBACK_FUNCTION_LINES * i], r));

      r = "[[:space:]]*+\\{[[:space:]]*+";
      ASSERT_TRUE(regex_search(lines[3 + CALLBACK_FUNCTION_LINES * i], r));

      r = getDpiMsgLRLine();
      ASSERT_TRUE(regex_search(lines[4 + CALLBACK_FUNCTION_LINES * i], r));
      r = functionBodyStream.str();
      ASSERT_TRUE(regex_search(lines[5 + CALLBACK_FUNCTION_LINES * i], r));

      r = "[[:space:]]*+\\}[[:space:]]*+";
      ASSERT_TRUE(regex_search(lines[9 + CALLBACK_FUNCTION_LINES * i], r));
      r = "[[:space:]]*+\\}[[:space:]]*+";
      ASSERT_TRUE(regex_search(lines[10 + CALLBACK_FUNCTION_LINES * i], r));
   }


}

string QosmosCodeGeneratorTests::getDpiMsgLRLine() {
   string pattern;
   pattern = "[[:space:]]*+DpiMsgLR[[:space:]]+\\*[[:space:]]*+pDpiMsg";
   pattern += "[[:space:]]*+=[[:space:]]*+";
   pattern += "static_cast<DpiMsgLR[[:space:]]+\\*>\\(";
   pattern += "[[:space:]]*+uapp_cnx->user_handle[[:space:]]*+";
   pattern += "\\)[[:space:]]*+";
   pattern += "\\;[[:space:]]*+";
   return pattern;
}

string QosmosCodeGeneratorTests::getCheckForNullLine() {
   string pattern;
   pattern = "[[:space:]]*+if[[:space:]]*+";
   pattern += "\\([[:space:]]*+";
   pattern += "uapp_cnx->user_handle[[:space:]]*+";
   pattern += "\\!\\=[[:space:]]*+0[[:space:]]*+\\)[[:space:]]*+";
   return pattern;
}

string QosmosCodeGeneratorTests::getCheckForEventName(const string& variableName) {
   string pattern;
   pattern = "void[[:space:]]+event_";
   pattern += variableName;
   pattern += "[[:space:]]*+\\(";
   pattern += "[[:space:]]*+struct";
   pattern += "[[:space:]]+uapp_cnx";
   pattern += "[[:space:]]*+\\*uapp_cnx";
   pattern += "[[:space:]]*+,";
   pattern += "[[:space:]]+const";
   pattern += "[[:space:]]+uevent_t";
   pattern += "[[:space:]]+\\*event";
   pattern += "[[:space:]]*+,";
   pattern += "[[:space:]]*+void";
   pattern += "[[:space:]]+\\*";
   pattern += "[[:space:]]*+\\)[[:space:]]*+";
   return pattern;
}

string QosmosCodeGeneratorTests::getCheckForHiddenVariable(const string& variableName) {
   string pattern;
   pattern = "[[:space:]]*+";
   pattern += variableName;
   pattern += "[[:space:]]*+\\*[[:space:]]*+var1[[:space:]]*+=[[:space:]]*+\\(";
   pattern += variableName;
   pattern += "[[:space:]]*+\\*[[:space:]]*+\\)event->data[[:space:]]*+\\;";
   return pattern;
}

string QosmosCodeGeneratorTests::getCheckForCharBuffer(size_t size) {
   stringstream pattern;
   pattern << "[[:space:]]*+char[[:space:]]+str\\[";
   pattern << size;
   pattern << "\\][[:space:]]*+\\;";
   return pattern.str();
}

string QosmosCodeGeneratorTests::getCheckForConversion(const string& variableName) {
   stringstream pattern;
   pattern << "[[:space:]]*+";
   pattern << variableName;
   pattern << "\\(var1\\,[[:space:]]*+str\\,[[:space:]]*+-1\\)[[:space:]]*+\\;";
   return pattern.str();
}

string QosmosCodeGeneratorTests::getCheckForAssignment(const string& variableName) {
   stringstream pattern;
   pattern << "[[:space:]]*+pDpiMsg->set_";
   pattern << variableName;
   pattern << "\\(str\\)[[:space:]]*+\\;";
   return pattern.str();
}
string QosmosCodeGeneratorTests::getCheckForRepeatedAssignment(const string& variableName) {
   stringstream pattern;
   pattern << "[[:space:]]*+pDpiMsg->add_";
   pattern << variableName;
   pattern << "\\(str\\)[[:space:]]*+\\;[[:space:]]*\\}";
   return pattern.str();
}
string QosmosCodeGeneratorTests::getCheckForAssignmentDeref(const string& variableName) {
   stringstream pattern;
   pattern << "[[:space:]]*+pDpiMsg->set_";
   pattern << variableName;
   pattern << "\\([[:space:]]*+\\*var1[[:space:]]*+\\)[[:space:]]*+\\;";
   return pattern.str();
}
//

TEST_F(QosmosCodeGeneratorTests, parenChecks) {
   QosmosCodeGeneratorConfigParser parser;

   MockQosmosCodeGenerator generator(parser);

   string function = generator.GetEventFunction("event_foo", "(");
   ASSERT_FALSE(parensMatch(function));
   function = generator.GetEventFunction("event_foo", ")");
   ASSERT_FALSE(parensMatch(function));
   function = generator.GetEventFunction("event_foo", "[");
   ASSERT_FALSE(parensMatch(function));
   function = generator.GetEventFunction("event_foo", "]");
   ASSERT_FALSE(parensMatch(function));
   function = generator.GetEventFunction("event_foo", "{");
   ASSERT_FALSE(parensMatch(function));
   function = generator.GetEventFunction("event_foo", "}");
   ASSERT_FALSE(parensMatch(function));
}

TEST_F(QosmosCodeGeneratorTests, AddHandleSafeRequiredBeforeUsed) {
   stringstream ss;
   QosmosCodeGeneratorConfigParser parser;

   MockQosmosCodeGenerator generator(parser);

   generator.AddDpiMsgVariableFrom(ss);
   ASSERT_EQ(0, ss.str().size());
   generator.AddHandleSafe(ss);
   ASSERT_TRUE(ss.str().size() > 0);
   ss.clear();
   generator.AddDpiMsgVariableFrom(ss);
   ASSERT_TRUE(ss.str().size() > 0);
}

TEST_F(QosmosCodeGeneratorTests, CurlyMadness) {
   stringstream ss;
   QosmosCodeGeneratorConfigParser parser;

   MockQosmosCodeGenerator generator(parser);

   generator.AddCloseCurl(ss);
   ASSERT_EQ(0, ss.str().size());
   generator.CloseAllCurl(ss);
   ASSERT_EQ(0, ss.str().size());

   generator.AddOpenCurl(ss);
   EXPECT_TRUE(ss.str().size() > 0);
   ss.str("");
   generator.AddCloseCurl(ss);
   EXPECT_TRUE(ss.str().size() > 0);
   ss.str("");
   generator.AddCloseCurl(ss);
   EXPECT_EQ(0, ss.str().size());
   ss.str("");
   generator.CloseAllCurl(ss);
   EXPECT_EQ(0, ss.str().size());
   ss.str("");


   generator.AddOpenCurl(ss);
   generator.AddOpenCurl(ss);
   ss.str("");
   generator.AddCloseCurl(ss);
   EXPECT_TRUE(ss.str().size() > 0);
   ss.str("");
   generator.CloseAllCurl(ss);
   EXPECT_TRUE(ss.str().size() > 0);
   ss.str("");
}

TEST_F(QosmosCodeGeneratorTests, BodyWithConverterToString) {
   QosmosCodeGeneratorConfigParser parser;

   MockQosmosCodeGenerator generator(parser);
   size_t bufferSize = 51234;
   string hiddenType = "weirdType";
   string converter = "convertWeirdToChar";
   string finalVariableName = "pktpath";

   string functionBody = generator.GetBodyWithConverterToString(bufferSize, hiddenType, converter, finalVariableName);

   ASSERT_TRUE(parensMatch(functionBody));

   stringstream ss(functionBody);
   string line;
   vector<string> lines;
   while (getline(ss, line)) {
      lines.push_back(line);
   }

   ASSERT_TRUE(lines.size() > 3);
   //cout << functionBody;
   regex r(getCheckForHiddenVariable(hiddenType));

   EXPECT_TRUE(regex_search(lines[0], r));

   r = getCheckForCharBuffer(bufferSize);
   EXPECT_TRUE(regex_search(lines[1], r));

   r = getCheckForConversion(converter);
   EXPECT_TRUE(regex_search(lines[2], r));

   r = getCheckForAssignment(finalVariableName);
   //cout << lines[3] << endl;
   EXPECT_TRUE(regex_search(lines[3], r));
}
TEST_F(QosmosCodeGeneratorTests, BodyWithConverterToRepeatedString) {
   QosmosCodeGeneratorConfigParser parser;

   MockQosmosCodeGenerator generator(parser);
   size_t bufferSize = 51234;
   string hiddenType = "weirdType";
   string converter = "convertWeirdToChar";
   string finalVariableName = "pktpath";

   string functionBody = generator.GetBodyWithConverterToRepeatedString(bufferSize, hiddenType, converter, finalVariableName);

   ASSERT_TRUE(parensMatch(functionBody));

   stringstream ss(functionBody);
   string line;
   vector<string> lines;
   while (getline(ss, line)) {
      lines.push_back(line);
   }

   ASSERT_TRUE(lines.size() > 3);
   //cout << functionBody;
   regex r(getCheckForHiddenVariable(hiddenType));

   EXPECT_TRUE(regex_search(lines[0], r));

   r = getCheckForCharBuffer(bufferSize);
   EXPECT_TRUE(regex_search(lines[1], r));

   r = getCheckForConversion(converter);
   EXPECT_TRUE(regex_search(lines[2], r));

   r = getCheckForRepeatedAssignment(finalVariableName);
   //cout << lines[3] << endl;
   EXPECT_TRUE(regex_search(lines[3], r));
}
TEST_F(QosmosCodeGeneratorTests, GetBodyWithConverterToStringDeref) {
   QosmosCodeGeneratorConfigParser parser;

   MockQosmosCodeGenerator generator(parser);
   string hiddenType = "weirdType";
   string finalVariableName = "name";

   string functionBody = generator.GetBodyWithConverterToStringDeref(hiddenType, finalVariableName);

   ASSERT_TRUE(parensMatch(functionBody));

   stringstream ss(functionBody);
   string line;
   vector<string> lines;
   while (getline(ss, line)) {
      lines.push_back(line);
   }

   ASSERT_TRUE(lines.size() == 2);

   regex r(getCheckForHiddenVariable(hiddenType));
   EXPECT_TRUE(regex_search(lines[0], r));

   r = getCheckForAssignmentDeref(finalVariableName);
   EXPECT_TRUE(regex_search(lines[1], r));

}

TEST_F(QosmosCodeGeneratorTests, AutoBodyCreation) {
   QosmosCodeGeneratorConfigParser parser;

   MockQosmosCodeGenerator generator(parser);
   string functionName = "foo";
   size_t bufferSize = 51234;
   string hiddenType = "weirdType";
   string converter = "convertWeirdToChar";

   ASSERT_FALSE(parser.addStringBodyInfo(functionName, bufferSize, hiddenType, converter));
   parser.addStringValue(functionName);
   ASSERT_TRUE(parser.addStringBodyInfo(functionName, bufferSize, hiddenType, converter));
   string functionBody = generator.GetBodyWithConverterToString(functionName);

   ASSERT_TRUE(parensMatch(functionBody));

   stringstream ss(functionBody);
   string line;
   vector<string> lines;
   while (getline(ss, line)) {
      lines.push_back(line);
   }

   ASSERT_TRUE(lines.size() > 3);
   //cout << functionBody;
   regex r(getCheckForHiddenVariable(hiddenType));
   EXPECT_TRUE(regex_search(lines[0], r));

   r = getCheckForCharBuffer(bufferSize);
   EXPECT_TRUE(regex_search(lines[1], r));

   r = getCheckForConversion(converter);
   EXPECT_TRUE(regex_search(lines[2], r));

   r = getCheckForAssignment(functionName);
   //cout << lines[3] << std::endl;
   EXPECT_TRUE(regex_search(lines[3], r));


}

TEST_F(QosmosCodeGeneratorTests, getInfoFailures) {
   QosmosCodeGeneratorConfigParser parser;

   vector<string> noNames = parser.getAllEventFunctionNames();
   ASSERT_TRUE(noNames.empty());
   ASSERT_TRUE(parser.getBodyOfEvent("foo") == "");
   string dummy1, dummy2;
   size_t sdummy1;
   ASSERT_FALSE(parser.getHookInfo("foo", dummy1, dummy2));
   ASSERT_FALSE(parser.getStringBodyInfo("foo", sdummy1, dummy1, dummy2));

}

TEST_F(QosmosCodeGeneratorTests, AddRemoveMacros) {
   QosmosCodeGeneratorConfigParser parser;

   MockQosmosCodeGenerator generator(parser);
   string functionName = "weirdthing";
   string qosmosPath = "Q_DOESNT_EXIST";
   string qosmosName = "Q_NOTHING_HERE";
   ASSERT_FALSE(parser.addHookInfo(functionName, qosmosPath, qosmosName));
   parser.addStringValue(functionName);
   ASSERT_TRUE(parser.addHookInfo(functionName, qosmosPath, qosmosName));

   string macroBody = generator.GetAddAndRemoveMacros();

   ASSERT_TRUE(parensMatch(macroBody));

   stringstream ss(macroBody);
   string line;
   vector<string> lines;
   while (getline(ss, line)) {
      lines.push_back(line);
   }

   ASSERT_TRUE(lines.size() == 6);
   regex r(getCheckForAddHookDefine());
   EXPECT_TRUE(regex_search(lines[0], r));
   r = getCheckForAddHook(functionName, qosmosPath, qosmosName);
   EXPECT_TRUE(regex_search(lines[1], r));
   r = getCheckForRemoveHookDefine();
   EXPECT_TRUE(regex_search(lines[3], r));
   r = getCheckForRemoveHook(functionName, qosmosPath, qosmosName);
   EXPECT_TRUE(regex_search(lines[4], r));
}

TEST_F(QosmosCodeGeneratorTests, AddRemoveMacrosMultiple) {
   QosmosCodeGeneratorConfigParser parser;

   MockQosmosCodeGenerator generator(parser);
   int totalEvents = 100;
   for (int i = 0; i < totalEvents; i++) {
      stringstream functionNameStream;
      functionNameStream << "weirdthing" << i;
      stringstream qosmosPathStream;
      qosmosPathStream << "Q_DOESNT_EXIST" << i;
      stringstream qosmosNameStream;
      qosmosNameStream << "Q_NOTHING_HERE" << i;
      ASSERT_FALSE(parser.addHookInfo(functionNameStream.str(), qosmosPathStream.str(), qosmosNameStream.str()));
      parser.addStringValue(functionNameStream.str());
      ASSERT_TRUE(parser.addHookInfo(functionNameStream.str(), qosmosPathStream.str(), qosmosNameStream.str()));
   }

   string macroBody = generator.GetAddAndRemoveMacros();

   ASSERT_TRUE(parensMatch(macroBody));

   stringstream ss(macroBody);
   string line;
   vector<string> lines;
   while (getline(ss, line)) {
      lines.push_back(line);
   }

   ASSERT_EQ(2 * totalEvents + 4, lines.size());
   int j = 0;
   regex r(getCheckForAddHookDefine());
   ASSERT_TRUE(regex_search(lines[j++], r));

   for (int i = 0; i < (totalEvents); i++) {
      stringstream functionNameStream;
      functionNameStream << "weirdthing" << i;
      stringstream qosmosPathStream;
      qosmosPathStream << "Q_DOESNT_EXIST" << i;
      stringstream qosmosNameStream;
      qosmosNameStream << "Q_NOTHING_HERE" << i;
      r = getCheckForAddHook(functionNameStream.str(), qosmosPathStream.str(), qosmosNameStream.str());
      ASSERT_TRUE(regex_search(lines[j++], r));
   }
   j++;
   r = getCheckForRemoveHookDefine();
   ASSERT_TRUE(regex_search(lines[j++], r));
   for (int i = 0; i < (totalEvents); i++) {

      stringstream functionNameStream;
      functionNameStream << "weirdthing" << i;
      stringstream qosmosPathStream;
      qosmosPathStream << "Q_DOESNT_EXIST" << i;
      stringstream qosmosNameStream;
      qosmosNameStream << "Q_NOTHING_HERE" << i;

      r = getCheckForRemoveHook(functionNameStream.str(), qosmosPathStream.str(), qosmosNameStream.str());
      ASSERT_TRUE(regex_search(lines[j++], r));
   }
   j++;


}

string QosmosCodeGeneratorTests::getCheckForAddHook(const string& functionName, const string& qosmosPath, const string& qosmosName) {
   stringstream pattern;
   pattern << "[[:space:]]*+uevent_hooks_add_parm[[:space:]]*+\\([[:space:]]*+";
   pattern << qosmosPath;
   pattern << "\\,[[:space:]]+";
   pattern << qosmosName;
   pattern << "[[:space:]]*+\\,[[:space:]]*+event_";
   pattern << functionName;
   pattern << "[[:space:]]*+\\,[[:space:]]*+NULL[[:space:]]*+\\)[[:space:]]*+\\;[[:space:]]*+\\\\";
   return pattern.str();
}

string QosmosCodeGeneratorTests::getCheckForAddHookDefine() {
   stringstream pattern;
   pattern << "\\#define[[:space:]]+AUTO_GEN_ADD_HOOK[[:space:]]+\\\\";
   return pattern.str();
}

string QosmosCodeGeneratorTests::getCheckForRemoveHook(const string& functionName, const string& qosmosPath, const string& qosmosName) {
   stringstream pattern;
   pattern << "[[:space:]]*+uevent_hooks_remove_parm[[:space:]]*+\\([[:space:]]*+";
   pattern << qosmosPath;
   pattern << "\\,[[:space:]]+";
   pattern << qosmosName;
   pattern << "[[:space:]]*+\\,[[:space:]]*+event_";
   pattern << functionName;
   pattern << "[[:space:]]*+\\,[[:space:]]*+NULL[[:space:]]*+\\)[[:space:]]*+\\;[[:space:]]*+\\\\";
   return pattern.str();
}

string QosmosCodeGeneratorTests::getCheckForRemoveHookDefine() {
   stringstream pattern;
   pattern << "\\#define[[:space:]]+AUTO_GEN_REMOVE_HOOK[[:space:]]+\\\\";

   return pattern.str();
}

TEST_F(QosmosCodeGeneratorTests, ParseStreamNoComments) {
   QosmosCodeGeneratorConfigParser parser;

   stringstream testStream;

   testStream << "A line with nothing interesting" << endl;
   testStream << "More uninteresting stuff" << endl;

   parser.parseConfigStream(testStream);

   string commentLines = parser.getCommentLines();
   ASSERT_TRUE(commentLines.empty());

}

TEST_F(QosmosCodeGeneratorTests, ParseStreamForQosmosTags) {
   QosmosCodeGeneratorConfigParser parser;

   stringstream testStream;

   string firstComment = "this is the comment";
   string secondComment = "this is a comment with the correct tag for parsing QOSMOS:Q_PROTO_BASE,Q_BASE_PATH,clep_path_t,data_clep_path2str";
   string thirdComment = "QOSMOS:Q_PROTO_BASE,Q_BASE_PATH,clep_path_t";
   string fourthComment = "QOSMOS:Q_PROTO_BASE,Q_BASE_PATH";
   testStream << "A line with nothing interesting" << endl;
   testStream << "More uninteresting stuff" << endl;
   testStream << "This is a line with a comment //" << firstComment << endl;
   testStream << "text type name1 =1  ;//" << firstComment << endl;
   testStream << "text type name2= 2; //" << secondComment << endl;
   testStream << "text type name3= 3; //" << thirdComment << endl;
   testStream << "text type name4= 4; //" << fourthComment << endl;
   testStream << "//" << secondComment << endl;
   testStream << "//" << endl;

   parser.parseConfigStream(testStream);

   string commentLines = parser.getQosmosLines();
   ASSERT_FALSE(commentLines.empty());
   //cout << commentLines << endl;

   stringstream ss(commentLines);
   string line;
   vector<string> lines;
   while (getline(ss, line)) {
      lines.push_back(line);
   }

   ASSERT_EQ(3, lines.size());
   ASSERT_EQ(secondComment, lines[0]);
   ASSERT_EQ(thirdComment, lines[1]);
   ASSERT_EQ(fourthComment, lines[2]);

}

TEST_F(QosmosCodeGeneratorTests, ParseStreamSomeComments) {
   QosmosCodeGeneratorConfigParser parser;

   stringstream testStream;

   string firstComment = "this is the comment";
   testStream << "A line with nothing interesting" << endl;
   testStream << "More uninteresting stuff" << endl;
   testStream << "This is a line with a comment //" << firstComment << endl;
   testStream << "tex_t type nam3 = 2;//" << firstComment << endl;
   testStream << "    text type name=2 ; //" << firstComment << endl; //valid whitespace
   testStream << "text type name=a;//" << firstComment << endl; //codes are digits
   testStream << "text name=1;//" << firstComment << endl; //missing something...
   testStream << "text type name=1//" << firstComment << endl; //missing something...
   testStream << "//" << endl;

   parser.parseConfigStream(testStream);

   string commentLines = parser.getCommentLines();
   ASSERT_FALSE(commentLines.empty());

   stringstream ss(commentLines);
   string line;
   vector<string> lines;
   while (getline(ss, line)) {
      lines.push_back(line);
   }

   ASSERT_EQ(2, lines.size());
   ASSERT_EQ(firstComment, lines[0]);
   ASSERT_EQ(firstComment, lines[1]);


}

TEST_F(QosmosCodeGeneratorTests, ParseQosmosLinesARECommentLines) {
   QosmosCodeGeneratorConfigParser parser;

   stringstream testStream;

   string firstComment = "this is the comment";
   string secondComment = "this is a comment with the correct tag for parsing ParseQosmosLinesARECommentLines QOSMOS:Q_PROTO_BASE,Q_BASE_PATH,clep_path_t,data_clep_path2str";
   testStream << "A line with nothing interesting" << endl;
   testStream << "More uninteresting stuff" << endl;
   testStream << "This is a line with a comment //" << firstComment << endl;
   testStream << "text type name =1  ;//" << firstComment << endl;
   testStream << "text type name= 2; //" << secondComment << endl;
   testStream << "//" << secondComment << endl;
   testStream << "//" << endl;

   parser.parseConfigStream(testStream);

   string commentLines = parser.getCommentLines();
   ASSERT_FALSE(commentLines.empty());

   stringstream ss(commentLines);
   string line;
   vector<string> lines;
   while (getline(ss, line)) {
      lines.push_back(line);
   }

   ASSERT_EQ(1, lines.size());
   ASSERT_EQ(secondComment, lines[0]);
   commentLines = parser.getQosmosLines();
   ASSERT_FALSE(commentLines.empty());

   stringstream qs(commentLines);
   lines.clear();
   while (getline(qs, line)) {
      lines.push_back(line);
   }

   ASSERT_EQ(1, lines.size());
   ASSERT_EQ(secondComment, lines[0]);
}

TEST_F(QosmosCodeGeneratorTests, ParseStreamForQosmosTagsOnStringLine) {
   QosmosCodeGeneratorConfigParser parser;

   stringstream testStream;

   string secondComment = "this is a comment with the correct tag for parsing QOSMOS:Q_PROTO_BASE,Q_BASE_PATH,clep_path_t,data_clep_path2str";
   testStream << "text string name= 2; //" << secondComment << endl;

   parser.parseConfigStream(testStream);
   vector<string> allFunctions = parser.getAllEventFunctionNames();

   ASSERT_FALSE(allFunctions.empty());
   ASSERT_EQ("name", allFunctions[0]);

}

TEST_F(QosmosCodeGeneratorTests, ParseStreamForQosmosTagsOnStringLineNotFound) {
   QosmosCodeGeneratorConfigParser parser;

   stringstream testStream;

   string secondComment = "this is a comment with the correct tag for parsingQOSMOS:Q_PROTO_BASE,Q_BASE_PATH,clep_path_t,data_clep_path2str";
   testStream << "text notype name= 2; //" << secondComment << endl;

   parser.parseConfigStream(testStream);
   vector<string> allFunctions = parser.getAllEventFunctionNames();

   ASSERT_TRUE(allFunctions.empty());

}

TEST_F(QosmosCodeGeneratorTests, ParseStreamForQosmosTagsOnStringLineFullCodeGenerate) {
   QosmosCodeGeneratorConfigParser parser;

   stringstream testStream;

   string functionName = "name";
   string qosmosPath_set = "Q_PROTO_BASE";
   string qosmosVar_set = "Q_BASE_PATH";
   size_t bufferSize_set = 512;
   string hiddenType_set = "clep_path_t";
   string converter_set = "data_clep_path2str";
   string secondComment = "this is a comment with the correct tag for parsing QOSMOS:";
   secondComment += qosmosPath_set;
   secondComment += ",";
   secondComment += qosmosVar_set;
   secondComment += ",";
   secondComment += hiddenType_set;
   secondComment += ",";
   secondComment += converter_set;
   testStream << "text string ";
   testStream << functionName;
   testStream << "= 2; //" << secondComment << endl;

   parser.parseConfigStream(testStream);
   vector<string> allFunctions = parser.getAllEventFunctionNames();

   ASSERT_FALSE(allFunctions.empty());
   ASSERT_EQ(functionName, allFunctions[0]);
   ASSERT_TRUE(parser.getQosmosLines() != "");
   string qosmosPath;
   string qosmosVar;
   size_t bufferSize;
   string hiddenType;
   string converter;
   ASSERT_TRUE(parser.getHookInfo(functionName, qosmosPath, qosmosVar));
   ASSERT_TRUE(parser.getStringBodyInfo(functionName, bufferSize, hiddenType, converter));
   ASSERT_EQ(qosmosPath_set, qosmosPath);
   ASSERT_EQ(qosmosVar_set, qosmosVar);
   ASSERT_EQ(bufferSize_set, bufferSize);
   ASSERT_EQ(hiddenType_set, hiddenType);
   ASSERT_EQ(converter_set, converter);
}

TEST_F(QosmosCodeGeneratorTests, ParseStreamForQosmosTagsOnStringLineFullCodeGenerateNoConverter) {
   QosmosCodeGeneratorConfigParser parser;

   stringstream testStream;

   string functionName = "name";
   string qosmosPath_set = "Q_PROTO_BASE";
   string qosmosVar_set = "Q_BASE_PATH";
   size_t bufferSize_set = 512;
   string secondComment = "this is a comment with the correct tag for parsing QOSMOS:";
   secondComment += qosmosPath_set;
   secondComment += ",";
   secondComment += qosmosVar_set;

   testStream << "text string ";
   testStream << functionName;
   testStream << "= 2; //" << secondComment << endl;

   parser.parseConfigStream(testStream);
   vector<string> allFunctions = parser.getAllEventFunctionNames();

   ASSERT_FALSE(allFunctions.empty());
   ASSERT_EQ(functionName, allFunctions[0]);
   ASSERT_TRUE(parser.getQosmosLines() != "");
   string qosmosPath;
   string qosmosVar;
   size_t bufferSize;
   string hiddenType;
   string converter;
   ASSERT_TRUE(parser.getHookInfo(functionName, qosmosPath, qosmosVar));
   ASSERT_TRUE(parser.getStringBodyInfo(functionName, bufferSize, hiddenType, converter));
   ASSERT_EQ(qosmosPath_set, qosmosPath);
   ASSERT_EQ(qosmosVar_set, qosmosVar);
   ASSERT_EQ(bufferSize_set, bufferSize);
   ASSERT_EQ("const char", hiddenType);
   ASSERT_EQ("", converter);
}

TEST_F(QosmosCodeGeneratorTests, ParseStreamForQosmosTagsOnUIntLineFullCodeGenerate) {
   QosmosCodeGeneratorConfigParser parser;

   stringstream testStream;

   string functionName = "name";
   string qosmosPath_set = "Q_PROTO_IP";
   string qosmosVar_set = "Q_IP_DST_ADDR32";
   string secondComment = "this is a comment with the correct tag for parsing QOSMOS:";
   secondComment += qosmosPath_set;
   secondComment += ",";
   secondComment += qosmosVar_set;

   testStream << "text uint32 ";
   testStream << functionName;
   testStream << "= 2; //" << secondComment << endl;

   parser.parseConfigStream(testStream);
   vector<string> allFunctions = parser.getAllEventFunctionNames();

   ASSERT_FALSE(allFunctions.empty());
   ASSERT_EQ(functionName, allFunctions[0]);
   ASSERT_TRUE(parser.getQosmosLines() != "");
   string qosmosPath;
   string qosmosVar;
   string hiddenType;
   ASSERT_TRUE(parser.getHookInfo(functionName, qosmosPath, qosmosVar));
   ASSERT_TRUE(parser.getGenericBodyInfo(functionName, hiddenType));
   ASSERT_EQ(qosmosPath_set, qosmosPath);
   ASSERT_EQ(qosmosVar_set, qosmosVar);
   ASSERT_EQ("uint32", hiddenType);
}

TEST_F(QosmosCodeGeneratorTests, ParseStreamForQosmosTagsOnUIntLineFullCodeGenerateFixed) {
   QosmosCodeGeneratorConfigParser parser;

   stringstream testStream;

   string functionName = "name";
   string qosmosPath_set = "Q_PROTO_IP";
   string qosmosVar_set = "Q_IP_DST_ADDR32";
   string secondComment = "this is a comment with the correct tag for parsing QOSMOS:";
   secondComment += qosmosPath_set;
   secondComment += ",";
   secondComment += qosmosVar_set;

   testStream << "text fixed32 ";
   testStream << functionName;
   testStream << "= 2; //" << secondComment << endl;

   parser.parseConfigStream(testStream);
   vector<string> allFunctions = parser.getAllEventFunctionNames();

   ASSERT_FALSE(allFunctions.empty());
   ASSERT_EQ(functionName, allFunctions[0]);
   ASSERT_TRUE(parser.getQosmosLines() != "");
   string qosmosPath;
   string qosmosVar;
   string hiddenType;
   ASSERT_TRUE(parser.getHookInfo(functionName, qosmosPath, qosmosVar));
   ASSERT_TRUE(parser.getGenericBodyInfo(functionName, hiddenType));
   ASSERT_EQ(qosmosPath_set, qosmosPath);
   ASSERT_EQ(qosmosVar_set, qosmosVar);
   ASSERT_EQ("uint32", hiddenType);
}

TEST_F(QosmosCodeGeneratorTests, ParseStreamForQosmosTagsOnUInt64LineFullCodeGenerate) {
   QosmosCodeGeneratorConfigParser parser;

   stringstream testStream;

   string functionName = "name";
   string qosmosPath_set = "Q_PROTO_BASE";
   string qosmosVar_set = "Q_UINT64";
   string secondComment = "this is a comment with the correct tag for parsing QOSMOS:";
   secondComment += qosmosPath_set;
   secondComment += ",";
   secondComment += qosmosVar_set;

   testStream << "text uint64 ";
   testStream << functionName;
   testStream << "= 2; //" << secondComment << endl;

   parser.parseConfigStream(testStream);
   vector<string> allFunctions = parser.getAllEventFunctionNames();

   ASSERT_FALSE(allFunctions.empty());
   ASSERT_EQ(functionName, allFunctions[0]);
   ASSERT_TRUE(parser.getQosmosLines() != "");
   string qosmosPath;
   string qosmosVar;
   string hiddenType;
   ASSERT_TRUE(parser.getHookInfo(functionName, qosmosPath, qosmosVar));
   ASSERT_TRUE(parser.getGenericBodyInfo(functionName, hiddenType));
   ASSERT_EQ(qosmosPath_set, qosmosPath);
   ASSERT_EQ(qosmosVar_set, qosmosVar);
   ASSERT_EQ("uint64", hiddenType);
}

TEST_F(QosmosCodeGeneratorTests, ParseStreamForQosmosTagsOnUInt64LineFullCodeGenerateFixed) {
   QosmosCodeGeneratorConfigParser parser;

   stringstream testStream;

   string functionName = "name";
   string qosmosPath_set = "Q_PROTO_BASE";
   string qosmosVar_set = "Q_UINT64";
   string secondComment = "this is a comment with the correct tag for parsing QOSMOS:";
   secondComment += qosmosPath_set;
   secondComment += ",";
   secondComment += qosmosVar_set;

   testStream << "text fixed64 ";
   testStream << functionName;
   testStream << "= 2; //" << secondComment << endl;

   parser.parseConfigStream(testStream);
   vector<string> allFunctions = parser.getAllEventFunctionNames();

   ASSERT_FALSE(allFunctions.empty());
   ASSERT_EQ(functionName, allFunctions[0]);
   ASSERT_TRUE(parser.getQosmosLines() != "");
   string qosmosPath;
   string qosmosVar;
   string hiddenType;
   ASSERT_TRUE(parser.getHookInfo(functionName, qosmosPath, qosmosVar));
   ASSERT_TRUE(parser.getGenericBodyInfo(functionName, hiddenType));
   ASSERT_EQ(qosmosPath_set, qosmosPath);
   ASSERT_EQ(qosmosVar_set, qosmosVar);
   ASSERT_EQ("uint64", hiddenType);
}

TEST_F(QosmosCodeGeneratorTests, ParseStreamForQosmosTagsOnSignedInt32) {
   t_testStream.str("");
   t_testStream << "text sfixed32 ";
   t_testStream << t_functionName;
   t_testStream << "= 2; //" << t_secondComment << endl;
   t_parser.parseConfigStream(t_testStream);
   vector<string> allFunctions = t_parser.getAllEventFunctionNames();
   ASSERT_TRUE(t_parser.getQosmosLines() != "");
   ASSERT_TRUE(t_parser.getGenericBodyInfo(t_functionName, t_hiddenType));
   ASSERT_EQ("int32", t_hiddenType);

   t_testStream.str("");
   t_testStream << "text int32 ";
   t_testStream << t_functionName;
   t_testStream << "= 2; //" << t_secondComment << endl;
   t_parser.parseConfigStream(t_testStream);
   allFunctions = t_parser.getAllEventFunctionNames();
   ASSERT_TRUE(t_parser.getQosmosLines() != "");
   ASSERT_TRUE(t_parser.getGenericBodyInfo(t_functionName, t_hiddenType));
   ASSERT_EQ("int32", t_hiddenType);

   t_testStream.str("");
   t_testStream << "text sint32 ";
   t_testStream << t_functionName;
   t_testStream << "= 2; //" << t_secondComment << endl;
   t_parser.parseConfigStream(t_testStream);
   allFunctions = t_parser.getAllEventFunctionNames();
   ASSERT_TRUE(t_parser.getQosmosLines() != "");
   ASSERT_TRUE(t_parser.getGenericBodyInfo(t_functionName, t_hiddenType));
   ASSERT_EQ("int32", t_hiddenType);
}

TEST_F(QosmosCodeGeneratorTests, ParseStreamForQosmosTagsOnSignedInt64) {
   t_testStream.str("");
   t_testStream << "text int64 ";
   t_testStream << t_functionName;
   t_testStream << "= 2; //" << t_secondComment << endl;
   t_parser.parseConfigStream(t_testStream);
   vector<string> allFunctions = t_parser.getAllEventFunctionNames();
   ASSERT_TRUE(t_parser.getQosmosLines() != "");
   ASSERT_TRUE(t_parser.getGenericBodyInfo(t_functionName, t_hiddenType));
   ASSERT_EQ("int64", t_hiddenType);

   t_testStream.str("");
   t_testStream << "text sint64 ";
   t_testStream << t_functionName;
   t_testStream << "= 2; //" << t_secondComment << endl;
   t_parser.parseConfigStream(t_testStream);
   allFunctions = t_parser.getAllEventFunctionNames();
   ASSERT_TRUE(t_parser.getQosmosLines() != "");
   ASSERT_TRUE(t_parser.getGenericBodyInfo(t_functionName, t_hiddenType));
   ASSERT_EQ("int64", t_hiddenType);

   t_testStream.str("");
   t_testStream << "text sfixed64 ";
   t_testStream << t_functionName;
   t_testStream << "= 2; //" << t_secondComment << endl;
   t_parser.parseConfigStream(t_testStream);
   allFunctions = t_parser.getAllEventFunctionNames();
   ASSERT_TRUE(t_parser.getQosmosLines() != "");
   ASSERT_TRUE(t_parser.getGenericBodyInfo(t_functionName, t_hiddenType));
   ASSERT_EQ("int64", t_hiddenType);
}

TEST_F(QosmosCodeGeneratorTests, ParseStreamForQosmosTagsOnFloat) {
   t_testStream.str("");
   t_testStream << "text float ";
   t_testStream << t_functionName;
   t_testStream << "= 2; //" << t_secondComment << endl;
   t_parser.parseConfigStream(t_testStream);
   vector<string> allFunctions = t_parser.getAllEventFunctionNames();
   ASSERT_TRUE(t_parser.getQosmosLines() != "");
   ASSERT_TRUE(t_parser.getGenericBodyInfo(t_functionName, t_hiddenType));
   ASSERT_EQ("float", t_hiddenType);

}

TEST_F(QosmosCodeGeneratorTests, ParseStreamForQosmosTagsOnDouble) {
   t_testStream.str("");
   t_testStream << "text double ";
   t_testStream << t_functionName;
   t_testStream << "= 2; //" << t_secondComment << endl;
   t_parser.parseConfigStream(t_testStream);
   vector<string> allFunctions = t_parser.getAllEventFunctionNames();
   ASSERT_TRUE(t_parser.getQosmosLines() != "");
   ASSERT_TRUE(t_parser.getGenericBodyInfo(t_functionName, t_hiddenType));
   ASSERT_EQ("double", t_hiddenType);

}

TEST_F(QosmosCodeGeneratorTests, ParseStreamForQosmosTagsOnBool) {
   t_testStream.str("");
   t_testStream << "text bool ";
   t_testStream << t_functionName;
   t_testStream << "= 2; //" << t_secondComment << endl;
   t_parser.parseConfigStream(t_testStream);
   vector<string> allFunctions = t_parser.getAllEventFunctionNames();
   ASSERT_TRUE(t_parser.getQosmosLines() != "");
   ASSERT_TRUE(t_parser.getGenericBodyInfo(t_functionName, t_hiddenType));
   ASSERT_EQ("bool", t_hiddenType);

}

