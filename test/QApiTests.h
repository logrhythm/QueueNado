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
      std::ostringstream oss;
      oss << "Push Until: " << q.mStats.FlushAsString() << std::endl;
      std::cout << oss.str();
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
      std::ostringstream oss;
      oss << "GetUntil: " << q.mStats.FlushAsString() << std::endl;
      std::cout << oss.str();
      return received;
   }



   template <typename Sender>
   size_t PushUntil(Sender q, std::string data, const size_t numberOfConsumers, std::atomic<size_t>& producerCount, std::atomic<size_t>& consumerCount, std::atomic<bool>& stopRunning) {
      using namespace std::chrono_literals;
      producerCount++;
      using namespace std::chrono_literals;
      while (numberOfConsumers < consumerCount.load()) {
         std::this_thread::sleep_for(2ns);
      }

      StopWatch watch;
      size_t amountPushed = 0;
      while (!stopRunning.load()) {
         std::string value = data;
         while (false == q.push(value) && !stopRunning.load()) {
            std::this_thread::sleep_for(100ns); // yield is too aggressive
         }
         ++amountPushed;
      }

      std::ostringstream oss;
      oss << "Push Until: " << q.mStats.FlushAsString() << std::endl;
      std::cout << oss.str();
      return amountPushed;
   }


   template <typename Receiver>
   size_t GetUntil(Receiver q, const std::string data, const size_t numberOfProducers, std::atomic<size_t>& producerCount, std::atomic<size_t>& consumerCount, std::atomic<bool>& stopRunning) {
      using namespace std::chrono_literals;
      consumerCount++;
      while (numberOfProducers < producerCount.load()) {
         std::this_thread::sleep_for(2ns);
      }

      StopWatch watch;
      size_t amountReceived = 0;
      while (!stopRunning.load()) {
         std::string value;
         while (false == q.pop(value) && !stopRunning.load()) {
            std::this_thread::sleep_for(100ns); // yield is too aggressive
         }
         if (!stopRunning.load()) {
            EXPECT_EQ(data.size(), value.size());
            EXPECT_FALSE(value.empty());
            ++amountReceived;
         }
      }
      std::ostringstream oss;
      oss << "GetUntil: " << q.mStats.FlushAsString() << std::endl;
      std::cout << oss.str();
      return amountReceived;
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

   template<typename T>
   void RunMPMC(T queue, std::string data, size_t numberProducers, size_t numberConsumers, const size_t timeToRunInSec) {
      std::atomic<size_t> producerCount{0};
      std::atomic<size_t> consumerCount{0};
      std::atomic<bool> producerStop{false};
      std::atomic<bool> consumerStop{false};


      using namespace std;
      using namespace std::chrono;
      auto producer = std::get <QAPI::index::sender>(queue);
      auto consumer = std::get <QAPI::index::receiver>(queue);
      std::vector<std::future<size_t>> producerResult;
      producerResult.reserve(numberProducers);

      for (size_t i = 0; i < numberProducers; ++i) {
         producerResult.emplace_back(std::async(std::launch::async, PushUntil<decltype(producer)>, producer, data, numberProducers,
                                                std::ref(producerCount), std::ref(consumerCount), std::ref(producerStop)));
      }
      std::vector<std::future<size_t>> consumerResult;
      consumerResult.reserve(numberConsumers);
      for (size_t i = 0; i < numberProducers; ++i) {
         consumerResult.emplace_back(std::async(std::launch::async, GetUntil<decltype(consumer)>, consumer, data, numberConsumers,
                                                std::ref(producerCount), std::ref(consumerCount), std::ref(consumerStop)));
      }

      using namespace std::chrono_literals;
      while (consumerCount.load() < numberConsumers && producerCount.load() < numberProducers) {
         std::this_thread::sleep_for(1us);
      }
      StopWatch elapsedRun;
      while (elapsedRun.ElapsedSec() < timeToRunInSec) {
         std::this_thread::sleep_for(1us);
      }

      producerStop.store(true);
      size_t amountProduced = 0;
      for (auto& result : producerResult) {
         amountProduced += result.get();
      }
      consumerStop.store(true);
      size_t amountConsumed = 0;
      for (auto& result : consumerResult) {
         amountConsumed += result.get();
      }

      auto elapsedTimeSec = elapsedRun.ElapsedSec();
      EXPECT_GE(amountConsumed + 100, amountProduced);
      std::cout << "Transaction/s: " << amountConsumed / elapsedTimeSec << std::endl;
      std::cout << "Transaction/s per consumer: " << amountConsumed / elapsedTimeSec / numberConsumers << std::endl;
      std::cout << "Transation MByte/s: " << amountConsumed* data.size() / (1024 * 1024) / elapsedTimeSec << std::endl;
   }

} // Q API Tests

