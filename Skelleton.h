#pragma once
#include <zmq.h>
#include <czmq.h>

class Skelleton {
public:

   explicit Skelleton(const std::string& binding) : mBinding(binding), mContext(NULL), mFace(NULL) {

   }

   virtual ~Skelleton() {
      if (mContext) {
         zctx_destroy(&mContext);
      }
   }

   virtual bool Initialize() {
      return false;
   }
protected:
   std::string mBinding;
   zctx_t* mContext;
   void* mFace;
};

