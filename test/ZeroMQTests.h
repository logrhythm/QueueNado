#pragma once

#include "gtest/gtest.h"
#include "ZeroMQ.h"
#include <pthread.h>
#include <sys/time.h>
#include <iostream>
#include "boost/thread/locks.hpp"
#include "boost/thread/recursive_mutex.hpp"
#include "ReceivePacketZMQ.h"

#ifndef PACKETS_TO_TEST
#define PACKETS_TO_TEST 400000
#endif
#ifndef THREAD_TEST_PACKETS
#define THREAD_TEST_PACKETS 100
#endif
#ifndef TEST_THREADS
#define TEST_THREADS 10
#endif

/// Defines a packet to be passed to afc_process function.
/// This is instead of including lots of our ThirdParty Engine header files
typedef struct data_pkt {
   struct data_pkt *next;
   void *user_handle;
   void *buffer;
   struct timeval timestamp;
   unsigned int len;
   unsigned char *data;
} data_pkt_t;
typedef struct data_pkt *data_ppacket;



class ZeroMQTests : public ::testing::Test
{
public:
    ZeroMQTests() : mPacketsSeen(0),mPacketsToTest(1),t_smallData(NULL), t_bigData(NULL), t_bigDataSize(0)  {}
    virtual ~ZeroMQTests() {
    	if (t_smallData){
    		delete[] t_smallData;
    	}
    	if (t_bigData) {
    		delete[] t_bigData;
    	}
    }
    void PacketBroadcasterReceiver(ZeroMQ <void*>* clientQueue, data_ppacket targetPacket) {
    	if (!clientQueue->SendClientReady()) {
    		std::cout << "Could not notify server" << std::endl;
    		return;
    	}
    	while (mPacketsToTest > mPacketsSeen) {
    		data_ppacket packet = (data_ppacket)clientQueue->GetPointer(1);
    		if (packet == NULL) {
    		    boost::this_thread::sleep(boost::posix_time::microseconds(1));
    		} else {
    			ASSERT_EQ(targetPacket,packet); // SAME ADDRESS!!!
    		    mPacketsSeen++;
    	    }
    	}
    	return;

    }
    int mPacketsSeen;
    int mPacketsToTest;
    static int gCurrentPacketSize;
protected:
	virtual void SetUp() {
		srandom ((unsigned) time (NULL));
	}
	virtual void TearDown() {}

	void MakeSmallData() {
		if (t_smallData)
			delete[] t_smallData;
		t_smallData = new u_char[16];
		for (int i = 0 ; i < 4 ; i ++) {
			long data = random();
			t_smallData[i*4] = (data >> 24) & 0xff;
			t_smallData[i*4+1] = (data >> 16) & 0xff;
			t_smallData[i*4+2] = (data >> 8) & 0xff;
			t_smallData[i*4+3] = (data) & 0xff;
		}
	}
	void MakeBigData(size_t size) {
		t_bigDataSize = size - (size%4);
		if (t_bigData)
			delete[] t_bigData;
		t_bigData = new u_char[t_bigDataSize];
		for (unsigned int i = 0 ; i < t_bigDataSize/4 ; i ++) {
			long data = random();
			t_bigData[i*4] = (data >> 24) & 0xff;
			t_bigData[i*4+1] = (data >> 16) & 0xff;
			t_bigData[i*4+2] = (data >> 8) & 0xff;
			t_bigData[i*4+3] = (data) & 0xff;
		}
	}
	void MakePacketsData() {
		long bigNumber = 1500;
		t_bigDataSize = bigNumber - (bigNumber%4);
		if (t_bigData)
			delete[] t_bigData;
		t_bigData = new u_char[t_bigDataSize];
		for (unsigned int i = 0 ; i < t_bigDataSize/4 ; i ++) {
			long data = random();
			t_bigData[i*4] = (data >> 24) & 0xff;
			t_bigData[i*4+1] = (data >> 16) & 0xff;
			t_bigData[i*4+2] = (data >> 8) & 0xff;
			t_bigData[i*4+3] = (data) & 0xff;
		}
	}
    void SetExpectedTime(size_t numberOfPackets, size_t packetSize, size_t RateInMbps, size_t transactionsPerSecond) {
    	size_t totalBytes = (numberOfPackets * packetSize);
    	t_totalTransactions = numberOfPackets;
    	t_packetSize = packetSize;
    	t_targetTransactionsPerSecond = transactionsPerSecond;
    	std::cout << std::dec <<  "Total MBytes: " << totalBytes/(1024*1024) << std::endl;
    	t_expectedTime =  ((totalBytes*1.0) / (RateInMbps/8.0)) ;

    	std::cout << "Expected Time :" << t_expectedTime/1000000.0 << "s at "<< RateInMbps << "Mbps" <<std::endl;
    }
    void StartTimedSection() {
       gettimeofday(&t_startTime,NULL);
    }
    void EndTimedSection() {
       gettimeofday(&t_endTime,NULL);
       t_elapsedUS = (t_endTime.tv_sec-t_startTime.tv_sec)*1000000L + (t_endTime.tv_usec - t_startTime.tv_usec);
       std::cout <<std::dec << "Elapsed Time :" << t_elapsedUS/1000000L << "." << std::setfill('0') << std::setw(6) <<  t_elapsedUS % 1000000<<  "s" << std::endl;
       double totalTransactionsPS = (t_totalTransactions*1.0)/(t_elapsedUS*1.0/1000000.0);
       std::cout << "Transactions/second :" << totalTransactionsPS  << " at ";
       double totalSpeed = (t_packetSize)*totalTransactionsPS/(125000.0);
       std::cout << totalSpeed << "Mbps" << std::endl ;
    }
    bool TimedSectionPassed() {
    //return true;
    	if (10000 > PACKETS_TO_TEST) {
    		return true;
    	}
        return (t_elapsedUS < t_expectedTime) || ((t_totalTransactions / (t_elapsedUS/1000000.0)) > t_targetTransactionsPerSecond) ;
    }

	std::string t_data;
	uLong t_header;
	u_char* t_smallData;
	u_char* t_bigData;
	size_t t_bigDataSize;
	pthread_t t_workerHandle;
	int t_totalTransactions;
	int t_targetTransactionsPerSecond;
	double t_expectedTime;
	time_t t_elapsedUS;
	timeval t_startTime;
	size_t t_packetSize;
	timeval t_endTime;
	boost::recursive_mutex mMutex;
};


