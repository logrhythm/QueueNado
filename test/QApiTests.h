#pragma once
#include <gtest/gtest.h>
#include <string>
#include <chrono>
#include <vector>
#include <atomic>
#include <iostream>
#include <future>

namespace QApiTests {
   using ResultType = std::vector<std::string>;


   template <typename Sender>
   ResultType Push(Sender q, size_t start, size_t stop, std::atomic<bool>& producerStart, std::atomic<bool>& consumerStart) {
      using namespace std::chrono_literals;
      std::vector<std::string> expected;
      expected.reserve(stop - start);
      producerStart.store(true);
      using namespace std::chrono_literals;
      while (!consumerStart.load()) {
         std::this_thread::sleep_for(2ns);
      }

      for (auto i = start; i < stop; ++i) {
         std::string value = std::to_string(i);
         expected.push_back(value);
         while (false == q.push(value)) {
            std::this_thread::sleep_for(1us); // // yield is too aggressive
         }
      }
      std::cout << q.mStats.FlushAsString() << std::endl;
      return expected;
   }


   template <typename Receiver>
   ResultType Get(Receiver q, size_t start, size_t stop, std::atomic<bool>& producerStart, std::atomic<bool>& consumerStart) {
      using namespace std::chrono_literals;
      std::vector<std::string> received;
      received.reserve(stop - start);
      consumerStart.store(true);
      while (!producerStart.load()) {
         std::this_thread::sleep_for(2ns);
      }

      for (auto i = start; i < stop; ++i) {
         std::string value;
         while (false == q.pop(value)) {
            std::this_thread::sleep_for(1us); // yield is too aggressive
         }
         received.push_back(value);
      }
      std::cout << q.mStats.FlushAsString() << std::endl;
      return received;
   }



   template<typename T>
   void RunSPSC(T queue, const int howMany) {
      std::atomic<bool> producerStart{false};
      std::atomic<bool> consumerStart{false};

      using namespace std;
      using namespace chrono;
      auto producer = std::get <QAPI::index::sender>(queue);
      auto consumer = std::get <QAPI::index::receiver>(queue);

      auto t1 = high_resolution_clock::now();
      size_t start = 0;
      size_t stop = howMany;
      auto prodResult = std::async(std::launch::async, Push<decltype(producer)>,
                                   producer, start, stop, std::ref(producerStart), std::ref(consumerStart));
      auto consResult = std::async(std::launch::async, Get<decltype(consumer)>,
                                   consumer, start, stop, std::ref(producerStart), std::ref(consumerStart));

      auto expected = prodResult.get();
      auto received = consResult.get();
      auto t2 = high_resolution_clock::now();
      auto us = duration_cast<microseconds>( t2 - t1 ).count();
      std::cout << "Push - Pull #" << howMany << " items in: " << us  << " us" << std::endl;
      std::cout << "Average: " << 1000 * ((float)us / (float) howMany) << " ns" << std::endl;

      EXPECT_EQ(howMany, received.size());
      EXPECT_EQ(expected, received);
   }

} // Q API Tests

