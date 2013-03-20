/* 
 * Author: Robert Weber
 *
 * Created on November 14, 2012, 2:21 PM
 */
#pragma once
#include <stdint.h>
#include <zmq.h>
#include <zlib.h>
#include <czmq.h>
#include <zframe.h>
#include <map>
#include <string>
#include <set>
#include "boost/thread.hpp"
#include "include/global.h"
#include "Headcrab.h"

class Crowbar {
public:
   explicit Crowbar(const std::string& binding);
   explicit Crowbar(const Headcrab& target);
   Crowbar(const std::string& binding, zctx_t* context);
   virtual ~Crowbar();

   bool Wield();
   bool Swing(const std::string& hit);
   bool Flurry( std::vector<std::string>& hits);
   bool BlockForKill(std::vector<std::string>& guts);
   bool WaitForKill(std::vector<std::string>& guts, const int timeout);
   bool BlockForKill(std::string& gut);
   bool WaitForKill(std::string& gut,const int timeout);
   void* GetTip();
   static int GetHighWater();
   zctx_t* GetContext();
private:

   Crowbar(const Crowbar& that) : mContext(NULL), mTip(NULL) {
   }

   zctx_t* mContext;
   std::string mBinding;
   void* mTip;
   bool mOwnsContext;
};
