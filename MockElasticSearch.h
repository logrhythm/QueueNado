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
   RunQueryGetIdsAlwaysPasses(false), RunQueryGetIdsAlwaysFails(false), mBulkUpdateResult(false),
   mRealSendAndExpectOkAndAck(true),
   mRealSendAndExpectOkAndFound(true),
   mRealSendAndForgetCommandToWorker(true),
   mRealSendAndGetReplyCommandToWorker(true),
   mRealGetDiskInfo(true),
   mRanSendAndExpectOkAndAck(false),
   mRanSendAndExpectOkAndFound(false),
   mRanSendAndForgetCommandToWorker(false),
   mRanSendAndGetReplyCommandToWorker(false),
   mRanGetDiskInfo(false),
   mReturnSendAndExpectOkAndAck(false),
   mReturnSendAndExpectOkAndFound(false),
   mReturnSendAndForgetCommandToWorker(false),
   mReturnSendAndGetReplyCommandToWorker(false),
   mReturnGetDiskInfo(false),
   mFakeBulkUpdate(true),
   mFakeGetOldestNFiles(true) {
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
   RunQueryGetIdsAlwaysPasses(false), RunQueryGetIdsAlwaysFails(false),
   mRealSendAndExpectOkAndAck(true),
   mRealSendAndExpectOkAndFound(true),
   mRealSendAndForgetCommandToWorker(true),
   mRealSendAndGetReplyCommandToWorker(true),
   mRealGetDiskInfo(true),
   mRanSendAndExpectOkAndAck(false),
   mRanSendAndExpectOkAndFound(false),
   mRanSendAndForgetCommandToWorker(false),
   mRanSendAndGetReplyCommandToWorker(false),
   mRanGetDiskInfo(false) {
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
   bool Initialize() {
      return ElasticSearch::Initialize();
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
      return ElasticSearch::DeleteIndex(indexName);
   }

   LR_VIRTUAL bool IndexClose(const std::string& indexName) {
      return ElasticSearch::IndexClose(indexName);
   }

   LR_VIRTUAL bool IndexOpen(const std::string& indexName) {
      return ElasticSearch::IndexOpen(indexName);
   }

   LR_VIRTUAL bool AddDoc(const std::string& indexName, const std::string& indexType, const std::string& id, const std::string& jsonData) {
      return ElasticSearch::AddDoc(indexName, indexType, id, jsonData);
   }

   LR_VIRTUAL bool UpdateDoc(const std::string& indexName, const std::string& indexType, const std::string& id, const std::string& jsonData) {
      if (mFailUpdateDoc) {
         return false;
      }
      if (mUpdateDocAlwaysPasses) {
         return true;
      }
      return ElasticSearch::UpdateDoc(indexName, indexType, id, jsonData);
   }

   LR_VIRTUAL bool DeleteDoc(const std::string& indexName, const std::string& indexType, const std::string& id) {
      return ElasticSearch::DeleteDoc(indexName, indexType, id);
   }

   bool ReplySent() {
      return mSocketClass->mReplySent;
   }

   void ReplySet(const bool set) {
      mSocketClass->mReplySent = set;
   }

   bool RunQueryGetIds(const std::string& indexName, const std::string& query,
           std::vector<std::pair<std::string, std::string> >& recordsToUpdate, const int recordsToQuery = 40,
           const bool cache = NO_CACHE) {
      if (RunQueryGetIdsAlwaysPasses) {
         return true;
      }
      if (RunQueryGetIdsAlwaysFails) {
         return false;
      }
      if (mQueryIdResults.empty()) {
         return ElasticSearch::RunQueryGetIds(indexName, query, recordsToUpdate, recordsToQuery, cache);
      }
      recordsToUpdate = mQueryIdResults;
      return true;
   }

   std::vector<std::tuple< std::string, std::string> > GetOldestNFiles(const unsigned int numberOfFiles,
           const std::string& path, IdsAndIndexes& relevantRecords, time_t& oldestTime) {
      if (mFakeGetOldestNFiles) {
         oldestTime = mOldestTime;
         return mOldestFiles;
      }
      return ElasticSearch::GetOldestNFiles(numberOfFiles, path, relevantRecords, oldestTime);
   }

   bool BulkUpdate(const IdsAndIndexes& idsAndIndex, const std::string& indexType, const std::string& jsonData) {
      if (mFakeBulkUpdate) {
         return mBulkUpdateResult;
      }
      return ElasticSearch::BulkUpdate(idsAndIndex, indexType, jsonData);
   }

   bool SendAndExpectOkAndAck(const std::string& command) {
      mRanSendAndExpectOkAndAck = true;
      if (mRealSendAndExpectOkAndAck) {
         return ElasticSearch::SendAndExpectOkAndAck(command);
      }
      return mReturnSendAndExpectOkAndAck;
   }

   bool SendAndForgetCommandToWorker(const std::string& command) {
      mRanSendAndForgetCommandToWorker = true;
      if (mRealSendAndForgetCommandToWorker) {
         return ElasticSearch::SendAndForgetCommandToWorker(command);
      }
      return mReturnSendAndForgetCommandToWorker;
   }

   bool SendAndGetReplyCommandToWorker(const std::string& command, std::string& reply) {
      mRanSendAndGetReplyCommandToWorker = true;
      if (mRealSendAndGetReplyCommandToWorker) {
         return ElasticSearch::SendAndGetReplyCommandToWorker(command, reply);
      }
      reply = mSendAndGetReplyReply;
      return mReturnSendAndGetReplyCommandToWorker;
   }

   bool GetDiskInfo(DiskInformation& diskInfo) {
      mRanGetDiskInfo = true;
      if (mRealGetDiskInfo) {
         return ElasticSearch::GetDiskInfo(diskInfo);
      }
      return false;
   }

   void BoostDiskInfo() {
      /*typedef std::map<std::string, std::string>*/
      DiskInfoStrings diskInfo;
      /*typedef std::map<std::string, uint64_t>*/
      DiskInfoIntegers diskInts;
      /*typedef std::tuple< DiskInfoStrings, DiskInfoIntegers>*/
      SingleDiskInfo foo(diskInfo, diskInts);

      /*typedef std::map<std::string, SingleDiskInfo >*/
      for (int i = 0; i < 100; i++) {
         mDiskInfo[std::to_string(i)] = foo;
      }
   }
   int HiddenWorkerSend(zmsg_t** message) {
      return zmsg_send(message, mWorkerThread);
   }
   void SetSocketTimeout(const size_t newTimeout) {
      mSocketTimeout = newTimeout;
   }
   bool OptimizeIndex(const std::string& index) {
      mOptimizedIndexes.insert(index);
      return ElasticSearch::OptimizeIndex(index);
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
   time_t mOldestTime;
   bool mBulkUpdateResult;
   std::vector<std::tuple< std::string, std::string> > mOldestFiles;
   bool mRealSendAndExpectOkAndAck;
   bool mRealSendAndExpectOkAndFound;
   bool mRealSendAndForgetCommandToWorker;
   bool mRealSendAndGetReplyCommandToWorker;
   bool mRealGetDiskInfo;
   bool mRanSendAndExpectOkAndAck;
   bool mRanSendAndExpectOkAndFound;
   bool mRanSendAndForgetCommandToWorker;
   bool mRanSendAndGetReplyCommandToWorker;
   bool mRanGetDiskInfo;
   bool mReturnSendAndExpectOkAndAck;
   bool mReturnSendAndExpectOkAndFound;
   bool mReturnSendAndForgetCommandToWorker;
   bool mReturnSendAndGetReplyCommandToWorker;
   bool mReturnGetDiskInfo;
   bool mFakeBulkUpdate;
   bool mFakeGetOldestNFiles;
   std::string mSendAndGetReplyReply;
   std::set<std::string> mOptimizedIndexes;

};
#endif