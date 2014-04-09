#include <gtest/gtest.h>
#include <string>
#include "concurrent.hpp"
#include <vector>
#include <atomic>
#include <iostream>
#include <memory>
#include <chrono>
#include <iostream>
namespace {
   struct DummyObject { };

   // Verify clean destruction
   struct TrueAtExit {
      std::atomic<bool>* flag;
      explicit TrueAtExit(std::atomic<bool>* f) : flag(f) {
         flag->store(false);
      }

      bool value() { return *flag; }

      ~TrueAtExit() { flag->store(true); }

      // concurrent improvement from the original Herb Sutter example
      // Which would copy/move the object into the concurrent wrapper
      // i.e the original concurrent wrapper could not use an object such as this (or a unique_ptr for that matter)
      TrueAtExit(const TrueAtExit&) = delete;
      TrueAtExit& operator=(const TrueAtExit&) = delete;
   };


   // Verify concurrent runs,. "no" delay for the caller. 
   struct DelayedCaller {
      void DoDelayedCall() {
         std::this_thread::sleep_for(std::chrono::seconds(1));
      }
   };
}

   
TEST(TestOfConcurrent, CompilerCheckForEmptyStruct) {
   concurrent<DummyObject> doNothing1{};
   concurrent<DummyObject> doNothing2;
   concurrent<DummyObject> doNothing3 = {};
}



namespace {
   struct Animal {
      virtual std::string sound() = 0;
   };
   struct Dog : public Animal {
      std::string sound() override { return {"Wof Wof"}; }
   };
   
   struct Cat : public Animal {
     std::string sound() override { return {"Miauu Miauu"}; }
   };
}

TEST(TestOfConcurrent, CompilerCheckUniquePtrTest) {
   typedef std::unique_ptr<Animal> RaiiAnimal;
   concurrent<RaiiAnimal> animal1 {new Dog};
   concurrent<RaiiAnimal> animal2 {new Cat};
   
   auto make_sound = []( RaiiAnimal& animal ) { return animal->sound(); };
   EXPECT_EQ("Wof Wof", animal1(make_sound).get());   
   EXPECT_EQ("Miauu Miauu", animal2(make_sound).get());   
}


TEST(TestOfConcurrent, VerifyDestruction) {
   std::atomic<bool> flag{true};
   {
      concurrent<TrueAtExit> notifyAtExit1{&flag};
      EXPECT_FALSE(flag); // i.e. constructor has run
   }
   {
      EXPECT_TRUE(flag); // notifyAtExit destructor
      concurrent<TrueAtExit> notifyAtExit2 = {&flag};
      EXPECT_FALSE(flag);
   }
   EXPECT_TRUE(flag); // notifyAtExit destructor
}

TEST(TestOfConcurrent, VerifyFifoCalls) {

   concurrent<std::string> asyncString = {"start"};
   auto received = asyncString([](std::string & s) { s.append(" received message"); return std::string{s}; });
      
   auto clear = asyncString([](std::string & s) { s.clear(); return s; });
      
   EXPECT_EQ("start received message", received.get());
   EXPECT_EQ("", clear.get());

   std::string toCompare;
   for (size_t index = 0; index < 100000; ++index) {
      toCompare.append(std::to_string(index)).append(" ");
      asyncString([ = ](std::string & s){s.append(std::to_string(index)).append(" ");});
   }
   
   auto appended = asyncString([](const std::string & s) { return s; });
   EXPECT_EQ(appended.get(), toCompare);
}

TEST(TestOfConcurrent, VerifyImmediateReturnForSlowFunctionCalls) {

   typedef std::chrono::steady_clock clock;
   auto start = clock::now();
   {
      concurrent<DelayedCaller> snail;
      for (size_t call = 0; call < 10; ++call) {
         snail([](DelayedCaller & slowRunner) { slowRunner.DoDelayedCall(); });
      }
      EXPECT_LT(std::chrono::duration_cast<std::chrono::seconds>(clock::now() - start).count(), 1);
   } // at destruction all 1 second calls will be executed before we quit
   
   EXPECT_TRUE(std::chrono::duration_cast<std::chrono::seconds>(clock::now() - start).count() >= 10); // 
}





