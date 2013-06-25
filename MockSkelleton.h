#pragma once
#include "Skelleton.h"
#include <atomic>
#include <thread>
#ifdef LR_DEBUG

class MockSkelleton : public Skelleton {
public:

   explicit MockSkelleton(const std::string& binding) : Skelleton(binding), mRepeating(false) {
   }

   virtual ~MockSkelleton() {
   }

   bool Initialize() override {
      mContext = zctx_new();
      if (!mContext) {
         return false;
      }
      mFace = zsocket_new(mContext, ZMQ_ROUTER);
      if (!mFace) {
         zctx_destroy(&mContext);
         mFace = NULL;
         return false;
      }
      if (zsocket_bind(mFace, mBinding.c_str()) < 0) {
         zctx_destroy(&mContext);
         mFace = NULL;
         return false;
      }
      return true;
   }

   void RepeatMessages() {
      while (mRepeating.load()) {
         if (zsocket_poll(mFace, 100)) {
            zmsg_t* msg = zmsg_recv(mFace);
            //std::cout << "got " << zmsg_size(msg) << " parts" ;
            zframe_t* envelope = zmsg_pop(msg);
            std::string reply = zmsg_popstr(msg);
            //std::cout << "  with " << reply << std::endl;
            reply += " reply";
            zmsg_destroy(&msg);
            msg = zmsg_new();
            zmsg_add(msg, envelope);
            zmsg_addstr(msg, reply.c_str());
            zmsg_send(&msg, mFace);
         }
      }

   }

   void BeginListenAndRepeat() {

      mRepeating.store(true);
      mRepeaterThread = new std::thread(&MockSkelleton::RepeatMessages, this);
   }

   void EndListendAndRepeat() {
      mRepeating.store(false);

      if (mRepeaterThread) {
         mRepeaterThread->join();
         delete mRepeaterThread;
         mRepeaterThread = NULL;
      }
   }
private:
   std::atomic<bool> mRepeating;
   std::thread* mRepeaterThread;
};
#endif

