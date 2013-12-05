#pragma once
#include "ElasticSearch.h"
#include "MockElasticSearchSocket.h"
#include "DiskPacketCapture.h"
#include "MockBoomStick.h"
#include "gmock/gmock.h"
#include "BoolReturns.h"
#include "include/global.h"
class BoomStick;
#ifdef LR_DEBUG

class MockElasticSearch : public ElasticSearch {
public:

   static std::string CreateFileNameWithPath(const std::vector<std::string>& paths, const std::string& id) {
      return DiskPacketCapture::BuildFilenameWithPath(paths, id);
   }

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

   void UpdateSessions(const std::set<std::string>& oldestSessionIds,
                                                              const unsigned int maxPerQuery, const time_t& indexStartTime) {
      ElasticSearch::UpdateSessions(oldestSessionIds, maxPerQuery,indexStartTime);
   }
   LR_VIRTUAL bool IndexActuallyExists(const std::string& indexName) {
      return ElasticSearch::IndexActuallyExists(indexName);
   }
   LR_VIRTUAL std::string ConstructSearchHeaderWithTime(const time_t& timeSince) {
      return ElasticSearch::ConstructSearchHeaderWithTime(timeSince);
   }
   LR_VIRTUAL bool GetLatestDateOfUpgradeWhereIndexesShouldBeIgnored(time_t& ignoreTime) {
      return ElasticSearch::GetLatestDateOfUpgradeWhereIndexesShouldBeIgnored(ignoreTime);
   }
   
   void SetValidNames(const std::set<std::string>& names) {
      ElasticSearch::mKnownIndexNames = names;
      ElasticSearch::mLastIndexListUpdateTime = std::time(NULL);
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

   PathAndFileNames GetOldestNFiles(const unsigned int numberOfFiles,
           const std::vector<std::string>& paths, ElasticSearch::ConstructPathWithFilename fileConstructor, IdsAndIndexes& relevantRecords, time_t& oldestTime, size_t& totalHits) {
      if (mFakeGetOldestNFiles) {
         oldestTime = mOldestTime;
         return mOldestFiles;
      }

      return ElasticSearch::GetOldestNFiles(numberOfFiles, paths, fileConstructor, relevantRecords, oldestTime, totalHits);
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
      mSocketTimeoutMs = newTimeout;
   }

   bool OptimizeIndex(const std::string& index) {
      mOptimizedIndexes.insert(index);
      return ElasticSearch::OptimizeIndex(index);
   }

   bool OptimizeIndexes(const std::set<std::string>& allIndexes,
           const std::set<std::string> excludes) LR_OVERRIDE {
      ElasticSearch::OptimizeIndexes(allIndexes, excludes);
   }

   std::set<std::string> GetIndexesThatAreActive() {
      return ElasticSearch::GetIndexesThatAreActive();
   }

   LR_VIRTUAL bool RunOptimize() {
      return ElasticSearch::RunOptimize();
   }

   LR_VIRTUAL bool UpdateIgnoreTimeInternally() {
      return ElasticSearch::UpdateIgnoreTimeInternally();
   }

   unsigned int GetTimesSinceLastUpgradeCheck() {
      return ElasticSearch::mTimesSinceLastUpgradeCheck;
   }

   time_t GetUpgradeIgnoreTime() {
      return ElasticSearch::mIgnoreTime;
   }

   LR_VIRTUAL std::string GetIgnoreTimeAsString(const size_t& ignoreTime) {
      return ElasticSearch::GetIgnoreTimeAsString(ignoreTime);
   }
   
   LR_VIRTUAL std::string GetListOfAllIndexesSince(time_t indexStartTime) {
      return ElasticSearch::GetListOfAllIndexesSince(indexStartTime);
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
   PathAndFileNames mOldestFiles;
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
using ::testing::_;
using ::testing::Invoke;
using ::testing::Return;
using ::testing::Throw;
using ::testing::SetArgReferee;
using ::testing::DoAll;

class GMockElasticSearch : public MockElasticSearch {
public:

   GMockElasticSearch(bool async) : MockElasticSearch(async), mBogusTime(0), realObject(async) {
      ON_CALL(*this, Initialize()).WillByDefault(Invoke(&realObject, &ElasticSearch::Initialize));
      ON_CALL(*this, GetLatestDateOfUpgradeWhereIndexesShouldBeIgnored(_)).WillByDefault(Invoke(&realObject, &ElasticSearch::GetLatestDateOfUpgradeWhereIndexesShouldBeIgnored));
      ON_CALL(*this, SendAndGetReplyCommandToWorker(_, _)).WillByDefault(Invoke(&realObject, &MockElasticSearch::SendAndGetReplyCommandToWorker));
   };

   GMockElasticSearch(BoomStick& transport, bool async) : MockElasticSearch(transport, async), realObject(transport, async) {
      ON_CALL(*this, Initialize()).WillByDefault(Invoke(&realObject, &ElasticSearch::Initialize));
      ON_CALL(*this, GetLatestDateOfUpgradeWhereIndexesShouldBeIgnored(_)).WillByDefault(Invoke(&realObject, &ElasticSearch::GetLatestDateOfUpgradeWhereIndexesShouldBeIgnored));
      ON_CALL(*this, SendAndGetReplyCommandToWorker(_, _)).WillByDefault(Invoke(&realObject, &MockElasticSearch::SendAndGetReplyCommandToWorker));
   };

   MOCK_METHOD0(Initialize, bool());
   MOCK_METHOD1(GetLatestDateOfUpgradeWhereIndexesShouldBeIgnored, bool(time_t&));
   MOCK_METHOD6(GetOldestNFiles, PathAndFileNames(const unsigned int numberOfFiles,
           const std::vector<std::string>& paths, ElasticSearch::ConstructPathWithFilename fileConstructor,
           IdsAndIndexes& relevantRecords, time_t& oldestTime, size_t& totalHits));
   MOCK_METHOD2(SendAndGetReplyCommandToWorker, bool (const std::string& command, std::string& reply));

   void DelegateInitializeToAlwaysFail() {
      ON_CALL(*this, Initialize())
              .WillByDefault(Invoke(&returnBools, &BoolReturns::ReturnFalse));
   }

   void DelegateGetOldestNFiles(const PathAndFileNames& bogusFileList, const IdsAndIndexes& bogusIdsAndIndex, const time_t bogusTime) {
      mBogusFileList = bogusFileList;
      mBogusIdsAndInddex = bogusIdsAndIndex;
      mBogusTime = bogusTime;
      EXPECT_CALL(*this, GetOldestNFiles(_, _, _, _, _, _))
              .WillRepeatedly(DoAll(SetArgReferee<3>(mBogusIdsAndInddex), SetArgReferee<4>(mBogusTime), Return(mBogusFileList)));
   }

   void DelegateSendAndGetReplyCommandToWorkerFails() {

      ON_CALL(*this, SendAndGetReplyCommandToWorker(_, _))
              .WillByDefault(Return(false));
   }

   PathAndFileNames mBogusFileList;
   IdsAndIndexes mBogusIdsAndInddex;
   time_t mBogusTime;
   BoolReturns returnBools;
private:
   MockElasticSearch realObject;
};

class GMockElasticSearchNoSend : public MockElasticSearch {
public:

   GMockElasticSearchNoSend(bool async) : MockElasticSearch(async) {
      ON_CALL(*this, SendAndGetReplyCommandToWorker(_, _)).
              WillByDefault(Return(false));
   };

   GMockElasticSearchNoSend(BoomStick& transport, bool async) : MockElasticSearch(transport, async) {
      ON_CALL(*this, SendAndGetReplyCommandToWorker(_, _)).
              WillByDefault(Return(false));
   };

   MOCK_METHOD2(SendAndGetReplyCommandToWorker, bool (const std::string& command, std::string& reply));

};
#endif