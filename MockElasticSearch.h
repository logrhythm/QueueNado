#pragma once
#include "ElasticSearch.h"
#include "MockElasticSearchSocket.h"
#include "MockBoomStick.h"
#include "include/global.h"
class BoomStick;
#ifdef LR_DEBUG

class MockElasticSearch : public ElasticSearch {
public:

   MockElasticSearch(bool async) : mMyTransport(""), ElasticSearch(mMyTransport, async), mFakeIndexList(true),
   mFakeDeleteIndex(true), mFakeDeleteValue(true), mFailAddDoc(false), mAddDocAlwaysPasses(true) {
      mMockListOfIndexes.insert("kibana-int");
      mMockListOfIndexes.insert("network_1999_01_01");
      mMockListOfIndexes.insert("network_2012_12_31");
      mMockListOfIndexes.insert("network_2013_01_01");
      mMockListOfIndexes.insert("network_2013_07_04");
      mMockListOfIndexes.insert("network_2013_12_31");
      mMockListOfIndexes.insert("network_2014_01_01");
      mMockListOfIndexes.insert("network_2100_12_31");
      mMockListOfIndexes.insert("twitter");
      delete std::get<1>(*mWorkerArgs);
      mSocketClass = new MockElasticSearchSocket(mTransport, mAsynchronous);
      std::get<1>(*mWorkerArgs) = mSocketClass;
   }

   MockElasticSearch(BoomStick& transport, bool async) : mMyTransport(""), ElasticSearch(transport, async), mFakeIndexList(true),
   mFakeDeleteIndex(true), mFakeDeleteValue(true), mFailAddDoc(false), mAddDocAlwaysPasses(true) {
      mMockListOfIndexes.insert("kibana-int");
      mMockListOfIndexes.insert("network_1999_01_01");
      mMockListOfIndexes.insert("network_2012_12_31");
      mMockListOfIndexes.insert("network_2013_01_01");
      mMockListOfIndexes.insert("network_2013_07_04");
      mMockListOfIndexes.insert("network_2013_12_31");
      mMockListOfIndexes.insert("network_2014_01_01");
      mMockListOfIndexes.insert("network_2100_12_31");
      mMockListOfIndexes.insert("twitter");
      delete std::get<1>(*mWorkerArgs);
      mSocketClass = new MockElasticSearchSocket(mTransport, mAsynchronous);
      std::get<1>(*mWorkerArgs) = mSocketClass;
   }

   virtual ~MockElasticSearch() {

   }

   LR_VIRTUAL std::set<std::string> GetListOfIndexeNames() {
      if (mFakeIndexList) {
         return mMockListOfIndexes;
      }
      return ElasticSearch::GetListOfIndexeNames();
   }

   LR_VIRTUAL bool CreateIndex(const std::string& indexName, int shards, int replicas) {
      return ElasticSearch::CreateIndex(indexName, shards, replicas);
   }

   LR_VIRTUAL bool DeleteIndex(const std::string& indexName) {
      if (mFakeDeleteIndex) {
         return mFakeDeleteValue;
      }
      ElasticSearch::DeleteIndex(indexName);
   }

   LR_VIRTUAL bool IndexClose(const std::string& indexName) {
      ElasticSearch::IndexClose(indexName);
   }

   LR_VIRTUAL bool IndexOpen(const std::string& indexName) {
      ElasticSearch::IndexOpen(indexName);
   }

   LR_VIRTUAL bool AddDoc(const std::string& indexName, const std::string& indexType, const std::string& id, const std::string& jsonData) {
      if (mFailAddDoc) {
         return false;
      }
      if (mAddDocAlwaysPasses) { 
         return true;
      }
      ElasticSearch::AddDoc(indexName, indexType, id, jsonData);
   }

   LR_VIRTUAL bool DeleteDoc(const std::string& indexName, const std::string& indexType, const std::string& id) {
      ElasticSearch::DeleteDoc(indexName, indexType, id);
   }

   bool ReplySent() {
      return mSocketClass->mReplySent;
   }

   void ReplySet(const bool set) {
      mSocketClass->mReplySent = set;
   }
   std::vector<std::pair<std::string, std::string> > 
         RunQueryGetIds(const std::string& indexType, const std::string& query) {
      if (mQueryIdResults.empty()) {
         return ElasticSearch::RunQueryGetIds(indexType,query);
      }
      return mQueryIdResults;
   }
   
   MockBoomStick mMyTransport;
   std::set<std::string> mMockListOfIndexes;
   bool mFakeIndexList;
   bool mFakeDeleteIndex;
   bool mFakeDeleteValue;
   MockElasticSearchSocket* mSocketClass;
   bool mFailAddDoc;
   bool mAddDocAlwaysPasses;
   std::vector<std::pair<std::string, std::string> > mQueryIdResults;

};
#endif