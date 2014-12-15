#include <cstdlib>
#include <cstdint>
#include <memory>
#include <czmq.h>
#include <g2log.hpp>

#include "FileSend.h"

FileSend::FileSend():
mLocation(""), 
mQueueLength(10),
mNextChunk(nullptr), 
mNextChunkId(0), 
mIdentity(nullptr), 
mTimeout(30000), //5 Minutes
mChunk(nullptr) {
	mCtx = zctx_new();
	CHECK(mCtx);
	mRouter = zsocket_new(mCtx, ZMQ_ROUTER);
	CHECK(mRouter);
}

int FileSend::SetLocation(const std::string& location){
	mLocation = location;
	zsocket_set_hwm(mRouter, mQueueLength * 2);

    return zsocket_bind(mRouter, mLocation.c_str());
}

void FileSend::SetTimeout(const int timeout){
	mTimeout = timeout;
}

void FileSend::FreeOldRequests(){
	if(mIdentity != nullptr){
		zframe_destroy(&mIdentity);
		mIdentity = nullptr;
	}
	if(mNextChunk != nullptr){
		free(mNextChunk);
		mNextChunk = nullptr;
	}
}

void FileSend::FreeChunk(){
	if(mChunk != nullptr){
		zframe_destroy(&mChunk);
		mChunk = nullptr;
	}
}

int FileSend::NextChunkId(){

	FreeChunk();
	FreeOldRequests();

	//Poll to see if anything is available on the pipeline:
    if(zsocket_poll(mRouter, mTimeout)){

    	// First frame is the identity of the client
		mIdentity = zframe_recv (mRouter);
		if (!mIdentity) {
			//Interrupt or client has terminated
			return -1;
		}

	} else {
		return -2;
	}

	//Poll to see if anything is available on the pipeline:
    if(zsocket_poll(mRouter, mTimeout)){

		// Second frame is next chunk requested of the file
    	mNextChunk = zstr_recv (mRouter);
    	if(!mNextChunk){
    		//Interrupt or client has terminated
    		return -1;
    	}
    	
    	mNextChunkId = atoi(mNextChunk);
    	return mNextChunkId;

    } else {
    	//timeout
    	return -2;
    }
}

int FileSend::SendData(uint8_t* data, size_t size){
	
	FreeChunk();
	FreeOldRequests();

	size_t next = NextChunkId();
	if(next < 0){
		//Interrupt or client has terminated
		return next;
	}

    mChunk = zframe_new(data, size);

    // Send chunk to client
    zframe_send (&mIdentity, mRouter, ZFRAME_REUSE + ZFRAME_MORE);
    zframe_send (&mChunk, mRouter, 0);

	return 0;
}

FileSend::~FileSend(){
	zsocket_unbind(mRouter, mLocation.c_str());
	zsocket_destroy(mCtx, mRouter);
    zctx_destroy(&mCtx);
    FreeOldRequests();
    FreeChunk();
}