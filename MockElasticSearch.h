#pragma once
#include "ElasticSearch.h"
#include "MockBoomStick.h"
#include "include/global.h"
class BoomStick;

class MockElasticSearch : public ElasticSearch {
public:

   MockElasticSearch() : mMyTransport(""), ElasticSearch(mMyTransport), mFakeIndexList(true),
   mFakeDeleteIndex(true), mFakeDeleteValue(true){
      mMockListOfIndexes.insert("kibana-int");
      mMockListOfIndexes.insert("network_1999_01_01");
      mMockListOfIndexes.insert("network_2012_12_31");
      mMockListOfIndexes.insert("network_2013_01_01");
      mMockListOfIndexes.insert("network_2013_07_04");
      mMockListOfIndexes.insert("network_2013_12_31");
      mMockListOfIndexes.insert("network_2014_01_01");
      mMockListOfIndexes.insert("network_2100_12_31");
      mMockListOfIndexes.insert("twitter");
   }

   virtual ~MockElasticSearch() {

   }

   LR_VIRTUAL std::set<std::string> GetListOfIndexeNames() override {
      if (mFakeIndexList) {
         return mMockListOfIndexes;
      }
      return ElasticSearch::GetListOfIndexeNames();
   }

   LR_VIRTUAL bool CreateIndex(const std::string& indexName, int shards, int replicas)override {
      return ElasticSearch::CreateIndex(indexName, shards, replicas);
   }

   LR_VIRTUAL bool DeleteIndex(const std::string& indexName) override {
      if (mFakeDeleteIndex) {
         return mFakeDeleteValue;
      }
      ElasticSearch::DeleteIndex(indexName);
   }

   LR_VIRTUAL bool IndexClose(const std::string& indexName) override {
      ElasticSearch::IndexClose(indexName);
   }

   LR_VIRTUAL bool IndexOpen(const std::string& indexName) override {
      ElasticSearch::IndexOpen(indexName);
   }

   LR_VIRTUAL bool AddDoc(const std::string& indexName, const std::string& indexType, const std::string& id, const std::string& jsonData)override {
      ElasticSearch::AddDoc(indexName, indexType, id, jsonData);
   }

   LR_VIRTUAL bool DeleteDoc(const std::string& indexName, const std::string& indexType, const std::string& id)override {
      ElasticSearch::DeleteDoc(indexName, indexType, id);
   }
   MockBoomStick mMyTransport;
   std::set<std::string> mMockListOfIndexes;
   bool mFakeIndexList;
   bool mFakeDeleteIndex;
   bool mFakeDeleteValue;
};