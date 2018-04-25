#pragma once

#include <tuple>
#include <memory>
#include <TimeStats.h>
#include <TriggerTimeStats.h>
#include <q/spsc.hpp>
#include <q/mpmc.hpp>

/* Inspired by https://github.com/KjellKod/Q/src/q/q_api.hpp*/


namespace QAPI {
// Base Queue API without pop() and push()
// It should be mentioned the thinking of what goes where
// it is a "controversy" whether what is tail and what is head
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
      bool lock_free() const { return mQueueRef.size(); }
      size_t usage() const { return mQueueRef.usage(); }

      std::shared_ptr<QType> mQueueStorage;
      QType& mQueueRef;
   };



// struct with: push() + base Queue API
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



// struct with : pop() + base Queue API
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
      TimeStats mStats;
   };  // ReceiverQ



   template<typename QType, typename... Args>
   std::pair<Sender<QType>, Receiver<QType>> CreateQueue(Args&& ... args) {
      std::shared_ptr<QType> ptr = std::make_shared<QType>(std::forward< Args >(args)...);
      return std::make_pair(Sender<QType> {ptr}, Receiver<QType> {ptr});
   }

   enum index {sender = 0, receiver = 1};
} // QueueAPI








