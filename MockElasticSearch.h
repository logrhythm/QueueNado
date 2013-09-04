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
   mFakeDeleteIndex(true), mFakeDeleteValue(true), mFailUpdateDoc(false), mUpdateDocAlwaysPasses(true),
           RunQueryGetIdsAlwaysPasses(false),RunQueryGetIdsAlwaysFails(false) {
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
      mSocketClass = new MockElasticSearchSocket(mTransport, IsAsynchronous());
      std::get<1>(*mWorkerArgs) = mSocketClass;
   }

   MockElasticSearch(BoomStick& transport, bool async) : mMyTransport(""), ElasticSearch(transport, async), mFakeIndexList(true),
   mFakeDeleteIndex(true), mFakeDeleteValue(true), mFailUpdateDoc(false), mUpdateDocAlwaysPasses(true), 
   RunQueryGetIdsAlwaysPasses(false),RunQueryGetIdsAlwaysFails(false) {
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
      mSocketClass = new MockElasticSearchSocket(mTransport, IsAsynchronous());
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
      ElasticSearch::AddDoc(indexName, indexType, id, jsonData);
   }
   LR_VIRTUAL bool UpdateDoc(const std::string& indexName, const std::string& indexType, const std::string& id, const std::string& jsonData) {
      if (mFailUpdateDoc) {
         return false;
      }
      if (mUpdateDocAlwaysPasses) { 
         return true;
      }
      ElasticSearch::UpdateDoc(indexName, indexType, id, jsonData);
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
   
   bool RunQueryGetIds(const std::string& indexType, const std::string& query, 
         std::vector<std::pair<std::string, std::string> > & matches) {
      if (RunQueryGetIdsAlwaysPasses) {
         return true;
      }
      if (RunQueryGetIdsAlwaysFails) {
         return false;
      }
      if (mQueryIdResults.empty()) {
         return ElasticSearch::RunQueryGetIds(indexType,query,matches);
      }
      matches = mQueryIdResults;
      return true;
   }
   
   MockBoomStick mMyTransport;
   std::set<std::string> mMockListOfIndexes;
   bool mFakeIndexList;
   bool mFakeDeleteIndex;
   bool mFakeDeleteValue;
   MockElasticSearchSocket* mSocketClass;
   bool mFailUpdateDoc;
   bool mUpdateDocAlwaysPasses;
   bool RunQueryGetIdsAlwaysPasses;
   std::vector<std::pair<std::string, std::string> > mQueryIdResults;
   bool RunQueryGetIdsAlwaysFails;

};
#endif