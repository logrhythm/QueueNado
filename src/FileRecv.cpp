#include <cstdlib>
#include <cstdint>
#include <memory>
#include <czmq.h>
#include <g2log.hpp>

#include "FileRecv.h"

FileRecv::FileRecv(): 
mQueueLength(10),
mTimeout(30000), //5 minutes
mOffset(0),
mSize(0),
mData(nullptr),
mChunk(nullptr) {
	mCtx = zctx_new();
	CHECK(mCtx);
	mDealer = zsocket_new(mCtx, ZMQ_DEALER);
	CHECK(mDealer);
	mCredit = mQueueLength;
}

int FileRecv::SetLocation(const std::string& location){
	return zsocket_connect(mDealer, location.c_str());
}

void FileRecv::SetTimeout(const int timeout){
    mTimeout = timeout;
}


void FileRecv::RequestChunks(){
	// Send enough data requests to fill pipeline:
	while (mCredit) {
        zstr_sendf (mDealer, "%ld", mOffset);
        mOffset += 1;
        mCredit--;
    }	
}

size_t FileRecv::Monitor(){
	
	//Erase any previous data from the last Monitor()
	FreeChunk();
    FreeData();

    RequestChunks();

    //Poll to see if anything is available on the pipeline:
    if(zsocket_poll(mDealer, mTimeout)){

    	mChunk = zframe_recv (mDealer);
    	if(!mChunk) {
    		//Interrupt or end of stream
    		return -1;
    	}

    	mSize = zframe_size (mChunk);
    	if(mSize <= 0){
    		//End of Stream
    		return 0;
    	}
    	
    	mData = (uint8_t *) malloc(mSize);	
		memcpy(mData, (void*) zframe_data(mChunk), mSize);

		mCredit++;
    	return mSize;

    } else {
    	//timeout
    	return -2;
    }
}

void FileRecv::FreeChunk(){
	if(mChunk != nullptr){
		zframe_destroy(&mChunk);
		mChunk = nullptr;
	}
}

void FileRecv::FreeData(){
	if(mData != nullptr){
		free(mData);
		mData = nullptr;
	}
}

uint8_t* FileRecv::GetChunkData(){
	return mData;
}

size_t FileRecv::GetChunkSize(){
	return mSize;
}

FileRecv::~FileRecv(){
	FreeChunk();
    FreeData();
	zsocket_destroy(mCtx, mDealer);
    zctx_destroy(&mCtx);
}