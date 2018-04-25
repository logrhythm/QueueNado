/* Not any company's property but Public-Domain
* Do with source-code as you will. No requirement to keep this
* header if need to use it/change it/ or do whatever with it
*
* Note that there is No guarantee that this code will work
* and I take no responsibility for this code and any problems you
* might get if using it.
*
* Modified from original test at:  https://github.com/KjellKod/Q
*/

#include "QAPI.h"
#include "QApiTests.h"
#include <q/spsc.hpp>
#include <q/mpmc.hpp>
#include <thread>
#include <algorithm>

namespace {
   const size_t kAmount = 1000000;
   const size_t kSmallQueueSize = 100;
}



using namespace QApiTests;

TEST(Performance, SPSC_Flexible_CircularFifo) {
   auto queue = QAPI::CreateQueue<spsc::flexible::circular_fifo<std::string>>(kAmount);
   RunSPSC(queue, kAmount);
}

TEST(Performance, SPSC_Flexible_CircularFifo_Smaller) {
   auto queue = QAPI::CreateQueue<spsc::flexible::circular_fifo<std::string>>(kSmallQueueSize);
   RunSPSC(queue, kAmount);
}


TEST(Performance, SPSC_Fixed_CircularFifo) {
   using namespace std;
   auto queue = QAPI::CreateQueue<spsc::fixed::circular_fifo<string, kAmount>>();
   RunSPSC(queue, kAmount);
}


TEST(Performance, SPSC_Fixed_CircularFifo_Smaller) {
   using namespace std;
   auto queue = QAPI::CreateQueue < spsc::fixed::circular_fifo < string, kSmallQueueSize>> ();
   RunSPSC(queue, kAmount);
}



TEST(Performance, MPMC_1_to_1) {
   using namespace std;
   auto queue = QAPI::CreateQueue<mpmc::dynamic_lock_queue<string>>(kAmount, std::chrono::milliseconds(1));
   RunSPSC(queue, kAmount);
}


TEST(Performance, MPMC_1_to_1_Smaller) {
   using namespace std;

   auto queue = QAPI::CreateQueue<mpmc::dynamic_lock_queue<string>>(kSmallQueueSize,
                std::chrono::milliseconds(1));
   RunSPSC(queue, kAmount);
}


template<typename T>
void RunMPSC(T queue) {
   std::atomic<bool> producerStart{false};
   std::atomic<bool> consumerStart{false};

 
   using namespace std;
   using namespace std::chrono;

   auto producer = std::get <QAPI::index::sender>(queue);
   auto consumer = std::get <QAPI::index::receiver>(queue);


   auto start1 = 0;
   auto stop1 = kAmount / 4 * 1;

   auto start2 = kAmount / 4 * 1;
   auto stop2 = kAmount / 4 * 2;

   auto start3 = kAmount / 4 * 2;
   auto stop3 = kAmount / 4 * 3;

   auto start4 = kAmount / 4 * 3;
   auto stop4 = kAmount / 4 * 4;

   auto start0 = 0;
   auto stop0 = kAmount;

   auto t1 = high_resolution_clock::now();

   auto p1 = std::async(std::launch::async, Push<decltype(producer)>, producer, start1, stop1,
                        std::ref(producerStart), std::ref(consumerStart));
   auto p2 = std::async(std::launch::async, Push<decltype(producer)>, producer, start2, stop2,
                        std::ref(producerStart), std::ref(consumerStart));
   auto p3 = std::async(std::launch::async, Push<decltype(producer)>, producer, start3, stop3,
                        std::ref(producerStart), std::ref(consumerStart));
   auto p4 = std::async(std::launch::async, Push<decltype(producer)>, producer, start4, stop4,
                        std::ref(producerStart), std::ref(consumerStart));
   auto c0 = std::async(std::launch::async, Get<decltype(consumer)>, consumer, start0, stop0,
                        std::ref(producerStart), std::ref(consumerStart));


   auto e1 = p1.get();
   auto e2 = p2.get();
   auto e3 = p3.get();
   auto e4 = p4.get();
   auto received =  c0.get();
   auto t2 = high_resolution_clock::now();

   auto us = duration_cast<microseconds>( t2 - t1 ).count();
   std::cout << "Push - Pull #" << kAmount << " items in: " << us  << " us" << std::endl;
   std::cout << "Average: " << 1000 * ((float)us / (float) kAmount) << " ns" << std::endl;


   std::vector<std::string> eAll;
   //eAll.reserve(kAmount);
   eAll.insert(eAll.end(), e1.begin(), e1.end());
   e1.clear();
   eAll.insert(eAll.end(), e2.begin(), e2.end());
   e2.clear();
   eAll.insert(eAll.end(), e3.begin(), e3.end());
   e3.clear();
   eAll.insert(eAll.end(), e4.begin(), e4.end());
   e4.clear();

   std::sort(eAll.begin(), eAll.end());
   std::sort(received.begin(), received.end());
   EXPECT_EQ(kAmount, received.size());
   EXPECT_EQ(eAll, received);
}


TEST(Performance, MPMC_4_to_1) {
   using namespace std;
   auto queue = QAPI::CreateQueue<mpmc::dynamic_lock_queue<string>>(kAmount,
                std::chrono::milliseconds(1));
   RunMPSC(queue);
}

TEST(Performance, MPMC_4_to_1_Smaller) {
   using namespace std;
   auto queue = QAPI::CreateQueue<mpmc::dynamic_lock_queue<string>>(kSmallQueueSize,
                std::chrono::milliseconds(1));
   RunMPSC(queue);
}
