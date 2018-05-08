/* Not any company's property but Public-Domain
* Do with source-code as you will. No requirement to keep this
* header if need to use it/change it/ or do whatever with it
*
* Note that there is No guarantee that this code will work
* and I take no responsibility for this code and any problems you
* might get if using it.
*
* Code & platform dependent issues with it was originally
* published at http://www.kjellkod.cc/threadsafecircularBase
* 2012-16-19  @author Kjell Hedström, hedstrom@kjellkod.cc
*
* Modified and inspired from KjellKod's code at:
* https://github.com/KjellKod/Q/src/q/q_api.hpp
*/

#pragma once

#include <tuple>
#include <memory>
#include <TimeStats.h>
#include <TriggerTimeStats.h>
#include <q/spsc.hpp>
#include <q/mpmc.hpp>

namespace QAPI {
   // Base Queue API without pop() and push()
   // This follows the 'tail' first design on FIFO
   // http://en.wikipedia.org/wiki/FIFO#Head_or_tail_first
   // This implementation follows "pop on head", "push on tail"
   template<typename QType>
   struct Base {
      Base(std::shared_ptr<QType> q)
         : mQueueStorage(q)
         , mQueueRef(*(q.get())) {
      }
      bool empty() const { return mQueueRef.empty();}
      bool full() const { return mQueueRef.full(); }
      size_t capacity() const { return mQueueRef.capacity(); }
      size_t capacity_free() const { return mQueueRef.capacity_free(); }
      size_t size() const { return mQueueRef.size(); }
      // GCC BUG:  std::atomic<size_t>{}.is_lock_free(); is not implemented in gcc 4.8.5
      // It is implemented in later versions (gc 5.3 and newer)
      //bool lock_free() const { return mQueueRef.lock_free(); } 
      size_t usage() const { return mQueueRef.usage(); }

      std::shared_ptr<QType> mQueueStorage;
      QType& mQueueRef;
   };


   // push() + base Queue API
   template<typename QType>
   struct Sender : public Base<QType> {
    public:
      Sender(std::shared_ptr<QType> q): Base<QType>(q) {}
      virtual ~Sender() = default;

      template<typename Element>
      bool push(Element& item) {
         TriggerTimeStats trigger(mStats);
         auto result = Base<QType>::mQueueRef.push(item);
         if (!result) {
            trigger.Skip();
         }
         return result;
      }
      TimeStats mStats;
   };


   namespace sfinae {
      // SFINAE: Substitution Failure Is Not An Error
      // Decide at compile time what function signature to use
      // 1. If 'wait_and_pop' exists in the queue it uses that
      // 2. If only 'pop' exists it implements 'wait_and_pop' expected
      // -- FYI: The wait is set to increments of 100 ns
      template <typename T, typename Element>
      bool wrapper(T& t, Element& e, std::chrono::milliseconds max_wait) {
         using milliseconds = std::chrono::milliseconds;
         using clock = std::chrono::steady_clock;
         using namespace std::chrono_literals;
         auto t1 = clock::now();
         bool result = false;
         while (!(result = t.pop(e))) {
            std::this_thread::sleep_for(100ns);
            auto elapsed_ms = std::chrono::duration_cast<milliseconds>(clock::now() - t1);
            if (elapsed_ms > max_wait) {
               return result;
            }
         }
         return result;
      }

      template <typename T, typename Element>
      auto match_call(T& t, Element& e, std::chrono::milliseconds ms, int) -> decltype( t.wait_and_pop(e, ms) )
      { return t.wait_and_pop(e, ms); }

      template <typename T, typename Element>
      auto match_call(T& t, Element& e, std::chrono::milliseconds ms, long) -> decltype( wrapper(t, e, ms) )
      { return wrapper(t, e, ms); }

      template <typename T, typename Element>
      int wait_and_pop (T& t, Element& e, std::chrono::milliseconds ms) {
         // SFINAE magic happens with the '0'.  
         // For the matching call the '0' will be typed to int. 
         // For non-matching call it will be typed to long
         return match_call(t, e, ms, 0);
      }
   } // sfinae



   // pop(), wait_and_pop + base Queue API
   // if the QType does not support wait_and_pop then 
   // it will follow the sfinae::wrapper's wait_and_pop implementation
   template<typename QType>
   struct Receiver : public Base<QType> {
    public:
      Receiver(std::shared_ptr<QType> q) :  Base<QType>(q) {}
      virtual ~Receiver() = default;

      template<typename Element>
      bool pop(Element& item) {
         TriggerTimeStats trigger(mStats);
         auto result =  Base<QType>::mQueueRef.pop(item);
         if (!result) {
            trigger.Skip();
         }
         return result;
      }

      template<typename Element>
      bool wait_and_pop(Element& item, const std::chrono::milliseconds wait_ms) {
         TriggerTimeStats trigger(mStats);
         auto result = sfinae::wait_and_pop(Base<QType>::mQueueRef, item, wait_ms);
         if (!result) {
            trigger.Skip();
         }
         return result;
      }
      TimeStats mStats;
   };  


   template<typename QType, typename... Args>
   std::pair<Sender<QType>, Receiver<QType>> CreateQueue(Args&& ... args) {
      std::shared_ptr<QType> ptr = std::make_shared<QType>(std::forward< Args >(args)...);
      return std::make_pair(Sender<QType> {ptr}, Receiver<QType> {ptr});
   }

   enum index {sender = 0, receiver = 1};
} // QueueAPI
